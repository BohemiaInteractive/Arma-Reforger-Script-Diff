[BaseContainerProps()]
class SCR_TutorialLogic_Navigation : SCR_BaseTutorialCourseLogic
{
	bool m_bMarkerCreated;
	ResourceName m_sCompassResourceName = "{61D4F80E49BF9B12}Prefabs/Items/Equipment/Compass/Compass_SY183.et";
	ResourceName m_sMapResourceName = "{13772C903CB5E4F7}Prefabs/Items/Equipment/Maps/PaperMap_01_folded.et";
	SCR_TutorialGamemodeComponent m_TutorialComponent;
	
	//------------------------------------------------------------------------------------------------
	override void OnCourseStart()
	{
		m_TutorialComponent = SCR_TutorialGamemodeComponent.GetInstance();
		
		GetGame().GetCallqueue().CallLater(CheckForNavigationItems, 250, true);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnCourseEnd()
	{
		GetGame().GetCallqueue().Remove(CheckForNavigationItems);
		
		if (!m_TutorialComponent)
			return;
		
		IEntity vehicle = GetGame().GetWorld().FindEntityByName("SmallJeep2");
		if (vehicle)
			m_TutorialComponent.ChangeVehicleLockState(vehicle, true);
	}

	//------------------------------------------------------------------------------------------------
	void CheckForNavigationItems()
	{
		if (!m_TutorialComponent)
			return;
		
		SCR_BaseTutorialStage stage = m_TutorialComponent.GetCurrentStage();
		if (!stage)
			return;
		
		if (!m_TutorialComponent.FindPrefabInPlayerInventory(m_sCompassResourceName))
			stage.RegisterWaypoint("Navigation_Compass", "", "PICKUP");
		else
			stage.UnregisterWaypoint("Navigation_Compass");
		
		if (!m_TutorialComponent.FindPrefabInPlayerInventory(m_sMapResourceName))
			stage.RegisterWaypoint("Navigation_Map", "", "PICKUP");
		else
			stage.UnregisterWaypoint("Navigation_Map");
	}
}