//! Component to display budget progress
class SCR_BudgetUIComponent : SCR_BaseEditorUIComponent
{
	[Attribute("0", UIWidgets.ComboBox, "", enums: ParamEnumArray.FromEnum(EEditableEntityBudget))]
	protected EEditableEntityBudget m_BudgetType;
	
	[Attribute("")]
	protected string m_sBudgetProgressRoot;
	
	protected SCR_CampaignBuildingEditorComponent m_CampaignBuildingComponent;
	protected SCR_BudgetEditorComponent m_BudgetManager;
	protected SCR_BudgetProgressEditorUIComponent m_BudgetProgress;
	
	protected int m_iCurrentBudget;
	protected int m_iMaxBudget;
	protected const int INVALID_BUDGET = -1;
	
	//------------------------------------------------------------------------------------------------
	// Override
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttachedScripted(Widget w)
	{	
		super.HandlerAttachedScripted(w);
		
		// Widget
		Widget budgetRoot = w.FindAnyWidget(m_sBudgetProgressRoot);
		if (!budgetRoot)
			budgetRoot = w;
		
		m_CampaignBuildingComponent = SCR_CampaignBuildingEditorComponent.Cast(SCR_CampaignBuildingEditorComponent.GetInstance(SCR_CampaignBuildingEditorComponent));
		m_BudgetProgress = SCR_BudgetProgressEditorUIComponent.Cast(budgetRoot.FindHandler(SCR_BudgetProgressEditorUIComponent));
		
		// Setup budget 
		m_BudgetManager = SCR_BudgetEditorComponent.Cast(SCR_BudgetEditorComponent.GetInstance(SCR_BudgetEditorComponent, false, true));
		if (m_BudgetManager)
		{
			InitializeBudgets();
			m_BudgetManager.Event_OnBudgetUpdated.Insert(OnBudgetUpdate);
			m_BudgetManager.Event_OnBudgetMaxUpdated.Insert(OnBudgetMaxUpdate);
			m_BudgetManager.Event_OnBudgetPreviewUpdated.Insert(OnBudgetPreviewUpdate);
			m_BudgetManager.Event_OnBudgetPreviewReset.Insert(ResetWidgetPreviewData);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		if (!m_BudgetManager)
			return;	
		
		m_BudgetManager.Event_OnBudgetUpdated.Remove(OnBudgetUpdate);
		m_BudgetManager.Event_OnBudgetMaxUpdated.Remove(OnBudgetMaxUpdate);
		m_BudgetManager.Event_OnBudgetPreviewUpdated.Remove(OnBudgetPreviewUpdate);
		m_BudgetManager.Event_OnBudgetPreviewReset.Remove(ResetWidgetPreviewData);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool IsUnique()
	{
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	// Protected
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	protected void InitializeBudgets()
	{
		if (!m_CampaignBuildingComponent || !m_BudgetManager)
			return;
		
		SCR_CampaignBuildingProviderComponent providerComponent = m_CampaignBuildingComponent.GetProviderComponent();
		if (!providerComponent)
			return;
		
		EEditableEntityBudget budgetToShow = m_CampaignBuildingComponent.GetShownBudget();
		if (budgetToShow == INVALID_BUDGET)
		{
			m_BudgetProgress.GetRootWidget().SetVisible(false);
			return;
		}
			
		SetBudgetType(budgetToShow);
		
		// Setup values 
		m_iCurrentBudget = m_BudgetManager.GetCurrentBudgetValue(m_BudgetType);
		m_BudgetManager.GetMaxBudgetValue(m_BudgetType, m_iMaxBudget);
		
		if (!m_CampaignBuildingComponent)
			return;
		
		m_BudgetProgress.HideBudgetChange();
		
		if (m_iMaxBudget != 0)
			m_BudgetProgress.ShowBudget(m_iCurrentBudget / m_iMaxBudget);
	}
	
	//------------------------------------------------------------------------------------------------
	// Callback
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	protected void OnBudgetUpdate(EEditableEntityBudget budgetType, int originalBudgetValue, int updatedBudgetValue, int maxBudgetValue)
	{
		m_iCurrentBudget = m_BudgetManager.GetCurrentBudgetValue(m_BudgetType);
		m_BudgetManager.GetMaxBudgetValue(m_BudgetType, m_iMaxBudget);
		
		m_BudgetProgress.HideBudgetChange();
		m_BudgetProgress.ShowBudget(m_iCurrentBudget / m_iMaxBudget);
	}
		
	//------------------------------------------------------------------------------------------------
	protected void OnBudgetMaxUpdate(EEditableEntityBudget budgetType, int currentBudgetValue, int maxBudgetValue)
	{
		m_BudgetManager.GetMaxBudgetValue(m_BudgetType, m_iMaxBudget);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnBudgetPreviewUpdate(EEditableEntityBudget budgetType, float previewBudgetValue, float budgetChange)
	{
		if (m_BudgetType != budgetType)
			return;
		
		m_BudgetProgress.ShowBudgetChange(budgetChange, previewBudgetValue);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ResetWidgetPreviewData()
	{
		m_BudgetProgress.HideBudgetChange();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Load icon of given budget from budgetUI and set it.
	protected void SetBudgetUIIcon(EEditableEntityBudget budgetType)
	{
		if (!m_BudgetManager || !m_BudgetProgress)
			return;
		
		SCR_EditableEntityCoreBudgetSetting budgetSettings = m_BudgetManager.GetBudgetSetting(budgetType);
		if (!budgetSettings)
			return;
		
		SCR_UIInfo info = budgetSettings.GetInfo();
		if (!info)
			return;

		info.SetIconTo(m_BudgetProgress.GetIconWidhget());
	}
	
	//------------------------------------------------------------------------------------------------
	// Public 
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Set a budget which will will be used in UI of provider detail.
	void SetBudgetType(EEditableEntityBudget type)
	{
		m_BudgetType = type;
		SetBudgetUIIcon(m_BudgetType);
	}
	
	//------------------------------------------------------------------------------------------------
	EEditableEntityBudget GetBudgetType()
	{
		return m_BudgetType;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_BudgetProgressEditorUIComponent GetUIComponent()
	{
		return m_BudgetProgress;
	}
}