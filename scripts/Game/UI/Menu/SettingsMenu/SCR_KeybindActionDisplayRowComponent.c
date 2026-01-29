/*
Component that handles displaying multiple binds on a row.
Allows to choose the number of displayed binds and how to show hidden ones, either by a clickable icon or by a number
Also handles repositioning and scrolling longer binds
*/

class SCR_KeybindActionDisplayRowComponent : SCR_ScriptedWidgetComponent
{
	[Attribute("{A06BEA3AE2B878B9}UI/layouts/Menus/SettingsMenu/BindingMenu/KeybindActionDisplay.layout")]
	ResourceName m_sLayout;
	
	[Attribute("1", desc: "maximum allowed number of displays")]
	int m_iMaxDisplays;
	
	[Attribute("1", UIWidgets.ComboBox, "Horizontal layout container fill origin", "", ParamEnumArray.FromEnum(HorizontalFillOrigin))]
	HorizontalFillOrigin m_eRowFillOrigin;
	
	[Attribute("0")]
	bool m_bShowAdditionalKeybindsNumber;
	
	[Attribute("#AR-ValueUnit_Short_Plus")]
	string m_sAdditionalKeybindsMessage;
	
	protected int m_iBindsLastIndex;
	
	protected Widget m_wHorizontalScrollFrame;
	protected HorizontalLayoutWidget m_wContainerWidget;
	protected ref array<SCR_KeybindActionDisplayComponent> m_aDisplays = {};
	
	protected RichTextWidget m_wAdditionalBindsText;
	protected Widget m_wAdditionalBindsOverlay;
	
	protected SCR_ModularButtonComponent m_AdditionalBindsButton;
	protected SCR_HorizontalScrollAnimationComponent m_HorizontalScrollComponent;
	
	protected const string ROOT_NAME = "KeybindActionDisplayRow";
	
	protected ref ScriptInvokerVoid m_OnAdditionalBindsClicked;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_wContainerWidget = HorizontalLayoutWidget.Cast(m_wRoot.FindAnyWidget("ActionDisplaysHorizontalLayout"));
		if (m_wContainerWidget)
			m_wContainerWidget.SetFillOrigin(m_eRowFillOrigin);
		
		m_wHorizontalScrollFrame = m_wRoot.FindAnyWidget("HorizontalScrollFrame");
		if (m_wHorizontalScrollFrame)
			m_HorizontalScrollComponent = SCR_HorizontalScrollAnimationComponent.FindComponent(m_wHorizontalScrollFrame);

		m_wAdditionalBindsText = RichTextWidget.Cast(m_wRoot.FindAnyWidget("AdditionalBinds"));
		m_wAdditionalBindsOverlay = m_wRoot.FindAnyWidget("AdditionalBindsOverlay");
		
		Widget root = w.FindAnyWidget("AdditionalBindsButton");
		if (root)
			m_AdditionalBindsButton = SCR_ModularButtonComponent.FindComponent(root);
		
		if (m_AdditionalBindsButton)
			m_AdditionalBindsButton.m_OnClicked.Insert(OnAdditionalBindsButtonClicked);
			
		ResetScrolling();
		
		// Init for editor work
		if (!GetGame().InPlayMode())
			CreateActionDisplays(null, null, EInputDeviceType.KEYBOARD);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ResetScrolling()
	{
		if (!m_wContainerWidget || !m_HorizontalScrollComponent)
			return;
		
		m_HorizontalScrollComponent.AnimationStop();
		m_HorizontalScrollComponent.ResetPosition();
		
		FrameSlot.SetAnchorMin(m_wContainerWidget, GetAlignment(), GetAlignment());
		FrameSlot.SetAnchorMax(m_wContainerWidget, GetAlignment(), GetAlignment());
		FrameSlot.SetAlignment(m_wContainerWidget, GetAlignment(), GetAlignment());
		
		LayoutSlot.SetPadding(m_wHorizontalScrollFrame, 0, 0, 0, 0);
	}
	
	//------------------------------------------------------------------------------------------------
	protected float GetAlignment()
	{
		switch (m_eRowFillOrigin)
		{
			case HorizontalFillOrigin.LEFT:		return 0;
			case HorizontalFillOrigin.CENTER:	return 0.5;
			case HorizontalFillOrigin.RIGHT:	return 1;
		}
		
		return 0;
	}
	
	// TODO: there seems to be an execution order issue with widget resizing, so we need to wait two frames. Ask for fix
	//------------------------------------------------------------------------------------------------
	protected void UpdateScrollFrameSkip()
	{
		GetGame().GetCallqueue().Call(UpdateScrolling, true);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnAdditionalBindsButtonClicked(SCR_ModularButtonComponent comp)
	{
		if (m_OnAdditionalBindsClicked)
			m_OnAdditionalBindsClicked.Invoke();
	}
	
	// --- Public ---
	//------------------------------------------------------------------------------------------------
	void UpdateScrolling(bool enable)
	{
		if (!m_wHorizontalScrollFrame || !m_wContainerWidget || !m_HorizontalScrollComponent || !m_wAdditionalBindsOverlay)
			return;
		
		ResetScrolling();
		
		float frameX, frameY, textX, textY, additionalBindsX, additionalBindsY;
		m_wHorizontalScrollFrame.GetScreenSize(frameX, frameY);
		m_wContainerWidget.GetScreenSize(textX, textY);
		m_wAdditionalBindsOverlay.GetScreenSize(additionalBindsX, additionalBindsY);

		vector alignment = FrameSlot.GetAlignment(m_wContainerWidget);
		
		bool move = enable && textX > frameX - additionalBindsX;
		
		// Adjust frame padding to avoid overlapping the Additional binds text
		if ((move || m_eRowFillOrigin == HorizontalFillOrigin.RIGHT) && m_iBindsLastIndex + 1 > m_iMaxDisplays)
			LayoutSlot.SetPadding(m_wHorizontalScrollFrame, 0, 0, additionalBindsX, 0);
		
		// Animation
		if (move)
		{
			FrameSlot.SetAnchorMin(m_wContainerWidget, 0, GetAlignment());
			FrameSlot.SetAnchorMax(m_wContainerWidget, 0, GetAlignment());
			FrameSlot.SetAlignment(m_wContainerWidget, 0, GetAlignment());
			
			m_HorizontalScrollComponent.AnimationStart();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void CreateActionDisplays(SCR_KeyBindingEntry entry, SCR_SettingsManagerKeybindModule settingsKeybindModule, EInputDeviceType device)
	{
		if (!m_wContainerWidget)
			return;
		
		// Clear old displays
		foreach (SCR_KeybindActionDisplayComponent display : m_aDisplays)
		{
			display.GetRootWidget().RemoveFromHierarchy();
		}
		
		m_aDisplays.Clear();
		
		// Find number of available bindings
		int lastIndex = m_iMaxDisplays - 1;
		m_iBindsLastIndex = 0;
		
		if (entry && settingsKeybindModule)
		{
			string finalPreset;
			string preset = entry.m_sPreset;
			
			if (device == EInputDeviceType.GAMEPAD)
			{
				if (!entry.m_sPresetGamepadOptional.IsEmpty())
					preset = entry.m_sPresetGamepadOptional;
				else
					preset = entry.m_sPreset;
				
				if (!preset.IsEmpty())
					finalPreset = settingsKeybindModule.GetGamepadPresetPrefix() + preset;
			}
			
			else if (device == EInputDeviceType.KEYBOARD && !preset.IsEmpty())
			{
				finalPreset = settingsKeybindModule.GetPrimaryPresetPrefix() + preset;
			}
			
			m_iBindsLastIndex = settingsKeybindModule.GetActionBindCount(entry.m_sActionName, finalPreset, device) - 1;
			
			if (m_iBindsLastIndex < lastIndex)
				lastIndex = m_iBindsLastIndex;
		}

		// Make sure at least 1 display is created
		lastIndex = Math.Clamp(lastIndex, 0, m_iMaxDisplays - 1);
		
		// Create displays
		int index;
		for (index = 0; index <= lastIndex; index++)
		{
			Widget w = GetGame().GetWorkspace().CreateWidgets(m_sLayout, m_wContainerWidget);
			if (!w)
				continue;

			SCR_KeybindActionDisplayComponent comp = SCR_KeybindActionDisplayComponent.FindComponent(w);
			if (!comp)
				continue;
			
			m_aDisplays.Insert(comp);
		}
		
		// Fill displays based on row alignment
		index = 0;
		SCR_KeybindActionDisplayData data;
		foreach (SCR_KeybindActionDisplayComponent comp : m_aDisplays)
		{
			if (m_eRowFillOrigin != HorizontalFillOrigin.RIGHT)
				data = new SCR_KeybindActionDisplayData(entry, settingsKeybindModule, device, SCR_EActionDisplayState.DEFAULT, index, lastIndex);
			else
				data = new SCR_KeybindActionDisplayData(entry, settingsKeybindModule, device, SCR_EActionDisplayState.DEFAULT, lastIndex - index, lastIndex);
			
			comp.UpdateActionDisplay(data);
			
			index ++;
		}
		
		// Update Additional binds text
		int remaining = m_iBindsLastIndex - lastIndex;
		bool show = remaining > 0;

		if (m_wAdditionalBindsOverlay)
			m_wAdditionalBindsOverlay.SetVisible(show);
		
		if (m_wAdditionalBindsText)
		{
			m_wAdditionalBindsText.SetVisible(show && m_bShowAdditionalKeybindsNumber);
			m_wAdditionalBindsText.SetTextFormat(m_sAdditionalKeybindsMessage, remaining);
		}
		
		if (m_AdditionalBindsButton)
			m_AdditionalBindsButton.SetVisible(show && !m_bShowAdditionalKeybindsNumber);
		
		// Check for scrolling
		GetGame().GetCallqueue().Call(UpdateScrollFrameSkip);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetAllActionDisplayStates(SCR_EActionDisplayState state)
	{
		SCR_KeybindActionDisplayData data;
		foreach (SCR_KeybindActionDisplayComponent comp : m_aDisplays)
		{
			data = comp.GetActionDisplayData();
			if (!data)
				continue;
			
			data.m_eState = state;
			
			comp.UpdateActionDisplay(data);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnAdditionalBindsClicked()
	{
		if (!m_OnAdditionalBindsClicked)
			m_OnAdditionalBindsClicked = new ScriptInvokerVoid();
		
		return m_OnAdditionalBindsClicked;
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_KeybindActionDisplayRowComponent FindComponent(notnull Widget w)
	{
		return SCR_KeybindActionDisplayRowComponent.Cast(w.FindHandler(SCR_KeybindActionDisplayRowComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_KeybindActionDisplayRowComponent FindComponentInHierarchy(notnull Widget w)
	{
		Widget actionRoot = w.FindAnyWidget(ROOT_NAME);
		if (!actionRoot)
			return null;
		
		return SCR_KeybindActionDisplayRowComponent.FindComponent(actionRoot);
	}
}