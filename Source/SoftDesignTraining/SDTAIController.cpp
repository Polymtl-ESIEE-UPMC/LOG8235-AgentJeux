// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTAIController.h"
#include "SDTCollectible.h"
#include "SoftDesignTraining.h"
#include "DrawDebugHelpers.h"

void ASDTAIController::Tick(float deltaTime)
{
    APawn* const pawn = GetPawn();
    UWorld* const World = GetWorld();
    
    if (pawn) {
        // position initiale de l'agent
        FVector Start = pawn->GetActorLocation();

        // Identification des paramètres pour les trois lancer de rayons
        FVector viewDirection = pawn->GetActorRotation().Vector();
        int rayDistance = 500 ;
        FVector EndLeft = Start + rayDistance * viewDirection.RotateAngleAxis(-45, FVector(0,0,1));
        FVector EndMiddle = Start + rayDistance * viewDirection;
        FVector EndRight = Start + rayDistance * viewDirection.RotateAngleAxis(45, FVector(0, 0, 1));
        FHitResult HitResult;
        TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes;
        FCollisionObjectQueryParams ObjectQueryParams(FCollisionObjectQueryParams::AllObjects);
        FCollisionQueryParams QueryParams = FCollisionQueryParams::DefaultQueryParam;
        QueryParams.AddIgnoredActor(pawn);

        World->LineTraceSingleByObjectType(HitResult, Start, EndLeft, ObjectQueryParams, QueryParams);
        World->LineTraceSingleByObjectType(HitResult, Start, EndRight, ObjectQueryParams, QueryParams);
        World->LineTraceSingleByObjectType(HitResult, Start, EndMiddle, ObjectQueryParams, QueryParams);

        //ajout de ligne de debug dans le jeu pour visualiser les lancer de rayons
        DrawDebugLine(World, Start, EndLeft, FColor::Orange, false, 0.1f);
        DrawDebugLine(World, Start, EndMiddle, FColor::Orange, false, 0.1f);
        DrawDebugLine(World, Start, EndRight, FColor::Orange, false, 0.1f);

        //AddAiMovement(pawn, viewDirection);
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




