[BaseContainerProps()]
class SCR_AnalyticsDataCollectionVoNDataModule : SCR_AnalyticsDataCollectionModule
{
	protected ref map<int, bool> m_mPlayerVoNActivation = new map<int, bool>();

	//------------------------------------------------------------------------------------------------
	protected override void Enable()
	{
		super.Enable();

		SCR_VoNComponent.GetOnVoNUsed().Insert(OnVoNUsed);
	}

	//------------------------------------------------------------------------------------------------
	protected override void Disable()
	{
		super.Enable();

		SCR_VoNComponent.GetOnVoNUsed().Remove(OnVoNUsed);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerConnected(int playerId)
	{
		if (m_mPlayerVoNActivation.Contains(playerId))
			return;

		m_mPlayerVoNActivation.Insert(playerId, false);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerDisconnected(int playerId, KickCauseCode cause, IEntity controlledEntity = null)
	{
		super.OnPlayerDisconnected(playerId, cause, controlledEntity);

		m_mPlayerVoNActivation.Remove(playerId);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnVoNUsed(int playerId)
	{
		if (m_mPlayerVoNActivation.Get(playerId))
			return;

		m_mPlayerVoNActivation.Set(playerId, true);

		SCR_PlayerData playerData = SCR_PlayerData.GetPlayerData(playerId);
		if (!playerData)
			return;

		playerData.dataEvent.is_verbal_comm = true;
	}

	#ifdef ENABLE_DIAG
	//------------------------------------------------------------------------------------------------
	override void DrawContent()
	{
		if (!m_bIsEnabled)
		{
			DbgUI.Text("Module disabled!");
			return;
		}

		foreach (int playerID, bool usedVoN : m_mPlayerVoNActivation)
		{
			string playerName = GetGame().GetPlayerManager().GetPlayerName(playerID);
			DbgUI.Text(string.Format("Player %1 - %2 used VoN communication = %3", playerID, playerName, usedVoN));			
		}
	}
	#endif

	//------------------------------------------------------------------------------------------------
	protected void ~SCR_AnalyticsDataCollectionVoNDataModule()
	{
		Disable();
	}
}