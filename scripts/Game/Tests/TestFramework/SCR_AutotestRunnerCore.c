/*!
Game core responsible for running the gameplay test suites.
*/
[BaseContainerProps(configRoot: true)]
sealed class SCR_AutotestRunnerCore : SCR_GameCoreBase
{
	protected ref SCR_TestRunner m_TestRunner;
	protected Game m_Game;
	
	//------------------------------------------------------------------------------------------------
	override bool CanCreate()
	{
		SCR_TestRunner.OnGameStart();
		if (SCR_TestRunner.HasInstance())
		{
			m_Game = GetGame();
			return true;
		}
		
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void OnUpdate(float timeSlice)
	{		
		if (SCR_TestRunner.HasInstance())
			SCR_TestRunner.GetInstance().OnUpdate(m_Game);
	}

	override void OnGameEnd()
	{
		SCR_TestRunner.OnGameEnd(m_Game);
	}
}