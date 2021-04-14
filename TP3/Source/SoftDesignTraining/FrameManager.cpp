
#include "SoftDesignTraining.h"
#include "FrameManager.h"


FrameManager* FrameManager::m_Instance;

FrameManager::FrameManager(){}

/// <summary>
/// Get Frame manager instance
/// </summary>
/// <returns> Return frame manager instance </returns>
FrameManager* FrameManager::GetInstance()
{
	if (!m_Instance)
	{
		m_Instance = new FrameManager();
	}

	return m_Instance;
}

void FrameManager::Destroy()
{
	delete m_Instance;
	m_Instance = nullptr;
}

/// <summary>
/// Verify if we can update the AI agent in the frame budget
/// </summary>
/// <param name="frameExecuted"> Actual frame execute </param>
/// <returns></returns>
bool FrameManager::canExecute(uint64& frameExecute) {

	uint64 deltaExecuteTime = GFrameCounter - frameExecute;

	bool canExecute = deltaExecuteTime >= m_maxExecutionTime;
	
	// if true save the frame executed
	if (canExecute)
		frameExecute = GFrameCounter;

	return canExecute;
}

/// <summary>
/// Increase AI count and calculate the max execution time
/// </summary>
void FrameManager::increaseAiCount()
{
	m_updateAiCount++;
	m_maxExecutionTime = (int) ceil(m_updateAiCount / m_frameBudget);
}

