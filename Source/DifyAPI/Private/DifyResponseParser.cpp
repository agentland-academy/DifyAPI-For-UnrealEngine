// Copyright Epic Games, Inc. All Rights Reserved.

#include "DifyResponseParser.h"
#include "DifyAPI.h"
#include "DifyChatComponent.h"

UDifyResponseParser::UDifyResponseParser(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{}

/////////////////////////// String ///////////////////////////
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

TArray<FString> UDifyResponseParser::ParseJsonStringValues(const FString& _OriginJson, FString _Key)
{
	TArray<FString> values;
	
	TSharedPtr<FJsonObject> jsonObject;
	TSharedRef<TJsonReader<>> reader = TJsonReaderFactory<>::Create(_OriginJson);

	bool bIsValidJson = FJsonSerializer::Deserialize(reader, jsonObject);
	if (!bIsValidJson)
	{
		return values;
	}

	if(!jsonObject.IsValid())
	{
		return values;
	}
	
	FStringView keyView = FStringView(*_Key);
	TArray<TSharedPtr<FJsonValue>> valueArray = jsonObject->GetArrayField(keyView);

	// 遍历数组中的每个值
	for(TSharedPtr<FJsonValue> valueJson : valueArray)
	{
		// 将JSON转换为字符串
		FString valueJsonString;

		// 如果是[｛"key":"value"｝]这种
		if(valueJson->Type == EJson::Object)
		{
			TSharedPtr<FJsonObject> valueJsonObject = valueJson->AsObject();
			TSharedRef<TJsonWriter<>> writer = TJsonWriterFactory<>::Create(&valueJsonString);
			bool bCoudSerialize =  
				FJsonSerializer::Serialize(valueJsonObject.ToSharedRef(), writer);
			if(!bCoudSerialize)
			{
				break;
			}
		}
		// 如果是["value1","value2"]这种
		else if(valueJson->Type == EJson::String)
		{
			valueJsonString = valueJson->AsString();
		}
		
		values.Add(valueJsonString);
	}

	return values;
}
 

FString UDifyResponseParser::ParseJsonStringValueByStruct(const FDifyChatResponse& _OriginResponse, FString _Key)
{
	FString answer = _OriginResponse.answer;
	return ParseJsonStringValue(answer, _Key);
}

TArray<FString> UDifyResponseParser::ParseJsonStringValuesByStruct(const FDifyChatResponse& _OriginResponse,FString _Key)
{
	FString answer = _OriginResponse.answer;
	return ParseJsonStringValues(answer, _Key);
}

/////////////////////////// Float ///////////////////////////
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

TArray<double> UDifyResponseParser::ParseJsonFloatValues(const FString& _OriginJson, FString _Key)
{
	TArray<double> floatValues;
	TArray<FString> stringValues = 
		ParseJsonStringValues(_OriginJson, _Key);

	for(const FString& stringValue : stringValues)
	{
		double floatValue = FCString::Atof(*stringValue);
		if (FMath::IsNaN(floatValue))
		{
			floatValue = 0.0;
		}
		floatValues.Add(floatValue);
	}
	
	return floatValues;
}


double UDifyResponseParser::ParseJsonFloatValueByStruct(const FDifyChatResponse& _OriginResponse, FString _Key)
{
	FString answer = _OriginResponse.answer;
	return ParseJsonFloatValue(answer, _Key);
}

TArray<double> UDifyResponseParser::ParseJsonFloatValuesByStruct(const FDifyChatResponse& _OriginResponse, FString _Key)
{
	FString answer = _OriginResponse.answer;
	return ParseJsonFloatValues(answer, _Key);
}
