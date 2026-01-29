[ComponentEditorProps(category: "GameScripted/Editor", description: "Content browser component. Works only with SCR_EditorBaseEntity!", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_PlacingEditorComponentClass: SCR_BaseEditorComponentClass
{
	[Attribute(category: "Placing", uiwidget: UIWidgets.Flags, desc: "System type of the entity.", enums: ParamEnumArray.FromEnum(EEditorPlacingFlags))]
	protected EEditorPlacingFlags m_AllowedPlacingFlags;
	
	[Attribute(category: "Placing", uiwidget: UIWidgets.ResourcePickerThumbnail, desc: "Editable entity used for testing. Should be a sphere 1 m in diameter.")]
	protected ResourceName m_TestPrefab;
	
	[Attribute(category: "Placing")]
	protected ref array<ref SCR_PlaceableEntitiesRegistry> m_Registries;
	
	[Attribute(category: "Effects")]
	protected ref array<ref SCR_BaseEditorEffect> m_EffectsPlaceStart;
	
	[Attribute(category: "Effects")]
	protected ref array<ref SCR_BaseEditorEffect> m_EffectsPlaceConfirm;
	
	[Attribute(category: "Effects")]
	protected ref array<ref SCR_BaseEditorEffect> m_EffectsPlaceCancel;
	
	protected int m_iPrefabCount;
	protected ref array<int> m_aIndexes = {};
	
	array<ref SCR_BaseEditorEffect> GetEffectsPlaceStart()
	{
		return m_EffectsPlaceStart;
	}
	array<ref SCR_BaseEditorEffect> GetEffectsPlaceConfirm()
	{
		return m_EffectsPlaceConfirm;
	}
	array<ref SCR_BaseEditorEffect> GetEffectsPlaceCancel()
	{
		return m_EffectsPlaceCancel;
	}
	
	/*
	Get registered prefab with given index.
	\param index Prefab index from the registry
	\return Prefab path (empty when the index is invalid)
	*/
	ResourceName GetPrefab(int index)
	{
		for (int i = m_aIndexes.Count() - 1; i >= 0; i--)
		{
			int registryIndex = m_aIndexes[i];
			if (index >= registryIndex)
				return m_Registries[i].GetPrefabs()[index - registryIndex];
		}
		return ResourceName.Empty;
	}
	/*
	Get index of registered prefab
	\param prefab Prefab path
	\return Prefab index (-1 when the prefab is not found)
	*/
	int GetPrefabID(ResourceName prefab)
	{
		foreach (int i, SCR_PlaceableEntitiesRegistry registry: m_Registries)
		{
			int index = registry.GetPrefabs().Find(prefab);
			if (index != -1)
				return index + m_aIndexes[i];
		}
		return -1;
	}
	/*
	Get number of prefabs in the registry.
	\return Number of prefabs
	*/
	int CountPrefabs()
	{
		return m_iPrefabCount;
	}
	/*
	Get registered prefabs.
	\param[out] outPrefabs Array to be filled with prefabs
	\return Number of prefabs
	*/
	int GetPrefabs(out notnull array<ResourceName> outPrefabs, bool onlyExposed = false)
	{
		foreach (SCR_PlaceableEntitiesRegistry registry: m_Registries)
		{
			outPrefabs.InsertAll(registry.GetPrefabs(onlyExposed));
		}
		return outPrefabs.Count();
	}
	/*
	Check if given placing flag is allowed to be changed.
	\return True when the flag can be changed
	*/
	bool HasPlacingFlag(EEditorPlacingFlags flag)
	{
		return SCR_Enum.HasFlag(m_AllowedPlacingFlags, flag);
	}
	/*
	Get prefab used for testing.
	\return Prefab
	*/
	ResourceName GetTestPrefab()
	{
		return m_TestPrefab;
	}
	
	void SCR_PlacingEditorComponentClass(IEntityComponentSource componentSource, IEntitySource parentSource, IEntitySource prefabSource)
	{
		foreach (SCR_PlaceableEntitiesRegistry registry: m_Registries)
		{
			m_aIndexes.Insert(m_iPrefabCount);
			m_iPrefabCount += registry.GetPrefabs().Count();
		}
	}
};

/** @ingroup Editor_Components
*/
class SCR_PlacingEditorComponent : SCR_BaseEditorComponent
{
	[Attribute(defvalue: "10", category: "Placing", desc: "Spacing between entities placed for multiple recipients")]
	protected float m_fSpacing;
	
	private ResourceName m_SelectedPrefab;
	private SCR_StatesEditorComponent m_StatesManager;
	protected SCR_PreviewEntityEditorComponent m_PreviewManager;
	protected SCR_BudgetEditorComponent m_BudgetManager;
	protected SCR_EditableEntityCore m_editableEntityCore;
	
	private SCR_SiteSlotEntity m_Slot;
	private int m_iEntityIndex;
	private ref map<int, IEntity> m_WaitingPreviews = new map<int, IEntity>;
	private vector m_vFixedPosition;
	private float m_fPreviewAnimationProgress = -1;
	private EEditorPlacingFlags m_PlacingFlags;
	private EEditorPlacingFlags m_CompatiblePlacingFlags;
	private ref set<SCR_EditableEntityComponent> m_Recipients;
	protected ref SCR_EditorPreviewParams m_InstantPlacingParam;
	
	protected bool m_bBlockPlacing;
	
	private ref ScriptInvoker Event_OnSelectedPrefabChange = new ScriptInvoker;
	private ref ScriptInvoker Event_OnPlacingPlayerChange = new ScriptInvoker;
	private ref ScriptInvoker Event_OnPlacingFlagsChange = new ScriptInvoker;
	
	private ref ScriptInvoker Event_OnRequestEntity = new ScriptInvoker;
	private ref ScriptInvoker Event_OnPlaceEntity = new ScriptInvoker;
	
	private ref ScriptInvoker Event_OnPlaceEntityServer = new ScriptInvoker;
	
	protected static SCR_EditableEntityComponent m_DelayedSpawnEntity;
	protected static ref SCR_EditorPreviewParams m_DelayedSpawnPreviewParams;
			
	static protected ref map<EEditableEntityBudget, int> m_accumulatedBudgetChanges = new map<EEditableEntityBudget, int>;
	
	//Basically every time someone has GM rights a new entity with its own SCR_PlacingEditorComponent is used.
	//Each GM client has ownership of that entity and will use it to send RPC's to server
	//However when server handles the spawning of the entity instead of using the instance owned by the server
	//(in which variables in other components like the budget one have been initialized)
	//they use the entity owned by the client that sent the RPC
	//This is one of the many, many, many issues that GM has with replication but there is no time for a rewrite,
	//so instead we have this single static variable. Welp.
	static SCR_PlacingEditorComponent serverPlacingEditorComponent;
	static bool m_accumulatedBudgetChangesClearQueued = false;
	
	bool IsThereEnoughBudgetToSpawnVehicleOccupants(array<ResourceName> resources)
	{
		foreach (ResourceName prefab : resources)
		{
			Resource prefabResource = Resource.Load(prefab);
		
			if(!prefabResource)
				return false;
			
			IEntityComponentSource editableEntitySource = SCR_EditableEntityComponentClass.GetEditableEntitySource(prefabResource);
			
			if(!IsThereEnoughBudgetToSpawn(editableEntitySource))
				return false;
		}
		
		return true;
	}
	
	void SetPlacingBlocked(bool blocked)
	{
		m_bBlockPlacing = blocked;
	}
	
	bool IsPlacingBlocked()
	{
		return m_bBlockPlacing;
	}
	
	/*
	Get registered prefab with given index.
	\param index Prefab index from the registry
	\return Prefab path (empty when the index is invalid)
	*/
	int GetPrefabID(ResourceName prefab)
	{
		if (prefab.IsEmpty())
			return -1;
		
		SCR_PlacingEditorComponentClass prefabData = SCR_PlacingEditorComponentClass.Cast(GetEditorComponentData());
		if (!prefabData)
			return -1;
		
		return prefabData.GetPrefabID(prefab);
	}
	
	/*!
	Create entity exactly where the preview entity is.
	The entity is created from a prefab defined by SetSelectedPrefab().
	\param unselectPrefab True to unselect prefab after placing the entity
	\param canBePlayer True if the entity should be spawned as player
	\return True if the request was sent
	*/
	bool CreateEntity(bool unselectPrefab = true, bool canBePlayer = false, SCR_EditableEntityComponent holder = null)
	{
		//--- Check if placing is allowed. If not, prevent the operation and send a notification to player.
		ENotification notification = -1;
		if (!CanCreateEntity(notification))
		{
			if (notification != -1)
				SendNotification(notification);
			return false;
		}
		
		SCR_PlacingEditorComponentClass prefabData = SCR_PlacingEditorComponentClass.Cast(GetEditorComponentData());
		if (!prefabData || !m_SelectedPrefab) return false;
		
		if (!m_PreviewManager.IsChange() && !m_InstantPlacingParam)
		{
			SendNotification(ENotification.EDITOR_TRANSFORMING_INCORRECT_POSITION);
			//SetSelectedPrefab(ResourceName.Empty, true);
			return false;
		}
		
		int prefabID = prefabData.GetPrefabID(m_SelectedPrefab);
		if (prefabID == -1)
		{
			Print(string.Format("Cannot place prefab @\"%1\", it's not registered in placeable entities!", m_SelectedPrefab.GetPath()), LogLevel.WARNING);
			SetSelectedPrefab(ResourceName.Empty, true);
			return false;
		}
		
		//--- If the entity can be spawned as player, get player ID
		int playerID = GetManager().GetPlayerID();

		if (!canBePlayer)
			canBePlayer = HasPlacingFlag(EEditorPlacingFlags.CHARACTER_PLAYER);

		if (canBePlayer)
		{
			Resource prefabResource = Resource.Load(m_SelectedPrefab);
			IEntityComponentSource component = SCR_BaseContainerTools.FindComponentSource(prefabResource, SCR_EditableEntityComponent);
			if (component && SCR_EditableEntityComponentClass.GetEntityType(component) != EEditableEntityType.CHARACTER)
			{
				SendNotification(ENotification.EDITOR_PLACING_CANNOT_AS_PLAYER);
				SetSelectedPrefab(ResourceName.Empty, true);
				return false;
			}
		}
		
		//~ If placing with crew or passengers. Send notification if not enough budget
		if (SCR_Enum.HasFlag(m_PlacingFlags, EEditorPlacingFlags.VEHICLE_CREWED) ||SCR_Enum.HasFlag(m_PlacingFlags, EEditorPlacingFlags.VEHICLE_PASSENGER))
		{
			array<ref SCR_EntityBudgetValue> budgetCosts = {};
			Resource resource = Resource.Load(m_SelectedPrefab);
			if (!resource.IsValid())
			{
				Print("Cannot load prefab " + m_SelectedPrefab + " | " + FilePath.StripPath(__FILE__) + ":" + __LINE__, LogLevel.WARNING);
				return false;
			}

			IEntityComponentSource source = SCR_EditableEntityComponentClass.GetEditableEntitySource(resource.GetResource().ToEntitySource());
			m_BudgetManager.GetVehicleOccupiedBudgetCosts(source, m_PlacingFlags, budgetCosts, false);
			EEditableEntityBudget blockingBudget;
			
			if (!m_BudgetManager.CanPlace(budgetCosts, blockingBudget))
				SCR_NotificationsComponent.SendLocal(ENotification.EDITOR_PLACING_BUDGET_MAX_FOR_VEHICLE_OCCUPANTS);				
		}
		
		//--- Create packet
		SCR_EditorPreviewParams params;
		if (m_InstantPlacingParam)
		{
			params = m_InstantPlacingParam;
		}
		else
		{
			SCR_EditableEntityComponent parent;
			SCR_LayersEditorComponent layersManager = SCR_LayersEditorComponent.Cast(GetInstance(SCR_LayersEditorComponent));
			if (layersManager)
				parent = layersManager.GetCurrentLayer();
			
			params = SCR_EditorPreviewParams.CreateParamsFromPreview(m_PreviewManager, parent, true);
		}
		
		if (!params)
			return false;
		
		//--- Save placing flags
		params.m_PlacingFlags = m_PlacingFlags;
		
		//--- Copy the preview, it will remain visible until callback from server is received
		int simulatedDelay = DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_EDITOR_NETWORK_DELAY) * 100;
		m_iEntityIndex++;
		if (IsProxy() || simulatedDelay > 0)
			m_WaitingPreviews.Insert(m_iEntityIndex, m_PreviewManager.CreateWaitingPreview());
		
		array<RplId> recipientIds;
		if (m_Recipients)
		{
			recipientIds = {};
			foreach (SCR_EditableEntityComponent entity: m_Recipients)
			{
				RplId id = Replication.FindId(entity);
				if (id.IsValid())
					recipientIds.Insert(id);
			}
		}
		
		RplId holderId = null;
		if (holder)
		{
			holderId = Replication.FindId(holder);
			if (!holderId.IsValid())
				holderId = null;
		}
		
		//--- Send request to server
		m_StatesManager.SetIsWaiting(true);
		Event_OnRequestEntity.Invoke(prefabID, params.m_vTransform, null);
		
		if (simulatedDelay > 0 && !Replication.IsRunning())
			GetGame().GetCallqueue().CallLater(CreateEntityServer, simulatedDelay, false, params, prefabID, playerID, m_iEntityIndex, !unselectPrefab, recipientIds, canBePlayer, holderId);
		else
			Rpc(CreateEntityServer, params, prefabID, playerID, m_iEntityIndex, !unselectPrefab, recipientIds, canBePlayer, holderId);
		
		//--- ToDo: Fail the request if server didn't respond in given time
		//GetGame().GetCallqueue().CallLater(CreateEntityOwner, 10000, false, prefabID, -1);
		
		m_Slot = null;
		m_InstantPlacingParam = null;
		
		//--- Stop placing (only after packet was created)
		if (unselectPrefab || m_InstantPlacingParam)
			SetSelectedPrefab(ResourceName.Empty, true);
		
		return true;
	}
	/*!
	Create entity with custom params.
	\param prefab Entity prefab
	\param param Preview param
	\param unselectPrefab True to unselect prefab after placing the entity
	\param canBePlayer True if the entity should be spawned as player
	\param recipients Array of entities for whom new entities will be created (e.g., waypoints for groups)
	\param[in] holder Entity to which the spawned entity will be attached (used in waypoints and tasks)
	\return True if the request was sent
	*/
	bool CreateEntity(ResourceName prefab, SCR_EditorPreviewParams param, bool unselectPrefab = true, bool canBePlayer = false, set<SCR_EditableEntityComponent> recipients = null, SCR_EditableEntityComponent holder = null)
	{
		if (m_bBlockPlacing)
			return false;

		m_SelectedPrefab = prefab;
		m_Recipients = recipients;
		SetInstantPlacing(param);
		return CreateEntity(unselectPrefab, canBePlayer, holder);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void CreateEntityServer(SCR_EditorPreviewParams params, RplId prefabID, int playerID, int entityIndex, bool isQueue, array<RplId> recipientIds, bool canBePlayer, RplId holderId)
	{
		if (m_bBlockPlacing)
			return;

		SCR_PlacingEditorComponentClass prefabData = SCR_PlacingEditorComponentClass.Cast(GetEditorComponentData());
		if (RplSession.Mode() == RplMode.Client || !prefabData) return;
		
		array<RplId> entityIds = {};
		
		if (!CanCreateEntity() || !params.Deserialize())
		{
			Rpc(CreateEntityOwner, prefabID, entityIds, entityIndex, false, false, RplId.Invalid(), 0);
			return;
		}
		
		//--- Get prefab
		ResourceName prefab = prefabData.GetPrefab(prefabID);
		if (prefab.IsEmpty())
		{
			Print("Cannot create entity, prefab not defined!", LogLevel.ERROR);
			Rpc(CreateEntityOwner, prefabID, entityIds, entityIndex, false, false, RplId.Invalid(), 0);
			return;
		}
		
		//~ Get variant if any
		prefab = SCR_EditableEntityComponentClass.GetRandomVariant(prefab);
		
		Resource prefabResource = Resource.Load(prefab);
		if (!prefabResource || !prefabResource.IsValid())
		{
			Print(string.Format("Cannot create entity, error when loading prefab '%1'!", prefab), LogLevel.ERROR);
			Rpc(CreateEntityOwner, prefabID, entityIds, entityIndex, false, false, RplId.Invalid(), 0);
			return;
		}
		IEntityComponentSource editableEntitySource = SCR_EditableEntityComponentClass.GetEditableEntitySource(prefabResource);
		if (!editableEntitySource)
		{
			Print(string.Format("Cannot create entity, prefab '%1' does not contain SCR_EditableEntityComponent!", prefab), LogLevel.ERROR);
			Rpc(CreateEntityOwner, prefabID, entityIds, entityIndex, false, false, RplId.Invalid(), 0);
			return;
		}
		
		EEditableEntityBudget blockingBudget;
		if (!CanPlaceEntityServer(editableEntitySource, blockingBudget, false, false))
		{
			Print(string.Format("Entity budget exceeded for player!"), LogLevel.ERROR);
			Rpc(CreateEntityOwner, prefabID, entityIds, entityIndex, false, false, RplId.Invalid(), 0);
			return;
		}
		
		//always allow player spawning
		if(!canBePlayer)
		{
			//server one should check if possible or not. Check variable description if you are confused
			bool canSpawn = IsThereEnoughBudgetToSpawn(editableEntitySource);
		
			if(!canSpawn)
				return;
		}

		OnBeforeEntityCreatedServer(prefab);
		
		SCR_EditableEntityComponent entity;
		bool hasRecipients = false;
		RplId currentLayerID;
		array<SCR_EditableEntityComponent> entities = {};
		if (recipientIds && !recipientIds.IsEmpty())
		{
			//--- Spawn entity for selected entities (e.g., waypoints for groups)
			array<SCR_EditableEntityComponent> recipients = {};
			foreach (RplId id: recipientIds)
			{
				SCR_EditableEntityComponent recipient = SCR_EditableEntityComponent.Cast(Replication.FindItem(id));
				if (recipient)
					recipients.Insert(recipient);
			}
			array<vector> offsets = GetOffsets(recipients.Count());
			array<ref array<int>> distances = {}; 
			
			for (int i = 0; i < recipients.Count(); i++)
			{
				array<int> distToPosition = {};
				vector position;
				recipients[i].GetPos(position);
				for (int j = 0; j < offsets.Count(); j++)
				{
					//precision to meters and 46km as max distance should be enough
					distToPosition.Insert(Math.Clamp(vector.DistanceSq(position, offsets[j]), 0, int.MAX));
				}
				distances.Insert(distToPosition);
			}
			
			// we try to minimize distances from groups to waypoints
			// in general it produces paths with less scrossings between them
			array<int> permutation = {};
			AssignmentSolver.Solve(distances, permutation);

			foreach (int i, SCR_EditableEntityComponent recipient: recipients)
			{
				params.m_Offset = offsets[permutation[i]];
				entity = SpawnEntityResource(params, prefabResource, playerID, isQueue, recipient, canBePlayer);
				
				entity.SetAuthor(playerID);
				
				entities.Insert(entity);
				entityIds.Insert(Replication.FindId(entity));
			}
			hasRecipients = true;
			currentLayerID = Replication.FindId(recipients[0].GetParentEntity());
		}
		else
		{
			//--- Spawn stand-alone entity
			entity = SpawnEntityResource(params, prefabResource, playerID, isQueue, null, canBePlayer);
			
			entity.SetAuthor(playerID);
			
			entities.Insert(entity);
			entityIds.Insert(Replication.FindId(entity));
			currentLayerID = Replication.FindId(params.m_CurrentLayer);
		}
		
		//++ Attach if there is an attacher
		if (holderId)
		{
			SCR_EditableEntityComponent holder = SCR_EditableEntityComponent.Cast(Replication.FindItem(holderId));
			if (holder)
			{
				foreach(SCR_EditableEntityComponent ent : entities)
				{
					ent.SetParentEntity(holder);
				}
			}
		}
		
		entity.OnCreatedServer(this);
		OnEntityCreatedServer(entities);
		
		Rpc(CreateEntityOwner, prefabID, entityIds, entityIndex, isQueue, hasRecipients, currentLayerID, 0);
		Event_OnPlaceEntityServer.Invoke(prefabID, entity, playerID);
	}
	
	bool IsThereEnoughBudgetToSpawn(IEntityComponentSource entitySource)
	{
		if(!entitySource)
			return false;
		
		if (!m_BudgetManager)
		{
			m_BudgetManager = SCR_BudgetEditorComponent.Cast(FindEditorComponent(SCR_BudgetEditorComponent, false, true));
			if (!m_BudgetManager)
				return true;
		}
	
		array<ref SCR_EntityBudgetValue> budgetCosts = {};
		map<EEditableEntityBudget, int> tempBudgetAggregation = m_accumulatedBudgetChanges;
		SCR_EditableEntityCoreBudgetSetting budgetSettings;

		m_BudgetManager.GetEntitySourcePreviewBudgetCosts(entitySource, budgetCosts);	

		bool budgetFound = false;
		foreach(SCR_EntityBudgetValue budget : budgetCosts)
		{
			const EEditableEntityBudget budgetType = budget.GetBudgetType();
			int accumulatedBudgetCost = 0;
			int maxBudgetForType = 0;
			
			bool found = tempBudgetAggregation.Find(budgetType, accumulatedBudgetCost);
			m_BudgetManager.GetMaxBudgetValue(budgetType, maxBudgetForType);

			bool hasBudget = m_editableEntityCore.GetBudget(budgetType, budgetSettings);
			int currentBudgetValue = 0;

			if(hasBudget)
				currentBudgetValue = budgetSettings.GetCurrentBudget();
			
			//We never take reserved budget into account, is this intentional?
			//const int reservedBudget = budgetSettings.GetReservedBudget();
			
			//if same budget, we aggregate them
			const int newAggregatedBudget = budget.GetBudgetValue() + accumulatedBudgetCost;
			const int estimatedBudget = newAggregatedBudget + currentBudgetValue;
			
			//check if budget goes over max
			if(estimatedBudget > maxBudgetForType)
			{	
				EEditableEntityBudget blockingBudget;
				bool canPlace = m_BudgetManager.CanPlaceEntitySource(entitySource, blockingBudget, false, false);

				return false;
			}
			//if all budgets are valid, we will update accumulated budgets and return true	
			tempBudgetAggregation[budgetType] = newAggregatedBudget;
		}
		
		m_accumulatedBudgetChanges = tempBudgetAggregation;
		
		if(!m_accumulatedBudgetChangesClearQueued)
		{
			GetGame().GetCallqueue().CallLater(ClearAccumulatedBudgetChanges);
			m_accumulatedBudgetChangesClearQueued = true;
		}
		
		return true;
	}
	
	protected static void ClearAccumulatedBudgetChanges()
	{
		m_accumulatedBudgetChanges.Clear();
		m_accumulatedBudgetChangesClearQueued = false;
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void CreateEntityOwner(int prefabID, array<RplId> entityIds, int entityIndex, int isQueue, bool hasRecipients, RplId currentLayerID, int attempt)
	{
		//~ Remove placing flag player
		if (HasPlacingFlag(EEditorPlacingFlags.CHARACTER_PLAYER))
			SetPlacingFlag(EEditorPlacingFlags.CHARACTER_PLAYER, false);
		
		//~ Todo: Make sure crew and Passenger que placing and placing again budget is updated correctly. 
		//~ Now if placing flag is active on start placing the budget is not correct. Same goes for queing
		if (HasPlacingFlag(EEditorPlacingFlags.VEHICLE_CREWED))
			SetPlacingFlag(EEditorPlacingFlags.VEHICLE_CREWED, false);
		if (HasPlacingFlag(EEditorPlacingFlags.VEHICLE_PASSENGER))
			SetPlacingFlag(EEditorPlacingFlags.VEHICLE_PASSENGER, false);
		
		//--- Delete the ghost preview which was waiting for server callback
		IEntity waitingPreview;
		if (m_WaitingPreviews.Find(entityIndex, waitingPreview))
		{
			m_WaitingPreviews.Remove(entityIndex);
			delete waitingPreview;
		}
		
		if (m_StatesManager && !m_StatesManager.SetIsWaiting(false))
		{
			SetSelectedPrefab();
			return;
		}
		
		set<SCR_EditableEntityComponent> entities = new set<SCR_EditableEntityComponent>();
		SCR_EditableEntityComponent entity;
		for (int i, count = entityIds.Count(); i < count; i++)
		{
			entity = SCR_EditableEntityComponent.Cast(Replication.FindItem(entityIds[i]));
			if (entity)
				entities.Insert(entity);
		}
		
		//--- Wait for entities to be streamed in
		if (entities.IsEmpty())
		{
			if (attempt < 30)
			{
				GetGame().GetCallqueue().CallLater(CreateEntityOwner, 1, false, prefabID, entityIds, entityIndex, isQueue, hasRecipients, currentLayerID, attempt + 1);
			}
			else
			{
				SCR_PlacingEditorComponentClass prefabData = SCR_PlacingEditorComponentClass.Cast(GetEditorComponentData());
				Print(string.Format("Error when creating entity from prefab '%1' (id = %2)!", prefabData.GetPrefab(prefabID), prefabID), LogLevel.ERROR);
			}

			return;
		}
		
		//--- Select placed entities
		SCR_BaseEditableEntityFilter selectedFilter = SCR_BaseEditableEntityFilter.GetInstance(EEditableEntityState.SELECTED);
		if (selectedFilter)
			selectedFilter.Replace(entities);
		
		//--- Set current layer to be the parent of newly created entity, to make sure its icon is visible
		SCR_EditableEntityComponent currentLayer = SCR_EditableEntityComponent.Cast(Replication.FindItem(currentLayerID));
		if (!currentLayerID.IsValid() || currentLayer)
		{
			SCR_LayersEditorComponent layerManager = SCR_LayersEditorComponent.Cast(SCR_LayersEditorComponent.GetInstance(SCR_LayersEditorComponent));
			if (layerManager)
				layerManager.SetCurrentLayer(currentLayer);
		}
		
		//--- Delayed check if entity is within budget (character budget update is delayed)
		if (isQueue)
		{
			GetGame().GetCallqueue().CallLater(CheckBudgetOwner, 100, false);
		}
		else
		{
			m_BudgetManager.ResetPreviewCost();
		}
		
		//--- Call event
		Event_OnPlaceEntity.Invoke(prefabID, entity);
		
		//--- Activate effects
		vector pos;
		entities[0].GetPos(pos);
		
		SCR_PlacingEditorComponentClass prefabData = SCR_PlacingEditorComponentClass.Cast(GetEditorComponentData());
		if (prefabData) SCR_BaseEditorEffect.Activate(prefabData.GetEffectsPlaceConfirm(), this, pos, entities);
	}
	/*!
	Check if the entity can be created.
	Evaluated both on client an on server (notification is used only on client though)
	To be overridden by inherited classes.
	\param[out] outNotification Notification to be sent when attempting to place the entity
	\return True to allow placing, false to prevent it
	*/
	bool CanCreateEntity(out ENotification outNotification = -1, inout SCR_EPreviewState previewStateToShow = SCR_EPreviewState.PLACEABLE)
	{
		return !m_bBlockPlacing;
	}
	/*!
	Function called right before entities are created on server.
	To be overloaded by inherited classes.
	\param prefab Prefab that entities to be created will use
	*/
	protected void OnBeforeEntityCreatedServer(ResourceName prefab);
	/*!
	Function called when entities are created on server.
	To be overloaded by inherited classes.
	\param entities Array of created entities
	*/
	protected void OnEntityCreatedServer(array<SCR_EditableEntityComponent> entities);
	
	protected void CheckBudgetOwner()
	{
		EEditableEntityBudget blockingBudget;
		if (!CanSelectEntityPrefab(m_SelectedPrefab, blockingBudget, true))
		{
			m_BudgetManager.ResetPreviewCost();
			SetSelectedPrefab(ResourceName.Empty, true);
		}
	}
	
	protected void OnBudgetMaxReached(EEditableEntityBudget entityBudget, bool maxReached)
	{
		if (maxReached)
			SetSelectedPrefab(ResourceName.Empty, true);
	}
	
	/*!
	Create entity from prefab resource.
	\param prefab Entity prefab
	\param transform Spawning transformation matrix
	\return Editable entity
	*/
	static SCR_EditableEntityComponent SpawnEntityResource(ResourceName prefab, vector transform[4])
	{
		return SpawnEntityResource(SCR_EditorPreviewParams.CreateParams(transform), Resource.Load(prefab));
	}
	/*!
	Create entity with given params.
	\param params Spawning params
	\prefabResource Prefab which will be spawned
	\playerID Id of entity requested spawning.
	\recipient Currently selected entity for whom the new entity is added (e.g., new waypoint for a group)
	\canBePlayer When true, spawned entity will become the player
	\return Editable entity
	*/
	static SCR_EditableEntityComponent SpawnEntityResource(SCR_EditorPreviewParams params, Resource prefabResource, int playerID = 0, bool isQueue = false, SCR_EditableEntityComponent recipient = null, bool canBePlayer = false)
	{		
		if (Replication.IsClient() || !prefabResource|| !prefabResource.IsValid())
			return null;
		
		//--- When spawning a player, get their respawn component first
		EEditorPlacingFlags compatiblePlacingFlags = GetCompatiblePlacingFlags(prefabResource);
		SCR_RespawnComponent respawnComponent;
		if (canBePlayer)
		{
			respawnComponent = SCR_RespawnComponent.Cast(GetGame().GetPlayerManager().GetPlayerRespawnComponent(playerID));
			if (!respawnComponent)
				return null;
		}
		
		IEntity owner;
		if (params.m_TargetInteraction == EEditableEntityInteraction.SLOT)
		{
			//--- Create in slot
			GenericEntity targetOwner = params.m_Target.GetOwner();
			SCR_SiteSlotEntity slot = SCR_SiteSlotEntity.Cast(targetOwner);
			if (slot)
			{
				//--- Use special slot function
				owner = slot.SpawnEntityInSlot(prefabResource, Math3D.MatrixToAngles(params.m_vTransform)[0], -1/*verticalMode*/);
			}
			else
			{
				//--- Position on the target
				EntitySpawnParams spawnParams = new EntitySpawnParams();
				spawnParams.TransformMode = ETransformMode.WORLD;
				targetOwner.GetWorldTransform(spawnParams.Transform);
				owner = GetGame().SpawnEntityPrefab(prefabResource, GetGame().GetWorld(), spawnParams);
			}
		}
		else
		{
			//--- Place freely
			//SCR_AIGroup.IgnoreSnapToTerrain(true);
			EntitySpawnParams spawnParams = new EntitySpawnParams();
			spawnParams.TransformMode = ETransformMode.WORLD;
			params.GetWorldTransform(spawnParams.Transform);
			owner = GetGame().SpawnEntityPrefab(prefabResource, GetGame().GetWorld(), spawnParams);
			
			if (respawnComponent && SCR_Enum.HasFlag(compatiblePlacingFlags, EEditorPlacingFlags.CHARACTER_PLAYER))
			{
				SCR_PossessSpawnData spawnData = SCR_PossessSpawnData.FromEntity(owner);
				spawnData.SetSkipPreload(true);
				if (!respawnComponent.RequestSpawn(spawnData))
					Print(string.Format("@\"%1\" control cannot be given to playerID=%2, error in RequestSpawn!", prefabResource.GetResource().GetResourceName().GetPath(), playerID), LogLevel.ERROR);
			}
		}
		
		//--- Initialize
		SCR_EditableEntityComponent entity = SCR_EditableEntityComponent.GetEditableEntity(owner);
		if (!entity)
			return null;

		SCR_EditableCommentComponent comment = SCR_EditableCommentComponent.Cast(params.m_Target);
		SCR_CompositionSlotManagerComponent slotManager = SCR_CompositionSlotManagerComponent.GetInstance();
		if (comment && slotManager)
			slotManager.SetOccupant(comment.GetOwnerScripted(), owner);

		//--- Assign faction when placed for a recipient
		if (recipient)
			SCR_FactionAffiliationComponent.SetFaction(entity.GetOwner(), recipient.GetFaction());
		
		//--- Call custom event
		SCR_EditableEntityComponent childEntity = entity.EOnEditorPlace(params.m_Parent, recipient, params.m_PlacingFlags, isQueue, playerID);
		
		//--- Set parent to current layer
		if (params.m_Parent)
		{		
			//~ Force entity as passenger
			if (params.m_TargetInteraction == EEditableEntityInteraction.PASSENGER)
			{
				//~ Forces entities to be spawned into passenger positions
				SCR_EditableVehicleComponent vehicle = SCR_EditableVehicleComponent.Cast(params.m_Parent);
				if (vehicle)
					entity.ForceVehicleCompartments(SCR_BaseCompartmentManagerComponent.PASSENGER_COMPARTMENT_TYPES);
			}
			//~ Not forced passenger set parent
			else 
			{
				childEntity.SetParentEntity(params.m_Parent);
			}
		}
		
		//--- New parent was created in EOnEditorPlace, reflect that
		if (childEntity != entity)
			params.m_Parent = childEntity;
		
		//--- Decide which layer should be set as current
		params.m_CurrentLayer = childEntity.GetParentEntity();
		
		//--- Original entity deleted in SetParentEntity, use parent as the entity (e.g., when placing a group inside a group)
		if (!owner)
			entity = params.m_Parent;
	
		//--- Orient according to vertical settings
		if (!entity.HasEntityFlag(EEditableEntityFlag.STATIC_POSITION))
		{
			SCR_EditableGroupComponent groupComp = SCR_EditableGroupComponent.Cast(entity);
			
			if (groupComp)
			{
				//GetGame().GetCallqueue().CallLater(SCR_RefPreviewEntity.SpawnAndApplyReference, 500, false, entity, params);
				
				SCR_AIGroup group = SCR_AIGroup.Cast(groupComp.GetOwnerScripted());
				if (group)
				{
					m_DelayedSpawnEntity = entity;
					m_DelayedSpawnPreviewParams = params;
					group.GetOnAllDelayedEntitySpawned().Insert(OnAIGroupAllEntitiesSpawned);
				}
			}
			else
			{
				SCR_RefPreviewEntity.SpawnAndApplyReference(entity, params);
			}
		}
		
		//--- Log message. Important for identifying problematic prefabs!
		vector logTransform[4];
		if (owner)
			owner.GetWorldTransform(logTransform);
		else
			params.GetWorldTransform(logTransform);
		
		if (recipient)
			Print(string.Format("@\"%1\" placed for %3 at %2", prefabResource.GetResource().GetResourceName().GetPath(), logTransform, recipient.GetDisplayName()), LogLevel.VERBOSE);
		else if (prefabResource)
			Print(string.Format("@\"%1\" placed at %2", prefabResource.GetResource().GetResourceName().GetPath(), logTransform), LogLevel.VERBOSE);
		else 
			Print(string.Format("@\"%1\" placed at %2", "Entity", logTransform), LogLevel.VERBOSE);	
		
		return entity;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Callback reacting to all delayed spawned entities being created
	protected static void OnAIGroupAllEntitiesSpawned(SCR_AIGroup group)
	{
		SCR_RefPreviewEntity.SpawnAndApplyReference(m_DelayedSpawnEntity, m_DelayedSpawnPreviewParams);
		
		// Cleanup 
		m_DelayedSpawnEntity = null;
		m_DelayedSpawnPreviewParams = null;
		group.GetOnAllDelayedEntitySpawned().Remove(OnAIGroupAllEntitiesSpawned);
	}
	
	protected static EEditorPlacingFlags GetCompatiblePlacingFlags(Resource prefabResource)
	{
		IEntityComponentSource component = SCR_EditableEntityComponentClass.GetEditableEntitySource(prefabResource);
		
		switch (SCR_EditableEntityComponentClass.GetEntityType(component))
		{
			case EEditableEntityType.CHARACTER:
				return EEditorPlacingFlags.CHARACTER_PLAYER;
			
			//~ Check if can spawn occupants within vehicle
			case EEditableEntityType.VEHICLE:
			{
				SCR_EditableVehicleUIInfo uiInfo = SCR_EditableVehicleUIInfo.Cast(SCR_EditableEntityComponentClass.GetInfo(component));
				if (!uiInfo)
					return 0;
				
				EEditorPlacingFlags vehiclePlacingFlags = 0; 
				
				//~ Check if can spawn with crew
				if (uiInfo.CanFillWithCrew())
					vehiclePlacingFlags |= EEditorPlacingFlags.VEHICLE_CREWED;
				
				//~ Check if can spawn with passengers
				if (uiInfo.CanFillWithPassengers())
					vehiclePlacingFlags |= EEditorPlacingFlags.VEHICLE_PASSENGER;
				
				return vehiclePlacingFlags;
			}
				
			case EEditableEntityType.TASK:
				return EEditorPlacingFlags.TASK_INACTIVE;
		}
		return 0;
	}
	
	protected bool CanPlaceEntityServer(IEntityComponentSource editableEntitySource, out EEditableEntityBudget blockingBudget, bool updatePreview, bool showNotification)
	{
		if (!m_BudgetManager)
		{
			m_BudgetManager = SCR_BudgetEditorComponent.Cast(FindEditorComponent(SCR_BudgetEditorComponent, false, true));
			if (!m_BudgetManager)
				return true;
		}
		
		return m_BudgetManager.CanPlaceEntitySource(editableEntitySource, blockingBudget, HasPlacingFlag(EEditorPlacingFlags.CHARACTER_PLAYER), updatePreview, showNotification);
	}
	
 	protected bool CanSelectEntityPrefab(ResourceName prefab, out EEditableEntityBudget blockingBudget, bool updatePreview = true, bool showBudgetMaxNotification = true)
	{
		if (!m_BudgetManager || prefab.IsEmpty()) 
			return false;
		
		// Load prefab to check entity budgets or entity type
		Resource entityPrefab = Resource.Load(prefab);
		// Get entity source
		IEntitySource entitySource = SCR_BaseContainerTools.FindEntitySource(entityPrefab);
		IEntityComponentSource editableEntitySource = SCR_EditableEntityComponentClass.GetEditableEntitySource(entitySource);
		
		return CanPlaceEntityServer(editableEntitySource, blockingBudget, updatePreview, showBudgetMaxNotification);
	}
	
	protected array<vector> GetOffsets(int count)
	{
		array<vector> result = {};
		if (count == 0)
			return result;
		
		if (count == 1)
		{
			result.Insert(vector.Zero);
			return result;
		}
		
		int rowCount = Math.Round(Math.Sqrt(count));
		int columnCount = Math.Ceil(Math.Sqrt(count));
		vector offset = -Vector((rowCount - 1) * m_fSpacing / 2, 0, (columnCount - 1) * m_fSpacing / 2);

		int row, column;
		for (int i = 0; i < count; i++)
		{
			row = Math.Floor(i / columnCount);
			column = i % columnCount;
			result.Insert(offset + Vector(row * m_fSpacing, 0, column * m_fSpacing));
		}
		return result;
	}
	protected void OnEntityUnregistered(SCR_EditableEntityComponent entity)
	{
		int index = m_Recipients.Find(entity);
		if (index >= 0)
		{
			m_Recipients.Remove(index);
			
			//--- No remaining recipients, stop placing
			if (m_Recipients.IsEmpty())
				SetSelectedPrefab();
		}
	}
	
	/*!
	Set instant placing parameters.
	When defined, selected entity will be placed immediatelly.
	\param param Placing param
	*/
	void SetInstantPlacing(SCR_EditorPreviewParams param)
	{
		m_InstantPlacingParam = param;
	}
	
	/*!
	Set currently placed prefab.
	\param prefab Prefab. Must be registered!
	\return true when prefab can be selected (fits budget + prefab data available)
	*/
	bool SetSelectedPrefab(ResourceName prefab = "", bool onConfirm = false, bool showBudgetMaxNotification = true, set<SCR_EditableEntityComponent> recipients = null)
	{
		if (prefab == m_SelectedPrefab) return true;

		//--- Check if entity is within budget
		EEditableEntityBudget blockingBudget;
		if (!prefab.IsEmpty() && !CanSelectEntityPrefab(prefab, blockingBudget, showBudgetMaxNotification))
		{
			return false;
		}
		
		//--- Place instantly
		if (m_InstantPlacingParam && !prefab.IsEmpty())
		{
			//--- This will prevent m_StatesManager.SetIsWaiting(false) from failing in CreateEntityOwner()
			m_StatesManager.SetSafeDialog(true);
			m_SelectedPrefab = prefab;
			m_Recipients = recipients;
			CreateEntity();
			return true;
		}
		
		//--- Check if the prefab is registered
		SCR_PlacingEditorComponentClass prefabData = SCR_PlacingEditorComponentClass.Cast(GetEditorComponentData());
		if (!prefab.IsEmpty() && prefabData.GetPrefabID(prefab) == -1)
		{
			Print(string.Format("Cannot initiate placing of prefab @\"%1\", it's not registered in placeable entities!", prefab), LogLevel.WARNING);
			return false;
		}
		
		if (prefab.IsEmpty() || HasPlacingFlag(EEditorPlacingFlags.CHARACTER_PLAYER))
		{
			m_BudgetManager.ResetPreviewCost();
		}
		
		if (m_StatesManager)
		{
			if (prefab)
			{
				if (!m_StatesManager.SetState(EEditorState.PLACING)) return false; //--- Exit when cannot set placing state
			}
			else
			{
				m_StatesManager.UnsetState(EEditorState.PLACING);
			}
		}
		
		//--- Track if recipients are deleted (stop placing when no recipient remains)
		if (prefab && recipients)
		{
			m_editableEntityCore.Event_OnEntityUnregistered.Insert(OnEntityUnregistered);
		}
		else if (!prefab && m_Recipients)
		{
			m_editableEntityCore.Event_OnEntityUnregistered.Remove(OnEntityUnregistered);
		}
		
		//--- Set the prefab
		ResourceName prefabPrev = m_SelectedPrefab;
		m_SelectedPrefab = prefab;
		m_Recipients = recipients;
		
		//--- Create preview entity
		int instanceCount = 1;
		if (recipients) instanceCount = recipients.Count();
		IEntity previewEntity = m_PreviewManager.CreatePreview(prefab, GetOffsets(instanceCount));//, fixedTransform);
		
		if (previewEntity)
		{
			//--- Teleport camera to fixed position
			if (m_PreviewManager.IsFixedPosition())
			{
				SCR_ManualCamera camera = SCR_CameraEditorComponent.GetCameraInstance();
				if (camera)
				{
					SCR_TeleportToCursorManualCameraComponent teleportComponent = SCR_TeleportToCursorManualCameraComponent.Cast(camera.FindCameraComponent(SCR_TeleportToCursorManualCameraComponent));
					if (teleportComponent) teleportComponent.TeleportCamera(previewEntity.GetOrigin(), true, true);
				}
			}
			m_CompatiblePlacingFlags = GetCompatiblePlacingFlags(Resource.Load(m_SelectedPrefab));
		}
		else
		{
			SetPlacingFlag(EEditorPlacingFlags.CHARACTER_PLAYER, false);
		}
		
		//--- Call event handlers
		Event_OnSelectedPrefabChange.Invoke(prefab, prefabPrev);
		
		if (prefab && !prefabPrev)
			SCR_BaseEditorEffect.Activate(prefabData.GetEffectsPlaceStart(), this);
		else if (!prefab && prefabPrev && !onConfirm)
			SCR_BaseEditorEffect.Activate(prefabData.GetEffectsPlaceCancel(), this);
		
		return true;
	}
	/*!
	Get currently placed prefab.
	\return prefab
	*/
	ResourceName GetSelectedPrefab()
	{
		return m_SelectedPrefab;
	}
	bool IsPlacing()
	{
		return !m_SelectedPrefab.IsEmpty();
	}
	/*!
	Attach plcing preview to a slot.
	\param slot Slot entity
	*/
	void SetSlot(SCR_SiteSlotEntity slot)
	{
		m_Slot = slot;
	}
	/*!
	Get slot which placing preview is currently attached to.
	\return Slot entity
	*/
	SCR_SiteSlotEntity GetSlot()
	{
		return m_Slot;
	}
	/*!
	Set value of placing flag.
	\param Placing flag
	\param toAdd True to set the flag, false to unset it
	*/
	void SetPlacingFlag(EEditorPlacingFlags flag, bool toAdd)
	{
		//--- When enabling, check if it's allowed (disabloing is always possible)
		if (toAdd && !IsPlacingFlagAllowed(flag))
		{
			Print(string.Format("Cannot enable placing flag %1, placing manager does not allow it!", typename.EnumToString(EEditorPlacingFlags, flag)), LogLevel.WARNING);
			return;
		}
		
		//--- Set the state
		EEditorPlacingFlags prevPlacingFlag = m_PlacingFlags;
		if (toAdd)
			m_PlacingFlags |= flag;
		else
			m_PlacingFlags &= ~flag;
		
		//--- If there was a change, call event
		if (m_PlacingFlags != prevPlacingFlag)
			Event_OnPlacingFlagsChange.Invoke(flag, toAdd);
		
		//~ Exception - update budget preview when placing as player or placing occupied vehicle
		UpdatePlacingFlagBudget(m_SelectedPrefab, flag, m_PlacingFlags, prevPlacingFlag);
	}
	
	/*!
	Update budget with given placing flags
	\param selectedPrefab Current selected prefab resource
	\param flagChanged Flag that was changed
	\param currentPlacingFlag Current placing flags
	\param prevPlacingFlag The placing flags that where active before new flags are set
	*/
	protected void UpdatePlacingFlagBudget(ResourceName selectedPrefab, EEditorPlacingFlags flagChanged, EEditorPlacingFlags currentPlacingFlag, EEditorPlacingFlags prevPlacingFlag)
	{		
		if (m_SelectedPrefab.IsEmpty())
			return;
		
		if (flagChanged == EEditorPlacingFlags.CHARACTER_PLAYER)
		{
			bool isPlacingPlayer = SCR_Enum.HasFlag(currentPlacingFlag, EEditorPlacingFlags.CHARACTER_PLAYER);
			EEditableEntityBudget blockingBudget;
			Resource resource = Resource.Load(selectedPrefab);
			if (!resource.IsValid())
			{
				Print("Cannot load " + selectedPrefab + " | " + FilePath.StripPath(__FILE__) + ":" + __LINE__, LogLevel.WARNING);
				return;
			}

			m_BudgetManager.CanPlaceEntitySource(SCR_EditableEntityComponentClass.GetEditableEntitySource(resource.GetResource().ToEntitySource()), blockingBudget, isPlacingPlayer);
		}
		
		if ((currentPlacingFlag & EEditorPlacingFlags.VEHICLE_CREWED || currentPlacingFlag & EEditorPlacingFlags.VEHICLE_PASSENGER) || (prevPlacingFlag & EEditorPlacingFlags.VEHICLE_CREWED || prevPlacingFlag & EEditorPlacingFlags.VEHICLE_PASSENGER))
		{
			array<ref SCR_EntityBudgetValue> budgetCosts = {};
			Resource resource = Resource.Load(selectedPrefab);
			if ( !resource.IsValid())
			{
				Print("Cannot load " + selectedPrefab + " | " + FilePath.StripPath(__FILE__) + ":" + __LINE__, LogLevel.WARNING);
				return;
			}

			IEntityComponentSource source = SCR_EditableEntityComponentClass.GetEditableEntitySource(resource.GetResource().ToEntitySource());
			m_BudgetManager.GetVehicleOccupiedBudgetCosts(source, currentPlacingFlag, budgetCosts);
			m_BudgetManager.UpdatePreviewCost(budgetCosts);		
		}
	}
	
	/*!
	Toggle value of placing flag.
	\param flag Placing flag
	*/
	void TogglePlacingFlag(EEditorPlacingFlags flag)
	{
		SetPlacingFlag(flag, !HasPlacingFlag(flag));
	}
	/*!
	Check if placing flag is active.
	\param flag Placing flag
	\return True when active
	*/
	bool HasPlacingFlag(EEditorPlacingFlags flag)
	{
		return SCR_Enum.HasFlag(m_PlacingFlags, flag);
	}
	/*!
	Check if placing flag is compatible with currently placed entity.
	\param flag Placing flag
	\return True when compatible
	*/
	bool IsPlacingFlagCompatible(EEditorPlacingFlags flag)
	{
		return SCR_Enum.HasFlag(m_CompatiblePlacingFlags, flag);
	}
	/*!
	Check if placing flag is allowed in placing manager settings.
	\param flag Placing flag
	\return True when allowed
	*/
	bool IsPlacingFlagAllowed(EEditorPlacingFlags flag)
	{
		SCR_PlacingEditorComponentClass prefabData = SCR_PlacingEditorComponentClass.Cast(GetEditorComponentData());
		return prefabData && prefabData.HasPlacingFlag(flag);
	}
	/*!
	Get invoker called when selected prefab changes.
	\return Script invoker
	*/
	ScriptInvoker GetOnSelectedPrefabChange()
	{
		return Event_OnSelectedPrefabChange;
	}
	/*!
	Get event called when placing flags change.
	Called only for editor user.
	\return Script invoker
	*/
	ScriptInvoker GetOnPlacingFlagsChange()
	{
		return Event_OnPlacingFlagsChange;
	}
	/*!
	Get event called when request for placing an entity is sent to server.
	Called only for editor user.
	\return Script invoker
	*/
	ScriptInvoker GetOnRequestEntity()
	{
		return Event_OnRequestEntity;
	}
	/*!
	Get event called when an entity is placed.
	Called only for editor user.
	\return Script invoker
	*/
	ScriptInvoker GetOnPlaceEntity()
	{
		return Event_OnPlaceEntity;
	}
	
	ScriptInvoker GetOnPlaceEntityServer()
	{
		return Event_OnPlaceEntityServer;
	}
		
	override void EOnEditorDebug(array<string> debugTexts)
	{
		if (!IsActive()) return;
		if (m_SelectedPrefab)
		{
			debugTexts.Insert(string.Format("Placing prefab: %1", FilePath.StripPath(m_SelectedPrefab.GetPath())));
		}
		else
		{
			debugTexts.Insert("Placing prefab: N/A");
		}
		debugTexts.Insert(string.Format("Placing flags: %1", SCR_Enum.FlagsToString(EEditorPlacingFlags, m_PlacingFlags)));
	}
	
	override protected void EOnEditorInitServer()
	{
		super.EOnEditorInitServer();
		
		m_BudgetManager = SCR_BudgetEditorComponent.Cast(FindEditorComponent(SCR_BudgetEditorComponent, false, true));
		m_editableEntityCore = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		
		if(!IsMaster())
			return;
		
		/*
		if(m_editableEntityCore)
		{
			m_editableEntityCore.Event_OnEntityBudgetUpdated.Insert(OnBudgetsUpdated);
		}
		
		serverPlacingEditorComponent = this;
		*/
	}
	
	override protected void EOnEditorDeleteServer()
	{
		super.EOnEditorDeleteServer();
		
		/*
		if(!IsMaster())
			return;
		
		if(m_editableEntityCore)
		{
			m_editableEntityCore.Event_OnEntityBudgetUpdated.Remove(OnBudgetsUpdated);
		}
		*/
	}
	
	override void EOnEditorActivate()
	{
		m_StatesManager = SCR_StatesEditorComponent.Cast(FindEditorComponent(SCR_StatesEditorComponent));
		m_PreviewManager = SCR_PreviewEntityEditorComponent.Cast(FindEditorComponent(SCR_PreviewEntityEditorComponent, true));
		m_BudgetManager = SCR_BudgetEditorComponent.Cast(FindEditorComponent(SCR_BudgetEditorComponent, false, true));
		m_editableEntityCore = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		
		if (m_BudgetManager)
		{
			m_BudgetManager.Event_OnBudgetMaxReached.Insert(OnBudgetMaxReached);
		}
		
		SCR_PlacingEditorComponentClass prefabData = SCR_PlacingEditorComponentClass.Cast(GetEditorComponentData());
	}
	
	override void EOnEditorDeactivate()
	{
		m_SelectedPrefab = ResourceName.Empty;
		
		if (m_BudgetManager)
		{
			m_BudgetManager.Event_OnBudgetMaxReached.Remove(OnBudgetMaxReached);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets the cycle waypoints to the given value on each group of the array
	//! \param[in] selectedGroups array of groups not null
	//! \param[in] value of type bool
	void SetCycleWaypoints(notnull set<SCR_EditableGroupComponent> selectedGroups, bool value)
	{
		
		if (Replication.IsRunning())
		{
			array<RplId> recipientIds = {};
			foreach (SCR_EditableGroupComponent entity: selectedGroups)
			{
				RplId id = Replication.FindId(entity);
				if (id.IsValid())
					recipientIds.Insert(id);
			}
			Rpc(SetCycleWaypointsServer, recipientIds, value);
		}
		else
		{
			foreach (SCR_EditableGroupComponent group : selectedGroups)
			{
				group.EnableCycledWaypoints(value);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void SetCycleWaypointsServer(array<RplId> selectedGroups, bool value)
	{
		foreach (RplId group : selectedGroups)
		{
			SCR_EditableGroupComponent.Cast(Replication.FindItem(group)).EnableCycledWaypoints(value);
		}
	}
};
