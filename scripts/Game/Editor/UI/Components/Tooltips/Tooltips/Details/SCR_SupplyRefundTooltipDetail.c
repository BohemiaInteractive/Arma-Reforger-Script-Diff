
[BaseContainerProps(), BaseContainerCustomTitleField("m_sDisplayName")]
class SCR_SupplyRefundTooltipDetail : SCR_EntityTooltipDetail
{
	//------------------------------------------------------------------------------------------------
	override bool InitDetail(SCR_EditableEntityComponent entity, Widget widget)
	{		
		TextWidget text = TextWidget.Cast(widget.FindAnyWidget("Text"));
		if (!text)
			return false;
			
		BaseGameMode gameMode = GetGame().GetGameMode();	
		if (!gameMode)
			return false;
		
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (!editorManager || editorManager.GetCurrentMode() != EEditorMode.BUILDING)
			return false;
		
		
		SCR_CampaignBuildingManagerComponent buildingManager = SCR_CampaignBuildingManagerComponent.Cast(gameMode.FindComponent(SCR_CampaignBuildingManagerComponent));
		if (!buildingManager)
			return false;
		
		SCR_EditableEntityUIInfo editableUiInfo = SCR_EditableEntityUIInfo.Cast(entity.GetInfo());
		if (!editableUiInfo)
			return false;
		
		array<ref SCR_EntityBudgetValue> budgets = {};
		if (!editableUiInfo.GetEntityBudgetCost(budgets) || budgets.IsEmpty())
			return false;
		
		int conflictBudget = 0;
		
		foreach (SCR_EntityBudgetValue budget: budgets)
		{
			if (budget.GetBudgetType() == EEditableEntityBudget.CAMPAIGN)
				conflictBudget += budget.GetBudgetValue();
		}
		
		if (conflictBudget <= 0)
			return false;
		
		//~ Calculate refund amound
		text.SetText(Math.Round(conflictBudget * (buildingManager.GetCompositionRefundPercentage() * 0.01)).ToString());
		
		return true;
	}
}
