[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityLabel, "m_Label")]
class EditablePrefabsLabel_Theme : EditablePrefabsLabel_Base
{
	[Attribute()]
	protected string m_FolderName;

	//------------------------------------------------------------------------------------------------
	override bool GetLabelValid(WorldEditorAPI api, IEntitySource entitySource, IEntityComponentSource componentSource, string targetPath, EEditableEntityType entityType, notnull array<EEditableEntityLabel> authoredLabels, out EEditableEntityLabel label)
	{
		array<string> folderNames = {};
		targetPath.Split("/", folderNames, true);

		foreach (string folderName : folderNames)
		{
			if (folderName == m_FolderName)
			{
				label = m_Label;
				return true;
			}
		}

		return false;
	}
}
