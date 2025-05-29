// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DifyJsonMaker.generated.h"



USTRUCT(BlueprintType)
struct FDifyJsonObject
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString JsonString;
	

	FDifyJsonObject() : JsonString(TEXT(""))	{}

	FDifyJsonObject(FString _JsonString) : JsonString(_JsonString) {}
};


UCLASS()
class DIFYAPI_API UDifyJsonMaker : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()


	/////////////////////////// 合成/Maker ///////////////////////////
	UFUNCTION(BlueprintPure, Category = "DifyAPI|Json|合成", meta = (DisplayName = "合成Json-String键值对"))
	static FDifyJsonObject JsonStringMaker(FString _Key,FString _Value);

	UFUNCTION(BlueprintPure, Category = "DifyAPI|Json|合成", meta = (DisplayName = "合成Json-String键值对（值为数组）"))
	static FDifyJsonObject JsonStringArrayMaker(FString _Key,TArray<FString> _Values);

	UFUNCTION(BlueprintPure, Category = "DifyAPI|Json|合成", meta = (DisplayName = "合成Json-Float键值对"))
	static FDifyJsonObject JsonFloatMaker(FString _Key,double _Value);

	UFUNCTION(BlueprintPure, Category = "DifyAPI|Json|合成", meta = (DisplayName = "合成Json-Float键值对（值为数组）"))
	static FDifyJsonObject JsonFloatArrayMaker(FString _Key,TArray<double> _Values);

	UFUNCTION(BlueprintPure, Category = "DifyAPI|Json|合成", meta = (DisplayName = "合成Json-Object键值对"))
	static FDifyJsonObject JsonObjectMaker(FString _Key,FDifyJsonObject _Value);
	
	UFUNCTION(BlueprintPure, Category = "DifyAPI|Json|合成", meta = (DisplayName = "合成Json-Object键值对（值为数组）"))
	static FDifyJsonObject JsonObjectArrayMaker(FString _Key,TArray<FDifyJsonObject> _Values);
	
	/////////////////////////// 追加/Append ///////////////////////////
	UFUNCTION(BlueprintPure, Category = "DifyAPI|Json|追加", meta = (DisplayName = "追加Json"))
	static FDifyJsonObject AppendJson(FDifyJsonObject _A,FDifyJsonObject _B);

	UFUNCTION(BlueprintPure, Category = "DifyAPI|Json|追加", meta = (DisplayName = "追加多个Json"))
	static FDifyJsonObject AppendJsons(FDifyJsonObject _A,TArray<FDifyJsonObject> _B);

	
	
};
