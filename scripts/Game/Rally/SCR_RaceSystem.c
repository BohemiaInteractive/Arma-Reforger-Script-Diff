class SCR_RaceSystem : GameSystem
{
	SCR_RaceEntity m_CurrentRace;

	//------------------------------------------------------------------------------------------------
	SCR_RaceEntity GetCurrentRace()
	{
		return m_CurrentRace;
	}

	//------------------------------------------------------------------------------------------------
	void SetCurrentRace(SCR_RaceEntity race)
	{
		m_CurrentRace = race;
	}

	//------------------------------------------------------------------------------------------------
	override static void InitInfo(WorldSystemInfo outInfo)
	{
		//remove the return for this to trigger again
		return;
		outInfo.SetAbstract(false);
		outInfo.SetLocation(WorldSystemLocation.Both);
		outInfo.AddPoint(WorldSystemPoint.Frame);
	}
}
