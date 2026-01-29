class SCR_AnimatedBeltComponentClass : ScriptComponentClass
{
	[Attribute("9", params: "1 600 1", desc: "How many bullets are visible in one UV texture (including half bullets assuming they come in pairs)", uiwidget: UIWidgets.EditBox, category: "Bullet data")]
	protected int m_iUVBulletCount;

	[Attribute("600", desc: "Rounds the gun shoots per minute. Affects belt-speed", uiwidget: UIWidgets.EditBox, category: "Bullet data")]
	protected int m_iRoundsPerMinute;

	private float m_iUVBulletCountPerMinuteInvRPM;
	private float m_iUVBulletCountInv;

	void SCR_AnimatedBeltComponentClass(IEntityComponentSource componentSource, IEntitySource parentSource, IEntitySource prefabSource)
	{
		int iUVBulletCountPerMinute = m_iUVBulletCount * 60;
		m_iUVBulletCountPerMinuteInvRPM = m_iRoundsPerMinute * (1 / iUVBulletCountPerMinute);
		m_iUVBulletCountInv = 1 / m_iUVBulletCount;
	}

	float GetUVBulletCountPerMinuteInvRPM() { return m_iUVBulletCountPerMinuteInvRPM; }
	float GetUVBulletCountInv() { return m_iUVBulletCountInv; }
}

class SCR_AnimatedBeltComponent : ScriptComponent
{
	protected static SCR_AnimatedBeltSystem s_system;
	protected float m_fBeltProgress;
	protected float m_fBeltTarget;
	protected float m_fBeltStartPoint;

	//------------------------------------------------------------------------------------------------
	//! Establish listeners to gunners entering this turret only after the magazine was attached to the turret
	override void OnAddedToParent(IEntity child, IEntity parent)
	{
		World world = GetOwner().GetWorld();
		s_system = SCR_AnimatedBeltSystem.Cast(world.FindSystem(SCR_AnimatedBeltSystem));

		IEntity turret = parent;
		if (!turret)
			return;

		EventHandlerManagerComponent eventHandler = EventHandlerManagerComponent.Cast(turret.FindComponent(EventHandlerManagerComponent));
		if (!eventHandler)
			return;

		eventHandler.RegisterScriptHandler("OnCompartmentEntered", child, OnCompartmentEntered);
		eventHandler.RegisterScriptHandler("OnCompartmentLeft", child, OnCompartmentLeft);
	}

	//------------------------------------------------------------------------------------------------
	//! establish listener to gunner firing to move the belt
	void OnCompartmentEntered(IEntity vehicle, BaseCompartmentManagerComponent mgr, IEntity occupant, int managerId, int slotID)
	{
		EventHandlerManagerComponent eventHandlerManager = EventHandlerManagerComponent.Cast(occupant.FindComponent(EventHandlerManagerComponent));
		if (eventHandlerManager)
			eventHandlerManager.RegisterScriptHandler("OnProjectileShot", this, OnProjectileShot);
	}

	//------------------------------------------------------------------------------------------------
	//! Start the UV shifting
	void OnProjectileShot(int playerID, BaseWeaponComponent weapon, IEntity entity)
	{
		if (s_system)
			s_system.Register(this);

		auto prefabData = SCR_AnimatedBeltComponentClass.Cast(GetComponentData(GetOwner()));

		float newBeltTarget = Math.Mod(m_fBeltTarget - prefabData.GetUVBulletCountInv(), 1);
		if (newBeltTarget > m_fBeltTarget)
		{
			m_fBeltProgress += 1;
			m_fBeltStartPoint = m_fBeltProgress;
		}

		m_fBeltTarget = newBeltTarget;
	}

	//------------------------------------------------------------------------------------------------
	//! Smoothly move the belt by exactly one bullet
	void Update(float timeSlice)
	{
		auto prefabData = SCR_AnimatedBeltComponentClass.Cast(GetComponentData(GetOwner()));

		m_fBeltProgress -= timeSlice * prefabData.GetUVBulletCountPerMinuteInvRPM();
		if (m_fBeltProgress < m_fBeltTarget)
		{
			m_fBeltProgress = m_fBeltTarget;
			if (s_system)
				s_system.Unregister(this);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! remove listeners to gunner when they leave and disconnect system
	void OnCompartmentLeft(IEntity vehicle, BaseCompartmentManagerComponent mgr, IEntity occupant, int managerId, int slotID)
	{
		DisconnectSystem();
		RemoveCharacterInvokers(occupant);
	}

	//------------------------------------------------------------------------------------------------
	//! removing all established invokers
	override void OnDelete(IEntity owner)
	{
		DisconnectSystem();
		RemoveTurretInvokers(owner);
		RemoveCharacterInvokers(owner);
	}

	//------------------------------------------------------------------------------------------------
	void DisconnectSystem()
	{
		if (!s_system)
			return;

		s_system.Unregister(this);
	}

	//------------------------------------------------------------------------------------------------
	void RemoveTurretInvokers(IEntity owner)
	{
		IEntity turret = owner.GetParent();
		if (!turret)
			return;

		EventHandlerManagerComponent turretEventHandler = EventHandlerManagerComponent.Cast(turret.FindComponent(EventHandlerManagerComponent));
		if (turretEventHandler)
		{
			turretEventHandler.RemoveScriptHandler("OnCompartmentEntered", GetOwner(), OnCompartmentEntered);
			turretEventHandler.RemoveScriptHandler("OnCompartmentLeft", GetOwner(), OnCompartmentLeft);
		}
	}

	//------------------------------------------------------------------------------------------------
	void RemoveCharacterInvokers(IEntity owner)
	{
		EventHandlerManagerComponent gunnerEventHandler = EventHandlerManagerComponent.Cast(owner.FindComponent(EventHandlerManagerComponent));
		if (gunnerEventHandler)
			gunnerEventHandler.RemoveScriptHandler("OnProjectileShot", this, OnProjectileShot);
	}
}
