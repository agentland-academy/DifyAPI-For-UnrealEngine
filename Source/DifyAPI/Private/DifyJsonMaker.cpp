// Fill out your copyright notice in the Description page of Project Settings.


#include "DifyJsonMaker.h"

FDifyJsonObject UDifyJsonMaker::JsonStringMaker(FString _Key,FString _Value)
{
	FDifyJsonObject difyJsonObject;
	difyJsonObject.JsonString = FString::Printf(TEXT("{\n    \"%s\":\"%s\"\n}"), *_Key, *_Value);
	return difyJsonObject;
}

FDifyJsonObject UDifyJsonMaker::JsonStringArrayMaker(FString _Key, TArray<FString> _Values)
{
	FString values;
	for(const FString& value : _Values)
	{
		values += FString::Printf(TEXT("\"%s\","), *value);
	}
	FDifyJsonObject difyJsonObject;
	// 加入后去掉最后一个逗号
	difyJsonObject.JsonString = FString::Printf(TEXT("{\n    \"%s\":[%s]\n}"), *_Key, *values.LeftChop(1));
	return difyJsonObject;
}

FDifyJsonObject UDifyJsonMaker::JsonFloatMaker(FString _Key, double _Value)
{
	FString valueString = FString::Printf(TEXT("%f"), _Value);
	return JsonStringMaker(_Key, valueString);
}

FDifyJsonObject UDifyJsonMaker::JsonFloatArrayMaker(FString _Key, TArray<double> _Values)
{
	TArray<FString> valueStrings;
	for(double value : _Values)
	{
		valueStrings.Add(FString::Printf(TEXT("%f"), value));
	}
	return JsonStringArrayMaker(_Key, valueStrings);
}

FDifyJsonObject UDifyJsonMaker::JsonObjectMaker(FString _Key, FDifyJsonObject _Value)
{
	TSharedPtr<FJsonObject> jsonObject = MakeShareable(new FJsonObject);

	TSharedPtr<FJsonObject> jsonObjectA;
	TSharedRef<TJsonReader<>> readerA = TJsonReaderFactory<>::Create(_Value.JsonString);
	bool bIsValidJsonA = FJsonSerializer::Deserialize(readerA, jsonObjectA);

	if(!bIsValidJsonA)
	{
		UE_LOG(LogTemp, Error, TEXT("[JsonObjectMaker] Failed to parse JSON: %s"), *_Value.JsonString);
		return FDifyJsonObject();
	}

	jsonObject->SetObjectField(_Key, jsonObjectA.ToSharedRef());

	FString outputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&outputString);
	FJsonSerializer::Serialize(jsonObject.ToSharedRef(), Writer);

	return FDifyJsonObject(outputString);
	
}

FDifyJsonObject UDifyJsonMaker::JsonObjectArrayMaker(FString _Key, TArray<FDifyJsonObject> _Values)
{
	
	TSharedPtr<FJsonObject> jsonObject = MakeShareable(new FJsonObject);
	TArray<TSharedPtr<FJsonValue>> jsonValues;
	
	for(auto & value : _Values)
	{
		if(value.JsonString.IsEmpty())
			continue;

		TSharedPtr<FJsonObject> jsonObjectA;

		// 解析每个JsonObject
		TSharedRef<TJsonReader<>> readerA = TJsonReaderFactory<>::Create(value.JsonString);
		bool bIsValidJsonA = FJsonSerializer::Deserialize(readerA, jsonObjectA);

		if(!bIsValidJsonA)
		{
			UE_LOG(LogTemp, Error, TEXT("[JsonObjectArrayMaker] Failed to parse JSON: %s"), *value.JsonString);
			continue;
		}
		
		// 遍历所有键值对并添加到jsonObject中
		for (const auto& pair :  jsonObjectA->Values)
		{
			if(!pair.Value.IsValid())
				continue;

			jsonValues.Add(MakeShared<FJsonValueObject>(pair.Value->AsObject()));
		}
	}
	
	jsonObject->SetArrayField(_Key, jsonValues);

	

	FString outputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&outputString);
	FJsonSerializer::Serialize(jsonObject.ToSharedRef(), Writer);

	return FDifyJsonObject(outputString);
}

FDifyJsonObject UDifyJsonMaker::AppendJson(FDifyJsonObject _A, FDifyJsonObject _B)
{
	FString jsonA = _A.JsonString;
	FString jsonB = _B.JsonString;

	
	TSharedPtr<FJsonObject> jsonObjectA;
	TSharedRef<TJsonReader<>> readerA = TJsonReaderFactory<>::Create(jsonA);
	bool bIsValidJsonA = FJsonSerializer::Deserialize(readerA, jsonObjectA);

	if(!bIsValidJsonA)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to parse JSON A: %s"), *jsonA);
		return FDifyJsonObject();
	}
	
	TSharedPtr<FJsonObject> jsonObjectB;
	TSharedRef<TJsonReader<>> readerB = TJsonReaderFactory<>::Create(jsonB);
	bool bIsValidJsonB = FJsonSerializer::Deserialize(readerB, jsonObjectB);
	if(!bIsValidJsonB)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to parse JSON B: %s"), *jsonB);
		return FDifyJsonObject();
	}

	// 合并jsonObjectB到jsonObjectA
	for (const auto& Pair : jsonObjectB->Values)
	{
		jsonObjectA->SetField(Pair.Key, Pair.Value);
	}


	
	
	//  jsonObjectA to FString
	FString jsonStringA;
	
	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(jsonObjectA.ToSharedRef(), Writer);
	
	UE_LOG(LogTemp, Error, TEXT("[JSON A]\n%s"), *OutputString);

	
	return FDifyJsonObject(OutputString);
}

FDifyJsonObject UDifyJsonMaker::AppendJsons(FDifyJsonObject _A, TArray<FDifyJsonObject> _B)
{
	if(_B.IsEmpty())
		return _A;

	for(auto& jsonObjectB : _B)
	{
		_A = AppendJson(_A, jsonObjectB);
	}

	return _A;
}
