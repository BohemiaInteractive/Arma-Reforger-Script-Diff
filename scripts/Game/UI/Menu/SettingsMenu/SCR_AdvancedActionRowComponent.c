class SCR_AdvancedActionRowComponent : SCR_ScriptedWidgetComponent
{
	[Attribute(UIColors.GetColorAttribute(UIColors.NEUTRAL_ACTIVE_STANDBY))]
	protected ref Color m_ActionColor;
	
	[Attribute("1")]
	protected bool m_bShowConflictsList;
	
	[Attribute(UIConstants.ACTION_DISPLAY_ICON_SCALE_BIG)]
	protected float m_fActionIconScale;
	
	[Attribute("0", UIWidgets.ComboBox, "Determines the colors of the rich text action icons", "", ParamEnumArray.FromEnum(SCR_EActionDisplayState))]
	protected SCR_EActionDisplayState m_eActionIconState;
	
	protected int m_iKeybindIndex;

	protected RichTextWidget m_wActionBindRichText;
	protected RichTextWidget m_wIndexRichText;
	protected RichTextWidget m_wWarningRichText;
	protected RichTextWidget m_wActionsRichText;
	
	protected SCR_HorizontalScrollAnimationComponent m_HorizontalScrollComponent;
	
	protected const string ROOT_NAME = "AdvancedKeybindActionDisplay";
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_wActionBindRichText = RichTextWidget.Cast(m_wRoot.FindAnyWidget("ActionRichText"));
		m_wIndexRichText = RichTextWidget.Cast(m_wRoot.FindAnyWidget("IndexRichText"));
		m_wWarningRichText = RichTextWidget.Cast(m_wRoot.FindAnyWidget("WarningText"));
		m_wActionsRichText = RichTextWidget.Cast(m_wRoot.FindAnyWidget("WarningActions"));

		Widget frame = m_wRoot.FindAnyWidget("HorizontalScrollFrame");
		if (frame)
			m_HorizontalScrollComponent = SCR_HorizontalScrollAnimationComponent.FindComponent(frame);
		
		if (m_HorizontalScrollComponent)
			StopScrolling();
	}
	
	//------------------------------------------------------------------------------------------------
	// Displays the action's specified bind for the chosen device and conflicting occurrances
	void Init(string actionName, string actionPreset, int index, SCR_SettingsManagerKeybindModule settingsKeybindModule, EInputDeviceType device)
	{
		m_iKeybindIndex = index;
		
		string deviceString = UIConstants.DEVICE_KEYBOARD;
		if (device == EInputDeviceType.GAMEPAD)
			deviceString = UIConstants.DEVICE_GAMEPAD;

		if (m_wActionBindRichText)
			m_wActionBindRichText.SetText(string.Format(
				"<action name='%1' preset='%2' index='%3' device='%4' scale='%5' state ='%6'/>", 
				actionName, 
				actionPreset, 
				index, 
				deviceString,
				m_fActionIconScale.ToString(),
				UIConstants.GetActionDisplayStateAttribute(m_eActionIconState)
			));

		if (!m_wIndexRichText)
			return;

		//offset index by 1 so it's comfortable for humans
		int order = m_iKeybindIndex + 1;

		//todo: check how localization deals with the dot: Updated with Wlib - removing "." 
		if (m_wIndexRichText)
			m_wIndexRichText.SetText(order.ToString() + ".");
		
		//check for conflicts and deal with them if they exist
		array<SCR_KeyBindingEntry> conflictedActions = {};
		bool conflicted = m_bShowConflictsList && settingsKeybindModule.IsActionConflicted(actionName, conflictedActions, m_iKeybindIndex, actionPreset);
		
		if (m_wWarningRichText)
			m_wWarningRichText.SetVisible(conflicted);

		if (m_wActionsRichText)
			m_wActionsRichText.SetVisible(conflicted);
		
		if (!conflicted || !m_wActionsRichText)
			return;

		string actionNames = conflictedActions.Get(0).m_sDisplayName;
		
		for (int i = 1, count = conflictedActions.Count(); i < count; i++)
		{
			actionNames = actionNames + ", " + conflictedActions.Get(i).m_sDisplayName;
		}

		m_wActionsRichText.SetText(actionNames);
	}
	
	//------------------------------------------------------------------------------------------------
	int GetKeybindIndex()
	{
		return m_iKeybindIndex;
	}
	
	//------------------------------------------------------------------------------------------------
	void StartScrolling()
	{
		if (!m_HorizontalScrollComponent)
			return;
		
		m_HorizontalScrollComponent.AnimationStart();
	}
	
	//------------------------------------------------------------------------------------------------
	void StopScrolling()
	{
		if (!m_HorizontalScrollComponent)
			return;
		
		m_HorizontalScrollComponent.AnimationStop();
		m_HorizontalScrollComponent.ResetPosition();
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_AdvancedActionRowComponent FindComponent(notnull Widget w)
	{
		return SCR_AdvancedActionRowComponent.Cast(w.FindHandler(SCR_AdvancedActionRowComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_AdvancedActionRowComponent FindComponentInHierarchy(notnull Widget w)
	{
		Widget actionRoot = w.FindAnyWidget(ROOT_NAME);
		if (!actionRoot)
			return null;
		
		return SCR_AdvancedActionRowComponent.FindComponent(actionRoot);
	}
}
