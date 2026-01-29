[ComponentEditorProps(category: "GameScripted/Editor", description: "Entity transformation (moving and rotating). Works only with SCR_EditorBaseEntity!", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_TransformingEditorComponentClass: SCR_BaseEditorComponentClass
{
	[Attribute(category: "Effects")]
	private ref array<ref SCR_BaseEditorEffect> m_EffectsTransforationStart;
	
	[Attribute(category: "Effects")]
	private ref array<ref SCR_BaseEditorEffect> m_EffectsTransformationConfirm;
	
	[Attribute(category: "Effects")]
	private ref array<ref SCR_BaseEditorEffect> m_EffectsTransformationCancel;
	
	array<ref SCR_BaseEditorEffect> GetEffectsTransforationStart()
	{
		return m_EffectsTransforationStart;
	}
	array<ref SCR_BaseEditorEffect> GetEffectsTransformationConfirm()
	{
		return m_EffectsTransformationConfirm;
	}
	array<ref SCR_BaseEditorEffect> GetEffectsTransformationCancel()
	{
		return m_EffectsTransformationCancel;
	}
};

/** @ingroup Editor_Components
*/

/*!
Manager of basic entity transformation in all axes, e.g., translation or rotation.
*/
class SCR_TransformingEditorComponent : SCR_BaseEditorComponent
{	
	protected ref set<SCR_EditableEntityComponent> m_aEditedEntities;
	protected ref map<SCR_EditableEntityComponent, vector> m_mServerEntityStartingPosition = new map<SCR_EditableEntityComponent, vector>();
	protected SCR_EditableEntityComponent m_EditedPivot;
	protected SCR_EditableEntityComponent m_EditedLayer;
	protected SCR_RefPreviewEntity m_RefEntity;
	protected SCR_StatesEditorComponent m_StatesManager;
	protected SCR_PreviewEntityEditorComponent m_PreviewManager;
	protected bool m_bIsComposition;
	protected bool m_bCanBeTransformed = true;
	
	protected ref ScriptInvoker Event_OnTransformationRequest = new ScriptInvoker;
	protected ref ScriptInvoker Event_OnTransformationStart = new ScriptInvoker;
	protected ref ScriptInvoker Event_OnTransformationConfirm = new ScriptInvoker;
	protected ref ScriptInvoker Event_OnTransformationCancel = new ScriptInvoker;
	// Called in beginning of Clean so system realying on tranforming entities can handle them before cleared in component
	protected ref ScriptInvokerVoid m_OnCleanStart = new ScriptInvokerVoid();
	
	//Server only script invokers
	protected ref ScriptInvoker Event_OnTransformationConfirmServer = new ScriptInvoker;
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Start
	
	//------------------------------------------------------------------------------------------------
	/*!
	Initiate editing process.
	Must be called on editor's owner.
	\param pivot Directly edited entity (most likely the one under cursor)
	\param entities Edited entities
	\param transform Pivot transformation, e.g., transformation of entity under cursor or position on which cursor clicked
	*/
	void StartEditing(SCR_EditableEntityComponent pivot, notnull set<SCR_EditableEntityComponent> entities, vector transform[4])
	{
		//--- Not an owner, ignore
		if (!IsOwner() || entities.IsEmpty()) return;
		
		if (m_PreviewManager) m_PreviewManager.InitTransform(transform);
		
		m_bIsComposition = true;
		int id = 0;
		array<int> entityIds = new array<int>;
		m_aEditedEntities = new set<SCR_EditableEntityComponent>;

		
		//--- Connect to layer manager
		SCR_LayersEditorComponent layersManager = SCR_LayersEditorComponent.Cast(GetInstance(SCR_LayersEditorComponent));
		SCR_EditableEntityComponent parentBelowCurrentLayer = null;
		if (layersManager)
		{
			m_EditedLayer = layersManager.GetCurrentLayer();
		
			//--- Is composition root edited as well? If not, insert it.
			parentBelowCurrentLayer = layersManager.GetParentBelowCurrentLayer(pivot);
			if (entities.Find(parentBelowCurrentLayer) != -1 && parentBelowCurrentLayer.IsReplicated(id))
			{
				if (m_aEditedEntities.Insert(parentBelowCurrentLayer))
					entityIds.Insert(id);
			}
		}
		
		//--- Add valid entities and check if they are all part of one composition
		foreach (SCR_EditableEntityComponent entity: entities)
		{
			if (entity.IsReplicated(id) && m_aEditedEntities.Find(entity) == -1)
			{
				m_aEditedEntities.Insert(entity);
				entityIds.Insert(id);
				
				if (entity != parentBelowCurrentLayer && !entity.IsChildOf(parentBelowCurrentLayer))
					m_bIsComposition = false;
			}
		}
		
		//--- Not valid entities found, ignore
		if (entityIds.IsEmpty())
		{
			Clean();
			return;
		}
		
		//--- All selected entities belong to one composition - use its rotation instead. so it can be snapped to slots
		if (m_bIsComposition)
			pivot = parentBelowCurrentLayer;
		
		//---- Validate pivot point
		int pivotId;
		if (!pivot.IsReplicated(pivotId))
		{
			Clean();
			return;
		}
		m_EditedPivot = pivot;
		
		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (core)
		{
			core.Event_OnEntityUnregistered.Insert(OnEntityUnregistered);
			core.Event_OnEntityVisibilityChanged.Insert(OnEntityVisibilityChanged);
			core.Event_OnEntityAccessKeyChanged.Insert(OnEntityAccessKeyChanged);
		}
		
		m_StatesManager.SetState(EEditorState.TRANSFORMING);
		m_StatesManager.SetIsWaiting(true);
		Event_OnTransformationRequest.Invoke(m_aEditedEntities);
		
		Rpc(StartEditingServer, pivotId, entityIds, vector.One, transform, m_PreviewManager.IsUnderwater(), m_PreviewManager.GetVerticalMode());
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void StartEditingServer(int pivotId, array<int> entityIds, vector dummyVector, vector transform[4], bool isUnderwater, EEditorTransformVertical verticalMode)
	{
		m_aEditedEntities = new set<SCR_EditableEntityComponent>;
		m_mServerEntityStartingPosition.Clear();
		
		m_EditedPivot = SCR_EditableEntityComponent.Cast(Replication.FindItem(pivotId));
		
		foreach (int id: entityIds)
		{
			SCR_EditableEntityComponent entity = SCR_EditableEntityComponent.Cast(Replication.FindItem(id));
			if (!entity) continue;
			
			GenericEntity owner = entity.GetOwner();
			if (!owner) return;
			
			vector pos;
			entity.GetPos(pos);
			
			m_aEditedEntities.Insert(entity);
			m_mServerEntityStartingPosition.Insert(entity, pos);
		}
		
		//--- Create reference entity
		EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.Transform = transform;
		
		EPreviewEntityFlag flags = EPreviewEntityFlag.IGNORE_PREFAB | EPreviewEntityFlag.ONLY_EDITABLE;
		if (isUnderwater)
			flags |= EPreviewEntityFlag.UNDERWATER;
		
		if (verticalMode == EEditorTransformVertical.GEOMETRY)
			flags |= EPreviewEntityFlag.GEOMETRY;

#ifdef PREVIEW_ENTITY_SHOW_REFERENCE
		const ResourceName material = "{D0126AF0E6A27141}Common/Materials/Colors/colorRed.emat"; // TODO: check for good const usage
#else
		const ResourceName material; // TODO: check for good const usage
#endif

		m_RefEntity = SCR_RefPreviewEntity.Cast(SCR_RefPreviewEntity.SpawnPreviewFromEditableEntities(m_aEditedEntities, "SCR_RefPreviewEntity", GetGame().GetWorld(), spawnParams, material, flags));
		
		int simulatedDelay = DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_EDITOR_NETWORK_DELAY) * 100;
		if (simulatedDelay > 0 && !Replication.IsRunning())
			GetGame().GetCallqueue().CallLater(StartEditingOwner, simulatedDelay, false);
		else
			Rpc(StartEditingOwner);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void StartEditingOwner()
	{
		if (!m_PreviewManager)
			return;

		//--- Transforming was meanwhile canceled, ignore
		if (!m_StatesManager.SetIsWaiting(false) || m_StatesManager.GetState() != EEditorState.TRANSFORMING)
		{
			CancelEditing();
			return;
		}

		//--- Create preview
		m_PreviewManager.CreatePreview(m_EditedPivot, m_aEditedEntities);
		Event_OnTransformationStart.Invoke(m_aEditedEntities);
		
		SCR_TransformingEditorComponentClass prefabData = SCR_TransformingEditorComponentClass.Cast(GetEditorComponentData());
		if (prefabData) SCR_BaseEditorEffect.Activate(prefabData.GetEffectsTransforationStart(), this, entities: m_aEditedEntities);
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Confirm
	
	//------------------------------------------------------------------------------------------------
	/*!
	Confirm editing process.
	Must be called on editor's owner.
	All entities marked for editing by StartEditing() will be transformed to match the current position and rotation of the preview entity.
	\return True if the request was sent
	*/
	bool ConfirmEditing()
	{
		//--- No change or incorrect position
		if (!m_PreviewManager || !m_PreviewManager.IsChange())
		{
			//CancelEditing();
			if (m_PreviewManager.IsEditing())
				SendNotification(ENotification.EDITOR_TRANSFORMING_INCORRECT_POSITION);
			
			return false;
		}
		
		//--- Not an owner, ignore
		if (!IsOwner()) return false;
		
		//--- Not editing, ignore
		if (!m_aEditedEntities || !m_PreviewManager.IsEditing()) return false;
		
		//--- Check if the current layer changed during transformation
		bool currentLayerChanged;
		SCR_LayersEditorComponent layersManager = SCR_LayersEditorComponent.Cast(GetInstance(SCR_LayersEditorComponent));
		SCR_EditableEntityComponent currentLayer;
		if (layersManager)
		{
			currentLayer = layersManager.GetCurrentLayer();
			currentLayerChanged = m_EditedLayer != layersManager.GetCurrentLayer();
		}
		
		//--- Create packet	
		SCR_EditorPreviewParams params = SCR_EditorPreviewParams.CreateParamsFromPreview(m_PreviewManager, currentLayer, currentLayerChanged);
		if (!params) return false;
		
		//--- Send request to server
		Rpc(ConfirmEditingServer, params, GetManager().GetPlayerID());
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void ConfirmEditingServer(SCR_EditorPreviewParams params, int playerID)
	{
		if (!params.Deserialize())
		{
			Rpc(CancelEditingServer);
			return;
		}

		bool result = false;
		if (m_RefEntity)
		{
			m_RefEntity.ApplyReference(params);
		
			//--- Mark slot as occupied
			if (params.m_Target)
			{
				SCR_SiteSlotEntity slotEntity = SCR_SiteSlotEntity.Cast(params.m_Target.GetOwner());
				if (slotEntity) slotEntity.SetOccupant(m_EditedPivot.GetOwner());
			}
			
			foreach (SCR_EditableEntityComponent entity : m_aEditedEntities)
			{
				entity.SetAuthor(playerID);
			}
			
			Event_OnTransformationConfirmServer.Invoke(m_aEditedEntities, m_mServerEntityStartingPosition);
			result = true;
		}
		
		Rpc(ConfirmEditingOwner, result);
		
		Clean();
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void ConfirmEditingOwner(bool result)
	{
		if (result)
		{
			vector transform[4];
			m_PreviewManager.GetPreviewTransform(transform);
			
			Event_OnTransformationConfirm.Invoke(m_aEditedEntities);
			SCR_TransformingEditorComponentClass prefabData = SCR_TransformingEditorComponentClass.Cast(GetEditorComponentData());
			if (prefabData) SCR_BaseEditorEffect.Activate(prefabData.GetEffectsTransformationConfirm(), this, transform[3], m_aEditedEntities);
		}
		else
		{
			//--- Evaluate failure when editing ends, not when it starts, as "empty drag" is more confusing for user
			SCR_NotificationsComponent.SendLocal(ENotification.EDITOR_TRANSFORMING_FAIL);
		}
		
		Clean();
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Cancel
	
	//------------------------------------------------------------------------------------------------
	/*!
	Cancel editing process.
	Must be called on editor's owner.
	*/
	void CancelEditing()
	{
		//--- Not an owner, ignore
		if (!IsOwner()) return;
		
		//--- Not editing, ignore
		if (!m_aEditedEntities) return;
		
		Event_OnTransformationCancel.Invoke(m_aEditedEntities);
		SCR_TransformingEditorComponentClass prefabData = SCR_TransformingEditorComponentClass.Cast(GetEditorComponentData());
		if (prefabData) SCR_BaseEditorEffect.Activate(prefabData.GetEffectsTransformationCancel(), this, entities: m_aEditedEntities);
		
		Rpc(CancelEditingServer);
		
		Clean();
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void CancelEditingServer()
	{
		Clean();
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Support Funcions
	
	//------------------------------------------------------------------------------------------------
	/*!
	Check if editing is currently on.
	\return True when editing
	*/
	bool IsEditing()
	{
		return m_aEditedEntities != null;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Check if given entity is edited
	\return True when editing the entity
	*/
	bool IsEditing(SCR_EditableEntityComponent entity)
	{
		return m_aEditedEntities && m_aEditedEntities.Find(entity) != -1;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get event called when request for editing is sent to server.
	Called only for editor user.
	\return Script invoker
	*/
	ScriptInvoker GetOnTransformationRequest()
	{
		return Event_OnTransformationRequest;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get event called when editing is confirmed by server.
	Called only for editor user.
	\return Script invoker
	*/
	ScriptInvoker GetOnTransformationStart()
	{
		return Event_OnTransformationStart;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get event called when editing is confirmed.
	Called only for editor user.
	\return Script invoker
	*/
	ScriptInvoker GetOnTransformationConfirm()
	{
		return Event_OnTransformationConfirm;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get event called when editing is confirmed on server only.
	Called only for editor user.
	\return Script invoker (returns null if not on server)
	*/
	ScriptInvoker GetOnTransformationConfirmServer()
	{
		if (!Replication.IsServer())
			return null;
		
		return Event_OnTransformationConfirmServer;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get event called when editing is canceled.
	Called only for editor user.
	\return Script invoker
	*/
	ScriptInvoker GetOnTransformationCancel()
	{
		return Event_OnTransformationCancel;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnCleanStart()
	{
		return m_OnCleanStart;
	}
	
	//------------------------------------------------------------------------------------------------
	bool CanBeTransformed()
	{
		return m_bCanBeTransformed;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetCanBeTransformed(bool val)
	{
		m_bCanBeTransformed = val;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Clean()
	{
		m_OnCleanStart.Invoke();
		
		m_aEditedEntities = null;
		m_EditedPivot = null;
#ifndef PREVIEW_ENTITY_SHOW_REFERENCE
		delete m_RefEntity;
#endif
		if (m_PreviewManager) m_PreviewManager.DeletePreview();
		
		if (m_StatesManager) m_StatesManager.UnsetState(EEditorState.TRANSFORMING);
		
		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (core)
		{
			core.Event_OnEntityUnregistered.Remove(OnEntityUnregistered);
			core.Event_OnEntityVisibilityChanged.Remove(OnEntityVisibilityChanged);
			core.Event_OnEntityAccessKeyChanged.Remove(OnEntityAccessKeyChanged);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnEntityUnregistered(SCR_EditableEntityComponent entity)
	{
		if (!m_aEditedEntities || m_aEditedEntities.Find(entity) == -1) return;
		
		CancelEditing();
		SendNotification(ENotification.EDITOR_TRANSFORMING_LOST_ACCESS, Replication.FindId(entity));
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnEntityVisibilityChanged(SCR_EditableEntityComponent entity)
	{
		if (!m_aEditedEntities || m_aEditedEntities.Find(entity) == -1) return;
		
		if (!entity.GetVisibleInHierarchy()) CancelEditing();
		SendNotification(ENotification.EDITOR_TRANSFORMING_LOST_ACCESS, Replication.FindId(entity));
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnEntityAccessKeyChanged(SCR_EditableEntityComponent entity)
	{
		if (!m_aEditedEntities || m_aEditedEntities.Find(entity) == -1) return;
		
		if (!entity.HasAccessInHierarchy()) CancelEditing();
		SendNotification(ENotification.EDITOR_TRANSFORMING_LOST_ACCESS, Replication.FindId(entity));
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Default Functions
	
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		/*
		if (Debug.KeyState(KeyCode.KC_DELETE) && m_aEditedEntities)
		{
			Debug.ClearKey(KeyCode.KC_DELETE);
			
			SCR_EditableEntityComponent entity = m_aEditedEntities[m_aEditedEntities.Count() - 1];
			if (entity) delete entity.GetOwner();
		}
		*/
		
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_EDITOR_TRANSFORM_START))
		{
			DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_EDITOR_TRANSFORM_START, false);
			
			SCR_EntitiesManagerEditorComponent entitiesManager = SCR_EntitiesManagerEditorComponent.Cast(GetInstance(SCR_EntitiesManagerEditorComponent));
			if (entitiesManager)
			{
				SCR_BaseEditableEntityFilter selectedFilter = entitiesManager.GetFilter(EEditableEntityState.SELECTED);
				if (selectedFilter)
				{
					set<SCR_EditableEntityComponent> selected = new set<SCR_EditableEntityComponent>;
					selectedFilter.GetEntities(selected);
					if (!selected.IsEmpty())
					{
						vector transform[4];
						selected[0].GetOwner().GetWorldTransform(transform);
						StartEditing(selected[0], selected, transform);
					}
				}
			}
		}
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_EDITOR_TRANSFORM_CONFIRM))
		{
			DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_EDITOR_TRANSFORM_CONFIRM, false);
			ConfirmEditing();
		}
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_EDITOR_TRANSFORM_CANCEL))
		{
			DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_EDITOR_TRANSFORM_CANCEL, false);
			CancelEditing();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnEditorActivate()
	{
		SetEventMask(GetOwner(), EntityEvent.FRAME);
		
		m_StatesManager = SCR_StatesEditorComponent.Cast(SCR_StatesEditorComponent.GetInstance(SCR_StatesEditorComponent, true));
		m_PreviewManager = SCR_PreviewEntityEditorComponent.Cast(SCR_PreviewEntityEditorComponent.GetInstance(SCR_PreviewEntityEditorComponent, true));
		
		DiagMenu.RegisterMenu(SCR_DebugMenuID.DEBUGUI_EDITOR_TRANSFORM, "Transforming", "Editor");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_EDITOR_TRANSFORM_START, "", "Start Transforming", "Transforming");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_EDITOR_TRANSFORM_CONFIRM, "", "Confirm Transforming", "Transforming");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_EDITOR_TRANSFORM_CANCEL, "", "Cancel Transforming", "Transforming");
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnEditorDeactivate()
	{
		Clean();
		
		DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_EDITOR_TRANSFORM_START);
		DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_EDITOR_TRANSFORM_CONFIRM);
		DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_EDITOR_TRANSFORM_CANCEL);
		DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_EDITOR_TRANSFORM_VERTICAL_MODE);
	}
};