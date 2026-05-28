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
		return SCR_TestRunner.ShouldCreate();
	}

	//------------------------------------------------------------------------------------------------
	override void OnUpdate(float timeSlice)
	{
		if (!m_TestRunner)
		{
			m_TestRunner = new SCR_TestRunner();
			m_Game = GetGame();
		}
		
		m_TestRunner.OnUpdate(m_Game)
	}

	override void OnGameEnd()
	{
		if (!m_TestRunner)
			return;
		
		m_TestRunner.Abort(m_Game);
	}
}