//! Map marker object base class 
//! Created through SCR_MapMarkerManagerComponent API
class SCR_MapMarkerBase
{
	protected const int SERIALIZED_BYTES = 34;	// total amount of serialized bytes without custom string
	
	// synchronized 
	protected SCR_EMapMarkerType m_eType;	// config type
	protected int m_iMarkerID = -1;			// network ID, -1 means the marker is not set as synchronized
	protected int m_iConfigID = -1;			// config id used when marker of a single type has bigger amount of configuration options
	protected int m_iMarkerOwnerID = -1;	// owner playerID, -1 is dedicated server
	protected int m_iFlags;
	protected int m_iPosWorldX;
	protected int m_iPosWorldY;
	protected int m_iFactionFlags;			// flags determining which factions are able to see the marker, 0 means no restrictions
	protected int m_iColorEntry;			// placed marker color entry id
	protected int m_iIconEntry;				// placed marker icon entry id
	protected int m_iRotation;
	protected string m_sCustomText;
	protected WorldTimestamp m_Timestamp;
	protected bool m_bIsTimestampVisible;
	
	// server only
	protected bool m_bIsServerSideDisabled; // in hosted server scenario, opposing faction markers have to be properly managed but still disabled from showing up
	protected bool m_bCanBeRemovedByOwner = true; // can this marker be deleted by user in map.

	// rest
	protected bool m_bTestVisibleFrame = true;			// only run update based on presence in visible frame
	protected bool m_bIsUpdateDisabled;				// is update currently disbled
	protected bool m_bIsDragged;						// currently being dragged
	protected bool m_bIsBlocked;						// is marker blocked from showing to the user (ex. UGC restrictions)
	protected int m_iScreenX;							// cached screen position X
	protected int m_iScreenY;						 	// cached screen position Y
	protected SCR_MapMarkerEntryConfig m_ConfigEntry; 	// marker entry associated with this marker type
	protected SCR_MapEntity m_MapEntity;
	protected ref Widget m_wRoot;
	protected SCR_MapMarkerWidgetComponent m_MarkerWidgetComp;
	protected ref Color m_Color;
		
	//------------------------------------------------------------------------------------------------
	Widget GetRootWidget()
	{
		return m_wRoot;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_EMapMarkerType GetType()
	{
		return m_eType;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetType(SCR_EMapMarkerType type)
	{
		m_eType = type;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetMarkerID()
	{
		return m_iMarkerID;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetMarkerID(int id)
	{
		m_iMarkerID = id;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetMarkerConfigID()
	{
		return m_iConfigID;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetMarkerConfigID(int id)
	{
		m_iConfigID = id;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetMarkerOwnerID()
	{
		return m_iMarkerOwnerID;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetMarkerOwnerID(int playerID)
	{
		m_iMarkerOwnerID = playerID;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetFlags()
	{
		return m_iFlags;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetFlags(int flags)
	{
		m_iFlags = flags;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Get faction flags representing indices of factions within FactionManager prefab
	int GetMarkerFactionFlags()
	{
		return m_iFactionFlags;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Add faction index to flags which you can acquire by calling GetFactionIndex()
	void AddMarkerFactionFlags(int flags)
	{
		if (flags < 0) // invalid id
			return;
		
		int flag = Math.Pow(2, flags);
		
		m_iFactionFlags |= flag;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set faction flags directly without converting the faction indices
	void SetMarkerFactionFlags(int flags)
	{
		m_iFactionFlags = flags;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check whether the marker is one of the defined list of factions
	bool IsFaction(int factionID)
	{
		int flag = Math.Pow(2, factionID);
		
		return (m_iFactionFlags & flag);
	}
	
	//------------------------------------------------------------------------------------------------
	void GetWorldPos(out int pos[2])
	{
		pos[0] = m_iPosWorldX;
		pos[1] = m_iPosWorldY;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetWorldPos(int posX, int posY)
	{
		m_iPosWorldX = posX;
		m_iPosWorldY = posY;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetRotation()
	{
		return m_iRotation;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetRotation(int angle)
	{
		m_iRotation = angle;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetColorEntry()
	{
		return m_iColorEntry;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetIconEntry()
	{
		return m_iIconEntry;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetColorEntry(int colorEntry)
	{
		m_iColorEntry = colorEntry;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetIconEntry(int iconEntry)
	{
		m_iIconEntry = iconEntry;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetCustomText()
	{
		return m_sCustomText;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetCustomText(string text)
	{
		m_sCustomText = text;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return int timestamp when the marker was created
	WorldTimestamp GetTimestamp()
	{
		return m_Timestamp;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] timestamp when the marker was created
	void SetTimestamp(WorldTimestamp timestamp)
	{
		m_Timestamp = timestamp;
	}

	//------------------------------------------------------------------------------------------------
	void SetTimestampVisibility(bool isVisible)
	{
		m_bIsTimestampVisible = isVisible;
	}

	//------------------------------------------------------------------------------------------------
	bool IsTimestampVisible()
	{
		return m_bIsTimestampVisible;
	}

	//------------------------------------------------------------------------------------------------
	//! Disable marker UI display on server -> for dedicated servers(no UI) or hosted server enemy faction 
	void SetServerDisabled(bool state)
	{
		m_bIsServerSideDisabled = state;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Disable posibility to remove marker from map by owner.
	void SetCanBeRemovedByOwner(bool state)
	{
		m_bCanBeRemovedByOwner = state
	}
	
	//------------------------------------------------------------------------------------------------
	bool CanBeRemovedByOwner()
	{
		return m_bCanBeRemovedByOwner;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Disable marker update based on visibility on screen, overriden by m_bIsBlocked
	void SetUpdateDisabled(bool state)
	{
		if (m_bIsBlocked && !state)
			SetUpdateDisabled(true);
		
		m_bIsUpdateDisabled = state;
		
		if (m_wRoot && m_wRoot.IsVisible() == state)
			m_wRoot.SetVisible(!state);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets if marker is blocked (this changes visibility)
	void SetBlocked(bool state)
	{
		m_bIsBlocked = state;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets if marker is blocked (this changes visibility)
	bool GetBlocked()
	{
		return m_bIsBlocked;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Used for temporary reasons such as hiding original marker during edit, not for filtering visiblity
	void SetVisible(bool state)
	{
		if (m_wRoot)
			m_wRoot.SetVisible(state);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set dragged state
	void SetDragged(bool state)
	{
		m_bIsDragged = state;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_MapMarkerWidgetComponent GetMarkerComponent()
	{
		return m_MarkerWidgetComp;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Test whether the marker is visible on screen
	bool TestVisibleFrame(vector visibleMin, vector visibleMax)
	{
		if(    m_iPosWorldX < visibleMin[0] 
			|| m_iPosWorldX > visibleMax[0] 
			|| m_iPosWorldY < visibleMin[2]
			|| m_iPosWorldY > visibleMax[2])
		{
			return false;
		}
		else 
		{
			SetUpdateDisabled(false);
			return true;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Requests profanity filtering for the text of this marker
	void RequestProfanityFilter()
	{
		SCR_MapMarkerManagerComponent mapMarkerManager = SCR_MapMarkerManagerComponent.GetInstance();
		if (!mapMarkerManager)
			return;

		SCR_ScriptProfanityFilterRequestCallback profanityCallback = mapMarkerManager.RequestProfanityFilter(m_sCustomText);
		if (!profanityCallback)
			return;
		
		profanityCallback.m_OnResult.Insert(OnProfanityFilteredCallback);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnProfanityFilteredCallback(array<string> texts)
	{
		if (texts.IsEmpty())
			return;

		m_sCustomText = texts[0];
		if (m_MarkerWidgetComp)
			m_MarkerWidgetComp.SetText(m_sCustomText, true);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Fetch marker definition from config & create widget
	//! \param[in] skipProfanityFilter determines whether the marker text should be checked for profanity
	void OnCreateMarker(bool skipProfanityFilter = false)
	{
		if (m_bIsServerSideDisabled)
			return;
		
		m_MapEntity = SCR_MapEntity.GetMapInstance();
		if (!m_MapEntity)
			return;
		
		Widget mapRoot = m_MapEntity.GetMapMenuRoot();
		if (!mapRoot)
			return;
		
		Widget mapFrame = mapRoot.FindAnyWidget(SCR_MapConstants.MAP_FRAME_NAME);
		if (!mapFrame)
			return;
		
		m_ConfigEntry = SCR_MapMarkerManagerComponent.GetInstance().GetMarkerConfig().GetMarkerEntryConfigByType(m_eType);
		if (!m_ConfigEntry)
			return;
		
		m_wRoot = GetGame().GetWorkspace().CreateWidgets(m_ConfigEntry.GetMarkerLayout(), mapFrame);
		if (!m_wRoot)
			return;

		m_MarkerWidgetComp = SCR_MapMarkerWidgetComponent.Cast(m_wRoot.FindHandler(SCR_MapMarkerWidgetComponent));
		m_MarkerWidgetComp.SetMarkerObject(this);
		m_ConfigEntry.InitClientSettings(this, m_MarkerWidgetComp, skipProfanityFilter);
		m_MarkerWidgetComp.SetRotation(m_iRotation);
		
		SCR_MapEntity.GetOnMapClose().Insert(OnMapClosed);
		SCR_MapEntity.GetOnLayerChanged().Insert(OnMapLayerChanged);
		OnMapLayerChanged(m_MapEntity.GetLayerIndex());
	}
	
	//------------------------------------------------------------------------------------------------
	void OnDelete()
	{
		if (m_wRoot)
			m_wRoot.RemoveFromHierarchy();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnMapClosed(MapConfiguration config)
	{
		SCR_MapEntity.GetOnMapClose().Remove(OnMapClosed);
		SCR_MapEntity.GetOnLayerChanged().Remove(OnMapLayerChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnMapLayerChanged(int layerID)
	{
		if (m_MarkerWidgetComp)
			m_MarkerWidgetComp.SetLayerID(layerID);
		
		 LayerChangeLogic(layerID);
	}
	
	//------------------------------------------------------------------------------------------------
	void LayerChangeLogic(int layerID)
	{
		if (m_ConfigEntry && m_MarkerWidgetComp)
			m_ConfigEntry.OnMapLayerChanged(m_MarkerWidgetComp, layerID);
	} 
	
	//------------------------------------------------------------------------------------------------
	//! Called from SCR_MapMarkerManagerComponent
	bool OnUpdate(vector visibleMin = vector.Zero, vector visibleMax = vector.Zero)
	{
		if (m_bIsServerSideDisabled || m_bIsDragged || m_bIsBlocked)
			return true;
		
		if (m_bTestVisibleFrame)
		{
			if ((m_iPosWorldX < visibleMin[0]) || (m_iPosWorldX > visibleMax[0]) || (m_iPosWorldY < visibleMin[2]) || (m_iPosWorldY > visibleMax[2]))
			{
				SetUpdateDisabled(true);
				return false;
			}
		}
		
		m_MapEntity.WorldToScreen(m_iPosWorldX, m_iPosWorldY, m_iScreenX, m_iScreenY, true);
		FrameSlot.SetPos(m_wRoot, GetGame().GetWorkspace().DPIUnscale(m_iScreenX), GetGame().GetWorkspace().DPIUnscale(m_iScreenY));	// needs unscaled coords
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Extract(SCR_MapMarkerBase instance, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
		snapshot.SerializeInt(instance.m_iPosWorldX);
		snapshot.SerializeInt(instance.m_iPosWorldY);
		snapshot.SerializeInt(instance.m_iMarkerID);
		snapshot.SerializeInt(instance.m_iMarkerOwnerID);
		snapshot.SerializeInt(instance.m_iFlags);
		snapshot.SerializeInt(instance.m_iConfigID);
		snapshot.SerializeInt(instance.m_iFactionFlags);
		snapshot.SerializeBytes(instance.m_iRotation, 2);
		snapshot.SerializeBytes(instance.m_eType, 1);
		snapshot.SerializeBytes(instance.m_iColorEntry, 1);
		snapshot.SerializeBytes(instance.m_iIconEntry, 2);
		snapshot.SerializeString(instance.m_sCustomText);
		snapshot.SerializeBool(instance.m_bIsTimestampVisible);
		snapshot.SerializeBytes(instance.m_Timestamp, 8);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	static bool Inject(SSnapSerializerBase snapshot, ScriptCtx ctx, SCR_MapMarkerBase instance)
	{
		snapshot.SerializeInt(instance.m_iPosWorldX);
		snapshot.SerializeInt(instance.m_iPosWorldY);
		snapshot.SerializeInt(instance.m_iMarkerID);
		snapshot.SerializeInt(instance.m_iMarkerOwnerID);
		snapshot.SerializeInt(instance.m_iFlags);
		snapshot.SerializeInt(instance.m_iConfigID);
		snapshot.SerializeInt(instance.m_iFactionFlags);
		snapshot.SerializeBytes(instance.m_iRotation, 2);
		snapshot.SerializeBytes(instance.m_eType, 1);
		snapshot.SerializeBytes(instance.m_iColorEntry, 1);
		snapshot.SerializeBytes(instance.m_iIconEntry, 2);
		snapshot.SerializeString(instance.m_sCustomText);
		snapshot.SerializeBool(instance.m_bIsTimestampVisible);
		snapshot.SerializeBytes(instance.m_Timestamp, 8);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	static void Encode(SSnapSerializerBase snapshot, ScriptCtx ctx, ScriptBitSerializer packet)
	{
		snapshot.Serialize(packet, SERIALIZED_BYTES);
		snapshot.EncodeString(packet);
		snapshot.EncodeBool(packet); // m_bIsTimestampVisible
		snapshot.Serialize(packet, 8); // m_Timestamp
	}

	//------------------------------------------------------------------------------------------------
	static bool Decode(ScriptBitSerializer packet, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
		snapshot.Serialize(packet, SERIALIZED_BYTES);
		snapshot.DecodeString(packet);
		snapshot.DecodeBool(packet); // m_bIsTimestampVisible
		snapshot.Serialize(packet, 8); // m_Timestamp
		return true;
	}

	//------------------------------------------------------------------------------------------------
	static bool SnapCompare(SSnapSerializerBase lhs, SSnapSerializerBase rhs , ScriptCtx ctx)
	{
		return lhs.CompareSnapshots(rhs, SERIALIZED_BYTES)	// m_iPosWorldX(4) + m_iPosWorldY(4) + m_iMarkerID(4) + m_iMarkerOwnerID(4) + m_iFlags(4) + m_iConfigID(4) + m_iFactionFlags(4) + m_iRotation(2) + m_eType(1) + m_iColorEntry(1) + m_iIconEntry(2)
			&& lhs.CompareStringSnapshots(rhs) // m_sCustomText
			&& lhs.CompareSnapshots(rhs, 4 + 8); // m_bIsTimestampVisible + m_Timestamp
	}

	//------------------------------------------------------------------------------------------------
	static bool PropCompare(SCR_MapMarkerBase instance, SSnapSerializerBase snapshot, ScriptCtx ctx)
	{
		return snapshot.CompareInt(instance.m_iPosWorldX)
			&& snapshot.CompareInt(instance.m_iPosWorldY) 
			&& snapshot.CompareInt(instance.m_iMarkerID)
			&& snapshot.CompareInt(instance.m_iMarkerOwnerID)
			&& snapshot.CompareInt(instance.m_iFlags)
			&& snapshot.CompareInt(instance.m_iConfigID)
			&& snapshot.CompareInt(instance.m_iFactionFlags)
			&& snapshot.Compare(instance.m_iRotation, 2)
			&& snapshot.Compare(instance.m_eType, 1)
			&& snapshot.Compare(instance.m_iColorEntry, 1)
			&& snapshot.Compare(instance.m_iIconEntry, 2)
			&& snapshot.CompareString(instance.m_sCustomText)
			&& snapshot.CompareBool(instance.m_bIsTimestampVisible)
			&& snapshot.Compare(instance.m_Timestamp, 8);
	}
};
