[BaseContainerProps(configRoot: true)]
class SCR_BoneSpeedSignalData : Managed
{
	[Attribute("", UIWidgets.EditBox, "Bone to be tracked")]
	protected string m_sTrackedBoneName;
	
	[Attribute("", UIWidgets.EditBox, "Bone to compare speed against. If not defined tracked bone speed calculated")]
	protected string m_sPivotBoneName;
	
	[Attribute("", UIWidgets.EditBox, "Signal name")]
	protected string m_sSignalName;
	
	//------------------------------------------------------------------------------------------------
	string GetTrackedBoneName()
	{
		return m_sTrackedBoneName;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetPivotBoneName()
	{
		return m_sPivotBoneName;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetSignalName()
	{
		return m_sSignalName;
	}
}