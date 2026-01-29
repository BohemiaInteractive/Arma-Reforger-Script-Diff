[BaseContainerProps(), SCR_BaseContainerHintCondition()]
class SCR_ManualCameraNightModeHintCondition : SCR_BaseHintCondition
{		
	//------------------------------------------------------------------------------------------------
	protected SCR_NightModeGameModeComponent GetNightModeComponent()
	{
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return null;
		
		return SCR_NightModeGameModeComponent.Cast(gameMode.FindComponent(SCR_NightModeGameModeComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnEditorNightModeEnabled(bool enabled)
	{
		//~ Do not show hint on disabling night mode
		if (!enabled)
			return;
		
		SCR_NightModeGameModeComponent nightModeComponent = GetNightModeComponent();
		if (!nightModeComponent)
			return;
		
		//~ Only activate if global nightmode is not enabled
		if (!nightModeComponent.IsGlobalNightModeEnabled())
			Activate();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnInitCondition(Managed owner)
	{
		SCR_NightModeGameModeComponent nightModeComponent = GetNightModeComponent();
		if (!nightModeComponent)
			return;
		
		nightModeComponent.GetOnLocalEditorNightModeEnabledChanged().Insert(OnEditorNightModeEnabled);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnExitCondition(Managed owner)
	{
		SCR_NightModeGameModeComponent nightModeComponent = GetNightModeComponent();
		if (!nightModeComponent)
			return;
		
		nightModeComponent.GetOnLocalEditorNightModeEnabledChanged().Insert(OnEditorNightModeEnabled);
	}
}
