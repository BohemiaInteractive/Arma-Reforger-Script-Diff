[ComponentEditorProps(category: "GameScripted/Editor", description: "Manager of current layer. Works only with SCR_EditorBaseEntity!", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_LayersEditorComponentClass: SCR_BaseEditorComponentClass
{
};

/** @ingroup Editor_Components
*/
/*!
Manager of current layer.
*/
class SCR_LayersEditorComponent : SCR_BaseEditorComponent
{
	[Attribute()]
	protected ResourceName m_sNewLayerPrefab;
	private SCR_EditableEntityComponent m_CurrentLayer;
	
	ref ScriptInvoker Event_OnCurrentLayerChange = new ScriptInvoker;
	
	protected bool m_bEditingLayersEnabled;
	protected SCR_BaseEditableEntityFilter m_SelectionFilter;
	protected SCR_EditableEntityCore m_Core;
	protected ref set<SCR_EditableEntityComponent> m_EntitiesToSelectAfterMove = new set<SCR_EditableEntityComponent>();
	protected SCR_EditableEntityComponent m_NewGroupLeader;
	protected bool m_bSelectAfterMoveCanceled;
	protected bool m_bSelectParentAfterMove;
	
	/*!
	\return True if layer editing is enabled
	*/
	bool IsEditingLayersEnabled()
	{
		return m_bEditingLayersEnabled;
	}
	
	/*!
	Set if layer editing should ne enabled.
	When disabled, it will not be possible to enter layers or move entities between them.
	\param enableLayerEditing True to enable layer editing
	*/
	void SetEditingLayersEnabled(bool enableLayerEditing)
	{
		if (m_bEditingLayersEnabled == enableLayerEditing)
			return;
		
		SetCurrentLayer(null);
		m_bEditingLayersEnabled = enableLayerEditing;
	}
	
	/*!
	Used to subscribe to Event_OnCurrentLayerChange.
	\return Script invoker is returned to subscribe to.
	*/
	ScriptInvoker GetOnCurrentLayerChange()
	{
		return Event_OnCurrentLayerChange;
	}

	//On editor mode change set layer null
	protected void OnEditorModeChanged(SCR_EditorModeEntity currentModeEntity, SCR_EditorModeEntity prevModeEntity)
	{
		if (prevModeEntity && prevModeEntity.GetModeType() == EEditorMode.EDIT)
			SetCurrentLayer(null);
	}
	
	/*!
	Set current layer.
	When a entity is focused / selected, it will include all siblings up to this current layer.
	\param entity Editable entity
	*/
	void SetCurrentLayer(SCR_EditableEntityComponent entity)
	{
		//--- Only root is allowed when layer editing is disabled
		if (!IsEditingLayersEnabled())
			entity = null;
		
		if (entity == m_CurrentLayer || (entity && !entity.HasEntityFlag(EEditableEntityFlag.LAYER)))
			return;
		
		SCR_EditableEntityComponent prevCurrentLayer = m_CurrentLayer;
		m_CurrentLayer = entity;
		Event_OnCurrentLayerChange.Invoke(m_CurrentLayer, prevCurrentLayer);
	}
	/*!
	Set selection current layer to the parent entity of the current current layer entity.
	\param entity Editable entity
	*/
	void SetCurrentLayerToParent()
	{
		SCR_EditableEntityComponent currentLayer = GetCurrentLayer();
		if (!currentLayer) return;
		
		SetCurrentLayer(currentLayer.GetParentEntity());
	}
	/*!
	When given entity is the current layer, move one layer up, otherwise set current layer to the entity's composition root
	(unless it's from inactive layer, in which case set the entity as current layer directly).
	\param entity Editable entity
	*/
	void ToggleCurrentLayer(SCR_EditableEntityComponent entity)
	{
		if (entity == GetCurrentLayer())
		{
			//--- Entity under cursor is the current layer, move one layer up
			if (entity) SetCurrentLayer(entity.GetParentEntity());
		}
		else
		{
			//--- Select entity's composition root
			SCR_EditableEntityComponent parent = GetParentBelowCurrentLayer(entity);
			if (!parent) parent = entity; //--- Entity from different layer used
			SetCurrentLayer(parent);
		}
	}
	
	/*!
	Get current layer.
	\return Editable entity
	*/
	SCR_EditableEntityComponent GetCurrentLayer()
	{
		return m_CurrentLayer;
	}
	
	/*!
	Creates a new layer and add selected entities in it
	*/
	void CreateNewLayerWithSelected(notnull set<SCR_EditableEntityComponent> entities, vector worldPosition)
	{
		array<RplId> entityIDs = new array<RplId>();
		RplId entityID;
		
		vector transform[4];
		entities[0].GetTransform(transform);
		transform[3] = worldPosition;
		
		foreach (SCR_EditableEntityComponent entity: entities)
		{			
			if (entity.IsReplicated(entityID))
				entityIDs.Insert(entityID);
		}
		
		Rpc(CreateNewLayerWithSelectedRpl, entityIDs, transform, Replication.FindId(m_CurrentLayer));
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void CreateNewLayerWithSelectedRpl(notnull array<RplId> entityIDs, vector transform[4], RplId currentLayerID)
	{
		EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.Transform = transform;
		
		IEntity layer = GetGame().SpawnEntityPrefab(Resource.Load(m_sNewLayerPrefab), GetGame().GetWorld(), spawnParams);
		SCR_EditableEntityComponent editableLayer = SCR_EditableEntityComponent.GetEditableEntity(layer);
		if (!editableLayer)
		{
			delete layer;
			return;
		}
		
		SCR_EditableEntityComponent currentLayer = SCR_EditableEntityComponent.Cast(Replication.FindItem(currentLayerID));
		if (currentLayer)
			editableLayer.SetParentEntity(currentLayer);
		
		MoveToLayerServer(entityIDs, Replication.FindId(editableLayer));
	}
	
	
	/*!
	Get parent below selection current layer, i.e., composition of an entity.
	When the entity is directly *below* current layer, the entity will be returned.
	When the entity is *above* the current layer, null is returned.
	Entity with IGNORE_LAYERS flag is always returned directly.
	\param entity Queried entity
	\return Editable entity
	*/
	SCR_EditableEntityComponent GetParentBelowCurrentLayer(SCR_EditableEntityComponent entity)
	{
		if (!entity || entity == m_CurrentLayer || entity.HasEntityFlag(EEditableEntityFlag.IGNORE_LAYERS))
			return entity;
		
		SCR_EditableEntityComponent parent;
		while (entity)
		{
			parent = entity.GetParentEntity();
			if (parent == m_CurrentLayer || (parent && parent.HasEntityFlag(EEditableEntityFlag.INDIVIDUAL_CHILDREN))) return entity;
			entity = parent;
		}
		return entity;
	}
	/*!
	Check if the entity is under given current layer.
	\param entity Queried entity
	\param onlyDirect True to check if the entity is directly under current layer, false to check it recursively
	\return True if under current layer, or if flagged with EEditableEntityFlag.IGNORE_LAYERS
	*/
	bool IsUnderCurrentLayer(SCR_EditableEntityComponent entity, bool onlyDirect = false, bool applyExceptions = false)
	{
		if (!entity)
			return false;
		
		//--- Always true when queried entity is the current layer or when there is no current layer (everything is under root in one way or another)
		//if (entity == m_CurrentLayer || (!onlyDirect && !m_CurrentLayer))
		if (!onlyDirect && (entity == m_CurrentLayer || !m_CurrentLayer))
			return true;
		
		if (applyExceptions)
		{
			//--- Root entities always visible, child entities only when their parent is
			if (entity.HasEntityFlag(EEditableEntityFlag.IGNORE_LAYERS))
			{
				SCR_EditableEntityComponent parent = entity.GetParentEntity();
				return !parent || IsUnderCurrentLayer(parent, onlyDirect);
			}
			
			while (entity)
			{
				entity = entity.GetParentEntity();
				if (entity == m_CurrentLayer)
					return true;
				
				if (onlyDirect && (!entity || !entity.HasEntityFlag(EEditableEntityFlag.INDIVIDUAL_CHILDREN)))
					return false;
			}
		}
		else
		{
			while (entity)
			{
				entity = entity.GetParentEntity();
				if (entity == m_CurrentLayer)
					return true;
				
				if (onlyDirect)
					return false;
			}
		}
		return false;
	}
	/*!
	Move all entities under the current root.
	Entities which are under other marked entities will remain under them.
	\param entities Affected entities
	*/
	void MoveToCurrentLayer(notnull set<SCR_EditableEntityComponent> entities)
	{
		MoveToLayer(entities, m_CurrentLayer);
	}
	/*!
	Move all entities under specific layer.
	Entities which are under other marked entities will remain under them.
	\param entities Affected entities
	\param layer Target layer
	*/
	void MoveToLayer(notnull set<SCR_EditableEntityComponent> entities, SCR_EditableEntityComponent layer)
	{
		array<RplId> entityIDs = new array<RplId>();
		RplId entityID;
		foreach (SCR_EditableEntityComponent entity: entities)
		{
			if (entity.IsReplicated(entityID))
				entityIDs.Insert(entityID);
		}
		
		Rpc(MoveToLayerServer, entityIDs, Replication.FindId(layer))
	}
	
	/*!
	Move all entities under specific layer then sets the layer as current and sets the entities moved selected
	Entities which are under other marked entities will remain under them.
	\param entities Affected entities
	\param layer Target layer
	*/
	void MoveToLayerAndSelect(notnull set<SCR_EditableEntityComponent> entities, SCR_EditableEntityComponent layer, bool selectParentAfterMove = false)
	{		
		if (!m_SelectionFilter)
			return;
		
		m_EntitiesToSelectAfterMove.Clear();
		
		selectParentAfterMove = (selectParentAfterMove && layer != null);
		
		foreach (SCR_EditableEntityComponent entity: entities)
		{
			m_EntitiesToSelectAfterMove.Insert(entity);
		}
		
		m_bSelectAfterMoveCanceled = false;
		m_Core.Event_OnParentEntityChanged.Insert(OnEditableParentChanged);
		m_SelectionFilter.GetOnChanged().Insert(OnSelectionChanged);
		
		SetCurrentLayer(layer);
		MoveToLayer(entities, layer);
	}
	
	void SplitGroupAndAddCharacters(SCR_EditableCharacterComponent leader, notnull set<SCR_EditableEntityComponent> entities)
	{
		if (!leader)
			return;
		
		m_EntitiesToSelectAfterMove.Clear();
		m_NewGroupLeader = leader;
		m_bSelectAfterMoveCanceled = false;
		m_bSelectParentAfterMove = true;
		m_Core.Event_OnParentEntityChanged.Insert(OnEditableParentChanged);
		m_SelectionFilter.GetOnChanged().Insert(OnSelectionChanged);
		
		foreach (SCR_EditableEntityComponent entity: entities)
		{
			m_EntitiesToSelectAfterMove.Insert(entity);
		}
		
		
		set<SCR_EditableEntityComponent> leaderSet = new set<SCR_EditableEntityComponent>;
		leaderSet.Insert(m_NewGroupLeader);
		MoveToLayer(leaderSet, null);
	}
	
	protected void AddCharactersToGroup(notnull set<SCR_EditableEntityComponent> entities, SCR_EditableEntityComponent group)
	{
		MoveToLayer(entities, group);
	}
	
	//Checkes if parent changed of entity that needs to be selected after move
	protected void OnEditableParentChanged(SCR_EditableEntityComponent entity, SCR_EditableEntityComponent newParent,  SCR_EditableEntityComponent prevParent)
	{
		if (!entity)
			return;
		
		if (m_EntitiesToSelectAfterMove.Contains(entity))
			GetGame().GetCallqueue().CallLater(DelayedSelectMovedEntity, 1, false, entity);
		else if (entity == m_NewGroupLeader)
		{
			if (m_EntitiesToSelectAfterMove.Count() > 0)
				AddCharactersToGroup(m_EntitiesToSelectAfterMove, newParent);
			else 
				EntitiesSelectAfterMoveDone(m_NewGroupLeader);
		}
	}
	
	//Delayed move so entity can be selected properly
	protected void DelayedSelectMovedEntity(SCR_EditableEntityComponent entity)
	{
		if (m_bSelectAfterMoveCanceled)
			return;
		
		if (!m_bSelectParentAfterMove && m_SelectionFilter)
			m_SelectionFilter.Add(entity, true);
		
		int index = m_EntitiesToSelectAfterMove.Find(entity);
		
		if (index >= 0)
			m_EntitiesToSelectAfterMove.Remove(index);
		
		//All entities have been selected after being moved
		if (m_EntitiesToSelectAfterMove.IsEmpty())	
			EntitiesSelectAfterMoveDone(entity);
	}
	
	//On selection changed
	protected void OnSelectionChanged(EEditableEntityState state, set<SCR_EditableEntityComponent> entitiesSelected, set<SCR_EditableEntityComponent> entitiesDeselected)
	{
		if (entitiesSelected && !entitiesSelected.IsEmpty())
		{
			foreach (SCR_EditableEntityComponent entity: entitiesSelected)
			{
				if (!entity)
					continue;
				
				if (!m_EntitiesToSelectAfterMove.Contains(entity) && entity != m_NewGroupLeader)
				{
					ClearEntitiesSelectAfterMove();
					return;
				}
			}
		}	
		if (entitiesDeselected && !entitiesDeselected.IsEmpty())
		{			
			foreach (SCR_EditableEntityComponent entity: entitiesDeselected)
			{
				if (!entity)
					continue;
				
				if (!m_EntitiesToSelectAfterMove.Contains(entity) && entity != m_NewGroupLeader)
				{
					ClearEntitiesSelectAfterMove();
					return;
				}
			}
		}
	}
	
	//Clear all var and event listeners
	protected void ClearEntitiesSelectAfterMove()
	{
		if (m_Core)
			m_Core.Event_OnParentEntityChanged.Remove(OnEditableParentChanged);
		if (m_SelectionFilter)
			m_SelectionFilter.GetOnChanged().Remove(OnSelectionChanged);
		
		m_bSelectParentAfterMove = false;
		m_bSelectAfterMoveCanceled = true;
		m_NewGroupLeader = null;
		m_EntitiesToSelectAfterMove.Clear();
	}
	
	protected void EntitiesSelectAfterMoveDone(SCR_EditableEntityComponent lastEntity)
	{
		if (m_bSelectParentAfterMove && m_SelectionFilter)
		{
			m_SelectionFilter.Add(lastEntity.GetParentEntity(), true);
		}
		
		ClearEntitiesSelectAfterMove();
	}
	
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void MoveToLayerServer(notnull array<RplId> entityIDs, RplId layerID)
	{
		//--- Deserialize
		SCR_EditableEntityComponent layer = SCR_EditableEntityComponent.Cast(Replication.FindItem(layerID));
		
		set<SCR_EditableEntityComponent> entities = new set<SCR_EditableEntityComponent>();
		SCR_EditableEntityComponent entityCandidate;
		foreach (RplId entityID: entityIDs)
		{
			entityCandidate = SCR_EditableEntityComponent.Cast(Replication.FindItem(entityID));
			if (entityCandidate)
				entities.Insert(entityCandidate);
		}
		
		//--- Move to target layer
		foreach (SCR_EditableEntityComponent entity: entities)
		{
			bool parentInEntities = false;
			SCR_EditableEntityComponent parent = entity.GetParentEntity();
			while (parent)
			{
				if (entities.Find(parent) != -1)
				{
					parentInEntities = true;
					break;
				}
				parent = parent.GetParentEntity();
			}
			if (!parentInEntities)
			{
				entity.SetParentEntity(layer, true);
			}
		}
	}
	
	protected void OnUserSettingsChanged()
	{
		BaseContainer editorSettings = GetGame().GetGameUserSettings().GetModule("SCR_EditorSettings");
		if (editorSettings)
		{
			bool state;
			editorSettings.Get("m_bLayerEditing", state);
			SetEditingLayersEnabled(state);
		}
	}
	
	protected void OnEntityUnregistered(SCR_EditableEntityComponent entity)
	{
		if (entity == m_CurrentLayer)
			SetCurrentLayer(null);
	}
	protected void OnEntityAccessKeyChanged(SCR_EditableEntityComponent entity)
	{
		if (entity == m_CurrentLayer && !entity.HasAccessInHierarchy())
			SetCurrentLayer(null);
	}
	protected void OnEntityVisibilityChanged(SCR_EditableEntityComponent entity)
	{
		if (entity == m_CurrentLayer && !entity.GetVisibleInHierarchy())
			SetCurrentLayer(null);
	}
	protected void OnAccessKeysChanged()
	{
		if (m_CurrentLayer && !m_CurrentLayer.HasAccessInHierarchy())
			SetCurrentLayer(null);
	}
	override void EOnEditorDebug(array<string> debugTexts)
	{
		if (!IsActive()) return;
		
		string currentLayerName = "/";
		if (m_CurrentLayer)
		{
			SCR_EditableEntityComponent currentLayer = m_CurrentLayer;
			while (currentLayer)
			{
				currentLayerName = "/" + currentLayer.GetDisplayName() + currentLayerName;
				currentLayer = currentLayer.GetParentEntity();
			}
		}
		debugTexts.Insert("Current Layer: Root:" + currentLayerName);
	}
	override void EOnEditorInit()
	{
		m_Core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (m_Core)
		{
			m_Core.Event_OnEntityUnregistered.Insert(OnEntityUnregistered);
			m_Core.Event_OnEntityAccessKeyChanged.Insert(OnEntityAccessKeyChanged);
			m_Core.Event_OnEntityVisibilityChanged.Insert(OnEntityVisibilityChanged);
		}
		SCR_AccessKeysEditorComponent accessKeysManager = SCR_AccessKeysEditorComponent.Cast(GetInstance(SCR_AccessKeysEditorComponent));
		if (accessKeysManager)
		{
			accessKeysManager.Event_OnChanged.Insert(OnAccessKeysChanged);
		}
		
		//Get layer enabled settings
		OnUserSettingsChanged();
		GetGame().OnUserSettingsChangedInvoker().Insert(OnUserSettingsChanged);
	}
	
	override void EOnEditorDelete()
	{
		if (m_Core)
		{
			m_Core.Event_OnEntityUnregistered.Remove(OnEntityUnregistered);
			m_Core.Event_OnEntityAccessKeyChanged.Remove(OnEntityAccessKeyChanged);
			m_Core.Event_OnEntityVisibilityChanged.Remove(OnEntityVisibilityChanged);
		}
		SCR_AccessKeysEditorComponent accessKeysManager = SCR_AccessKeysEditorComponent.Cast(GetInstance(SCR_AccessKeysEditorComponent));
		if (accessKeysManager)
		{
			accessKeysManager.Event_OnChanged.Remove(OnAccessKeysChanged);
		}
		
		if (!m_EntitiesToSelectAfterMove.IsEmpty())
			ClearEntitiesSelectAfterMove();
	}
	
	override void EOnEditorOpen()
	{
		super.EOnEditorOpen();
		
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (editorManager) 
			editorManager.GetOnModeChange().Insert(OnEditorModeChanged);
		
		//Get selection filter
		if (!m_SelectionFilter)
			m_SelectionFilter = SCR_BaseEditableEntityFilter.GetInstance(EEditableEntityState.SELECTED);
	}
	
	override void EOnEditorClose()
	{
		super.EOnEditorClose();
		
		//Go out of editing layer
		SetCurrentLayer(null);
		
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (editorManager) 
			editorManager.GetOnModeChange().Remove(OnEditorModeChanged);
	}
	
	override void OnDelete(IEntity owner)
	{
		if (GetGame())
			GetGame().OnUserSettingsChangedInvoker().Remove(OnUserSettingsChanged);
	}
};
