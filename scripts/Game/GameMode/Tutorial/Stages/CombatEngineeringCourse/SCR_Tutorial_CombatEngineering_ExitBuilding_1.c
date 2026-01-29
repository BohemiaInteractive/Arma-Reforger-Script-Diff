[EntityEditorProps(insertable: false)]
class SCR_Tutorial_CombatEngineering_ExitBuilding_1Class : SCR_BaseTutorialStageClass
{
}

class SCR_Tutorial_CombatEngineering_ExitBuilding_1 : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		IEntity service = GetGame().GetWorld().FindEntityByName("VEHICLE_REQUESTING_BOARD");
		if (service)
		{
			SCR_CampaignBuildingProviderComponent buildingComponent = SCR_CampaignBuildingProviderComponent.Cast(service.FindComponent(SCR_CampaignBuildingProviderComponent));
			if (buildingComponent)
				buildingComponent.SetPlayerCooldown(1, 3600);
		}
		
		GetGame().GetCallqueue().CallLater(ToggleEditor, 3600000);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{		
		return !IsBuildingModeOpen();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ToggleEditor()
	{
		if (!IsBuildingModeOpen())
			return;
		
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core)
			return;
		
		SCR_EditorManagerEntity editorManager = core.GetEditorManager();
		if (editorManager)
			editorManager.Toggle();
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_Tutorial_CombatEngineering_ExitBuilding_1()
	{
		GetGame().GetCallqueue().Remove(ToggleEditor);
		
		if (m_TutorialComponent)
		{
			BaseWorld world = GetGame().GetWorld();
			if (!world)
				return;
			
			m_TutorialComponent.BlockBuildingModeAccess(world.FindEntityByName("VEHICLE_REQUESTING_BOARD"), true);
		}
	}
}