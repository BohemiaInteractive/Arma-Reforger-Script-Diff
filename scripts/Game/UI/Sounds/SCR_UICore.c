//------------------------------------------------------------------------------------------------
[BaseContainerProps(configRoot: true)]
class SCR_UICore: SCR_GameCoreBase
{
	[Attribute("", UIWidgets.ResourceNamePicker, "UI sounds entity", "et")]
	private ResourceName m_UISoundEntityPrefab;
	
	private static SCR_UICore s_Instance;
	
	//------------------------------------------------------------------------------------------------
	//! Spawn entities for WidgetAnimator, SCR_UISoundsEntity and SCR_PlayerNamesFilterCache
	override void OnGameStart()
	{
		if (!s_Instance)
			s_Instance = this;
		
		if (!SCR_UISoundEntity.GetInstance())
			GetGame().SpawnEntityPrefab(Resource.Load(m_UISoundEntityPrefab));
		
		SCR_PlayerNamesFilterCache.GetInstance();
		
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_UI_SHOW_ALL_SETTINGS, "", "Show all settings", "UI");
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnGameEnd()
	{
		SCR_PlayerNamesFilterCache.GetInstance().Destroy();
		
		if (s_Instance == this)
			s_Instance = null;
	}
	
}
