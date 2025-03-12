// Copyright PengHY 2025. All rights Reserved.


#include "DifyChatComponent.h"

#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"


UDifyChatComponent::UDifyChatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	ChatName = "Default Dify Agent";
	UserName = "Player255";
	DifyChatType = EDifyChatType::SingleChat;
	ConversationID = "";
}



// Called when the game starts
void UDifyChatComponent::BeginPlay()
{
	Super::BeginPlay();

	//SentDifyPostRequest(TODO, TODO, TODO);
	
	// ...
	
}



//----------------------------------------------------
// Ŀ�ģ���Dify����Post����
//----------------------------------------------------
void UDifyChatComponent::SentDifyPostRequest(FString _Message)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();

	HttpRequest->SetURL(DifyURL);

	HttpRequest->SetVerb(TEXT("POST"));

	FString authorization = "Bearer " + DifyAPIKey;

	HttpRequest->SetHeader(TEXT("Authorization"), authorization);

	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetObjectField(TEXT("inputs"), MakeShareable(new FJsonObject));
	JsonObject->SetStringField(TEXT("query"), _Message);
	JsonObject->SetStringField(TEXT("response_mode"), TEXT("blocking"));

	//����ǵ��ģ��Ͳ���conversation_id
	FString conversation_id = "";

	if(DifyChatType == EDifyChatType::MultiChat)
		conversation_id = ConversationID;
	
	JsonObject->SetStringField(TEXT("conversation_id"), conversation_id); 
	JsonObject->SetStringField(TEXT("user"), UserName);

	
	TArray<TSharedPtr<FJsonValue>> FilesArray;
	JsonObject->SetArrayField(TEXT("files"), FilesArray);//������

	// ��JSON����ת��Ϊ�ַ���
	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	// ������������
	HttpRequest->SetContentAsString(OutputString);

	// �󶨻ص�����
	HttpRequest->OnProcessRequestComplete().BindLambda([this](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
	{
		if (bWasSuccessful && Response.IsValid())
		{
			FString logText =
				"[DifyChat]:\nCode��" + Response->GetResponseCode();
			logText+= "\n" + Response->GetContentAsString();
			UE_LOG(LogTemp, Log, TEXT("%s"), *logText);
		}
		else
		{
			FString logText = "[DifyChat]:\nRequest failed";
			UE_LOG(LogTemp, Log, TEXT("%s"), *logText);
		}
		//�������ص����ݣ�Ȼ��ֱ�ӹ㲥ί��
		ParseDifyResponse(Response->GetContentAsString());
		
		//OnDifyChatResponse.Broadcast(*Response->GetContentAsString());
		
	});

	HttpRequest->ProcessRequest();
	
}

//----------------------------------------------------
// Ŀ�ģ�����Dify���ص�����,���㲥ί��
//----------------------------------------------------
void UDifyChatComponent::ParseDifyResponse(FString _Response)
{
	////////////////////////// ����JSON���� //////////////////////////
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(_Response);
	if (!FJsonSerializer::Deserialize(Reader, JsonObject))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to parse JSON"));
		return;
	}

	////////////////////////// JSON�ο� //////////////////////////
	///event
	///task_id
	///id
	///message_id
	///conversation_id
	///mode
	///answer
	///metadata(�ò���Ӧ��)
	///created_at
	////////////////////////// ����JSON /////////////////////////
	FDifyChatResponse difyChatResponse;
	
	difyChatResponse.event			= JsonObject->GetStringField(TEXT("event"));
	difyChatResponse.task_id		= JsonObject->GetStringField(TEXT("task_id"));
	difyChatResponse.id				= JsonObject->GetStringField(TEXT("id"));
	difyChatResponse.message_id		= JsonObject->GetStringField(TEXT("message_id"));
	difyChatResponse.conversation_id= JsonObject->GetStringField(TEXT("conversation_id"));
	difyChatResponse.mode			= JsonObject->GetStringField(TEXT("mode"));
	difyChatResponse.answer			= JsonObject->GetStringField(TEXT("answer"));
	difyChatResponse.created_at		= JsonObject->GetStringField(TEXT("created_at"));
	difyChatResponse.ChatName		= ChatName;

	//����ConversationID
	ConversationID = difyChatResponse.conversation_id;
	
	//������ϣ������ٴη�������
	bIsWaitingDifyResponse = false;

	//�㲥ί��,����ͼʹ��
	OnDifyChatResponse.Broadcast(difyChatResponse);
}

void UDifyChatComponent::InitDifyChat(FString _DifyURL, FString _DifyAPIKey, FString _ChatName, FString _UserName,
		EDifyChatType _DifyChatType)
{
	DifyURL			= _DifyURL;
	DifyAPIKey		= _DifyAPIKey;
	ChatName		= _ChatName;
	UserName		= _UserName;
	DifyChatType	= _DifyChatType;
}

void UDifyChatComponent::TalkToAI(FString _Message)
{
	//�����һ����û���أ��Ͳ�����
	if (bIsWaitingDifyResponse)
	{
		UE_LOG(LogTemp, Error, TEXT("[Waiting for the last response]"));
		return;
	}

	//��������,������Ϊ���ڵȴ�����
	SentDifyPostRequest(_Message);
	bIsWaitingDifyResponse = true;

	//�㲥ί��,����ͼʹ��
	OnDifyChatTalkTo.Broadcast(UserName, _Message);
}


