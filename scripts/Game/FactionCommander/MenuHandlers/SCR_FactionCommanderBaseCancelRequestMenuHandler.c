[BaseContainerProps()]
class SCR_FactionCommanderBaseCancelRequestMenuHandler : SCR_FactionCommanderBaseTaskMenuHandler
{
	[Attribute(desc:"If true, player voted as commander can cancel request")]
	protected bool m_bCanBeCanceledByCommander;

	//------------------------------------------------------------------------------------------------
	override bool CanEntryBeSelected(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity, out string disabledText = "")
	{
		SCR_BaseRequestedTaskEntity task = SCR_BaseRequestedTaskEntity.Cast(hoveredEntity);
		if (!task)
			return false;

		// commander can cancel all requests if it is allowed by config
		if (SCR_FactionCommanderPlayerComponent.IsLocalPlayerCommander() && m_bCanBeCanceledByCommander)
			return true;

		SCR_PlayerControllerGroupComponent playerControllerGroupComponent = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!playerControllerGroupComponent)
			return false;

		// group leader can't cancel own request if a group is already assigned
		if (task.GetTaskAssigneeCount() > 0 ||
			!task.RequestedByLocalPlayerGroup() ||
			!playerControllerGroupComponent.IsPlayerLeaderOwnGroup())
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnCommandIssued(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity, SCR_AIGroup group)
	{
		SCR_FactionCommanderPlayerComponent factionCommanderPlayerComponent = GetLocalCommanderComponent();

		if (!factionCommanderPlayerComponent)
			return;

		SCR_BaseRequestedTaskEntity task = SCR_BaseRequestedTaskEntity.Cast(hoveredEntity);
		if (!task)
			return;

		factionCommanderPlayerComponent.CancelTask(playerId, task.GetTaskID());
	}
}
