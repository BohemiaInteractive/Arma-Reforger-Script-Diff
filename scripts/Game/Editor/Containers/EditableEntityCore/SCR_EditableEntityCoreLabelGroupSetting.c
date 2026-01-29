[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityLabelGroup, "m_LabelGroupType")]
class SCR_EditableEntityCoreLabelGroupSetting
{
	[Attribute()]
	protected ref SCR_UIInfo m_Info;
	
	[Attribute("0", desc: "Order of this filter groups, lowest value is shown/returned first")]
	private int m_Order;
	
	[Attribute("0", UIWidgets.ComboBox, "", enums: ParamEnumArray.FromEnum(EEditableEntityLabelGroup))]
	private EEditableEntityLabelGroup m_LabelGroupType;
	
	[Attribute("0", UIWidgets.ComboBox, "", enums: ParamEnumArray.FromEnum(EEditableEntityLabel))]
	private EEditableEntityLabel m_ConditionalLabel;
	
	[Attribute("0", desc: "All active labels in this group need to match for the entity to show, 0=OR 1=AND")]
	private bool m_bRequiresAllLabelsMatch;
	
	SCR_UIInfo GetInfo()
	{
		return m_Info;
	}
	
	int GetOrder()
	{
		return m_Order;
	}
	
	EEditableEntityLabelGroup GetLabelGroupType()
	{
		return m_LabelGroupType;
	}
	EEditableEntityLabel GetConditionalLabelType()
	{
		return m_ConditionalLabel;
	}
	
	bool GetRequiresAllLabelsMatch()
	{
		return m_bRequiresAllLabelsMatch;
	}
};