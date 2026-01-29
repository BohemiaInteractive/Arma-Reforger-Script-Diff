[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityState, "m_State")]
/** @ingroup Editor_Components_Entities
*/
/*!
*/
class SCR_EntityTypesEditableEntityFilter : SCR_BaseEditableEntityFilter
{
	[Attribute(uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EEditableEntityType))]
	private ref array<EEditableEntityType> m_aTypes;
	
	override bool CanAdd(SCR_EditableEntityComponent entity)
	{
		return m_aTypes.Contains(entity.GetEntityType());
	}
};