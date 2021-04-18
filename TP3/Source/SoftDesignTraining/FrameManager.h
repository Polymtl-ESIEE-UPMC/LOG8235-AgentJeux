#pragma once

#include "CoreMinimal.h"



class FrameManager
{
public:

	static FrameManager* GetInstance();
	static void Destroy();
	bool CanExecute(uint64& frameExecute);
	void IncreaseAiCount();
	
private:
	//Singleton
	FrameManager();
	static FrameManager* m_Instance;

	// Define budget for updating AI at 30 fps with only 1/2 of the budget
	const float m_budget = (1.0f / 30.0f / 2.0f);
	const float m_execution_time = 0.003f;
	float m_frameBudgetAI = m_budget / m_execution_time;
	int m_maxExecutionTime;
	int m_updateAiCount = 0;
};