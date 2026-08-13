class SCR_ChangeRankPlayerListAction : SCR_PlayerListBaseAction
{
	[Attribute(defvalue: "1", uiwidget: UIWidgets.ComboBox, desc: "Determines if action should result in player getting promoted or demoted", params: "-1 1", enums: {ParamEnum("Demote", "-1"), ParamEnum("Promote", "1")})]
	protected int m_iChangeType;

	//------------------------------------------------------------------------------------------------
	override bool CanBeShown(int targetPlayerId, int userPlayerId, SCR_PlayerController userPlayerController)
	{
		if (!SCR_XPHandlerComponent.IsXpSystemEnabled())
			return false;

		EPlayerRole userRoleFlags = GetGame().GetPlayerManager().GetPlayerRoles(userPlayerId);
		if (!(userRoleFlags & EPlayerRole.ADMINISTRATOR || userRoleFlags & EPlayerRole.SESSION_ADMINISTRATOR))
			return false;

		SCR_Faction faction = SCR_Faction.Cast(SCR_FactionManager.SGetPlayerFaction(targetPlayerId));
		return faction != null;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformed(int targetPlayerId, int userPlayerId, SCR_PlayerController userPlayerController)
	{
		if (!SCR_XPHandlerComponent.IsXpSystemEnabled())
			return false;

		PlayerManager playerMgr = GetGame().GetPlayerManager();
		EPlayerRole userRoleFlags = playerMgr.GetPlayerRoles(userPlayerId);
		if (!(userRoleFlags & EPlayerRole.ADMINISTRATOR || userRoleFlags & EPlayerRole.SESSION_ADMINISTRATOR))
			return false;

		if (!Replication.IsServer())
			return true;

		PlayerController targetPlayerController = playerMgr.GetPlayerController(targetPlayerId);
		if (!targetPlayerController)
			return false;

		SCR_PlayerXPHandlerComponent xpHandlerComp = SCR_PlayerXPHandlerComponent.Cast(targetPlayerController.FindComponent(SCR_PlayerXPHandlerComponent));
		if (!xpHandlerComp)
			return false;

		SCR_ECharacterRank currentTargetRank = xpHandlerComp.GetPlayerRankByXP();
		if (currentTargetRank == SCR_ECharacterRank.INVALID)
			return false;

		SCR_Faction faction = SCR_Faction.Cast(SCR_FactionManager.SGetPlayerFaction(targetPlayerId));
		if (!faction)
			return false;

		SCR_ECharacterRank newRank;
		if (m_iChangeType < 0)
			newRank = faction.GetRanks().GetPreviousRank(currentTargetRank);
		else
			newRank = faction.GetRanks().GetNextRank(currentTargetRank);

		return newRank != SCR_ECharacterRank.INVALID;
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(int targetPlayerId, int userPlayerId, SCR_PlayerController userPlayerController)
	{
		PlayerController targetPlayerController = GetGame().GetPlayerManager().GetPlayerController(targetPlayerId);
		if (!targetPlayerController)
			return;

		SCR_PlayerXPHandlerComponent xpHandlerComp = SCR_PlayerXPHandlerComponent.Cast(targetPlayerController.FindComponent(SCR_PlayerXPHandlerComponent));
		if (xpHandlerComp)
			xpHandlerComp.CheatRank_S(m_iChangeType < 0);
	}
}
