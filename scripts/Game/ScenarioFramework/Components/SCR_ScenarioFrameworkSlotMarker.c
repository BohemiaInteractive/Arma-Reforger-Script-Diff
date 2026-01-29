[EntityEditorProps(category: "GameScripted/ScenarioFramework/Slot", description: "")]
class SCR_ScenarioFrameworkSlotMarkerClass : SCR_ScenarioFrameworkSlotBaseClass
{
}

class SCR_ScenarioFrameworkSlotMarker : SCR_ScenarioFrameworkSlotBase
{
	[Attribute(desc: "Marker Type", category: "Map Marker")]
	protected ref SCR_ScenarioFrameworkMarkerType m_MapMarkerType;

	[Attribute(desc: "When enabled, map marker can by removed by owner on map.", category: "Map Marker")]
	protected bool m_bCanBeRemovedByOwner;

	protected ref SCR_MapMarkerBase m_MapMarker;
	protected SCR_MapMarkerDotCircle m_MarkerDotCircleEntity;

	//------------------------------------------------------------------------------------------------
	//! \return Represents the type of map marker in the scenario.
	SCR_ScenarioFrameworkMarkerType GetMapMarkerType()
	{
		return m_MapMarkerType;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] type Sets map marker type for scenario framework marker.
	void SetMapMarkerType(SCR_ScenarioFrameworkMarkerType type)
	{
		m_MapMarkerType = type;
	}

	//------------------------------------------------------------------------------------------------
	//! \return Represents the map marker object used by the method.
	SCR_MapMarkerBase GetMapMarker()
	{
		return m_MapMarker;
	}

	//------------------------------------------------------------------------------------------------
	//! Spawns map marker after all children spawned in scenario layer.
	//! \param[in] layer for which this is called.
	override void AfterAllChildrenSpawned(SCR_ScenarioFrameworkLayerBase layer)
	{
		super.AfterAllChildrenSpawned(this);

		if (!GetIsTerminated())
			CreateMapMarker();
	}

	//------------------------------------------------------------------------------------------------
	//! Restores default state, removes map marker, nullifies it, calls base method.
	//! \param[in] includeChildren Includes children objects in default restoration process.
	//! \param[in] reinitAfterRestoration Resets object after restoration, optionally reinitializing it.
	//! \param[in] affectRandomization Affects randomization during default restoration.
	override void RestoreToDefault(bool includeChildren = false, bool reinitAfterRestoration = false, bool affectRandomization = true)
	{
		super.RestoreToDefault(includeChildren, reinitAfterRestoration, affectRandomization);
		RemoveMapMarker();
	}

	//------------------------------------------------------------------------------------------------
	//! Creates a map marker based on provided type, sets its position, custom text, and adds it to map marker manager if possible
	protected void CreateMapMarker()
	{
		if (m_MarkerDotCircleEntity || m_MapMarker)
			return;

		SCR_ScenarioFrameworkMarkerDotCircle mapMarkerDotCircle = SCR_ScenarioFrameworkMarkerDotCircle.Cast(m_MapMarkerType);
		if (mapMarkerDotCircle)
		{
			if (m_MarkerDotCircleEntity)
				return;

			EntitySpawnParams params();
			params.TransformMode = ETransformMode.WORLD;
			GetOwner().GetTransform(params.Transform);
			m_MarkerDotCircleEntity = SCR_MapMarkerDotCircle.Cast(GetGame().SpawnEntityPrefabEx(mapMarkerDotCircle.m_sMarkerPrefab, false, params: params));
			if (!m_MarkerDotCircleEntity)
				return;

			m_MarkerDotCircleEntity.m_DotColor = mapMarkerDotCircle.m_DotColor;
			m_MarkerDotCircleEntity.m_fDotDensity = mapMarkerDotCircle.m_fDotDensity;
			m_MarkerDotCircleEntity.m_fRadius = mapMarkerDotCircle.m_fRadius;
		}

		SCR_MapMarkerManagerComponent mapMarkerMgr = SCR_MapMarkerManagerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_MapMarkerManagerComponent));
		if (!mapMarkerMgr)
			return;

		m_MapMarker = new SCR_MapMarkerBase();

		SCR_ScenarioFrameworkMarkerCustom mapMarkerCustom = SCR_ScenarioFrameworkMarkerCustom.Cast(m_MapMarkerType);
		if (mapMarkerCustom)
		{
			m_MapMarker.SetType(SCR_EMapMarkerType.PLACED_CUSTOM);
			m_MapMarker.SetIconEntry(mapMarkerCustom.m_eMapMarkerIcon);
			m_MapMarker.SetRotation(mapMarkerCustom.m_iMapMarkerRotation);
			m_MapMarker.SetColorEntry(mapMarkerCustom.m_eMapMarkerColor);
		}
		else
		{
			SCR_ScenarioFrameworkMarkerMilitary mapMarkerMilitary = SCR_ScenarioFrameworkMarkerMilitary.Cast(m_MapMarkerType);
			if (!mapMarkerMilitary)
				return;

			m_MapMarker = mapMarkerMgr.PrepareMilitaryMarker(mapMarkerMilitary.m_eMapMarkerFactionIcon, mapMarkerMilitary.m_eMapMarkerDimension, mapMarkerMilitary.m_eMapMarkerType1Modifier | mapMarkerMilitary.m_eMapMarkerType2Modifier);
		}

		vector worldPos = GetOwner().GetOrigin();
		m_MapMarker.SetWorldPos(worldPos[0], worldPos[2]);
		m_MapMarker.SetCustomText(m_MapMarkerType.m_sMapMarkerText);
		m_MapMarker.SetCanBeRemovedByOwner(m_bCanBeRemovedByOwner);

		FactionManager factionManager = GetGame().GetFactionManager();
		if (factionManager)
		{
			Faction faction = factionManager.GetFactionByKey(m_sFactionKey);
			if (faction)
				m_MapMarker.AddMarkerFactionFlags(factionManager.GetFactionIndex(faction));
		}

		mapMarkerMgr.InsertStaticMarker(m_MapMarker, false, true);
	}

	//------------------------------------------------------------------------------------------------
	//! Removes map marker by ID from map marker manager.
	void RemoveMapMarker()
	{
		if (m_MarkerDotCircleEntity)
			SCR_EntityHelper.DeleteEntityAndChildren(m_MarkerDotCircleEntity);

		if (!m_MapMarker)
			return;

		SCR_MapMarkerManagerComponent markerMgr = SCR_MapMarkerManagerComponent.GetInstance();
		if (!markerMgr)
			return;
		
		const int markerID = m_MapMarker.GetMarkerID();
		SCR_MapMarkerBase marker = markerMgr.GetStaticMarkerByID(markerID);
		if (marker)
			markerMgr.RemoveStaticMarker(marker);

		m_MapMarker = null;
	}

	//------------------------------------------------------------------------------------------------
	override void SetIsTerminated(bool state)
	{
		super.SetIsTerminated(state);
		if (state)
			RemoveMapMarker();
	}

	//------------------------------------------------------------------------------------------------
	//! Removes map marker when not in edit mode, despawns object if in edit mode.
	void ~SCR_ScenarioFrameworkSlotMarker()
	{
		if (SCR_Global.IsEditMode())
			return;

		DynamicDespawn(this);
		RemoveMapMarker();
	}
}

[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkMarkerType : ScriptAndConfig
{
	[Attribute(desc: "Text which will be displayed for the Map Marker", category: "Map Marker")]
	LocalizedString m_sMapMarkerText;
}

[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkMarkerCustom : SCR_ScenarioFrameworkMarkerType
{
	[Attribute("0", UIWidgets.ComboBox, "Marker Icon", "", ParamEnumArray.FromEnum(SCR_EScenarioFrameworkMarkerCustom), category: "Map Marker")]
	SCR_EScenarioFrameworkMarkerCustom m_eMapMarkerIcon;

	[Attribute("0", UIWidgets.ComboBox, "Marker Color", "", ParamEnumArray.FromEnum(SCR_EScenarioFrameworkMarkerCustomColor), category: "Map Marker")]
	SCR_EScenarioFrameworkMarkerCustomColor m_eMapMarkerColor;

	[Attribute(defvalue: "0", uiwidget: UIWidgets.Slider, desc: "Rotation of the Map Marker", params: "-180 180 1", category: "Map Marker")]
	int m_iMapMarkerRotation;
}

[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkMarkerMilitary : SCR_ScenarioFrameworkMarkerType
{
	[Attribute(defvalue: EMilitarySymbolIdentity.BLUFOR.ToString(), UIWidgets.ComboBox, "Marker Faction Icon. Not all of these combinations will work as they have to be properly defined in MapMarkerConfig.conf", "", ParamEnumArray.FromEnum(EMilitarySymbolIdentity), category: "Map Marker")]
	EMilitarySymbolIdentity m_eMapMarkerFactionIcon;

	[Attribute(defvalue: EMilitarySymbolDimension.LAND.ToString(), UIWidgets.ComboBox, "Marker Dimension. Not all of these combinations will work as they have to be properly defined in MapMarkerConfig.conf", "", ParamEnumArray.FromEnum(EMilitarySymbolDimension), category: "Map Marker")]
	EMilitarySymbolDimension m_eMapMarkerDimension;

	[Attribute(defvalue: EMilitarySymbolIcon.INFANTRY.ToString(), UIWidgets.ComboBox, "Marker Type 1 modifier. Not all of these combinations will work as they have to be properly defined in MapMarkerConfig.conf", "", ParamEnumArray.FromEnum(EMilitarySymbolIcon), category: "Map Marker")]
	EMilitarySymbolIcon m_eMapMarkerType1Modifier;

	[Attribute(defvalue: EMilitarySymbolIcon.INFANTRY.ToString(), UIWidgets.ComboBox, "Marker Type 2 modifier. Not all of these combinations will work as they have to be properly defined in MapMarkerConfig.conf", "", ParamEnumArray.FromEnum(EMilitarySymbolIcon), category: "Map Marker")]
	EMilitarySymbolIcon m_eMapMarkerType2Modifier;
}

[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkMarkerDotCircle : SCR_ScenarioFrameworkMarkerType
{
	[Attribute(defvalue: "1000", desc: "Meter Radius", params: "0 inf 0.01")]
	float m_fRadius;

	[Attribute(defvalue: "1 0 0 1", desc: "Dot Color")]
	ref Color m_DotColor;

	[Attribute(defvalue: "0.02", desc: "Dot Density", params: "0 inf 0.01")]
	float m_fDotDensity;

	[Attribute(defvalue: "{EC95FBEA75AE409B}Prefabs/Markers/MapMarkerDotCircle.et", desc: "Marker Prefab", uiwidget: UIWidgets.ResourcePickerThumbnail, params: "et")]
	ResourceName m_sMarkerPrefab;
}

enum SCR_EScenarioFrameworkMarkerCustom
{
	CIRCLE = 0,
	CIRCLE2,
	CROSS,
	CROSS2,
	DOT,
	DOT2,
	DROP_POINT,
	DROP_POINT2,
	ENTRY_POINT,
	ENTRY_POINT2,
	FLAG,
	FLAG2,
	FLAG3,
	FORTIFICATION,
	FORTIFICATION2,
	MARK_EXCLAMATION,
	MARK_EXCLAMATION2,
	MARK_EXCLAMATION3,
	MARK_QUESTION,
	MARK_QUESTION2,
	MARK_QUESTION3,
	MINE_FIELD,
	MINE_FIELD2,
	MINE_FIELD3,
	MINE_SINGLE,
	MINE_SINGLE2,
	MINE_SINGLE3,
	OBJECTIVE_MARKER,
	OBJECTIVE_MARKER2,
	OBSERVATION_POST,
	OBSERVATION_POST2,
	PICK_UP,
	PICK_UP2,
	POINT_OF_INTEREST,
	POINT_OF_INTEREST2,
	POINT_OF_INTEREST3,
	POINT_SPECIAL,
	POINT_SPECIAL2,
	RECON_OUTPOST,
	RECON_OUTPOST2,
	WAYPOINT,
	WAYPOINT2,
	DEFEND,
	DEFEND2,
	DESTROY,
	DESTROY2,
	HEAL,
	HELP,
	HELP2,
	ATTACK,
	ATTACK_MAIN,
	CONTAIN,
	CONTAIN2,
	CONTAIN3,
	RETAIN,
	RETAIN2,
	STRONG_POINT,
	STRONG_POINT2,
	TARGET_REFERENCE_POINT,
	TARGET_REFERENCE_POINT2,
	AMBUSH,
	AMBUSH2,
	RECONNAISSANCE,
	SEARCH_AREA,
	DIRECTION_OF_ATTACK,
	DIRECTION_OF_ATTACK_MAIN,
	DIRECTION_OF_ATTACK_PLANNED,
	FOLLOW_AND_SUPPORT,
	FOLLOW_AND_SUPPORT2,
	JOIN,
	JOIN2,
	JOIN3,
	ARROW_LARGE,
	ARROW_LARGE2,
	ARROW_LARGE3,
	ARROW_MEDIUM,
	ARROW_MEDIUM2,
	ARROW_MEDIUM3,
	ARROW_SMALL,
	ARROW_SMALL2,
	ARROW_SMALL3,
	ARROW_CURVE_LARGE,
	ARROW_CURVE_LARGE2,
	ARROW_CURVE_LARGE3,
	ARROW_CURVE_MEDIUM,
	ARROW_CURVE_MEDIUM2,
	ARROW_CURVE_MEDIUM3,
	ARROW_CURVE_SMALL,
	ARROW_CURVE_SMALL2,
	ARROW_CURVE_SMALL3
}

enum SCR_EScenarioFrameworkMarkerCustomColor
{
	WHITE = 0,
	REFORGER_ORANGE,
	ORANGE,
	RED,
	OPFOR,
	INDEPENDENT,
	GREEN,
	BLUE,
	BLUFOR,
	DARK_BLUE,
	MAGENTA,
	CIVILIAN,
	DARK_PINK
}
