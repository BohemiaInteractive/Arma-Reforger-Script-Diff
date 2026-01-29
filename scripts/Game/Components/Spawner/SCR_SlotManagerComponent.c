[EntityEditorProps(insertable: false)]
class SCR_SpawnerSlotManagerClass : GenericEntityClass
{
}

//! Class used for managing changes and removals of slots present in world
class SCR_SpawnerSlotManager : GenericEntity
{
	protected static SCR_SpawnerSlotManager s_Instance;
	
	protected ref array <SCR_EntitySpawnerSlotComponent> m_aSlots = {};
	protected SCR_EditableEntityCore m_EditableEntityCore;
	
	protected ref ScriptInvoker m_OnSlotCreated;
	protected ref ScriptInvoker m_OnSlotRemoved;
	protected ref ScriptInvoker m_OnSlotUpdated;
	
	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvoker GetOnSlotCreated()
	{
		if (!m_OnSlotCreated)
			m_OnSlotCreated = new ScriptInvoker();
		
		return m_OnSlotCreated;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvoker GetOnSlotRemoved()
	{
		if (!m_OnSlotRemoved)
			m_OnSlotRemoved = new ScriptInvoker();
		
		return m_OnSlotRemoved;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvoker GetOnSlotUpdated()
	{
		if (!m_OnSlotUpdated)
			m_OnSlotUpdated = new ScriptInvoker();
		
		return m_OnSlotUpdated;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return the SCR_SpawnerSlotManager instance - cannot be null
	static SCR_SpawnerSlotManager GetInstance()
	{
		if (s_Instance)
			return s_Instance;
	
		s_Instance = SCR_SpawnerSlotManager.Cast(GetGame().SpawnEntity(SCR_SpawnerSlotManager, GetGame().GetWorld()));	
		if (!s_Instance)
			return null;
		
		s_Instance.SetupSlotUpdatedInvoker();
		return s_Instance;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetupSlotUpdatedInvoker()
	{
		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (core)
			core.Event_OnEntityTransformChanged.Insert(OnSlotUpdated);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] slot
	void RegisterSlot(SCR_EntitySpawnerSlotComponent slot)
	{
		m_aSlots.Insert(slot);
		
		if (m_EditableEntityCore)
			m_EditableEntityCore.Event_OnEntityTransformChanged.Insert(OnSlotUpdated);
		
		if (m_OnSlotCreated)
			m_OnSlotCreated.Invoke(slot);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] slot
	void UnregisterSlot(notnull SCR_EntitySpawnerSlotComponent slot)
	{	
		if (m_aSlots.Contains(slot))
			m_aSlots.RemoveItem(slot);
		
		if (m_OnSlotRemoved)
			m_OnSlotRemoved.Invoke(slot);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] editableComponent
	void OnSlotUpdated(notnull SCR_EditableEntityComponent editableComponent)
	{
		IEntity slotOwner = editableComponent.GetOwner();
		if (!slotOwner)
			return;
		
		SCR_EntitySpawnerSlotComponent slot = SCR_EntitySpawnerSlotComponent.Cast(slotOwner.FindComponent(SCR_EntitySpawnerSlotComponent));
		if (slot && m_OnSlotUpdated)
			m_OnSlotUpdated.Invoke(slot, slotOwner.GetOrigin());
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (core)
			core.Event_OnEntityTransformChanged.Insert(OnSlotUpdated);
		else
			Print("SCR_EditableEntityCore not found! Slot transformation won't be updated!", LogLevel.WARNING);	
	}
	
	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_SpawnerSlotManager()
	{
		if (s_Instance == this)
			s_Instance = null;
	}
}
