[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionAppendBriefingEntryTextBasedOnTask : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Faction key that corresponds with the SCR_Faction set in FactionManager")]
	FactionKey m_sFactionKey;

	[Attribute()]
	int m_iEntryID;

	[Attribute(desc: "From which task to fetch text")]
	ref SCR_ScenarioFrameworkGet m_Getter;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!m_Getter || !CanActivate())
			return;

		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		if (!entityWrapper)
			return;

		SCR_ScenarioFrameworkTask task = SCR_ScenarioFrameworkTask.Cast(entityWrapper.GetValue());
		if (!task)
			return;

		SCR_BaseGameMode gamemode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gamemode)
			return;

		SCR_RespawnBriefingComponent respawnBriefing = SCR_RespawnBriefingComponent.Cast(gamemode.FindComponent(SCR_RespawnBriefingComponent));
		if (!respawnBriefing)
			return;

		SCR_JournalSetupConfig journalSetupConfig = respawnBriefing.GetJournalSetup();
		if (!journalSetupConfig)
			return;

		// Resolve Alias
		SCR_FactionAliasComponent factionAliasComponent = SCR_FactionAliasComponent.Cast(GetGame().GetFactionManager().FindComponent(SCR_FactionAliasComponent));
		if (factionAliasComponent) 
			m_sFactionKey = factionAliasComponent.ResolveFactionAlias(m_sFactionKey);	

		SCR_JournalConfig journalConfig = journalSetupConfig.GetJournalConfig(m_sFactionKey);
		if (!journalConfig)
			return;

		array<ref SCR_JournalEntry> journalEntries = {};
		journalEntries = journalConfig.GetEntries();
		if (journalEntries.IsEmpty())
			return;

		SCR_JournalEntry targetJournalEntry;
		foreach (SCR_JournalEntry journalEntry : journalEntries)
		{
			if (journalEntry.GetEntryID() != m_iEntryID)
				continue;

			targetJournalEntry = journalEntry;
			break;
		}

		if (!targetJournalEntry)
			return;

		array<string> previousStrings = {};
		array<string> taskStrings = {};
		previousStrings = respawnBriefing.GetBriefingStringParamByID(m_iEntryID);
		if (previousStrings)
			taskStrings.InsertAll(previousStrings);

		taskStrings.Insert(task.GetTaskExecutionBriefing());
		taskStrings.Insert(task.GetSlotTask().GetSpawnedEntityDisplayName());

		respawnBriefing.RewriteEntry_SA(m_sFactionKey, m_iEntryID, targetJournalEntry.GetEntryText(), taskStrings);
	}
}