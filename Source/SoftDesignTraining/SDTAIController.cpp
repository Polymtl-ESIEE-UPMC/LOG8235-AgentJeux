// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTAIController.h"
#include "SoftDesignTraining.h"
#include "DrawDebugHelpers.h"

void ASDTAIController::Tick(float deltaTime)
{
    APawn* const pawn = GetPawn();
    UWorld* const World = GetWorld();
    
    if (pawn) {
        FVector Start = pawn->GetActorLocation();
        FVector End = Start + 1000*pawn->GetActorRotation().Vector();
        TArray < FHitResult > HitResult;
        DrawDebugLine(World, Start, End, FColor::Orange, false, 0.1f);

        World->LineTraceMultiByChannel(HitResult, Start, End, ECollisionChannel::ECC_WorldStatic);

        UE_LOG(LogTemp, Warning, TEXT("%f"), HitResult[0].Distance);

        //AddAiMovement(pawn, FVector(1.0f, 0.0f, 0.0f));
    }
}

void ASDTAIController::AddAiMovement(APawn* pawn, FVector movementDirection) {
    // Adding movement 
    movementDirection.Normalize();
    pawn->AddMovementInput(movementDirection, movementSpeed);

    // get current rotation
    FVector viewDirection = pawn->GetActorRotation().Vector();

    //finding the angle between the view direction and the movement direction
    float rotationAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(viewDirection, movementDirection)));

    // update current rotation
    FRotator deltaRotation = FRotator(0, rotationAngle, 0);
    pawn->AddActorWorldRotation(deltaRotation);

}




