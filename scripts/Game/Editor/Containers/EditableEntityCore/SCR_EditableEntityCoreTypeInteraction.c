[BaseContainerProps(), SCR_EditableEntityCoreTypeInteractionTitle()]
class SCR_EditableEntityCoreTypeInteraction
{
	[Attribute("0", UIWidgets.ComboBox, "", enums: ParamEnumArray.FromEnum(EEditableEntityType))]
	private EEditableEntityType m_TargetType;
	
	[Attribute("", UIWidgets.ComboBox, "", enums: ParamEnumArray.FromEnum(EEditableEntityInteraction))]
	private EEditableEntityInteraction m_Interaction;
	
	EEditableEntityType GetType()
	{
		return m_TargetType;
	}
	EEditableEntityInteraction GetInteraction()
	{
		return m_Interaction;
	}
};

class SCR_EditableEntityCoreTypeInteractionTitle: BaseContainerCustomTitle
{
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		EEditableEntityType type;
		if (!source.Get("m_TargetType", type)) return false;
		
		EEditableEntityInteraction interaction;
		if (!source.Get("m_Interaction", interaction)) return false;
		
		title = string.Format("with %1, interact as %2", typename.EnumToString(EEditableEntityType, type), typename.EnumToString(EEditableEntityInteraction, interaction));
		return true;
	}
};