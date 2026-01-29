class SCR_CampaignBuildingBudgetEditorUIComponent : SCR_BudgetEditorUIComponent
{	
	[Attribute(desc: "Budgets to be shown in UI", UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EEditableEntityBudget))]
	protected ref array<EEditableEntityBudget> m_aVisibleBudgetUI;
	
	protected static const int UNLIMITED_PROP_BUDGET = -1;
	
	//------------------------------------------------------------------------------------------------
	override void InitializeBudgets()
	{		
		array<ref SCR_EditableEntityCoreBudgetSetting> budgets = {};
		m_BudgetManager.GetBudgets(budgets);
		
		foreach (SCR_EditableEntityCoreBudgetSetting budget : budgets)
		{
			if (!m_aVisibleBudgetUI.Contains(budget.GetBudgetType()))
				continue;
						
			int MaxBudgetValue = GetMaxBudgetValue(budget.GetBudgetType());
			
			if (MaxBudgetValue == UNLIMITED_PROP_BUDGET)
			{
				UnregisterEvents();
				return;
			}
			
			OnBudgetAdd(budget, MaxBudgetValue);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnBudgetAdd(SCR_EditableEntityCoreBudgetSetting budget, int maxBudget)
	{
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace || !m_Layout || m_BudgetEntryPrefab.IsEmpty())
			return;
		
		EEditableEntityBudget budgetType = budget.GetBudgetType();
		if (m_BudgetWidgets.Contains(budgetType))
			return;
		
		Widget budgetWidget = workspace.CreateWidgets(m_BudgetEntryPrefab, m_Layout);
		SetBudgetData(budgetWidget, GetCurrentBudgetValue(), maxBudget, budget.GetInfo());
		
		m_BudgetWidgets.Insert(budgetType, budgetWidget);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnBudgetUpdate(EEditableEntityBudget budgetType, int originalBudgetValue, int updatedBudgetValue, int maxBudgetValue)
	{
		TextWidget budgetWidget;
		if (m_BudgetWidgets.Find(budgetType, budgetWidget))
		{
			SetBudgetData(budgetWidget, GetCurrentBudgetValue(), maxBudgetValue);
			SetBudgetPreviewData(budgetWidget, 0, 0);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get current provider component
	SCR_CampaignBuildingProviderComponent GetProviderComponent()
	{
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core)
			return null;

		SCR_EditorManagerEntity editorManager = core.GetEditorManager();
		if (!editorManager)
			return null;
		
		SCR_EditorModeEntity modeEntity = editorManager.FindModeEntity(EEditorMode.BUILDING);
		if (!modeEntity)
			return null;

		SCR_CampaignBuildingEditorComponent editorComponent = SCR_CampaignBuildingEditorComponent.Cast(modeEntity.FindComponent(SCR_CampaignBuildingEditorComponent));
		if (!editorComponent)
			return null;
		
		return editorComponent.GetProviderComponent();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return max budget value from provider
	int GetMaxBudgetValue(EEditableEntityBudget budget)
	{
		SCR_CampaignBuildingProviderComponent provider = GetProviderComponent();
		if (!provider)
			return UNLIMITED_PROP_BUDGET;
		
		return provider.GetMaxBudgetValue(budget);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return current budget value from provider
	int GetCurrentBudgetValue()
	{
		SCR_CampaignBuildingProviderComponent provider = GetProviderComponent();
		if (!provider)
			return UNLIMITED_PROP_BUDGET;
		
		return provider.GetCurrentPropValue();
	}
}
