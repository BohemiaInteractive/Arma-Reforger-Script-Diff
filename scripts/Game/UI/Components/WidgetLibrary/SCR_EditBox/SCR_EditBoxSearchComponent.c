//------------------------------------------------------------------------------------------------
class SCR_EditBoxSearchComponent : SCR_EditBoxComponent
{
	[Attribute("1")]
	protected bool m_bColorizeText;

	[Attribute(UIColors.GetColorAttribute(UIColors.NEUTRAL_ACTIVE_STANDBY), UIWidgets.ColorPicker)]
	protected ref Color m_cDefault;

	[Attribute(UIColors.GetColorAttribute(UIColors.HIGHLIGHTED), UIWidgets.ColorPicker)]
	protected ref Color m_cFocused;

	[Attribute(UIColors.GetColorAttribute(UIColors.CONTRAST_COLOR), UIWidgets.ColorPicker)]
	protected ref Color m_cSearched;

	[Attribute()]
	protected string m_sMessage;
	
	[Attribute("1", desc: "Should the message be hidden if there's text in the search bar")]
	protected bool m_bHideMessageOnSearch;
	
	protected ref array<Widget> m_aColorizedElements = {};
	protected SCR_ESearchBoxState m_eState;
	protected bool m_bIsFilterActive;
	
	protected RichTextWidget m_wMessage;
	protected SCR_ModularButtonComponent m_MessageButton;
	protected string m_sLastSearch;

	//------------------------------------------------------------------------------------------------
	override protected void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_wMessage = RichTextWidget.Cast(m_wRoot.FindAnyWidget("EditBoxMessage"));
		if (m_wMessage)
			m_aColorizedElements.Insert(m_wMessage);
		
		if (m_wImgWriteIcon)
			m_aColorizedElements.Insert(m_wImgWriteIcon);

		if (m_bColorizeText && m_wEditBoxWidget)
			m_aColorizedElements.Insert(m_wEditBoxWidget);

		// Message button
		Widget messageButton = m_wRoot.FindAnyWidget("EditBoxMessageButton");
		if (messageButton)
			m_MessageButton = SCR_ModularButtonComponent.FindComponent(messageButton);
		
		if (m_MessageButton)
			m_MessageButton.m_OnClicked.Insert(OnInternalButtonClicked);
		
		SetMessage(m_sMessage);
		
		m_wBorder.SetVisible(!m_bColorizeText);

		UpdateWidgets(false);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		// Moving from the write icon button to the edit field will trigger OnMouseEnter, which we don't want if we're in write mode already
		if (IsInWriteMode())
			return false;
		
		return super.OnMouseEnter(w, x, y);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnHandlerFocus()
	{	
		super.OnHandlerFocus();

		m_eState = SCR_ESearchBoxState.FOCUSED;
		UpdateWidgets();
	}

	//------------------------------------------------------------------------------------------------
	override void OnHandlerFocusLost()
	{
		// On PC, changing focus doubles as confirmation
		if (!GetGame().IsPlatformGameConsole() && m_bIsInWriteMode && GetValue() != m_sLastSearch)
			OnConfirm(m_wEditBox);
		
		super.OnHandlerFocusLost();
				
		if (m_bIsFilterActive)
			m_eState = SCR_ESearchBoxState.SEARCHED;
		else
			m_eState = SCR_ESearchBoxState.DEFAULT;

		UpdateWidgets();
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnConfirm(Widget w)
	{
		if (m_bIsInWriteMode)
			super.OnConfirm(w);

		m_bIsFilterActive = GetValue() != string.Empty;
		m_sLastSearch = GetValue();
	}

	//------------------------------------------------------------------------------------------------
	override void UpdateInteractionState(bool forceDisabled)
	{
		super.UpdateInteractionState(forceDisabled);
		
		if (!m_wMessage)
			return;

		bool hideMessage = m_bHideMessageOnSearch && (m_bIsInWriteMode || !GetEditBoxText().IsEmpty());
		
		m_wMessage.SetVisible(!hideMessage);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateWidgets(bool animate = true)
	{
		Color color;

		switch (m_eState)
		{
			case SCR_ESearchBoxState.FOCUSED:
				color = m_cFocused;
				break;

			case SCR_ESearchBoxState.SEARCHED:
				color = m_cSearched;
				break;
			
			default:
				color = m_cDefault;
				break;
		}

		foreach (Widget element : m_aColorizedElements)
		{
			if (animate)
				AnimateWidget.Color(element, color, m_fColorsAnimationTime);
			else
				element.SetColor(color);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	bool SetMessage(string message)
	{
		if (!m_wMessage || !m_MessageButton)
			return false;
		
		m_MessageButton.SetVisible(!message.IsEmpty());
		
		if (!message.IsEmpty())
			m_wMessage.SetTextFormat(message);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Static method to easily find component by providing name and parent.
	//! Searching all children will go through whole hierarchy, instead of immediate chidren
	static SCR_EditBoxSearchComponent GetEditBoxSearchComponent(string name, Widget parent, bool searchAllChildren = true)
	{
		SCR_EditBoxSearchComponent comp = SCR_EditBoxSearchComponent.Cast(
				SCR_WLibComponentBase.GetComponent(SCR_EditBoxSearchComponent, name, parent, searchAllChildren)
			);
		return comp;
	}
}

//------------------------------------------------------------------------------------------------
enum SCR_ESearchBoxState
{
	DEFAULT,
	FOCUSED,
	SEARCHED
}
