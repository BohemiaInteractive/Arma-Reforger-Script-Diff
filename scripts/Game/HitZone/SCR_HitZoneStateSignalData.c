[BaseContainerProps(configRoot: true)]
class SCR_HitZoneStateSignalData
{	
	[Attribute("", UIWidgets.Auto, "Signal name")]
	string m_sSignalName;
	
	[Attribute("", UIWidgets.Auto, "Parent hitzone name")]
	ref array<string> m_aHitZoneNames;
}