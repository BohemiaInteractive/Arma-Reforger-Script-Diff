[ComponentEditorProps(category: "GameScripted/Misc", description: "")]
class SCR_MapMarkerSyncComponentClass : ScriptComponentClass
{
}

//! Used for Client->Server RPC ask methods for spawn and removal of static version of networked markers 
//! Attached to PlayerController
class SCR_MapMarkerSyncComponent : ScriptComponent
{
	protected bool m_bIsDeleteRestricted = true;	// server side only, only allow marker owners to delete their placed markers  
	protected int m_iPlacedMarkerLimit = 10;		// server side only, limit of allowed synchronized markers per client
	protected ref array<int> m_OwnedMarkers = {}; 	// server side only, list of markers owned by this controller to enforce limits
	
	//------------------------------------------------------------------------------------------------
	//! Ask to add a networked marker
	//! Called by marker manager
	//! \param[in] marker
	void AskAddStaticMarker(notnull SCR_MapMarkerBase marker)
	{				
		Rpc(RPC_AskAddStaticMarker, marker);
	}
		
	//------------------------------------------------------------------------------------------------
	//! Ask to remove a networked marker
	//! Called by marker manager
	//! \param[in] markerID
	void AskRemoveStaticMarker(int markerID)
	{
		SCR_MapMarkerManagerComponent markerMgr = SCR_MapMarkerManagerComponent.GetInstance();
		if (!markerMgr)
			return;
		
		SCR_MapMarkerBase marker = markerMgr.GetStaticMarkerByID(markerID);
		if (!marker)	// client side sanity check
			return;
		
		Rpc(RPC_AskRemoveStaticMarker, markerID);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Clear owned markers
	//! Server side only
	//! Called by marker manager
	void ClearOwnedMarkers()
	{
		for (int i; i < m_OwnedMarkers.Count(); i++)
		{
			if (m_OwnedMarkers.IsIndexValid(i))
			{
				AskRemoveStaticMarker(m_OwnedMarkers[i]);
				i--;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Used only on server
	//! Updates current markers for enforcing limits directly within the SCR_MapMarkerSyncComponent of the marker owner's player controller
	void RemovePlayerMarker(int markerID)
	{
		m_OwnedMarkers.RemoveItemOrdered(markerID);
	}
	
	//------------------------------------------------------------------------------------------------
	// RPC
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
   	protected void RPC_AskAddStaticMarker(SCR_MapMarkerBase markerData)
	{	
		SCR_MapMarkerManagerComponent markerMgr = SCR_MapMarkerManagerComponent.GetInstance();
		if (!markerMgr || m_iPlacedMarkerLimit <= 0)
			return;
			
		if (m_OwnedMarkers.Count() >= m_iPlacedMarkerLimit)	// remove first if over limit
			Rpc(RPC_AskRemoveStaticMarker, m_OwnedMarkers[0]);
		
		markerMgr.AssignMarkerUID(markerData);
		
		markerData.SetMarkerOwnerID(SCR_PlayerController.Cast(GetOwner()).GetPlayerId());
		m_OwnedMarkers.Insert(markerData.GetMarkerID());
		
		markerMgr.OnAddSynchedMarker(markerData);	// add server side
		markerMgr.OnAskAddStaticMarker(markerData);
	}
			
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
   	protected void RPC_AskRemoveStaticMarker(int markerID)
	{	
		SCR_MapMarkerManagerComponent markerMgr = SCR_MapMarkerManagerComponent.GetInstance();
		if (!markerMgr)
			return;
		
		SCR_MapMarkerBase marker = markerMgr.GetStaticMarkerByID(markerID);
		if (!marker)
			marker = markerMgr.GetDisabledMarkerByID(markerID);

		if (!marker 
			|| marker.GetMarkerOwnerID() == -1 																				// cannot delete server marker by client request
			||(m_bIsDeleteRestricted && marker.GetMarkerOwnerID() != SCR_PlayerController.Cast(GetOwner()).GetPlayerId())) 	// delete by anyone not allowed && is not the callers marker
			return;

		if (marker.GetMarkerOwnerID() != -1)	// dedicated servers dont track their marker limits
		{
			PlayerController ownerController = GetGame().GetPlayerManager().GetPlayerController(marker.GetMarkerOwnerID());
			if (ownerController)
			{
				SCR_MapMarkerSyncComponent ownerSyncComp = SCR_MapMarkerSyncComponent.Cast(ownerController.FindComponent(SCR_MapMarkerSyncComponent));
				if (ownerSyncComp)
					ownerSyncComp.RemovePlayerMarker(markerID);	// this has to be called on this component of the markers owner, not the RPC sender 
			}
		}
		
		markerMgr.OnRemoveSynchedMarker(markerID);	// remove server side
		markerMgr.OnAskRemoveStaticMarker(markerID);
	}	
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SCR_MissionHeader header = SCR_MissionHeader.Cast(GetGame().GetMissionHeader());
		if (header)
		{
			m_iPlacedMarkerLimit = header.m_iMapMarkerLimitPerPlayer;
			m_bIsDeleteRestricted = !header.m_bMapMarkerEnableDeleteByAnyone;
		}
	}
}
