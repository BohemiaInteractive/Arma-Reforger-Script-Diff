[BaseContainerProps()]
class SCR_SpawnPointBlacklistZone
{
	[Attribute(uiwidget: UIWidgets.Coords, desc: "Blacklist zone center", params: "inf inf purpose=coords space=world anglesVar=Angles")]
	vector m_vCenter;

	[Attribute("500", desc: "Blacklist zone radius", params: "0.1 inf")]
	float m_fRadius;
}