class SCR_MainMenuTile_MenuComponent: SCR_TileBaseComponent
{
	[Attribute("")]
	string m_sTitle;
	
	[Attribute("")]
	string m_sDescription;
	
	[Attribute("")]
	ResourceName m_sImage;
	
	[Attribute("-1", UIWidgets.SearchComboBox, enumType: ChimeraMenuPreset)]
	protected ChimeraMenuPreset m_pMenuPreset;
	
	protected TextWidget m_wName;
	protected TextWidget m_wDescription;
	protected Widget m_wTextContent;
	protected Widget m_DisabledTileShadow;
	protected ImageWidget m_wBackground;
	
	protected SCR_InputButtonComponent m_Play;
	
	protected bool m_bFocused;
	protected bool m_bDisabled;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_wName = TextWidget.Cast(w.FindAnyWidget("Name"));
		m_wName.SetText(m_sTitle);
		
		m_wDescription = TextWidget.Cast(w.FindAnyWidget("Description"));
		m_wDescription.SetVisible(false);
		m_wDescription.SetText(m_sDescription);
		
		m_wTextContent = w.FindAnyWidget("TextSizeLayout");		
		
		m_wBackground = ImageWidget.Cast(w.FindAnyWidget("Shading"));
		m_wBackground.SetVisible(false);
		
		m_DisabledTileShadow = w.FindAnyWidget("m_DisabledTileShadow");
		m_DisabledTileShadow.SetVisible(false);
		
		SCR_ButtonImageComponent comp = SCR_ButtonImageComponent.Cast(w.FindHandler(SCR_ButtonImageComponent));
		if (comp)
		{
			if (!m_sImage.IsEmpty())
				comp.SetImage(m_sImage);
		}

		m_Play = SCR_InputButtonComponent.GetInputButtonComponent(SCR_ScenarioUICommon.BUTTON_PLAY, w);
		m_Play.m_OnActivated.Insert(OnPlay);
		m_Play.SetVisible(m_bFocused, false);
		
		bool isMouseInteraction = GetGame().GetInputManager().GetLastUsedInputDevice() == EInputDeviceType.MOUSE;
		
		UpdateButtonInputVisibility(m_Play.GetRootWidget(), !isMouseInteraction);
		
		GetGame().OnInputDeviceUserChangedInvoker().Insert(OnInputDeviceUserChanged);
	}

	// React on switching between input methods
	//------------------------------------------------------------------------------------------------
	protected void OnInputDeviceUserChanged(EInputDeviceType oldDevice, EInputDeviceType newDevice)
	{
		if (m_bDisabled || newDevice == EInputDeviceType.TRACK_IR || newDevice == EInputDeviceType.JOYSTICK)
			return;

		bool isMouseInteraction = newDevice == EInputDeviceType.MOUSE;
		
		UpdateButtonInputVisibility(m_Play.GetRootWidget(), !isMouseInteraction);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateButtonInputVisibility(notnull Widget root, bool visibility)
	{
		Widget buttonWidget = root.FindAnyWidget("InputButtonDisplayRoot");
		if (!buttonWidget)
			return;
		
		buttonWidget.SetEnabled(visibility);
		buttonWidget.SetVisible(visibility);
	}
	
	//------------------------------------------------------------------------------------------------
	void DisableTile()
	{
		m_bDisabled = true;
		
		m_DisabledTileShadow.SetVisible(true);
		
		m_Play.SetEnabled(false);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		OnPlay();

		return super.OnClick(w, x, y, button);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		if (m_bDisabled)
			return super.OnFocus(w, x, y);

		m_bFocused = true;
		float position[4] = {20.0, 0.0, 20.0, 25.0};
		
		AnimateWidget.Padding(m_wTextContent, position, 5.0);
		m_wDescription.SetVisible(true);
		m_wBackground.SetVisible(true);		
		
		m_Play.SetVisible(m_bFocused, false);
		
		return super.OnFocus(w, x, y);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{		
		m_bFocused = false;
		float position[4] = {20.0, 0.0, 20.0, -70.0};
		
		AnimateWidget.Padding(m_wTextContent, position, 5.0);
		m_wDescription.SetVisible(false);
		m_wBackground.SetVisible(false);
		
		m_Play.SetVisible(m_bFocused, false);
		
		return super.OnFocusLost(w, x, y);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlay()
	{
		if (m_bDisabled)
			return;
		
		GetGame().GetMenuManager().OpenMenu(m_pMenuPreset);
	}
}