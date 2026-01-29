//------------------------------------------------------------------------------------------------
//! Marker dynamic entry example
//! Utilizes marker system to display player positions to everyone in the session -> to see this work, configure it within MapMarkerConfig.conf
[BaseContainerProps(), SCR_MapMarkerTitle()]
class SCR_MapMarkerEntryDynamicExample : SCR_MapMarkerEntryDynamic
{
	// Optional: Define attributes for anything you want to be able to change through MapMarkerConfig instead of script	
	[Attribute("", UIWidgets.Object, "Visual configuration")]
	protected ref SCR_MarkerSimpleConfig m_EntryConfig;
	
	//------------------------------------------------------------------------------------------------
	//
	// Logic for creating & deleting the markers, in this case we just use gamemode events to create marker on player spawn and remove them on player death/deletion 
	//
	//------------------------------------------------------------------------------------------------
	//! SCR_BaseGameMode event
	void OnPlayerSpawned(int playerId, IEntity player)
	{
		if (m_MarkerMgr.GetDynamicMarkerByTarget(GetMarkerType(), player))
			return;
		
		SCR_MapMarkerEntity markerEnt = m_MarkerMgr.InsertDynamicMarker(GetMarkerType(), player, 1);
		markerEnt.SetGlobalVisible(true);
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_BaseGameMode event
	void OnPlayerKilled(notnull SCR_InstigatorContextData instigatorContextData)
	{		
		SCR_MapMarkerEntity markerEnt = m_MarkerMgr.GetDynamicMarkerByTarget(GetMarkerType(), instigatorContextData.GetVictimEntity());
		if (markerEnt)
			m_MarkerMgr.RemoveDynamicMarker(markerEnt);
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_BaseGameMode event
	void OnPlayerDeleted(int playerId, IEntity player)
	{		
		SCR_MapMarkerEntity markerEnt = m_MarkerMgr.GetDynamicMarkerByTarget(GetMarkerType(), player);
		if (markerEnt)
			m_MarkerMgr.RemoveDynamicMarker(markerEnt);
	}
	
	//------------------------------------------------------------------------------------------------
	//
	// Unique marker type is required for dynamic markers because each requires a custom create/remove logic -> define it here!
	// While we only add this class to marker config, it needs to be searchable by type using this enum 
	//
	//------------------------------------------------------------------------------------------------
	override SCR_EMapMarkerType GetMarkerType()
	{
	 	return SCR_EMapMarkerType.DYNAMIC_EXAMPLE;
	}
	
	//------------------------------------------------------------------------------------------------
	//
	// This is called during marker widget creation in order to set up marker visuals
	// Any visual attributes you add to this class should be set through the SCR_MapMarkerDynamicWComponent component here
	// If SCR_MapMarkerDynamicWComponent is not enough for the job, you will have to create a separate marker layout (assign it in marker config entry) and append your own widget component there
	//
	//------------------------------------------------------------------------------------------------
	override void InitClientSettingsDynamic(notnull SCR_MapMarkerEntity marker, notnull SCR_MapMarkerDynamicWComponent widgetComp)
	{
		super.InitClientSettingsDynamic(marker, widgetComp);
		
		string imgset, icon;
		m_EntryConfig.GetIconResource(imgset, icon);
		
		widgetComp.SetImage(imgset, icon);
		widgetComp.SetColor(m_EntryConfig.GetColor());
		widgetComp.SetText(m_EntryConfig.GetText());
	}
	
	//------------------------------------------------------------------------------------------------
	//
	// This serves as entry point and is called automatically by marekr manager when you append this class in the marker config
	// Logic for creation/removal is setup from here
	//
	//------------------------------------------------------------------------------------------------
	override void InitServerLogic()
	{	
		super.InitServerLogic();
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode)
			return;
				
		gameMode.GetOnPlayerSpawned().Insert(OnPlayerSpawned);
		gameMode.GetOnPlayerKilled().Insert(OnPlayerKilled);
		gameMode.GetOnPlayerDeleted().Insert(OnPlayerDeleted);
	}
}