[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(SCR_EServicePointType, "m_eServicePointType")]
class SCR_BaseServiceIconInfo
{
	[Attribute(defvalue: "0", uiwidget: UIWidgets.ComboBox, desc: "The SCR_EServicePointType value", enumType: SCR_EServicePointType)]
	protected SCR_EServicePointType m_eServicePointType;
	
	[Attribute(defvalue: "0", uiwidget: UIWidgets.ComboBox, desc: "The EEditableEntityLabel for the building service icon", enumType: EEditableEntityLabel)]
	protected EEditableEntityLabel m_eEditableEntityLabel;

	[Attribute("{F7E8D4834A3AFF2F}UI/Imagesets/Conflict/conflict-icons-bw.imageset", desc: "The imageset containing the icon")]
	protected ResourceName m_sImageSet;

	[Attribute("", desc: "The name of the icon in the imageset")]
	protected string m_sIconSetName;

	[Attribute("", desc: "The localized display name of the icon")]
	protected string m_sServiceName;

	//------------------------------------------------------------------------------------------------
	SCR_EServicePointType GetServicePointType()
	{
		return m_eServicePointType;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_EServicePointType GetEntityLabel()
	{
		return m_eEditableEntityLabel;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetImageSet()
	{
		return m_sImageSet;
	}

	//------------------------------------------------------------------------------------------------
	string GetIconName()
	{
		return m_sIconSetName;
	}

	//------------------------------------------------------------------------------------------------
	string GetServiceName()
	{
		return m_sServiceName;
	}
}