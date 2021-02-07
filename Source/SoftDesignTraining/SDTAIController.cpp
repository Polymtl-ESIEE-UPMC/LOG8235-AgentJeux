// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTAIController.h"
#include "SDTCollectible.h"
#include "SoftDesignTraining.h"
#include "SoftDesignTrainingMainCharacter.h"
#include "DrawDebugHelpers.h"
#include "SDTUtils.h"

void ASDTAIController::Tick(float deltaTime)
{
    APawn* const pawn = GetPawn();
    UWorld* const world = GetWorld();

    FVector viewDirection = pawn->GetActorRotation().Vector();

    TArray<FHitResult> HitResults = CollectActorsInFOV(pawn, world);

    bool deathZoneAhead = DetectDeathZone(pawn, world, viewDirection);

    APawn* playerPawn = world->GetFirstPlayerController()->GetPawn();
    float angle = 0;

    Navigate(pawn, world, deltaTime);

    if (deathZoneAhead)
    {
        // do something
    }
    else if (playerPawn && isPlayerVisible(pawn, playerPawn->GetActorLocation(), viewDirection))
    {
        //check if powered_up
        ASoftDesignTrainingMainCharacter* mainCharacter = static_cast<ASoftDesignTrainingMainCharacter*>(world->GetFirstPlayerController()->GetCharacter());
        if (mainCharacter->IsPoweredUp())
        {
            // main character is powerd-up, run
        }
        else
        {
            // main character is not powered-up, attack
        }
    }
    else
    {
        /*
        float minDist = -1;
        FHitResult ClosestHit;

        for (int i = 0; i < HitResults.Num(); ++i)
        {
            TArray<FHitResult>::ElementType HitResult = HitResults[i];
            float dist = (HitResult.ImpactPoint - pawn->GetActorLocation()).Size();
            if (minDist == -1 || dist < minDist)
            {
                minDist = dist;
                ClosestHit = HitResult;
            }
            
            DrawDebugDirectionalArrow(World, HitResult.ImpactPoint,
                                      HitResult.ImpactPoint + FVector(0, 0, 200.0f), 20,
                                      FColor::Blue,
                                      false, 1.0f, 0, 20);
        }


        if (ClosestHit.GetComponent() != nullptr)
        {

            FVector_NetQuantize ImpactPoint = ClosestHit.ImpactPoint;
            FVector toTarget = ImpactPoint - pawn->GetActorLocation();

            float res = toTarget.ToOrientationRotator().Yaw - pawn->GetActorRotation().Yaw;

            float delta = (res / FMath::Abs(res)) * 1; //todo: Make UProperty Rotate speed

            if (ClosestHit.GetComponent()->GetCollisionObjectType() == COLLISION_COLLECTIBLE)
            {
                angle = delta;
            }

            if (ClosestHit.GetComponent()->GetCollisionObjectType() == ECC_WorldStatic)
            {
                angle = -delta;
            }
        }

        DrawDebugDirectionalArrow(World, ClosestHit.ImpactPoint,
                                  ClosestHit.ImpactPoint + FVector(0, 0, 200.0f), 20,
                                  FColor::Green,
                                  false, 1.0f, 0, 50);*/

    }
}

bool ASDTAIController::isPlayerVisible(APawn* pawn, FVector playerPosition, FVector viewDirection)
{
    FVector playerDirection = playerPosition - pawn->GetActorLocation();
    int playerDistance = playerDirection.Size();
    
    // verify if player is within the view distance and view angle
    if (playerDistance > viewDistance)
        return false;
    int angle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(viewDirection, playerDirection)));

    
    if (FMath::Abs(angle) <= viewAngle) {
       // raycast to see if the player is visible

        FVector Start = pawn->GetActorLocation();
        FHitResult HitResult;
        FCollisionObjectQueryParams ObjectQueryParams(FCollisionObjectQueryParams::AllObjects);
        FCollisionQueryParams QueryParams = FCollisionQueryParams::DefaultQueryParam;
        QueryParams.AddIgnoredActor(pawn);

        GetWorld()->LineTraceSingleByObjectType(HitResult, Start, playerPosition, ObjectQueryParams, QueryParams);
        DrawDebugLine(GetWorld(), Start, playerPosition, FColor::Orange, false, 0.1f);
        return HitResult.GetComponent()->GetCollisionObjectType() == ECC_GameTraceChannel4;
    }
    return false;
}

bool ASDTAIController::isGonnaHitWall(APawn const* pawn, UWorld const* world, FCollisionObjectQueryParams objectQueryParamsWall, FHitResult wallHit, FCollisionQueryParams queryParams, int side)
{
    switch (side) {
        case RIGHT_SIDE: 
        {
            FVector wallDetectionRight;
            wallDetectionRight = pawn->GetActorLocation() + wallDetectionDistance * pawn->GetActorRightVector();
            return world->LineTraceSingleByObjectType(wallHit, pawn->GetActorLocation(), wallDetectionRight, objectQueryParamsWall, queryParams);
        }
        break;
        case LEFT_SIDE: 
        {
            FVector wallDetectionLeft;
            wallDetectionLeft = pawn->GetActorLocation() + wallDetectionDistance * pawn->GetActorRightVector() * -1.0f;
            return world->LineTraceSingleByObjectType(wallHit, pawn->GetActorLocation(), wallDetectionLeft, objectQueryParamsWall, queryParams);
        }
        break;
        case FRONTSIDE: 
        {
            FVector wallDetectionForward;
            wallDetectionForward = pawn->GetActorLocation() + wallDetectionDistance * pawn->GetActorForwardVector();
            return world->LineTraceSingleByObjectType(wallHit, pawn->GetActorLocation(), wallDetectionForward, objectQueryParamsWall, queryParams);
        }
        break;
        case BACKSIDE: 
        {
            FVector wallDetectionBack;
            wallDetectionBack = pawn->GetActorLocation() + wallDetectionDistance * pawn->GetActorForwardVector() * -1.0f;
            return world->LineTraceSingleByObjectType(wallHit, pawn->GetActorLocation(), wallDetectionBack, objectQueryParamsWall, queryParams);
        }
        break;
    }
    return false;
}

bool ASDTAIController::DetectDeathZone(APawn* pawn, UWorld* World, FVector viewDirection)
{
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
void ASDTAIController::AddAIMovement(APawn* pawn, FVector movementDirection)
{
    // Adding movement 
    movementDirection.Normalize();
    pawn->AddMovementInput(movementDirection, movementSpeed);
}

void ASDTAIController::AIChangingSpeed(float acceleration, float deltaTime)
{
    movementSpeed = FMath::Clamp((movementSpeed + acceleration * deltaTime), 0.1f, 1.f);
}

void ASDTAIController::Rotate(APawn* pawn, FVector direction) 
{
    FVector forwardVector = pawn->GetActorForwardVector();
    direction.Normalize();
    FVector newRotation = (direction - forwardVector);
    newRotation.Normalize();
    pawn->SetActorRotation(FMath::Lerp(forwardVector.ToOrientationRotator(), newRotation.ToOrientationRotator(), 0.05f));
    
    if (forwardVector.Equals(direction, 0.6f))
        m_isRotating = false;
}

TArray<FHitResult> ASDTAIController::CollectActorsInFOV(APawn const* pawn, UWorld const* World) const
{
    TArray<FHitResult> OverlapResults = CollectTargetActorsInFrontOfCharacter(pawn, World);
    
    DrawDebugCone(World, pawn->GetActorLocation(), pawn->GetActorRotation().Vector(), viewDistance, FMath::DegreesToRadians(viewAngle),
                  FMath::DegreesToRadians(viewAngle), 24, FColor::Green);
    
    return OverlapResults.FilterByPredicate([World, pawn, this](FHitResult HitResult)
    {
        DrawDebugDirectionalArrow(World, HitResult.ImpactPoint,
                                  HitResult.ImpactPoint + FVector(0, 0, 200.0f), 20,
                                  FColor::Yellow,
                                  false, 1.0f, 0, 10);
        
        return IsInsideCone(pawn, HitResult.ImpactPoint) && (HitResult.ImpactPoint - pawn->GetActorLocation()).Size();
    });
}

TArray<FHitResult> ASDTAIController::CollectTargetActorsInFrontOfCharacter(APawn const* pawn, UWorld const* World) const
{
    const float radius = viewDistance * FMath::Atan(FMath::DegreesToRadians(viewAngle));

    FCollisionShape CollisionShape;
    TArray<FHitResult> HitResults;

    CollisionShape.SetSphere(radius);
    FCollisionQueryParams QueryParams = FCollisionQueryParams::DefaultQueryParam;
    QueryParams.AddIgnoredActor(pawn);
    FVector Pos = pawn->GetActorLocation() + pawn->GetActorForwardVector() * viewDistance;

    World->SweepMultiByObjectType(HitResults, pawn->GetActorLocation(), Pos,
                                  FQuat::Identity, FCollisionObjectQueryParams::AllObjects,
                                  CollisionShape, QueryParams);


    DrawDebugSphere(World, pawn->GetActorLocation(), radius, 12, FColor::Silver);
    DrawDebugSphere(World, Pos, radius, 12, FColor::Silver);

    return HitResults;
}

bool ASDTAIController::IsInsideCone(APawn const* pawn, FVector const Point) const
{
    FVector const toTarget = Point - pawn->GetActorLocation();
    FVector const pawnForward = pawn->GetActorForwardVector();

    return std::abs(std::acos(FVector::DotProduct(pawnForward.GetSafeNormal(), toTarget.GetSafeNormal()))) < FMath::DegreesToRadians(viewAngle) && toTarget.
        Size() < viewDistance;
}

void ASDTAIController::Navigate(APawn* pawn, UWorld* world, float deltaTime) {

    FCollisionObjectQueryParams objectQueryParamsWall;
    objectQueryParamsWall.AddObjectTypesToQuery(ECollisionChannel::ECC_WorldStatic);
    FHitResult wallhit;
    FCollisionQueryParams queryParams = FCollisionQueryParams();
    queryParams.AddIgnoredActor(pawn);

    if (!m_isRotating) {
        // Verify if the AI will hit a wall or a death trap if it move forward 
        if (!isGonnaHitWall(pawn, world, objectQueryParamsWall, wallhit, queryParams, FRONTSIDE) && !DetectDeathZone(pawn, world, pawn->GetActorRotation().Vector())) {
            AIChangingSpeed(accelerationSpeed, deltaTime);
            AddAIMovement(pawn, pawn->GetActorRotation().Vector());
        }
        // Verify if the AI will hit a wall if it move to the right  
        else if (!isGonnaHitWall(pawn, world, objectQueryParamsWall, wallhit, queryParams, RIGHT_SIDE)) {
            m_newRotatingDirection = FVector(1.f, 1.f, 0.f);
            m_isRotating = true;
            AIChangingSpeed(accelerationSpeed * decelerationSpeed, deltaTime);
            // Rotate to the right 
            Rotate(pawn, m_newRotatingDirection);
        }
        // Verify if the AI will hit a wall if it move to the Left 
        else if (!isGonnaHitWall(pawn, world, objectQueryParamsWall, wallhit, queryParams, LEFT_SIDE)) {
            m_newRotatingDirection = FVector(-1.f, -1.f, 0.f);
            m_isRotating = true;
            AIChangingSpeed(accelerationSpeed * decelerationSpeed, deltaTime);
            // Rotate to the Left 
            Rotate(pawn, m_newRotatingDirection);
        }
        // Verify if the AI will hit a wall if it move backward 
        else if (!isGonnaHitWall(pawn, world, objectQueryParamsWall, wallhit, queryParams, BACKSIDE)) {
            m_newRotatingDirection = -1.f * pawn->GetActorForwardVector();
            m_isRotating = true;
            AIChangingSpeed(accelerationSpeed * decelerationSpeed, deltaTime);
            // Rotate to move backward 
            Rotate(pawn, m_newRotatingDirection);
        }
    }
    else {
        Rotate(pawn, m_newRotatingDirection);
    }
}
