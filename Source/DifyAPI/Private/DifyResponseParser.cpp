// Copyright Epic Games, Inc. All Rights Reserved.

#include "DifyResponseParser.h"
#include "DifyAPI.h"
#include "DifyChatComponent.h"

UDifyResponseParser::UDifyResponseParser(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{}


FString UDifyResponseParser::ParseJsonStringValue(const FString& _OriginJson, FString _Key)
{
	// 解析json
	TSharedPtr<FJsonObject> jsonObject;
	TSharedRef<TJsonReader<>> reader = TJsonReaderFactory<>::Create(_OriginJson);

	bool bIsValidJson = FJsonSerializer::Deserialize(reader, jsonObject);
	if (!bIsValidJson)
	{
		return FString();
	}

	
	if(!jsonObject.IsValid())
	{
		return FString();
	}

	FString value;
	bool bCouldet = jsonObject->TryGetStringField(_Key, value);
	if (!bCouldet)
	{
		return FString();
	}
	return value;
}

FString UDifyResponseParser::ParseJsonStringValueByStruct(const FDifyChatResponse& _OriginResponse, FString _Key)
{
	return "114514";
}

double UDifyResponseParser::ParseJsonFloatValue(const FString& _OriginJson, FString _Key)
{
	return 114514;
}

double UDifyResponseParser::ParseJsonFloatValueByStruct(const FDifyChatResponse& _OriginResponse, FString _Key)
{
	return 114514;
}

