[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityLabel, "m_Label")]
class EditablePrefabsLabel_Trait : EditablePrefabsLabel_Base
{
	//------------------------------------------------------------------------------------------------
	override bool GetLabelValid(WorldEditorAPI api, IEntitySource entitySource, IEntityComponentSource componentSource, string targetPath, EEditableEntityType entityType, notnull array<EEditableEntityLabel> authoredLabels, out EEditableEntityLabel label)
	{
		if (m_Label == EEditableEntityLabel.TRAIT_REARMING)
		{
			IEntityComponentSource arsenalComponentSource = SCR_BaseContainerTools.FindComponentSource(entitySource, SCR_ArsenalComponent);
			if (arsenalComponentSource)
			{
				label = m_Label;
				return true;
			}
		}

		return false;
	}
}
