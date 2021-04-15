// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTAIAgentGroupManager.h"
#include "SoftDesignTraining.h"
#include "SDTBaseAIController.h"
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

void SDTAIAgentGroupManager::RegisterAIAgent(ASDTAIController* aiAgent)
{
    m_registeredAgents.Add(aiAgent);
}

void SDTAIAgentGroupManager::UnregisterAIAgent(ASDTAIController* aiAgent)
{
    m_registeredAgents.Remove(aiAgent);
}

/// <summary>
/// Draw sphere over the head of each agents in the group
/// </summary>
void SDTAIAgentGroupManager::DrawSphere() {

	for (int i = 0; i < m_registeredAgents.Num(); i++)
	{
		AAIController* aicontroller = m_registeredAgents[i];
		if (aicontroller)
		{
			FVector actorLocation = aicontroller->GetPawn()->GetActorLocation();
			DrawDebugSphere(aicontroller->GetWorld(), actorLocation + FVector(0.f, 0.f, 100.f), 25.0f, 32, FColor::Green);
		}
	}
}

/// <summary>
/// Generate point for the agent around the player
/// </summary>
void SDTAIAgentGroupManager::GenerateSurroundingPoints() {
	
	//Check if there more than one agent in the group
	if (m_registeredAgents.Num() <= 1)
		return;

	// Check if the generation of points was already done in the frame by one agent
	if (m_lastFrameUpdatePoint == GFrameCounter)
		return;
	m_lastFrameUpdatePoint = GFrameCounter;

	//Reset all the surrounding points
	m_surroundingPoints.Empty();

	float circle = 2.f * PI;
	float numbersurroundingPoints = m_registeredAgents.Num();
	float angle = circle / numbersurroundingPoints;
	float radius = 150.f;
	float currentAngle = 0.f;

	AAIController* aicontroller = m_registeredAgents[0];
	ACharacter* playerCharacter = UGameplayStatics::GetPlayerCharacter(aicontroller->GetWorld(), 0);
	FVector playerPosition = playerCharacter->GetActorLocation();

	for (int i = 0; i < m_registeredAgents.Num(); i++) {
		FVector position = playerPosition;

		position.X += radius * cos(currentAngle);
		position.Y += radius * sin(currentAngle);

		m_registeredAgents[i]->SetSurroundingPoint(position);
		currentAngle += angle;

		DrawDebugSphere(aicontroller->GetWorld(), position + FVector(0.f, 0.f, 100.f), 25.0f, 32, FColor::Purple);
	}

}