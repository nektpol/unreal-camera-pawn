// Fill out your copyright notice in the Description page of Project Settings.


#include "TDCP_CameraPawn.h"

// Sets default values
ATDCP_CameraPawn::ATDCP_CameraPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ATDCP_CameraPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATDCP_CameraPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ATDCP_CameraPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

