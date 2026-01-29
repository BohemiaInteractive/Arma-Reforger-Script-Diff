//------------------------------------------------------------------------------------------------
//! Marker dynamic entry base
[BaseContainerProps(), SCR_MapMarkerTitle()]
class SCR_MapMarkerEntryDynamic: SCR_MapMarkerEntryConfig
{
	[Attribute("{DD74BE2BBAE07192}Prefabs/Markers/MapMarkerEntityBase.et", UIWidgets.ResourceNamePicker, desc: "Marker prefab used in replication", params: "et")]
	protected ResourceName m_sMarkerPrefab;
	
	protected SCR_MapMarkerManagerComponent m_MarkerMgr;
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetMarkerPrefab()
	{
		return m_sMarkerPrefab;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Override this to set up logic & event behavior on server
	override void InitServerLogic()
	{
		 m_MarkerMgr = SCR_MapMarkerManagerComponent.GetInstance();
	}
		
	//------------------------------------------------------------------------------------------------
	override void InitClientSettingsDynamic(notnull SCR_MapMarkerEntity marker, notnull SCR_MapMarkerDynamicWComponent widgetComp)
	{
		super.InitClientSettingsDynamic(marker, widgetComp);
		
		widgetComp.SetText(marker.GetText());
		
		ResourceName imageset;
		string quad;
		marker.GetImageResource(imageset, quad);
		widgetComp.SetImage(imageset, quad);
	}
}