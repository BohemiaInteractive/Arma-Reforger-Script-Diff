[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityLabel, "m_Label")]
class EditablePrefabsLabel_Base
{
	[Attribute("0", UIWidgets.ComboBox, "", enumType: EEditableEntityLabel)]
	protected EEditableEntityLabel m_Label;

	//------------------------------------------------------------------------------------------------
	//! \param[in] api
	//! \param[in] entitySource
	//! \param[in] componentSource
	//! \param[in] targetPath
	//! \param[in] entityType
	//! \param[in] authoredLabels
	//! \param[out] label
	//! \return
	bool GetLabelValid(WorldEditorAPI api, IEntitySource entitySource, IEntityComponentSource componentSource, string targetPath, EEditableEntityType entityType, notnull array<EEditableEntityLabel> authoredLabels, out EEditableEntityLabel label)
	{
		return false;
	}
}
