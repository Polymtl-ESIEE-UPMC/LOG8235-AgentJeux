// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "SDTBaseAIController.h"
#include "CoreMinimal.h"

/**
 * 
 */
class SOFTDESIGNTRAINING_API SDTAIAgentGroupManager
{
public:
    static SDTAIAgentGroupManager* GetInstance();
    static void Destroy();

    void RegisterAIAgent(ASDTBaseAIController* aiAgent);
    void UnregisterAIAgent(ASDTBaseAIController* aiAgent);

private:

    //SINGLETON
    SDTAIAgentGroupManager();
    static SDTAIAgentGroupManager* m_Instance;

    TArray<ASDTBaseAIController*> m_registeredAgents;
};
