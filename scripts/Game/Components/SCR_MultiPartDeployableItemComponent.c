[ComponentEditorProps(category: "GameScripted/Misc", description: "")]
class SCR_MultiPartDeployableItemComponentClass : SCR_BaseDeployableInventoryItemComponentClass
{
	[Attribute(defvalue: "7", desc: "Radius (meters) in which game will look for items", params: "0 inf", category: "Validation")]
	protected int m_iSearchRadius;

	[Attribute(defvalue: "15", desc: "Height in meters that will be used to check if deployment position is under something.\nNot used if deployment user action is not selected to prevent indoors placement", params: "0 inf 0.01", category: "Validation")]
	protected float m_fIndoorsCheckHeight;

	[Attribute(defvalue: "1", desc: "If this entity should be deleted when it is deployed", category: "Setup")]
	protected bool m_bDeleteThisPartOnDeployment;

	[Attribute(desc: "If only currently loaded magazines should be dropped when this entity is deleted", category: "Setup")]
	protected bool m_bRemoveLoadedMagazineWhenDeleted;

	[Attribute(desc: "If all stored items should be dropped when this entity is deleted", category: "Setup")]
	protected bool m_bRemoveAllItemsWhenDeleted;

	[Attribute(desc: "Variants", category: "Setup")]
	protected ref array<ref SCR_DeployableVariantContainer> m_aVariants;

	[Attribute(defvalue: SCR_ESurfaceMonitoringBehaviour.DONT_MONITOR.ToString(), desc: "How this entity should react if the object on which it was deployed was deleted/destroyed/dismantled.", uiwidget: UIWidgets.ComboBox, enumType: SCR_ESurfaceMonitoringBehaviour)]
	protected SCR_ESurfaceMonitoringBehaviour m_eSurfaceObservationBehaviour;

	[Attribute(desc: "If forward vector of spawned entity should face the player (by default it will face away from player)", category: "Setup")]
	protected bool m_bFrontTowardPlayer;

	protected IEntity m_PreviewEntity;
	protected int m_iPreviewVariant = -1;
	protected SCR_EPreviewState m_ePreviewState;

	protected const LocalizedString REASON_NO_SPACE = "#AR-UserAction_Assemble_NoSpace";
	protected const LocalizedString REASON_NO_SPACE_TERRAIN = "#AR-UserAction_Assemble_NoSpaceTerrain";
	protected const LocalizedString REASON_OBSTRUCTED_BY = "#AR-UserAction_Assemble_ObstructedBy";

	//------------------------------------------------------------------------------------------------
	//! \return pointer to the array with all variants
	array<ref SCR_DeployableVariantContainer> GetVariants()
	{
		return m_aVariants;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] variantId of requested variant
	//! \return pointer to the variant container that is shared by all instances of this prefab or null if such variant is not found
	SCR_DeployableVariantContainer GetVariantContainer(int variantId)
	{
		if (!m_aVariants || m_aVariants.IsEmpty())
			return null;

		foreach (SCR_DeployableVariantContainer variant : m_aVariants)
		{
			if (variant.GetVariantId() == variantId)
				return variant;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_ESurfaceMonitoringBehaviour GetMonitoringBehaviour()
	{
		return m_eSurfaceObservationBehaviour;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetSearchRadius()
	{
		return m_iSearchRadius;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetIndoorsCheckHeight()
	{
		return m_fIndoorsCheckHeight;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float IsDeletedWhenDeployed()
	{
		return m_bDeleteThisPartOnDeployment;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool MustDropLoadedMagazines()
	{
		return m_bRemoveLoadedMagazineWhenDeleted;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool MustDropAllStoredItems()
	{
		return m_bRemoveAllItemsWhenDeleted;
	}

	//------------------------------------------------------------------------------------------------
	bool IsFrontFacingPlayer()
	{
		return m_bFrontTowardPlayer;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] ent
	void SetPreviewEntity(notnull IEntity ent, int variantId)
	{
		if (variantId == m_iPreviewVariant && ent && m_PreviewEntity)
			return;

		SCR_EntityHelper.DeleteEntityAndChildren(m_PreviewEntity);
		m_PreviewEntity = ent;
		if (ent)
			m_iPreviewVariant = variantId;
		else
			m_iPreviewVariant = -1;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	IEntity GetPreviewEntity()
	{
		return m_PreviewEntity;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetPreviewVariantId()
	{
		return m_iPreviewVariant;
	}

	//------------------------------------------------------------------------------------------------
	//! \return Current preview state
	SCR_EPreviewState GetPreviewState()
	{
		return m_ePreviewState;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	void SetPreviewState(SCR_EPreviewState newState)
	{
		m_ePreviewState = newState;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] collidingBody which was detected during space validation
	//! \return localized string with a reason that coresponds to the provided object
	LocalizedString GetSpaceValidationFailureReason(notnull IEntity collidingBody)
	{
		if (GenericTerrainEntity.Cast(collidingBody))
			return REASON_NO_SPACE_TERRAIN;

		IEntity topMostParent = collidingBody.GetRootParent();
		InventoryItemComponent iic = InventoryItemComponent.Cast(collidingBody.FindComponent(InventoryItemComponent));
		if (iic)
		{
			UIInfo uiInfo = iic.GetUIInfo();
			if (uiInfo && uiInfo.GetName().IsEmpty())
				uiInfo = null;

			if (uiInfo)
				return WidgetManager.Translate(REASON_OBSTRUCTED_BY, uiInfo.GetName());
		}

		SCR_EditableEntityComponent editableEntityComp = SCR_EditableEntityComponent.Cast(collidingBody.FindComponent(SCR_EditableEntityComponent));
		if (!editableEntityComp)
			return REASON_NO_SPACE;

		SCR_UIInfo scriptedUiInfo = editableEntityComp.GetInfo(collidingBody);
		if (!scriptedUiInfo || scriptedUiInfo.GetName().IsEmpty())
			return REASON_NO_SPACE;

		return WidgetManager.Translate(REASON_OBSTRUCTED_BY, scriptedUiInfo.GetName());
	}

	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_MultiPartDeployableItemComponentClass()
	{
		//This is only called when all instances are destroyed
		SCR_EntityHelper.DeleteEntityAndChildren(m_PreviewEntity);
	}
}

class SCR_MultiPartDeployableItemComponent : SCR_BaseDeployableInventoryItemComponent
{
	protected ref SCR_DeployableVariantContainer m_VariantContainer;
	protected ref array<ref SCR_RequiredDeployablePart> m_aFoundElements;
	protected float m_fReplacementPrefabHealthScaled = 1;
	protected int m_iCurrentVariant = -1;

	protected static const LocalizedString SEARCH_FAIL_REASON_MISSING_PART = "#AR-UserAction_Assemble_MissingPart";

	protected static const ResourceName CAN_BUILD_MATERIAL = "{56EBF5038622AC95}Assets/Conflict/CanBuild.emat";
	protected static const ResourceName CANNOT_BUILD_MATERIAL = "{14A9DCEA57D1C381}Assets/Conflict/CannotBuild.emat";
	protected static const ResourceName TRANSPARENT_MATERIAL = "{8FBC3A6E946F056E}Common/Materials/Default_Transparent.emat";

	protected static const float MINOR_SAFETY_OFFSET = 0.05;

#ifdef ENABLE_DIAG
	//! debug shape for for indicating what area is checked by CheckAvailableSpace()
	protected ref SCR_DebugShapeManager m_DebugShapeMgr = new SCR_DebugShapeManager();
#endif
#ifdef WORKBENCH
	protected static bool s_bShowBoundingBoxDebug;

	//------------------------------------------------------------------------------------------------
	override array<ref WB_UIMenuItem> _WB_GetContextMenuItems(IEntity owner)
	{
		array<ref WB_UIMenuItem> items = { new WB_UIMenuItem("Toggle replacement prefabs bounding box visibility", 0) };

		return items;
	}

	//------------------------------------------------------------------------------------------------
	override void _WB_OnContextMenu(IEntity owner, int id)
	{
		switch (id)
		{
			case 0:
				s_bShowBoundingBoxDebug = !s_bShowBoundingBoxDebug;
				GenericWorldEntity world = GetGame().GetWorldEntity();
				if (!world)
					return;

				IEntity child = world.GetChildren();
				SCR_MultiPartDeployableItemComponent comp;
				while (child)
				{
					comp = SCR_MultiPartDeployableItemComponent.Cast(child.FindComponent(SCR_MultiPartDeployableItemComponent));
					if (comp)
						DrawBoundingBoxDebug(child);

					child = child.GetSibling();
				}

				break;

			default:
				break;
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] owner
	void DrawBoundingBoxDebug(notnull IEntity owner)
	{
		if (GetGame().InPlayMode())
			return;

		m_DebugShapeMgr.Clear();
		if (!s_bShowBoundingBoxDebug)
			return;

		SCR_MultiPartDeployableItemComponentClass data = SCR_MultiPartDeployableItemComponentClass.Cast(GetComponentData(GetOwner()));
		if (!data)
			return;

		int r, g, b;
		const int a = 1;
		Color color = new Color(r, g, b, a);
		vector mat[4];
		array<ref SCR_DeployableVariantContainer> variants = data.GetVariants();
		foreach (int i, SCR_DeployableVariantContainer variant : variants)
		{
			m_VariantContainer = variant;

			vector mins, maxs;
			m_VariantContainer.GetReplacementPrefabBoundingBox(mins, maxs);
			if (mins == vector.Zero && maxs == vector.Zero)
				continue;

			owner.GetTransform(mat);
			if (!m_VariantContainer.IsUsingPartPositionAndRotation())
				ComputeTransform(mat, owner.GetYawPitchRoll());

			mat[3] = mat[3] + MINOR_SAFETY_OFFSET * mat[1];
			ShapeFlags shapeFlags = ShapeFlags.ONCE | ShapeFlags.WIREFRAME;
			Shape shape = Shape.Create(ShapeType.BBOX, color.PackToInt(), shapeFlags, mins, maxs);
			if (shape)
			{
				shape.SetMatrix(mat);
				m_DebugShapeMgr.Add(shape);
			}

			switch (Math.Mod(i, 3))
			{
				case 0:
					r = Math.Min(r + 51, 255);
					break;

				case 1:
					g = Math.Min(g + 51, 255);
					break;

				case 2:
					b = Math.Min(b + 51, 255);
					break;
			}

			color = new Color(r, g, b, a);
		}

		m_VariantContainer = null;
	}

	//------------------------------------------------------------------------------------------------
	override void _WB_AfterWorldUpdate(IEntity owner, float timeSlice)
	{
		DrawBoundingBoxDebug(owner);
	}
#endif

	//------------------------------------------------------------------------------------------------
	//! \return
	bool IsUsingPartPositionAndRotation()
	{
		return m_VariantContainer && m_VariantContainer.IsUsingPartPositionAndRotation();
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	vector GetAdditionaPlacementOffset()
	{
		if (m_VariantContainer && !m_VariantContainer.IsUsingPartPositionAndRotation())
			return m_VariantContainer.GetAdditionaPlacementOffset();

		return vector.Zero;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	vector GetAdditionaPlacementRotation()
	{
		if (m_VariantContainer && !m_VariantContainer.IsUsingPartPositionAndRotation())
			return m_VariantContainer.GetAdditionaPlacementRotation();

		return vector.Zero;
	}

	//------------------------------------------------------------------------------------------------
	//! \return true if forward vector should face toward the player
	bool GetFrontTowardPlayer()
	{
		SCR_MultiPartDeployableItemComponentClass data = SCR_MultiPartDeployableItemComponentClass.Cast(GetComponentData(GetOwner()));
		if (!data)
			return false;

		return data.IsFrontFacingPlayer();
	}

	//------------------------------------------------------------------------------------------------
	//! \return 0-1 scaled amount of health that replacement prefab will have upon deployment
	float GetReplacementPrefabHealthScaled()
	{
		return m_fReplacementPrefabHealthScaled;
	}

	//------------------------------------------------------------------------------------------------
	//! 0-1 scaled amount of health that replacement prefab will have upon deployment
	void SetReplacmentPrefabHealthScaled(float newHealthValue)
	{
		m_fReplacementPrefabHealthScaled = Math.Clamp(newHealthValue, 0, 1);
	}

	//------------------------------------------------------------------------------------------------
	//! Method used to find and cache the data about the variant
	//! \param[int] variant id that is meant to be found
	//! \return true if such variant was found, otherwise false
	bool FetchVariantData(int variant)
	{
		SCR_MultiPartDeployableItemComponentClass data = SCR_MultiPartDeployableItemComponentClass.Cast(GetComponentData(GetOwner()));
		if (!data)
			return false;

		m_VariantContainer = data.GetVariantContainer(variant);
		m_iCurrentVariant = variant;
		return m_VariantContainer != null;
	}

	//------------------------------------------------------------------------------------------------
	//! Clears the information about currently handled variant
	void ClearCache()
	{
		m_iCurrentVariant = -1;
		m_VariantContainer = null;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetCurrentlyCachedVariantId()
	{
		return m_iCurrentVariant;
	}

	//------------------------------------------------------------------------------------------------
	//! Method used to check ground on the deployment position is not tilted too much for currently used variant
	//! \param[in] up vector of the surface
	//! \param[in] useDataFromOwnerPosition when ture, then value up is overriden by a result of a trace at the position of the owner of this component
	//! \return true if tilt is within spec for current variant
	bool CheckSurfaceTilt(vector up, bool useDataFromOwnerPosition = false)
	{
		if (!m_VariantContainer)
			return false;

		float maxTilt = m_VariantContainer.GetMaxAllowedTilt();
		if (maxTilt < 0)
			return true;

		if (useDataFromOwnerPosition)
		{
			IEntity owner = GetOwner();
			TraceParam param = new TraceParam();
			param.Exclude = owner;
	
			vector transform[4];
			SCR_TerrainHelper.GetTerrainBasis(owner.GetOrigin() + vector.Up * 0.1, transform, owner.GetWorld(), false, param);
			up = transform[1];
		}

		return Math.Acos(vector.Dot(up, vector.Up)) * Math.RAD2DEG < maxTilt;
	}

	//------------------------------------------------------------------------------------------------
	//! Method used to check if there are required partes within the area of deployment
	//! \param[out] failReason string containing the text about missing item with their name
	//! \param[in] position from which script will search for required elements. If position == -vector.One then owner origin will be used
	//! \return true if all required elements were found
	bool FindRequiredElements(out string failReason, vector position = -vector.One)
	{
		if (!m_VariantContainer)
			return false;

		IEntity owner = GetOwner();
		SCR_MultiPartDeployableItemComponentClass data = SCR_MultiPartDeployableItemComponentClass.Cast(GetComponentData(owner));
		if (!data)
			return false;

		if (!m_aFoundElements)
			m_aFoundElements = {};

		m_aFoundElements.Clear();
		if (m_VariantContainer.GetRequiredElements(m_aFoundElements) < 1)
			return true;

		BaseWorld world = owner.GetWorld();
		if (position == -vector.One)
			position = owner.GetOrigin();

		world.QueryEntitiesBySphere(position, data.GetSearchRadius(), EvaluateFoundEntity);
		foreach (SCR_RequiredDeployablePart partReq : m_aFoundElements)
		{
			int difference;
			if (!partReq.IsRequirementMet(difference))
			{
				failReason = WidgetManager.Translate(SEARCH_FAIL_REASON_MISSING_PART, difference.ToString(), partReq.GetMissingPartName());
				return false;
			}
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Callback method used to filter found elements and store all required parts
	//! \param[in] ent entity that was found with the query
	//! \return true if query should continue fetching more entities
	protected bool EvaluateFoundEntity(IEntity ent)
	{
		if (!m_aFoundElements || m_aFoundElements.IsEmpty())
			return false;

		if (!ent)
			return true;

		if (ent == GetOwner())
			return true;

		if (ent.GetParent())
			return true;//skip items that are not on the ground

		if (!ent.GetPrefabData())
			return true;//f.e. particles dont have prefab data

		if (ChimeraCharacter.Cast(ent))
			return true;//character cannot be a required element

		BaseContainer foundPrefab = ent.GetPrefabData().GetPrefab();
		vector origin = GetOwner().GetOrigin();
		float distToEnt = vector.DistanceSq(origin, ent.GetOrigin());
		foreach (SCR_RequiredDeployablePart partReq : m_aFoundElements)
		{
			if (partReq.EvaluateFoundEntity(origin, distToEnt, foundPrefab, ent))
				return true;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Method that is meant to be called by clients to check if there is enough space for deployed entity
	//! \param[inout] transform matrix with position which is going to be checked - marked as inout as operations done on static arrays are done through the pointers thus will impact what is passed into the method
	//! \param[in] direction in which spawned entity should be aligned to
	//! \param[out] reason of what is obstructing the space
	//! \return true if there are no obstacles
	bool CheckAvailableSpace(inout vector transform[4], vector direction = vector.Zero, out LocalizedString reason = string.Empty)
	{
		if (!m_VariantContainer)
			return false;

		IEntity owner = GetOwner();
		SCR_MultiPartDeployableItemComponentClass data = SCR_MultiPartDeployableItemComponentClass.Cast(GetComponentData(owner));
		if (!data)
			return false;

		TraceOBB paramOBB = new TraceOBB();
		if (!m_VariantContainer.GetReplacementPrefabBoundingBox(paramOBB.Mins, paramOBB.Maxs))
			return true;//no volume to check thus skip this requirement

		vector mat[4] = transform;//because attributes that are static arrays are passed as a reference thus we need to copy it
		if (!m_VariantContainer.IsUsingPartPositionAndRotation())
			ComputeTransform(mat, direction);

		array<IEntity> excludeArray = {owner, data.GetPreviewEntity()};
		if (m_aFoundElements)
		{
			foreach (SCR_RequiredDeployablePart partReq : m_aFoundElements)
			{
				if (partReq.GetFoundEntities())
					excludeArray.InsertAll(partReq.GetFoundEntities());
			}
		}

		paramOBB.Mat[0] = mat[0];
		paramOBB.Mat[1] = mat[1];
		paramOBB.Mat[2] = mat[2];
		paramOBB.Start = mat[3] + MINOR_SAFETY_OFFSET * paramOBB.Mat[1];//+5cm to ensure that we wont collide with the ground due to minor unevenness
		paramOBB.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
		paramOBB.ExcludeArray = excludeArray;
		paramOBB.LayerMask = EPhysicsLayerPresets.Projectile;

		owner.GetWorld().TracePosition(paramOBB, FilterTracedEntities);

#ifdef ENABLE_DIAG
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_ITEM_PLACEMENT_VISUALIZATION))
		{
			m_DebugShapeMgr.Clear();
			ShapeFlags shapeFlags = ShapeFlags.ONCE | ShapeFlags.WIREFRAME;

			int color = Color.RED;
			if (!paramOBB.TraceEnt)
				color = Color.GREEN;

			Shape shape = Shape.Create(ShapeType.BBOX, color, shapeFlags, paramOBB.Mins, paramOBB.Maxs);
			if (shape)
			{
				mat[0] = paramOBB.Mat[0];
				mat[1] = paramOBB.Mat[1];
				mat[2] = paramOBB.Mat[2];
				mat[3] = paramOBB.Start;
				shape.SetMatrix(mat);
				m_DebugShapeMgr.Add(shape);
			}
		}
#endif
		if (!paramOBB.TraceEnt)
			return true;

		reason = data.GetSpaceValidationFailureReason(paramOBB.TraceEnt);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Method that is meant to be called by clients to check if spawn position will be under something
	bool CheckIfPositionIsIndoors(inout vector transform[4])
	{
		IEntity owner = GetOwner();
		SCR_MultiPartDeployableItemComponentClass data = SCR_MultiPartDeployableItemComponentClass.Cast(GetComponentData(owner));

		TraceParam traceParam = new TraceParam();
		traceParam.Start = transform[3] + (vector.Up * MINOR_SAFETY_OFFSET);//+5cm to ensure that we wont collide with the ground
		traceParam.End = traceParam.Start + (vector.Up * data.GetIndoorsCheckHeight());
		traceParam.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
		traceParam.LayerMask = EPhysicsLayerPresets.Projectile;
		traceParam.Exclude = owner;
		owner.GetWorld().TraceMove(traceParam, FilterTracedEntities);

		return traceParam.TraceEnt;
	}

	//------------------------------------------------------------------------------------------------
	//! Creates or updates preview entity used by this deployable entity as a visualisation of final product
	//! \param[inout] transform - marked as inout as operations done on static arrays are done through the pointers thus will impact what is passed into the method
	//! \param[in] direction
	void VisualizeReplacementEntity(inout vector transform[4], vector direction = vector.Zero)
	{
		if (!m_VariantContainer)
			return;

		IEntity owner = GetOwner();
		SCR_MultiPartDeployableItemComponentClass data = SCR_MultiPartDeployableItemComponentClass.Cast(GetComponentData(owner));
		if (!data)
			return;

		IEntity previewEnt = data.GetPreviewEntity();
		if (!previewEnt || data.GetPreviewVariantId() != m_iCurrentVariant)
		{
			ResourceName replacementPrefab = m_VariantContainer.GetPreviewResource();
			if (replacementPrefab.IsEmpty())
				return;

			Resource resource = Resource.Load(replacementPrefab);
			if (!resource.IsValid())
				return;

			BaseResourceObject resourceObject = resource.GetResource();
			if (!resourceObject)
				return;

			BaseWorld world = owner.GetWorld();
			VObject vObj = resourceObject.ToVObject();
			if (vObj)
			{
				previewEnt = GetGame().SpawnEntity(GenericEntity, world);
				previewEnt.SetObject(vObj, string.Empty);
			}
			else
			{
				previewEnt = GetGame().SpawnEntityPrefab(resource, world);
			}

			if (!previewEnt)
				return;

			SCR_Global.SetMaterial(previewEnt, CAN_BUILD_MATERIAL);
			data.SetPreviewEntity(previewEnt, m_iCurrentVariant);
			data.SetPreviewState(SCR_EPreviewState.PLACEABLE);
			previewEnt.ClearFlags(EntityFlags.TRACEABLE | EntityFlags.ACTIVE, true);
		}

		vector mat[4] = transform;//because attributes that are static arrays are passed as a reference thus we need to copy it
		if (!m_VariantContainer.IsUsingPartPositionAndRotation())
			ComputeTransform(mat, direction);

		if (!(previewEnt.GetFlags() & EntityFlags.VISIBLE))
			previewEnt.SetFlags(EntityFlags.VISIBLE, true);

		previewEnt.SetTransform(mat);
		previewEnt.Update();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] state
	void SetPreviewState(SCR_EPreviewState state)
	{
		SCR_MultiPartDeployableItemComponentClass data = SCR_MultiPartDeployableItemComponentClass.Cast(GetComponentData(GetOwner()));
		if (!data)
			return;

		IEntity previewEnt = data.GetPreviewEntity();
		if (!previewEnt)
			return;

		if (data.GetPreviewState() == state)
			return;

		switch (state)
		{
			case SCR_EPreviewState.PLACEABLE:
				SCR_Global.SetMaterial(previewEnt, CAN_BUILD_MATERIAL);
				break;

			case SCR_EPreviewState.BLOCKED:
				SCR_Global.SetMaterial(previewEnt, CANNOT_BUILD_MATERIAL);
				break;

			case SCR_EPreviewState.NONE:
				SCR_Global.SetMaterial(previewEnt, TRANSPARENT_MATERIAL);
				break;
		}

		data.SetPreviewState(state);
	}

	//------------------------------------------------------------------------------------------------
	//! Callback method used to filter out things that shouldnt prevent deployment
	//! \param[in] e
	//! \param[in] start
	//! \param[in] dir
	//! \return
	bool FilterTracedEntities(notnull IEntity e, vector start = "0 0 0", vector dir = "0 0 0")
	{
		if (!e)
			return false;

		if (e.GetRootParent() == GetOwner())
			return false;

		if (ChimeraCharacter.Cast(e))
			return false;//ignore characters

		if (e && ChimeraCharacter.Cast(e.GetRootParent()))
			return false;//ignore character equipment

		if (SCR_BaseDebrisSmallEntity.Cast(e))
			return false;//ignore debris

		if (!e.GetPrefabData())
			return false;//ignore f.e. particles or managers

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Gets called when deploy action is executed by player
	//! \param[in] userEntity player who deployed it
	//! \param[in] direction in which it should be deployed
	//! \param[in] reload
	//! \param[in] state should the resulting state be considered as deployed
	void Deploy(int variantId, IEntity userEntity = null, vector direction = vector.Zero, bool reload = false, bool state = true)
	{
		if (!m_RplComponent || m_RplComponent.IsProxy())
			return;

		if (m_bIsDeployed)
			return;

		if (!m_VariantContainer || variantId != m_VariantContainer.GetVariantId())
			return;

		IEntity owner = GetOwner();
		vector position = owner.GetOrigin() + vector.Up * 0.1;
		if (!m_VariantContainer.IsUsingPartPositionAndRotation())
			position += owner.VectorToParent(m_VariantContainer.GetAdditionaPlacementOffset());

		TraceParam param = new TraceParam();
		array<IEntity> excludeArray = {owner, userEntity};
		param.ExcludeArray = excludeArray;
		param.LayerMask = EPhysicsLayerPresets.Projectile;
		SCR_TerrainHelper.GetTerrainBasis(position, m_aOriginalTransform, GetGame().GetWorld(), false, param);

		if (!m_VariantContainer.IsUsingPartPositionAndRotation())
			ComputeTransform(m_aOriginalTransform, direction);

		EntitySpawnParams params = new EntitySpawnParams();
		params.Transform = m_aOriginalTransform;
		params.TransformMode = ETransformMode.WORLD;

		Resource resource = Resource.Load(m_VariantContainer.GetReplacementPrefab());
		if (!resource.IsValid())
			return;

		m_ReplacementEntity = GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), params);
		if (!m_ReplacementEntity)
			return;

		m_PreviousOwner = userEntity;

		PlayerManager playerMgr = GetGame().GetPlayerManager();
		int playerID = playerMgr.GetPlayerIdFromControlledEntity(userEntity);
		SetAuthor(m_ReplacementEntity, playerID);

		if (param.TraceEnt)
		{
			SCR_MultiPartDeployableItemComponent deployableComp = SCR_MultiPartDeployableItemComponent.Cast(m_ReplacementEntity.FindComponent(SCR_MultiPartDeployableItemComponent));
			if (deployableComp)
				deployableComp.SurfaceMonitoringInit(param.TraceEnt);
		}

		array<ref SCR_AdditionalDeployablePart> additionalPartsToSpawn = m_VariantContainer.GetAdditionalPrefabList();
		if (additionalPartsToSpawn)
		{
			vector spawnedMat[4];
			IEntity spawnedEntity;
			InventoryItemComponent spawnedIIC;
			SCR_DeployableInventoryItemInventoryComponent spawnedDIIC;
			foreach (SCR_AdditionalDeployablePart additionalPrefab : additionalPartsToSpawn)
			{
				additionalPrefab.SpawnPrefabs(playerID, userEntity, params);
			}
		}

		m_bIsDeployed = state;
		Replication.BumpMe();

		SCR_BaseDeployableInventoryItemComponentClass data = SCR_BaseDeployableInventoryItemComponentClass.Cast(GetComponentData(GetOwner()));
		if (data && data.IsSoundEnabled() && !reload)
		{
			RPC_PlaySoundOnDeployBroadcast(m_bIsDeployed);
			Rpc(RPC_PlaySoundOnDeployBroadcast, m_bIsDeployed);
		}

		if (m_OnDeployedStateChanged)
			m_OnDeployedStateChanged.Invoke(m_bIsDeployed, this);

		DeleteParts();
		SetPreviewState(SCR_EPreviewState.NONE);
	}

	//------------------------------------------------------------------------------------------------
	//! Method used to inform spawned entities about who is their author
	//! \param[in] ent entity for which author will be set
	//! \param[in] playerID
	static void SetAuthor(notnull IEntity ent, int playerID)
	{
		SCR_EditableEntityComponent editableComp = SCR_EditableEntityComponent.Cast(ent.FindComponent(SCR_EditableEntityComponent));
		if (!editableComp)
			return;

		editableComp.SetAuthor(playerID);
	}

	//------------------------------------------------------------------------------------------------
	//! Method used to initialize monitoring of the surface on which this object was deployed
	//! \param[in] surface that will be monitored
	void SurfaceMonitoringInit(notnull IEntity surface)
	{
		IEntity owner = GetOwner();
		SCR_MultiPartDeployableItemComponentClass data = SCR_MultiPartDeployableItemComponentClass.Cast(GetComponentData(owner));
		if (!data || data.GetMonitoringBehaviour() == SCR_ESurfaceMonitoringBehaviour.DONT_MONITOR)
			return;

		SCR_BaseDeployableInventoryItemComponent deployableComp = SCR_BaseDeployableInventoryItemComponent.Cast(surface.FindComponent(SCR_BaseDeployableInventoryItemComponent));
		if (deployableComp)
			deployableComp.GetOnDeployedStateChanged().Insert(OnMonitoredSurfaceDismantled);

		HitZoneContainerComponent parentDamageManager = HitZoneContainerComponent.Cast(surface.FindComponent(HitZoneContainerComponent));
		if (parentDamageManager)
		{
			SCR_HitZone hitZone = SCR_HitZone.Cast(parentDamageManager.GetDefaultHitZone());
			if (hitZone)
				hitZone.GetOnDamageStateChanged().Insert(OnMonitoredSurfaceDamageStateChanged);
		}

		SCR_EditableEntityComponent editableComponent = SCR_EditableEntityComponent.Cast(surface.FindComponent(SCR_EditableEntityComponent));
		if (editableComponent)
			editableComponent.GetOnDeleted().Insert(OnMonitoredSurfaceDeleted);
	}

	//------------------------------------------------------------------------------------------------
	//! Callback method used to update this entity when surface on which it was deployed changed its damage state
	//! \param[in] ent that was being monitored
	//! \param[in] observedHitZone which damage state was monitored
	//! \param[in] deployableComp which deployment state was monitored
	protected void StopMonitoringSurface(IEntity ent, SCR_HitZone observedHitZone = null, SCR_BaseDeployableInventoryItemComponent deployableComp = null)
	{
		if (!deployableComp)
			deployableComp = SCR_BaseDeployableInventoryItemComponent.Cast(ent.FindComponent(SCR_BaseDeployableInventoryItemComponent));

		if (deployableComp)
			deployableComp.GetOnDeployedStateChanged().Remove(OnMonitoredSurfaceDismantled);

		if (!observedHitZone)
		{
			HitZoneContainerComponent surfaceDamageManager = HitZoneContainerComponent.Cast(ent.FindComponent(HitZoneContainerComponent));
			if (surfaceDamageManager)
				observedHitZone = SCR_HitZone.Cast(surfaceDamageManager.GetDefaultHitZone());
		}

		if (observedHitZone)
			observedHitZone.GetOnDamageStateChanged().Remove(OnMonitoredSurfaceDamageStateChanged);

		SCR_EditableEntityComponent editableComponent = SCR_EditableEntityComponent.Cast(ent.FindComponent(SCR_EditableEntityComponent));
		if (editableComponent)
			editableComponent.GetOnDeleted().Remove(OnMonitoredSurfaceDeleted);
	}

	//------------------------------------------------------------------------------------------------
	//! Method used to execute reactionary behaviour on the changes made to the surface on which this object was deployed
	//! \param[in] beaviour override of the behaviour that was defined for this prefab
	protected void ExecuteObservationBehaviour(SCR_ESurfaceMonitoringBehaviour beaviour = -1)
	{
		IEntity owner = GetOwner();
		SCR_MultiPartDeployableItemComponentClass data = SCR_MultiPartDeployableItemComponentClass.Cast(GetComponentData(owner));
		if (!data)
			return;

		if (beaviour == -1)
			beaviour = data.GetMonitoringBehaviour();

		switch (beaviour)
		{
			case SCR_ESurfaceMonitoringBehaviour.DISMANTLE:
				ClearCache();
				FetchVariantData(0);
				Deploy(0);
				return;

			case SCR_ESurfaceMonitoringBehaviour.DESTROY:
				DamageManagerComponent dmgMgrComp = DamageManagerComponent.Cast(owner.FindComponent(DamageManagerComponent));
				if (!dmgMgrComp)
				{
					ExecuteObservationBehaviour(SCR_ESurfaceMonitoringBehaviour.DISMANTLE);
					return;
				}

				vector outMat[3];
				SCR_DamageContext damageContext = new SCR_DamageContext(EDamageType.TRUE, dmgMgrComp.GetMaxHealth(), outMat, GetOwner(), null, Instigator.CreateInstigator(null), null, -1, -1);
				damageContext.damageEffect = new SCR_InstantDamageEffect();
				dmgMgrComp.HandleDamage(damageContext);
				return;

			case SCR_ESurfaceMonitoringBehaviour.DELETE:
				// if something is deployed on top of this object, then lets inform them via deployment, because this component doesnt have a deletion invoker
				if (m_OnDeployedStateChanged)
					m_OnDeployedStateChanged.Invoke(!m_bIsDeployed, this);

				RplComponent.DeleteRplEntity(owner, false);
				return;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Callback method used to update this entity when surface on which it was deployed changed its damage state
	//! \param[in] damagedHitZone
	void OnMonitoredSurfaceDamageStateChanged(SCR_HitZone damagedHitZone)
	{
		if (damagedHitZone.GetDamageState() != EDamageState.DESTROYED)
			return;

		StopMonitoringSurface(damagedHitZone.GetOwner(), damagedHitZone);
		ExecuteObservationBehaviour();
	}

	//------------------------------------------------------------------------------------------------
	//! Callback method used to update this entity when surface on which it was deployed changed its deployment state
	//! \param[in] newState
	//! \param[in] component
	void OnMonitoredSurfaceDismantled(bool newState, SCR_BaseDeployableInventoryItemComponent component)
	{
		StopMonitoringSurface(component.GetOwner(), deployableComp: component);
		ExecuteObservationBehaviour();
	}

	//------------------------------------------------------------------------------------------------
	//! Callback method used to update this entity when surface on which it was deployed got deleted
	//! \param[in] deletedEntity
	void OnMonitoredSurfaceDeleted(IEntity deletedEntity)
	{
		ExecuteObservationBehaviour();
	}

	//------------------------------------------------------------------------------------------------
	//! Transform to have the right direction
	//! \param[inout] transform matrix with position - marked as inout as operations done on static arrays are done through the pointers thus will impact what is passed into the method
	//! \param[in] direction in which spawned entity should be aligned to
	protected void ComputeTransform(inout vector transform[4], vector direction)
	{
		vector directionTrans[4];
		direction += m_VariantContainer.GetAdditionaPlacementRotation();

		Math3D.AnglesToMatrix(direction, directionTrans);

		vector right = transform[1] * directionTrans[0];
		vector forward = transform[1] * right;
		right.Normalize();
		forward.Normalize();

		SCR_MultiPartDeployableItemComponentClass data = SCR_MultiPartDeployableItemComponentClass.Cast(GetComponentData(GetOwner()));
		if (data && data.IsFrontFacingPlayer())
		{
			transform[0] = forward;
			transform[2] = right;
		}
		else
		{
			transform[0] = -forward;
			transform[2] = -right;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Method for deleting elements that are marked for Deletion
	//! If f.e. you would want to get damage value of a tire to pass it to assembled entity (f.e. bicycle) then override this
	protected void DeleteParts()
	{
		foreach (SCR_RequiredDeployablePart partReq : m_aFoundElements)
		{
			if (!partReq.ShouldDeleteFoundEntities())
				continue;

			for (int i = partReq.GetNumberOfFoundEntities() - 1; i >= 0; i--)
			{
				if (!partReq.DeleteFoundEntity(i, m_PreviousOwner))
					Print("WARNING: SCR_MultiPartDeployableItemComponent => Deletion of required entity failed as this entity doesnt exist anymore", LogLevel.WARNING);
			}
		}

		TransferDamageValues();

		IEntity owner = GetOwner();
		SCR_MultiPartDeployableItemComponentClass data = SCR_MultiPartDeployableItemComponentClass.Cast(GetComponentData(owner));
		if (!data)
			return;

		if (data.IsDeletedWhenDeployed())
		{
			if (data.MustDropAllStoredItems())
			{
				TryRemoveAllItemsFromStorage(owner, m_PreviousOwner);
				TryRemoveMagazineFromWeapons(owner, m_PreviousOwner);//because removing from storage doesnt removed currently attached magazines
			}
			else if (data.MustDropLoadedMagazines())
			{
				TryRemoveMagazineFromWeapons(owner, m_PreviousOwner);
			}

			GetGame().GetCallqueue().CallLater(DelayedDeletion);//delay to allow other replicated logic to be executed on clients
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Method meant to be used to transfer information about health between this entity and deployed entity
	protected void TransferDamageValues()
	{
		DamageManagerComponent damageMgr = DamageManagerComponent.Cast(m_ReplacementEntity.FindComponent(DamageManagerComponent));
		if (!float.AlmostEqual(m_fReplacementPrefabHealthScaled, 1) && damageMgr)
		{
			if (damageMgr)
				damageMgr.SetHealthScaled(m_fReplacementPrefabHealthScaled);
		}

		damageMgr = DamageManagerComponent.Cast(GetOwner().FindComponent(DamageManagerComponent));
		if (damageMgr && !float.AlmostEqual(damageMgr.GetHealthScaled(), 1))
		{
			SCR_MultiPartDeployableItemComponent deployableComp = SCR_MultiPartDeployableItemComponent.Cast(m_ReplacementEntity.FindComponent(SCR_MultiPartDeployableItemComponent));
			if (deployableComp)
				deployableComp.SetReplacmentPrefabHealthScaled(damageMgr.GetHealthScaled());
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Method that attempts to remove any magazine from a weapon
	static void TryRemoveMagazineFromWeapons(notnull IEntity owner, notnull IEntity user)
	{
		InventoryStorageManagerComponent storage = InventoryStorageManagerComponent.Cast(user.FindComponent(InventoryStorageManagerComponent));
		if (!storage)
			return;

		BaseMuzzleComponent muzzleComp = BaseMuzzleComponent.Cast(owner.FindComponent(BaseMuzzleComponent));
		if (muzzleComp && !muzzleComp.IsDisposable())
		{
			BaseMagazineComponent currentMagazine = muzzleComp.GetMagazine();
			InventoryItemComponent magIIC;
			if (currentMagazine)
				magIIC = InventoryItemComponent.Cast(currentMagazine.GetOwner().FindComponent(InventoryItemComponent));

			InventoryStorageSlot parentSlot;
			if (magIIC)
				parentSlot = magIIC.GetParentSlot();

			BaseInventoryStorageComponent magContainer;
			if (parentSlot)
				magContainer = parentSlot.GetStorage();

			if (magContainer)
				storage.TryRemoveItemFromStorage(currentMagazine.GetOwner(), magContainer);
		}

		array<Managed> weaponSlots = {};
		owner.FindComponents(WeaponSlotComponent, weaponSlots);
		if (weaponSlots.IsEmpty())
			return;

		IEntity weapon;
		WeaponSlotComponent weaponSlot;
		InventoryStorageSlot slot;
		foreach (Managed managedComp : weaponSlots)
		{
			weaponSlot = WeaponSlotComponent.Cast(managedComp);
			if (!weaponSlot)
				continue;

			slot = weaponSlot.GetSlotInfo();
			if (!slot)
				continue;

			weapon = slot.GetAttachedEntity();
			if (!weapon)
				continue;

			TryRemoveMagazineFromWeapons(weapon, user);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Method that attempts to remove all items stored by provided entity
	static void TryRemoveAllItemsFromStorage(notnull IEntity owner, notnull IEntity user)
	{
		InventoryStorageManagerComponent storageMgr = InventoryStorageManagerComponent.Cast(user.FindComponent(InventoryStorageManagerComponent));
		if (!storageMgr)
			return;

		BaseInventoryStorageComponent storage = BaseInventoryStorageComponent.Cast(owner.FindComponent(BaseInventoryStorageComponent));
		if (!storage)
			return;

		array<IEntity> outItems = {};
		storage.GetAll(outItems);
		foreach (IEntity item : outItems)
		{
			storageMgr.TryRemoveItemFromStorage(item, storage);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Method used to delte this entity but after some delay to ensure that other logic can execute correctly especially for proxy
	void DelayedDeletion()
	{
		SCR_EntityHelper.DeleteEntityAndChildren(GetOwner());
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnDelete(IEntity owner)
	{
		SCR_MultiPartDeployableItemComponentClass data = SCR_MultiPartDeployableItemComponentClass.Cast(GetComponentData(GetOwner()));
		if (!data)
			return;

		if (data.IsDeletedWhenDeployed())
			m_ReplacementEntity = null;//break the connection to prevent the deletion of deployed entity

		super.OnDelete(owner);
	}

	//------------------------------------------------------------------------------------------------
	protected override void RPC_PlaySoundOnDeployBroadcast(bool deploy)
	{
		IEntity ent = GetOwner();
		SCR_MultiPartDeployableItemComponentClass data = SCR_MultiPartDeployableItemComponentClass.Cast(GetComponentData(ent));
		if (!data)
			return;

		if (data.IsDeletedWhenDeployed() && m_ReplacementEntity)
			ent = m_ReplacementEntity;

		if (!ent)
			return;

		SoundComponent soundComp = SoundComponent.Cast(ent.FindComponent(SoundComponent));
		if (soundComp)
		{
			if (deploy)
				soundComp.SoundEvent(SCR_SoundEvent.SOUND_DEPLOY);
			else
				soundComp.SoundEvent(SCR_SoundEvent.SOUND_UNDEPLOY);

			return;
		}

		SCR_SoundManagerModule soundMan = SCR_SoundManagerModule.GetInstance(ent.GetWorld());
		if (!soundMan)
			return;

		if (deploy)
			soundMan.CreateAndPlayAudioSource(ent, SCR_SoundEvent.SOUND_DEPLOY);
		else
			soundMan.CreateAndPlayAudioSource(ent, SCR_SoundEvent.SOUND_UNDEPLOY);
	}

	//------------------------------------------------------------------------------------------------
	protected override bool RplSave(ScriptBitWriter writer)
	{
		bool hasData = !float.AlmostEqual(m_fReplacementPrefabHealthScaled, 1);
		writer.WriteBool(hasData);
		if (hasData)
			writer.WriteFloat01(m_fReplacementPrefabHealthScaled);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected override bool RplLoad(ScriptBitReader reader)
	{
		bool hasData;
		reader.ReadBool(hasData);
		if (hasData)
			reader.ReadFloat01(m_fReplacementPrefabHealthScaled);

		return true;
	}
}
