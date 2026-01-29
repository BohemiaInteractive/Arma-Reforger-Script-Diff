//! @ingroup ManualCamera

//! Hide player HUD when the camera is active.
[BaseContainerProps(), SCR_BaseManualCameraComponentTitle()]
class SCR_HideHUDManualCameraComponent : SCR_BaseManualCameraComponent
{
	[Attribute("", UIWidgets.Flags, enums: ParamEnumArray.FromEnum(EHudLayers))]
	private EHudLayers m_LayersWhitelist;
	
	//------------------------------------------------------------------------------------------------
	override bool EOnCameraInit()
	{
		SCR_HUDManagerComponent hudManager = GetGame().GetHUDManager();
		if (!hudManager) return false;
		
		hudManager.SetVisibleLayers(m_LayersWhitelist);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void EOnCameraExit()
	{
		SCR_HUDManagerComponent hudManager = GetGame().GetHUDManager();
		if (!hudManager) return;
		
		hudManager.SetVisibleLayers();
	}
}
