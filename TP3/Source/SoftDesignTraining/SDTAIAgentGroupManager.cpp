// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTBaseAIController.h"
#include "SDTAIAgentGroupManager.h"
#include "DrawDebugHelpers.h"

SDTAIAgentGroupManager* SDTAIAgentGroupManager::m_Instance;

SDTAIAgentGroupManager::SDTAIAgentGroupManager()
{
}

SDTAIAgentGroupManager* SDTAIAgentGroupManager::GetInstance()
{
    if (!m_Instance)
    {
        m_Instance = new SDTAIAgentGroupManager();
    }

    return m_Instance;
}

void SDTAIAgentGroupManager::Destroy()
{
    delete m_Instance;
    m_Instance = nullptr;
}

void SDTAIAgentGroupManager::RegisterAIAgent(ASDTBaseAIController* aiAgent)
{
    DrawDebugSphere(aiAgent->GetWorld(), aiAgent->GetPawn()->GetActorLocation(), 100, 25, FColor::Green, 0, 5);
    m_registeredAgents.Add(aiAgent);
}

void SDTAIAgentGroupManager::UnregisterAIAgent(ASDTBaseAIController* aiAgent)
{
    m_registeredAgents.Remove(aiAgent);
}