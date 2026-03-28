// Developed by Neko Creative Technologies

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TDCP_Interface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UTDCP_Interface : public UInterface
{
	GENERATED_BODY()
};


class TOPDOWNCAMERAPAWN_API ITDCP_Interface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="TDCP Interface")
	void SetCameraMovementEnabled(bool bEnable);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="TDCP Interface")
	void SetEdgeScrollEnabled(bool bEnable);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="TDCP Interface")
	void SetRotationEnabled(bool bEnable);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="TDCP Interface")
	void SetZoomEnabled(bool bEnable);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="TDCP Interface")
	void ResetCameraRotation();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="TDCP Interface")
	void ResetCameraZoom();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="TDCP Interface")
	void SnapCameraToLocation(FVector NewLocation);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="TDCP Interface")
	void InterpCameraToLocation(FVector NewLocation, float Duration);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="TDCP Interface")
	void SetCameraMoveSpeed(float Speed);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="TDCP Interface")
	void SetRotationSpeed(float Speed);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="TDCP Interface")
	void SetZoomSpeed(float Speed);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="TDCP Interface")
	void SetMinMaxZoom(float MinZoom, float MaxZoom);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="TDCP Interface")
	void SetTiltZoomEnabled(bool bEnable, float TiltAmount);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="TDCP Interface")
	void SetEdgeScrollSpeed(float Speed);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="TDCP Interface")
	void SetEdgeScrollThreshold(float Pixels);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="TDCP Interface")
	void SetCameraRelativeMovement(bool bEnable);
};
