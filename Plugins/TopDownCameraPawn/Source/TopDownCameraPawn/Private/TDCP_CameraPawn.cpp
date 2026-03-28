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
				Subsystem->AddMappingContext(CameraMovementMappingContext, 0);
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
	
	// 1. Calculate edge scroll
	FVector2D EdgeInput;
	CalculateEdgeScroll(EdgeInput);

	// 2. Compute total TargetVelocity
	FVector FinalVelocity = MoveInputVelocity;
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





