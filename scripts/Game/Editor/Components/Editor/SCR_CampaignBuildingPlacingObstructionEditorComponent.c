[ComponentEditorProps(category: "GameScripted/Editor", description: "Placing obstruction. Disallow placing of the composition in define cases.", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_CampaignBuildingPlacingObstructionEditorComponentClass : SCR_BaseEditorComponentClass
{

}

/** @ingroup Editor_Components
*/

/*!

*/
class SCR_CampaignBuildingPlacingObstructionEditorComponent : SCR_BaseEditorComponent
{
	[Attribute(defvalue: "20", desc: "Entity tilt greater then this value in angles will trigger a warning.")]
	protected float m_fEntityTiltWarning;

	[Attribute(defvalue: "25", desc: "A difference between max and min tilt of two entities in composition to trigger a warning")]
	protected float m_fTiltDifferenceWarning;

	protected ResourceName m_sCompositionResourceName;

	protected bool m_bCanBeCreated = true;
	protected bool m_bSuperiorCanBeCreated = false;
	protected bool m_bTraceEntityPosition;
	protected SCR_CampaignBuildingEditorComponent m_CampaignBuildingComponent;
	protected ScriptedGameTriggerEntity m_AreaTrigger;
	protected ECantBuildNotificationType m_eBlockingReason;
	protected SCR_EditablePreviewEntity m_PreviewEnt;
	protected ref array<ref Tuple3<IEntity, float, vector>> m_aCompositionEntities = {};

	//Adding this value to a sea level as the composition preview, even above sea doesn't have it's Y value exactly a zero.
	protected const float SEA_LEVEL_OFFSET = 0.01;

	// With the preview we don't need to be that strict as it is sometimes even wanted to have an entities tide together for an example row of sandbag walls.
	protected const float PREVIEW_CHECK_FACTOR = 0.1;

	// The heigh above the ground where the trace stops means it will ignore an entities that are close to surface like sidewalks so they don't block the placement.
	protected const float HEIGHT_ABOVE_GROUND_BUFFER = 0.3;
	protected const float HEIGHT_ABOVE_GROUND_VEHICLE_BUFFER = 0.5;

	protected const float BOUNDING_BOX_FACTOR = 0.5;
	protected const float BOUNDING_BOX_VEHICLE_FACTOR = 0.8;
	protected const float MINIMAL_PROTECTION_RADIUS_TO_EVALUATE = 0.2;
	protected const float MAXIMAL_HEIGHT_ABOVE_TERRAIN_TO_EVALUATE = 1;
	protected const float Y_BUFFER = 0.3;
	protected const int MINIMAL_ENTITY_RADIUS_TO_EVALUATE_TILT = 1;
	protected float m_fSafeZoneRadius = 0;
	protected float m_fCylinderHeight = 0.5;
	protected float m_fRollMin;
	protected float m_fRollMax;
	protected float m_fPitchMin;
	protected float m_fPitchMax;
	protected bool m_bIsVehicle;

	protected vector m_vTraceOffset = Vector(0, 10, 0);
	protected vector m_vCylinderVectorOffset = Vector(0, m_fCylinderHeight * 0.5, 0);

	//------------------------------------------------------------------------------------------------
	override protected void EOnEditorActivate()
	{
		InitVariables();
	}

	//------------------------------------------------------------------------------------------------
	override protected void EOnEditorActivateServer()
	{
		InitVariables();
	}

	//------------------------------------------------------------------------------------------------
	override protected void EOnEditorOpen()
	{
		InitVariables();

		SCR_CampaignBuildingEditorComponent CampaignBuildingEditorComponent = SCR_CampaignBuildingEditorComponent.Cast(SCR_CampaignBuildingEditorComponent.GetInstance(SCR_CampaignBuildingEditorComponent));
		if (!CampaignBuildingEditorComponent)
			return;

		CampaignBuildingEditorComponent.GetOnObstructionEventTriggered().Insert(SetSuperiorCanBeCreated);
	}

	//------------------------------------------------------------------------------------------------
	override protected void EOnEditorClose()
	{
		SCR_CampaignBuildingEditorComponent CampaignBuildingEditorComponent = SCR_CampaignBuildingEditorComponent.Cast(SCR_CampaignBuildingEditorComponent.GetInstance(SCR_CampaignBuildingEditorComponent));
		if (!CampaignBuildingEditorComponent)
			return;

		CampaignBuildingEditorComponent.GetOnObstructionEventTriggered().Remove(SetSuperiorCanBeCreated);
	}

	//------------------------------------------------------------------------------------------------
	override protected void EOnEditorOpenServer()
	{
		InitVariables();
	}

	//------------------------------------------------------------------------------------------------
	//! Method called once the preview is spawned to initiate all the variables and start evaluate the obstruction
	void OnPreviewCreated(notnull SCR_EditablePreviewEntity previewEnt)
	{
		SCR_PreviewEntityEditorComponent PreviewEntityComponent = SCR_PreviewEntityEditorComponent.Cast(SCR_PreviewEntityEditorComponent.GetInstance(SCR_PreviewEntityEditorComponent));
		if (PreviewEntityComponent)
			PreviewEntityComponent.SetLastPreviewState(SCR_EPreviewState.NONE);

		m_PreviewEnt = previewEnt;
		SetInitialCanBeCreatedState(previewEnt);
		GetAllEntitiesToEvaluate(previewEnt);

		SCR_CampaignBuildingPlacingEditorComponent placingComponent = SCR_CampaignBuildingPlacingEditorComponent.Cast(FindEditorComponent(SCR_CampaignBuildingPlacingEditorComponent, true, true));
		if (!placingComponent)
			return;

		m_sCompositionResourceName = placingComponent.GetSelectedPrefab();
	}

	//------------------------------------------------------------------------------------------------
	//! Return current canbeCreated value.
	bool CanBeCreated()
	{
		return m_bCanBeCreated;
	}

	//------------------------------------------------------------------------------------------------
	//! Set if the entity can be created, outside of the obstruction system. This is superior to internal m_bCanBeCreated
	void SetSuperiorCanBeCreated(bool val)
	{
		m_bSuperiorCanBeCreated = val;
	}

	//------------------------------------------------------------------------------------------------
	//! Set the initial state of the preview can / can't be created (based on the place where player place the preview into the world.)
	protected void SetInitialCanBeCreatedState(notnull SCR_EditablePreviewEntity previewEnt)
	{
		if (!previewEnt)
			return;

		float distance = vector.DistanceSq(m_AreaTrigger.GetOrigin(), previewEnt.GetOrigin());
		if (distance < m_AreaTrigger.GetSphereRadius() * m_AreaTrigger.GetSphereRadius())
			m_bCanBeCreated = true;

		m_bCanBeCreated = false;
		m_eBlockingReason = ECantBuildNotificationType.OUT_OF_AREA;
		return;
	}

	//------------------------------------------------------------------------------------------------
	private void InitVariables()
	{
		m_CampaignBuildingComponent = SCR_CampaignBuildingEditorComponent.Cast(FindEditorComponent(SCR_CampaignBuildingEditorComponent, true, true));
		if (!m_CampaignBuildingComponent)
			return;

		m_AreaTrigger = m_CampaignBuildingComponent.GetTrigger();
	}

	//------------------------------------------------------------------------------------------------
	// Check if the preview is outisde of the building radius. if preview doesn't exist return false - preview doesn't exist on server and the same CanCreateEntity is used on server too.
	bool IsPreviewOutOfRange(SCR_EditorPreviewParams instantPlacingParam, out ENotification outNotification = -1)
	{
		// In case of WP placed with controller, the preview of the WP doesn't exist and we need to use placing params.
		if (instantPlacingParam)
		{
			vector outTransform[4];
			instantPlacingParam.GetWorldTransform(outTransform);

			if ((vector.DistanceSqXZ(m_AreaTrigger.GetOrigin(), outTransform[3]) >= m_AreaTrigger.GetSphereRadius() * m_AreaTrigger.GetSphereRadius()))
			{
				outNotification = ENotification.EDITOR_PLACING_OUT_OF_CAMPAIGN_BUILDING_ZONE;
				return true;
			}
		}

		if (!m_AreaTrigger || !m_PreviewEnt)
			return false;

		if ((vector.DistanceSqXZ(m_AreaTrigger.GetOrigin(), m_PreviewEnt.GetOrigin()) >= m_AreaTrigger.GetSphereRadius() * m_AreaTrigger.GetSphereRadius()))
		{
			outNotification = ENotification.EDITOR_PLACING_OUT_OF_CAMPAIGN_BUILDING_ZONE;
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	// Make an array of all entities to evaluate
	protected void GetAllEntitiesToEvaluate(notnull SCR_EditablePreviewEntity rootEnt)
	{
		m_aCompositionEntities.Clear();
		array<IEntity> compositionEntities = {};
		vector entityOrigin[4];

		SCR_EntityHelper.GetHierarchyEntityList(rootEnt, compositionEntities);
		float protectionRadius;
		
		World world = GetGame().GetWorld();
		if (!world)
			return;

		foreach (IEntity ent : compositionEntities)
		{
			if (IsPreviewVehicle(ent))
				m_bIsVehicle = true;

			// Center of the entity is burrided under the ground (fondations) check if it has set additinal obstruction radius to evaluta (heliport)
			vector entityCenter;
			entityCenter = SCR_EntityHelper.GetEntityCenterWorld(ent);

			float surfaceY = world.GetSurfaceY(entityCenter[0], entityCenter[2]);

			// When a preview is created from prefab, it's created at 0,0,0 so y < 0 test is enough. When it's created from placed composition, we have to check a real world y at given place
			if ((entityCenter[1] < 0) || entityCenter[1] < (surfaceY - Y_BUFFER))
			{
				SCR_CampaignBuildingPlacingEditorComponent placingComponent = SCR_CampaignBuildingPlacingEditorComponent.Cast(FindEditorComponent(SCR_CampaignBuildingPlacingEditorComponent, true, true));
				if (!placingComponent)
					continue;

				ResourceName resName = placingComponent.GetSelectedPrefab();
				if (resName.IsEmpty())
				{
					SCR_CampaignBuildingTransformingEditorComponent transformingComponent = SCR_CampaignBuildingTransformingEditorComponent.Cast(FindEditorComponent(SCR_CampaignBuildingTransformingEditorComponent, true, true));
					if (!transformingComponent)
						continue;

					SCR_EditableEntityComponent editablePivot = transformingComponent.GetEditedPivot();
					if (!editablePivot)
						continue;

					IEntity pivotEnt = editablePivot.GetOwner();
					if (!pivotEnt)
						continue;

					resName = pivotEnt.GetPrefabData().GetPrefabName();
				}

				if (resName.IsEmpty())
					continue;

				protectionRadius = GetPredefineProtectionRadius(resName);
			}
			else
			{
				protectionRadius = GetEntityProtectionRadius(ent);
			}

			if (protectionRadius > 0)
			{

				ent.GetLocalTransform(entityOrigin);
				// If the entity is too small or in the air (props on tables etc) ignore it in test
				if (protectionRadius < MINIMAL_PROTECTION_RADIUS_TO_EVALUATE || entityOrigin[3][1] > MAXIMAL_HEIGHT_ABOVE_TERRAIN_TO_EVALUATE)
					continue;

				vector entityLocalTransform[4];
				ent.GetLocalTransform(entityLocalTransform);
				vector transformAngles = Math3D.MatrixToAngles(entityLocalTransform);

				m_aCompositionEntities.Insert(new Tuple3<IEntity, float, vector>(ent, protectionRadius, transformAngles));
			}
		}
	}


	//------------------------------------------------------------------------------------------------
	float GetPredefineProtectionRadius(ResourceName resName)
	{
		Resource entityPrefab = Resource.Load(resName);
		if (!entityPrefab.IsValid())
			return 0;

		IEntitySource entitySource = SCR_BaseContainerTools.FindEntitySource(entityPrefab);
		if (!entitySource)
			return 0;

		IEntityComponentSource entityComponentSource;
		entityComponentSource = SCR_CampaignBuildingCompositionComponentClass.GetCampaignBuildingCompositionSource(entitySource);
		if (!entityComponentSource)
			return 0;

		return SCR_CampaignBuildingCompositionComponentClass.GetProtectionRadius(entityComponentSource);
	}

	//------------------------------------------------------------------------------------------------
	//! Check the given preview is vehicle
	bool IsPreviewVehicle(IEntity ent)
	{
		SCR_EditablePreviewComponent previewEditableEntity = SCR_EditablePreviewComponent.Cast(ent.FindComponent(SCR_EditablePreviewComponent));
		if (!previewEditableEntity)
			return false;

		SCR_EditableEntityUIInfo editableUiInfo = SCR_EditableEntityUIInfo.Cast(previewEditableEntity.GetInfo());

		return editableUiInfo && editableUiInfo.HasEntityLabel(EEditableEntityType.VEHICLE) && !editableUiInfo.HasEntityLabel(EEditableEntityLabel.VEHICLE_HELICOPTER);
	}

	//------------------------------------------------------------------------------------------------
	//! Check the preview position. Is suitable to build the composition here?
	bool CanCreate(out ENotification outNotification = -1, out SCR_EPreviewState previewStateToShow = SCR_EPreviewState.PLACEABLE)
	{
		// superior can be created was set to false which means some external condition block placing the composition. No need to continue with any evaluation.
		if (m_bSuperiorCanBeCreated)
		{
			outNotification = ENotification.EDITOR_PLACING_BLOCKED;
			previewStateToShow = SCR_EPreviewState.BLOCKED;
			return false;
		}
			
		m_bCanBeCreated = true;
		m_fRollMin = 0;
		m_fRollMax = 0;
		m_fPitchMin = 0;
		m_fPitchMax = 0;

		if (m_bIsVehicle)
		{
			if (!CheckVehiclePosition())
			{
				m_bCanBeCreated = false;
				outNotification = ENotification.EDITOR_PLACING_BLOCKED;
				previewStateToShow = SCR_EPreviewState.BLOCKED;
			}

			return m_bCanBeCreated;
		}

		foreach (Tuple3<IEntity, float, vector> compositionEntity : m_aCompositionEntities)
		{
			if (compositionEntity.param1 && !CheckEntityPosition(compositionEntity.param1.GetOrigin(), compositionEntity.param2))
			{
				m_bCanBeCreated = false;
				outNotification = ENotification.EDITOR_PLACING_BLOCKED;
				previewStateToShow = SCR_EPreviewState.BLOCKED;
				break;
			}

			if (compositionEntity.param2 < MINIMAL_ENTITY_RADIUS_TO_EVALUATE_TILT)
				continue;

			// Evaluation here needs to continue as the check can later run to an entity that will block placement completely.
			IsCompositionTilted(compositionEntity.param1, compositionEntity.param2, compositionEntity.param3, previewStateToShow);
		}

		if (m_bCanBeCreated && (Math.AbsFloat(m_fRollMax - m_fRollMin) > m_fTiltDifferenceWarning || Math.AbsFloat(m_fPitchMax - m_fPitchMin) > m_fTiltDifferenceWarning))
			previewStateToShow = SCR_EPreviewState.WARNING;

		return m_bCanBeCreated;
	}

	//------------------------------------------------------------------------------------------------
	//! Calculate a sphere radius about the entity which will be tested for obstruction
	protected float GetEntityProtectionRadius(notnull IEntity ent)
	{
		vector vectorMin, vectorMax;
		ent.GetBounds(vectorMin, vectorMax);
		return vector.DistanceXZ(vectorMin, vectorMax) * BOUNDING_BOX_FACTOR;
	}

	//------------------------------------------------------------------------------------------------
	//! Check the tilt of the entities in compositions. If it goes over the set limits, the colour of preview material will be changed to warn player.
	protected bool IsCompositionTilted(IEntity ent, float entityRadius, vector originalTransformAngles, out SCR_EPreviewState previewStateToShow = SCR_EPreviewState.PLACEABLE)
	{
		vector transformMat[4];
		ent.GetWorldTransform(transformMat);

		vector transformAngles = Math3D.MatrixToAngles(transformMat);
		vector transformAnglesToTest = originalTransformAngles - transformAngles;

		if (transformAnglesToTest[1] < -m_fEntityTiltWarning || transformAnglesToTest[1] > m_fEntityTiltWarning)
		{
			previewStateToShow = SCR_EPreviewState.WARNING;
			return true;
		}

		if (transformAnglesToTest[2] < -m_fEntityTiltWarning || transformAnglesToTest[2] > m_fEntityTiltWarning)
		{
			previewStateToShow = SCR_EPreviewState.WARNING;
			return true;
		}

		// The tilt of the entity itself is withing the range. But save it's value to compare min and max of whole composition later to see if the maximum difference isn't triggered.
		if (transformAnglesToTest[1] < m_fRollMin)
			m_fRollMin = transformAnglesToTest[1];

		if (transformAnglesToTest[1] > m_fRollMax)
			m_fRollMax = transformAnglesToTest[1];

		if (transformAnglesToTest[2] < m_fPitchMin)
			m_fPitchMin = transformAnglesToTest[2];

		if (transformAnglesToTest[2] > m_fPitchMax)
			m_fPitchMax = transformAnglesToTest[2];

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Check entity position in given radius.
	protected bool CheckEntityPosition(vector pos, float safeZoneRadius)
	{
		m_bTraceEntityPosition = true;

		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return false;

		// Check if the entity to be placed is in the building range (WP on gamepad don't have a preview so previous filtration doesn't apply to them).
		if ((vector.DistanceSqXZ(m_AreaTrigger.GetOrigin(), pos) >= m_AreaTrigger.GetSphereRadius() * m_AreaTrigger.GetSphereRadius()))
			return false;

		m_fSafeZoneRadius = safeZoneRadius;

		// First do the sea level check as it is cheap and don't need to continue with trace if the composition is in the sea.
		float val = world.GetOceanBaseHeight();
		if (pos[1] < world.GetOceanBaseHeight() + SEA_LEVEL_OFFSET)
			return false;

		// No check for the nearby previews as we don't want to allow player to build a cliping compositions
		if (!world.QueryEntitiesBySphere(pos, safeZoneRadius * PREVIEW_CHECK_FACTOR, EvaluateBlockingEntity))
			return false;

		// Check for solid obstacles such as houses, vehciles, other palyers etc.
		if (m_bTraceEntityPosition && TraceEntityOnPosition(pos, world, safeZoneRadius))
			return false;

		// Check if the placing isn't blocked because the origin of the preview is in water.
		return !ChimeraWorldUtils.TryGetWaterSurfaceSimple(world, pos);
	}

	//------------------------------------------------------------------------------------------------
	//! Check entity position in given bounding box.
	protected bool CheckVehiclePosition()
	{
		if (!m_PreviewEnt)
			return false;

		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return false;

		vector transform[4];
		m_PreviewEnt.GetTransform(transform);

		vector outBoundMin, outBoundMax;
		m_PreviewEnt.GetPreviewBounds(outBoundMin, outBoundMax);
		outBoundMin[1] = HEIGHT_ABOVE_GROUND_VEHICLE_BUFFER;
		outBoundMin[0] = outBoundMin[0] * BOUNDING_BOX_VEHICLE_FACTOR;
		outBoundMin[2] = outBoundMin[2] * BOUNDING_BOX_VEHICLE_FACTOR;
				
		TraceOBB paramOBB = new TraceOBB();
		Math3D.MatrixIdentity3(paramOBB.Mat);
		paramOBB.Mat[0] = transform[0];
		paramOBB.Mat[1] = transform[1];
		paramOBB.Mat[2] = transform[2];
		paramOBB.Start = transform[3] + 0.05 * paramOBB.Mat[1];
		paramOBB.Flags = TraceFlags.ENTS | TraceFlags.OCEAN | TraceFlags.WORLD;
		paramOBB.Mins = outBoundMin;
		paramOBB.Maxs = outBoundMax;
		
		// First do the sea level check as it is cheap and don't need to continue with trace if the composition is in the sea.
		float val = world.GetOceanBaseHeight();
		if (m_PreviewEnt.GetOrigin()[1] < world.GetOceanBaseHeight() + SEA_LEVEL_OFFSET)
			return false;
		
		// No check for the nearby previews as we don't want to allow player to build a cliping compositions
		if (!world.QueryEntitiesByOBB(outBoundMin, outBoundMax, transform, EvaluateBlockingEntity))
			return false;
		
		// Check if the placing isn't blocked because the origin of the preview is in water.
		if (ChimeraWorldUtils.TryGetWaterSurfaceSimple(world, m_PreviewEnt.GetOrigin()))
			return false;			
		
		return world.TracePosition(paramOBB, EvaluateBlockingEntity) > 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Evaluate the entity query found if it really can blocks the spawning of the composition.
	//! \param[in] ent Entity found during query as potential obstacel to placing.
	bool EvaluateBlockingEntity(IEntity ent)
	{
		if (!ent)
			return true;

		if (HasObstructionException(ent))
			return true;

		IEntity rootEnt = SCR_EntityHelper.GetMainParent(ent);
		if (!rootEnt)
			return true;

		SCR_BasePreviewEntity previewEnt = SCR_BasePreviewEntity.Cast(ent);
		if (!previewEnt)
			return true;

		SCR_EditablePreviewEntity ePrevEnt = SCR_EditablePreviewEntity.Cast(rootEnt);

		return ePrevEnt && m_PreviewEnt == ePrevEnt;
	}

	//------------------------------------------------------------------------------------------------
	//! Check if detected entity has set any obstruction exception rulles.
	bool HasObstructionException(notnull IEntity ent)
	{
		SCR_CampaignBuildingObstructionExceptionComponent obstructionException = SCR_CampaignBuildingObstructionExceptionComponent.Cast(ent.FindComponent(SCR_CampaignBuildingObstructionExceptionComponent));
		if (!obstructionException)
			return false;

		// the whitelist is empty means any prefab can colide with this entity. If set check if this entity is on the list of those that can colide.
		if (obstructionException.IsWhitelistEmpty() || obstructionException.IsOnWhitelist(m_sCompositionResourceName))
		{
			m_bTraceEntityPosition = false;
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Trace at the position of the preview to find any possibly cliping entities.
	protected bool TraceEntityOnPosition(vector position, notnull BaseWorld world, float safeZoneRadius)
	{
		TraceParam trace = new TraceParam();
		trace.Start = position;
		trace.End = position - m_vTraceOffset;
		trace.Flags = TraceFlags.ENTS | TraceFlags.OCEAN | TraceFlags.WORLD;
		float traceCoef = world.TraceMove(trace, null);
		position[1] = Math.Max(trace.Start[1] - traceCoef * m_vTraceOffset[1] + 0.01, world.GetSurfaceY(position[0], position[2]) + HEIGHT_ABOVE_GROUND_BUFFER);

		if (TraceCylinder(position + m_vCylinderVectorOffset, safeZoneRadius, m_fCylinderHeight, TraceFlags.ENTS, world))
			return false;

		m_eBlockingReason = ECantBuildNotificationType.BLOCKED;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Check for intersections within given cylinder.
	Performs 4 diagonal traces along cylinders circumference.
	\param pos Cylinder center position
	\param radius Cylinder radius
	\param height Cylinder full height
	\param flags Tracing flags
	\param world World which is being traced
	\return False if an intersection was found, true if the cylinder is devoid of obstacles
	*/
	bool TraceCylinder(vector pos, float radius = 0.5, float height = 2, TraceFlags flags = TraceFlags.ENTS | TraceFlags.OCEAN, BaseWorld world = null)
	{
		if (!world)
			world = GetGame().GetWorld();

		float heightHalf = height * 0.5;

		TraceParam trace = new TraceParam();
		trace.Flags = flags;

		vector dir = {radius, heightHalf, 0};
		trace.Start = pos + dir;
		trace.End = pos - dir;
		if (world.TraceMove(trace, null) < 1)
			return false;

		dir = {-radius, heightHalf, 0};
		trace.Start = pos + dir;
		trace.End = pos - dir;
		if (world.TraceMove(trace, null) < 1)
			return false;

		dir = {0, heightHalf, radius};
		trace.Start = pos + dir;
		trace.End = pos - dir;
		if (world.TraceMove(trace, null) < 1)
			return false;

		dir = {0, heightHalf, -radius};
		trace.Start = pos + dir;
		trace.End = pos - dir;
		if (world.TraceMove(trace, null) < 1)
			return false;

		return true;
	}
}
