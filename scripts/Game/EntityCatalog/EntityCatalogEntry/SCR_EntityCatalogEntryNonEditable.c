/**
Entity Entry within the SCR_EntityCatalog. This is meant for NON-EDITABLE ENTITIES ONLY! For editable entities use SCR_EntityCatalogEntry!
*/
[BaseContainerProps(), BaseContainerCustomStringTitleField("DO NOT USE THIS CLASS - It is unable to obtain UIInfo (Use inherit versions)!")]
class SCR_EntityCatalogEntryNonEditable : SCR_EntityCatalogEntry
{	
	[Attribute(desc: "Set the labels for the non-editable entity. NOTE! Only use this when the Prefab is non-editable otherwise it will take these labels instead of the EditableEntity Labels on the prefab. If this is required add your own Labels to a custom SCR_BaseEntityCatalogData to prevent breaking data for others", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EEditableEntityLabel))]
	protected ref array<EEditableEntityLabel> m_aLabels;
	
	//======================================== LABELS ========================================\\
	//--------------------------------- Get all Labels ---------------------------------\\
	override int GetEditableEntityLabels(notnull out array<EEditableEntityLabel> editableEntityLables)
	{
		editableEntityLables.Copy(m_aLabels);
		return editableEntityLables.Count();
	}
	
	//--------------------------------- Has Label ---------------------------------\\
	override bool HasEditableEntityLabel(EEditableEntityLabel editableEntityLabel)
	{
		return m_aLabels.Contains(editableEntityLabel);
	}
	
	//--------------------------------- Has Any label ---------------------------------\\
	override bool HasAnyEditableEntityLabels(notnull array<EEditableEntityLabel> editableEntityLables)
	{
		foreach (EEditableEntityLabel label: editableEntityLables)
		{
			if (HasEditableEntityLabel(label))
				return true;
		}
		
		return false;
	}
	
	//--------------------------------- Has all labels ---------------------------------\\
	override bool HasAllEditableEntityLabels(notnull array<EEditableEntityLabel> editableEntityLables)
	{
		foreach (EEditableEntityLabel label: editableEntityLables)
		{
			if (!HasEditableEntityLabel(label))
				return false;
		}
		
		return true;
	}
	
	//--------------------------------- Class Specific Init ---------------------------------\\
	//~ Base NonEditable has no class specific init
	override void ClassSpecificInit()
	{
	
	}
}