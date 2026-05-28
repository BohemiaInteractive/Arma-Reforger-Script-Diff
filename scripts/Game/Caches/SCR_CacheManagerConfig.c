[BaseContainerProps(configRoot: true)]
class SCR_CacheTerrainConfig : ScriptAndConfig
{
	[Attribute(uiwidget: UIWidgets.Object, params: "inf inf purpose=coords space=world anglesVar=Angles", category: "Cache Config", desc: "List of all possible cache locations")]
	ref array<ref SCR_CacheLocationData> m_aCachePool;
}