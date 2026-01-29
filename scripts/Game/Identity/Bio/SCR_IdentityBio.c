[BaseContainerProps(), BaseContainerCustomDoubleCheckTitleField("m_bIsUnique", "m_sBio", "1", "UNIQUE - %1", "%1")]
class SCR_IdentityBio
{
	[Attribute(desc: "Bio as displayed in the game", uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sBio;
	
	[Attribute("0", desc: "Unique Bio's will only be assigned once in a gaming session and can never be duplicated.")]
	protected bool m_bIsUnique;
	
	//------------------------------------------------------------------------------------------------
	//! \return The bio of the character
	string GetBioText()
	{
		return m_sBio;
	}

	//------------------------------------------------------------------------------------------------
	//! \return If the character will only show up once in the game
	bool IsUnique()
	{
		return m_bIsUnique;
	}	
};