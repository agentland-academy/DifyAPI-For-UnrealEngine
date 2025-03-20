// Copyright PengHY 2025. All rights Reserved.


#include "DifyChatComponent.h"

#include "HttpModule.h"
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
	DifyChatResponseMode = EDifyChatResponseMode::Blocking;
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
	////////////////////////////////// ������������� ////////////////////////////////
	
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();

	HttpRequest->SetURL(DifyURL);

	HttpRequest->SetVerb(TEXT("POST"));

	FString authorization = "Bearer " + DifyAPIKey;

	HttpRequest->SetHeader(TEXT("Authorization"), authorization);

	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetObjectField(TEXT("inputs"), MakeShareable(new FJsonObject));
	JsonObject->SetStringField(TEXT("query"), _Message);


	FString responseMode = "";

	if(DifyChatResponseMode == EDifyChatResponseMode::Blocking)
		responseMode = "blocking";
	else
		responseMode = "streaming";
	
	//��ʽ or ����
	JsonObject->SetStringField(TEXT("response_mode"), responseMode);

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


	////////////////////////////////// ��Dify����Ӧʱ���Ļص� ////////////////////////////////
	
	HttpRequest->OnRequestProgress64().BindLambda([this](const FHttpRequestPtr& _Request, uint64 _BytesSent, uint64 _BytesReceived)
	{
		UE_LOG(LogTemp, Log, TEXT("BytesSent: %llu, BytesReceived: %llu"), _BytesSent, _BytesReceived);
		OnDifyResponding(_Request);
	});
	
	////////////////////////////////// ��Dify����Ӧ�󡿵Ļص� ////////////////////////////////

	HttpRequest->OnProcessRequestComplete().BindLambda([this](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
	{
		OnDifyResponded();
	});
	
	////////////////////////////////// �������� ////////////////////////////////

	HttpRequest->ProcessRequest();
}


//----------------------------------------------------
// Ŀ�ģ���Dify�ظ�ʱ����ȡ���ص�Դ����
//----------------------------------------------------
void UDifyChatComponent::OnDifyResponding(const FHttpRequestPtr& _Request)
{
	const FHttpResponsePtr response = _Request->GetResponse();

	//�����ھ�˵������ʧ��
	if(!response.IsValid())
	{
		FString logText = "[DifyChat]:\nRequest failed";
		UE_LOG(LogTemp, Log, TEXT("%s"), *logText);
		return ;
	}

	//��ȡ���ص��ַ�����ʽ������
	FString responseString = response->GetContentAsString();
	
	//�����blockingģʽ��ֱ�ӽ�������
	if(DifyChatResponseMode == EDifyChatResponseMode::Blocking)
	{
		ParseDifyResponse(responseString);
		return ;
	}

	//�����streamingģʽ��һ��������
		
	TArray<FString> dataBlocks;

	//������ƥ��data:{...}
	const FRegexPattern difyResponsePattern(TEXT("\\{\"event\": \"message\".*?\\}"));//"data: \\{.*?\\}"
	FRegexMatcher difyResponseMatcher(difyResponsePattern, responseString);

	// ����Ӧ�ÿ����Ż��������Ҳ�֪����ô��:)
	while (difyResponseMatcher.FindNext())
	{
		FString match = difyResponseMatcher.GetCaptureGroup(0);
		dataBlocks.Add(match);
	}
		

	//UE_LOG(LogTemp, Log, TEXT("[DataBlocks]:%s"),*responseString);
		
	int testI = dataBlocks.Num();
	UE_LOG(LogTemp, Log, TEXT("[DataBlocks.Num]:%d"),testI);

	//��ʱ��ͬʱ�·��ض��data{}������Ҫ��ѭ��
	for(int i = LastDataBlocksIndex; i < dataBlocks.Num(); i++)
	{
		responseString = dataBlocks[i];
			
		//�������ص����ݣ�Ȼ��ֱ�ӹ㲥ί��
		ParseDifyResponse(responseString);
	}
		
	//��������
	if(dataBlocks.Num() > LastDataBlocksIndex) // ��ʱ�����ǿյģ�һ�ж�û��
		LastDataBlocksIndex = dataBlocks.Num();
}


//----------------------------------------------------
// Ŀ�ģ���Dify�ظ������󣬹㲥ί�У���������
//----------------------------------------------------
void UDifyChatComponent::OnDifyResponded()
{
	//�㲥����Ӧ��ί��
	OnDifyChatResponded.Broadcast();

	//��һ�ֵķ���������Ȼ��0��ʼ
	LastDataBlocksIndex = 0;
		
	//����Ϊ���ڵȴ�����
	bIsWaitingDifyResponse = false;
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
		//_Response
		UE_LOG(LogTemp, Error, TEXT("[Error_Response]:\n%s"), *_Response);
		UE_LOG(LogTemp, Error, TEXT("Failed to parse JSON"));
		return;
	}

	////////////////////////// JSON�ο� //////////////////////////
	///event 
	///task_id 
	///id 
	///message_id 
	///conversation_id 
	///created_at
	///answer
	////////blocking/////////
	///mode
	///metadata(�ò���Ӧ��)
	////////////////////////// ����JSON /////////////////////////
	FDifyChatResponse difyChatResponse;
	
	difyChatResponse.event = JsonObject->GetStringField(TEXT("event"));

	//message_end�¼����������������ٴ��ж�
	//message_end�¼�������Ҫ����,�Ƚ���������
	//if(difyChatResponse.event == "message_end")
    //{
    //    return;
    //}
	
	difyChatResponse.task_id		= JsonObject->GetStringField(TEXT("task_id"));
	difyChatResponse.id				= JsonObject->GetStringField(TEXT("id"));
	difyChatResponse.message_id		= JsonObject->GetStringField(TEXT("message_id"));
	difyChatResponse.conversation_id= JsonObject->GetStringField(TEXT("conversation_id"));
	difyChatResponse.created_at		= JsonObject->GetStringField(TEXT("created_at"));
	difyChatResponse.ChatName		= ChatName;
	difyChatResponse.answer			= JsonObject->GetStringField(TEXT("answer"));

	// ����ģʽ�£�����mode�ֶ�
	if(DifyChatResponseMode == EDifyChatResponseMode::Blocking)
	{
		difyChatResponse.mode = JsonObject->GetStringField(TEXT("mode"));
	}
	
	UE_LOG(LogTemp, Log, TEXT("\n[event]:\n%s"), *difyChatResponse.event);
	UE_LOG(LogTemp, Log, TEXT("\n[answer]:\n%s"), *difyChatResponse.answer);

	
	//����ConversationID
	ConversationID = difyChatResponse.conversation_id;

	//�㲥����Ӧʱ��ί��
	OnDifyChatResponding.Broadcast(difyChatResponse);
	
}

//----------------------------------------------------
// Ŀ�ģ���һ���ڵ����ʼ��DifyChat
//----------------------------------------------------
void UDifyChatComponent::InitDifyChat(FString _DifyURL, FString _DifyAPIKey, FString _ChatName, FString _UserName,
		EDifyChatType _DifyChatType, EDifyChatResponseMode _DifyChatResponseMode)
{
	DifyURL					= _DifyURL;
	DifyAPIKey				= _DifyAPIKey;
	ChatName				= _ChatName;
	UserName				= _UserName;
	DifyChatType			= _DifyChatType;
	DifyChatResponseMode	= _DifyChatResponseMode;
}


//----------------------------------------------------
// Ŀ�ģ���Dify������Ϣ
//----------------------------------------------------
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

	//�㲥[��Dify�Ի���]ί��
	OnDifyChatTalkTo.Broadcast(UserName, _Message);
}


