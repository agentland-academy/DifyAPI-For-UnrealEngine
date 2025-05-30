// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "DifySceneCapture.generated.h"

/**
 * 
 */
UCLASS()
class UDifySceneCapture : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()
private:
	static UTextureRenderTarget2D* CreatDefaultRenderTarget2D(float _Width, float _Height);
public:
	UFUNCTION(BlueprintPure, Category = "DifyAPI|场景捕获", meta = (DisplayName = "获取指定变换的场景捕捉2D纹理"))
	static UTextureRenderTarget2D* GetSceneTexture2D(
		FVector _Location,
		FRotator _Rotation,
		float _FOV = 90.f,
		float _Width = 512,
		float _Height = 512
		);

	UFUNCTION(BlueprintPure, Category = "DifyAPI|场景捕获", meta = (DisplayName = "获取Actor的场景捕捉2D纹理"))
	static UTextureRenderTarget2D* GetSceneTexture2DByActor(
		AActor* _Actor,
		float _FOV = 90.f,
		float _Width = 512,
		float _Height = 512
		);

	UFUNCTION(BlueprintPure, Category = "DifyAPI|场景捕获", meta = (DisplayName = "获取Pawn的场景捕捉2D纹理"))
	static UTextureRenderTarget2D* GetSceneTexture2DByPawn(
		APawn* _Pawn,
		float _FOV = 90.f,
		float _Width = 512,
		float _Height = 512
		);

	
};
