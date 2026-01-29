[BaseContainerProps(configRoot: true)]
class SCR_InputButtonLayoutConfig: InputButtonLayoutConfig
{
	//! Get the right style
	//! \params SCR_EButtonSize (int)
	//! \return Config entry containing information about imageset etc.
	SCR_InputButtonStyle GetButtonSize(SCR_EButtonSize eButtonSize)
	{
		if (eButtonSize == -1)
			return null;

		foreach (SCR_InputButtonLayoutEntry layoutType : m_aLayoutTypes)
		{
			if (layoutType.m_eType == eButtonSize)
				return layoutType.m_ButtonStyle;
		}
		return null;
	}
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(SCR_EButtonSize, "m_eType")]
class SCR_InputButtonLayoutEntry: InputButtonLayoutEntry
{
}

[BaseContainerProps()]
class SCR_InputButtonStyle: InputButtonStyle
{
	[Attribute("hold_triangle")]
	string m_sHoldIndicator;
}
