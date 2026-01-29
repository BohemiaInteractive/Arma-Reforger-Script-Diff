[BaseContainerProps()]
class SCR_AnalyticsDataCollectionMilitaryBaseEventsModule : SCR_AnalyticsDataCollectionModule
{
	//------------------------------------------------------------------------------------------------
	protected override void Enable()
	{
		super.Enable();

		SCR_CampaignMilitaryBaseComponent.GetOnFactionChangedExtended().Insert(OnBaseFactionChanged);
		SCR_CampaignMilitaryBaseComponent.GetOnBaseUnderAttack().Insert(OnBaseUnderAttack);
	}

	//------------------------------------------------------------------------------------------------
	protected override void Disable()
	{
		super.Disable();

		SCR_CampaignMilitaryBaseComponent.GetOnFactionChangedExtended().Remove(OnBaseFactionChanged);
		SCR_CampaignMilitaryBaseComponent.GetOnBaseUnderAttack().Insert(OnBaseUnderAttack);
	}

	//------------------------------------------------------------------------------------------------
	//! Base was captured, send event to analytics db
	//! \param[in] base
	//! \param[in] previousFaction
	//! \param[in] currentFaction
	protected void OnBaseFactionChanged(SCR_CampaignMilitaryBaseComponent base, Faction previousFaction, Faction currentFaction)
	{
		SCR_SessionDataEvent sessionEvent = SetSessionEventBaseInfo(base, previousFaction, currentFaction);
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
	protected void OnBaseUnderAttack(SCR_CampaignMilitaryBaseComponent base, Faction defendingFaction, Faction attackingFaction)
	{
		SCR_SessionDataEvent sessionEvent = SetSessionEventBaseInfo(base, defendingFaction, attackingFaction);
		if (!sessionEvent)
			return;

		//Send event of SessionBaseAttacked
		GetGame().GetStatsApi().SessionBaseAttacked(sessionEvent);
	}

	//------------------------------------------------------------------------------------------------
	//! Gets string names of base, attacking faction and defending faction
	//! \param[in] base
	//! \param[in] defendingFaction
	//! \param[in] attackingFaction
	//! \param[out] baseName
	//! \param[out] defendingFactionName
	//! \param[out] attackingFactionName
	protected void GetBaseAndFactionNames(out string baseName, out string defendingFactionName, out string attackingFactionName, SCR_CampaignMilitaryBaseComponent base, Faction defendingFaction, Faction attackingFaction)
	{
		baseName = base.GetBaseName();
		defendingFactionName = defendingFaction.GetFactionKey();
		attackingFactionName = attackingFaction.GetFactionKey();
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_SessionDataEvent SetSessionEventBaseInfo(SCR_CampaignMilitaryBaseComponent base, Faction defendingFaction, Faction attackingFaction)
	{
		if (!base || !defendingFaction || !attackingFaction)
			return null;

		string baseName, defendingFactionName, attackingFactionName;
		GetBaseAndFactionNames(baseName, defendingFactionName, attackingFactionName, base, defendingFaction, attackingFaction);

		SCR_SessionDataEvent sessionEvent = GetSessionDataEvent();
		if (!sessionEvent)
			return null;

		sessionEvent.name_base = baseName;
		sessionEvent.name_faction_base_def = defendingFactionName;
		sessionEvent.name_faction_base_att = attackingFactionName;

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
