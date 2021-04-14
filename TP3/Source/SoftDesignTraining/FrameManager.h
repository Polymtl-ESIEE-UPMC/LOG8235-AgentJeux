#pragma once

#include "CoreMinimal.h"

// Define budget for updating AI at 30 fps
#define BUDGET 1.0f/30.0f/4.0f
#define EXECUTION_TIME 0.004f

class FrameManager
{
public:

	static FrameManager* GetInstance();
	static void Destroy();
	bool canExecute(uint64&);
	void increaseAiCount();
	
private:
	//Singleton
	FrameManager();
	static FrameManager* m_Instance;

	float m_frameBudget = BUDGET / EXECUTION_TIME;
	int m_maxExecutionTime;
	int m_updateAiCount = 0;
};

