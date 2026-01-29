[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityLabel, "m_LabelType")]
class SCR_EditableEntityCoreLabelSettingExtended : SCR_EditableEntityCoreLabelSetting
{
	[Attribute(desc: "If label is check via GetLabelUIInfoIfValid and the current editor mode is within the black list than label is considered invalid and will be hidden", uiwidget: UIWidgets.Flags, enums: ParamEnumArray.FromEnum(EEditorMode))]
	protected EEditorMode m_eBlackListedModes;
	
	//------------------------------------------------------------------------------------------------
	override bool IsValid(EEditorMode currentMode)
	{
		//~ No current mode or no blacklist modes set
		if (currentMode == 0 || m_eBlackListedModes == 0)
			return true;

		return !SCR_Enum.HasFlag(m_eBlackListedModes, currentMode);
	}
}
