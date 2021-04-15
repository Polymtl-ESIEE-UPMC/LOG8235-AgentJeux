// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "SDTAIController.h"
#include "CoreMinimal.h"

/**
 * 
 */
class SOFTDESIGNTRAINING_API SDTAIAgentGroupManager
{
public:
    static SDTAIAgentGroupManager* GetInstance();
    static void Destroy();

    void RegisterAIAgent(ASDTAIController* aiAgent);
    void UnregisterAIAgent(ASDTAIController* aiAgent);
    void DrawSphere();
    void GenerateSurroundingPoints();

private:

    //SINGLETON
    SDTAIAgentGroupManager();
    static SDTAIAgentGroupManager* m_Instance;

    TArray<ASDTAIController*> m_registeredAgents;
    uint64 m_lastFrameUpdatePoint;
    TArray<FVector> m_surroundingPoints;
};
