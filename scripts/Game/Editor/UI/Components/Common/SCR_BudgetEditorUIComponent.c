class SCR_BudgetEditorUIComponent : SCR_BaseEditorUIComponent
{	
	[Attribute("0.1")]
	protected float m_fDisabledOpacity;
	
	const string WIDGET_BUDGET_TEXT = "PercentageText";
	const string WIDGET_BUDGET_ICON = "Icon";
	const string WIDGET_BUDGET_PROGRESSBAR = "RadialProgressBar";
	const string WIDGET_BUDGETPREVIEW_TEXT = "Preview";
	const string WIDGET_ICON_AREA = "IconArea";
	const string WIDGET_LOCK_VISUALS = "LockVisuals";
	
	protected SCR_BudgetEditorComponent m_BudgetManager;
	protected ResourceName m_BudgetEntryPrefab;
	
	protected Widget m_Layout;
	
	protected ref map<EEditableEntityBudget, Widget> m_BudgetWidgets = new map<EEditableEntityBudget, Widget>();
	
	//------------------------------------------------------------------------------------------------
	protected void OnBudgetMaxUpdate(EEditableEntityBudget budgetType, int currentBudgetValue, int maxBudgetValue)
	{
		Widget budgetWidget;
		if (m_BudgetWidgets.Find(budgetType, budgetWidget))
		{
			SetBudgetData(budgetWidget, currentBudgetValue, maxBudgetValue);
			SetBudgetPreviewData(budgetWidget, 0, 0);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnBudgetUpdate(EEditableEntityBudget budgetType, int originalBudgetValue, int updatedBudgetValue, int maxBudgetValue)
	{
		Widget budgetWidget;
		if (m_BudgetWidgets.Find(budgetType, budgetWidget))
		{
			SetBudgetData(budgetWidget, updatedBudgetValue, maxBudgetValue);
			SetBudgetPreviewData(budgetWidget, 0, 0);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnBudgetAdd(SCR_EditableEntityCoreBudgetSetting budget, int maxBudget)
	{
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace || !m_Layout || m_BudgetEntryPrefab.IsEmpty())
			return;
		
		EEditableEntityBudget budgetType = budget.GetBudgetType();
		if (m_BudgetWidgets.Contains(budgetType))
			return;
		
		Widget budgetWidget = workspace.CreateWidgets(m_BudgetEntryPrefab, m_Layout);
		SetBudgetData(budgetWidget, budget.GetCurrentBudget(), maxBudget, budget.GetInfo());
		
		m_BudgetWidgets.Insert(budgetType, budgetWidget);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnBudgetPreviewUpdate(EEditableEntityBudget budgetType, float previewBudgetValue, float budgetChange)
	{
		Widget budgetWidget;
		if (m_BudgetWidgets.Find(budgetType, budgetWidget))
			SetBudgetPreviewData(budgetWidget, previewBudgetValue, budgetChange);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void InitializeBudgets()
	{
		array<ref SCR_EditableEntityCoreBudgetSetting> budgets = {};
		m_BudgetManager.GetBudgets(budgets);
		
		foreach (SCR_EditableEntityCoreBudgetSetting budget : budgets)
		{
			int maxBudget;
			m_BudgetManager.GetMaxBudgetValue(budget.GetBudgetType(), maxBudget);
			
			OnBudgetAdd(budget, maxBudget);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetBudgetData(Widget budgetWidget, int budgetValue, int maxBudgetValue, SCR_UIInfo info = null)
	{
		float budgetProgress;
		if (maxBudgetValue > 0)
			budgetProgress = budgetValue / (float) maxBudgetValue;
		else
			budgetProgress = 1;
		
		TextWidget budgetValueText = TextWidget.Cast(budgetWidget.FindAnyWidget(WIDGET_BUDGET_TEXT));
		if (budgetValueText)
			budgetValueText.SetTextFormat("#AR-ValueUnit_Percentage", (budgetProgress * 100).ToString(-1, 0));
		
		bool isBudgetEnabled = maxBudgetValue > 0;
		
		//~ TODO: Currently budget percentage is hidden. This should still show the server percentage
		if (budgetValueText)
			budgetValueText.SetVisible(isBudgetEnabled);
		
		Widget lockVisuals = budgetWidget.FindAnyWidget(WIDGET_LOCK_VISUALS);
		if (lockVisuals)
			lockVisuals.SetVisible(!isBudgetEnabled);
		
		Widget iconVisuals = budgetWidget.FindAnyWidget(WIDGET_ICON_AREA);
		if (iconVisuals)
		{
			if (!isBudgetEnabled)
				iconVisuals.SetOpacity(m_fDisabledOpacity);
			else 
				iconVisuals.SetOpacity(1);
		}
		
		if (info)
		{
			ImageWidget budgetIcon = ImageWidget.Cast(budgetWidget.FindAnyWidget(WIDGET_BUDGET_ICON));
			info.SetIconTo(budgetIcon);
			SCR_LinkTooltipTargetEditorUIComponent.SetInfo(budgetWidget, info);
		}
		
		Widget progressBarWidget = budgetWidget.FindAnyWidget(WIDGET_BUDGET_PROGRESSBAR);
		if (!progressBarWidget)
			return;
		
		SCR_RadialProgressBarUIComponent progressBar = SCR_RadialProgressBarUIComponent.Cast(progressBarWidget.FindHandler(SCR_RadialProgressBarUIComponent));
		if (!progressBar)
			return;
		
		if (isBudgetEnabled)
			progressBar.SetProgress(budgetProgress);
		else 
			progressBar.SetProgress(0);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetBudgetPreviewData(Widget w, float previewBudgetValue, float budgetChange)
	{		
		TextWidget previewText = TextWidget.Cast(w.FindAnyWidget(WIDGET_BUDGETPREVIEW_TEXT));
		
		Widget progressBarWidget = w.FindAnyWidget(WIDGET_BUDGET_PROGRESSBAR);
		if (!progressBarWidget)
			return;
		
		SCR_RadialProgressBarUIComponent progressBar = SCR_RadialProgressBarUIComponent.Cast(progressBarWidget.FindHandler(SCR_RadialProgressBarUIComponent));
		if (!progressBar)
			return;
		
		if (previewText && budgetChange != 0)
		{
			previewText.SetVisible(true);
			
			// If budget change is <1%, only show +, once progress bars are implemented this will simply make the bar red
			bool isLessThenOne = false;
			string amount;
			if (budgetChange > 0 && budgetChange < 1)
			{
				isLessThenOne = true;
				amount = "1";
			}
			else
			{
				amount = budgetChange.ToString(-1, 0);
			}
			
			if (!isLessThenOne)
				previewText.SetTextFormat("#AR-ValueUnit_Percentage_Add", amount);
			else 
				previewText.SetTextFormat("#AR-ValueUnit_Percentage_AddLessThen", amount);
			
			if (progressBar)
				progressBar.SetPreviewProgress(previewBudgetValue * 0.01);
		}
		else if (previewText)
		{
			previewText.SetVisible(false);
			
			if(progressBar)
				progressBar.SetPreviewProgress(0);			
		}
	}
	
	//------------------------------------------------------------------------------------------------
	private void ResetWidgetPreviewData()
	{
		Widget budgetWidget = m_Layout.GetChildren();
		int i = 0;
		while (budgetWidget && i++ < 100)
		{ 
			SetBudgetPreviewData(budgetWidget, 0, 0);
			budgetWidget = budgetWidget.GetSibling();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UnregisterEvents()
	{
		if (!m_BudgetManager)
			return;
		
		m_BudgetManager.Event_OnBudgetUpdated.Remove(OnBudgetUpdate);
		m_BudgetManager.Event_OnBudgetMaxUpdated.Remove(OnBudgetMaxUpdate);
		m_BudgetManager.Event_OnBudgetPreviewUpdated.Remove(OnBudgetPreviewUpdate);
		m_BudgetManager.Event_OnBudgetPreviewReset.Remove(ResetWidgetPreviewData);
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttachedScripted(Widget w)
	{
		m_Layout = w;
		
		m_BudgetEntryPrefab = SCR_LayoutTemplateComponent.GetLayout(m_Layout);
		if (m_BudgetEntryPrefab.IsEmpty())
			return;
		
		// Remove any existing budget widgets, used for configuring UI layouts
		Widget debugBudgetWidget = m_Layout.GetChildren();
		int i = 0;
		while(debugBudgetWidget && i++ < 100)
		{
			Widget debugFilterSibling = debugBudgetWidget.GetSibling();
			debugBudgetWidget.RemoveFromHierarchy();
			debugBudgetWidget = debugFilterSibling;
		}
		
		m_BudgetManager = SCR_BudgetEditorComponent.Cast(SCR_BudgetEditorComponent.GetInstance(SCR_BudgetEditorComponent, false, true));
		if (m_BudgetManager)
		{
			InitializeBudgets();
			m_BudgetManager.Event_OnBudgetUpdated.Insert(OnBudgetUpdate);
			m_BudgetManager.Event_OnBudgetMaxUpdated.Insert(OnBudgetMaxUpdate);
			m_BudgetManager.Event_OnBudgetPreviewUpdated.Insert(OnBudgetPreviewUpdate);
			m_BudgetManager.Event_OnBudgetPreviewReset.Insert(ResetWidgetPreviewData);
		}

		if (m_BudgetManager)
			m_BudgetManager.DemandBudgetUpdateFromServer();
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		m_BudgetWidgets.Clear();
		
		UnregisterEvents();
	}
}
