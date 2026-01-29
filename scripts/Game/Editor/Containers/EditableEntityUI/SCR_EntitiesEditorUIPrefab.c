[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityType, "m_Type")]
class SCR_EntitiesEditorUIPrefab
{
	[Attribute("0", UIWidgets.ComboBox, "Entity type.", enums: ParamEnumArray.FromEnum(EEditableEntityType))]
	private EEditableEntityType m_Type;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Layout created for the entity and rendered on its position every frame.", "layout")]
	private ResourceName m_Layout;
	
	EEditableEntityType GetType()
	{
		return m_Type;
	}
	ResourceName GetLayout()
	{
		return m_Layout;
	}
};