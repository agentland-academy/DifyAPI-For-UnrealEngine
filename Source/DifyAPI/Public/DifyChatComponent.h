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

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Key;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Value;
};

//Dify返回的数据结构
USTRUCT(BlueprintType)
struct FDifyChatResponse
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString event;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString task_id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString message_id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString conversation_id;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString mode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString answer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString created_at;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ChatName;
};


//Dify返回的Image数据结构
USTRUCT(BlueprintType)
struct FDifyImageResponse
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Size;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Extension;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Mime_type;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Created_by;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Created_at;

	FDifyImageResponse() : ID(TEXT("")){}
	FDifyImageResponse(FString _ID) : ID(_ID) {}

	bool IsEmpty() const
    {
		// ID为空就表示没有图片
        return ID.IsEmpty();
    }
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

UCLASS(BlueprintType,ClassGroup=(Agent), meta=(BlueprintSpawnableComponent) )
class DIFYAPI_API UDifyChatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UDifyChatComponent();

protected://===================== Functions ========================//
	
	virtual void BeginPlay() override;

	virtual void BeginDestroy() override;
	
	// 向Dify服务器发一张图片
	void SentAnImageToDifyRequest(FString _Message, UTextureRenderTarget2D* _File);

	//收到DifyImage响应后的回调，然后继续发送TEXT信息
	void OnDifyImageResponded(FHttpResponsePtr _Response,FString _Message);

	// 解析DifyImage返回的数据
	bool ParseDifyImageResponse(FString _Response, FDifyImageResponse& _OutDifyImageResponse);
	
	//向Dify发送Post请求
	//UFUNCTION()
	void SentDifyPostRequest(FString _Message,FDifyImageResponse _ImageResponse);

	//收到Dify响应时的回调
	void OnDifyResponding(const FHttpRequestPtr& _Request);
	
	//收到Dify响应后的回调
	void OnDifyResponded();
	
	// 解析Dify返回的数据
	void ParseDifyResponse(FString _Response);

	//名字
	void SetChatName(FString _ChatName) { ChatName = _ChatName; }

	//对话类型
	void SetDifyChatType(EDifyChatType _DifyChatType) { DifyChatType = _DifyChatType; }

	//玩家名字
	void SetUserName(FString _UserName) { UserName = _UserName; }

public://===================== Functions ========================//
	
	//初始化ChatAI,刚创建时就用这个
	UFUNCTION(BlueprintCallable, Category = "DifyAPI|Chat" )
	void InitDifyChat(FString _DifyURL,FString _DifyAPIKey, FString _ChatName, FString _UserName,
						EDifyChatType _DifyChatType,
						EDifyChatResponseMode _DifyChatResponseMode,
						const TArray<FDifyChatInputs>& _DifyInputs);
	
	//与Dify通信
	UFUNCTION(BlueprintCallable, Category = "DifyAPI|Chat")
	void TalkToAI(FString _Message,UTextureRenderTarget2D* _File);

public://===================== 参数 ========================//
	
	//Dify的URL
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DifyAPI|参数",
		meta=(DisplayName="Dify Chat URL",ToolTip = "http://xxx/v1/chat-messages"))
	FString DifyURL;

	//Dify服务器的URL,显示名称为“upload”
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DifyAPI|参数",
		meta=(DisplayName="Dify File Upload URL",ToolTip = "http://xxx/v1/files/upload"))
	FString DifyFileUploadURL;

	//Dify的API密钥
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DifyAPI|参数")
	FString DifyAPIKey;

	//额外的输入
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DifyAPI|参数")
	TArray<FDifyChatInputs> DifyInputs = {};
	
	//对话类型，单次 or 多轮
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DifyAPI|参数")
	EDifyChatType DifyChatType; 

	//回应类型，Streaming or Blocking
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DifyAPI|参数")
	EDifyChatResponseMode DifyChatResponseMode;

	//对话名字
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DifyAPI|参数")
	FString ChatName;

	//玩家名字
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DifyAPI|参数")
	FString UserName;

	//上一轮（本轮）完整的回应内容
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DifyAPI|参数")
	FDifyChatResponse LastCompletedResponse;

	//是否正在等待Dify返回
	UPROPERTY(BlueprintReadOnly, Category = "DifyAPI|参数")
	bool bIsWaitingDifyResponse = false;
	
	//对话ID（只在多轮对话中有用）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DifyAPI|参数")
	FString ConversationID;

public://===================== 委托 ========================//

	//------------------------------------------
	// 委托：Dify回应时触发
	//
	// 参数：
	//	- Response：此次响应新增的内容,若是blocking模式则为完整内容
	//------------------------------------------
	UPROPERTY(BlueprintAssignable, Category = "DifyAPI|委托")
	FDifyChatRespondingDelegate OnDifyChatResponding;

	//------------------------------------------
	// 委托：Dify回应结束后触发
	//
	// 参数：
	//	- Response：此次响应的完整内容
	//------------------------------------------
	UPROPERTY(BlueprintAssignable, Category = "DifyAPI|委托")
	FDifyChatRespondedDelegate OnDifyChatResponded;
	
	
	//-----------------------------------------------------
	// 委托：向 Dify 发送数据后触发
	//
	// 参数：
	//	- UserName：与Dify对话的用户名称
	//	- ChatName：Dify的名字
	//	- Message：要发送的消息
	//-----------------------------------------------------
	UPROPERTY(BlueprintAssignable, Category = "DifyAPI|委托")
	FDifyChatTalkToDelegate OnDifyChatTalkTo;

protected://===================== 参数 ========================//
	 //在streaming模式下，当前返回的和之前返回的内容都在一起。但是并不是每一次都只多返回一个，
	 //所以需要一个额外的索引记录上一次返回到哪里。
	int LastDataBlocksIndex = 0;

	//当前的Http请求
	TSharedPtr<IHttpRequest,ESPMode::ThreadSafe> CurrentHttpRequest;
};

