// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTAIController.h"
#include "SDTCollectible.h"
#include "SoftDesignTraining.h"
#include "SoftDesignTrainingMainCharacter.h"
#include "DrawDebugHelpers.h"
#include "SDTUtils.h"

void ASDTAIController::Tick(float deltaTime)
{
    APawn* pawn = GetPawn();
    UWorld* world = GetWorld(); 

    if (IsDeathZoneAhead())
    {
        Navigation(pawn, world, true, deltaTime);
    }
    else if (world->GetFirstPlayerController()->GetPawn() && IsPlayerVisible())
    {
        NavigateToPlayer(deltaTime);
    }
    // Detect PickUp
    else if (GetCollectibleDirection() != FVector(0, 0, 0))
    {
        NavigateToCollectible(deltaTime);
    }
    else {
        Navigation(pawn, world, false, deltaTime);
    }
    FVector viewDirection = GetPawn()->GetActorRotation().Vector();
    ChangeAISpeed(accelerationSpeed, deltaTime);
    MoveAI(viewDirection);
}

// Add a movement input to the pawn and set the rotation to be in the same direction
// Entry    FVector movementDirection -> pawn's movement direction 
// Return   -
void ASDTAIController::MoveAI(FVector movementDirection)
{
    // Adding movement 
    movementDirection.Normalize();
    GetPawn()->AddMovementInput(movementDirection, movementSpeed);
}

/// <summary>
/// Add some velocity to the agent if they keep going forward
/// </summary>
/// <param name="acceleration"> The AI's acceleration </param>
/// <param name="deltaTime"> Time between two frames </param>
void ASDTAIController::ChangeAISpeed(float acceleration, float deltaTime)
{
    movementSpeed = FMath::Clamp((movementSpeed + acceleration * deltaTime), 0.1f, 1.f);
}

/// <summary>
/// Rotate the AI to the new direction
/// </summary>
/// <param name="direction"> Direction to rotate </param>
void ASDTAIController::RotateAI(FVector direction)
{
    GetPawn()->SetActorRotation(direction.ToOrientationQuat());
}

/// <summary>
/// Verify if the AI is near a death zone 
/// </summary>
/// <returns>
///     True: Return true if the AI is near the death zone and visible 
///     False: Return False if the AI is not near the death zone or it's not visible
/// </returns>
bool ASDTAIController::IsDeathZoneAhead()
{
    APawn* const pawn = GetPawn();
    UWorld* const world = GetWorld();
    TArray<FHitResult> hitResults = CollectActorsInFOV();

    FHitResult isNearAI;
    FCollisionObjectQueryParams objectQueryParams(FCollisionObjectQueryParams::AllObjects);
    FCollisionQueryParams queryParams = FCollisionQueryParams::DefaultQueryParam;
    queryParams.AddIgnoredActor(GetPawn());

    for (auto& hit : hitResults) {
        world->LineTraceSingleByObjectType(isNearAI, pawn->GetActorLocation(), hit.GetActor()->GetActorLocation(), objectQueryParams, queryParams);
        DrawDebugLine(GetWorld(), pawn->GetActorLocation(), hit.GetActor()->GetActorLocation(), FColor::Blue, false, 0.1f);
        if (hit.GetComponent()->GetCollisionObjectType() == ECC_GameTraceChannel3 && isNearAI.GetComponent()->GetCollisionObjectType() == ECC_GameTraceChannel3)
            return true;
    }
    return false;
}

/// <summary>
/// Verify if the player is visible to the AI
/// </summary>
/// <returns>
///     True: The player is visible 
///     False The player is behind a wall
/// </returns>
bool ASDTAIController::IsPlayerVisible()
{

    FVector firstPlayerPosition = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();
    FVector viewDirection = GetPawn()->GetActorRotation().Vector();
    FVector playerDirection = firstPlayerPosition - GetPawn()->GetActorLocation();
    int playerDistance = playerDirection.Size();
    
    // verify if player is within the view distance and view angle
    if (playerDistance > viewDistance)
        return false;
    int angle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(viewDirection, playerDirection)));
    
    if (FMath::Abs(angle) <= viewAngle) {
       // raycast to see if the player is visible

        FVector Start = GetPawn()->GetActorLocation();
        FHitResult HitResult;
        FCollisionObjectQueryParams ObjectQueryParams(FCollisionObjectQueryParams::AllObjects);
        FCollisionQueryParams QueryParams = FCollisionQueryParams::DefaultQueryParam;
        QueryParams.AddIgnoredActor(GetPawn());

        GetWorld()->LineTraceSingleByObjectType(HitResult, Start, firstPlayerPosition, ObjectQueryParams, QueryParams);
        DrawDebugLine(GetWorld(), Start, firstPlayerPosition, FColor::Orange, false, 0.1f);
        return HitResult.GetComponent()->GetCollisionObjectType() == ECC_GameTraceChannel4;
    }
    return false;
}


/// <summary>
/// TODO
/// </summary>
/// <returns></returns>
FVector ASDTAIController::GetCollectibleDirection()
{
    APawn* const pawn = GetPawn();
    UWorld* const world = GetWorld();
    TArray<FHitResult> HitResults = CollectActorsInFOV();

    TArray<FHitResult> OutHits;
    FHitResult outHit;

    FCollisionShape HitBox;
    HitBox = FCollisionShape::MakeBox(FVector(viewDistance, viewDistance, 50));

    FVector SweepStart = pawn->GetActorLocation();
    FVector SweepEnd = pawn->GetActorLocation() + pawn->GetActorForwardVector() * viewDistance;

    //Debug
    //DrawDebugBox(world, SweepEnd, HitBox.GetExtent(), FColor::Green, false, 0.1f, 0, 5.0f);
    bool isCollectibleHit = GetWorld()->SweepMultiByObjectType(OutHits, SweepStart, SweepEnd, FQuat::Identity, COLLISION_COLLECTIBLE, HitBox);

    if (isCollectibleHit)
    {
        for (auto& Hit : OutHits)
        {
            FVector const toTarget = FVector(FVector2D(Hit.Actor->GetActorLocation() - pawn->GetActorLocation()), 0.0f);

            bool canSee = !world->LineTraceSingleByObjectType(outHit, pawn->GetActorLocation(), Hit.Actor->GetActorLocation(), ECC_WorldStatic) &&
                (std::acos(FVector::DotProduct(pawn->GetActorForwardVector().GetSafeNormal(), toTarget.GetSafeNormal()))) < PI / 3.0f;
            ASDTCollectible* collectible = dynamic_cast<ASDTCollectible*>(Hit.GetActor());

            if (canSee && collectible->GetStaticMeshComponent()->IsVisible())
            {
                //UE_LOG(LogTemp, Warning, TEXT("The vector value is: %s"), *toTarget.ToString());
                return toTarget;
            }

        }
    }
    return FVector(0, 0, 0); // Valeur retournée dans le cas où aucune balle n'est détectée

}

/// <summary>
/// Collect all the object in the AI FOV
/// </summary>
/// <returns>
/// Return an Array of HitResult with all the objects in the AI's FOV
/// </returns>
TArray<FHitResult> ASDTAIController::CollectActorsInFOV()
{
    APawn* const pawn = GetPawn();
    UWorld* const world = GetWorld();
    TArray<FHitResult> OverlapResults = CollectTargetActorsInFrontOfCharacter();
    
    DrawDebugCone(world, pawn->GetActorLocation(), pawn->GetActorRotation().Vector(), viewDistance, FMath::DegreesToRadians(viewAngle),
                  FMath::DegreesToRadians(viewAngle), 24, FColor::Green);
    
    return OverlapResults.FilterByPredicate([world, pawn, this](FHitResult HitResult)
    {
        /*DrawDebugDirectionalArrow(world, HitResult.ImpactPoint,
                                  HitResult.ImpactPoint + FVector(0, 0, 200.0f), 20,
                                  FColor::Yellow,
                                  false, 1.0f, 0, 10);*/
        
        return IsInsideCone(HitResult.ImpactPoint) && (HitResult.ImpactPoint - pawn->GetActorLocation()).Size();
    });
}

/// <summary>
/// Collect all the object in front of the AI
/// </summary>
/// <returns>
///     An array of hitResults  
/// </returns>
TArray<FHitResult> ASDTAIController::CollectTargetActorsInFrontOfCharacter()
{
    APawn* const pawn = GetPawn();
    UWorld* const world = GetWorld();
    const float radius = viewDistance * FMath::Atan(FMath::DegreesToRadians(viewAngle));

    FCollisionShape CollisionShape;
    TArray<FHitResult> HitResults;

    CollisionShape.SetSphere(radius);
    FCollisionQueryParams QueryParams = FCollisionQueryParams::DefaultQueryParam;
    QueryParams.AddIgnoredActor(pawn);
    FVector Pos = pawn->GetActorLocation() + pawn->GetActorForwardVector() * viewDistance;

    world->SweepMultiByObjectType(HitResults, pawn->GetActorLocation(), Pos,
                                  FQuat::Identity, FCollisionObjectQueryParams::AllObjects,
                                  CollisionShape, QueryParams);


    DrawDebugSphere(world, pawn->GetActorLocation(), radius, 12, FColor::Silver);
    DrawDebugSphere(world, Pos, radius, 12, FColor::Silver);

    return HitResults;
}

/// <summary>
/// Determine if an object position is inside the AI's cone vision 
/// </summary>
/// <param name="Point"> Object's position </param>
/// <returns>
///     Boolean to tell if the object is inside the AI's cone vision
///     True : It's inside 
///     False: It's outside
/// </returns>
bool ASDTAIController::IsInsideCone(FVector const Point)
{
    FVector const toTarget = Point - GetPawn()->GetActorLocation();
    FVector const pawnForward = GetPawn()->GetActorForwardVector();

    return std::abs(std::acos(FVector::DotProduct(pawnForward.GetSafeNormal(), toTarget.GetSafeNormal()))) < FMath::DegreesToRadians(viewAngle) && toTarget.
        Size() < viewDistance;
}

/// <summary>
/// Wall Detection to help the AI to navigate in the maze 
/// </summary>
/// <param name="pawn"> AI Pawn </param>
/// <param name="world"> World level </param>
/// <param name="start"> AI pawn position </param>
/// <param name="end"> Max vision of the AI </param>
/// <returns> 
///     Boolean to tell if the raycast hit a wall or not
///     True : Hit a wall
///     False: Hit nothing 
/// </returns>
bool ASDTAIController::IsGonnaHitWall(APawn* pawn, UWorld* world, FVector start, FVector end) 
{
    FCollisionObjectQueryParams objectQueryParamsWall;
    objectQueryParamsWall.AddObjectTypesToQuery(ECC_WorldStatic);
    FHitResult hitResult;
    FCollisionQueryParams queryParams = FCollisionQueryParams();
    queryParams.AddIgnoredActor(pawn);

    return world->LineTraceSingleByObjectType(hitResult, start, end, objectQueryParamsWall, queryParams);
}

void ASDTAIController::NavigateToPlayer(float deltaTime) {
    //check if powered_up
    ASoftDesignTrainingMainCharacter* mainCharacter = static_cast<ASoftDesignTrainingMainCharacter*>(GetWorld()->GetFirstPlayerController()->GetCharacter());
    if (mainCharacter->IsPoweredUp())
    {
        // main character is powerd-up, run
    }
    else
    {
        // main character is not powered-up, attack
    }
}

void ASDTAIController::NavigateToCollectible(float deltaTime) {
    RotateAI(GetCollectibleDirection().GetSafeNormal() * 0.2f + GetPawn()->GetActorForwardVector().GetSafeNormal() * 0.8f);
}

/// <summary>
///  Logic for the AI's navigation in the maze
/// </summary>
/// <param name="pawn"> AI pawn </param>
/// <param name="world"> World </param>
/// <param name="deathTrapAhead"> Boolean to know  if a death trap is ahead or not </param>
/// <param name="deltaTime"> time between two frames </param>
void ASDTAIController::Navigation(APawn* pawn, UWorld* world, bool deathTrapAhead, float deltaTime) {

    //Check if collision within a angle of 30 degrees 
    bool isHitFront = IsGonnaHitWall(pawn, world, pawn->GetActorLocation(), pawn->GetActorLocation() + wallDetectionDistance * pawn->GetActorForwardVector());
    bool isHitRight = IsGonnaHitWall(pawn, world, pawn->GetActorLocation(), pawn->GetActorLocation() + wallDetectionDistance * pawn->GetActorForwardVector().RotateAngleAxis(15, FVector(0, 0, 1)));
    bool isHitLeft = IsGonnaHitWall(pawn, world, pawn->GetActorLocation(), pawn->GetActorLocation() + wallDetectionDistance * pawn->GetActorForwardVector().RotateAngleAxis(-15, FVector(0, 0, 1)));

    //A enlever quand c'est finie ou mettre une option pour afficher les debugs line
    DrawDebugLine(world, pawn->GetActorLocation(),
        pawn->GetActorLocation() + wallDetectionDistance * pawn->GetActorForwardVector(), FColor::Blue, false, 0.1f);
    DrawDebugLine(world, pawn->GetActorLocation(),
        pawn->GetActorLocation() + wallDetectionDistance * pawn->GetActorForwardVector().RotateAngleAxis(15, FVector(0, 0, 1)), FColor::Blue, false, 0.1f);
    DrawDebugLine(world, pawn->GetActorLocation(),
        pawn->GetActorLocation() + wallDetectionDistance * pawn->GetActorForwardVector().RotateAngleAxis(-15, FVector(0, 0, 1)), FColor::Blue, false, 0.1f);

    if (isHitFront || isHitRight || isHitLeft || deathTrapAhead)
    {
        FVector newPawnDirection;
        //Change direction if collide 
        if (deathTrapAhead) {
            newPawnDirection = FVector(FVector::CrossProduct(FVector::UpVector, pawn->GetActorRightVector()));
        }
        else if (isHitRight) {
            newPawnDirection = FVector(FVector::CrossProduct(pawn->GetActorForwardVector(), FVector::UpVector));
        }
        else if (isHitLeft) {
            newPawnDirection = FVector(FVector::CrossProduct(FVector::UpVector, pawn->GetActorForwardVector()));
        }
        else if (isHitFront) {
            if (FMath::RandBool()) {
                newPawnDirection = FVector(FVector::CrossProduct(FVector::UpVector, pawn->GetActorForwardVector()));
            }
            else {
                newPawnDirection = FVector(FVector::CrossProduct(pawn->GetActorForwardVector(), FVector::UpVector));
            }
        }
        newPawnDirection.Normalize();
        if(deathTrapAhead)
            pawn->SetActorRotation(newPawnDirection.Rotation());
        else
            pawn->SetActorRotation(FMath::Lerp(pawn->GetActorRotation(), newPawnDirection.Rotation(), 0.05f));
        ChangeAISpeed(decelerationSpeed, deltaTime);
    }
}

