class SCR_EstablishBaseMapCommandCursor : SCR_MapCommandCursor
{
	protected Faction m_CommanderFaction;
	protected int m_iPlayerId;
	protected IEntity m_HoveredEntity;
	protected ENotification m_eBlockingNotification;
	protected BaseWorld m_World;
	protected float m_fOceanBaseHeight;
	protected bool m_bIsOcean;
	protected float m_fSignalRangeOffset;
	protected SCR_MapEntity m_MapEntity;
	protected ImageWidget m_wEstablishRad;
	protected ImageWidget m_wBuildingRad;
	protected ImageWidget m_wRelayRad;
	protected ImageWidget m_wRadioRad;
	protected Widget m_wBaseRadii;
	protected float m_fEstablishDiameter;
	protected float m_fBuildingDiameter;
	protected float m_fRelayDiameter;
	protected float m_fRadioDiameter;
	protected float m_fFrameOffset;

	protected const int CONSTRUCTION_TRUCK_BUILDING_RADIUS = 35;
	protected const ResourceName BASE_PREFAB = "{1391CE8C0E255636}Prefabs/Systems/MilitaryBase/ConflictMilitaryBase.et";
	protected const ResourceName BASE_RADII_DISPLAY_LAYOUT = "{9D4CD0EA93AC1FD9}UI/layouts/Map/MapEstablishBaseRadii.layout";
	protected const string WIDGET_NAME_ESTABLISHING_RADIUS = "BaseEstablishingRadius";
	protected const string WIDGET_NAME_BUILDING_RADIUS = "BaseBuildingRadius";
	protected const string WIDGET_NAME_RADIO_RADIUS = "BaseRadioRadius";
	protected const string WIDGET_NAME_RELAY_RADIUS = "BaseRelayRadius";
	protected const string CONTAINER_PROPERTY_NAME_BUIDL_RADIUS = "m_iRadius";
	protected const string CONTAINER_PROPERTY_NAME_RADIO_RANGE = "Transmitting Range";
	protected const string CONTAINER_PROPERTY_NAME_TRANS_LIST = "Transceivers";

	protected const int RELAY_TRANSMITING_RANGE_DIAMETER = 4000;

	//------------------------------------------------------------------------------------------------
	int GetPlayerId()
	{
		return m_iPlayerId;
	}

	//------------------------------------------------------------------------------------------------
	void SetData(notnull Faction commanderFaction, int playerId, IEntity hoveredEntity)
	{
		m_CommanderFaction = commanderFaction;
		m_iPlayerId = playerId;
		m_HoveredEntity = hoveredEntity;
	}

	//------------------------------------------------------------------------------------------------
	override void ShowCursor(vector startCursorPosition)
	{
		super.ShowCursor(startCursorPosition);

		m_MapEntity = SCR_MapEntity.GetMapInstance();
		if (!m_MapEntity)
			return;

		Widget mapRoot = m_MapEntity.GetMapMenuRoot();
		if (!mapRoot)
			return;

		Widget mapFrame = mapRoot.FindAnyWidget(SCR_MapConstants.MAP_FRAME_NAME);
		if (!mapFrame)
			return;

		SCR_CampaignFactionCommanderHandlerComponent commanderHandlerComp = SCR_CampaignFactionCommanderHandlerComponent.Cast(SCR_FactionCommanderHandlerComponent.GetInstance());
		if (!commanderHandlerComp)
			return;

		int buildRadius;
		float baseRadioRangeRadius;
		if (!FindDefaultBuildAndRadioRange(buildRadius, baseRadioRangeRadius))
			return;
		
		m_fEstablishDiameter = commanderHandlerComp.GetBaseMinDistance(SCR_ECampaignBaseType.BASE) * 2;
		m_fBuildingDiameter = buildRadius * 2;
		m_fRadioDiameter = (baseRadioRangeRadius + m_fSignalRangeOffset) * 2;
		m_fRelayDiameter = GetDefaultRelayDiameter() + m_fSignalRangeOffset * 2;

		WorkspaceWidget workspace = mapFrame.GetWorkspace();
		m_wBaseRadii = workspace.CreateWidgets(BASE_RADII_DISPLAY_LAYOUT, mapFrame);
		if (!m_wBaseRadii)
			return;

		m_wEstablishRad = ImageWidget.Cast(m_wBaseRadii.FindAnyWidget(WIDGET_NAME_ESTABLISHING_RADIUS));
		m_wRadioRad = ImageWidget.Cast(m_wBaseRadii.FindAnyWidget(WIDGET_NAME_RADIO_RADIUS));
		m_wRelayRad = ImageWidget.Cast(m_wBaseRadii.FindAnyWidget(WIDGET_NAME_RELAY_RADIUS));
		m_wBuildingRad = ImageWidget.Cast(m_wBaseRadii.FindAnyWidget(WIDGET_NAME_BUILDING_RADIUS));

		UpdateRadiiSize(workspace.DPIUnscale(m_MapEntity.GetCurrentZoom()));

		m_MapEntity.GetOnMapZoom().Insert(OnMapZoom);	// zoom for scaling
		GetGame().GetCallqueue().CallLater(UpdateLayoutPosition, repeat: true);
		SCR_CampaignFeedbackComponent feedbackComp = SCR_CampaignFeedbackComponent.GetInstance();
		if (feedbackComp)
			feedbackComp.ShowHint(EHint.CONFLICT_BUILD_REQUEST_BASE_ESTABLISH_LEGEND, true, true);
	}

	//------------------------------------------------------------------------------------------------
	override void DisableSelection(bool toggleOffCursors = true)
	{
		super.DisableSelection(toggleOffCursors);

		if (!m_wBaseRadii)
			return;

		m_wBaseRadii.RemoveFromHierarchy();
		GetGame().GetCallqueue().Remove(UpdateLayoutPosition);
		m_MapEntity.GetOnMapZoom().Remove(OnMapZoom);	// zoom for scaling
		SCR_CampaignFeedbackComponent feedbackComp = SCR_CampaignFeedbackComponent.GetInstance();
		if (feedbackComp)
			feedbackComp.HideHint(EHint.CONFLICT_BUILD_REQUEST_BASE_ESTABLISH_LEGEND);
	}

	//------------------------------------------------------------------------------------------------
	//! Fetches from default base prefab information about build and radio radius
	//! \param[in] buildRadius radius in which players can build in the base
	//! \param[in] baseRadioRadius radius in which base relays the radio transmitions/signal
	protected bool FindDefaultBuildAndRadioRange(out int buildRadius, out float baseRadioRadius)
	{
		Resource res = Resource.Load(BASE_PREFAB);
		if (!res)
			return false;

		BaseResourceObject object = res.GetResource();
		if (!object)
			return false;

		IEntitySource entitySrc = object.ToEntitySource();
		if (!entitySrc)
			return false;

		IEntityComponentSource component;
		BaseContainerList transceiversList;
		BaseContainer transceiver;
		typename componentType;
		for (int i, count = entitySrc.GetComponentCount(); i < count; i++)
		{
			component = entitySrc.GetComponent(i);
			componentType = component.GetClassName().ToType();
			if (!componentType)
				continue;

			if (componentType.IsInherited(SCR_MilitaryBaseComponent))
			{
				if (!component.Get(CONTAINER_PROPERTY_NAME_BUIDL_RADIUS, buildRadius))
					return false;
			}
			else if (componentType.IsInherited(SCR_CoverageRadioComponent))
			{
				transceiversList = component.GetObjectArray(CONTAINER_PROPERTY_NAME_TRANS_LIST);
				for (int transId, transCount = transceiversList.Count(); transId < transCount; transId++)
				{
					transceiver = transceiversList.Get(transId);
					if (!transceiver.GetClassName().ToType().IsInherited(RelayTransceiver))
						continue;

					if (!transceiver.Get(CONTAINER_PROPERTY_NAME_RADIO_RANGE, baseRadioRadius))
						return false;
				}
			}
		}

		return buildRadius > 0 && baseRadioRadius > 0;
	}

	//------------------------------------------------------------------------------------------------
	protected float GetDefaultRelayDiameter()
	{
		return RELAY_TRANSMITING_RANGE_DIAMETER;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] zoomVal 
	protected void UpdateRadiiSize(float zoomVal)
	{
		if (m_wEstablishRad)
			m_wEstablishRad.SetSize(m_fEstablishDiameter * zoomVal, m_fEstablishDiameter * zoomVal);

		if (m_wBuildingRad)
			m_wBuildingRad.SetSize(m_fBuildingDiameter * zoomVal, m_fBuildingDiameter * zoomVal);

		if (m_wRadioRad)
			m_wRadioRad.SetSize(m_fRadioDiameter * zoomVal, m_fRadioDiameter * zoomVal);

		if (m_wRelayRad)
			m_wRelayRad.SetSize(m_fRelayDiameter * zoomVal, m_fRelayDiameter * zoomVal);

		float x, y;
		m_wBaseRadii.GetScreenSize(x,y);
		m_fFrameOffset = x * 0.5; // these are circles, thus their widgets are square, so we only use X for the offset
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateLayoutPosition()
	{
		if (!m_MapEntity)
			return;

		if (!m_wBaseRadii)
			return;

		WorkspaceWidget workspace = m_wBaseRadii.GetWorkspace();
		if (!workspace)
			return;

		float cursorX = workspace.DPIScale(SCR_MapCursorInfo.x);
		float cursorY = workspace.DPIScale(SCR_MapCursorInfo.y);
		FrameSlot.SetPos(m_wBaseRadii, SCR_MapCursorInfo.x, SCR_MapCursorInfo.y);
	}

	//------------------------------------------------------------------------------------------------
	//! SCR_MapEntity event triggered when player zooms in or out while viewing the map
	//! \param[in] zoomVal
	protected void OnMapZoom(float zoomVal)
	{
		if (!m_wBaseRadii)
			return;

		UpdateRadiiSize(m_wBaseRadii.GetWorkspace().DPIUnscale(zoomVal));
	}

	//------------------------------------------------------------------------------------------------
	override protected bool CanExecuteCommand(vector position)
	{
		m_eBlockingNotification = ENotification.UNKNOWN;

		SCR_GameModeCampaign gameModeCampaign = SCR_GameModeCampaign.Cast(GetGame().GetGameMode());
		if (!gameModeCampaign || !gameModeCampaign.GetEstablishingBasesEnabled())
			return false;

		SCR_EstablishBaseTaskEntity hoveredTask = SCR_EstablishBaseTaskEntity.Cast(m_HoveredEntity);
		if (hoveredTask)
			return true;

		if (!gameModeCampaign.GetBaseManager().CanFactionBuildNewBase(m_CommanderFaction))
		{
			m_eBlockingNotification = ENotification.GROUP_TASK_ESTABLISH_BASE_LIMIT_REACHED;
			return false;
		}

		if (m_World && m_bIsOcean && m_World.GetSurfaceY(position[0], position[2]) < m_fOceanBaseHeight)
		{
			m_eBlockingNotification = ENotification.GROUP_TASK_ESTABLISH_BASE_INVALID_AREA;
			return false;
		}

		if (!gameModeCampaign.GetBaseManager().IsPositionInFactionRadioSignal(position, m_CommanderFaction, m_fSignalRangeOffset))
		{
			m_eBlockingNotification = ENotification.GROUP_TASK_ESTABLISH_BASE_OUTSIDE_RADIO_RANGE;
			return false;
		}

		if (SCR_FactionCommanderEstablishBaseMenuHandlerHelper.IsNearAnyBase(m_CommanderFaction, position))
		{
			m_eBlockingNotification = ENotification.GROUP_TASK_ESTABLISH_BASE_TOO_CLOSE_ANOTHER_BASE;
			return false;
		}

		if (SCR_FactionCommanderEstablishBaseMenuHandlerHelper.IsNearAnyEstablishTask(m_CommanderFaction, position))
		{
			m_eBlockingNotification = ENotification.GROUP_TASK_ESTABLISH_BASE_TOO_CLOSE_ANOTHER_OBJECTIVE;
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnCommandNotExecuted(vector position)
	{
		super.OnCommandNotExecuted(position);

		if (m_eBlockingNotification != ENotification.UNKNOWN)
			SCR_NotificationsComponent.SendLocal(m_eBlockingNotification);
	}

	//------------------------------------------------------------------------------------------------
	void SCR_EstablishBaseMapCommandCursor()
	{
		// The maximum distance from the base at which an establish base objective can be placed must be reduced by establish base radius + construction truck building radius
		// It serves as a preventive measure so that a newly built base can always connect to the nearest base.
		m_fSignalRangeOffset = -SCR_CampaignFactionCommanderHandlerComponent.Cast(SCR_FactionCommanderHandlerComponent.GetInstance()).GetBaseEstablishingRadius() - CONSTRUCTION_TRUCK_BUILDING_RADIUS;

		m_World = GetGame().GetWorld();
		if (!m_World)
			return;

		if (!m_World.IsOcean())
			return;

		m_bIsOcean = true;
		m_fOceanBaseHeight = m_World.GetOceanBaseHeight();
	}
}
