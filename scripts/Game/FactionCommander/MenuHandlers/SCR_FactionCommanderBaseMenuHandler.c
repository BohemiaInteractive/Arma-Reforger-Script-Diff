[BaseContainerProps()]
class SCR_FactionCommanderBaseMenuHandler
{
	protected SCR_FactionCommanderMenuEntry m_MenuEntry;

	//------------------------------------------------------------------------------------------------
	void SetMenuEntry(SCR_FactionCommanderMenuEntry menuEntry)
	{
		m_MenuEntry = menuEntry;
	}

	//------------------------------------------------------------------------------------------------
	SCR_FactionCommanderMenuEntry GetMenuEntry()
	{
		return m_MenuEntry;
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_FactionCommanderPlayerComponent GetLocalCommanderComponent()
	{
		return SCR_FactionCommanderPlayerComponent.Cast(GetGame().GetPlayerController().FindComponent(SCR_FactionCommanderPlayerComponent));
	}

	//------------------------------------------------------------------------------------------------
	bool CanQuickEntryBeShown(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	bool CanEntryBeShown(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	bool CanEntryBeSelected(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity, out string disabledText = "")
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	bool CanGroupEntryBeShown(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity, SCR_AIGroup group)
	{
		if (!group || group.GetPlayerCount() == 0 || group.GetGroupRole() == SCR_EGroupRole.COMMANDER)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	bool CanGroupEntryBeSelected(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity, SCR_AIGroup group, out string disabledText = "")
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	void OnEntryHovered(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity);

	//------------------------------------------------------------------------------------------------
	void OnCommandIssued(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity, SCR_AIGroup group);
}
