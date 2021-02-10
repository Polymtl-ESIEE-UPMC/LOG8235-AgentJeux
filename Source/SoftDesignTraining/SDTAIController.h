// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"

#include "SDTAIController.generated.h"

#define RIGHT_SIDE  0
#define LEFT_SIDE  1
#define FRONTSIDE  2
#define BACKSIDE 3


/**
 * 
 */
UCLASS(ClassGroup = AI, config = Game)
class SOFTDESIGNTRAINING_API ASDTAIController : public AAIController
{
    GENERATED_BODY()
public:
    virtual void Tick(float deltaTime) override;
    void IncrementAIDeathCount();
    void IncrementAICollectibleCount();

    /**Movement speed of the agent. 
    Needs to be a value between 0 and 1. 
    0 : The agent doesn't move. 
    1 : The agent moves at full speed. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI, meta = (ClampMin = "0.0", ClampMax = "1.0") )
    float movementSpeed = 0.1f;

    /**Acceleration speed of the agent.
    The agent is going faster if it still going in the same direction
    0 : The agent speed is const*/
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI, meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float accelerationSpeed = 1.0f;

    /**deceleration speed of the agent when they turn right or left.
    The agent is slowing down when it will turn right or left
    0 : The agent speed doesn't change */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI, meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float decelerationSpeed = -7.0f;

    /* View distance of the agent.
    If the player is further than the view distance, the player will not be detected.
    Needs to be greater than 0.*/
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI, meta = (ClampMin = "0.0"))
    float viewDistance = 500;

    /* View angle of the agent.
    If the angle between the agent view direction and the direction to the player is bigger than the view angle, the player will not be detected.
    Needs to be between 0 and 180.
    At a value of 180, the pawn will all around him.*/
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI, meta = (ClampMin = "0.0", ClampMax = "180.0"))
    float viewAngle = 40;

    /*True: Display debug lines
    False: no debug lines*/
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
    bool displayDebugLines = false;

    /* Minimal distance for wall detection 
    If the wall is further than 175 unit, the agent won't detect it, if the wall is inside 175 unit, the agent will detect it and move accordingly
    Need to be greater than 0*/
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI, meta = (ClampMin = "0.0", ClampMax = "500.0"))
    float wallDetectionDistance = 255.0f;

    // Determine the test's lenght in sec
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI, meta = (ClampMin = "0.0", ClampMax = "180.0"))
    float m_testPeriod = 60;

private:
    void MoveAI(FVector movementDirection);
    void ChangeAISpeed(float acceleration, float deltaTime);
    void RotateAI(FVector direction);
    bool IsDeathZoneAhead();
    bool IsPlayerVisible();

    FVector GetCollectibleDirection();
    TArray<FHitResult> CollectActorsInFOV();
    TArray<FHitResult> CollectTargetActorsInFrontOfCharacter();
    bool IsInsideCone(FVector const Point);

    bool IsGonnaHitWall(APawn* pawn, UWorld* world, FVector start, FVector end);

    void NavigateToPlayer(APawn* pawn, float deltaTime);
    void NavigateToCollectible(float deltaTime);
    void Navigation(APawn* pawn, UWorld* world, bool deathTrap, float deltaTime);

    void DisplayTestInformation(float delatTime);

    // Information about the AI automatic test
    int m_numberPickUp = 0;
    int m_numberAIDeath = 0;
    float m_timer = 0.f;
};
