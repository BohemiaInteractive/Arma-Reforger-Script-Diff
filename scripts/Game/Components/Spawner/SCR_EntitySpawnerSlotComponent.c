[EntityEditorProps(category: "GameScripted/Spawner", description: "Slot used for spawning entities.")]
class SCR_EntitySpawnerSlotComponentClass : ScriptComponentClass
{
	[Attribute(uiwidget: UIWidgets.ComboBox, desc: "Slot size.", enums: ParamEnumArray.FromEnum(SCR_EEntitySpawnerSlotType), category: "Entity Spawner Slot")]
	protected SCR_EEntitySpawnerSlotType m_eSlotType;

	[Attribute(defvalue: "-5 0 -5", category: "Slot Boundaries")]
	protected vector m_vMinBounds;

	[Attribute(defvalue: "5 5 5", category: "Slot Boundaries")]
	protected vector m_vMaxBounds;

	[Attribute(defvalue: "10", params: "0 inf", desc: "Maximum distance for rally point.", category: "Entity Spawner Slot")]
	protected float m_fMaxRallyPointDistance;
	
	[Attribute(defvalue: "30", params: "1 inf", desc: "Maximum distance for teleporting characters out of slot.", category: "Entity Spawner")]
	protected float m_fTeleportMaxDistance;
	
	[Attribute(defvalue: "3", params: "1 inf", desc: "Size of Cylinder used to search empty terrain position. Should be big enough to prevent empty position being too close to slot center.", category: "Entity Spawner")]
	protected float m_fTeleportSearchSize;

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetMaxRallyPointDistance()
	{
		return m_fMaxRallyPointDistance;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	vector GetMinBoundsVector()
	{
		return m_vMinBounds;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	vector GetMaxBoundsVector()
	{
		return m_vMaxBounds;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_EEntitySpawnerSlotType GetSlotType()
	{
		return m_eSlotType;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	float GetTeleportMaximumDistance()
	{
		return m_fTeleportMaxDistance;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	float GetTeleportSearchSize()
	{
		return m_fTeleportSearchSize;
	}
}

//------------------------------------------------------------------------------------------------
//! Specifies slot to be used with nearby Entity Spawner Components
class SCR_EntitySpawnerSlotComponent : ScriptComponent
{
#ifdef WORKBENCH

	[Attribute("0", desc: "Draw visualisation of bounds", category: "Debug")]
	protected bool m_bShowDebugShape;

	protected ref Shape m_DebugShape;
	protected int m_iDebugShapeColor = Color.CYAN;
#endif

	protected RplComponent m_RplComponent;
	protected SCR_SpawnerSlotManager m_SlotManager;
	protected SCR_EntityLabelPointComponent m_RallyPointLabelComponent;
	protected ref array<ChimeraCharacter> m_aCharacterArray;
	protected ref array<IEntity> m_aExcludedEntities;
	
	//------------------------------------------------------------------------------------------------
	//! Returns true, if SCR_EntityLabelPointComponent is in range defined by m_fMaxRallyPointDistance
	//! \param[in] labelComp
	//! \return
	bool IsEntityLabelInRange(notnull SCR_EntityLabelPointComponent labelComp)
	{
		SCR_EntitySpawnerSlotComponentClass prefabData = SCR_EntitySpawnerSlotComponentClass.Cast(GetComponentData(GetOwner()));
		if (!prefabData)
			return false;

		float maxDistance = prefabData.GetMaxRallyPointDistance();

		return vector.DistanceSqXZ(labelComp.GetOwner().GetOrigin(), GetOwner().GetOrigin()) > maxDistance * maxDistance;
	}

	//------------------------------------------------------------------------------------------------
	//! Assign labelComponent to be used as Rally Point. If component owner isn't in range, Rally point won't be assigned
	//! \param[in] labelComp
	void SetRallyPoint(notnull SCR_EntityLabelPointComponent labelComp)
	{
		if (!labelComp.HasLabel(EEditableEntityLabel.GAMELOGIC_RALLYPOINT) && !IsEntityLabelInRange(labelComp))
			return;

		m_RallyPointLabelComponent = labelComp;
		m_RallyPointLabelComponent.GetOnOwnerUpdated().Insert(OnRallyPointUpdated);
	}

	//------------------------------------------------------------------------------------------------
	//! Get Assigned Rally point. If there is none, Slot will try to find new one.
	//! \return
	SCR_EntityLabelPointComponent GetRallyPoint()
	{
		SCR_EntitySpawnerSlotComponentClass prefabData = SCR_EntitySpawnerSlotComponentClass.Cast(GetComponentData(GetOwner()));
		if (!prefabData)
			return null;

		if (!m_RallyPointLabelComponent)
			GetGame().GetWorld().QueryEntitiesBySphere(GetOwner().GetOrigin(), prefabData.GetMaxRallyPointDistance(), RallyPointSearchCallback);

		return m_RallyPointLabelComponent;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_EEntitySpawnerSlotType GetSlotType()
	{
		SCR_EntitySpawnerSlotComponentClass prefabData = SCR_EntitySpawnerSlotComponentClass.Cast(GetComponentData(GetOwner()));
		if (!prefabData)
			return null;

		return prefabData.GetSlotType();
	}

	//------------------------------------------------------------------------------------------------
	//! Returns true, if slot is occupied
	bool IsOccupied()
	{
		SCR_EntitySpawnerSlotComponentClass prefabData = SCR_EntitySpawnerSlotComponentClass.Cast(GetComponentData(GetOwner()));
		if (!prefabData)
			return false;

		TraceOBB trace = new TraceOBB();
		GetOwner().GetWorldTransform(trace.Mat);
		
		if (!m_aExcludedEntities)
			FillExcludedEntities();
		
		trace.Start = GetOwner().GetOrigin();
		trace.ExcludeArray = m_aExcludedEntities;
		trace.LayerMask = EPhysicsLayerPresets.Projectile;
		trace.Flags = TraceFlags.ENTS;
		trace.Mins = prefabData.GetMinBoundsVector();
		trace.Maxs = prefabData.GetMaxBoundsVector();

		GetGame().GetWorld().TracePosition(trace, TraceCallback);

		return trace.TraceEnt;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Moves away all characters from slot, if there is suitable empty position in vicinity.
	void MoveCharactersFromSlot()
	{
		if (!m_RplComponent || m_RplComponent.IsProxy())
			return;
		
		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!playerManager)
			return;
		
		SCR_EntitySpawnerSlotComponentClass prefabData = SCR_EntitySpawnerSlotComponentClass.Cast(GetComponentData(GetOwner()));
		if (!prefabData)
			return;
		
		//Get All characters in slot bounds
		array<ChimeraCharacter> characterArray = {};
		GetCharactersInSlot(characterArray);
		
		if (characterArray.IsEmpty())
			return;
		
		//Obtain all empty terrain positions
		array <vector> positions = {};
		if (SCR_WorldTools.FindAllEmptyTerrainPositions(positions, GetOwner().GetOrigin(), prefabData.GetTeleportMaximumDistance(), prefabData.GetTeleportSearchSize(), maxResults:characterArray.Count()) == 0)
			return;
		
		vector transform[4];
		PlayerController playerController;
		SCR_SpawnerRequestComponent requestComp;
		SCR_EditableEntityComponent editableComp;
		int playerId;
		
		//Move characters to found free empty terrain positions
		foreach (int i, ChimeraCharacter character : characterArray)
		{	
			//If there is not enough Empty terrain positions, stop teleporting characters
			if (!positions.IsIndexValid(i))
				break;
			
			playerId = playerManager.GetPlayerIdFromControlledEntity(character);	
			if (playerId > 0)
			{
				playerController = playerManager.GetPlayerController(playerId);
				requestComp = SCR_SpawnerRequestComponent.Cast(playerController.FindComponent(SCR_SpawnerRequestComponent));
				if (requestComp)
					requestComp.RequestPlayerTeleport(positions[i]);
			}
			else
			{
				editableComp = SCR_EditableEntityComponent.Cast(character.FindComponent(SCR_EditableEntityComponent));
				if (!editableComp)
					continue;
		
				character.GetWorldTransform(transform);
				transform[3] = positions[i];
				SCR_TerrainHelper.OrientToTerrain(transform);
		
				editableComp.SetTransform(transform);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get all ChimeraCharacters in slot. Returns count.
	//! \param[in] characterArray outs array containing all ChimeraCharacters in slot
	int GetCharactersInSlot(out array<ChimeraCharacter> characterArray)
	{
		SCR_EntitySpawnerSlotComponentClass prefabData = SCR_EntitySpawnerSlotComponentClass.Cast(GetComponentData(GetOwner()));
		if (!prefabData)
			return 0;
		
		vector transform[4];
		GetOwner().GetTransform(transform);
		
		m_aCharacterArray = {};
		GetGame().GetWorld().QueryEntitiesByOBB(prefabData.GetMinBoundsVector(), prefabData.GetMaxBoundsVector(), transform, CharacterFoundCallback, FilterCharactersCallback);
		characterArray.InsertAll(m_aCharacterArray);
		
		m_aCharacterArray = null;
		return characterArray.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void FillExcludedEntities()
	{
		m_aExcludedEntities = {};
	
		SCR_EditableEntityComponent comp = SCR_EditableEntityComponent.Cast(GetOwner().FindComponent(SCR_EditableEntityComponent));
		if (!comp)
			return;
		
		set<SCR_EditableEntityComponent> childrenSet = new set<SCR_EditableEntityComponent> ();
		SCR_EditableEntityComponent parent = comp.GetParentEntity();
		if (!parent)
			return;
		
		parent.GetChildren(childrenSet);
		foreach(SCR_EditableEntityComponent editableChildren : childrenSet)
		{
			if (!editableChildren)
				continue;
			
			m_aExcludedEntities.Insert(editableChildren.GetOwner());
		}
			
		m_aExcludedEntities.Insert(parent.GetOwner());
		
	}
	
	//------------------------------------------------------------------------------------------------
	//! Callback used in GetCharacterInSlot for query. Adds living ChimeraCharacters into array.
	//! \param[in] ent
	//! \return always true
	protected bool CharacterFoundCallback(IEntity ent)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(ent);
		if (character && m_aCharacterArray)
		{
			if (!IsEntityDestroyed(character))
				m_aCharacterArray.Insert(character);
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Filtering used in GetCharacterInSlot for query
	//! \return
	protected bool FilterCharactersCallback(IEntity ent)
	{
		if (ent.IsInherited(ChimeraCharacter))
			return true;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Callback for TracePosition in IsOccupied. Can be overridden to implement custom conditions for checks in traces
	//! \param[in] ent
	protected bool TraceCallback(IEntity ent)
	{
		// Return false if entity is not in desired simulation state, contains cloth component (prevent detection of pouches) or is a weapon
		if (ent.IsLoaded() || ent.GetPhysics().GetSimulationState() == 0 || ent.FindComponent(BaseLoadoutClothComponent) || ent.FindComponent(WeaponComponent))
			return false;

		//Ignore proxy entities
		EntityFlags entityFlags = ent.GetFlags();
		if (entityFlags & EntityFlags.PROXY)
			return false;
		
		if (ent.IsInherited(ChimeraCharacter))
				return false;
		
		// Filter out dead bodies and wrecks
		if (IsEntityDestroyed(ent))
		{
			if (ent.IsInherited(Vehicle) && m_RplComponent)
			{
				if (!m_RplComponent.IsProxy())
					SCR_EntityHelper.DeleteEntityAndChildren(ent);

				return false;
			}
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsEntityDestroyed(IEntity entity)
	{
		SCR_DamageManagerComponent damageManager = SCR_DamageManagerComponent.Cast(entity.FindComponent(SCR_DamageManagerComponent));
		if (damageManager)
			return damageManager.IsDestroyed();

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Called when Rally Point gets moved.
	protected void OnRallyPointUpdated()
	{
		if (IsEntityLabelInRange(m_RallyPointLabelComponent))
			return;

		m_RallyPointLabelComponent.GetOnOwnerUpdated().Remove(OnRallyPointUpdated);
		m_RallyPointLabelComponent = null;
		m_RallyPointLabelComponent = GetRallyPoint();
	}

	//------------------------------------------------------------------------------------------------
	//! Callback for Query in GetRallyPoint. Returns false, once SCR_EntityLabelPointComponent matching parameters was found (And thus ending query)
	protected bool RallyPointSearchCallback(IEntity ent)
	{
		SCR_EntityLabelPointComponent labelComp = SCR_EntityLabelPointComponent.Cast(ent.FindComponent(SCR_EntityLabelPointComponent));
		if (labelComp && labelComp.HasLabel(EEditableEntityLabel.GAMELOGIC_RALLYPOINT))
		{
			m_RallyPointLabelComponent = labelComp;
			return false;
		}

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
#ifdef WORKBENCH
		if (m_bShowDebugShape)
			DrawDebugShape();
#endif

		if (SCR_Global.IsEditMode())
			return;

		m_SlotManager = SCR_SpawnerSlotManager.GetInstance();
		if (!m_SlotManager)
		{
			Print("Gamemode is missing SCR_SlotManagerComponent, which is required for functionality of SCR_EntitySpawnerSlotComponent", LogLevel.WARNING);
			return;
		}

		m_SlotManager.RegisterSlot(this);

		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (!m_RplComponent)
			Print("SCR_EntitySpawnerSlotComponent is missing RplComponent. It won't work properly without it", LogLevel.WARNING);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.INIT);

#ifdef WORKBENCH
		if (m_bShowDebugShape)
			SetEventMask(owner, EntityEvent.FRAME);
#endif
	}

	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_EntitySpawnerSlotComponent()
	{
		if (m_SlotManager)
			m_SlotManager.UnregisterSlot(this);

		if (m_RallyPointLabelComponent)
			m_RallyPointLabelComponent.GetOnOwnerUpdated().Remove(OnRallyPointUpdated);
	}

#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	protected void DrawDebugShape()
	{
		SCR_EntitySpawnerSlotComponentClass prefabData = SCR_EntitySpawnerSlotComponentClass.Cast(GetComponentData(GetOwner()));
		if (!prefabData)
			return;

		vector transform[4];
		IEntity owner = GetOwner();
		owner.GetTransform(transform);

		int shapeFlags = ShapeFlags.WIREFRAME;
		m_DebugShape = Shape.Create(ShapeType.BBOX, m_iDebugShapeColor, shapeFlags, prefabData.GetMinBoundsVector(), prefabData.GetMaxBoundsVector());

		m_DebugShape.SetMatrix(transform);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (m_bShowDebugShape && m_DebugShape)
		{
			vector transform[4];
			owner.GetTransform(transform);
			m_DebugShape.SetMatrix(transform);
		}
	}

	//------------------------------------------------------------------------------------------------
	override int _WB_GetAfterWorldUpdateSpecs(IEntity owner, IEntitySource src)
	{
		return EEntityFrameUpdateSpecs.CALL_WHEN_ENTITY_VISIBLE;
	}
	
	//------------------------------------------------------------------------------------------------
	override event void _WB_AfterWorldUpdate(IEntity owner, float timeSlice)
	{
		if (m_bShowDebugShape && m_DebugShape)
		{
			vector transform[4];
			owner.GetTransform(transform);
			m_DebugShape.SetMatrix(transform);
		}
	}
#endif
}

enum SCR_EEntitySpawnerSlotType
{
	GROUP_SMALL = 1 << 0,
	GROUP_MEDIUM = 1 << 1,
	GROUP_BIG = 1 << 2,

	VEHICLE_SMALL = 1 << 3,
	VEHICLE_MEDIUM = 1 << 4,
	VEHICLE_LARGE = 1 << 5
}
