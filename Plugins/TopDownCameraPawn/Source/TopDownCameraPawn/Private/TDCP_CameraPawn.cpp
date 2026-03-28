// Developed by Neko Creative Technologies


#include "TDCP_CameraPawn.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

ATDCP_CameraPawn::ATDCP_CameraPawn()
{
	PrimaryActorTick.bCanEverTick = true;
	
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	SetRootComponent(Root);

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(Root);
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->TargetArmLength = 1200.f;
	CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraBoom);
}

void ATDCP_CameraPawn::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	CameraBoom->TargetArmLength = DefaultArmLength;
	CameraBoom->SetRelativeRotation(FRotator(DefaultPitch, 0.f, 0.f));
}

void ATDCP_CameraPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	if (!CameraMovementMappingContext)
	{
		UE_LOG(LogTemp, Warning, TEXT("CameraMovementMappingContext is not set!"));
		return;
	}

	if (APlayerController* PC = Cast<APlayerController>(NewController))
	{
		PlayerController = PC;
		
		if (bEnableEdgeScroll)
		{
			PlayerController->SetShowMouseCursor(true);
			// Keep mouse inside the viewport
			FInputModeGameAndUI InputMode;
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
			InputMode.SetHideCursorDuringCapture(false); // keep it visible
			PlayerController->SetInputMode(InputMode);
		}
		
		if (ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
				LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				if (bMovementEnabled)
				{
					Subsystem->AddMappingContext(CameraMovementMappingContext, 0);
				}
			}
		}
	}
}

void ATDCP_CameraPawn::UnPossessed()
{
	Super::UnPossessed();

	PlayerController = nullptr;
	MoveInputVelocity = FVector::ZeroVector;
	TargetVelocity = FVector::ZeroVector;
	CurrentVelocity = FVector::ZeroVector;
	
	if (!CameraMovementMappingContext)
	{
		return;
	}

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
				LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				Subsystem->RemoveMappingContext(CameraMovementMappingContext);
			}
		}
	}
}

void ATDCP_CameraPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATDCP_CameraPawn::HandleMoveInput);
		EIC->BindAction(MoveAction, ETriggerEvent::Completed, this, &ATDCP_CameraPawn::HandleMoveInput);
		
		EIC->BindAction(RotateAction, ETriggerEvent::Triggered, this, &ATDCP_CameraPawn::HandleRotateInput);
		EIC->BindAction(RotateAction, ETriggerEvent::Completed, this, &ATDCP_CameraPawn::HandleRotateInput);
		EIC->BindAction(ResetRotateAction, ETriggerEvent::Started, this, &ATDCP_CameraPawn::ResetRotation);
		
		EIC->BindAction(ZoomAction, ETriggerEvent::Triggered, this, &ATDCP_CameraPawn::HandleZoomInput);
		EIC->BindAction(ZoomAction, ETriggerEvent::Completed, this, &ATDCP_CameraPawn::HandleZoomInput);
		EIC->BindAction(ResetZoomAction, ETriggerEvent::Started, this, &ATDCP_CameraPawn::ResetZoom);
	}

}

void ATDCP_CameraPawn::BeginPlay()
{
	Super::BeginPlay();
	
	DefaultYaw = GetActorRotation().Yaw;
	CurrentYaw = DefaultYaw;
	TargetYaw = DefaultYaw;
	
	DefaultBoomRotation = CameraBoom->GetRelativeRotation();
	DefaultFOV = Camera->FieldOfView;
	
	// Initialize zoom
	if (ZoomType == EZoomType::ArmLength)
	{
		TargetZoom = CameraBoom->TargetArmLength;
		CurrentZoom = TargetZoom;
	}
	else
	{
		TargetZoom = Camera->FieldOfView;
		CurrentZoom = TargetZoom;
	}
	

}

void ATDCP_CameraPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bInterpToLocationActive)
	{
		InterpToLocationElapsed += DeltaTime;
		const float Alpha = InterpToLocationDuration <= 0.f
			? 1.f
			: FMath::Clamp(InterpToLocationElapsed / InterpToLocationDuration, 0.f, 1.f);
		SetActorLocation(FMath::Lerp(InterpToLocationStart, InterpToLocationTarget, Alpha), true);

		if (Alpha >= 1.f)
		{
			bInterpToLocationActive = false;
		}
	}
	
	// 1. Calculate edge scroll
	FVector2D EdgeInput = FVector2D::ZeroVector;
	if (bMovementEnabled)
	{
		CalculateEdgeScroll(EdgeInput);
	}

	// 2. Compute total TargetVelocity
	FVector FinalVelocity = bMovementEnabled ? MoveInputVelocity : FVector::ZeroVector;
	if (!EdgeInput.IsNearlyZero())
	{
		const float Yaw = bEdgeScrollCameraRelative ? GetActorRotation().Yaw : 0.f;
		const FRotator YawRotation(0.f, Yaw, 0.f);

		const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		FVector EdgeScrollVelocity = (Forward * EdgeInput.Y + Right * EdgeInput.X).GetClampedToMaxSize(1.f) * EdgeScrollSpeed;

		FinalVelocity += EdgeScrollVelocity;
	}

	TargetVelocity = FinalVelocity.GetClampedToMaxSize(MoveSpeed);

	// 3. Update movement, rotation, zoom
	UpdateMovement(DeltaTime);
	UpdateRotation(DeltaTime);
	UpdateZoom(DeltaTime);
}

void ATDCP_CameraPawn::SetCameraMovementEnabled_Implementation(bool bEnable)
{
	bMovementEnabled = bEnable;

	if (PlayerController && CameraMovementMappingContext)
	{
		if (ULocalPlayer* LP = PlayerController->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
				LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				if (bMovementEnabled)
				{
					Subsystem->AddMappingContext(CameraMovementMappingContext, 0);
				}
				else
				{
					Subsystem->RemoveMappingContext(CameraMovementMappingContext);
				}
			}
		}
	}

	if (!bMovementEnabled)
	{
		MoveInputVelocity = FVector::ZeroVector;
		TargetVelocity = FVector::ZeroVector;
		CurrentVelocity = FVector::ZeroVector;
	}
}

void ATDCP_CameraPawn::SetEdgeScrollEnabled_Implementation(bool bEnable)
{
	bEnableEdgeScroll = bEnable;
}

void ATDCP_CameraPawn::SetRotationEnabled_Implementation(bool bEnable)
{
	bEnableRotation = bEnable;
}

void ATDCP_CameraPawn::SetZoomEnabled_Implementation(bool bEnable)
{
	bZoomEnabled = bEnable;
	if (!bZoomEnabled)
	{
		TargetZoom = CurrentZoom;
	}
}

void ATDCP_CameraPawn::ResetCameraRotation_Implementation()
{
	ResetRotation();
}

void ATDCP_CameraPawn::ResetCameraZoom_Implementation()
{
	ResetZoom();
}

void ATDCP_CameraPawn::SnapCameraToLocation_Implementation(FVector NewLocation)
{
	bInterpToLocationActive = false;
	SetActorLocation(NewLocation, true);
}

void ATDCP_CameraPawn::InterpCameraToLocation_Implementation(FVector NewLocation, float Duration)
{
	if (Duration <= 0.f)
	{
		SnapCameraToLocation_Implementation(NewLocation);
		return;
	}

	bInterpToLocationActive = true;
	InterpToLocationDuration = Duration;
	InterpToLocationElapsed = 0.f;
	InterpToLocationStart = GetActorLocation();
	InterpToLocationTarget = NewLocation;
}

void ATDCP_CameraPawn::SetCameraMoveSpeed_Implementation(float Speed)
{
	MoveSpeed = FMath::Max(0.f, Speed);
	MoveInputVelocity = MoveInputVelocity.GetClampedToMaxSize(MoveSpeed);
	TargetVelocity = TargetVelocity.GetClampedToMaxSize(MoveSpeed);
}

void ATDCP_CameraPawn::SetRotationSpeed_Implementation(float Speed)
{
	RotationSpeed = FMath::Max(0.f, Speed);
}

void ATDCP_CameraPawn::SetZoomSpeed_Implementation(float Speed)
{
	ZoomStep = FMath::Max(0.f, Speed);
}

void ATDCP_CameraPawn::SetMinMaxZoom_Implementation(float InMinZoom, float InMaxZoom)
{
	MinZoom = FMath::Min(InMinZoom, InMaxZoom);
	MaxZoom = FMath::Max(InMinZoom, InMaxZoom);
	TargetZoom = FMath::Clamp(TargetZoom, MinZoom, MaxZoom);
	CurrentZoom = FMath::Clamp(CurrentZoom, MinZoom, MaxZoom);
}

void ATDCP_CameraPawn::SetTiltZoomEnabled_Implementation(bool bEnable, float InTiltAmount)
{
	bTiltWithZoom = bEnable;
	TiltAmount = InTiltAmount;
}

void ATDCP_CameraPawn::SetEdgeScrollSpeed_Implementation(float Speed)
{
	EdgeScrollSpeed = FMath::Max(0.f, Speed);
}

void ATDCP_CameraPawn::SetEdgeScrollThreshold_Implementation(float Pixels)
{
	EdgeScrollThreshold = FMath::Max(0.f, Pixels);
}

void ATDCP_CameraPawn::SetCameraRelativeMovement_Implementation(bool bEnable)
{
	bCameraRelativeMovement = bEnable;
}

void ATDCP_CameraPawn::UpdateMovement(const float& DeltaTime)
{
	const float InterpSpeed = TargetVelocity.IsNearlyZero() ? Deceleration : Acceleration;
	CurrentVelocity = FMath::VInterpTo(CurrentVelocity, TargetVelocity, DeltaTime, InterpSpeed);

	// Prevent micro jitter
	if (CurrentVelocity.SizeSquared() < 1.f)
	{
		CurrentVelocity = FVector::ZeroVector;
	}

	AddActorWorldOffset(CurrentVelocity * DeltaTime, true);
}

void ATDCP_CameraPawn::UpdateRotation(const float& DeltaTime)
{
	CurrentYaw = FMath::FInterpTo(CurrentYaw, TargetYaw, DeltaTime, RotationInterpSpeed);
	SetActorRotation(FRotator(0.f, CurrentYaw, 0.f));
}

void ATDCP_CameraPawn::UpdateZoom(const float& DeltaTime)
{
	CurrentZoom = FMath::FInterpTo(CurrentZoom, TargetZoom, DeltaTime, ZoomInterpSpeed);
    
        if (ZoomType == EZoomType::ArmLength)
        {
            CameraBoom->TargetArmLength = CurrentZoom;
    
            if (bTiltWithZoom)
            {
                float ZoomAlpha = (CurrentZoom - MinZoom) / (MaxZoom - MinZoom);
                float TiltPitch = FMath::Lerp(-TiltAmount, TiltAmount, ZoomAlpha); // -TiltAmount = zoom in, TiltAmount = zoom out
    
                FRotator NewRotation = DefaultBoomRotation;
                NewRotation.Pitch += TiltPitch;
                CameraBoom->SetRelativeRotation(NewRotation);
            }
        }
        else
        {
            Camera->SetFieldOfView(CurrentZoom);
        }
}

void ATDCP_CameraPawn::CalculateEdgeScroll(FVector2D& OutEdgeInput)
{
	OutEdgeInput = FVector2D::ZeroVector;

	if (!bEnableEdgeScroll || !PlayerController) return;

	float MouseX, MouseY;
	if (!PlayerController->GetMousePosition(MouseX, MouseY)) return;

	int32 ViewportX, ViewportY;
	PlayerController->GetViewportSize(ViewportX, ViewportY);

	if (MouseX <= EdgeScrollThreshold) OutEdgeInput.X = -1.f;
	else if (MouseX >= ViewportX - EdgeScrollThreshold) OutEdgeInput.X = 1.f;

	if (MouseY <= EdgeScrollThreshold) OutEdgeInput.Y = 1.f;
	else if (MouseY >= ViewportY - EdgeScrollThreshold) OutEdgeInput.Y = -1.f;
}

void ATDCP_CameraPawn::HandleZoomInput(const FInputActionValue& Value)
{
	if (!bZoomEnabled) return;

	const float AxisValue = Value.Get<float>();
	if (FMath::IsNearlyZero(AxisValue)) return;

	TargetZoom -= AxisValue * ZoomStep;
	TargetZoom = FMath::Clamp(TargetZoom, MinZoom, MaxZoom);
}

void ATDCP_CameraPawn::ResetRotation()
{
	TargetYaw = DefaultYaw;
}

void ATDCP_CameraPawn::ResetZoom()
{
	if (ZoomType == EZoomType::ArmLength)
	{
		TargetZoom = (MinZoom + MaxZoom) / 2.f; // Smoothly interpolate
	}
	else
	{
		TargetZoom = DefaultFOV; // Smoothly interpolate
	}
}

void ATDCP_CameraPawn::HandleMoveInput(const FInputActionValue& Value)
{
	if (!bMovementEnabled)
	{
		MoveInputVelocity = FVector::ZeroVector;
		return;
	}

	const FVector2D Input = Value.Get<FVector2D>();
	if (!Controller) return;

	if (Input.IsNearlyZero())
	{
		MoveInputVelocity = FVector::ZeroVector;
		return;
	}

	const float Yaw = bCameraRelativeMovement ? GetActorRotation().Yaw : 0.f;
	const FRotator YawRotation(0.f, Yaw, 0.f);

	const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	const FVector Direction = (Forward * Input.Y + Right * Input.X).GetClampedToMaxSize(1.f);

	MoveInputVelocity = Direction * MoveSpeed;
}

void ATDCP_CameraPawn::HandleRotateInput(const FInputActionValue& Value)
{
	if (!bEnableRotation) return;

	const float Input = Value.Get<float>();

	if (FMath::IsNearlyZero(Input)) return;

	TargetYaw += Input * RotationSpeed * GetWorld()->GetDeltaSeconds();
}





