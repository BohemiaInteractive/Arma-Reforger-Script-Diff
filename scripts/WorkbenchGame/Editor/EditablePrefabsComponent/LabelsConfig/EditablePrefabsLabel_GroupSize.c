[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityLabel, "m_Label")]
class EditablePrefabsLabel_GroupSize : EditablePrefabsLabel_Base
{
	[Attribute(desc:"GROUPSIZE_SMALL label will be added to groups >= x entities")]
	protected int m_GroupSizeSmall;

	[Attribute(desc:"GROUPSIZE_MEDIUM label will be added to groups >= x entities")]
	protected int m_GroupSizeMedium;

	[Attribute(desc:"GROUPSIZE_LARGE label will be added to groups >= x entities")]
	protected int m_GroupSizeLarge;

	//------------------------------------------------------------------------------------------------
	override bool GetLabelValid(WorldEditorAPI api, IEntitySource entitySource, IEntityComponentSource componentSource, string targetPath, EEditableEntityType entityType, notnull array<EEditableEntityLabel> authoredLabels, out EEditableEntityLabel label)
	{
		if (entityType != EEditableEntityType.GROUP)
			return false;

		array<ResourceName> memberPrefabs;
		entitySource.Get("m_aUnitPrefabSlots", memberPrefabs);

		if (!memberPrefabs || memberPrefabs.IsEmpty())
			return false;

		int memberCount = memberPrefabs.Count();
		if (memberCount >= m_GroupSizeLarge)
		{
			label = EEditableEntityLabel.GROUPSIZE_LARGE;
			return true;
		}
		else if (memberCount >= m_GroupSizeMedium)
		{
			label = EEditableEntityLabel.GROUPSIZE_MEDIUM;
			return true;
		}
		else if (memberCount >= m_GroupSizeSmall)
		{
			label = EEditableEntityLabel.GROUPSIZE_SMALL;
			return true;
		}

		return false;
	}
}
