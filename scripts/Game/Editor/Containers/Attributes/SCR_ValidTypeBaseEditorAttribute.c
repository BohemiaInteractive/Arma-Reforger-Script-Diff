
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_ValidTypeBaseEditorAttribute : SCR_BaseEditorAttribute
{	
	[Attribute("0", UIWidgets.ComboBox, "If editing entities of type then this attribute will only show if this type (ignored if empty) (attribute needs to support this)", "", ParamEnumArray.FromEnum(EEditableEntityType))]
	protected ref array<ref EEditableEntityType> m_aWhiteListEntityTypes;
	
	[Attribute("0", UIWidgets.ComboBox, "If editing entities of type then this attribute will not show (attribute needs to support this)", "", ParamEnumArray.FromEnum(EEditableEntityType))]
	protected ref array<ref EEditableEntityType> m_aBlackListEntityTypes;
	
	protected bool IsValidEntityType(EEditableEntityType entityType)
	{
		if (!m_aWhiteListEntityTypes.IsEmpty())
		{
			return m_aWhiteListEntityTypes.Contains(entityType);
		}
		else
		{
			return !m_aBlackListEntityTypes.Contains(entityType);
		}
	}
};

	