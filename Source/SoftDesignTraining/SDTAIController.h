// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"

#include "SDTAIController.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = AI, config = Game)
class SOFTDESIGNTRAINING_API ASDTAIController : public AAIController
{
    GENERATED_BODY()
public:
    virtual void Tick(float deltaTime) override;
    void AddAiMovement(APawn* pawn, FVector directionVector, float rotateAngle);
    bool isPlayerVisible(APawn* pawn, FVector playerPosition, FVector viewDirection);
    bool DetectDeathZone(APawn* pawn, UWorld* World, FVector viewDirection);
    TArray<FHitResult> CollectVisibleWalls(APawn* pawn, UWorld* World);

    TArray<FHitResult> CollectActorsInFOV(APawn const* pawn, UWorld const* World) const;
    
    TArray<FHitResult> CollectTargetActorsInFrontOfCharacter(APawn const* pawn, UWorld const* World) const;
    
    bool IsInsideCone(APawn const * pawn, FVector const Point) const;

    /** Movement speed of the agent. 
    Needs to be a value between 0 and 1. 
    0 : The agent doesn't move. 
    1 : The agent moves at full speed. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI, meta = (ClampMin = "0.0", ClampMax = "1.0") )
    float movementSpeed = 0.1f;

    /** View distance of the agent.
    If the player is further than the view distance, the player will not be detected.
    Needs to be greater than 0.*/
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI, meta = (ClampMin = "0.0"))
    float viewDistance = 500;

    /** View angle of the player.
    If the angle between the agent view direction and the direction to the player is bigger than the view angle, the player will not be detected.
    Needs to be between 0 and 180.
    At a value of 180, the pawn will all around him.*/
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI, meta = (ClampMin = "0.0", ClampMax = "180.0"))
    float viewAngle = 40;
};
