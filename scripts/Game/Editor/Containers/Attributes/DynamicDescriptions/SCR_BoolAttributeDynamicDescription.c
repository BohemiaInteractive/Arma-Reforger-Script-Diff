/**
Shows dynamic description if bool meets the condition
*/
[BaseContainerProps(), BaseContainerCustomCheckIntTitleField("m_bShowDescriptionOnBool", "Bool condition True", "Bool condition False", 1)]
class SCR_BoolAttributeDynamicDescription : SCR_BaseAttributeDynamicDescription
{
	[Attribute("0", desc: "If bool equals this value than the condition is shown")]
	protected bool m_bShowDescriptionOnBool;
	
	[Attribute("0", desc: "If true it will only show the description if the condition was false before and is now true (meaning if you save the attributes and open it again it will not show the warning), else it will always set the warning if the condition is true wether it was changed or not")]
	protected bool m_bOnlyShowIfConditionChanged;
	
	protected bool m_bStartingValue;
	
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
		
		m_bStartingValue = var.GetBool();
	}
	
	//------------------------------------------------------------------------------------------------
	override bool IsValid(notnull SCR_BaseEditorAttribute attribute, notnull SCR_BaseEditorAttributeUIComponent attributeUi)
	{
		if (!super.IsValid(attribute, attributeUi))
			return false;
		
		SCR_BaseEditorAttributeVar var = attribute.GetVariableOrCopy();
		if (!var)
			return false;
		
		if (!m_bOnlyShowIfConditionChanged)
			return m_bShowDescriptionOnBool == var.GetBool();
		else 
			return m_bShowDescriptionOnBool == var.GetBool() && m_bStartingValue != var.GetBool();
	}
};