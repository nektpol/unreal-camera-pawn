// Developed by Neko Creative Technologies

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "TDCP_CameraPawn.generated.h"

class UCameraComponent;
class USpringArmComponent;

UCLASS()
class TOPDOWNCAMERAPAWN_API ATDCP_CameraPawn : public APawn
{
	GENERATED_BODY()

public:
	ATDCP_CameraPawn();
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;


private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> Camera;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TDCP|Settings", meta = (AllowPrivateAccess = "true"))
	float DefaultArmLength = 1200.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TDCP|Settings", meta = (AllowPrivateAccess = "true"))
	float DefaultPitch = -60.f;
};
