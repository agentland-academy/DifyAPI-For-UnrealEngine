// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interfaces/IHttpRequest.h"
#include "DifyChatComponent.generated.h"

//Dify���ص����ݽṹ
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

//ί��,��[dify���غ�]
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDifyChatRespondedDelegate);

//ί��,��[dify����ʱ]
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDifyChatRespondingDelegate, FDifyChatResponse, Response);

//ί��,��[��dify�������ݺ�]
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDifyChatTalkToDelegate, FString, UserName , FString, Message);


//���ֶԻ� or ���ֶԻ�
UENUM(BlueprintType)
enum class EDifyChatType : uint8
{
	SingleChat UMETA(DisplayName = "Single Chat"),
	MultiChat UMETA(DisplayName = "Multi Chat")
};


//��ʽ or ����
UENUM(BlueprintType)
enum class EDifyChatResponseMode : uint8
{
	Streaming  UMETA(DisplayName = "Streaming"),
	Blocking  UMETA(DisplayName = "Blocking")
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
	
	
	//��Dify����Post����
	UFUNCTION(BlueprintCallable, Category = "DifyChat")
	void SentDifyPostRequest(FString _Message);

	//�յ�Dify��Ӧʱ�Ļص�
	void OnDifyResponding(const FHttpRequestPtr& _Request);
	
	//�յ�Dify��Ӧ��Ļص�
	void OnDifyResponded();
	
	// ����Dify���ص�����
	UFUNCTION(BlueprintCallable, Category = "DifyChat")
	void ParseDifyResponse(FString _Response);

public:
	/////////////////////����ChatAI�Ļ�������/////////////////////

	//����
	UFUNCTION(BlueprintCallable, Category = "DifyChat")
	void SetChatName(FString _ChatName) { ChatName = _ChatName; }

	//�Ի�����
	UFUNCTION(BlueprintCallable, Category = "DifyChat")
	void SetDifyChatType(EDifyChatType _DifyChatType) { DifyChatType = _DifyChatType; }

	//�������
	UFUNCTION(BlueprintCallable, Category = "DifyChat")
	void SetUserName(FString _UserName) { UserName = _UserName; }

	//��ʼ��ChatAI,�մ���ʱ�������
	UFUNCTION(BlueprintCallable, Category = "DifyChat")
	void InitDifyChat(FString _DifyURL, FString _DifyAPIKey, FString _ChatName, FString _UserName,
						EDifyChatType _DifyChatType, EDifyChatResponseMode _DifyChatResponseMode);
	

	/////////////////////ChatAI�Ĺ���/////////////////////
	UFUNCTION(BlueprintCallable, Category = "DifyChat")
	void TalkToAI(FString _Message);
	

protected:
	///////////////////// �������� /////////////////////

	//Dify��URL
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DifyChat")
	FString DifyURL;

	//Dify��API��Կ
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DifyChat")
	FString DifyAPIKey;
	
	//�Ի����ͣ����� or ����
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DifyChat")
	EDifyChatType DifyChatType;

	//��Ӧ���ͣ�Streaming or Blocking
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DifyChat")
	EDifyChatResponseMode DifyChatResponseMode;

	//�Ի�����
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DifyChat")
	FString ChatName;

	//�������
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DifyChat")
	FString UserName;
	
	///////////////////// ί�� /////////////////////

	//Dify��Ӧʱ
	UPROPERTY(BlueprintAssignable, Category = "DifyChat")
	FDifyChatRespondingDelegate OnDifyChatResponding;

	//Dify��Ӧ��
	UPROPERTY(BlueprintAssignable, Category = "DifyChat")
	FDifyChatRespondedDelegate OnDifyChatResponded;
	
	
	//��Dify�Ի�
	UPROPERTY(BlueprintAssignable, Category = "DifyChat")
	FDifyChatTalkToDelegate OnDifyChatTalkTo;

	///////////////////// ���� /////////////////////

	//�Ƿ����ڵȴ�Dify����
	UPROPERTY(BlueprintReadOnly, Category = "DifyChat")
	bool bIsWaitingDifyResponse = false;
	
	//�Ի�ID��ֻ�ڶ��ֶԻ������ã�
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DifyChat")
	FString ConversationID;

	/*
	 ��streamingģʽ�£���ǰ���صĺ�֮ǰ���ص����ݶ���һ�𡣵��ǲ�����ÿһ�ζ�ֻ�෵��һ����
	 ������Ҫһ�������������¼��һ�η��ص������ˡ�
	 */
	int LastDataBlocksIndex = 0;
	
};

