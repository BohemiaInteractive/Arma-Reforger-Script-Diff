/**
Holds logic when a dynamic description should be shown when attributes are hovered over. Includes extra logic for button attributes. Do not use base class
*/
[BaseContainerProps(), BaseContainerCustomStringTitleField("DO NOT USE BASE CLASS")]
class SCR_BaseButtonAttributeDynamicDescription : SCR_BaseAttributeDynamicDescription
{
	[Attribute("1", desc: "If true it then it has priority over button descriptions if hovering over a attribute button, making sure it is always shown first")]
	protected bool m_bPriorityOverButtonDescription;
	
	//------------------------------------------------------------------------------------------------
	/*!
	Return true if has priority over button descriptions
	\return True if priority over button descriptions
	*/
	bool HasPriorityOverButton()
	{
		return m_bPriorityOverButtonDescription;
	}
};
