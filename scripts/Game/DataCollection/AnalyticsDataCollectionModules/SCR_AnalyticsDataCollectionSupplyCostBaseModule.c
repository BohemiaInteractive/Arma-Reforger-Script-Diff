[BaseContainerProps()]
class SCR_AnalyticsDataCollectionSupplyCostBaseModule : SCR_AnalyticsDataCollectionModule
{
	protected ref map<int, ref SCR_AnalyticsPlayerSupplySpendingData> m_mPlayerSupplySpendingData = new map<int, ref SCR_AnalyticsPlayerSupplySpendingData>();

	#ifdef ENABLE_DIAG
	protected int m_iSelectedPlayerDataIndex;
	protected SCR_AnalyticsPlayerSupplySpendingData m_SelectedPlayerData;
	#endif

	//------------------------------------------------------------------------------------------------
	protected override void OnPlayerConnected(int playerId)
	{
		// Player has not spent any supplies yet
		if (!m_mPlayerSupplySpendingData.Contains(playerId))
			return;

		SCR_PlayerData playerData = SCR_PlayerData.GetPlayerData(playerId);
		if (!playerData)
			return;

		// Resource spending data has been added already by another supply cost module, no need to add it again
		if (playerData.dataEvent.amt_res_spent != 0)
			return;

		AddPlayerResourceSpendingData(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] playerID
	//! \param[in] itemName
	//! \param[in] cost
	protected void AddSupplySpendingData(int playerID, string itemName, int cost)
	{
		SCR_AnalyticsPlayerSupplySpendingData spendingData = m_mPlayerSupplySpendingData.Get(playerID);
		if (!spendingData)
			spendingData = new SCR_AnalyticsPlayerSupplySpendingData();

		spendingData.AddItemSupplyCost(itemName, cost);
		m_mPlayerSupplySpendingData.Set(playerID, spendingData);

		AddPlayerResourceSpendingData(playerID);
 	}
 
 	//------------------------------------------------------------------------------------------------
	//! \return player spending data or null if player with playerID does not have any spending data
	SCR_AnalyticsPlayerSupplySpendingData GetPlayerSpendingData(int playerID)
	{
		return m_mPlayerSupplySpendingData.Get(playerID);
	}

	//------------------------------------------------------------------------------------------------
	//! Gets the supply spending data from all modules and inserts it into dataEvent
	void AddPlayerResourceSpendingData(int playerID)
	{
		IEntity gameModeEntity = GetGame().GetDataCollector().GetOwner();
		if (!gameModeEntity)
			return;

		SCR_PlayerData playerData = SCR_PlayerData.GetPlayerData(playerID);
		if (!playerData)
			return;

		SCR_AnalyticsDataCollectionComponent analyticsDataCollector = SCR_AnalyticsDataCollectionComponent.Cast(gameModeEntity.FindComponent(SCR_AnalyticsDataCollectionComponent));
		if (!analyticsDataCollector)
			return;

		array<ref SCR_AnalyticsDataCollectionModule> modules = analyticsDataCollector.GetAllModules();
		SCR_AnalyticsDataCollectionSupplyCostBaseModule supplyCostModule;
		int resourcesSpent;
		map<string, int> itemSpendingData;
		array<string> resourceConsumptionArray = {};

		foreach (SCR_AnalyticsDataCollectionModule module : modules)
		{
			supplyCostModule = SCR_AnalyticsDataCollectionSupplyCostBaseModule.Cast(module);
			if (!supplyCostModule)
				continue;

			SCR_AnalyticsPlayerSupplySpendingData playerSpendingData = supplyCostModule.GetPlayerSpendingData(playerID);
			if (!playerSpendingData)
				continue;

			resourcesSpent += playerSpendingData.GetAggregateSupplyCost();
			itemSpendingData = playerSpendingData.GetItemSupplyCosts();

			foreach (string itemName, int spentSupplies : itemSpendingData)
			{
				resourceConsumptionArray.Insert(string.Format("%1: %2", itemName, spentSupplies));
			}
		}

		playerData.dataEvent.amt_res_spent = resourcesSpent;
		playerData.dataEvent.amt_res_consumption = SCR_AnalyticsDataCollectionHelper.GetShortDataArray(resourceConsumptionArray.ToString());
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnGameModeEnd()
	{
		// TODO: Handle data to send them to analytics db
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

		if (m_mPlayerSupplySpendingData.IsEmpty())
		{
			DbgUI.Text("No data recorded yet.");
			return;
		}

		if (DbgUI.Button("Previous Player"))
			OnSetSelectedPlayerIndex(m_iSelectedPlayerDataIndex - 1);

		DbgUI.SameLine();

		if (DbgUI.Button("Next Player"))
			OnSetSelectedPlayerIndex(m_iSelectedPlayerDataIndex + 1);

		m_SelectedPlayerData = m_mPlayerSupplySpendingData.GetElement(m_iSelectedPlayerDataIndex);

		DrawPlayerData();
	}

	//------------------------------------------------------------------------------------------------
	protected void DrawPlayerData()
	{
		if (!m_SelectedPlayerData)
			return;

		int playerId = m_mPlayerSupplySpendingData.GetKey(m_iSelectedPlayerDataIndex);
		DbgUI.Text(string.Format("Player %1: %2", playerId, GetGame().GetPlayerManager().GetPlayerName(playerId)));

		map <string, int> itemSupplyCosts = m_SelectedPlayerData.GetItemSupplyCosts();
		foreach (string itemName, int cost : itemSupplyCosts)
		{
			DbgUI.Text(string.Format("%1: %2", itemName, cost));
		}
	}

	//------------------------------------------------------------------------------------------------
	private void OnSetSelectedPlayerIndex(int index)
	{
		m_iSelectedPlayerDataIndex = Math.Clamp(index, 0, m_mPlayerSupplySpendingData.Count() - 1);
	}
	#endif
}