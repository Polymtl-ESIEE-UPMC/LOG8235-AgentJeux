// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTAIController.h"
#include "SoftDesignTraining.h"
#include "UnrealEd.h"
#include "Editor/EditorEngine.h"

void ASDTAIController::Tick(float deltaTime)
{
	DisplayTestInformation(deltaTime);
}

void ASDTAIController::IncrementAIDeathCount() {
	m_numberAIDeath++;
}

void ASDTAIController::IncrementAICollectibleCount() {
	m_numberPickUp++;
}

void ASDTAIController::DisplayTestInformation(float delatTime) {

	//Verify is editor is simulating 
	if (GEditor->IsSimulatingInEditor()) {
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
}

