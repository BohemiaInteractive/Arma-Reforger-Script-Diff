[BaseContainerProps()]
class SCR_AnalyticsDataCollectionMilitaryBaseEventsModule : SCR_AnalyticsDataCollectionModule
{
	protected SCR_CampaignMilitaryBaseComponent baseInCreation;
	
	//------------------------------------------------------------------------------------------------
	protected override void Enable()
	{
		super.Enable();

		SCR_CampaignMilitaryBaseComponent.GetOnFactionChangedExtended().Insert(OnBaseFactionChanged);
		SCR_CampaignMilitaryBaseComponent.GetOnBaseUnderAttack().Insert(OnBaseUnderAttack);
		SCR_CampaignMilitaryBaseComponent.GetOnBaseCreatedAsFOB().Insert(HandleBaseCreation);
		SCR_CampaignMilitaryBaseManager.GetOnBaseDisassembled().Insert(OnBaseDisassembledAsFOB);
	}

	//------------------------------------------------------------------------------------------------
	protected override void Disable()
	{
		super.Disable();

		SCR_CampaignMilitaryBaseComponent.GetOnFactionChangedExtended().Remove(OnBaseFactionChanged);
		SCR_CampaignMilitaryBaseComponent.GetOnBaseUnderAttack().Remove(OnBaseUnderAttack);
		SCR_CampaignMilitaryBaseComponent.GetOnBaseCreatedAsFOB().Remove(HandleBaseCreation);
		SCR_CampaignMilitaryBaseManager.GetOnBaseDisassembled().Remove(OnBaseDisassembledAsFOB);
	}

	//------------------------------------------------------------------------------------------------
	//! Base was captured, send event to analytics db
	//! \param[in] base
	//! \param[in] previousFaction
	//! \param[in] currentFaction
	protected void OnBaseFactionChanged(notnull SCR_CampaignMilitaryBaseComponent base, Faction previousFaction, Faction currentFaction)
	{
		if(baseInCreation)
		{
			if(base == baseInCreation)
			{
				GetGame().GetCallqueue().Remove(OnBaseCreatedAsFOB);
				OnBaseCreatedAsFOB(base, currentFaction);
				return;
			}
		}
		
		SCR_SessionDataEvent sessionEvent = SetSessionEventBaseInfoWhenOffensive(base, previousFaction, currentFaction);
		if (!sessionEvent)
			return;

		//Send event of SessionBaseCaptured
		GetGame().GetStatsApi().SessionBaseCaptured(sessionEvent);
	}

	//------------------------------------------------------------------------------------------------
	//! Base is under attack, send event to analytics db
	//! \param[in] base
	//! \param[in] defendingFaction
	//! \param[in] attackingFaction
	protected void OnBaseUnderAttack(notnull SCR_CampaignMilitaryBaseComponent base, Faction defendingFaction, Faction attackingFaction)
	{
		SCR_SessionDataEvent sessionEvent = SetSessionEventBaseInfoWhenOffensive(base, defendingFaction, attackingFaction);
		if (!sessionEvent)
			return;

		//Send event of SessionBaseAttacked
		GetGame().GetStatsApi().SessionBaseAttacked(sessionEvent);
	}
	
	//------------------------------------------------------------------------------------------------
	//! OnBaseCreatedAsFOB is inserted on a queue
	//! \param[in] base
	//! \param[in] establishingFaction
	protected void HandleBaseCreation(notnull SCR_CampaignMilitaryBaseComponent base, Faction establishingFaction)
	{
		baseInCreation = base;
		GetGame().GetCallqueue().CallLater(OnBaseCreatedAsFOB, 1000, false, base, establishingFaction);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Base is created, send event to analytics db
	//! \param[in] base
	//! \param[in] establishingFaction
	protected void OnBaseCreatedAsFOB(notnull SCR_CampaignMilitaryBaseComponent base, Faction establishingFaction)
	{
		if(baseInCreation)
			baseInCreation = null;
		
		SCR_SessionDataEvent sessionEvent = SetSessionEventBaseInfoWhenCreatingOrErasing(base, establishingFaction);
		if (!sessionEvent)
			return;
		
		//Send event of SessionBaseAttacked
		GetGame().GetStatsApi().CreateSessionEvent(sessionEvent, "baseCreated");
	}
	
	//------------------------------------------------------------------------------------------------
	//! Base is erased, send event to analytics db
	//! \param[in] base
	//! \param[in] establishingFaction
	protected void OnBaseDisassembledAsFOB(notnull SCR_CampaignMilitaryBaseComponent base, Faction establishingFaction)
	{
		SCR_SessionDataEvent sessionEvent = SetSessionEventBaseInfoWhenCreatingOrErasing(base, establishingFaction);
		if (!sessionEvent)
			return;
		
		//Send event of SessionBaseAttacked
		GetGame().GetStatsApi().CreateSessionEvent(sessionEvent, "baseDisassembled");
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_SessionDataEvent SetSessionEventBaseInfoWhenCreatingOrErasing(notnull SCR_CampaignMilitaryBaseComponent base, Faction establishingFaction)
	{
		if (!base)
			return null;

		SCR_SessionDataEvent sessionEvent = GetSessionDataEvent();
		if (!sessionEvent)
			return null;
		
		sessionEvent.name_base = "FoB";
		sessionEvent.name_faction_base_def = "";
		sessionEvent.name_faction_base_att = "";
		if (establishingFaction)
			sessionEvent.name_faction_base_establishing = establishingFaction.GetFactionKey();
		else
			sessionEvent.name_faction_base_establishing = "Unknown faction";
		
		vector basePosition;
		
		if (base.GetOwner())
			basePosition = base.GetOwner().GetOrigin();
		else
			basePosition = vector.Zero;
		
		sessionEvent.base_position_x = basePosition[0];
		sessionEvent.base_position_y = basePosition[1];
		sessionEvent.base_position_z = basePosition[2];
		
		return sessionEvent;
	}
	
	protected SCR_SessionDataEvent SetSessionEventBaseInfoWhenOffensive(notnull SCR_CampaignMilitaryBaseComponent base, Faction defendingFaction, Faction attackingFaction)
	{
		if (!base || !defendingFaction || !attackingFaction)
			return null;

		SCR_SessionDataEvent sessionEvent = GetSessionDataEvent();
		if (!sessionEvent)
			return null;
		
		sessionEvent.name_base = base.GetBaseName();
		sessionEvent.name_faction_base_def = defendingFaction.GetFactionKey();
		sessionEvent.name_faction_base_att = attackingFaction.GetFactionKey();
		sessionEvent.name_faction_base_establishing = "";

		vector basePosition = base.GetOwner().GetOrigin();
		sessionEvent.base_position_x = basePosition[0];
		sessionEvent.base_position_y = basePosition[1];
		sessionEvent.base_position_z = basePosition[2];
		
		return sessionEvent;
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

		DbgUI.Text("This module does not save any data.");
	}
	#endif

	//------------------------------------------------------------------------------------------------
	protected void ~SCR_AnalyticsDataCollectionMilitaryBaseEventsModule()
	{
		Disable();
	}
}
