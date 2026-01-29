[EntityEditorProps(category: "GameScripted/ScenarioFramework/SlotWaypoint", description: "")]
class SCR_ScenarioFrameworkSlotWaypointClass : SCR_ScenarioFrameworkSlotBaseClass
{
	// prefab properties here
}

//------------------------------------------------------------------------------------------------
class SCR_ScenarioFrameworkSlotWaypoint : SCR_ScenarioFrameworkSlotBase
{
	[Attribute(desc: "Waypoint settings", category: "Waypoint")]
	ref SCR_ScenarioFrameworkWaypoint m_Waypoint;

	//------------------------------------------------------------------------------------------------
	//! Spawns random or specified asset at specified position or waypoint, optionally ignores orientation to terrain, and requests nav
	//! \return the spawned entity.
	override IEntity SpawnAsset()
	{
		//If Randomization is enabled, it will try to apply settings from Attributes.
		//If it fails anywhere, original m_sObjectToSpawn will be used.
		if (m_bRandomizePerFaction)
		{
			ResourceName randomAsset;
			if (SCR_StringHelper.IsEmptyOrWhiteSpace(GetRandomlySpawnedObject()))
			{
				GetRandomAsset(randomAsset);
				if (!SCR_StringHelper.IsEmptyOrWhiteSpace(randomAsset))
					m_sObjectToSpawn = randomAsset;
			}
			else
			{
				randomAsset = GetRandomlySpawnedObject();
			}
		}

		if (m_Waypoint)
			m_sObjectToSpawn = m_Waypoint.GetWaypointPrefab();

		Resource resource = Resource.Load(m_sObjectToSpawn);
		if (!resource || !resource.IsValid())
			return null;

		GetOwner().GetWorldTransform(m_SpawnParams.Transform);
		m_SpawnParams.TransformMode = ETransformMode.WORLD;
		//--- Apply rotation
		vector angles = Math3D.MatrixToAngles(m_SpawnParams.Transform);
		Math3D.AnglesToMatrix(angles, m_SpawnParams.Transform);

		//--- Spawn the prefab
		BaseResourceObject resourceObject = resource.GetResource();
		if (!resourceObject)
			return null;

		if (m_bIgnoreOrientChildrenToTerrain)
		{
			IEntityComponentSource slotCompositionComponent = SCR_BaseContainerTools.FindComponentSource(resourceObject, SCR_SlotCompositionComponent);
			if (slotCompositionComponent)
			{
				SCR_SlotCompositionComponent.IgnoreOrientChildrenToTerrain();
			}
		}

		IEntity entity = GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), m_SpawnParams);
		SCR_AIWorld aiWorld = SCR_AIWorld.Cast(GetGame().GetAIWorld());
		if (aiWorld)
			aiWorld.RequestNavmeshRebuildEntity(entity);

		m_aSpawnedEntities.Insert(entity);

		if (m_vPosition != vector.Zero)
			entity.SetOrigin(m_vPosition);

		if (m_Waypoint)
		{
			m_Waypoint.m_SlotWaypoint = this;
			m_Waypoint.SetupWaypoint(entity);
		}

		return entity;
	}

	//------------------------------------------------------------------------------------------------
	//! Draws debug shapes for debugging purposes, creating cylinder shapes for center and outer radius based on specified parameters.
	//! \param[in] draw Draws debug shapes representing debug shape center and debug shape outer based on debug shape radius, color, and owner's origin
	override protected void DrawDebugShape(bool draw)
	{
		if (!draw)
			return;
		
		Shape dbgShapeCenter = null;
		Shape dbgShapeOuter = null;

		float radius = m_fDebugShapeRadius;
		if (m_Waypoint)
			radius = m_Waypoint.GetWaypointCompletionRadius();


		dbgShapeCenter = Shape.CreateCylinder(
										m_iDebugShapeColor,
										ShapeFlags.TRANSP | ShapeFlags.DOUBLESIDE | ShapeFlags.NOZWRITE | ShapeFlags.ONCE | ShapeFlags.NOOUTLINE,
										GetOwner().GetOrigin(),
										0.25,
										10
								);

		dbgShapeOuter = Shape.CreateCylinder(
										m_iDebugShapeColor,
										ShapeFlags.TRANSP | ShapeFlags.DOUBLESIDE | ShapeFlags.NOZWRITE | ShapeFlags.ONCE | ShapeFlags.NOOUTLINE,
										GetOwner().GetOrigin(),
										radius,
										2
								);
	}
	
#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	//! Manages key changes for debug shapes, object spawning, and deletion in Workbench.
	//! \param[in] owner The owner represents the entity triggering the key change event in the method.
	//! \param[in] src represents the source container for key changes in the method.
	//! \param[in] key that corresponds with the workbench attribute
	override bool _WB_OnKeyChanged(IEntity owner, BaseContainer src, string key, BaseContainerList ownerContainers, IEntity parent)
	{
		if (key == "m_bShowDebugShapesInWorkbench")
			DrawDebugShape(m_bShowDebugShapesInWorkbench);

		if (key == "coords")
		{
			SCR_EntityHelper.DeleteEntityAndChildren(m_PreviewEntity);

			Resource resource = Resource.Load(m_sObjectToSpawn);
			if (!resource)
				return false;

			SpawnEntityPreview(owner, resource);
			return true;
		}
		else if (key == "m_sObjectToSpawn")
		{
			SCR_EntityHelper.DeleteEntityAndChildren(m_PreviewEntity);
			return false;
		}
		return false;
	}
#endif
	
	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_ScenarioFrameworkSlotWaypoint(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_iDebugShapeColor = ARGB(100, 0x00, 0x10, 0xFF);
	}
}
