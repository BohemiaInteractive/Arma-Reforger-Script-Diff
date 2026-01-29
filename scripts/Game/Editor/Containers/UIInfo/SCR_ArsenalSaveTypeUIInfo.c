[BaseContainerProps(), SCR_BaseContainerLocalizedTitleField("Name")]
class SCR_ArsenalSaveTypeUIInfo : SCR_UIDescription
{
	[Attribute(desc: "Localized name to be displayed for player action. This can be empty to not display any specific type of saving for the action and use fallback default instead",uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sPlayerActionDisplayName;
	
	//------------------------------------------------------------------------------------------------
	//! \return Localized name to be displayed for player action. This can be empty to not display any specific type of saving for the action
	LocalizedString GetPlayerActionDisplayName()
	{
		return m_sPlayerActionDisplayName;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void CopyFrom(SCR_UIName source)
	{
		SCR_ArsenalSaveTypeUIInfo sourceInfo = SCR_ArsenalSaveTypeUIInfo.Cast(source);
		if (sourceInfo)
		{
			m_sPlayerActionDisplayName = sourceInfo.m_sPlayerActionDisplayName;
		}

		super.CopyFrom(source);
	}
}
