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

    void IncrementAIDeathCount();
    void IncrementAICollectibleCount();

private:

    void DisplayTestInformation(float delatTime);

    // Information about the AI automatic test
    int m_numberPickUp = 0;
    int m_numberAIDeath = 0;
    float m_timer = 0.f;

    // Determine the test's lenght in sec
    UPROPERTY(EditAnywhere)
        float m_testPeriod = 60;
};
