// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "DifyResponseParser.generated.h"

struct FDifyChatResponse;

UCLASS()
class UDifyResponseParser : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()
	
	/////////////////////////// String ///////////////////////////
	UFUNCTION(BlueprintPure, Category = "DifyAPI|Json|响应解析", meta = (DisplayName = "作为String-解析Json"))
	static FString ParseJsonStringValue(const FString& _OriginJson, FString _Key);

	UFUNCTION(BlueprintPure, Category = "DifyAPI|Json|响应解析", meta = (DisplayName = "作为String数组-解析Json"))
	static TArray<FString> ParseJsonStringValues(const FString& _OriginJson, FString _Key);
	
	UFUNCTION(BlueprintPure, Category = "DifyAPI|Json|响应解析", meta = (DisplayName = "作为String-解析Dify响应的Json"))
	static FString ParseJsonStringValueByStruct(const FDifyChatResponse& _OriginResponse, FString _Key);

	UFUNCTION(BlueprintPure, Category = "DifyAPI|Json|响应解析", meta = (DisplayName = "作为String数组-解析Dify响应的Json"))
	static TArray<FString> ParseJsonStringValuesByStruct(const FDifyChatResponse& _OriginResponse, FString _Key);
	
	/////////////////////////// float ///////////////////////////
	UFUNCTION(BlueprintPure, Category = "DifyAPI|Json|响应解析", meta = (DisplayName = "作为Float-解析Json"))
	static double ParseJsonFloatValue(const FString& _OriginJson, FString _Key);

	UFUNCTION(BlueprintPure, Category = "DifyAPI|Json|响应解析", meta = (DisplayName = "作为Float数组-解析Json"))
	static TArray<double> ParseJsonFloatValues(const FString& _OriginJson, FString _Key);
	
	UFUNCTION(BlueprintPure, Category = "DifyAPI|Json|响应解析", meta = (DisplayName = "作为Float-解析Dify响应的Json"))
	static double ParseJsonFloatValueByStruct(const FDifyChatResponse& _OriginResponse, FString _Key);

	UFUNCTION(BlueprintPure, Category = "DifyAPI|Json|响应解析", meta = (DisplayName = "作为Float数组-解析Dify响应的Json"))
	static TArray<double> ParseJsonFloatValuesByStruct(const FDifyChatResponse& _OriginResponse, FString _Key);
	
	

};

