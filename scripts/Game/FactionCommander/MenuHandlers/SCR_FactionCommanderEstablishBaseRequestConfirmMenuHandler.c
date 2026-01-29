[BaseContainerProps()]
class SCR_FactionCommanderEstablishBaseRequestConfirmMenuHandler : SCR_FactionCommanderMainCommandMenuHandler
{
	[Attribute("", UIWidgets.ResourceNamePicker, "", "et")]
	protected ResourceName m_sEstablishTaskPrefab;

	//------------------------------------------------------------------------------------------------
	override bool CanEntryBeShown(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity)
	{
		if (!super.CanEntryBeShown(commanderFaction, playerId, position, hoveredEntity))
			return false;

		return SCR_EstablishBaseRequestedTaskEntity.Cast(hoveredEntity) != null;
	}

	//------------------------------------------------------------------------------------------------
	override void OnCommandIssued(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity, SCR_AIGroup group)
	{
		SCR_FactionCommanderPlayerComponent factionCommanderPlayerComponent = GetLocalCommanderComponent();
		if (!factionCommanderPlayerComponent)
			return;

		SCR_EstablishBaseRequestedTaskEntity requestTask = SCR_EstablishBaseRequestedTaskEntity.Cast(hoveredEntity);
		if (requestTask)
			factionCommanderPlayerComponent.CreateTask(m_sEstablishTaskPrefab, hoveredEntity.GetOrigin(), group, playerId);
	}
}
