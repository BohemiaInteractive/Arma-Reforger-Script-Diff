[EntityEditorProps(category: "GameScripted/Building", description: "Component attached to a composition layout, holding information about the final composition to be built")]
class SCR_CampaignBuildingLayoutComponentClass : ScriptComponentClass
{
}

class SCR_CampaignBuildingLayoutComponent : ScriptComponent
{
	[RplProp(onRplName: "SpawnPreviewIfBuildingModeOpened")]
	protected int m_iPrefabId = INVALID_PREFAB_ID;

	[RplProp()]
	protected int m_iToBuildValue;

	// The value is float, because GM can set a building progress in percentage and if the value would be an int, the rounding of value is causing a confusion.
	[RplProp()]
	protected float m_fCurrentBuildValue;

	protected SCR_BasePreviewEntity m_PreviewEntity;

	protected static const int EMPTY_BUILDING_VALUE = 0;
	protected static const int INVALID_PREFAB_ID = -1;
	protected ref ScriptInvokerInt m_OnAddBuildingValueInt;
	protected ref ScriptInvokerVoid m_OnAddBuildingValueVoid;
	protected ref ScriptInvokerVoid m_OnCompositionIdSet;

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		GetOnAddBuildingValueInt().Insert(EvaluateBuildingStatus);
		GetOnCompositionIdSet().Insert(SpawnPreviewIfBuildingModeOpened);

		if (!SetOnEditorOpenEvent())
		{
			SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
			if (core)
				core.Event_OnEditorManagerInitOwner.Insert(SetOnEditorOpenEvent);
		}

		GetOnAddBuildingValueVoid().Insert(LockCompositionInteraction);
	}

	//------------------------------------------------------------------------------------------------
	//! Set the event on editor mode opening.
	//! \return
	bool SetOnEditorOpenEvent()
	{
		SCR_EditorManagerEntity editorManagerEntity = SCR_EditorManagerEntity.GetInstance();
		if (editorManagerEntity)
			editorManagerEntity.GetOnModeChange().Insert(OnEditorModeChanged);

		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (core)
			core.Event_OnEditorManagerInitOwner.Remove(SetOnEditorOpenEvent);

		return editorManagerEntity != null;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvokerInt GetOnAddBuildingValueInt()
	{
		if (!m_OnAddBuildingValueInt)
			m_OnAddBuildingValueInt = new ScriptInvokerInt();

		return m_OnAddBuildingValueInt;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvokerVoid GetOnAddBuildingValueVoid()
	{
		if (!m_OnAddBuildingValueVoid)
			m_OnAddBuildingValueVoid = new ScriptInvokerVoid();

		return m_OnAddBuildingValueVoid;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvokerVoid GetOnCompositionIdSet()
	{
		if (!m_OnCompositionIdSet)
			m_OnCompositionIdSet = new ScriptInvokerVoid();

		return m_OnCompositionIdSet;
	}

	//------------------------------------------------------------------------------------------------
	//! Event called when the editor mode changed to delete the composition preview.
	//! \param[in] currentModeEntity
	//! \param[in] prevModeEntity
	void OnEditorModeChanged(SCR_EditorModeEntity currentModeEntity, SCR_EditorModeEntity prevModeEntity)
	{
		if (currentModeEntity && currentModeEntity.GetModeType() == EEditorMode.BUILDING && IsLayoutInBuildingRange(currentModeEntity))
			SpawnPreview();
		else
			DeletePreview();
	}	

	//------------------------------------------------------------------------------------------------
	//! Is this layout in building radius of the provider
	//! \param[in] editorModeEntity
	//! \return
	bool IsLayoutInBuildingRange(notnull SCR_EditorModeEntity editorModeEntity)
	{
		SCR_CampaignBuildingEditorComponent buildingEditorComponent = SCR_CampaignBuildingEditorComponent.Cast(editorModeEntity.FindComponent(SCR_CampaignBuildingEditorComponent));
		if (!buildingEditorComponent)
			return false;
		
		SCR_CampaignBuildingProviderComponent providerComponent = buildingEditorComponent.GetProviderComponent();
		if (!providerComponent)
			return false;
		
		return vector.DistanceSq(GetOwner().GetOrigin(), providerComponent.GetOwner().GetOrigin()) <= providerComponent.GetBuildingRadius() * providerComponent.GetBuildingRadius();
	}

	//------------------------------------------------------------------------------------------------
	//! \return true if the building preview for this layout exists, false otherwise.
	bool HasBuildingPreview()
	{
		return m_PreviewEntity;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] currentBuildValue
	void EvaluateBuildingStatus(int currentBuildValue)
	{
#ifdef ENABLE_DIAG
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_INSTANT_BUILDING))
		{
			// Calling the method one frame later, otherwise it can causing spawning the same composition twice.
			GetGame().GetCallqueue().CallLater(SpawnComposition, 0, false);
			return;
		}
#endif
		if (currentBuildValue > m_iToBuildValue * 0.5)
			CreateUnsafeAreaEvent();
		
		if (currentBuildValue < m_iToBuildValue)
			return;

		SpawnComposition();
		GetOnAddBuildingValueInt().Remove(EvaluateBuildingStatus);
	}

	//------------------------------------------------------------------------------------------------
	//! Get the building value that defines how long it takes to build a composition (based on the tool, number of people building it etc.)
	//! \param[in] prefabID
	//! \return
	int GetBuildingValue(int prefabID)
	{
		ResourceName resName = GetCompositionResourceName(prefabID);
		if (resName.IsEmpty())
			return EMPTY_BUILDING_VALUE;

		SCR_CampaignBuildingManagerComponent buildingManagerComponent = GetBuildingManagerComponent();
		if (!buildingManagerComponent)
			return EMPTY_BUILDING_VALUE;

		SCR_CampaignBuildingCompositionOutlineManager outlineManager = buildingManagerComponent.GetOutlineManager();
		if (!outlineManager)
			return EMPTY_BUILDING_VALUE;

		return outlineManager.GetCompositionBuildingValue(resName);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] prefabID
	//! \return resource name based on prefab ID.
	protected ResourceName GetCompositionResourceName(int prefabID)
	{
		SCR_CampaignBuildingManagerComponent buildingManagerComponent = GetBuildingManagerComponent();
		if (!buildingManagerComponent)
			return string.Empty;

		return buildingManagerComponent.GetCompositionResourceName(prefabID);
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_CampaignBuildingManagerComponent GetBuildingManagerComponent()
	{
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return null;

		return SCR_CampaignBuildingManagerComponent.Cast(gameMode.FindComponent(SCR_CampaignBuildingManagerComponent));
	}

	//------------------------------------------------------------------------------------------------
	//! Save prefab ID of the composition, which is used by a Free Roam mode to know what composition this preview represents.
	//! \param[in] prefabId
	void SetPrefabId(int prefabId)
	{
		m_iPrefabId = prefabId;
		m_iToBuildValue = GetBuildingValue(prefabId);

		if (m_OnCompositionIdSet)
			m_OnCompositionIdSet.Invoke(prefabId);

		EvaluateBuildingStatus(m_fCurrentBuildValue);

		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetPrefabId()
	{
		return m_iPrefabId;
	}

	//------------------------------------------------------------------------------------------------
	//! Spawn the composition belonging to this layout.
	void SpawnComposition()
	{
		IEntity ent = GetOwner().GetRootParent();
		if (!ent)
			return;

		SCR_EditorLinkComponent linkComponent = SCR_EditorLinkComponent.Cast(ent.FindComponent(SCR_EditorLinkComponent));
		if (!linkComponent)
			return;

		linkComponent.SpawnComposition();

		// Call this as the composition can be spawned directly if the building value is set to zero. Yet we want to lock the composition to disable the interaction.
		LockCompositionInteraction();

		EntitySpawnParams spawnParams = new EntitySpawnParams;
		ent.GetWorldTransform(spawnParams.Transform);

		SCR_EditableEntityComponent entity = SCR_EditableEntityComponent.GetEditableEntity(ent);
		if (entity)
		{
			SCR_EditorPreviewParams params = SCR_EditorPreviewParams.CreateParams(spawnParams.Transform, verticalMode: EEditorTransformVertical.TERRAIN);
			SCR_RefPreviewEntity.SpawnAndApplyReference(entity, params);
		}

		SCR_EntityHelper.DeleteEntityAndChildren(GetOwner());
	}

	//------------------------------------------------------------------------------------------------
	//! Create a preview if it does not exist
	void SpawnPreview()
	{
		if (m_PreviewEntity)
			return;

		IEntity ent = GetOwner().GetRootParent();
		if (!ent)
			return;

		ResourceName resName = GetCompositionResourceName(m_iPrefabId);
		if (resName.IsEmpty())
			return;

		Resource res = Resource.Load(resName);
		if (!res.IsValid())
			return;

		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return;

		SCR_CampaignBuildingManagerComponent buildingManagerComponent = SCR_CampaignBuildingManagerComponent.Cast(gameMode.FindComponent(SCR_CampaignBuildingManagerComponent));
		if (!buildingManagerComponent)
			return;

		EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;
		ent.GetWorldTransform(spawnParams.Transform);

		m_PreviewEntity = SCR_PrefabPreviewEntity.SpawnPreviewFromPrefab(res, "SCR_PrefabPreviewEntity", ent.GetWorld(), spawnParams, "{58F07022C12D0CF5}Assets/Editor/PlacingPreview/Preview.emat");
		m_PreviewEntity.SetPreviewTransform(spawnParams.Transform, EEditorTransformVertical.TERRAIN);
		if (!m_PreviewEntity)
			return;

		ent.AddChild(m_PreviewEntity, -1, EAddChildFlags.RECALC_LOCAL_TRANSFORM);

		m_PreviewEntity.Update();

		IEntity playerEntity = SCR_PlayerController.GetLocalMainEntity();
		if (!playerEntity)
			return;
		
		SCR_CharacterControllerComponent characterController = SCR_CharacterControllerComponent.Cast(playerEntity.FindComponent(SCR_CharacterControllerComponent));
		if (!characterController)
			return;
		
		characterController.GetOnPlayerDeath().Insert(DeletePreview);
	}

	//------------------------------------------------------------------------------------------------
	//! Spawn the composition preview if the building mode is active (open)
	void SpawnPreviewIfBuildingModeOpened()
	{
		SCR_EditorModeEntity modeEntity = GetBuildingModeEntity();
		if (!modeEntity)
			return;

		if (modeEntity.IsOpened())
			SpawnPreview();
	}

	//------------------------------------------------------------------------------------------------
	//!
	void DeletePreview()
	{
		if (!m_PreviewEntity)
			return;
		
		IEntity playerEntity = SCR_PlayerController.GetLocalMainEntity();
		if (!playerEntity)
			return;
		
		SCR_CharacterControllerComponent characterController = SCR_CharacterControllerComponent.Cast(playerEntity.FindComponent(SCR_CharacterControllerComponent));
		if (!characterController)
			return;
		
		characterController.GetOnPlayerDeath().Remove(DeletePreview);
		
		// if there was open an editor, remove the invoker
		SCR_EditorModeEntity modeEntity = GetBuildingModeEntity();
		if (modeEntity)
			modeEntity.GetOnClosed().Remove(DeletePreview);

		IEntity ent = m_PreviewEntity.GetRootParent();
		if (!ent)
			ent = GetOwner().GetRootParent();
		
		if (ent)
			ent.RemoveChild(m_PreviewEntity);
		
		m_PreviewEntity.Update();
		SCR_EntityHelper.DeleteEntityAndChildren(m_PreviewEntity);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] value
	void AddBuildingValue(int value)
	{
		m_fCurrentBuildValue += value;
		m_fCurrentBuildValue = Math.Clamp(m_fCurrentBuildValue, 0, m_iToBuildValue);

		if (m_OnAddBuildingValueInt)
			m_OnAddBuildingValueInt.Invoke(m_fCurrentBuildValue);

		if (m_OnAddBuildingValueVoid)
			m_OnAddBuildingValueVoid.Invoke();

		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] newValue
	void SetBuildingValue(float newValue)
	{
		newValue = Math.Clamp(newValue, 0, m_iToBuildValue);
		m_fCurrentBuildValue = newValue;

		if (m_OnAddBuildingValueInt)
			m_OnAddBuildingValueInt.Invoke(m_fCurrentBuildValue);

		if (m_OnAddBuildingValueVoid)
			m_OnAddBuildingValueVoid.Invoke();

		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	//! \return value that needs to be reached to spawn the composition.
	int GetToBuildValue()
	{
		return m_iToBuildValue;
	}

	//------------------------------------------------------------------------------------------------
	//! \return current value
	float GetCurrentBuildValue()
	{
		return m_fCurrentBuildValue;
	}

	//------------------------------------------------------------------------------------------------
	//! Search for an editor mode entity (exists only when the player is in editor mode)
	//! \return
	SCR_EditorModeEntity GetBuildingModeEntity()
	{
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core)
			return null;

		SCR_EditorManagerEntity editorManager = core.GetEditorManager();
		if (!editorManager)
			return null;

		return editorManager.FindModeEntity(EEditorMode.BUILDING);
	}

	//------------------------------------------------------------------------------------------------
	//! Once someone for the 1st time add a value to build a composition,
	//! set the lock at this composition to disallow any manipulation (deleting, moving) with this composition until it's fully build.
	void LockCompositionInteraction()
	{
		IEntity rootEnt = GetOwner().GetRootParent();
		if (!rootEnt)
			return;

		SCR_CampaignBuildingCompositionComponent compositionComponent = SCR_CampaignBuildingCompositionComponent.Cast(rootEnt.FindComponent(SCR_CampaignBuildingCompositionComponent));
		if (!compositionComponent)
			return;

		compositionComponent.SetInteractionLockServer(true);
		GetOnAddBuildingValueVoid().Remove(LockCompositionInteraction);
	}

	//------------------------------------------------------------------------------------------------
	protected void SnapEntityToTerrain(IEntity entity)
	{
		if (!entity)
			return;
		
		vector transform[4];
		Math3D.MatrixIdentity4(transform);
		
		transform[3] = entity.GetOrigin();
		
		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return;
		
		transform[3][1] = world.GetSurfaceY(transform[3][0], transform[3][2]);
		entity.SetTransform(transform);
	}

	//------------------------------------------------------------------------------------------------
	override void OnChildAdded(IEntity parent, IEntity child)
	{
		super.OnChildAdded(parent, child);
		
		// child local transform is applied one frame later on server so snapping to terrain needs to be delayed by one frame
		RplComponent rplComp = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		if (!rplComp || rplComp.IsProxy())
			SnapEntityToTerrain(child);
		else
			GetGame().GetCallqueue().CallLater(SnapEntityToTerrain, 0, false, child);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Create a danger event for AI to clear the area where the composition is about to spawn.
	protected void CreateUnsafeAreaEvent()
	{
		vector boundMin, boundMax;
		GetOwner().GetWorldBounds(boundMin, boundMax);
		float previewSize = vector.DistanceXZ(boundMin, boundMax) * 1.25;
		
		SCR_AIDangerEvent_UnsafeArea unsafeAreaEvent = new SCR_AIDangerEvent_UnsafeArea();
		unsafeAreaEvent.SetDangerType(EAIDangerEventType.Danger_UnsafeArea);
		unsafeAreaEvent.SetPosition(GetOwner().GetOrigin());
		unsafeAreaEvent.SetRadius(previewSize);
		GetGame().GetAIWorld().RequestBroadcastDangerEvent(unsafeAreaEvent);	
	}

	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_CampaignBuildingLayoutComponent()
	{
		// Delete preview when the layout is removed (composition build, layout deleted from editor mode, etc.);
		DeletePreview();
	}
}
