// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTAIController.h"
#include "SoftDesignTraining.h"
#include "SoftDesignTrainingMainCharacter.h"
#include "SDTCollectible.h"
#include "SDTFleeLocation.h"
#include "SDTPathFollowingComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
//#include "UnrealMathUtility.h"
#include "SDTUtils.h"
#include "EngineUtils.h"

ASDTAIController::ASDTAIController(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer.SetDefaultSubobjectClass<USDTPathFollowingComponent>(TEXT("PathFollowingComponent")))
{
}

void ASDTAIController::Tick(float DeltaSeconds)
{
    // My idea of how we could do the navigation was the following : 
    // We keep 3 variables in the class to help indicate the state of the ai
    // targetCollectible -> collectible the closest collectible we are trying to go to
    //           we can use this target to verify the availability of the power-up
    // path -> current path we are following
    // pawnState -> enum to keep track of the current state { FleeingPlayer, ChasingPlayer, CollectingPowerUps }
    // the main tick methode would consist of updating the target and path
    // and then taking a step in the direction of the path

    // to update the target and path here are the following steps :
    // 1. if the player is visible and not powered-up, set the path to the player location and pawnState to ChasingPlayer
    // 2. if the player is visible and powered-up, set path to closest escape point and pawnState to FleeingPlayer
    // 3. if player is not visible, but pawnState is ChasingPlayer. Do not change path, the pawn must go to the last known location of the player which has been recorded in step 1.
    // 4. if player is not visible, but pawnState is FleeingPlayer. Do not change path, the pawn must reach the escape point
    // 5. if targetCollectible is a nullptr or is on cooldown, set targetCollectible and path to closest collectible
    // 6. if the agent finishes a path, set targetCollectible and path to closest collectible
    
    // I believe this should give us the correct agent behavior
    // I have updated the Choosebehavior function accordingly
    // here are the functions we need to work on :
    //      setPathToLocation(FVector location) -> defines a new path to the given location 
    //      setPathToBestEscapePoint() -> chooses the best escape point and calls setPathToLocation() with its location
    //      ShowNavigationPath() -> Show current navigation path DrawDebugLine and DrawDebugSphere
    //      GoToBestTarget(float deltaTime) -> takes a step in the direction of the path 

    // update the target and path
    ChooseBehavior(DeltaSeconds);
    
    // follow the path  
    GoToBestTarget(DeltaSeconds);
}

void ASDTAIController::GoToBestTarget(float deltaTime)
{
    //Move to target depending on current behavior
}

void ASDTAIController::OnMoveToTarget()
{
    m_ReachedTarget = false;
}

void ASDTAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
    Super::OnMoveCompleted(RequestID, Result);

    m_ReachedTarget = true;
}

void ASDTAIController::ShowNavigationPath()
{
    //Show current navigation path DrawDebugLine and DrawDebugSphere
}

void ASDTAIController::ChooseBehavior(float deltaTime)
{
    UpdatePlayerInteraction(deltaTime);
}

void ASDTAIController::UpdatePlayerInteraction(float deltaTime)
{
    //finish jump before updating AI state
    if (AtJumpSegment)
        return;

    APawn* selfPawn = GetPawn();
    if (!selfPawn)
        return;

    ACharacter* playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!playerCharacter)
        return;

    FVector detectionStartLocation = selfPawn->GetActorLocation() + selfPawn->GetActorForwardVector() * m_DetectionCapsuleForwardStartingOffset;
    FVector detectionEndLocation = detectionStartLocation + selfPawn->GetActorForwardVector() * m_DetectionCapsuleHalfLength * 2;

    TArray<TEnumAsByte<EObjectTypeQuery>> detectionTraceObjectTypes;
    // I commented this section since i don't think it is usefull to detect powerups in front of the player
    // detectionTraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(COLLISION_COLLECTIBLE));
    detectionTraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(COLLISION_PLAYER));

    TArray<FHitResult> allDetectionHits;
    GetWorld()->SweepMultiByObjectType(allDetectionHits, detectionStartLocation, detectionEndLocation, FQuat::Identity, detectionTraceObjectTypes, FCollisionShape::MakeSphere(m_DetectionCapsuleRadius));

    FHitResult detectionHit;
    GetHightestPriorityDetectionHit(allDetectionHits, detectionHit);

    //Set behavior based on hit
    if (detectionHit.GetComponent() != nullptr && detectionHit.GetComponent()->GetCollisionObjectType() == COLLISION_PLAYER) {
        ASoftDesignTrainingMainCharacter* mainCharacter = static_cast<ASoftDesignTrainingMainCharacter*>(GetWorld()->GetFirstPlayerController()->GetCharacter());
        
        if (mainCharacter->IsPoweredUp()) {

            // agent needs to flee the player
            pawnState = FleeingPlayer;
            setPathToBestEscapePoint();
            return;

        }
        else {

            // agent is chasing the player and we set the path to the current player position
            pawnState = ChasingPlayer;
            setPathToLocation(mainCharacter->GetActorLocation());
            return;

        }

        
    }

    // if the actor is not already following a path or the target collectible has been picked up by someone else, find a new path to follow to the closest collectible
    ASDTCollectible* target = Cast<ASDTCollectible>(targetCollectible);
    if (!hasPath || target!= nullptr ||  target->IsOnCooldown()) {
        setTargetCollectible();
        setPathToLocation(target->GetActorLocation());
    }


    DrawDebugCapsule(GetWorld(), detectionStartLocation + m_DetectionCapsuleHalfLength * selfPawn->GetActorForwardVector(), m_DetectionCapsuleHalfLength, m_DetectionCapsuleRadius, selfPawn->GetActorQuat() * selfPawn->GetActorUpVector().ToOrientationQuat(), FColor::Blue);
}

void ASDTAIController::GetHightestPriorityDetectionHit(const TArray<FHitResult>& hits, FHitResult& outDetectionHit)
{
    for (const FHitResult& hit : hits)
    {
        if (UPrimitiveComponent* component = hit.GetComponent())
        {
            if (component->GetCollisionObjectType() == COLLISION_PLAYER)
            {
                //we can't get more important than the player
                outDetectionHit = hit;
                return;
            }
            else if (component->GetCollisionObjectType() == COLLISION_COLLECTIBLE)
            {
                outDetectionHit = hit;
            }
        }
    }
}

void ASDTAIController::AIStateInterrupted()
{
    StopMovement();
    m_ReachedTarget = true;
}

void ASDTAIController::setTargetCollectible() 
{
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASDTCollectible::StaticClass(), FoundActors);

    FVector pawnLocation = GetPawn()->GetActorLocation();
    float bestDistance;
    AActor* closestCollectible = nullptr;
    for (AActor* collectible : FoundActors)
    {
        if (closestCollectible == nullptr){
            closestCollectible = collectible;
            bestDistance = (pawnLocation - closestCollectible->GetActorLocation()).SizeSquared();
        }
        else {
            float currentDistance = (pawnLocation - collectible->GetActorLocation()).SizeSquared();
            if (currentDistance < bestDistance) {
                closestCollectible = collectible;
                bestDistance = currentDistance;
            }
        }
    }

    targetCollectible = closestCollectible;
}

void ASDTAIController::setPathToLocation(FVector location) {

}
void ASDTAIController::setPathToBestEscapePoint() {

}