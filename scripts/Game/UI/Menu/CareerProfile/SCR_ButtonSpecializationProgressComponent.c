//------------------------------------------------------------------------------------------------
class SCR_ButtonSpecializationProgressComponent : SCR_WLibComponentBase 
{
	protected RichTextWidget m_wSpecializationTitle;
	protected RichTextWidget m_wSpecializationValue;
	protected SCR_ProgressBarWithInactiveColor m_SpecializationBar;
	
	protected const float m_fMinValue = 0;
	protected const float m_fMaxValue = 100;
	
	protected int m_ibuttonId;

	[Attribute()]
	protected bool m_bIsActive;
	
	[Attribute("1 1 1 1", UIWidgets.ColorPicker)]
	protected ref Color m_ColorInactive;
	
	[Attribute("1 1 1 0.3", UIWidgets.ColorPicker)]
	protected ref Color m_ColorInactiveHovered;
	
	[Attribute("0.898 0.541 0.184 1", UIWidgets.ColorPicker)]
	protected ref Color m_ColorActive;
	
	// Arguments passed: m_ibuttonId
	protected ref ScriptInvoker m_OnClicked = new ScriptInvoker();
	
	// Arguments passed: m_ibuttonId, bool isHovered
	protected ref ScriptInvoker m_OnMouseEnter = new ScriptInvoker();
	protected ref ScriptInvoker m_OnMouseLeave = new ScriptInvoker();
	
	//------------------------------------------------------------------------------------------------
	protected override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_wSpecializationTitle = RichTextWidget.Cast(w.FindAnyWidget("SpecializationTitle"));
		if (!m_wSpecializationTitle)
			return;
		
		m_wSpecializationValue = RichTextWidget.Cast(w.FindAnyWidget("SpecializationValue"));
		if (!m_wSpecializationValue)
			return;
		
		m_wSpecializationTitle.SetText("");
		m_wSpecializationTitle.SetColor(m_ColorInactive);
		m_wSpecializationValue.SetText("");
		m_wSpecializationValue.SetColor(m_ColorInactive);
		
		m_SpecializationBar = SCR_ProgressBarWithInactiveColor.Cast(SCR_WLibProgressBarComponent.GetProgressBar("SpecializationProgressBar", w, true));
		if (!m_SpecializationBar)
			return;
		
		m_SpecializationBar.SetMin(m_fMinValue);
		m_SpecializationBar.SetMax(m_fMaxValue);
		m_SpecializationBar.SetActive(false);
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnClicked()
	{
		return m_OnClicked;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override bool OnClick(Widget w, int x, int y, int button)
	{
		super.OnClick(w, x, y, button);
		if (button != 0)
			return false;
		
		m_OnClicked.Invoke(m_ibuttonId);
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void Activate(bool animate = true)
	{
		if (m_bIsActive)
			return;
		
		m_bIsActive = true;
		PlaySound(m_sSoundClicked);
		
		if (animate)
		{
			AnimateWidget.Color(m_wSpecializationValue, m_ColorActive, m_fAnimationRate);
			AnimateWidget.Color(m_wSpecializationTitle, m_ColorActive, m_fAnimationRate);
			m_SpecializationBar.SetActive(true);
		}
		else
		{
			AnimateWidget.StopAnimation(m_wSpecializationValue, WidgetAnimationColor);
			m_wSpecializationValue.SetColor(m_ColorActive);
			
			AnimateWidget.StopAnimation(m_wSpecializationTitle, WidgetAnimationColor);
			m_wSpecializationTitle.SetColor(m_ColorActive);
			
			m_SpecializationBar.SetActive(true);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void Deactivate(bool animate = true)
	{
		if (!m_bIsActive)
			return;
		
		m_bIsActive = false;
		PlaySound(m_sSoundClicked);
		
		if (animate)
		{
			AnimateWidget.Color(m_wSpecializationValue, m_ColorInactive, m_fAnimationRate);
			AnimateWidget.Color(m_wSpecializationTitle, m_ColorInactive, m_fAnimationRate);
			m_SpecializationBar.SetActive(false);
		}
		else
		{
			AnimateWidget.StopAnimation(m_wSpecializationValue, WidgetAnimationColor);
			m_wSpecializationValue.SetColor(m_ColorInactive);
			
			AnimateWidget.StopAnimation(m_wSpecializationTitle, WidgetAnimationColor);
			m_wSpecializationValue.SetColor(m_ColorInactive);
			
			m_SpecializationBar.SetActive(false);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected override bool OnMouseEnter(Widget w, int x, int y)
	{
		super.OnMouseEnter(w, x, y);
		m_OnMouseEnter.Invoke(m_ibuttonId, true);
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		super.OnMouseLeave(w, enterW, x, y);
		
		m_OnMouseLeave.Invoke(m_ibuttonId, false);
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void Hide()
	{
		m_wSpecializationValue.SetOpacity(0);
		m_wSpecializationTitle.SetOpacity(0);
		m_SpecializationBar.Hide();
	}
	
	//------------------------------------------------------------------------------------------------
	void Unhide()
	{
		m_wSpecializationValue.SetOpacity(255);
		m_wSpecializationTitle.SetOpacity(255);
		m_SpecializationBar.Unhide();
	}
	
	//------------------------------------------------------------------------------------------------
	void SetOnHover(bool OnHover = true)
	{
		if(m_bIsActive)
			return;
			
		if (OnHover)
		{
			AnimateWidget.Color(m_wSpecializationValue, m_ColorInactiveHovered, m_fAnimationRate);
			AnimateWidget.Color(m_wSpecializationTitle, m_ColorInactiveHovered, m_fAnimationRate);
			GetGame().GetCallqueue().CallLater(m_SpecializationBar.SetOnHover, m_fAnimationRate, false, true);
			return;
		}
		
		AnimateWidget.Color(m_wSpecializationValue, m_ColorInactive, m_fAnimationRate);
		AnimateWidget.Color(m_wSpecializationTitle, m_ColorInactive, m_fAnimationRate);
		GetGame().GetCallqueue().CallLater(m_SpecializationBar.SetOnHover, m_fAnimationRate, false, false);
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnMouseEnter()
	{
		return m_OnMouseEnter;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnMouseLeave()
	{
		return m_OnMouseLeave;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetTitle(string text)
	{
		if (!m_wSpecializationTitle)
			return;
		
		m_wSpecializationTitle.SetText(text);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetValue(float num)
	{
		if (!m_wSpecializationValue || !m_SpecializationBar)
			return;
		
		m_wSpecializationValue.SetText(""+num.ToString(-1, 2)+"%");
			
		m_SpecializationBar.SetValue(num);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetButtonId(int n)
	{
		if (n < 0 || n > SCR_PlayerDataConfigs.GetInstance().SPECIALIZATIONS_COUNT)
		{
			Print ("SCR_ButtonSpecializationProgress: Trying to create a Specialization Progress button for a non-valid specialization id", LogLevel.ERROR);
			return;
		}
		
		m_ibuttonId = n;
		SetTitle(""+(n+1)+". "+"#AR-CareerProfile_Specialization"+(n+1));
	}
	
	//------------------------------------------------------------------------------------------------
	void SetFocus()
	{
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
			return;
		
		GetGame().GetCallqueue().CallLater(workspace.SetFocusedWidget, 1000, false, m_wRoot, true);
	}
	
	//------------------------------------------------------------------------------------------------
	int GetButtonId()
	{
		return m_ibuttonId;
	}
};