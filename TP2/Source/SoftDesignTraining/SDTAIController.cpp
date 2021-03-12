// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTAIController.h"
#include "SoftDesignTraining.h"
#include "SDTCollectible.h"
#include "SDTFleeLocation.h"
#include "SDTPathFollowingComponent.h"
#include "SoftDesignTrainingMainCharacter.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
//#include "UnrealMathUtility.h"
#include "SDTUtils.h"
#include "EngineUtils.h"

ASDTAIController::ASDTAIController(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer.SetDefaultSubobjectClass<USDTPathFollowingComponent>(TEXT("PathFollowingComponent")))
{
}

void ASDTAIController::GoToBestTarget(float deltaTime)
{
    //Move to target depending on current behavior
    EPathFollowingRequestResult::Type moveResult  = MoveToLocation(m_location);
    if (moveResult == EPathFollowingRequestResult::RequestSuccessful) {
        OnMoveToTarget();
    }
}

void ASDTAIController::OnMoveToTarget()
{
    m_hasPath = true;
    m_ReachedTarget = false;
}

void ASDTAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
    Super::OnMoveCompleted(RequestID, Result);
    m_ReachedTarget = true;
    m_targetCollectible = nullptr;
    m_hasPath = false;
}

void ASDTAIController::ShowNavigationPath()
{
    //Show current navigation path DrawDebugLine and DrawDebugSphere
    if (m_PathFollowingComponent->GetPath().IsValid()) {
        //Get all the point of the path 
        const TArray<FNavPathPoint>& points = m_PathFollowingComponent->GetPath()->GetPathPoints();
        FVector PathStartingPoint = points[0].Location;
        FVector PathEndingPoint;
        //Draw the path with some DrawDebugSphere 
        for (FNavPathPoint point : points) {
            PathEndingPoint = point.Location;
            DrawDebugLine(GetWorld(), PathStartingPoint, PathEndingPoint, FColor::Red);
            DrawDebugSphere(GetWorld(), PathEndingPoint, 10.0f, 10, FColor::Green);
            PathStartingPoint = PathEndingPoint;
        }
    }
}

void ASDTAIController::ChooseBehavior(float deltaTime)
{
    UpdatePlayerInteraction(deltaTime);
}

void ASDTAIController::UpdatePlayerInteraction(float deltaTime)
{
    
    //finish jump before updating AI state
    if (AtJumpSegment) {
        UpdateJump(deltaTime);
        return;
    }
        
    APawn* selfPawn = GetPawn();
    if (!selfPawn)
        return;

    //Commenter pour le mode simulation <- A enlever 
    /*ACharacter* playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!playerCharacter) {
        return;
    }*/

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
            // Stop the agent to make him flee the player
            AIStateInterrupted();
            // agent needs to flee the player
            m_pawnState = FleeingPlayer;
            setPathToBestEscapePoint();
            return;

        }
        else {
            // Stop the agent and make him chase the player
            AIStateInterrupted();
            // agent is chasing the player and we set the path to the current player position
            m_pawnState = ChasingPlayer;
            m_location = mainCharacter->GetActorLocation();
            return;
        }
    }

    // if the actor is not already following a path or the target collectible has been picked up by someone else, find a new path to follow to the closest collectible
    ASDTCollectible* target = Cast<ASDTCollectible>(m_targetCollectible);
    if (m_targetCollectible != nullptr && Cast<ASDTCollectible>(m_targetCollectible)->IsOnCooldown()) {
        AIStateInterrupted();
    }
    if (!m_hasPath || target != nullptr) {
        setTargetCollectible();
        if (!Cast<ASDTCollectible>(m_targetCollectible)->IsOnCooldown()) {
            FVector location = m_targetCollectible->GetActorLocation();
            m_location = location;
        }
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
    m_targetCollectible = nullptr;
    m_hasPath = false;
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
        if (!Cast<ASDTCollectible>(collectible)->IsOnCooldown()) {
            if (closestCollectible == nullptr) {
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
    }

    m_targetCollectible = closestCollectible;
}


void ASDTAIController::setPathToBestEscapePoint() {
    ///Meme algo que pour collectible, mais pour le flee point 

}

void ASDTAIController::JumpStart() {
    // Return if alredy at jumpSegment
    if (AtJumpSegment)
        return;
    else {
        AIheight = GetPawn()->GetActorLocation().Z;
        AtJumpSegment = true;
        InAir = true;
        Landing = false;
        jumpTimer = 0;
    }
}

void ASDTAIController::UpdateJump(float delatTime) {
    APawn* selfPawn = GetPawn();
    if (!selfPawn)
        return;
    FVector position = selfPawn->GetActorLocation();
    jumpTimer += delatTime;

    float minJumpTime;
    float maxJumpTime;
    JumpCurve->GetTimeRange(minJumpTime, maxJumpTime);

    // Jump is finish reset
    if (jumpTimer >= maxJumpTime) {
        Landing = true;
        InAir = false;
        AtJumpSegment = false;
        jumpTimer = 0;
    }
    else {
        position.Z = AIheight + JumpApexHeight * JumpCurve->GetFloatValue(jumpTimer);
        selfPawn->SetActorLocation(position);
    }
}