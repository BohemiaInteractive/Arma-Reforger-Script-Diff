//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// Many interactable elements do not use this as their base! It is not generic enough: sounds, opacity and animation settings would ideally be handled in a more centralized way (stylesheets?) together with other data like color, because having them on each component makes mantaining a project wide consistent look impossible. In the end, most layouts using children of this ended up with a SCR_ModularButtonComponent on them anyways. It is missing key features like caching of interaction states and events, and handling of enabled state is very limited (it should keep track of multiple stacking state change requests)

//! Base class for all final Reforger interactive elements


//------------------------------------------------------------------------------------------------
class SCR_WLibComponentBase : SCR_ScriptedWidgetComponent
{
	[Attribute(SCR_SoundEvent.SOUND_FE_BUTTON_HOVER, UIWidgets.EditBox, "")]
	protected string m_sSoundHovered;

	[Attribute(SCR_SoundEvent.CLICK, UIWidgets.EditBox, "")]
	protected string m_sSoundClicked;
	
	[Attribute(SCR_SoundEvent.SOUND_INV_CLOSE,"Sound used for closing container")]
	protected string m_sSoundContainerClosed;

	[Attribute(defvalue: "0.2", UIWidgets.EditBox, "How fast each animation proceeds")]
	protected float m_fAnimationTime;

	[Attribute()]
	protected bool m_bMouseOverToFocus;

	[Attribute("0.3")]
	protected float m_fDisabledOpacity;

	protected float m_fAnimationRate;

	protected ref Managed m_UserData; // User data - can be accessed with SetData, GetData
	
	static const float START_ANIMATION_RATE = 10001; 	// Custom rate that creates instant animations and supress sounds playing at start
	static const float START_ANIMATION_PERIOD = 250; // Time since start in ms without sounds and animations

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_fAnimationRate = START_ANIMATION_RATE;
		// Set correct animation rate after the first frame, so animations are not played at the start of the visualization
		GetGame().GetCallqueue().CallLater(SetAnimationRate, START_ANIMATION_PERIOD);

		if (!w.IsEnabled())
			SetEnabled(false, false);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		// Accept only LMB as valid click
		if (button != 0)
			return false;

		PlaySound(m_sSoundClicked);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		// Do not remove the 'w.IsFocusable()' part of condition, it ensures that refocusing is happening only in case mouse enters focusable widget
		if (m_bMouseOverToFocus && w.IsFocusable() && GetGame().GetInputManager().GetLastUsedInputDevice() == EInputDeviceType.MOUSE)
		{
			GetGame().GetWorkspace().SetFocusedWidget(null);
			GetGame().GetWorkspace().SetFocusedWidget(w);
		}
		else
			PlaySound(m_sSoundHovered);
		
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		PlaySound(m_sSoundHovered);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnEnabled(bool animate)
	{
		if (animate && m_fAnimationRate != START_ANIMATION_RATE)
			AnimateWidget.Opacity(m_wRoot, 1, m_fAnimationRate);
		else
			m_wRoot.SetOpacity(1);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDisabled(bool animate)
	{
		if (animate && m_fAnimationRate != START_ANIMATION_RATE)
			AnimateWidget.Opacity(m_wRoot, m_fDisabledOpacity, m_fAnimationRate);
		else
			m_wRoot.SetOpacity(m_fDisabledOpacity);
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsChildWidget(Widget parent, Widget child)
	{
		if (parent == null || child == null)
			return false;

		child = child.GetParent();
		while (child)
		{
			if (child == parent)
				return true;

			child = child.GetParent();
		}
		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected void SetAnimationRate()
	{
		m_fAnimationRate = 1 / m_fAnimationTime;
	}

	//------------------------------------------------------------------------------------------------
	protected void PlaySound(string sound)
	{
		if (m_fAnimationRate != START_ANIMATION_RATE && sound != string.Empty)
			SCR_UISoundEntity.SoundEvent(sound);
	}

	//------------------------------------------------------------------------------------------------
	static bool SetTexture(ImageWidget widget, ResourceName texture, string image = "")
	{
		if (!widget || texture == ResourceName.Empty)
			return false;

		bool success;

		if (texture.EndsWith(".edds"))
			success = widget.LoadImageTexture(0, texture);
		else
			success = widget.LoadImageFromSet(0, texture, image);

		if (success)
		{
			int x, y;
			widget.GetImageSize(0, x, y);
			widget.SetSize(x, y);
		}

		return success;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetHoverSound(string soundHover)
	{
		m_sSoundHovered = soundHover;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetClickedSound(string soundClicked)
	{
		m_sSoundClicked = soundClicked;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetHoverSound()
	{
		return m_sSoundHovered;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetClickedSound()
	{
		return m_sSoundClicked;
				
	}
	
	//------------------------------------------------------------------------------------------------
	void SetEnabled(bool enabled, bool animate = true)
	{
		if (!m_wRoot || m_wRoot.IsEnabled() == enabled)
			return;

		m_wRoot.SetEnabled(enabled);
		if (enabled)
			OnEnabled(animate);
		else
			OnDisabled(animate);
	}

	//------------------------------------------------------------------------------------------------
	void SetVisible(bool visible, bool animate = true)
	{
		if (!m_wRoot)
			return;
		
		if (animate)
			AnimateWidget.Opacity(m_wRoot, visible, m_fAnimationRate, true);
		else
			m_wRoot.SetVisible(visible);
	}

	//------------------------------------------------------------------------------------------------
	bool IsEnabled()
	{
		if (!m_wRoot)
			return false;
		
		return m_wRoot.IsEnabled();
	}
	
	//------------------------------------------------------------------------------------------------
	void SetMouseOverToFocus(bool mouseOverToFocus)
	{
		m_bMouseOverToFocus = mouseOverToFocus;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetDisabledOpacity(float newDisabledOpacity)
	{
		m_fDisabledOpacity = newDisabledOpacity;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Store custom data
	//! \param[in] data
	void SetData(Managed data)
	{
		m_UserData = data;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get stored data
	//! \return data if set, null otherwise
	Managed GetData()
	{
		return m_UserData;
	}
};


//---- REFACTOR NOTE END ----