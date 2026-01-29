//------------------------------------------------------------------------------------------------
[EntityEditorProps(category: "GameScripted/Markers")]
class SCR_MapMarkerEntityClass : GenericEntityClass
{
	[Attribute(defvalue: "1", desc:"Update delay in seconds", params: "0 inf")]
	protected float m_fUpdateDelayData;

	//------------------------------------------------------------------------------------------------
	float GetUpdateDelay()
	{
		return m_fUpdateDelayData;
	}
};

//------------------------------------------------------------------------------------------------
//! Dynamic map marker entity base class 
//! Spawned by marker manager when creating a dynamic marker -> see marker config for customization
class SCR_MapMarkerEntity : GenericEntity
{	
	[RplProp(onRplName: "OnUpdateType")]
	protected SCR_EMapMarkerType m_eType;
	
	[RplProp()]
	protected int m_iConfigID = -1;			// config id used when marker of a single type has bigger amount of configuration options
	
	[RplProp(condition: RplCondition.NoOwner, onRplName: "OnUpdatePosition")]
	protected vector m_vPos;
	
	[RplProp(onRplName: "OnUpdateVisibility")]
	protected bool m_bIsGlobalVisible; 					// is this marker visible based on server main conditions, such as it having assigned target
	
	protected bool m_bIsLocalVisible = true;			// is this marker visible to the player based on custom local conditions
	protected int m_iScreenX;
	protected int m_iScreenY;
	protected float m_fUpdateDelay = 1;
	protected float m_fTimeTracker;
	
	protected string m_sText;
	protected ResourceName m_sImageset;
	protected string m_sIconName;
	protected SCR_MapMarkerEntryDynamic m_ConfigEntry; 	// marker entry associated with this marker type
	
	protected Widget m_wRoot;
	protected SCR_MapEntity m_MapEntity;
	protected SCR_MapMarkerDynamicWComponent m_MarkerWidgetComp;
	
	// server side
	protected IEntity m_Target;
	protected Faction m_MarkerFaction;					// if null, no streaming restrictions are applied
	
	//------------------------------------------------------------------------------------------------
	// RPL EVENTS
	//------------------------------------------------------------------------------------------------
	//! used client side for initial visibility set
	protected void OnUpdateType()
	{		
		OnUpdateVisibility();
	}
	
	//------------------------------------------------------------------------------------------------
	//! for override within children classes
	protected void OnUpdatePosition()
    {}
		
	//------------------------------------------------------------------------------------------------
	//! Create or destroy marker widget based on current state
	protected void OnUpdateVisibility()
	{
		SCR_MapEntity mapEnt = SCR_MapEntity.GetMapInstance();
		if (!mapEnt || !mapEnt.IsOpen() || mapEnt.GetMapUIComponent(SCR_MapMarkersUI) == null)	// map not open or makers not enabled
			return;
		
		if (!IsVisible() && m_wRoot)	// should not be visible
			OnDelete();
	
		if (IsVisible() && !m_wRoot)	// should be visible
			OnCreateMarker();
	}
	
	//------------------------------------------------------------------------------------------------
	// API CLIENT		
	//------------------------------------------------------------------------------------------------
	void SetLocalVisible(bool state)
	{
		m_bIsLocalVisible = state;
		OnUpdateVisibility();
	}
		
	//------------------------------------------------------------------------------------------------
	SCR_EMapMarkerType GetType()
	{
		return m_eType;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetMarkerConfigID()
	{
		return m_iConfigID;
	}
		
	//------------------------------------------------------------------------------------------------
	void SetText(string text)
	{
		m_sText = text;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetText()
	{
		return m_sText;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetImage(string imageset, string icon)
	{
		m_sImageset = imageset;
		m_sIconName = icon;
	}
	
	//------------------------------------------------------------------------------------------------
	void GetImageResource(out ResourceName imageset, out string imageQuad)
	{
		imageset = m_sImageset;
		imageQuad = m_sIconName;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get world position coordinates
	vector GetWorldPos()
	{
		return m_vPos;
	}
	
	//------------------------------------------------------------------------------------------------
	// API SERVER
	//------------------------------------------------------------------------------------------------
	void SetType(SCR_EMapMarkerType type, int configID = -1)
	{
		m_eType = type;
		OnUpdateType();
		
		if (configID != -1)
			m_iConfigID = configID;
		
		Replication.BumpMe();
	}
		
	//------------------------------------------------------------------------------------------------
	void SetGlobalVisible(bool state)
	{
		m_bIsGlobalVisible = state;
		OnUpdateVisibility();
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return entity this marker is tracking
	IEntity GetTarget()
	{
		return m_Target;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set entity this marker is tracking
	void SetTarget(IEntity target)
	{
		m_Target = target;
		
		if (target)
			SetEventMask(EntityEvent.FRAME);
		else 
			ClearEventMask(EntityEvent.FRAME);
	}
	
	//------------------------------------------------------------------------------------------------
	Faction GetFaction()
	{
		return m_MarkerFaction;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetFaction(Faction faction)
	{
		SCR_MapMarkerManagerComponent markerMgr = SCR_MapMarkerManagerComponent.GetInstance();
		if (!markerMgr)
			return;
		
		m_MarkerFaction = faction;
		
		markerMgr.SetMarkerStreamRules(this);
	}
	
	//------------------------------------------------------------------------------------------------
	// EVENTS & OTHERS
	//------------------------------------------------------------------------------------------------
	protected bool IsVisible()
	{
		return m_bIsGlobalVisible && m_bIsLocalVisible;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Fetch marker definition from config & create widget
	void OnCreateMarker()
	{
		if (!IsVisible())
			return;
		
		if (!m_MapEntity)
			m_MapEntity = SCR_MapEntity.GetMapInstance();
		
		if (!m_ConfigEntry)
			m_ConfigEntry = SCR_MapMarkerEntryDynamic.Cast(SCR_MapMarkerManagerComponent.GetInstance().GetMarkerConfig().GetMarkerEntryConfigByType(m_eType));
		
		if (!m_MapEntity || !m_ConfigEntry)
			return;
		
		Widget mapFrame = m_MapEntity.GetMapMenuRoot().FindAnyWidget(SCR_MapConstants.MAP_FRAME_NAME);
		if (!mapFrame)
			return;
		
		m_wRoot = GetGame().GetWorkspace().CreateWidgets(m_ConfigEntry.GetMarkerLayout(), mapFrame);
		if (!m_wRoot)
			return;

		m_MarkerWidgetComp = SCR_MapMarkerDynamicWComponent.Cast(m_wRoot.FindHandler(SCR_MapMarkerDynamicWComponent));
		m_MarkerWidgetComp.SetMarkerEntity(this);	// todo needs base class
		m_ConfigEntry.InitClientSettingsDynamic(this, m_MarkerWidgetComp);
		
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
			m_ConfigEntry.OnMapLayerChangedDynamic(m_MarkerWidgetComp, layerID);
	} 
	
	//------------------------------------------------------------------------------------------------
	//! Called from SCR_MapMarkerManagerComponent
	void OnUpdate()
	{
		if (!m_wRoot)
			return;
		
		m_MapEntity.WorldToScreen(m_vPos[0], m_vPos[2], m_iScreenX, m_iScreenY, true);
		FrameSlot.SetPos(m_wRoot, GetGame().GetWorkspace().DPIUnscale(m_iScreenX), GetGame().GetWorkspace().DPIUnscale(m_iScreenY));	// needs unscaled coords
	}
	
	//------------------------------------------------------------------------------------------------
	// OVERRIDES
	//------------------------------------------------------------------------------------------------
	override protected void EOnInit(IEntity owner)
	{
		SCR_MapMarkerManagerComponent markerMgr = SCR_MapMarkerManagerComponent.GetInstance();
		if (!markerMgr)
			return;
		
		markerMgr.RegisterDynamicMarker(this);	
		
		BaseRplComponent rplComp = BaseRplComponent.Cast(FindComponent(BaseRplComponent));
		if (rplComp.IsOwner())			// Only authority runs frame update
		{			
			SetFlags(EntityFlags.ACTIVE, true);
			SetEventMask(EntityEvent.FRAME);
		}

		SCR_MapMarkerEntityClass markerEntityClass = SCR_MapMarkerEntityClass.Cast(GetPrefabData());
		if (markerEntityClass)
			m_fUpdateDelay = markerEntityClass.GetUpdateDelay();

		m_fTimeTracker = m_fUpdateDelay; // tick first update instantly
	}
	
	//------------------------------------------------------------------------------------------------
	//! Authority side update
	override protected void EOnFrame(IEntity owner, float timeSlice)
	{		
		if (!m_Target)
			return;
		
		m_fTimeTracker += timeSlice;
		if (m_fTimeTracker >= m_fUpdateDelay)
		{
			m_fTimeTracker = 0;
			m_vPos = m_Target.GetOrigin();
			Replication.BumpMe();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_MapMarkerEntity(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_MapMarkerEntity() 
	{
		if (SCR_MapEntity.GetMapInstance() && SCR_MapEntity.GetMapInstance().IsOpen())
			OnDelete();	
		
		SCR_MapMarkerManagerComponent markerMgr = SCR_MapMarkerManagerComponent.GetInstance();
		if (markerMgr)
			markerMgr.UnregisterDynamicMarker(this);
	}
	
}
