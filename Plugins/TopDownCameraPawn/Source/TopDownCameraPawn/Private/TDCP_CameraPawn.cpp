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
	}

}

void ATDCP_CameraPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

void ATDCP_CameraPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	UpdateMovement(DeltaTime);

}

void ATDCP_CameraPawn::UpdateMovement(const float& DeltaTime)
{
	const float InterpSpeed = TargetVelocity.IsNearlyZero() ? Deceleration : Acceleration;

	CurrentVelocity = FMath::VInterpTo(CurrentVelocity,	TargetVelocity,	DeltaTime, InterpSpeed);

	// Prevent micro jitter
	if (CurrentVelocity.SizeSquared() < 1.f)
	{
		CurrentVelocity = FVector::ZeroVector;
	}

	AddActorWorldOffset(CurrentVelocity * DeltaTime, true);
}

void ATDCP_CameraPawn::HandleMoveInput(const FInputActionValue& Value)
{
	const FVector2D Input = Value.Get<FVector2D>();

	if (!Controller) return;

	if (Input.IsNearlyZero())
	{
		TargetVelocity = FVector::ZeroVector;
		return;
	}

	const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);

	const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector Right   = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	const FVector Direction = (Forward * Input.Y + Right * Input.X).GetClampedToMaxSize(1.f);

	TargetVelocity = Direction * MoveSpeed;
}





