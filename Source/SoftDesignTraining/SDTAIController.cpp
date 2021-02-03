// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTAIController.h"
#include "SDTCollectible.h"
#include "SoftDesignTraining.h"
#include "DrawDebugHelpers.h"

void ASDTAIController::Tick(float deltaTime)
{
    APawn* const pawn = GetPawn();
    UWorld* const World = GetWorld();
    
    FVector viewDirection = pawn->GetActorRotation().Vector();
        
    TArray<FHitResult> visibleElements = CollectVisibleElements(pawn, World, viewDirection);
    bool deathZoneAhead = DetectDeathZone(pawn, World, viewDirection);

    APawn* playerPawn = World->GetFirstPlayerController()->GetPawn();

    if (deathZoneAhead) {
        // do something
    }
    else if (playerPawn && isPlayerVisible(pawn, playerPawn->GetActorLocation(), viewDirection)) {
        //check if powered_up
    }
    else {
        for (int i = 0; i < visibleElements.Num(); ++i) {
            if (visibleElements[i].GetComponent() != nullptr)
            {
                // check if a consumable is detected
                if (visibleElements[i].GetComponent()->GetCollisionObjectType() == ECC_GameTraceChannel5 ) {
                    //do something
                }

                // check if a wall is detected
                if (visibleElements[i].GetComponent()->GetCollisionObjectType() == ECC_WorldStatic) {
                    //do something
                }
            }
    
        }
    }


    //AddAiMovement(pawn, viewDirection);
    
}

bool ASDTAIController::isPlayerVisible(APawn* pawn, FVector playerPosition , FVector viewDirection) {
    FVector playerDirection = playerPosition - pawn->GetActorLocation();
    int playerDistance = playerDirection.Size();
    
    if (playerDistance > viewDistance)
        return false;

    int angle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(viewDirection, playerDirection)));

    return FMath::Abs(angle) <= viewAngle;
}

TArray<FHitResult> ASDTAIController::CollectVisibleElements(APawn* pawn, UWorld* World, FVector viewDirection) {
        // position initiale de l'agent
        FVector Start = pawn->GetActorLocation();

        // Identification des paramètres pour les trois lancer de rayons
        int rayDistance = 500 ;
        FVector EndLeft = Start + rayDistance * viewDirection.RotateAngleAxis(-30, FVector(0,0,1));
        FVector EndMiddle = Start + rayDistance * viewDirection;
        FVector EndRight = Start + rayDistance * viewDirection.RotateAngleAxis(30, FVector(0, 0, 1));
        FHitResult HitResultLeft, HitResultMiddle, HitResultRight;
        TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes;
        FCollisionObjectQueryParams ObjectQueryParams(FCollisionObjectQueryParams::AllObjects);
        FCollisionQueryParams QueryParams = FCollisionQueryParams::DefaultQueryParam;
        QueryParams.AddIgnoredActor(pawn);

        World->LineTraceSingleByObjectType(HitResultLeft, Start, EndLeft, ObjectQueryParams, QueryParams);
        World->LineTraceSingleByObjectType(HitResultMiddle, Start, EndRight, ObjectQueryParams, QueryParams);
        World->LineTraceSingleByObjectType(HitResultRight, Start, EndMiddle, ObjectQueryParams, QueryParams);

        //ajout de ligne de debug dans le jeu pour visualiser les lancer de rayons
        DrawDebugLine(World, Start, EndLeft, FColor::Orange, false, 0.1f);
        DrawDebugLine(World, Start, EndMiddle, FColor::Orange, false, 0.1f);
        DrawDebugLine(World, Start, EndRight, FColor::Orange, false, 0.1f);

        TArray<FHitResult> visibleElements;
        visibleElements.Add(HitResultLeft);
        visibleElements.Add(HitResultMiddle);
        visibleElements.Add(HitResultRight);
        return visibleElements;
}

bool ASDTAIController::DetectDeathZone(APawn* pawn, UWorld* World, FVector viewDirection) {
    // Identification des paramètres pour les trois lancer de rayons
    int rayDistance = 500;
    FVector Start = pawn->GetActorLocation();
    FVector End = Start + rayDistance * viewDirection.RotateAngleAxis(15, FVector(0, 1, 0));
    FHitResult HitResult;
    FCollisionObjectQueryParams ObjectQueryParams(FCollisionObjectQueryParams::AllObjects);
    FCollisionQueryParams QueryParams = FCollisionQueryParams::DefaultQueryParam;
    QueryParams.AddIgnoredActor(pawn);

    World->LineTraceSingleByObjectType(HitResult, Start, End, ObjectQueryParams, QueryParams);
    DrawDebugLine(World, Start, End, FColor::Orange, false, 0.1f);

    if (HitResult.GetComponent())
        return HitResult.GetComponent()->GetCollisionObjectType() == ECC_GameTraceChannel3;
    return false;
}

// Add a movement input to the pawn and set the rotation to be in the same direction
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




