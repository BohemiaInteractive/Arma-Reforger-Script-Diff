[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionUnlockAchievement : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Player getter")];
	ref SCR_ScenarioFrameworkGet m_PlayerGetter;
	
	[Attribute("0", UIWidgets.SearchComboBox, "Achievement to unlock", "", ParamEnumArray.FromEnum(AchievementId))]
	AchievementId m_eAchievementToUnlock;
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		IEntity entity;
		if (!ValidateInputEntity(object, m_PlayerGetter, entity))
		{
			PrintFormat("ScenarioFramework Action: Entity not found for Action %1.", this, level:LogLevel.ERROR);
			return;
		}
		
		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!playerManager)
			return;
		
		int playerId = playerManager.GetPlayerIdFromControlledEntity(entity);

		PlayerController playerController = playerManager.GetPlayerController(playerId);
		if (!playerController)
		{
			PrintFormat("ScenarioFramework Action: Entity provided for Action %1 is not player character.", this, level:LogLevel.ERROR);
			return;
		}
		
		SCR_AchievementsHandler handler = SCR_AchievementsHandler.Cast(playerController.FindComponent(SCR_AchievementsHandler));
		if (!handler)
			return;
	
		handler.UnlockAchievement(m_eAchievementToUnlock);
	}
}