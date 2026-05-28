[BaseContainerProps()]
class SCR_CacheLocationData
{
	[Attribute(desc: "Description")]
	string m_sDescription;
	
	[Attribute(uiwidget: UIWidgets.Coords, desc: "World coordinates for the cache spawn", params: "inf inf purpose=coords space=world anglesVar=Angles")]
	vector m_vPosition;
}