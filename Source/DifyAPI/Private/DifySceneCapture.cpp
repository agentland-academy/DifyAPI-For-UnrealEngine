// Fill out your copyright notice in the Description page of Project Settings.


#include "DifySceneCapture.h"

#include "Components/SceneCaptureComponent2D.h"
#include "Engine/SceneCapture2D.h"
#include "Engine/TextureRenderTarget2D.h"


UDifySceneCapture::UDifySceneCapture(const FObjectInitializer& _ObjectInitializer)
: Super(_ObjectInitializer)
{}


UTextureRenderTarget2D* UDifySceneCapture::CreatDefaultRenderTarget2D(float _Width, float _Height)
{
	UTextureRenderTarget2D* renderTarget = NewObject<UTextureRenderTarget2D>();
	check(renderTarget);
	renderTarget->RenderTargetFormat = RTF_RGBA8_SRGB;
	renderTarget->ClearColor = FLinearColor::Black;
	renderTarget->bAutoGenerateMips = false;
	renderTarget->bCanCreateUAV = false;
	renderTarget->TargetGamma = 2.2f; 
	renderTarget->InitAutoFormat(_Width, _Height);	
	renderTarget->UpdateResourceImmediate(true);

	return renderTarget;
}

UTextureRenderTarget2D* UDifySceneCapture::GetSceneTexture2D(FVector _Location, FRotator _Rotation, float _FOV,
                                                             float _Width, float _Height)
{
	////////////////////// 创建RenderTarget2D //////////////////////
	UTextureRenderTarget2D* renderTarget = CreatDefaultRenderTarget2D(_Width, _Height);

	/////////////////////// 创建SceneCapture2D //////////////////////
	ASceneCapture2D* sceneCapture = GEngine->GameViewport->GetWorld()->SpawnActor<ASceneCapture2D>(
		ASceneCapture2D::StaticClass(), _Location, FRotator(_Rotation));
	sceneCapture->GetCaptureComponent2D()->FOVAngle = _FOV;
	sceneCapture->GetCaptureComponent2D()->TextureTarget = renderTarget;
	sceneCapture->GetCaptureComponent2D()->CaptureSource = ESceneCaptureSource::SCS_FinalToneCurveHDR;
	sceneCapture->GetCaptureComponent2D()->bCaptureEveryFrame = false;
	sceneCapture->GetCaptureComponent2D()->bCaptureOnMovement = false;

	////////////////////// 捕获场景 //////////////////////
	sceneCapture->GetCaptureComponent2D()->CaptureScene();
	sceneCapture->Destroy();

	return renderTarget;
}

UTextureRenderTarget2D* UDifySceneCapture::GetSceneTexture2DByActor(AActor* _Actor, float _FOV, float _Width,
		float _Height)
{
	if(_Actor == nullptr || _Actor->GetWorld() == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[DifySceneCapture] Actor is null or World is null!"));
		return CreatDefaultRenderTarget2D(_Width, _Height);
	}
	
	return GetSceneTexture2D(
		_Actor->GetActorLocation(),
		_Actor->GetActorRotation(),
		_FOV,
		_Width,
		_Height
	);
}

UTextureRenderTarget2D* UDifySceneCapture::GetSceneTexture2DByPawn(APawn* _Pawn, float _FOV, float _Width,
		float _Height)
{
	if(_Pawn == nullptr || _Pawn->GetWorld() == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[DifySceneCapture] Pawn is null or World is null!"));
		return CreatDefaultRenderTarget2D(_Width, _Height);
	}

	FVector eyesLocation;
	FRotator eyesRotation;
	
	if(IsValid(_Pawn->GetController()))
	{
		_Pawn->GetController()->GetPlayerViewPoint(eyesLocation, eyesRotation);
	}
	else
	{
		_Pawn->GetActorEyesViewPoint(eyesLocation, eyesRotation);
	}
	
	return GetSceneTexture2D(
		eyesLocation,
		eyesRotation,
		_FOV,
		_Width,
		_Height
	);
}
