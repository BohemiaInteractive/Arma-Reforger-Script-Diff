/**
Shows dynamic description if value meets the condition
*/
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EAttributeValueDynamicDescription, "m_eDisplayIfValue", "Value: %1")]
class SCR_ValueAttributeDynamicDescription : SCR_BaseAttributeDynamicDescription
{
	[Attribute(desc: "If the current value is x to value given condition than display the description", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EAttributeValueDynamicDescription))]
	protected EAttributeValueDynamicDescription m_eDisplayIfValue;
	
	[Attribute(desc: "The value condition that is changed compaired to the attribute value")]
	protected float m_fValueCondition;
	
	[Attribute("0", desc: "If true it will only show the description if the condition was false before and is now true (meaning if you save the attributes and open it again it will not show the warning), else it will always set the warning if the condition is true wether it was changed or not")]
	protected bool m_bOnlyShowIfConditionChanged;
	
	protected float m_fStartingValue;
	
	//------------------------------------------------------------------------------------------------
	override void InitDynamicDescription(notnull SCR_BaseEditorAttribute attribute, notnull SCR_BaseEditorAttributeUIComponent attributeUi)
	{
		super.InitDynamicDescription(attribute);
		
		//~ No need to set starting value if always checks if condition true
		if (!m_bOnlyShowIfConditionChanged)
			return;
		
		SCR_BaseEditorAttributeVar var = attribute.GetVariableOrCopy();
		if (!var)
			return;
		
		m_fStartingValue = var.GetFloat();
	}
	
	//------------------------------------------------------------------------------------------------
	override bool IsValid(notnull SCR_BaseEditorAttribute attribute, notnull SCR_BaseEditorAttributeUIComponent attributeUi)
	{
		if (!super.IsValid(attribute, attributeUi))
			return false;
		
		SCR_BaseEditorAttributeVar var = attribute.GetVariableOrCopy();
		if (!var)
			return false;
		
		float value = var.GetFloat();
		
		switch (m_eDisplayIfValue)
		{
			case EAttributeValueDynamicDescription.LESS_THAN :
			{
				if (!m_bOnlyShowIfConditionChanged)
					return value < m_fValueCondition;
					
				return m_fStartingValue >= m_fValueCondition && value < m_fValueCondition;
			}
			case EAttributeValueDynamicDescription.LESS_OR_EQUAL_THAN :
			{
				if (!m_bOnlyShowIfConditionChanged)
					return value <= m_fValueCondition;

				return m_fStartingValue > m_fValueCondition && value <= m_fValueCondition;
			}
			case EAttributeValueDynamicDescription.EQUAL_TO :
			{
				if (!m_bOnlyShowIfConditionChanged)
					return value == m_fValueCondition;
				
				return value != m_fStartingValue && value == m_fValueCondition;
			}
			case EAttributeValueDynamicDescription.GREATER_THAN :
			{
				if (!m_bOnlyShowIfConditionChanged)
					return value > m_fValueCondition;
				 
				return m_fStartingValue <= m_fValueCondition && value > m_fValueCondition;
			}
			case EAttributeValueDynamicDescription.GREATER_OR_EQUAL_THAN :
			{
				if (!m_bOnlyShowIfConditionChanged)
					return value >= m_fValueCondition;
				
				return m_fStartingValue < m_fValueCondition && value >= m_fValueCondition;
			}
			case EAttributeValueDynamicDescription.NOT_EQUAL_TO :
			{
				if (!m_bOnlyShowIfConditionChanged)
					return value != m_fValueCondition;
				
				return value != m_fStartingValue && value != m_fValueCondition;
			}
		}
		
		return false;
	}
};

enum EAttributeValueDynamicDescription
{
	LESS_THAN = 0,
	LESS_OR_EQUAL_THAN = 1,
	EQUAL_TO = 2,
	GREATER_THAN = 3,
	GREATER_OR_EQUAL_THAN = 4,
	NOT_EQUAL_TO = 5,
};