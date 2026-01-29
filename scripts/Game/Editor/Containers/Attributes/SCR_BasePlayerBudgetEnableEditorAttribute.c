
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_BasePlayerBudgetEnableEditorAttribute: SCR_BaseEditorAttribute
{	
	//~ ToDo: this is temporary for when the budgets get a Init value function
	[Attribute(desc: "The budget Value if in Multiplayer when budget is enabled")]
	protected int m_iBudgetValueMP;
	
	[Attribute(desc: "The budget Value if in Singleplayer when budget is enabled")]
	protected int m_iBudgetValueSP;
	
	protected SCR_EditorManagerEntity m_EditorManager;
	
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
	
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{				 
		SetEditorManager(item);
		if (!m_EditorManager) 
			return null;
		
		SCR_BudgetEditorComponent budgetComponent = SCR_BudgetEditorComponent.Cast(m_EditorManager.FindComponent(SCR_BudgetEditorComponent));
		if (!budgetComponent) 
			return null;
		
		int maxBudget;
		budgetComponent.GetMaxBudgetValue(GetBudgetType(), maxBudget);

		return SCR_BaseEditorAttributeVar.CreateBool(maxBudget > 0);
	}
	
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{		
		if (!var) 
			return;
		
		if (!m_EditorManager)
			SetEditorManager(item);
		
		SCR_EditablePlayerDelegateComponent delegate = SCR_EditablePlayerDelegateComponent.Cast(item);
		if (!delegate) 
			return;
		
		if (!m_EditorManager) 
			return;
		
		SCR_BudgetEditorComponent budgetComponent = SCR_BudgetEditorComponent.Cast(m_EditorManager.FindComponent(SCR_BudgetEditorComponent));
		if (!budgetComponent) 
			return;
		
		if (var.GetBool())		
		{
			int budgetValue;
			
			if (Replication.IsRunning())
				budgetValue = m_iBudgetValueMP;
			else 
				budgetValue = m_iBudgetValueSP;
			
			budgetComponent.SetMaxBudgetValue(GetBudgetType(), budgetValue);
			SCR_NotificationsComponent.SendToUnlimitedEditorPlayersAndPlayer(playerID, GetEnableNotification(), playerID, delegate.GetPlayerID());
		}
		else 
		{
			budgetComponent.SetMaxBudgetValue(GetBudgetType(), 0);
			SCR_NotificationsComponent.SendToUnlimitedEditorPlayersAndPlayer(playerID, GetDisableNotification(), playerID, delegate.GetPlayerID());
		}
	}
	
	protected EEditableEntityBudget GetBudgetType()
	{
		return EEditableEntityBudget.SYSTEMS;
	}
	
	protected ENotification GetEnableNotification()
	{
		return ENotification.UNKNOWN;
	}
	
	protected ENotification GetDisableNotification()
	{
		return ENotification.UNKNOWN;
	}
};