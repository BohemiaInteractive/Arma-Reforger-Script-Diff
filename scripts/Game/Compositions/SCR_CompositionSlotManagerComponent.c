void OnCompositionSlotManagerEntityChangedMethod(SCR_CompositionSlotManagerComponent slotManager, IEntity slot);
typedef func OnCompositionSlotManagerEntityChangedMethod;
typedef ScriptInvokerBase<OnCompositionSlotManagerEntityChangedMethod> OnCompsitionSlotManagerEntityChanged;

[ComponentEditorProps(category: "GameScripted/Compositions", description: "")]
class SCR_CompositionSlotManagerComponentClass : ScriptComponentClass
{
}

class SCR_CompositionSlotManagerComponent : ScriptComponent
{	
	protected ref map<EntityID, IEntity> m_aOccupiedStatic = new map<EntityID, IEntity>();
	protected ref map<RplId, IEntity> m_aOccupiedDynamic = new map<RplId, IEntity>();
	protected ref array<IEntity> m_aQueriedEntities;
	
	protected ref OnCompsitionSlotManagerEntityChanged m_OnEntityChanged;

	//------------------------------------------------------------------------------------------------
	//! \return
	OnCompsitionSlotManagerEntityChanged GetOnEntityChanged()
	{
		if (!m_OnEntityChanged)
			m_OnEntityChanged = new OnCompsitionSlotManagerEntityChanged();

		return m_OnEntityChanged;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	static SCR_CompositionSlotManagerComponent GetInstance()
	{
		if (GetGame().GetGameMode())
			return SCR_CompositionSlotManagerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_CompositionSlotManagerComponent));
		else
			return null;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] slot
	//! \param[in] occupant
	void SetOccupant(IEntity slot, IEntity occupant)
	{
		if (Replication.IsClient())
			return;
		
		RplId rplID = Replication.FindId(slot);
		if (rplID.IsValid())
			SetOccupiedDynamic(rplID, occupant != null, occupant);
		else
			SetOccupiedStatic(slot.GetID(), occupant != null, occupant);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] pos
	//! \param[in] occupant
	void SetOccupant(vector pos, IEntity occupant)
	{
		if (Replication.IsClient())
			return;
		
		m_aQueriedEntities = {};
		GetOwner().GetWorld().QueryEntitiesBySphere(pos, 1, QueryEntity, null, EQueryEntitiesFlags.STATIC);
		if (!m_aQueriedEntities.IsEmpty())
			SetOccupant(m_aQueriedEntities[0], occupant);
		
		m_aQueriedEntities = null;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] slot
	//! \return
	IEntity GetOccupant(IEntity slot)
	{
		IEntity occupant;
		RplId slotID = Replication.FindId(slot);
		if (slotID.IsValid())
			m_aOccupiedDynamic.Find(slotID, occupant);
		else
			m_aOccupiedStatic.Find(slot.GetID(), occupant);
		
		return occupant;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] slot
	//! \return
	bool IsOccupied(IEntity slot)
	{
		RplId slotID = Replication.FindId(slot);
		if (slotID.IsValid())
			return m_aOccupiedDynamic.Contains(slotID);
		else
			return m_aOccupiedStatic.Contains(slot.GetID());
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] occupant
	//! \return
	IEntity GetSlot(IEntity occupant)
	{
		for (int i = 0, count = m_aOccupiedStatic.Count(); i < count; i++)
		{
			if (m_aOccupiedStatic.GetElement(i) == occupant)
				return GetGame().GetWorld().FindEntityByID(m_aOccupiedStatic.GetKey(i));
		}

		for (int i = 0, count = m_aOccupiedDynamic.Count(); i < count; i++)
		{
			if (m_aOccupiedDynamic.GetElement(i) == occupant)
				return IEntity.Cast(Replication.FindItem(m_aOccupiedDynamic.GetKey(i)));
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] occupant
	//! \return
	bool IsInSlot(IEntity occupant)
	{
		for (int i = 0, count = m_aOccupiedStatic.Count(); i < count; i++)
		{
			if (m_aOccupiedStatic.GetElement(i) == occupant)
				return true;
		}

		for (int i = 0, count = m_aOccupiedDynamic.Count(); i < count; i++)
		{
			if (m_aOccupiedDynamic.GetElement(i) == occupant)
				return true;
		}

		return false;
	}
	
	//--- Static

	//------------------------------------------------------------------------------------------------
	protected void SetOccupiedStatic(EntityID slotID, bool isOccupied, IEntity occupant)
	{
		if (Replication.IsServer() && ModifyArray(slotID, isOccupied, occupant))
			Rpc(SetOccupiedStaticBroadcast, slotID, isOccupied);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void SetOccupiedStaticBroadcast(EntityID slotID, bool isOccupied)
	{
		ModifyArray(slotID, isOccupied, null);
	}
	
	//--- Dynamic

	//------------------------------------------------------------------------------------------------
	protected void SetOccupiedDynamic(RplId slotID, bool isOccupied, IEntity occupant)
	{
		if (Replication.IsServer() && ModifyArray(slotID, isOccupied, occupant))
			Rpc(SetOccupiedDynamicBroadcast, slotID, isOccupied);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void SetOccupiedDynamicBroadcast(RplId slotID, bool isOccupied)
	{
		ModifyArray(slotID, isOccupied, null);
	}
	
	//--- Arrays

	//------------------------------------------------------------------------------------------------
	protected bool ModifyArray(EntityID slotID, bool isOccupied, IEntity occupant)
	{
		if (isOccupied)
		{
			bool isChange = m_aOccupiedStatic.Insert(slotID, occupant);
			UpdateSlot(GetGame().GetWorld().FindEntityByID(slotID), isOccupied);
			return isChange;
		}
		else
		{
			if (m_aOccupiedStatic.Find(slotID, occupant))
			{
				m_aOccupiedStatic.Remove(slotID);
				UpdateSlot(GetGame().GetWorld().FindEntityByID(slotID), isOccupied);
				return true;
			}
			else
			{
				return false;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool ModifyArray(RplId slotID, bool isOccupied, IEntity occupant)
	{
		if (isOccupied)
		{
			bool isChange = m_aOccupiedDynamic.Insert(slotID, occupant);
			UpdateSlot(IEntity.Cast(Replication.FindItem(slotID)), isOccupied);
			return isChange;
		}
		else
		{
			if (m_aOccupiedDynamic.Find(slotID, occupant))
			{
				m_aOccupiedDynamic.Remove(slotID);
				UpdateSlot(IEntity.Cast(Replication.FindItem(slotID)), isOccupied);
				return true;
			}
			else
			{
				return false;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateSlot(IEntity slot, bool isOccupied)
	{
		if (!slot)
			return;
		
		SCR_EditableEntityComponent entity = SCR_EditableEntityComponent.Cast(slot.FindComponent(SCR_EditableEntityComponent));
		if (!entity)
			return;

		if (entity.GetEntityType(slot) == EEditableEntityType.SLOT)
			entity.SetVisible(!isOccupied);
	
		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (core)
			core.Event_OnEntityExtendedChange.Invoke(entity, isOccupied);

		if (m_OnEntityChanged)
			m_OnEntityChanged.Invoke(this, slot);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnEntityChanged(SCR_EditableEntityComponent entity)
	{
		IEntity slot = GetSlot(entity.GetOwner());
		if (!slot)
			return;
		
		//--- Ignore when not deleted and position didn't change (e.g., only rotation did)
		if (entity.IsRegistered())
		{
			vector transform[4];
			entity.GetTransform(transform);
			if (vector.Distance(transform[3], slot.GetWorldTransformAxis(3)) < 0.1)
			{
				if (m_OnEntityChanged)
					m_OnEntityChanged.Invoke(this, slot);

				return;
			}
		}
		
		SetOccupant(slot, null);
		if (m_OnEntityChanged)
			m_OnEntityChanged.Invoke(this, slot);
	}

	//------------------------------------------------------------------------------------------------
	protected bool QueryEntity(IEntity entity)
	{
		if (entity.IsInherited(SCR_SiteSlotEntity))
		{
			m_aQueriedEntities.Insert(entity);
			return false;
		}
		else
		{
			return true;
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool RplSave(ScriptBitWriter writer)
	{
		int occupiedCount = m_aOccupiedStatic.Count();
		writer.WriteInt(occupiedCount);
		for (int i = 0; i < occupiedCount; i++)
		{
			writer.WriteEntityId(m_aOccupiedStatic.GetKey(i));
		}
		
		occupiedCount = m_aOccupiedDynamic.Count();
		writer.WriteInt(occupiedCount);
		for (int i = 0; i < occupiedCount; i++)
		{
			writer.WriteRplId(m_aOccupiedDynamic.GetKey(i));
		}
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		int occupiedCount;
		reader.ReadInt(occupiedCount);
		
		EntityID entityID;
		m_aOccupiedStatic.Clear();
		for (int i = 0; i < occupiedCount; i++)
		{
			reader.ReadEntityId(entityID);
			m_aOccupiedStatic.Insert(entityID, null);
			UpdateSlot(GetGame().GetWorld().FindEntityByID(entityID), true);
		}
		
		RplId rplID;
		reader.ReadInt(occupiedCount);
		m_aOccupiedDynamic.Clear();
		for (int i = 0; i < occupiedCount; i++)
		{
			reader.ReadRplId(rplID);
			m_aOccupiedDynamic.Insert(rplID, null);
			UpdateSlot(IEntity.Cast(Replication.FindItem(rplID)), true);
		}
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (core)
		{
			core.Event_OnEntityUnregistered.Insert(OnEntityChanged);
			core.Event_OnEntityTransformChanged.Insert(OnEntityChanged);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (core)
		{
			core.Event_OnEntityUnregistered.Remove(OnEntityChanged);
			core.Event_OnEntityTransformChanged.Remove(OnEntityChanged);
		}
	}
}
