[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EGameOverTypes, "m_GameOverScreenId")]
class SCR_DeathMatchGameOverScreenInfo: SCR_BaseGameOverScreenInfo
{	
	[Attribute("1", desc: "If true gets name of otherPlayer to be subtitle param, else gets name of Player")]
	protected bool m_bGetWinningSubtitleParam;
	
	//Get winning faction name
	override string GetSubtitleParam(int playerId, array<int> otherPlayerIds)
	{
		PlayerManager playerManager = GetGame().GetPlayerManager();
		
		if (!playerManager)
			return super.GetSubtitleParam(playerId, otherPlayerIds);
		
		if (m_bGetWinningSubtitleParam)
			return SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(otherPlayerIds[0]);
		else 
			return SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(playerId);
	}
};