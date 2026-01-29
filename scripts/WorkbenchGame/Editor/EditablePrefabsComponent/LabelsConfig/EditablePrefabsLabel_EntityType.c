[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityLabel, "m_Label")]
class EditablePrefabsLabel_EntityType : EditablePrefabsLabel_Base
{
	[Attribute("0", UIWidgets.ComboBox, enumType: EEditableEntityType)]
	protected EEditableEntityType m_EntityType;

	//------------------------------------------------------------------------------------------------
	override bool GetLabelValid(WorldEditorAPI api, IEntitySource entitySource, IEntityComponentSource componentSource, string targetPath, EEditableEntityType entityType, notnull array<EEditableEntityLabel> authoredLabels, out EEditableEntityLabel label)
	{
		// Skip objects/generic label on Composition and System entity types
		if (m_EntityType == EEditableEntityType.GENERIC &&
			(authoredLabels.Contains(EEditableEntityLabel.ENTITYTYPE_COMPOSITION)
			|| authoredLabels.Contains(EEditableEntityLabel.ENTITYTYPE_SYSTEM)
			|| entityType == EEditableEntityType.SYSTEM))
		{
			return false;
		}

		if (entityType == m_EntityType)
		{
			label = m_Label;
			return true;
		}

		return false;
	}
}
