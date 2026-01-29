[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionSetExecutionEntryTextBasedOnGeneratedTasks : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Faction key that corresponds with the SCR_Faction set in FactionManager", category: "Asset")]
	FactionKey m_sFactionKey;

	[Attribute()]
	int m_iEntryID;

	[Attribute(desc: "Text that you want to use. Leave empty if you want to utilize the one set in config.")]
	string m_sTargetText;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
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

		SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
	    if (!taskSystem)
	        return;

		array<SCR_Task> tasks = {};
		taskSystem.GetTasks(tasks);

		array<SCR_ScenarioFrameworkTask> frameworkTasks = {};
		foreach (SCR_Task task : tasks)
		{
			if (SCR_ScenarioFrameworkTask.Cast(task))
				frameworkTasks.Insert(SCR_ScenarioFrameworkTask.Cast(task));
		}

		array<string> taskStrings = {};
		foreach (SCR_ScenarioFrameworkTask frameworkTask : frameworkTasks)
		{
			taskStrings.Insert(frameworkTask.GetTaskExecutionBriefing());
			taskStrings.Insert(frameworkTask.GetSlotTask().GetSpawnedEntityDisplayName());
		}

		if (m_sTargetText.IsEmpty())
			m_sTargetText = targetJournalEntry.GetEntryText();

		respawnBriefing.RewriteEntry_SA(m_sFactionKey, m_iEntryID, m_sTargetText, taskStrings);
	}
}