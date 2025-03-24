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

void UDifyChatComponent::BeginDestroy()
{
	//如果请求还在进行，先取消
	if(CurrentHttpRequest.IsValid())
	{
		CurrentHttpRequest->CancelRequest();
		CurrentHttpRequest.Reset();
	}
	Super::BeginDestroy();
}


//----------------------------------------------------
// 目的：向Dify发送Post请求
//----------------------------------------------------
void UDifyChatComponent::SentDifyPostRequest(FString _Message)
{
	////////////////////////////////// 设置请求的内容 ////////////////////////////////
	
	CurrentHttpRequest = FHttpModule::Get().CreateRequest();

	 
	
	CurrentHttpRequest->SetURL(DifyURL);
	CurrentHttpRequest->SetVerb(TEXT("POST"));

	//KEY
	FString authorization = "Bearer " + DifyAPIKey;
	CurrentHttpRequest->SetHeader(TEXT("Authorization"), authorization);
	CurrentHttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);


	//  "inputs": {"Prompt": "角色设定：\r身份背景\r2070年代传奇摇滚小子/恐怖分子\r前军用科技特工"}
	
	//Inputs
	TSharedPtr<FJsonObject> InputsObject = MakeShareable(new FJsonObject);
	for(FDifyChatInputs& input : DifyInputs)
    {
		FString key = input.Key;
		FString value = input.Value;
		
        InputsObject->SetStringField(input.Key, input.Value);
    }
	JsonObject->SetObjectField(TEXT("inputs"), InputsObject);
	
	//Query
	JsonObject->SetStringField(TEXT("query"), _Message);

	//流式 or 阻塞
	FString responseMode = "";
	if(DifyChatResponseMode == EDifyChatResponseMode::Blocking)
		responseMode = "blocking";
	else
		responseMode = "streaming";
	JsonObject->SetStringField(TEXT("response_mode"), responseMode);
	
	
	//如果是单聊，就不传conversation_id
	FString conversation_id = "";
	if(DifyChatType == EDifyChatType::MultiChat)
		conversation_id = ConversationID;
	JsonObject->SetStringField(TEXT("conversation_id"), conversation_id);


	//username
	JsonObject->SetStringField(TEXT("user"), UserName);

	//FilesArray
	TArray<TSharedPtr<FJsonValue>> FilesArray;
	JsonObject->SetArrayField(TEXT("files"), FilesArray);//空数组

	// 将JSON对象转换为字符串
	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	// 设置请求内容
	CurrentHttpRequest->SetContentAsString(OutputString);


	////////////////////////////////// 绑定Dify【响应时】的回调 ////////////////////////////////
	
	CurrentHttpRequest->OnRequestProgress64().BindLambda(
	[WeakThis = TWeakObjectPtr<UDifyChatComponent>(this)]
		(const FHttpRequestPtr& _Request, uint64 _BytesSent, uint64 _BytesReceived)
	{
		UE_LOG(LogTemp, Log, TEXT("BytesSent: %llu, BytesReceived: %llu"), _BytesSent, _BytesReceived);

		if(WeakThis.IsValid())
			WeakThis->OnDifyResponding(_Request);
	});
	
	////////////////////////////////// 绑定Dify【响应后】的回调 ////////////////////////////////

	CurrentHttpRequest->OnProcessRequestComplete().BindLambda(
		[WeakThis = TWeakObjectPtr<UDifyChatComponent>(this)]
		(FHttpRequestPtr _Request, FHttpResponsePtr _Response, bool bWasSuccessful)
	{
		const int responseCode = _Response->GetResponseCode();
		// 只有代码为200才是正常响应
		if(responseCode != 200) 
		{
			FString logText = "[DifyChatError]:\nCode:" + FString::FromInt(responseCode);
			logText+= "\n" + _Response->GetContentAsString();
			//输出报错
			UE_LOG(LogTemp, Error, TEXT("%s"), *logText);
		}

		if(WeakThis.IsValid())
			WeakThis->OnDifyResponded();
	});
	
	////////////////////////////////// 发送请求 ////////////////////////////////

	CurrentHttpRequest->ProcessRequest();
}


//----------------------------------------------------
// 目的：在Dify回复时，获取返回的源数据
//----------------------------------------------------
void UDifyChatComponent::OnDifyResponding(const FHttpRequestPtr& _Request)
{
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
	
	//如果是blocking模式，直接解析数据
	if(DifyChatResponseMode == EDifyChatResponseMode::Blocking)
	{
		ParseDifyResponse(responseString);
		return ;
	}

	//如果是streaming模式，一条条解析
		
	TArray<FString> dataBlocks;

	//用正则匹配data:{...}
	const FRegexPattern difyResponsePattern(TEXT("\\{\"event\": \"message\".*?\\}"));//"data: \\{.*?\\}"
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
}


//----------------------------------------------------
// 目的：在Dify回复结束后，广播委托，重置属性
//----------------------------------------------------
void UDifyChatComponent::OnDifyResponded()
{
	//广播【响应后】委托
	OnDifyChatResponded.Broadcast();

	//下一轮的返回索引当然从0开始
	LastDataBlocksIndex = 0;
		
	//设置为不在等待返回
	bIsWaitingDifyResponse = false;
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
	
}

//----------------------------------------------------
// 目的：在一个节点里初始化DifyChat
//----------------------------------------------------
void UDifyChatComponent::InitDifyChat(FString _DifyURL, FString _DifyAPIKey, FString _ChatName, FString _UserName,
		EDifyChatType _DifyChatType, EDifyChatResponseMode _DifyChatResponseMode, TArray<FDifyChatInputs> _DifyInputs)
{
	DifyURL					= _DifyURL;
	DifyAPIKey				= _DifyAPIKey;
	ChatName				= _ChatName;
	UserName				= _UserName;
	DifyChatType			= _DifyChatType;
	DifyChatResponseMode	= _DifyChatResponseMode;
	DifyInputs				= _DifyInputs;
	
}


//----------------------------------------------------
// 目的：向Dify发送消息
//----------------------------------------------------
void UDifyChatComponent::TalkToAI(FString _Message)
{
	//如果上一个还没返回，就不发送
	if (bIsWaitingDifyResponse)
	{
		UE_LOG(LogTemp, Error, TEXT("[Waiting for the last response]"));
		return;
	}

	LastDataBlocksIndex = 0;

	//发送请求,并设置为正在等待返回
	SentDifyPostRequest(_Message);
	bIsWaitingDifyResponse = true;

	//广播[向Dify对话后]委托
	OnDifyChatTalkTo.Broadcast(UserName, _Message);
}


