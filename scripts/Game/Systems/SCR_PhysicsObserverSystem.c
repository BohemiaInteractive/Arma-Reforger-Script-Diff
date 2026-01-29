class SCR_PhysicsObserverSystem : GameSystem
{
	protected ref array<ref Tuple3<IEntity, bool, SCR_PhysicsObserverAttribute>> m_aObservedEntities;

	//------------------------------------------------------------------------------------------------
	override bool ShouldBePaused()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	static SCR_PhysicsObserverSystem GetInstance()
	{
		World world = GetGame().GetWorld();
		SCR_PhysicsObserverSystem system = SCR_PhysicsObserverSystem.Cast(world.FindSystem(SCR_PhysicsObserverSystem));
		return system;
	}

	//------------------------------------------------------------------------------------------------
	//! Adds entity to the list of observed physics activity, while ensure that entity wasnt already added
	//! \param[in] ent
	//! \param[in] iic
	void RegisterEntity(notnull IEntity ent, InventoryItemComponent iic = null)
	{
		Physics phys = ent.GetPhysics();
		if (!phys)
			return;

		if (!m_aObservedEntities)
			m_aObservedEntities = {};

		foreach (Tuple3<IEntity, bool, SCR_PhysicsObserverAttribute> entry : m_aObservedEntities)
		{
			if (entry && entry.param1 == ent)
				return;
		}

		if (!iic)
		{
			iic = InventoryItemComponent.Cast(ent.FindComponent(InventoryItemComponent));
			if (!iic)
				return;
		}

		SCR_PhysicsObserverAttribute attribute = SCR_PhysicsObserverAttribute.Cast(iic.FindAttribute(SCR_PhysicsObserverAttribute));
		if (!attribute)
			return;

		m_aObservedEntities.Insert(new Tuple3<IEntity, bool, SCR_PhysicsObserverAttribute>(ent, phys.IsActive(), attribute));
		if (!IsEnabled())
			Enable(true);
	}

	//------------------------------------------------------------------------------------------------
	//! Stops observation of the entity physics activity
	//! \param[in] ent
	void UnregisterEntity(notnull IEntity ent)
	{
		if (!m_aObservedEntities || m_aObservedEntities.IsEmpty())
			return;

		foreach (int i, Tuple3<IEntity, bool, SCR_PhysicsObserverAttribute> entry : m_aObservedEntities)
		{
			if (!entry || entry.param1 != ent)
				continue;

			//Only null the entry to ensure that someone wont try to delete an entry mid update loop
			m_aObservedEntities[i] = null;
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdatePoint(WorldUpdatePointArgs args)
	{
		if (!m_aObservedEntities || m_aObservedEntities.IsEmpty())
		{
			Enable(false);
			return;
		}

		Physics phys;
		SCR_PhysicsObserverAttribute attribute;
		foreach (int i, Tuple3<IEntity, bool, SCR_PhysicsObserverAttribute> entry : m_aObservedEntities)
		{
			if (!entry)
				continue;

			if (!entry.param1)
			{
				m_aObservedEntities[i] = null;
				continue;
			}

			phys = entry.param1.GetPhysics();
			if (phys.IsActive() == entry.param2)
				continue;

			entry.param2 = !entry.param2;
			attribute = entry.param3;
			if (attribute)
				attribute.OnPhysicsStateChanged(entry.param1, entry.param2);
			else
				m_aObservedEntities[i] = null;
		}

		// When update finished looping through then remove null entries
		m_aObservedEntities.RemoveItem(null);
	}
}
