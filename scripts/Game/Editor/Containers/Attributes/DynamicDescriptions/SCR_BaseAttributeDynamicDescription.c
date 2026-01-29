/**
Holds logic when a dynamic description should be shown when attributes are hovered over. Do not use base class
*/
[BaseContainerProps(), BaseContainerCustomStringTitleField("DO NOT USE BASE CLASS")]
class SCR_BaseAttributeDynamicDescription
{
	[Attribute(desc: "It uses color, icon and description to display dynamic description")]
	protected ref SCR_EditorAttributeUIInfo m_DescriptionDisplayInfo;
	
	[Attribute("1", desc: "If false than it will never be shown")]
	protected bool m_bEnabled;
	
	//------------------------------------------------------------------------------------------------
	/*!
	Init the dynamic description
	\param attribute Attribute linked to the desciption
	*/
	void InitDynamicDescription(notnull SCR_BaseEditorAttribute attribute, notnull SCR_BaseEditorAttributeUIComponent attributeUi);
	
	//------------------------------------------------------------------------------------------------
	/*!
	Check if conditions are valid to display the Dynamic description
	\param attribute Attribute linked to the desciption
	\return true if valid
	*/
	bool IsValid(notnull SCR_BaseEditorAttribute attribute, notnull SCR_BaseEditorAttributeUIComponent attributeUi)
	{
		return m_bEnabled;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Init the dynamic description
	\param attribute Attribute linked to the desciption
	\param[out] uiInfo UiInfo with icon, description and color to display the description
	\param[out] param1 Optional param 1 to add to the description
	\param[out] param2 Optional param 2 to add to the description
	\param[out] param3 Optional param 3 to add to the description
	*/
	void GetDescriptionData(notnull SCR_BaseEditorAttribute attribute, notnull SCR_BaseEditorAttributeUIComponent attributeUi, out SCR_EditorAttributeUIInfo uiInfo, out string param1 = string.Empty, out string param2 = string.Empty, out string param3 = string.Empty)
	{
		uiInfo = m_DescriptionDisplayInfo;
	}
};

