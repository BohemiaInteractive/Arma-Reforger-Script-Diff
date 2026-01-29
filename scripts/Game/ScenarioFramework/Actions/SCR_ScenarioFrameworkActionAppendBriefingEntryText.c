[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionAppendBriefingEntryText : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Faction key that corresponds with the SCR_Faction set in FactionManager")]
	FactionKey m_sFactionKey;

	[Attribute()]
	int m_iEntryID;

	[Attribute()]
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

		string finalText = targetJournalEntry.GetEntryText() + "<br/>" + "<br/>" + m_sTargetText;
			targetJournalEntry.SetEntryText(finalText);
	}
}