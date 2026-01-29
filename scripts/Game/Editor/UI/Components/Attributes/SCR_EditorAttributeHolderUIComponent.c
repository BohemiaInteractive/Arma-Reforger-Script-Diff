class SCR_EditorAttributeHolderUIComponent: ScriptedWidgetComponent
{
	[Attribute("AttributeHolder")]
	protected string m_sAttributeHolderName;
	
	[Attribute("TickboxHolder")]
	protected string m_sTickBoxAttributeName;
	[Attribute("GamePadLockedSelector")]
	protected string m_sGamePadLockedSelectorName;
	
	[Attribute("HintWidget")]
	protected string m_sAttributeHintName;
	[Attribute("HintText")]
	protected string m_sAttributeHintTextName;
	
	protected Widget m_wAttributeHolder;
	protected SCR_AttributeTickboxUIComponent m_TickBoxAttribute;
	protected Widget m_GamePadLockedSelector;
	protected Widget m_wAttributeHint;
	protected RichTextWidget m_wAttributeHintText;
	
	protected SCR_BaseEditorAttributeUIComponent m_AttributeComponent;
	
	//============================ GETTERS ============================\\
	Widget GetAttributeHolder()
	{
		return m_wAttributeHolder;
	}
	
	//============================ SETTERS ============================\\
	void SetAttributeReference(SCR_BaseEditorAttributeUIComponent attribute)
	{
		m_AttributeComponent = attribute;
	}
	
	void SetHintText(string hint)
	{
		m_wAttributeHintText.SetText(hint);
	}
	
	void ShowHintText(bool show)
	{
		m_wAttributeHint.SetVisible(show);
	}
	
	//============================ FOCUS ============================\\
	//If Disabled attribute is focused or not with gamepad
	protected void GamePadLockedSelectorFocusChanged(bool newFocus)
	{
		if (!m_AttributeComponent.GetAttribute().GetHasConflictingValues())
			return;
		
		if (newFocus)
			m_AttributeComponent.GetOnAttributeUIFocusChanged().Invoke(m_AttributeComponent);
		else
			m_AttributeComponent.GetOnAttributeUIFocusChanged().Invoke(null);
	}
	
	
	//============================ HANDLER ATTACHED ============================\\
	override protected void HandlerAttached(Widget w)
	{
		m_wAttributeHolder =  w.FindAnyWidget(m_sAttributeHolderName);
		
		Widget tickbox = w.FindAnyWidget(m_sTickBoxAttributeName);
		m_TickBoxAttribute = SCR_AttributeTickboxUIComponent.Cast(tickbox.FindHandler(SCR_AttributeTickboxUIComponent));
		
		m_GamePadLockedSelector = w.FindAnyWidget(m_sGamePadLockedSelectorName);
		
		if (m_GamePadLockedSelector)
		{
			SCR_OnFocusUIComponent focusComponent = SCR_OnFocusUIComponent.Cast(m_GamePadLockedSelector.FindHandler(SCR_OnFocusUIComponent));
			
			if (focusComponent)
				focusComponent.GetOnFocusChanged().Insert(GamePadLockedSelectorFocusChanged);
		}
		
		m_wAttributeHint = w.FindAnyWidget(m_sAttributeHintName);
		if (!m_wAttributeHint)
			return;
		
		m_wAttributeHintText = RichTextWidget.Cast(m_wAttributeHint.FindAnyWidget(m_sAttributeHintTextName));
	}
	

};
