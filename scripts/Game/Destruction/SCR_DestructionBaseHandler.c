#define ENABLE_BASE_DESTRUCTION
//------------------------------------------------------------------------------------------------
//! Base destruction handler, destruction handler types extend from this
//! TODO: Move this to damage manager instead
//! Ported from SCR_DestructionDamageManagerComponent
[BaseContainerProps()]
class SCR_DestructionBaseHandler
{
	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Model to swap to on destruction", params: "xob et")]
	protected ResourceName m_sWreckModel;

	[Attribute("1", UIWidgets.Slider, desc: "Delay for the wreck model switch upon destruction (ms)", params: "1 10000 1")]
	protected int m_iWreckDelay;

	[Attribute("100", UIWidgets.Slider, desc: "Default mass of the wreck physics, to be set if the part did not have physics before destruction", params: "0 10000 1")]
	protected float m_fDefaultWreckMass;

	[Attribute("0", UIWidgets.CheckBox, "If true the part will detach from parent after hitzone is destroyed")]
	protected bool m_bDetachAfterDestroyed;

	[Attribute("1", UIWidgets.CheckBox, "If true the part will be hidden after it is destroyed, unless wreck model is provided")]
	protected bool m_bAllowHideWreck;

	[Attribute("1", UIWidgets.CheckBox, "If true and not detached the part will be deleted after parent entity is destroyed")]
	protected bool m_bDeleteAfterParentDestroyed;

	[Attribute("0", UIWidgets.CheckBox, "If true the physics will be disabled after hitzone is destroyed")]
	protected bool m_bDisablePhysicsAfterDestroyed;

	[Attribute("0", UIWidgets.CheckBox, "If true and this entity doesnt have a parent then it will be permanently deleted")]
	protected bool m_bDeleteAfterDestroyed;
#ifdef ENABLE_BASE_DESTRUCTION

	protected IEntity m_pOwner; // TODO: Remove once we can get the owner dynamically
	protected bool m_bIsDestructionDelayed;

	//------------------------------------------------------------------------------------------------
	//! Destroy
	void StartDestruction(bool immediate = false)
	{
		if (immediate)
		{
			if (m_bIsDestructionDelayed)
				GetGame().GetCallqueue().Remove(StartDestruction);

			// Delegation is only allowed through destruction manager, ensuring the event will always be triggered inside frame
			// This includes deletion, physics deactivation, deletion
			SCR_MPDestructionManager destructionManager = SCR_MPDestructionManager.GetInstance();
			if (destructionManager)
				destructionManager.DestroyInFrame(this);
			else
				Debug.DPrint("SCR_MPDestructionManager is missing, cannot perform DestroyInFrame");

			return;
		}

		if (!m_bIsDestructionDelayed)
		{
			m_bIsDestructionDelayed = true;

			// The delay allows for particle effects to envelop vehicle before it is transformed to wreck.
			// CallLater is used as a simple measure to delay that operation
			GetGame().GetCallqueue().CallLater(StartDestruction, m_iWreckDelay, param1: true);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Revert model back to default
	void OnRepair()
	{
		if (!m_pOwner || m_pOwner.IsDeleted())
			return;

		ResourceName object = SCR_Global.GetPrefabAttributeResource(m_pOwner, "MeshObject", "Object");
		SetModel(object);

		// Some objects have no valid destruction physics
		if (!m_bDisablePhysicsAfterDestroyed)
			return;

		m_pOwner.SetFlags(EntityFlags.TRACEABLE, false);

		Physics physics = m_pOwner.GetPhysics();
		if (!physics)
			return;

		// Set proper physics simulation state for slotted entities, use just collision physics.
		if (m_pOwner.GetParent())
			physics.ChangeSimulationState(SimulationState.COLLISION);
		else
			physics.ChangeSimulationState(SimulationState.SIMULATION);
	}

	//------------------------------------------------------------------------------------------------
	//! Handle destruction
	//! Must happen inside frame event, for example via SCR_MPDestructionManager
	void HandleDestruction()
	{
		m_bIsDestructionDelayed = false;

		if (!m_pOwner || m_pOwner.IsDeleted())
			return;

		IEntity parent = m_pOwner.GetParent();
		if (parent && parent.IsDeleted())
			return;

		// Destroy all children slotted entities
		array<EntitySlotInfo> slotInfos = {};
		EntitySlotInfo.GetSlotInfos(m_pOwner, slotInfos);
		IEntity slotEntity;
		SCR_DamageManagerComponent damageMgr;
		HitZone hitZone;
		foreach (EntitySlotInfo slotInfo : slotInfos)
		{
			if (!slotInfo)
				continue;

			slotEntity = slotInfo.GetAttachedEntity();
			if (!slotEntity)
				continue;

			if (slotEntity.IsDeleted())
				continue;

			damageMgr = SCR_DamageManagerComponent.GetDamageManager(slotEntity);
			if (!damageMgr)
				continue;

			hitZone = damageMgr.GetDefaultHitZone();
			if (!hitZone)
				continue;

			if (damageMgr.GetState() == EDamageState.DESTROYED)
			{
				SCR_DestructibleHitzone destructibleHitZone = SCR_DestructibleHitzone.Cast(hitZone);
				if (destructibleHitZone)
					destructibleHitZone.StartDestruction(true);
			}
			else
			{
				hitZone.SetHealth(0);
			}
		}

		// Check parent only if the part is not set to be deleted anyway
		if (m_bDeleteAfterParentDestroyed && parent)
		{
			HitZoneContainerComponent parentContainer = HitZoneContainerComponent.Cast(parent.FindComponent(HitZoneContainerComponent));
			if (parentContainer && parentContainer.GetDefaultHitZone() && parentContainer.GetDefaultHitZone().GetDamageState() == EDamageState.DESTROYED)
			{
				DeleteSelf();
				return;
			}
		}

		if (m_bDeleteAfterDestroyed)
		{
			DeleteSelf();
			return;
		}

		if (m_bAllowHideWreck || !m_sWreckModel.IsEmpty())
			SetModel(m_sWreckModel, m_bAllowHideWreck);

		if (m_bDetachAfterDestroyed)
			DetachFromParent(true);

		// Some objects have no valid destruction physics
		if (!m_bDisablePhysicsAfterDestroyed)
			return;

		m_pOwner.ClearFlags(EntityFlags.TRACEABLE, false);

		Physics physics = m_pOwner.GetPhysics();
		if (physics)
			physics.ChangeSimulationState(SimulationState.NONE);
	}

	//------------------------------------------------------------------------------------------------
	//! Checks whether provided entity has parent and if so, tries to find a slot which it would belong to
	//! and unregister from it.
	private void DetachFromParent(bool updateHierarchy)
	{
		if (!m_pOwner || m_pOwner.IsDeleted())
			return;

		IEntity parent = m_pOwner.GetParent();
		if (!parent || parent.IsDeleted())
			return;

		Physics physics = m_pOwner.GetPhysics();
		if (!physics)
			return;

		vector velocity = physics.GetVelocity();
		vector angularVelocity = physics.GetAngularVelocity();

		EntitySlotInfo slotInfo = EntitySlotInfo.GetSlotInfo(m_pOwner);
		if (slotInfo)
			slotInfo.DetachEntity(updateHierarchy);

		if (physics.IsDynamic() && !m_pOwner.GetParent())
		{
			physics.SetVelocity(velocity);
			physics.SetAngularVelocity(angularVelocity);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Deletes self
	void DeleteSelf()
	{
		if (!m_pOwner || m_pOwner.IsDeleted())
			return;

		if (m_pOwner.GetParent())
			DetachFromParent(false);

		m_pOwner.SetObject(null, string.Empty);
		RplComponent.DeleteRplEntity(m_pOwner, true);
	}

	//------------------------------------------------------------------------------------------------
	/*! Sets the model of the object
	\param modelName Path to model
	\param allowEmpty Allow setting empty model
	*/
	protected void SetModel(ResourceName modelName, bool allowEmpty = false)
	{
		if (!m_pOwner || m_pOwner.IsDeleted())
			return;

		Vehicle vehicle = Vehicle.Cast(m_pOwner);
		if (vehicle)
		{
			vehicle.SetWreckModel(modelName);
			SCR_BaseEffectManagerComponent effectComp = SCR_BaseEffectManagerComponent.Cast(vehicle.FindComponent(SCR_BaseEffectManagerComponent));
			if (effectComp)
				effectComp.Deactivate(vehicle);
		}
		else
		{
			Resource resource = Resource.Load(modelName);
			VObject model;
			if (resource && resource.IsValid())
				model = resource.GetResource().ToVObject();
			
			if (model || allowEmpty)
				m_pOwner.SetObject(model, string.Empty);
		}

		m_pOwner.Update();

		// If there is no model, ignore the rest
		Physics physics = m_pOwner.GetPhysics();
		if (!physics)
			return;

		// If the object has dynamic physics, pass the state
		float mass = physics.GetMass();
		if (mass == 0)
			mass = m_fDefaultWreckMass;

		if (physics.UpdateGeometries())
		{
			physics.SetActive(ActiveState.ACTIVE);
			physics.EnableGravity(true);
			m_pOwner.SetFlags(EntityFlags.TRACEABLE);
		}
		else
		{
			physics.ChangeSimulationState(SimulationState.NONE);
			m_pOwner.ClearFlags(EntityFlags.TRACEABLE);
		}
	}

	//------------------------------------------------------------------------------------------------
	void Init(IEntity owner, HitZone hitZone)
	{
		m_pOwner = owner;
	}
#endif
}
