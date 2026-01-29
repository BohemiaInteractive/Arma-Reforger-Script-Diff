/**
Modes and Budget attribute for getting and setting varriables in Editor Attribute window
*/
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_BasePlayerBudgetEditorAttribute: SCR_BaseValueListEditorAttribute
{	
	protected SCR_EditorManagerEntity m_EditorManager;
	
	//------------------------------------------------------------------------------------------------
	protected void SetEditorManager(Managed item)
	{	
		m_EditorManager = null;
		
		SCR_EditablePlayerDelegateComponent delegate = SCR_EditablePlayerDelegateComponent.Cast(item);
		if (!delegate) 
			return;
		
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core) 
			return;
		
		m_EditorManager = core.GetEditorManager(delegate.GetPlayerID());
	}
	
	//---- REFACTOR NOTE START: Mot used as it's just returning null ----
	
	//------------------------------------------------------------------------------------------------
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{		
		//~ Todo: Disabled for now until serverwide budgets can be set
		return null;
				 
		SetEditorManager(item);
		if (!m_EditorManager) 
			return null;
		
		SCR_BudgetEditorComponent budgetComponent = SCR_BudgetEditorComponent.Cast(m_EditorManager.FindComponent(SCR_BudgetEditorComponent));
		if (!budgetComponent) 
			return null;
		
		int maxBudget;
		budgetComponent.GetMaxBudgetValue(GetBudgetType(), maxBudget);

		return SCR_BaseEditorAttributeVar.CreateFloat(maxBudget);
	}
	
	//---- REFACTOR NOTE END ----
	
	//------------------------------------------------------------------------------------------------
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{		
		if (!var) 
			return;
		
		if (!m_EditorManager)
			SetEditorManager(item);
		if (!m_EditorManager) 
			return;
		
		SCR_BudgetEditorComponent budgetComponent = SCR_BudgetEditorComponent.Cast(m_EditorManager.FindComponent(SCR_BudgetEditorComponent));
		if (!budgetComponent) 
			return;
		
		budgetComponent.DelayedSetMaxBudgetSetup(GetBudgetType(), var.GetFloat(), playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	protected EEditableEntityBudget GetBudgetType()
	{
		return EEditableEntityBudget.SYSTEMS;
	}
};