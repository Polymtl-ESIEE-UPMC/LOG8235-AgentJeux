// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTAIController.h"
#include "SoftDesignTraining.h"
#include "SDTCollectible.h"
#include "SDTFleeLocation.h"
#include "SDTPathFollowingComponent.h"
#include "SoftDesignTrainingMainCharacter.h"
#include "DrawDebugHelpers.h"
#include "NavigationSystem.h"
#include "Kismet/KismetMathLibrary.h"
//#include "UnrealMathUtility.h"
#include "SDTUtils.h"
#include "EngineUtils.h"

ASDTAIController::ASDTAIController(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer.SetDefaultSubobjectClass<USDTPathFollowingComponent>(TEXT("PathFollowingComponent")))
{
}

/// <summary>
/// Move the AI to the best target determine in ChooseBehavior
/// </summary>
/// <param name="deltaTime"> Time between two frames </param>
void ASDTAIController::GoToBestTarget(float deltaTime)
{
    //Move to target depending on current behavior
    EPathFollowingRequestResult::Type moveResult  = MoveToLocation(m_location);
    if (moveResult == EPathFollowingRequestResult::RequestSuccessful) {
        OnMoveToTarget();
    }
}

/// <summary>
/// Put the AI on move to stop any other indication for another path 
/// </summary>
void ASDTAIController::OnMoveToTarget()
{
    m_hasPath = true;
    m_ReachedTarget = false;
}

/// <summary>
/// When the AI complete is path, reset all the attributes connected to the path system and path determination
/// </summary>
/// <param name="RequestID"> FAIRequestID </param>
/// <param name="Result"> FPathFollowingResult </param>
void ASDTAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
    Super::OnMoveCompleted(RequestID, Result);
    m_ReachedTarget = true;
    m_targetCollectible = nullptr;
    m_hasPath = false;
    m_pawnState = CollectingPowerUps;
}

/// <summary>
/// Show the AI Navigation path 
/// </summary>
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

/// <summary>
/// Choose the IA behavior
/// </summary>
/// <param name="deltaTime"> Time between two frames </param>
void ASDTAIController::ChooseBehavior(float deltaTime)
{
    UpdatePlayerInteraction(deltaTime);
}


/// <summary>
/// Update the Ai interaction path with the world 
/// </summary>
/// <param name="deltaTime"> Time between two frames </param>
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

    ACharacter* playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!playerCharacter) {
        return;
    }

    FVector detectionStartLocation = selfPawn->GetActorLocation() + selfPawn->GetActorForwardVector() * m_DetectionCapsuleForwardStartingOffset;
    FVector detectionEndLocation = detectionStartLocation + selfPawn->GetActorForwardVector() * m_DetectionCapsuleHalfLength * 2;

    TArray<TEnumAsByte<EObjectTypeQuery>> detectionTraceObjectTypes;
    detectionTraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(COLLISION_PLAYER));

    TArray<FHitResult> allDetectionHits;
    GetWorld()->SweepMultiByObjectType(allDetectionHits, detectionStartLocation, detectionEndLocation, FQuat::Identity, detectionTraceObjectTypes, FCollisionShape::MakeSphere(m_DetectionCapsuleRadius));

    FHitResult detectionHit;
    GetHightestPriorityDetectionHit(allDetectionHits, detectionHit);

    //Set behavior based on hit
    if (detectionHit.GetComponent() != nullptr && detectionHit.GetComponent()->GetCollisionObjectType() == COLLISION_PLAYER) {
        ASoftDesignTrainingMainCharacter* mainCharacter = static_cast<ASoftDesignTrainingMainCharacter*>(GetWorld()->GetFirstPlayerController()->GetCharacter());

        FVector Start = GetPawn()->GetActorLocation();
        FHitResult HitResult;
        FCollisionObjectQueryParams ObjectQueryParams(FCollisionObjectQueryParams::AllObjects);
        FCollisionQueryParams QueryParams = FCollisionQueryParams::DefaultQueryParam;
        QueryParams.AddIgnoredActor(GetPawn());

        GetWorld()->LineTraceSingleByObjectType(HitResult, Start, mainCharacter->GetActorLocation(), ObjectQueryParams, QueryParams);
        
        if (HitResult.GetComponent()->GetCollisionObjectType() != ECC_GameTraceChannel4) {
            //player is not visible
        }
        else if (mainCharacter->IsPoweredUp()) {
            // Stop the agent to make him flee the player
            if (m_pawnState != FleeingPlayer)
                AIStateInterrupted();
            // agent needs to flee the player
            m_pawnState = FleeingPlayer;
            setPathToBestEscapePoint();
            return;

        }
        else {
            if(m_pawnState != ChasingPlayer)
                AIStateInterrupted();
            // agent is chasing the player and we set the path to the current player position
            m_pawnState = ChasingPlayer;
            m_location = mainCharacter->GetActorLocation();
            return;
        }
    }
    else if (m_pawnState == CollectingPowerUps) {
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
    }

    DrawDebugCapsule(GetWorld(), detectionStartLocation + m_DetectionCapsuleHalfLength * selfPawn->GetActorForwardVector(), m_DetectionCapsuleHalfLength, m_DetectionCapsuleRadius, selfPawn->GetActorQuat() * selfPawn->GetActorUpVector().ToOrientationQuat(), FColor::Blue);
}

/// <summary>
/// Determine the highest priority on a detection hit
/// </summary>
/// <param name="hits"> Array of hits results </param>
/// <param name="outDetectionHit"> Array of hits results </param>
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

/// <summary>
/// Determine the best and closest collectible for the AI
/// </summary>
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
            //Determined path to the collectible point location
            float pathLength = UNavigationSystemV1::FindPathToLocationSynchronously(GetWorld(), pawnLocation, collectible->GetActorLocation())->GetPathLength();
            if (closestCollectible == nullptr) {
                closestCollectible = collectible;
                bestDistance = pathLength;
            }
            else {
                if (pathLength < bestDistance) {
                    closestCollectible = collectible;
                    bestDistance = pathLength;
                }
            }

        }
    }
    m_targetCollectible = closestCollectible;
}

/// <summary>
/// Determine the best and closest escape point for the AI without crossing the player
/// </summary>
void ASDTAIController::setPathToBestEscapePoint() {
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASDTFleeLocation::StaticClass(), FoundActors);

    ASoftDesignTrainingMainCharacter* mainCharacter = static_cast<ASoftDesignTrainingMainCharacter*>(GetWorld()->GetFirstPlayerController()->GetCharacter());

    FVector pawnLocation = GetPawn()->GetActorLocation();
    float bestLength;
    AActor* closestEscapePoint = nullptr;
    for (AActor* escapePoint : FoundActors)
    {
        //Determined path to the escape point location
        UNavigationPath* path = UNavigationSystemV1::FindPathToLocationSynchronously(GetWorld(), pawnLocation, escapePoint->GetActorLocation());
        float pathLength = path->GetPathLength();
        if (closestEscapePoint == nullptr) {
            closestEscapePoint = escapePoint;
            bestLength = pathLength;
        }
        else {
            FVector nextPoint = path->GetPath()->GetPathPoints()[0].Location;
            float distToPoint = (mainCharacter->GetActorLocation() - nextPoint).SizeSquared();
            float distToPlayer = (mainCharacter->GetActorLocation() - pawnLocation).SizeSquared();
            if (distToPlayer < distToPoint && pathLength < bestLength) {
                closestEscapePoint = escapePoint;
                bestLength = pathLength;
            }
        }
        
    }
    m_location = closestEscapePoint->GetActorLocation();
}

/// <summary>
/// Function to start the jump animation over the death trap
/// </summary>
void ASDTAIController::JumpStart() {
    // Return if alredy at jumpSegment
    if (AtJumpSegment)
        return;
    else {
        m_AIheight = GetPawn()->GetActorLocation().Z;
        AtJumpSegment = true;
        InAir = true;
        Landing = false;
        jumpTimer = 0;
    }
}

/// <summary>
/// Function to update the jump animation and height with the jump curve 
/// </summary>
/// <param name="delatTime"> Time between two frames</param>
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
        position.Z = m_AIheight + JumpApexHeight * JumpCurve->GetFloatValue(jumpTimer);
        selfPawn->SetActorLocation(position);
    }
}