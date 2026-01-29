//! Base class of all group settings
//! It does not support action cause value
[BaseContainerProps(visible: false, insertable: false)]
class SCR_AIGroupSetting : SCR_AISettingBase
{
	/*!
	When making own setting classes, DO NOT declare own constructor to pass data.
	Instead, make a static factory Create() method. See any derived class for example.
	The Setting class must be compatible both with manual construction and construction as property.
	Declaring custom constructor will cause the constructor to be called with incorrect data when Setting object is created inside property.
	*/
	
	override int GetMaxActionCause() { return ACTION_CAUSE_NOT_SUPPORTED; }
}


// Base class for AI group settings which support cause
[BaseContainerProps(visible: false, insertable: false)]
class SCR_AIGroupSettingWithCause : SCR_AIGroupSetting
{
	[Attribute(typename.EnumToString(SCR_EAIActivityCause, SCR_EAIActivityCause.ALWAYS), UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_EAIActivityCause), desc: "The setting applies to actions of this cause and all below it (higher in the combo box list)")]
	protected SCR_EAIActivityCause m_eCause;
	
	//---------------------------------------------------------------------------------------------------
	protected void Init(SCR_EAISettingOrigin origin, SCR_EAIActivityCause cause)
	{
		Init(origin);
		m_eCause = cause;
	}
	
	//---------------------------------------------------------------------------------------------------
	override int GetMaxActionCause()
	{
		return m_eCause;
	}
}