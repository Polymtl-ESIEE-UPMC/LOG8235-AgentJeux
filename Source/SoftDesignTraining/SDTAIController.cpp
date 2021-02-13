// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTAIController.h"
#include "SDTCollectible.h"
#include "SoftDesignTraining.h"
#include "SoftDesignTrainingMainCharacter.h"
#include "DrawDebugHelpers.h"
#include "SDTUtils.h"

void ASDTAIController::Tick(float deltaTime)
{ 

    if (GetWorld()->GetFirstPlayerController()->GetPawn() && IsPlayerVisible())
    {
        NavigateToPlayer();
    }
    // Detect PickUp
    else if (GetCollectibleDirection() != FVector(0, 0, 0))
    {
        NavigateToCollectible();
    }
    else {
        Navigate();
    }
    ChangeAISpeed(acceleration, deltaTime);
    FVector viewDirection = GetPawn()->GetActorRotation().Vector();
    MoveAI(viewDirection);
    DisplayTestInformation(deltaTime);
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

void ASDTAIController::RotateAI(FRotator rotator)
{
    GetPawn()->SetActorRotation(rotator);
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
        if(displayDebugLines)
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
        if (displayDebugLines)
            DrawDebugLine(GetWorld(), Start, firstPlayerPosition, FColor::Orange, false, 0.1f);
        return HitResult.GetComponent()->GetCollisionObjectType() == ECC_GameTraceChannel4;
    }
    return false;
}

/// <summary>
///     Checks if collectible is visible to the ageng
/// </summary>
/// <param name="newDirection"> The Direction towards the collectible </param>
/// <param name="outHit"> First blocking Hit found </param>
/// <param name="Hit"> Collectible being checked </param>
/// <returns>
///     Boolean to tell if the object is inside visible to the AI
///     True : It's visible
///     False: It's not visible 
///</returns>
bool ASDTAIController::isCollectibleVisible(FVector newDirection, FHitResult outHit, FHitResult Hit)
{

    APawn* const pawn = GetPawn();
    UWorld* const world = GetWorld();

    return !world->LineTraceSingleByObjectType(outHit, pawn->GetActorLocation(), Hit.Actor->GetActorLocation(), ECC_WorldStatic) &&
        (std::acos(FVector::DotProduct(pawn->GetActorForwardVector().GetSafeNormal(), newDirection.GetSafeNormal()))) < PI / 3.0f;

}

/// <summary>
///     Identifies collectible in front of agent and if visible, rotates agent towards collectible
/// </summary>
/// <returns>
///     a new direction vector if collectible found else a null vector
///</returns>
FVector ASDTAIController::GetCollectibleDirection()
{
    APawn* const pawn = GetPawn();

    TArray<FHitResult> OutHits;
    FHitResult outHit;

    FCollisionShape DetectionBox;
    DetectionBox = FCollisionShape::MakeBox(FVector(viewDistance, viewDistance, 50));

    FVector Start = pawn->GetActorLocation();
    FVector End = pawn->GetActorLocation() + pawn->GetActorForwardVector() * viewDistance;

    bool isCollectibleFound = GetWorld()->SweepMultiByObjectType(OutHits, Start, End, FQuat::Identity, COLLISION_COLLECTIBLE, DetectionBox);

    if (isCollectibleFound)
    {
        for (auto& Hit : OutHits)
        {
            FVector const newDirection = FVector(FVector2D(Hit.Actor->GetActorLocation() - pawn->GetActorLocation()), 0.0f);
            bool visible = isCollectibleVisible(newDirection, outHit, Hit);
            ASDTCollectible* collectible = dynamic_cast<ASDTCollectible*>(Hit.GetActor());

            if (visible && collectible->GetStaticMeshComponent()->IsVisible())
            {
                return newDirection;
            }
        }
    }
    return FVector(0, 0, 0); // Valeur retourn�e dans le cas o� aucune balle n'est d�tect�e
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
    
    if (displayDebugLines)
        DrawDebugCone(world, pawn->GetActorLocation(), pawn->GetActorRotation().Vector(), viewDistance, FMath::DegreesToRadians(viewAngle),
                  FMath::DegreesToRadians(viewAngle), 24, FColor::Green);
    
    return OverlapResults.FilterByPredicate([world, pawn, this](FHitResult HitResult)
    {     
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

    if (displayDebugLines) {
        DrawDebugSphere(world, pawn->GetActorLocation(), radius, 12, FColor::Silver);
        DrawDebugSphere(world, Pos, radius, 12, FColor::Silver);
    }

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
bool ASDTAIController::IsGonnaHitWall(FVector end) 
{
    FCollisionObjectQueryParams objectQueryParamsWall;
    objectQueryParamsWall.AddObjectTypesToQuery(ECC_WorldStatic);
    FHitResult hitResult;
    FCollisionQueryParams queryParams = FCollisionQueryParams();
    queryParams.AddIgnoredActor(GetPawn());

    return GetWorld()->LineTraceSingleByObjectType(hitResult, GetPawn()->GetActorLocation(), end, objectQueryParamsWall, queryParams);
}

/// <summary>
/// Behavior when player is detected. If powered up, flee and otherwise attack
/// </summary>
/// <param name="pawn"></param>
/// <param name="deltaTime"></param>
void ASDTAIController::NavigateToPlayer() {
    //check if powered_up
    ASoftDesignTrainingMainCharacter* mainCharacter = static_cast<ASoftDesignTrainingMainCharacter*>(GetWorld()->GetFirstPlayerController()->GetCharacter());
    if (mainCharacter->IsPoweredUp())
    {
        // main character is powerd-up, run
        acceleration = decelerationSpeed;
        FVector newPawnDirection;
        newPawnDirection = FVector(FVector::CrossProduct(FVector::UpVector, GetPawn()->GetActorRightVector()));
        newPawnDirection.Normalize();
        RotateAI(newPawnDirection.Rotation());
    }
    else
    {
        // main character is not powered-up, attack
        acceleration = accelerationSpeed;
        FVector newPawnDirection;
        newPawnDirection = FVector(FVector2D(mainCharacter->GetActorLocation() - GetPawn()->GetActorLocation()), 0.0f);
        newPawnDirection.Normalize();
        RotateAI(newPawnDirection.Rotation());
    }
}

void ASDTAIController::NavigateToCollectible() {
    acceleration = accelerationSpeed;
    RotateAI(GetCollectibleDirection().GetSafeNormal() * 0.2f + GetPawn()->GetActorForwardVector().GetSafeNormal() * 0.8f);
}

/// <summary>
///  Logic for the AI's navigation in the maze
/// </summary>
/// <param name="pawn"> AI pawn </param>
/// <param name="world"> World </param>
/// <param name="deathTrapAhead"> Boolean to know  if a death trap is ahead or not </param>
/// <param name="deltaTime"> time between two frames </param>
void ASDTAIController::Navigate() {

    APawn* const pawn = GetPawn();
    UWorld* const world = GetWorld();

    if (displayDebugLines){
        DrawDebugLine(world, pawn->GetActorLocation(),
            pawn->GetActorLocation() + wallDetectionDistance * pawn->GetActorForwardVector(), FColor::Blue, false, 0.1f);
        DrawDebugLine(world, pawn->GetActorLocation(),
            pawn->GetActorLocation() + wallDetectionDistance * pawn->GetActorForwardVector().RotateAngleAxis(15, FVector(0, 0, 1)), FColor::Blue, false, 0.1f);
        DrawDebugLine(world, pawn->GetActorLocation(),
            pawn->GetActorLocation() + wallDetectionDistance * pawn->GetActorForwardVector().RotateAngleAxis(-15, FVector(0, 0, 1)), FColor::Blue, false, 0.1f);
    }

    //Check if collision within a angle of 30 degrees 
    bool isHitFront = IsGonnaHitWall(pawn->GetActorLocation() + wallDetectionDistance * pawn->GetActorForwardVector());
    bool isHitRight = IsGonnaHitWall(pawn->GetActorLocation() + wallDetectionDistance * pawn->GetActorForwardVector().RotateAngleAxis(15, FVector(0, 0, 1)));
    bool isHitLeft = IsGonnaHitWall(pawn->GetActorLocation() + wallDetectionDistance * pawn->GetActorForwardVector().RotateAngleAxis(-15, FVector(0, 0, 1)));
    bool isDeathZoneAhead = IsDeathZoneAhead();

    if (isHitFront || isHitRight || isHitLeft || isDeathZoneAhead)
    {
        acceleration = decelerationSpeed;
        FVector newPawnDirection;
        //Change direction if collide 
        if (isDeathZoneAhead) {
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
        if(isDeathZoneAhead)
            RotateAI(newPawnDirection.Rotation());
        else
            RotateAI(FMath::Lerp(pawn->GetActorRotation(), newPawnDirection.Rotation(), 0.05f));
    }
    else {
        acceleration = accelerationSpeed;
    }
}


/// <summary>
/// Increment the AI'sDeath count
/// </summary>
void ASDTAIController::IncrementAIDeathCount() {
	m_numberAIDeath++;
}

/// <summary>
/// Increment the AI's Pick up count
/// </summary>
void ASDTAIController::IncrementAICollectibleCount() {
	m_numberPickUp++;
}

/// <summary>
/// Display information about the AI
/// </summary>
/// <param name="delatTime"> Time between two frames </param>
void ASDTAIController::DisplayTestInformation(float delatTime) {

	//Show stats and verify if GEngine is not 0
	if (GEngine) {
		GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Green, FString::Printf(TEXT("------------------------------"), *(GetPawn()->GetName())));
		GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Red, FString::Printf(TEXT("Total of AI's Death: %s"), *FString::FromInt(m_numberAIDeath)));
		GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Red, FString::Printf(TEXT("Total of AI's pickup: %s"), *FString::FromInt(m_numberPickUp)));
		GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Red, FString::Printf(TEXT("Timer: %s"), *FString::FromInt(m_timer)));
		GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Green, FString::Printf(TEXT("------------- %s -------------"), *(GetPawn()->GetName())));

		m_timer += delatTime;

		//Reset stats if Period is over
		if (m_timer >= m_testPeriod) {
			m_numberAIDeath = 0;
			m_numberPickUp = 0;
			m_timer = 0;
		}
	}
}

