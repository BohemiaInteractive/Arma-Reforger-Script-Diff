class SCR_MapLocatorClass : GenericEntityClass
{
};

class SCR_MapLocator : GenericEntity
{
	[Attribute("{2229B9CC8917D3A3}UI/layouts/Map/MapLocationHint.layout", params: "et class=layout")]
	protected ResourceName m_sUIHintLayout;
	protected Widget m_wUIHintLayout;

	[Attribute("MapLocationHint_Text")]
	protected string m_sMapLocationHintWidgetName1;
	[Attribute("MapLocationHint_Text2")]
	protected string m_sMapLocationHintWidgetName2;

	protected TextWidget m_wUIHintText;
	protected TextWidget m_wUIHintText2;

    [Attribute()]
    protected ref SCR_MapLocationQuadHint m_WorldDirections;

	protected int m_iGridSizeX;
	protected int m_iGridSizeY;
	
	protected const float angleA = 0.775;
	protected const float angleB = 0.325;
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		if (!GetGame().InPlayMode() || !m_WorldDirections) 
			return;
		SCR_MapEntity.GetOnMapOpen().Insert(ShowMapHint);
		SCR_MapEntity.GetOnMapClose().Insert(HideMapHint);
		
		vector mins,maxs;
		GetGame().GetWorldEntity().GetWorldBounds(mins, maxs);
		
		m_iGridSizeX = maxs[0]/3;
		m_iGridSizeY = maxs[2]/3;
	}
	
	protected void ShowMapHint(MapConfiguration config)
	{
		//--- ToDo: Don't hardcode conditions, let the map config define whether to show location hint or not
		if (SCR_DeployMenuMain.GetDeployMenu())
			return;
		m_wUIHintLayout = GetGame().GetWorkspace().CreateWidgets(m_sUIHintLayout);
		if (!m_wUIHintLayout)
			return;
		m_wUIHintText = TextWidget.Cast(m_wUIHintLayout.FindAnyWidget(m_sMapLocationHintWidgetName1));
		m_wUIHintText2= TextWidget.Cast(m_wUIHintLayout.FindAnyWidget(m_sMapLocationHintWidgetName2));
		CalculateClosestLocation();
		GetGame().GetCallqueue().CallLater(CalculateClosestLocation,10000,true);
	}
	
	protected void HideMapHint(MapConfiguration config)
	{
		if (m_wUIHintLayout)
			m_wUIHintLayout.RemoveFromHierarchy();
		m_wUIHintLayout = null;
		m_wUIHintText = null;
		m_wUIHintText2 = null;
		GetGame().GetCallqueue().Remove(CalculateClosestLocation);
	}
	
	protected void CalculateClosestLocation()
	{
		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
	       if (!core)
	           return;
		ChimeraCharacter player = ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		if (!player)
		{
			if (m_wUIHintLayout)
			{
				m_wUIHintLayout.RemoveFromHierarchy();
				m_wUIHintLayout = null;
			}
			return;
		}
		vector posPlayer = player.GetOrigin();
		
		SCR_EditableEntityComponent nearest = core.FindNearestEntity(posPlayer, EEditableEntityType.COMMENT, EEditableEntityFlag.LOCAL);
		if (!nearest)
			return;
		GenericEntity nearestLocation = nearest.GetOwner();
		SCR_MapDescriptorComponent mapDescr = SCR_MapDescriptorComponent.Cast(nearestLocation.FindComponent(SCR_MapDescriptorComponent));
		string closestLocationName;
		if (!mapDescr)
			return;
		MapItem item = mapDescr.Item();
		closestLocationName = item.GetDisplayName();

		vector lastLocationPos = nearestLocation.GetOrigin();
		float lastDistance = vector.DistanceSqXZ(lastLocationPos, posPlayer);
	
		LocalizedString closeLocationAzimuth;
		vector result = posPlayer - lastLocationPos;
		result.Normalize();
	
		float angle1 = vector.DotXZ(result,vector.Forward);
		float angle2 = vector.DotXZ(result,vector.Right);
			
		if (angle2 > 0)
		{
			if (angle1 >= angleA)
				closeLocationAzimuth = "#AR-MapLocationHint_DirectionNorth";
			if (angle1 < angleA && angle1 >= angleB )
				closeLocationAzimuth = "#AR-MapLocationHint_DirectionNorthEast";
			if (angle1 < angleB && angle1 >=-angleB)
				closeLocationAzimuth = "#AR-MapLocationHint_DirectionEast";
			if (angle1 < -angleB && angle1 >=-angleA)
				closeLocationAzimuth = "#AR-MapLocationHint_DirectionSouthEast";
			if (angle1 < -angleA)
				closeLocationAzimuth = "#AR-MapLocationHint_DirectionSouth";
		}
		else
		{
			if (angle1 >= angleA)
				closeLocationAzimuth = "#AR-MapLocationHint_DirectionNorth";
			if (angle1 < angleA && angle1 >= angleB )
				closeLocationAzimuth = "#AR-MapLocationHint_DirectionNorthWest";
			if (angle1 < angleB && angle1 >=-angleB)
				closeLocationAzimuth = "#AR-MapLocationHint_DirectionWest";
			if (angle1 < -angleB && angle1 >=-angleA)
				closeLocationAzimuth = "#AR-MapLocationHint_DirectionSouthWest";
			if (angle1 < -angleA)
				closeLocationAzimuth = "#AR-MapLocationHint_DirectionSouth";
		};
		
		int playerGridPositionX = posPlayer[0]/m_iGridSizeX;
		int playerGridPositionY = posPlayer[2]/m_iGridSizeY;
		
		int playerGridID = GetGridIndex(playerGridPositionX,playerGridPositionY);
		m_wUIHintText.SetTextFormat(m_WorldDirections.GetPlayerPositionHint(), m_WorldDirections.GetQuadHint(playerGridID));
		if (lastDistance < 40000)
			m_wUIHintText2.SetTextFormat("#AR-MapLocationHint_PlayerPositionNear", closestLocationName);
		else
		{
			m_wUIHintText2.SetVisible(!closestLocationName.IsEmpty());
			m_wUIHintText2.SetTextFormat(closeLocationAzimuth, closestLocationName);
		}
	}
	
	protected int GetGridIndex(int x, int y)
	{
		return 3*y + x;
	}

	//------------------------------------------------------------------------------------------------
	void SCR_MapLocator(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(configRoot: true)]
class SCR_MapLocationQuadHint
{
	[Attribute("#AR-MapLocationHint_PlayerPositionIslandName")]
	protected LocalizedString m_sPlayerPositionHint;

	[Attribute(desc: "Description of given quad. Order: SW, S, SE, W, C, E, NW, N, NE", params: "MaxSize=9")]
	protected ref array<LocalizedString> m_aQuadHints;

	//------------------------------------------------------------------------------------------------
	LocalizedString GetPlayerPositionHint()
	{
		return m_sPlayerPositionHint;
	}

	LocalizedString GetQuadHint(int index)
	{
		if (!m_aQuadHints.IsIndexValid(index))
			return LocalizedString.Empty;

		return m_aQuadHints[index];
	}
};