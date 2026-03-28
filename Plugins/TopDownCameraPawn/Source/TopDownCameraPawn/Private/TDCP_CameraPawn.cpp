// Developed by Neko Creative Technologies


#include "TDCP_CameraPawn.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

ATDCP_CameraPawn::ATDCP_CameraPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->TargetArmLength = 1200.f;
	CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
}

void ATDCP_CameraPawn::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	CameraBoom->TargetArmLength = DefaultArmLength;
	CameraBoom->SetRelativeRotation(FRotator(DefaultPitch, 0.f, 0.f));
}

void ATDCP_CameraPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

void ATDCP_CameraPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ATDCP_CameraPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

