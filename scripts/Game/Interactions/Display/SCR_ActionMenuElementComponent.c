//#define DEBUG_ACTIONICONS
class SCR_ActionMenuElementComponent : SCR_ScriptedWidgetComponent
{
	protected ref SCR_ActionMenuElementWidgets m_Widgets = new SCR_ActionMenuElementWidgets();
	
	protected const float NEARBY_INTERACTION_OPACITY_DIVIDER = 3;
	protected const float NEARBY_INTERACTION_OPACITY_MULTIPLIER = 0.9;
	protected const float NEARBY_INTERACTION_OPACITY_CUTOFF = 0.1;
	
	protected const int m_iFontSizeSelected = 24;
	protected const int m_iFontSizeNotSelected = 20;

	protected bool m_bIsSelected;
	protected bool m_bIsDisabled;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_Widgets.Init(w);
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_NearbyContextWidgetComponentInteract GetIconComponent()
	{
		return m_Widgets.m_IconOverlayComponent;
	}

	//------------------------------------------------------------------------------------------------
	void SetVisible(bool state)
	{
		m_wRoot.SetVisible(state);
	}

	//------------------------------------------------------------------------------------------------
	//! Set all the data and how it should be displayed
	//! \param[in] userAction Context the Widget gets assigned to
	//! \param[in] isSelected is the userAction currently selected by the player
	//! \param[in] isAvailable is the userAction available (not disabled/blocked)
	//! \param[in] actionUIInfo containing all the information for the widget Icon, etc. Can be null
	//! return True if the userAction has at least one actionName
	bool Set(notnull BaseUserAction userAction, bool isSelected, bool isAvailable, SCR_ActionUIInfo actionUIInfo)
	{
		m_bIsSelected = isSelected;		
		m_bIsDisabled = !isAvailable;

		string sActionName = userAction.GetActionName();

		array<string> actionStrings = {};
		sActionName.Split("%CTX_HACK%", actionStrings, true);

		int actionStringsCount = actionStrings.Count();

		if (actionStringsCount == 0 || !actionStrings[0])
			return false;

		// Set string in widget and format it with provided ActionNameParams (if any)
		m_Widgets.m_wActionText.SetTextFormat(
			actionStrings[0],
			userAction.ActionNameParams[0],
			userAction.ActionNameParams[1],
			userAction.ActionNameParams[2],
			userAction.ActionNameParams[3],
			userAction.ActionNameParams[4],
			userAction.ActionNameParams[5],
			userAction.ActionNameParams[6],
			userAction.ActionNameParams[7],
			userAction.ActionNameParams[8]
		);

		if (actionStringsCount > 1 && actionStrings[1])
		{
			m_Widgets.m_wActionContext.SetText(actionStrings[1]);
			m_Widgets.m_wActionContext.SetVisible(true);
		}
		else
		{
			m_Widgets.m_wActionContext.SetText("");
			m_Widgets.m_wActionContext.SetVisible(false);
		}

		SetIconFromAction(actionUIInfo, userAction);

		m_Widgets.m_wIconOverlay.SetVisible(!isSelected);
		
		if (isSelected)
		{
			m_Widgets.m_wActionText.SetExactFontSize(m_iFontSizeSelected);
			m_Widgets.m_wActionContext.SetExactFontSize(m_iFontSizeSelected);
		}
		else
		{
			m_Widgets.m_wActionText.SetExactFontSize(m_iFontSizeNotSelected);
			m_Widgets.m_wActionContext.SetExactFontSize(m_iFontSizeNotSelected);
		}
		
		UpdateColors();

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Try to set the Icon defined in the UIInfo from action first
	protected void SetIconFromAction(SCR_ActionUIInfo info, BaseUserAction userAction)
	{
		if (userAction)
		{
			SCR_ActionUIInfo actionUIInfo = SCR_ActionUIInfo.Cast(userAction.GetUIInfo());
	
			if (actionUIInfo && actionUIInfo.SetIconTo(m_Widgets.m_wIcon, m_Widgets.m_wIconGlow))
				return;
		}
		
		if (info && info.SetIconTo(m_Widgets.m_wIcon, m_Widgets.m_wIconGlow))
			return;
		
		#ifdef DEBUG_ACTIONICONS
		Print("DEBUG_ACTIONICONS:: No icons found for "+ userAction);
		#endif

		m_Widgets.m_wIcon.LoadImageFromSet(0, UIConstants.ICONS_IMAGE_SET, UIConstants.ICON_INTERACT_DEFAULT);
		m_Widgets.m_wIconGlow.LoadImageFromSet(0, UIConstants.ICONS_GLOW_IMAGE_SET, UIConstants.ICON_INTERACT_DEFAULT);
	}

	//------------------------------------------------------------------------------------------------
	void UpdateColors()
	{
		Color textColor;
		if (m_bIsDisabled)
		{
			textColor = GUIColors.DISABLED;
			
			m_Widgets.m_wIcon.LoadImageFromSet(0, UIConstants.ICONS_IMAGE_SET, UIConstants.ICON_INTERACT_DISABLED);
			m_Widgets.m_wIconGlow.LoadImageFromSet(0, UIConstants.ICONS_GLOW_IMAGE_SET, UIConstants.ICON_INTERACT_DISABLED);
			m_Widgets.m_wIconGlow.SetOpacity(UIConstants.DISABLED_WIDGET_OPACITY);
			m_Widgets.m_wIcon.SetColor(textColor);			
		}
		else if (m_bIsSelected)
		{
			textColor = GUIColors.ENABLED;
		}
		else
		{
			textColor = UIColors.IDLE_ACTIVE;
			
			m_Widgets.m_wIconGlow.SetOpacity(UIConstants.ENABLED_WIDGET_OPACITY);
			m_Widgets.m_wIcon.SetColor(textColor);
		}

		m_Widgets.m_wActionText.SetColor(textColor);
		m_Widgets.m_wActionContext.SetColor(textColor);
	}

	//------------------------------------------------------------------------------------------------
	void SetOpacity(float position)
	{
		if (position > NEARBY_INTERACTION_OPACITY_DIVIDER)
		{
			m_wRoot.SetOpacity(0.0);
			return;
		}

		float opacity = UIConstants.ENABLED_WIDGET_OPACITY;
		if (position != 0)
			opacity = UIConstants.ENABLED_WIDGET_OPACITY * (NEARBY_INTERACTION_OPACITY_MULTIPLIER - (Math.AbsFloat(position) / NEARBY_INTERACTION_OPACITY_DIVIDER));

		// if opacity is less then UIConstants.NEARBY_INTERACTION_OPACITY_CUTOFF (10%), just make it invisible
		if (opacity < NEARBY_INTERACTION_OPACITY_CUTOFF)
			opacity = 0;

		m_wRoot.SetOpacity(Math.Clamp(opacity, 0.0, UIConstants.ENABLED_WIDGET_OPACITY));
		UpdateColors();
	}
}
