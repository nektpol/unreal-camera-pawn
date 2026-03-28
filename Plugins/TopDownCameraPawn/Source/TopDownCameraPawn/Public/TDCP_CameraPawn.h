// Developed by Neko Creative Technologies

#pragma once

#include "CoreMinimal.h"
#include "TDCP_Interface.h"
#include "GameFramework/Pawn.h"
#include "TDCP_CameraPawn.generated.h"

struct FInputActionValue;
class UInputMappingContext;
class UInputAction;
class UCameraComponent;
class USpringArmComponent;

UENUM(BlueprintType)
enum class EZoomType : uint8
{
	FOV         UMETA(DisplayName = "FOV"),
	ArmLength   UMETA(DisplayName = "Arm Length")
};

UCLASS()
class TOPDOWNCAMERAPAWN_API ATDCP_CameraPawn : public APawn, public ITDCP_Interface
{
	GENERATED_BODY()

public:
	ATDCP_CameraPawn();
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void SetCameraMovementEnabled_Implementation(bool bEnable) override;
	virtual void SetEdgeScrollEnabled_Implementation(bool bEnable) override;
	virtual void SetRotationEnabled_Implementation(bool bEnable) override;
	virtual void SetZoomEnabled_Implementation(bool bEnable) override;
	virtual void ResetCameraRotation_Implementation() override;
	virtual void ResetCameraZoom_Implementation() override;
	virtual void SnapCameraToLocation_Implementation(FVector NewLocation) override;
	virtual void InterpCameraToLocation_Implementation(FVector NewLocation, float Duration) override;
	virtual void SetCameraMoveSpeed_Implementation(float Speed) override;
	virtual void SetRotationSpeed_Implementation(float Speed) override;
	virtual void SetZoomSpeed_Implementation(float Speed) override;
	virtual void SetMinMaxZoom_Implementation(float InMinZoom, float InMaxZoom) override;
	virtual void SetTiltZoomEnabled_Implementation(bool bEnable, float InTiltAmount) override;
	virtual void SetEdgeScrollSpeed_Implementation(float Speed) override;
	virtual void SetEdgeScrollThreshold_Implementation(float Pixels) override;
	virtual void SetCameraRelativeMovement_Implementation(bool bEnable) override;
	virtual void SetFollowActor_Implementation(AActor* FollowActor) override;
	virtual void StopFollowingActor_Implementation() override;

protected:
	virtual void BeginPlay() override;


private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> Root;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> Camera;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TDCP|Settings", meta = (AllowPrivateAccess = "true"))
	float DefaultArmLength = 1200.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TDCP|Settings", meta = (AllowPrivateAccess = "true"))
	float DefaultPitch = -60.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TDCP|Settings|Movement", meta = (AllowPrivateAccess = "true"))
	float MoveSpeed = 1500.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TDCP|Settings|Movement", meta = (AllowPrivateAccess = "true"))
	float Acceleration = 5.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TDCP|Settings|Movement", meta = (AllowPrivateAccess = "true"))
	float Deceleration = 8.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TDCP|Settings|Movement", meta = (AllowPrivateAccess = "true"))
	bool bCameraRelativeMovement = true;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TDCP|Settings|Rotation", meta = (AllowPrivateAccess = "true"))
	bool bEnableRotation = true;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TDCP|Settings|Rotation", meta = (AllowPrivateAccess = "true"))
	float RotationSpeed = 120.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TDCP|Settings|Rotation", meta = (AllowPrivateAccess = "true"))
	float RotationInterpSpeed = 6.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TDCP|Settings|Zoom", meta = (AllowPrivateAccess = "true"))
	EZoomType ZoomType = EZoomType::ArmLength;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TDCP|Settings|Zoom", meta = (AllowPrivateAccess = "true"))
	float ZoomStep = 100.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TDCP|Settings|Zoom", meta = (AllowPrivateAccess = "true"))
	float MinZoom = 400.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TDCP|Settings|Zoom", meta = (AllowPrivateAccess = "true"))
	float MaxZoom = 1600.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TDCP|Settings|Zoom", meta = (AllowPrivateAccess = "true"))
	float ZoomInterpSpeed = 8.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TDCP|Settings|Zoom", meta = (AllowPrivateAccess = "true"))
	bool bTiltWithZoom = false;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TDCP|Settings|Zoom", meta = (AllowPrivateAccess = "true"))
	float TiltAmount = 15.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TDCP|Settings|EdgeScroll", meta = (AllowPrivateAccess = "true"))
	bool bEnableEdgeScroll = true;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TDCP|Settings|EdgeScroll", meta = (AllowPrivateAccess = "true"))
	float EdgeScrollThreshold = 20.f; // pixels from edge of screen
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TDCP|Settings|EdgeScroll", meta = (AllowPrivateAccess = "true"))
	float EdgeScrollSpeed = 600.f; // units per second
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TDCP|Settings|EdgeScroll", meta = (AllowPrivateAccess = "true"))
	bool bEdgeScrollCameraRelative = true; // moves relative to pawn rotation or world axes
	bool bMovementEnabled = true;
	bool bZoomEnabled = true;
	UPROPERTY(Transient)
	TObjectPtr<AActor> FollowedActor = nullptr;
	bool bMovementEnabledBeforeFollow = true;
	
	FVector CurrentVelocity = FVector::ZeroVector;
	FVector MoveInputVelocity = FVector::ZeroVector;
	FVector TargetVelocity = FVector::ZeroVector;
	bool bInterpToLocationActive = false;
	float InterpToLocationDuration = 0.f;
	float InterpToLocationElapsed = 0.f;
	FVector InterpToLocationStart = FVector::ZeroVector;
	FVector InterpToLocationTarget = FVector::ZeroVector;
	
	float TargetYaw = 0.f;
	float CurrentYaw = 0.f;
	float DefaultYaw = 0.f;
	
	float TargetZoom = 0.f;
	float CurrentZoom = 0.f;
	float DefaultFOV = 90.f;
	FRotator DefaultBoomRotation = FRotator::ZeroRotator;
	UPROPERTY(Transient)
	APlayerController* PlayerController = nullptr;
	
	void UpdateMovement(const float& DeltaTime);
	void UpdateRotation(const float& DeltaTime);
	void UpdateZoom(const float& DeltaTime);
	void CalculateEdgeScroll(FVector2D& OutEdgeInput);
	
	
#pragma region Input

protected:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TDCP|Input")
	UInputMappingContext* CameraMovementMappingContext;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TDCP|Input")
	UInputAction* MoveAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TDCP|Input")
	UInputAction* RotateAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TDCP|Input")
	UInputAction* ResetRotateAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="TDCP|Input")
	UInputAction* ZoomAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="TDCP|Input")
	UInputAction* ResetZoomAction;
	
private:
	void HandleMoveInput(const FInputActionValue& Value);
	void HandleRotateInput(const FInputActionValue& Value);
	void HandleZoomInput(const FInputActionValue& Value);
	void ResetRotation();
	void ResetZoom();

#pragma endregion 
};
