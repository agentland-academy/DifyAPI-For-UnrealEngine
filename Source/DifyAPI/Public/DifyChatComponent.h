// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DifyChatComponent.generated.h"

//Dify返回的数据结构
USTRUCT(BlueprintType)
struct FDifyChatResponse
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DifyChat")
	FString event;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DifyChat")
	FString task_id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DifyChat")
	FString id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DifyChat")
	FString message_id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DifyChat")
	FString conversation_id;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DifyChat")
	FString mode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DifyChat")
	FString answer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DifyChat")
	FString created_at;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DifyChat")
	FString ChatName;
};

//委托,在dify返回数据后能够让蓝图使用
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDifyChatResponseDelegate, FDifyChatResponse, Response);

//委托,在向dify发送数据后能够让蓝图使用
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDifyChatTalkToDelegate, FString, UserName , FString, Message);


UENUM(BlueprintType)
enum class EDifyChatType : uint8
{
	SingleChat UMETA(DisplayName = "Single Chat"),
	MultiChat UMETA(DisplayName = "Multi Chat")
};




UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DIFYAPI_API UDifyChatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UDifyChatComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	
	//像Dify发送Post请求
	UFUNCTION(BlueprintCallable, Category = "DifyChat")
	void SentDifyPostRequest(FString _Message);

	// 解析Dify返回的数据
	UFUNCTION(BlueprintCallable, Category = "DifyChat")
	void ParseDifyResponse(FString _Response);

public:
	/////////////////////设置ChatAI的基本属性/////////////////////

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
	UFUNCTION(BlueprintCallable, Category = "DifyChat")
	void InitDifyChat(FString _DifyURL, FString _DifyAPIKey, FString _ChatName, FString _UserName, EDifyChatType _DifyChatType);
	

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
	
	//对话类型
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DifyChat")
	EDifyChatType DifyChatType;

	//对话名字
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DifyChat")
	FString ChatName;

	//玩家名字
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DifyChat")
	FString UserName;
	
	///////////////////// 委托 /////////////////////

	//Dify回应
	UPROPERTY(BlueprintAssignable, Category = "DifyChat")
	FDifyChatResponseDelegate OnDifyChatResponse;
	//向Dify对话
	UPROPERTY(BlueprintAssignable, Category = "DifyChat")
	FDifyChatTalkToDelegate OnDifyChatTalkTo;

	///////////////////// 参数 /////////////////////

	//是否正在等待Dify返回
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DifyChat")
	bool bIsWaitingDifyResponse = false;
	
	//对话ID（只在多轮对话中有用）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DifyChat")
	FString ConversationID;
	
};

