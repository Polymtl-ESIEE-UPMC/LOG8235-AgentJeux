// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTAIController.h"
#include "SDTCollectible.h"
#include "SoftDesignTraining.h"
#include "SoftDesignTrainingMainCharacter.h"
#include "DrawDebugHelpers.h"
#include "SDTUtils.h"

void ASDTAIController::Tick(float deltaTime)
{
    
    if (IsDeathZoneAhead())
    {
        NavigateAvoidDeathZone(deltaTime);
    }
    else if (GetWorld()->GetFirstPlayerController()->GetPawn() && IsPlayerVisible())
    {
        NavigateToPlayer(deltaTime);
    }
    // Detect PickUp
    else if (GetCollectibleDirection() != FVector(0, 0, 0))
    {
        NavigateToCollectible(deltaTime);
    }
    else if (IsGonnaHitWall()) {
        NavigateAvoidWall(deltaTime);
    }
    else {
        // Accelerate
        ChangeAISpeed(accelerationSpeed, deltaTime);
    }
    FVector viewDirection = GetPawn()->GetActorRotation().Vector();
    MoveAI(viewDirection);
}

// Add a movement input to the pawn and set the rotation to be in the same direction
void ASDTAIController::MoveAI(FVector movementDirection)
{
    // Adding movement 
    movementDirection.Normalize();
    GetPawn()->AddMovementInput(movementDirection, movementSpeed);
}

// Add some velocity to the agent if they keep going forward 
void ASDTAIController::ChangeAISpeed(float acceleration, float deltaTime)
{
    movementSpeed = FMath::Clamp((movementSpeed + acceleration * deltaTime), 0.1f, 1.f);
}

void ASDTAIController::RotateAI(FVector direction)
{
    GetPawn()->SetActorRotation(direction.ToOrientationQuat());
}

void ASDTAIController::RotateAI(FRotator rotator)
{
    GetPawn()->SetActorRotation(rotator);
}

bool ASDTAIController::IsDeathZoneAhead()
{
    // Identification des paramètres pour les trois lancer de rayons
    FVector viewDirection = GetPawn()->GetActorRotation().Vector();
    int rayDistance = 350;
    FVector Start = GetPawn()->GetActorLocation();
    FVector End = Start + rayDistance * viewDirection.RotateAngleAxis(45, FVector(0, 1, 0));
    FHitResult HitResult;
    FCollisionObjectQueryParams ObjectQueryParams(FCollisionObjectQueryParams::AllObjects);
    FCollisionQueryParams QueryParams = FCollisionQueryParams::DefaultQueryParam;
    QueryParams.AddIgnoredActor(GetPawn());

    GetWorld()->LineTraceSingleByObjectType(HitResult, Start, End, ObjectQueryParams, QueryParams);
    DrawDebugLine(GetWorld(), Start, End, FColor::Orange, false, 0.1f);

    if (HitResult.GetComponent())
        return HitResult.GetComponent()->GetCollisionObjectType() == ECC_GameTraceChannel3;
    return false;
}

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
    DrawDebugBox(world, SweepEnd, HitBox.GetExtent(), FColor::Green, false, 0.1f, 0, 5.0f);
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
                UE_LOG(LogTemp, Warning, TEXT("The vector value is: %s"), *toTarget.ToString());
                return toTarget;
            }

        }
    }
    return FVector(0, 0, 0); // Valeur retournée dans le cas où aucune balle n'est détectée

}

TArray<FHitResult> ASDTAIController::CollectActorsInFOV()
{
    APawn* const pawn = GetPawn();
    UWorld* const world = GetWorld();
    TArray<FHitResult> OverlapResults = CollectTargetActorsInFrontOfCharacter();
    
    DrawDebugCone(world, pawn->GetActorLocation(), pawn->GetActorRotation().Vector(), viewDistance, FMath::DegreesToRadians(viewAngle),
                  FMath::DegreesToRadians(viewAngle), 24, FColor::Green);
    
    return OverlapResults.FilterByPredicate([world, pawn, this](FHitResult HitResult)
    {
        DrawDebugDirectionalArrow(world, HitResult.ImpactPoint,
                                  HitResult.ImpactPoint + FVector(0, 0, 200.0f), 20,
                                  FColor::Yellow,
                                  false, 1.0f, 0, 10);
        
        return IsInsideCone(HitResult.ImpactPoint) && (HitResult.ImpactPoint - pawn->GetActorLocation()).Size();
    });
}

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

bool ASDTAIController::IsInsideCone(FVector const Point)
{
    FVector const toTarget = Point - GetPawn()->GetActorLocation();
    FVector const pawnForward = GetPawn()->GetActorForwardVector();

    return std::abs(std::acos(FVector::DotProduct(pawnForward.GetSafeNormal(), toTarget.GetSafeNormal()))) < FMath::DegreesToRadians(viewAngle) && toTarget.
        Size() < viewDistance;
}

bool ASDTAIController::IsGonnaHitWall() {
    APawn* const pawn = GetPawn();
    UWorld* const world = GetWorld();
    FCollisionObjectQueryParams objectQueryParamsWall;
    objectQueryParamsWall.AddObjectTypesToQuery(ECC_WorldStatic);
    FHitResult hitResult;
    FCollisionQueryParams queryParams = FCollisionQueryParams();
    queryParams.AddIgnoredActor(pawn);

    //Check if collision within a angle of 30 degrees 
    return world->LineTraceSingleByObjectType
    (
        hitResult,
        pawn->GetActorLocation(),
        pawn->GetActorLocation() + wallDetectionDistance * pawn->GetActorForwardVector(),
        objectQueryParamsWall, queryParams
    ) || world->LineTraceSingleByObjectType
    (
        hitResult,
        pawn->GetActorLocation(),
        pawn->GetActorLocation() + wallDetectionDistance * pawn->GetActorForwardVector().RotateAngleAxis(15, FVector(0, 0, 1)),
        objectQueryParamsWall, queryParams
    ) || world->LineTraceSingleByObjectType
    (
        hitResult,
        pawn->GetActorLocation(),
        pawn->GetActorLocation() + wallDetectionDistance * pawn->GetActorForwardVector().RotateAngleAxis(-15, FVector(0, 0, 1)),
        objectQueryParamsWall, queryParams
    );
}

void ASDTAIController::NavigateAvoidDeathZone(float deltaTime) {
    // TODO
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

void ASDTAIController::NavigateAvoidWall(float deltaTime) {

    //A enlever quand c'est finie ou mettre une option pour afficher les debugs line
    DrawDebugLine(GetWorld(), GetPawn()->GetActorLocation(),
        GetPawn()->GetActorLocation() + wallDetectionDistance * GetPawn()->GetActorForwardVector(), FColor::Blue, false, 0.1f);
    DrawDebugLine(GetWorld(), GetPawn()->GetActorLocation(),
        GetPawn()->GetActorLocation() + wallDetectionDistance * GetPawn()->GetActorForwardVector().RotateAngleAxis(15, FVector(0, 0, 1)), FColor::Blue, false, 0.1f);
    DrawDebugLine(GetWorld(), GetPawn()->GetActorLocation(),
        GetPawn()->GetActorLocation() + wallDetectionDistance * GetPawn()->GetActorForwardVector().RotateAngleAxis(-15, FVector(0, 0, 1)), FColor::Blue, false, 0.1f);

    // A améliorer pour tourner à gauche ou à droite
    FVector newPawnDirection;
    if (1) {
        // Turn right 
        newPawnDirection = FVector(FVector::CrossProduct(FVector::UpVector, GetPawn()->GetActorForwardVector()));
    }
    else {
        // Turn Left 
        newPawnDirection = FVector(FVector::CrossProduct(GetPawn()->GetActorForwardVector(), FVector::UpVector));
    }
    //RotateAI(FMath::Lerp(GetPawn()->GetActorRotation(), newPawnDirection.Rotation(), 0.05f));
    RotateAI(newPawnDirection);
    ChangeAISpeed(decelerationSpeed, deltaTime);
}
//void ASDTAIController::Navigate(APawn* pawn, UWorld* world, float deltaTime) {
//
//    //A enlever quand c'est finie ou mettre une option pour afficher les debugs line
//    DrawDebugLine(world, pawn->GetActorLocation(),
//        pawn->GetActorLocation() + wallDetectionDistance * pawn->GetActorForwardVector(), FColor::Blue, false, 0.1f);
//
//    DrawDebugLine(world, pawn->GetActorLocation(),
//        pawn->GetActorLocation() + wallDetectionDistance * pawn->GetActorForwardVector().RotateAngleAxis(15, FVector(0, 0, 1)), FColor::Blue, false, 0.1f);
//    DrawDebugLine(world, pawn->GetActorLocation(),
//        pawn->GetActorLocation() + wallDetectionDistance * pawn->GetActorForwardVector().RotateAngleAxis(-15, FVector(0, 0, 1)), FColor::Blue, false, 0.1f);
//
//    //Change direction if collide 
//    if (isGonnaHitWall(pawn, world)) {
//        // A améliorer pour tourner à gauche ou à droite
//        if (1) {
//            // Turn right 
//            FVector newPawnDirection = FVector(FVector::CrossProduct(FVector::UpVector, pawn->GetActorForwardVector()));
//            newPawnDirection.Normalize();
//            pawn->SetActorRotation(FMath::Lerp(pawn->GetActorRotation(), newPawnDirection.Rotation(), 0.05f));
//            ChangeAISpeed(decelerationSpeed, deltaTime);
//        }
//        else {
//            // Turn Left 
//            FVector newPawnDirection = FVector(FVector::CrossProduct(pawn->GetActorForwardVector(), FVector::UpVector));
//            newPawnDirection.Normalize();
//            pawn->SetActorRotation(FMath::Lerp(pawn->GetActorRotation(), newPawnDirection.Rotation(), 0.05f));
//            ChangeAISpeed(decelerationSpeed, deltaTime);
//        }    
//    }
//    //TODO Ajouter une rotation pour éviter un deathTrap
//    else {
//        ChangeAISpeed(accelerationSpeed, deltaTime);
//    }
//      
//    
//}
