[BaseContainerProps()]
class SCR_FactionCommanderBaseTaskMenuHandler : SCR_FactionCommanderBaseMenuHandler
{
	//------------------------------------------------------------------------------------------------
	protected ResourceName GetTaskPrefab()
	{
		if (!m_MenuEntry)
			return ResourceName.Empty;

		SCR_FactionCommanderTaskRootMenuEntry taskRootMenuEntry = SCR_FactionCommanderTaskRootMenuEntry.Cast(m_MenuEntry);
		if (!taskRootMenuEntry)
			return ResourceName.Empty;

		return taskRootMenuEntry.GetTaskPrefab();
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsTaskValid(IEntity hoveredEntity)
	{
		SCR_Task task = SCR_Task.Cast(hoveredEntity);
		if (!task)
			return false;

		if (!m_MenuEntry)
			return false;

		SCR_FactionCommanderTaskRootMenuEntry taskRootMenuEntry = SCR_FactionCommanderTaskRootMenuEntry.Cast(m_MenuEntry);
		if (!taskRootMenuEntry)
			return false;

		return task.Type() == taskRootMenuEntry.GetTaskPrefabClassTypename();
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	typename GetTaskPrefabClassTypename()
	{
		if (!m_MenuEntry)
			return typename.Empty;

		SCR_FactionCommanderTaskRootMenuEntry taskRootMenuEntry = SCR_FactionCommanderTaskRootMenuEntry.Cast(m_MenuEntry);
		if (!taskRootMenuEntry)
			return typename.Empty;

		return taskRootMenuEntry.GetTaskPrefabClassTypename();
	}

	//------------------------------------------------------------------------------------------------
	override bool CanEntryBeShown(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity)
	{
		return IsTaskValid(hoveredEntity);
	}

	//------------------------------------------------------------------------------------------------
	override bool CanGroupEntryBeShown(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity, SCR_AIGroup group)
	{
		if (!super.CanGroupEntryBeShown(commanderFaction, playerId, position, hoveredEntity, group))
			return false;

		SCR_GroupTaskManagerComponent groupTaskManager = SCR_GroupTaskManagerComponent.GetInstance();
		if (!groupTaskManager)
			return true;

		SCR_GroupTaskRelevance relevance = groupTaskManager.GetGroupTaskRelevance(GetTaskPrefabClassTypename());
		if (!relevance)
			return true;

		return relevance.IsTaskExecutableByGroup(group);
	}
}
