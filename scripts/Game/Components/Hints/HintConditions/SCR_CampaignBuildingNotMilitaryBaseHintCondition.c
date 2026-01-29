[BaseContainerProps(), SCR_BaseContainerHintCondition()]
class SCR_CampaignBuildingNotMilitaryBaseHintCondition : SCR_BaseEditorHintCondition
{	
	//------------------------------------------------------------------------------------------------
	override protected void OnInitConditionEditor(SCR_EditorManagerEntity editorManager)
	{
		if (IsRegisteredAtMilitaryBase())
			return;
		
		editorManager.GetOnOpened().Insert(Activate);
		editorManager.GetOnClosed().Insert(Deactivate);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnExitConditionEditor(SCR_EditorManagerEntity editorManager)
	{
		if (IsRegisteredAtMilitaryBase())
			return;
		
		editorManager.GetOnOpened().Remove(Activate);
		editorManager.GetOnClosed().Remove(Deactivate);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check if the provider can register at military base. 
	bool IsRegisteredAtMilitaryBase()
	{
		SCR_CampaignBuildingEditorComponent CampaignBuildingEditorComponent = SCR_CampaignBuildingEditorComponent.Cast(SCR_CampaignBuildingEditorComponent.GetInstance(SCR_CampaignBuildingEditorComponent));
		if (!CampaignBuildingEditorComponent)
			return false;
		
		SCR_CampaignBuildingProviderComponent providerComponent = CampaignBuildingEditorComponent.GetProviderComponent();
		if (!providerComponent)
			return false;
		
		return providerComponent.CanRegisterAtMilitaryBase();
	}
}
