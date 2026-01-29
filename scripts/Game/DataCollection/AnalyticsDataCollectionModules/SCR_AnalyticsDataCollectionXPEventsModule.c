[BaseContainerProps()]
class SCR_AnalyticsDataCollectionXPEventsModule : SCR_AnalyticsDataCollectionModule
{
	protected ref map<int, ref SCR_AnalyticsPlayerXPData> m_mPlayerXPData = new map<int, ref SCR_AnalyticsPlayerXPData>();

	#ifdef ENABLE_DIAG
	protected int m_iSelectedPlayerDataIndex;
	protected SCR_AnalyticsPlayerXPData m_SelectedPlayerData;
	#endif

	//------------------------------------------------------------------------------------------------
	protected override void Enable()
	{
		super.Enable();

		SCR_CharacterRankComponent.s_OnRankChanged.Insert(OnPlayerRankChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void Disable()
	{
		super.Disable();

		SCR_CharacterRankComponent.s_OnRankChanged.Remove(OnPlayerRankChanged);
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnGameModeEnd()
	{
		Disable();
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnPlayerConnected(int playerId)
	{
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);
		if (playerController)
		{
			SCR_PlayerXPHandlerComponent xpHandlerComponent = SCR_PlayerXPHandlerComponent.Cast(playerController.FindComponent(SCR_PlayerXPHandlerComponent));
			if (xpHandlerComponent)
				xpHandlerComponent.GetOnPlayerXPChanged().Insert(OnPlayerXPChanged);
		}
		
		if (m_mPlayerXPData.Contains(playerId))
			return;

		SCR_AnalyticsPlayerXPData data = new SCR_AnalyticsPlayerXPData();
		m_mPlayerXPData.Insert(playerId, data);

		SCR_PlayerData playerData = SCR_PlayerData.GetPlayerData(playerId);
		if (!playerData)
			return;

		playerData.dataEvent.name_rank_level = SCR_Enum.GetEnumName(SCR_ECharacterRank, data.GetRank());
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnPlayerDisconnected(int playerId, KickCauseCode cause, IEntity controlledEntity)
	{
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);
		if (playerController)
		{
			SCR_PlayerXPHandlerComponent xpHandlerComponent = SCR_PlayerXPHandlerComponent.Cast(playerController.FindComponent(SCR_PlayerXPHandlerComponent));
			if (xpHandlerComponent)
				xpHandlerComponent.GetOnPlayerXPChanged().Remove(OnPlayerXPChanged);
		}
		
		SCR_AnalyticsPlayerXPData data = m_mPlayerXPData.Get(playerId);
		if (!data)
			return;

		SCR_PlayerData playerData = SCR_PlayerData.GetPlayerData(playerId);
		if (!playerData)
			return;

		playerData.dataEvent.name_rank_level = SCR_Enum.GetEnumName(SCR_ECharacterRank, data.GetRank());
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlayerRankChanged(SCR_ECharacterRank prevRank, SCR_ECharacterRank newRank, IEntity playerEntity, bool silent)
	{
		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(playerEntity);
		if (playerId <= 0)
			return;

		SCR_AnalyticsPlayerXPData data = m_mPlayerXPData.Get(playerId);
		if (!data)
			return;

		// This event is called on spawn for players with non-default ranks
		// No need to continue in that case as player rank did not actually change
		if (data.GetRank() == newRank)
			return;

		data.SetRank(newRank);
		m_mPlayerXPData.Set(playerId, data);

		SCR_PlayerData playerData = SCR_PlayerData.GetPlayerData(playerId);
		if (!playerData)
			return;

		playerData.dataEvent.name_rank_level = SCR_Enum.GetEnumName(SCR_ECharacterRank, data.GetRank());

		GetGame().GetStatsApi().PlayerRankGained(playerId, playerData.dataEvent);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlayerXPChanged(int playerId, int currentXP, int XPToAdd, SCR_EXPRewards rewardId)
	{
		if (playerId <= 0)
			return;

		SCR_AnalyticsPlayerXPData data = m_mPlayerXPData.Get(playerId);
		if (!data)
			return;

		// new XP data added
		if (rewardId != SCR_EXPRewards.UNDEFINED)
		{
			data.AddXPReward(rewardId, XPToAdd);
			m_mPlayerXPData.Set(playerId, data);
		}

		SetPlayerXPData(playerId, data);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetPlayerXPData(int playerId, SCR_AnalyticsPlayerXPData data)
	{
		SCR_PlayerData playerData = SCR_PlayerData.GetPlayerData(playerId);
		if (!playerData)
			return;

		map<SCR_EXPRewards, int> rewardData = data.GetXPRewards();
		array<string> xpRewardsString = {};

		foreach(SCR_EXPRewards reward, int amount : rewardData)
		{
			xpRewardsString.Insert(string.Format("%1: %2", SCR_Enum.GetEnumName(SCR_EXPRewards, reward), amount));
		}

		playerData.dataEvent.amt_xp_gained = data.GetCurrentXP();
		playerData.dataEvent.array_rank_points_source = SCR_AnalyticsDataCollectionHelper.GetShortDataArray(xpRewardsString.ToString());
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

		if (m_mPlayerXPData.IsEmpty())
			return;

		if (DbgUI.Button("Previous Player"))
			OnSetSelectedPlayerIndex(m_iSelectedPlayerDataIndex - 1);

		DbgUI.SameLine();

		if (DbgUI.Button("Next Player"))
			OnSetSelectedPlayerIndex(m_iSelectedPlayerDataIndex + 1);	

		m_SelectedPlayerData = m_mPlayerXPData.GetElement(m_iSelectedPlayerDataIndex);

		DrawPlayerData();	
	}

	//------------------------------------------------------------------------------------------------
	void DrawPlayerData()
	{
		if (!m_SelectedPlayerData)
			return;

		int playerId = m_mPlayerXPData.GetKey(m_iSelectedPlayerDataIndex);
		string rankName = SCR_Enum.GetEnumName(SCR_ECharacterRank, m_SelectedPlayerData.GetRank());
		DbgUI.Text(string.Format("Player %1: %2, XP: %3, RANK: %4", playerId, GetGame().GetPlayerManager().GetPlayerName(playerId), m_SelectedPlayerData.GetCurrentXP(), rankName));

		map<SCR_EXPRewards, int> xpEventsRewards = m_SelectedPlayerData.GetXPRewards();
		string rewardName;

		foreach (SCR_EXPRewards rewardType, int rewardAmount : xpEventsRewards)
		{
			rewardName = SCR_Enum.GetEnumName(SCR_EXPRewards, rewardType);
			DbgUI.Text(string.Format("%1: %2", rewardName, rewardAmount));
		}
	}

	//------------------------------------------------------------------------------------------------
	private void OnSetSelectedPlayerIndex(int index)
	{
		m_iSelectedPlayerDataIndex = Math.Clamp(index, 0, m_mPlayerXPData.Count() - 1);
	}
	#endif

	//------------------------------------------------------------------------------------------------
	protected void ~SCR_AnalyticsDataCollectionXPEventsModule()
	{
		Disable();
	}
}
