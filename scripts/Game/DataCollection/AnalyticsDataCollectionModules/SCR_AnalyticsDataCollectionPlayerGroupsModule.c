[BaseContainerProps()]
class SCR_AnalyticsDataCollectionPlayerGroupsModule : SCR_AnalyticsDataCollectionModule
{
	//------------------------------------------------------------------------------------------------
	override void SessionMeasures()
	{
		if (!m_bIsEnabled)
			return;

		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;

		int playersInMultiMemberGroups;
		array<SCR_AIGroup> groups = {};
		groupsManager.GetAllPlayableGroups(groups);
		int groupPlayerCount;

		foreach (SCR_AIGroup group : groups)
		{
			// We count only players who are in group with other players
			groupPlayerCount = group.GetPlayerCount();
			if (groupPlayerCount > 1)
				playersInMultiMemberGroups += groupPlayerCount;
		}

		SCR_SessionDataEvent sessionDataEvent = GetSessionDataEvent();
		if (!sessionDataEvent)
			return;

		sessionDataEvent.num_players_in_squad = playersInMultiMemberGroups;
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
	protected void ~SCR_AnalyticsDataCollectionPlayerGroupsModule()
	{
		Disable();
	}
}
