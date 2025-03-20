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
	////////////////////////////////// 设置请求的内容 ////////////////////////////////
	
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


	////////////////////////////////// 绑定Dify【响应时】的回调 ////////////////////////////////
	
	HttpRequest->OnRequestProgress64().BindLambda([this](const FHttpRequestPtr& _Request, uint64 _BytesSent, uint64 _BytesReceived)
	{
		UE_LOG(LogTemp, Log, TEXT("BytesSent: %llu, BytesReceived: %llu"), _BytesSent, _BytesReceived);

		const FHttpResponsePtr response = _Request->GetResponse();

		//不存在就说明请求失败
		if(!response.IsValid())
		{
			FString logText = "[DifyChat]:\nRequest failed";
			UE_LOG(LogTemp, Log, TEXT("%s"), *logText);
			return ;
		}

		//获取返回的字符串格式的数据
		FString responseString = response->GetContentAsString();
		
		TArray<FString> dataBlocks;

		//用正则匹配data:{...}
		//const FRegexPattern difyResponsePattern(TEXT("data: \\{.*?\\}"));
		const FRegexPattern difyResponsePattern(TEXT("\\{\"event\": \"message\".*?\\}"));
		FRegexMatcher difyResponseMatcher(difyResponsePattern, responseString);

		// 这里应该可以优化，但是我不知道怎么搞:)
		while (difyResponseMatcher.FindNext())
		{
			FString match = difyResponseMatcher.GetCaptureGroup(0);
			dataBlocks.Add(match);
		}
		

		//UE_LOG(LogTemp, Log, TEXT("[DataBlocks]:%s"),*responseString);

		
		int testI = dataBlocks.Num();
		UE_LOG(LogTemp, Log, TEXT("[DataBlocks.Num]:%d"),testI);

		//有时会同时新返回多个data{}，所以要用循环
		for(int i = LastDataBlocksIndex; i < dataBlocks.Num(); i++)
        {
			responseString = dataBlocks[i];

			//解析返回的数据，然后直接广播委托
			ParseDifyResponse(responseString);
        }

		

		//更新索引
		if(dataBlocks.Num() > LastDataBlocksIndex) // 有时返回是空的，一行都没有
			LastDataBlocksIndex = dataBlocks.Num();
		
		
		//lastResponse.TrimStartAndEndInline(); //不知道要不要这一行
	});
	
	////////////////////////////////// 绑定Dify【响应后】的回调 ////////////////////////////////

	HttpRequest->OnProcessRequestComplete().BindLambda([this](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
	{
		// if (bWasSuccessful && Response.IsValid())
		// {
		// 	FString logText =
		// 		"[DifyChat]:\nCode：" + Response->GetResponseCode();
		// 	logText+= "\n" + Response->GetContentAsString();
		// 	UE_LOG(LogTemp, Log, TEXT("%s"), *logText);
		// }
		// else
		// {
		// 	FString logText = "[DifyChat]:\nRequest failed";
		// 	UE_LOG(LogTemp, Log, TEXT("%s"), *logText);
		// }
		// //解析返回的数据，然后直接广播委托
		// ParseDifyResponse(Response->GetContentAsString());
		
		//OnDifyChatResponse.Broadcast(*Response->GetContentAsString());

		//广播【响应后】委托
		OnDifyChatResponded.Broadcast();

		//下一轮的返回索引当然从0开始
		LastDataBlocksIndex = 0;
		
		//设置为不在等待返回
		bIsWaitingDifyResponse = false;
		
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
		//_Response
		UE_LOG(LogTemp, Error, TEXT("[Error_Response]:\n%s"), *_Response);
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
	///answer
	////////blocking/////////
	///mode
	///metadata(用不着应该)
	////////////////////////// 解析JSON /////////////////////////
	FDifyChatResponse difyChatResponse;
	
	difyChatResponse.event = JsonObject->GetStringField(TEXT("event"));

	//message_end事件解析不过，不用再次判断
	//message_end事件，不需要解析,等结束就行了
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

	// 阻塞模式下，还有mode字段
	if(DifyChatResponseMode == EDifyChatResponseMode::Blocking)
	{
		difyChatResponse.mode = JsonObject->GetStringField(TEXT("mode"));
	}
	
	UE_LOG(LogTemp, Log, TEXT("\n[event]:\n%s"), *difyChatResponse.event);
	UE_LOG(LogTemp, Log, TEXT("\n[answer]:\n%s"), *difyChatResponse.answer);

	
	//保存ConversationID
	ConversationID = difyChatResponse.conversation_id;

	//广播【响应时】委托
	OnDifyChatResponding.Broadcast(difyChatResponse);

	// 阻塞模式 或者 事件为message_end时 说明返回结束
	//bool bIsMessageEnd = false;
	//bIsMessageEnd = (DifyChatResponseMode == EDifyChatResponseMode::Blocking)
					//|| (difyChatResponse.event == "message_end");
	
	//UE_LOG(LogTemp, Log, TEXT("[event]:\n%s"), *difyChatResponse.event);
	
	
	//广播委托,让蓝图使用

	//if(!bIsMessageEnd)
	//	OnDifyChatResponding.Broadcast(difyChatResponse);
	//else
	//	OnDifyChatResponded.Broadcast(difyChatResponse);


	
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

	//广播[向Dify对话后]委托
	OnDifyChatTalkTo.Broadcast(UserName, _Message);
}


