[BaseContainerProps()]
class SCR_FactionCommanderConfirmMenuHandler : SCR_FactionCommanderBaseTaskMenuHandler
{
	[Attribute(desc:"If true, player voted as commander can confirm command")]
	protected bool m_bCanBeConfirmedByCommander;

	//------------------------------------------------------------------------------------------------
	override bool CanEntryBeShown(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity)
	{
		if (!super.CanEntryBeShown(commanderFaction, playerId, position, hoveredEntity))
			return false;

		return SCR_FactionCommanderPlayerComponent.IsLocalPlayerCommander();
	}

	//------------------------------------------------------------------------------------------------
	override bool CanEntryBeSelected(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity, out string disabledText = "")
	{
		if (!IsTaskValid(hoveredEntity))
			return false;

		// commander can confirm all tasks if it is allowed by config
		if (SCR_FactionCommanderPlayerComponent.IsLocalPlayerCommander() && m_bCanBeConfirmedByCommander)
			return true;

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void OnCommandIssued(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity, SCR_AIGroup group)
	{
		SCR_Task task = SCR_Task.Cast(hoveredEntity);
		if (!task)
			return;

		if (!IsTaskValid(hoveredEntity))
			return;

		SCR_FactionCommanderPlayerComponent factionCommanderPlayerComponent = GetLocalCommanderComponent();
		if (!factionCommanderPlayerComponent)
			return;

		factionCommanderPlayerComponent.FinishTask(playerId, task.GetTaskID());
	}
}
