class SCR_ListEntryHelper
{
	// Modular button effect tags
	static const string EFFECT_ICON_COLOR =			"IconColor";
	static const string EFFECT_BACKGROUND_COLOR =	"BackgroundColor";
	static const string EFFECT_NAME_COLOR =			"NameColor";
	static const string EFFECT_WRAPPER_COLOR =		"WrapperColor";
	static const string EFFECT_MESSAGE_COLOR =		"MessageColor";
	
	//------------------------------------------------------------------------------------------------
	static void UpdateMouseButtonColor(SCR_ModularButtonComponent button, bool inErrorState, bool entryFocused)
	{
		if (!button)
			return;
		
		SCR_ButtonEffectColor effect = SCR_ButtonEffectColor.Cast(button.FindEffect(EFFECT_ICON_COLOR));
		if (!effect)
			return;
		
		SCR_ButtonEffectColor effectBackground = SCR_ButtonEffectColor.Cast(button.FindEffect(EFFECT_BACKGROUND_COLOR));
		if (!effectBackground)
			return;
		
		Color color = UIColors.CopyColor(UIColors.IDLE_DISABLED);
		Color highlightedColor = UIColors.CopyColor(UIColors.HIGHLIGHTED);
		
		if (inErrorState)
		{
			color = UIColors.CopyColor(UIColors.WARNING_DISABLED);
			highlightedColor = UIColors.CopyColor(UIColors.WARNING);
		}
		else if (entryFocused)
		{
			color = UIColors.CopyColor(UIColors.NEUTRAL_ACTIVE_STANDBY);
		}
		
		effect.m_cDefault = color;
		effect.m_cToggledOff = color;
		effect.m_cHovered = highlightedColor;
		effect.m_cActivatedHovered = highlightedColor;
		effect.m_cFocusGained = highlightedColor;
		
		effectBackground.m_cHovered = highlightedColor;
		effectBackground.m_cActivatedHovered = highlightedColor;
		effectBackground.m_cFocusGained = highlightedColor;

		button.InvokeAllEnabledEffects(false);
	}
}
