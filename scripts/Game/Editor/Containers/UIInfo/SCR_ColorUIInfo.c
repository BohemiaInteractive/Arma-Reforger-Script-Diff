[BaseContainerProps(), SCR_BaseContainerLocalizedTitleField("Name")]
class SCR_ColorUIInfo : SCR_UIInfo
{
	[Attribute("1.0 1.0 1.0 1.0")]
	protected ref Color m_cColor;
	
	//------------------------------------------------------------------------------------------------
	//! Get Color
	//! \return UI Info colour
	Color GetColor()
	{
		return Color.FromInt(m_cColor.PackToInt());
	}

	//------------------------------------------------------------------------------------------------
	//~ Protected, to be overridden and/or made public by inherited classes
	override protected void CopyFrom(SCR_UIName source)
	{
		SCR_ColorUIInfo sourceInfo = SCR_ColorUIInfo.Cast(source);
		if (sourceInfo)
		{
			Icon = sourceInfo.Icon;
			IconSetName = sourceInfo.IconSetName;
			m_cColor = sourceInfo.m_cColor;
		}
		
		super.CopyFrom(source);
	}
}
