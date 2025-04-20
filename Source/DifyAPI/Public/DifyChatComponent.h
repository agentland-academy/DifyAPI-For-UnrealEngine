// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interfaces/IHttpRequest.h"
#include "DifyChatComponent.generated.h"


//额外的输入
USTRUCT(BlueprintType)
struct FDifyChatInputs
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DifyInput")
    FString Key;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DifyInput")
    FString Value;
};



//Dify返回的数据结构
USTRUCT(BlueprintType)
struct FDifyChatResponse
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DifyResponse")
	FString event;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DifyResponse")
	FString task_id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DifyResponse")
	FString id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DifyResponse")
	FString message_id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DifyResponse")
	FString conversation_id;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DifyResponse")
	FString mode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DifyResponse")
	FString answer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DifyResponse")
	FString created_at;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DifyResponse")
	FString ChatName;
};

//委托,在[dify返回后]
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDifyChatRespondedDelegate, FDifyChatResponse, Response);

//委托,在[dify返回时]
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDifyChatRespondingDelegate, FDifyChatResponse, Response);

//委托,在[向dify发送数据后]
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FDifyChatTalkToDelegate, FString, UserName , FString, ChatName ,FString, Message);


//单轮对话 or 多轮对话
UENUM(BlueprintType)
enum class EDifyChatType : uint8
{
	SingleChat UMETA(DisplayName = "Single Chat"),
	MultiChat UMETA(DisplayName = "Multi Chat")
};


//流式 or 阻塞
UENUM(BlueprintType)
enum class EDifyChatResponseMode : uint8
{
	Streaming  UMETA(DisplayName = "Streaming"),
	Blocking  UMETA(DisplayName = "Blocking")
};


UCLASS( ClassGroup=(Agent), meta=(BlueprintSpawnableComponent) )
class DIFYAPI_API UDifyChatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UDifyChatComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void BeginDestroy() override;
	
	
	//像Dify发送Post请求
	UFUNCTION(BlueprintCallable, Category = "DifyChat")
	void SentDifyPostRequest(FString _Message);

	//收到Dify响应时的回调
	void OnDifyResponding(const FHttpRequestPtr& _Request);
	
	//收到Dify响应后的回调
	void OnDifyResponded();
	
	// 解析Dify返回的数据
	UFUNCTION(BlueprintCallable, Category = "DifyChat")
	void ParseDifyResponse(FString _Response);

public:
	/////////////////////设置ChatAI的基本属性设置函数/////////////////////

	//名字
	UFUNCTION(BlueprintCallable, Category = "DifyChat")
	void SetChatName(FString _ChatName) { ChatName = _ChatName; }

	//对话类型
	UFUNCTION(BlueprintCallable, Category = "DifyChat")
	void SetDifyChatType(EDifyChatType _DifyChatType) { DifyChatType = _DifyChatType; }

	//玩家名字
	UFUNCTION(BlueprintCallable, Category = "DifyChat")
	void SetUserName(FString _UserName) { UserName = _UserName; }
	
	//初始化ChatAI,刚创建时就用这个
	UFUNCTION(BlueprintCallable, Category = "DifyChat" )
	void InitDifyChat(FString _DifyURL, FString _DifyAPIKey, FString _ChatName, FString _UserName,
						EDifyChatType _DifyChatType,
						EDifyChatResponseMode _DifyChatResponseMode,
						TArray<FDifyChatInputs> _DifyInputs);
	

	/////////////////////ChatAI的功能/////////////////////
	UFUNCTION(BlueprintCallable, Category = "DifyChat")
	void TalkToAI(FString _Message);
	

protected:
	///////////////////// 基本属性 /////////////////////

	//Dify的URL
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DifyChat")
	FString DifyURL;

	//Dify的API密钥
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DifyChat")
	FString DifyAPIKey;

	//额外的输入
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DifyChat")
	TArray<FDifyChatInputs> DifyInputs = {};
	
	//对话类型，单次 or 多轮
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DifyChat")
	EDifyChatType DifyChatType; 

	//回应类型，Streaming or Blocking
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DifyChat", meta=(AllowPrivateAccess="true"))
	EDifyChatResponseMode DifyChatResponseMode;

	//对话名字
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DifyChat")
	FString ChatName;

	//玩家名字
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DifyChat")
	FString UserName;

	//上一轮（本轮）完整的回应内容
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DifyChat")
	FDifyChatResponse LastCompletedResponse;

	
	
	//===================== 委托 ========================//

	
	//------------------------------------------
	// 委托：Dify回应时触发
	//
	// 参数：
	//	- Response：此次响应新增的内容,若是blocking模式则为完整内容
	//------------------------------------------
	UPROPERTY(BlueprintAssignable, Category = "DifyChat")
	FDifyChatRespondingDelegate OnDifyChatResponding;

	//------------------------------------------
	// 委托：Dify回应结束后触发
	//
	// 参数：
	//	- Response：此次响应的完整内容
	//------------------------------------------
	UPROPERTY(BlueprintAssignable, Category = "DifyChat")
	FDifyChatRespondedDelegate OnDifyChatResponded;
	
	
	//-----------------------------------------------------
	// 委托：向 Dify 发送数据后触发
	//
	// 参数：
	//	- UserName：与Dify对话的用户名称
	//	- ChatName：Dify的名字
	//	- Message：要发送的消息
	//-----------------------------------------------------
	UPROPERTY(BlueprintAssignable, Category = "DifyChat")
	FDifyChatTalkToDelegate OnDifyChatTalkTo;

	///////////////////// 参数 /////////////////////

	//是否正在等待Dify返回
	UPROPERTY(BlueprintReadOnly, Category = "DifyChat")
	bool bIsWaitingDifyResponse = false;
	
	//对话ID（只在多轮对话中有用）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DifyChat")
	FString ConversationID;

	/*
	 在streaming模式下，当前返回的和之前返回的内容都在一起。但是并不是每一次都只多返回一个，
	 所以需要一个额外的索引记录上一次返回到哪里了。
	 */
	int LastDataBlocksIndex = 0;

	//当前的Http请求
	TSharedPtr<IHttpRequest> CurrentHttpRequest;
	
};

