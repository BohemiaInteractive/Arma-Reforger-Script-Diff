[BaseContainerProps()]
class SCR_DataCollectorModule : Managed
{
	protected ref map<int, TextWidget> m_StatsVisualization;

	protected float m_fTimeSinceUpdate = 0;
	[Attribute("1", UIWidgets.Slider, desc: "Seconds between updates of the module")]
	float m_fUpdatePeriod;

	//***************************//
	/* OVERRIDDEN BY ALL MODULES */
	//***************************//

	//------------------------------------------------------------------------------------------------
	void Update(float timeTick);

	//***********************************************************//
	/* OVERRIDDEN BY SOME MODULES BUT CALLING THEIR SUPER METHOD */
	//***********************************************************//

	//------------------------------------------------------------------------------------------------
	void OnPlayerDisconnected(int playerID, IEntity controlledEntity = null)
	{
		if (!controlledEntity)
			controlledEntity = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);

		if (controlledEntity)
			RemoveInvokers(controlledEntity);
	}

	//------------------------------------------------------------------------------------------------
	protected void AddInvokers(IEntity player);

	//------------------------------------------------------------------------------------------------
	protected void RemoveInvokers(IEntity player);

	//------------------------------------------------------------------------------------------------
	void OnPlayerSpawned(int playerID, IEntity controlledEntity)
	{
		AddInvokers(controlledEntity);
	}

	//***********************************************************//
	/* OVERRIDDEN BY SOME MODULES BUT CALLING THEIR SUPER METHOD */
	/************ (unless the call is very specific) *************/
	//***********************************************************//

	//------------------------------------------------------------------------------------------------
	void OnPlayerAuditSuccess(int playerID);
	
	//------------------------------------------------------------------------------------------------
	void OnPlayerKilled(int playerId, IEntity playerEntity, IEntity killerEntity, notnull Instigator instigator, notnull SCR_InstigatorContextData instigatorContextData);
	
	//------------------------------------------------------------------------------------------------
	void OnAIKilled(IEntity AIEntity, IEntity killerEntity, notnull Instigator instigator, notnull SCR_InstigatorContextData instigatorContextData);
	
	//------------------------------------------------------------------------------------------------
	void OnGameModeEnd();
	
	//------------------------------------------------------------------------------------------------
	void InitModule();

	//*****************//
	/* NEVER OVERRIDDEN*/
	//*****************//

	//------------------------------------------------------------------------------------------------
	void OnControlledEntityChanged(IEntity from, IEntity to)
	{
		//If the previous entity exists, remove the invokers from it
		if (from)
			RemoveInvokers(from);

		//If there's no entity yet we can assume the player disconnected or it hasn't spawned.
		//In this case we do nothing and keep listening to the spawn invoker.
		//Otherwise, we add the invokers to the 'to' entity
		if (to)
			AddInvokers(to);
	}

	//************************************************************************//
	/*********************** CREATED ONLY FOR PROTOTYPE ***********************/
	/* These create a textwidget on screen with the stats updated dynamically */
	//************************************************************************//
	//************************************************************************//

#ifdef ENABLE_DIAG
	//------------------------------------------------------------------------------------------------
	void CreateVisualization()
	{
		m_StatsVisualization = new map<int, TextWidget>();
	}

	//------------------------------------------------------------------------------------------------
	protected void CreateEntry(string text, float value, int id)
	{
		if (!m_StatsVisualization || !GetGame().GetDataCollector() || !GetGame().GetDataCollector().GetUIHandler())
			return;

		Widget entry = GetGame().GetDataCollector().GetUIHandler().CreateEntry();

		if (!entry)
		{
			Print("UI handler from DataCollector couldn't create a visual widget", LogLevel.ERROR);
			return;
		}

		TextWidget StatName = TextWidget.Cast(entry.FindAnyWidget("StatName"));
		TextWidget StatValue = TextWidget.Cast(entry.FindAnyWidget("StatValue"));
		if (!StatName || !StatValue)
		{
			Print("StatName or StatValue textwidgets could not be found in data collector ui handler's layout", LogLevel.ERROR);
			return;
		}

		StatName.SetText(text); StatValue.SetText(value.ToString());
		m_StatsVisualization.Insert(id, StatValue);
	}
#endif
};
