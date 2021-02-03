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
    void AddAiMovement(APawn* pawn, FVector directionVector);
    bool isPlayerVisible(APawn* pawn, FVector playerPosition, FVector viewDirection);
    bool DetectDeathZone(APawn* pawn, UWorld* World, FVector viewDirection);
    TArray<FHitResult> CollectVisibleElements(APawn* pawn, UWorld* World, FVector viewDirection);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
    float movementSpeed = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
    float viewDistance = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
    float viewAngle = 40;
};
