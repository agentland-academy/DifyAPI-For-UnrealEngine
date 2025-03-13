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
// 目的：向Dify发送Post请求
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


	FString responseMode = "";

	if(DifyChatResponseMode == EDifyChatResponseMode::Blocking)
		responseMode = "blocking";
	else
		responseMode = "streaming";
	
	//流式 or 阻塞
	JsonObject->SetStringField(TEXT("response_mode"), responseMode);

	//如果是单聊，就不传conversation_id
	FString conversation_id = "";

	if(DifyChatType == EDifyChatType::MultiChat)
		conversation_id = ConversationID;
	
	JsonObject->SetStringField(TEXT("conversation_id"), conversation_id); 
	JsonObject->SetStringField(TEXT("user"), UserName);

	
	TArray<TSharedPtr<FJsonValue>> FilesArray;
	JsonObject->SetArrayField(TEXT("files"), FilesArray);//空数组

	// 将JSON对象转换为字符串
	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	// 设置请求内容
	HttpRequest->SetContentAsString(OutputString);


	// 绑定进度回调函数
	HttpRequest->OnRequestProgress64().BindLambda([this](FHttpRequestPtr _Request, uint64 BytesSent, uint64 BytesReceived)
	{
		UE_LOG(LogTemp, Log, TEXT("BytesSent: %d, BytesReceived: %d"), BytesSent, BytesReceived);

		const FHttpResponsePtr Response = _Request->GetResponse();


		//不存在就说明请求失败
		if(!Response.IsValid())
		{
			FString logText = "[DifyChat]:\nRequest failed";
			UE_LOG(LogTemp, Log, TEXT("%s"), *logText);
			return ;
		}
		
		
		//FString logText =
				//"[DifyChat]:\nCode：" + Response->GetResponseCode();
		//logText+= "\n" + Response->GetContentAsString();

		// 只需最后一个data{}
		FString lastResponse = Response->GetContentAsString();
		
		TArray<FString> DataBlocks;
		lastResponse.ParseIntoArray(DataBlocks, TEXT("data: "), true);
		
		if (DataBlocks.Num() > 0)
			lastResponse = DataBlocks.Last();
		
		lastResponse.TrimStartAndEndInline();
		

		UE_LOG(LogTemp, Log, TEXT("%s"), *lastResponse);

		//解析返回的数据，然后直接广播委托
		ParseDifyResponse(lastResponse);
		
	});
	

	

	// 绑定回调函数,请求完成后调用
	HttpRequest->OnProcessRequestComplete().BindLambda([this](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
	{
		if (bWasSuccessful && Response.IsValid())
		{
			FString logText =
				"[DifyChat]:\nCode：" + Response->GetResponseCode();
			logText+= "\n" + Response->GetContentAsString();
			UE_LOG(LogTemp, Log, TEXT("%s"), *logText);
		}
		else
		{
			FString logText = "[DifyChat]:\nRequest failed";
			UE_LOG(LogTemp, Log, TEXT("%s"), *logText);
		}
		//解析返回的数据，然后直接广播委托
		ParseDifyResponse(Response->GetContentAsString());
		
		//OnDifyChatResponse.Broadcast(*Response->GetContentAsString());
		
	});

	HttpRequest->ProcessRequest();
	
}

//----------------------------------------------------
// 目的：解析Dify返回的数据,并广播委托
//----------------------------------------------------
void UDifyChatComponent::ParseDifyResponse(FString _Response)
{
	////////////////////////// 创建JSON对象 //////////////////////////
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(_Response);
	if (!FJsonSerializer::Deserialize(Reader, JsonObject))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to parse JSON"));
		return;
	}

	////////////////////////// JSON参考 //////////////////////////
	///event
	///task_id
	///id
	///message_id
	///conversation_id
	///created_at
	////////blocking/////////
	///mode
	///answer
	///metadata(用不着应该)
	////////////////////////// 解析JSON /////////////////////////
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

	//保存ConversationID
	ConversationID = difyChatResponse.conversation_id;
	
	//解析完毕，可以再次发送请求
	bIsWaitingDifyResponse = false;

	// 阻塞模式 或者 事件为message_end时 说明返回结束
	bool bIsMessageEnd = false;
	bIsMessageEnd = (DifyChatResponseMode == EDifyChatResponseMode::Blocking) || 
		(difyChatResponse.event == "message_end");
	
	//UE_LOG(LogTemp, Log, TEXT("[event]:\n%s"), *difyChatResponse.event);
	
	
	//广播委托,让蓝图使用

	if(!bIsMessageEnd)
		OnDifyChatResponding.Broadcast(difyChatResponse);
	else
		OnDifyChatResponded.Broadcast(difyChatResponse);


	
}


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

void UDifyChatComponent::TalkToAI(FString _Message)
{
	//如果上一个还没返回，就不发送
	if (bIsWaitingDifyResponse)
	{
		UE_LOG(LogTemp, Error, TEXT("[Waiting for the last response]"));
		return;
	}

	//发送请求,并设置为正在等待返回
	SentDifyPostRequest(_Message);
	bIsWaitingDifyResponse = true;

	//广播委托,让蓝图使用
	OnDifyChatTalkTo.Broadcast(UserName, _Message);
}


