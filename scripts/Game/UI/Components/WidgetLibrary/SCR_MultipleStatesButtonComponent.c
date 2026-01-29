//! Multiple states button. 
//! Works with WLib_ButtonMultipleStates widget prefab.
// This class displays a button with multiple possible states.

//------------------------------------------------------------------------------------------------
class SCR_MultipleStatesButtonComponent : SCR_ButtonBaseComponent 
{
	[Attribute()]
	protected ref array<ref MultipleStateButtonState> m_aStates;
	
	[Attribute()]
	protected ResourceName m_wIconSetTexture;
	
	/*[Attribute()]
	ref array<string> m_aStateNames;
	
	[Attribute()]
	ref array<string> m_aStateIcons;*/
	
	[Attribute()]
	protected int m_iStateSelected;
	
	[Attribute()]
 	protected bool m_bEnabled = true; 
	
	[Attribute("", "auto", "Action ID to be displayed as hint")]
	protected string m_sHintAction;
	
	protected TextWidget m_ContentText;
	protected TextWidget m_ProgressText;
	protected ImageWidget m_Progress;
	protected ImageWidget m_wIcon;
	protected RichTextWidget m_wActionHint;
	
	ref ScriptInvoker m_OnStateChange = new ScriptInvoker;
	ref ScriptInvoker m_OnHover = new ScriptInvoker;
	ref ScriptInvoker m_OnHoverLeave = new ScriptInvoker;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_ContentText = TextWidget.Cast(m_wRoot.FindAnyWidget("ContentText"));
		m_Progress = ImageWidget.Cast(m_wRoot.FindAnyWidget("Progress"));
		m_ProgressText = TextWidget.Cast(m_wRoot.FindAnyWidget("ProgressNumber"));	
		m_wIcon = ImageWidget.Cast(m_wRoot.FindAnyWidget("ContentImage"));
		m_wActionHint = RichTextWidget.Cast(m_wRoot.FindAnyWidget("ActionHint"));
		
		if (!m_sHintAction.IsEmpty())
			m_wActionHint.SetText(string.Format("<action name=\"%1\"/>", m_sHintAction));
		else
			m_wActionHint.SetText(string.Empty);
		
		Init();
	}
		
	//------------------------------------------------------------------------------------------------
	void Init()
	{
		/*if (m_aStateNames.Count()>0)
			ChangeState(m_iStateSelected);*/
		
		if (m_aStates.Count() > 0)
			ChangeState(m_iStateSelected);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (!m_bEnabled)
			return false;
		super.OnClick(w, x, y, button);
		if (button != 0)
			return false;

		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void ChangeState(int state)
	{
		if (!m_aStates)
			return;
		
		if (state >= 0 && state < m_aStates.Count())
		{
			m_iStateSelected = state;

			// Text
			string name = m_aStates[state].m_sName;
			m_ContentText.SetText(name);
			
			// Image
			m_wIcon.LoadImageFromSet(0, m_wIconSetTexture, m_aStates[state].m_sIcon);
			
			// Color 
			if(m_aStates[state].m_cContentColor)
			{
				if (m_ContentText)
					m_ContentText.SetColor(m_aStates[state].m_cContentColor);
				
				if (m_wIcon)
					m_wIcon.SetColor(m_aStates[state].m_cContentColor);
			}
		}
		
		m_OnStateChange.Invoke(state);
	}
	
	//------------------------------------------------------------------------------------------------
	void StartProgress()
	{
		m_Progress.SetVisible(true);
		m_Progress.SetMaskProgress(0);
		m_ContentText.SetText(m_aStates[m_iStateSelected].m_sName);
		SetProgressText("0%");
	}
	
	//------------------------------------------------------------------------------------------------
	void SetProgress(int progress)
	{
		if (!m_Progress.IsVisible())
			m_Progress.SetVisible(true);
		m_Progress.SetMaskProgress(progress/100);
		SetProgressText("" + progress + "%");
		
	}
	
	//------------------------------------------------------------------------------------------------
	void FinishProgress()
	{
		m_Progress.SetVisible(false);
		SetProgressText("");
	}
	
	//------------------------------------------------------------------------------------------------
	int GetSelectedItem()
	{
		return m_iStateSelected;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetProgressText(string text)
	{
		m_ProgressText.SetText(text);
	
	}
	
	//------------------------------------------------------------------------------------------------
	void SetHintVisible(bool show)
	{
		m_wActionHint.SetVisible(show);
	}
	
	//------------------------------------------------------------------------------------------------
	void EnableButton(bool bEnable)
	{
		if (!m_wRoot)
			return;
		
		m_bEnabled = bEnable;
		if (m_bEnabled)
		{
			m_wRoot.SetOpacity(1);
			
		}
		else
		{
			m_wRoot.SetOpacity(0.6);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsButtonEnabled()
	{
		return m_bEnabled;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnMenuSelect()
	{
		if (m_bEnabled)
			super.OnMenuSelect();
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseButtonDown(Widget w, int x, int y, int button)
	{
		if (m_bEnabled)
			return super.OnMouseButtonDown(w, x, y, button);
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseButtonUp(Widget w, int x, int y, int button)
	{
		if (m_bEnabled)
			return super.OnMouseButtonUp(w, x, y, button);
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		if (m_bEnabled)
		{
			m_OnHover.Invoke();
			return super.OnMouseEnter(w, x, y);
		}
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		if (m_bEnabled)
		{
			m_OnHoverLeave.Invoke();
			return super.OnMouseLeave(w, enterW, x, y);
		}
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		if (m_bEnabled)
			return super.OnFocus(w, x, y);
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		if (m_bEnabled)
			return super.OnFocusLost(w, x, y);
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetCurrentContent()
	{
		return m_aStates[m_iStateSelected].m_sName;
	}	
	
	//------------------------------------------------------------------------------------------------
	//! Static method to easily find component by providing name and parent.
	//! Searching all children will go through whole hierarchy, instead of immediate chidren
	static SCR_MultipleStatesButtonComponent GetMultipleStatesButtonComponent(string name, Widget parent, bool searchAllChildren = true)
	{
		auto comp = SCR_MultipleStatesButtonComponent.Cast(
			SCR_WLibComponentBase.GetComponent(SCR_MultipleStatesButtonComponent, name, parent, searchAllChildren)
		);
		return comp;
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps()] 
class MultipleStateButtonState 
{
	[Attribute()]
	string m_sName;
	
	[Attribute()]
	string m_sIcon;
	
	[Attribute("1 1 1 1", UIWidgets.ColorPicker)]
	ref Color m_cContentColor;
};