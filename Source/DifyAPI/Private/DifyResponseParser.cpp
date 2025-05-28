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
	
	bool bCouldGet = jsonObject->TryGetStringField(_Key, value);
	if (!bCouldGet)
	{
		return FString();
	}
	
	return value;
}

FString UDifyResponseParser::ParseJsonStringValueByStruct(const FDifyChatResponse& _OriginResponse, FString _Key)
{
	FString answer = _OriginResponse.answer;
	return ParseJsonStringValue(answer, _Key);
}

double UDifyResponseParser::ParseJsonFloatValue(const FString& _OriginJson, FString _Key)
{
	FString value = ParseJsonStringValue(_OriginJson, _Key);
	double floatValue = FCString::Atof(*value);
	if (FMath::IsNaN(floatValue))
	{
		return 0.0;
	}
	return floatValue;
}

double UDifyResponseParser::ParseJsonFloatValueByStruct(const FDifyChatResponse& _OriginResponse, FString _Key)
{
	FString answer = _OriginResponse.answer;
	return ParseJsonFloatValue(answer, _Key);
}

