[BaseContainerProps(), SCR_BaseContainerHintCondition()]
class SCR_CampaignBuildingCloseHintCondition: SCR_BaseEditorHintCondition
{		
	//------------------------------------------------------------------------------------------------
	override protected void OnExitConditionEditor(SCR_EditorManagerEntity editorManager)
	{
		// Call later because closing editor might hide the hint.
		GetGame().GetCallqueue().CallLater(Activate, 100, false);
	}
}
