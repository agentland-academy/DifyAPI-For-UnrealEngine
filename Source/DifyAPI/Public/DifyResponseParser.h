// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "DifyResponseParser.generated.h"

struct FDifyChatResponse;

UCLASS()
class UDifyResponseParser : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	
	
	UFUNCTION(BlueprintPure, Category = "DifyAPI | 响应解析", meta = (DisplayName = "解析Dify响应的Json字符串值"))
	static FString ParseJsonStringValue(const FString& _OriginJson, FString _Key);

	UFUNCTION(BlueprintPure, Category = "DifyAPI | 响应解析", meta = (DisplayName = "解析Dify响应的Json字符串值数组"))
	static TArray<FString> ParseJsonStringValues(const FString& _OriginJson, FString _Key);
	
	UFUNCTION(BlueprintPure, Category = "DifyAPI | 响应解析", meta = (DisplayName = "解析Dify响应的Json字符串值"))
	static FString ParseJsonStringValueByStruct(const FDifyChatResponse& _OriginResponse, FString _Key);

	UFUNCTION(BlueprintPure, Category = "DifyAPI | 响应解析", meta = (DisplayName = "解析Dify响应的Json字符串值数组"))
	static TArray<FString> ParseJsonStringValuesByStruct(const FDifyChatResponse& _OriginResponse, FString _Key);
	
	
	UFUNCTION(BlueprintPure, Category = "DifyAPI | 响应解析", meta = (DisplayName = "解析Dify响应的Json浮点值"))
	static double ParseJsonFloatValue(const FString& _OriginJson, FString _Key);
	
	UFUNCTION(BlueprintPure, Category = "DifyAPI | 响应解析", meta = (DisplayName = "解析Dify响应的Json浮点值"))
	static double ParseJsonFloatValueByStruct(const FDifyChatResponse& _OriginResponse, FString _Key);

};

