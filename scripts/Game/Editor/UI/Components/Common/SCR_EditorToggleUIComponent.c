//! @ingroup Editor_UI Editor_UI_Components

class SCR_EditorToggleUIComponent : ScriptedWidgetComponent
{
	[Attribute()]
	protected string m_sProgressWidgetName;
	
	[Attribute()]
	protected string m_sInitLoadingWidgetName;
	
	[Attribute()]
	protected string m_sLoadingWidgetName;
	
	[Attribute()]
	protected string m_sBlurName;
	
	[Attribute()]
	protected string m_sAutoInitName;
	
	[Attribute("0.4", desc: "When the actual charging starts. This should be after ping is evaluated. Thinging this value also means changing the process min of m_RadialProcessBar and the Ping input!")]
	protected float m_fChargeThreshold;
	
	protected SCR_EditorManagerEntity m_EditorManager;
	protected MenuManager m_MenuManager;
	protected Widget m_Widget;
	protected Widget m_Blur;
	protected Widget m_AutoInit;
	protected Widget m_ProgressWidget;
	protected SCR_ImageRadialProgressBarUIComponent m_RadialProcessBar;
	protected SCR_FadeUIComponent m_LoadingWidget;
	protected SCR_FadeUIComponent m_InitLoadingWidget;
	
	//State
	protected bool m_bCharging;
	
	//------------------------------------------------------------------------------------------------
	protected void ChargeStart()
	{
		if (m_MenuManager.IsAnyDialogOpen() || m_EditorManager.GetCurrentModeEntity().GetPreventClose())
			return;
		
		m_bCharging = true;
		m_Blur.SetVisible(true);
		
		if (m_Widget.IsVisible())
			return;
		
		m_Widget.SetVisible(true);
		m_RadialProcessBar.FadeImage(true);
		m_ProgressWidget.SetVisible(true);
		m_LoadingWidget.GetFadeWidget().SetVisible(false);
	}

	//------------------------------------------------------------------------------------------------
	protected void ChargeFail()
	{
		m_bCharging = false;
		m_Blur.SetVisible(false);
		m_Widget.SetVisible(false);
		
		if (m_RadialProcessBar.GetProgress() == m_RadialProcessBar.GetMaxProgress() || m_RadialProcessBar.GetProgress() <= m_RadialProcessBar.GetMinProgress())
			return;
		
		m_RadialProcessBar.InstantProgressMaxMin(false);
	}

	//------------------------------------------------------------------------------------------------
	protected void WaitStart()
	{
		SCR_EditorModeEntity mode = m_EditorManager.GetCurrentModeEntity();
		if (mode)
			mode.GetInfo().SetIconTo(m_RadialProcessBar.GetFadeImage());
		
		if (!m_Widget.IsVisible())
		{
			m_AutoInit.SetVisible(m_EditorManager.IsAutoInit());
			m_Blur.SetVisible(true);
			m_Widget.SetVisible(true);
			m_RadialProcessBar.InstantProgressMaxMin(false);
			m_RadialProcessBar.FadeImage(true);
			//m_InitLoadingWidget.FadeIn();		
			m_LoadingWidget.FadeIn();
		}
		
		if (m_InitLoadingWidget && m_EditorManager.IsAutoInit())
			m_InitLoadingWidget.FadeIn();		
	}

	//------------------------------------------------------------------------------------------------
	protected void WaitEnd()
	{
		m_RadialProcessBar.SetProgress(m_RadialProcessBar.GetMinProgress());
		
		if (m_AutoInit.IsVisible())
		{
			AnimateWidget.Opacity(m_AutoInit, 0, UIConstants.FADE_RATE_SUPER_SLOW);
			
			if (m_InitLoadingWidget)
				m_InitLoadingWidget.FadeOut(true);
		}
		
		m_Blur.SetVisible(false);
		m_LoadingWidget.FadeOut(false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnRadialProgressFadeDone(SCR_ImageRadialProgressBarUIComponent radial, bool isFadeIn)
	{
		if (!m_bCharging && !isFadeIn)
		{
			m_RadialProcessBar.InstantProgressMaxMin(false);
			m_Widget.SetVisible(false);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnEditorToggleValue(float value, EActionTrigger reason)
	{
		SCR_EditorModeEntity mode = m_EditorManager.GetCurrentModeEntity();
		
		if (m_MenuManager.IsAnyDialogOpen() || mode && mode.GetPreventClose())
			ChargeFail();
		
		if (value == 0)
		{
			//Charge has failed
			if (m_bCharging)
				ChargeFail();
			
			return;
		}
		//If cannot open editor
		else if (!m_EditorManager || !m_EditorManager.CanToggle())
		{
			//Safty if rights where removed
			if (m_bCharging)
				ChargeFail();
			
			return;
		}
		//Visualize the charge only after ping input time window
		else if (value < m_fChargeThreshold)
		{
			return;
		}
		
		if (!m_bCharging)
			ChargeStart();
		else
			m_RadialProcessBar.SetProgress(value);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnEditorToggleDone()
	{
		//Editor failed to open, terminate
		if (!m_EditorManager.IsInTransition())
			ChargeFail();
		
		m_bCharging = false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnRequest()
	{
		WaitStart();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRequestEnd()
	{
		m_bCharging = false;
		WaitEnd();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnAsyncLoad(float progress)
	{
		if (progress < 1)
			WaitStart();
		else
			WaitEnd();
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		if (SCR_Global.IsEditMode())
			return;
		
		m_EditorManager = SCR_EditorManagerEntity.GetInstance();
		if (!m_EditorManager)
			return;
		
		InputManager inputManager = GetGame().GetInputManager();
		if (!inputManager)
			return;
		
		m_ProgressWidget = w.FindWidget(m_sProgressWidgetName);
		if (!m_ProgressWidget)
		{
			Print(string.Format("ProgressBarWidget '%1' not found!", m_sProgressWidgetName), LogLevel.WARNING);
			return;
		}
		
		m_RadialProcessBar = SCR_ImageRadialProgressBarUIComponent.Cast(m_ProgressWidget.GetChildren().FindHandler(SCR_ImageRadialProgressBarUIComponent));
		if (!m_RadialProcessBar)
			return;
		
		m_RadialProcessBar.GetOnFadeDone().Insert(OnRadialProgressFadeDone);
		
		m_Blur = w.FindWidget(m_sBlurName);
		m_AutoInit = w.FindWidget(m_sAutoInitName);
		m_AutoInit.SetVisible(false);
		
		Widget loadingWidget =  w.FindAnyWidget(m_sLoadingWidgetName);
		m_LoadingWidget = SCR_FadeUIComponent.Cast(loadingWidget.FindHandler(SCR_FadeUIComponent));
		m_LoadingWidget.GetFadeWidget().SetVisible(false);
		
		Widget initLoadingWidget = w.FindAnyWidget(m_sInitLoadingWidgetName);
		m_InitLoadingWidget = SCR_FadeUIComponent.Cast(initLoadingWidget.FindHandler(SCR_FadeUIComponent));
		
		m_Widget = w;
		m_Widget.SetVisible(false);
		
		m_EditorManager.GetOnRequest().Insert(OnRequest);
		m_EditorManager.GetOnOpened().Insert(OnRequestEnd);
		m_EditorManager.GetOnClosed().Insert(OnRequestEnd);
		m_EditorManager.GetOnAsyncLoad().Insert(OnAsyncLoad);
		
		inputManager.AddActionListener("EditorToggle", EActionTrigger.VALUE, OnEditorToggleValue);
		inputManager.AddActionListener("EditorToggle", EActionTrigger.UP, OnEditorToggleDone);
		
		m_MenuManager = GetGame().GetMenuManager();
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		if (m_EditorManager)
		{
			m_EditorManager.GetOnRequest().Remove(OnRequest);
			m_EditorManager.GetOnOpened().Remove(OnRequestEnd);
			m_EditorManager.GetOnClosed().Remove(OnRequestEnd);
			m_EditorManager.GetOnAsyncLoad().Remove(OnAsyncLoad);
		}
		
		InputManager inputManager = GetGame().GetInputManager();
		if (inputManager)
		{
			inputManager.RemoveActionListener("EditorToggle", EActionTrigger.VALUE, OnEditorToggleValue);
			inputManager.RemoveActionListener("EditorToggle", EActionTrigger.UP, OnEditorToggleDone);
			
		}
		
		if (m_RadialProcessBar)
			m_RadialProcessBar.GetOnFadeDone().Remove(OnRadialProgressFadeDone);
	}
}
