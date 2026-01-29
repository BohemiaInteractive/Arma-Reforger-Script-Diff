#define ENABLE_BASE_DESTRUCTION
class SCR_DestructionDamageManagerComponentClass: SCR_DamageManagerComponentClass
{
	[Attribute("100", UIWidgets.Slider, "Base health value of the object. Damage received above this value results in destruction (overrides HPMax in hit zone)", "0.01 100000 0.01", category: "Destruction Setup")]
	float m_fBaseHealth;
	[Attribute("50", UIWidgets.Slider, "Relative contact force to damage multiplier", "0.01 50000 0.01", category: "Destruction Setup")]
	float m_fForceToDamageScale;
	[Attribute("0.05", UIWidgets.Slider, "Contact momentum to damage multiplier", "0.01 10 0.01", category: "Destruction Setup")]
	float m_fMomentumToDamageScale;
	[Attribute("0.5", UIWidgets.Slider, "Minimum relative contact force (impulse / mass) threshold below which contacts are ignored", "0 50000 0.01", category: "Destruction Setup")]
	float m_fRelativeContactForceThresholdMinimum;
	[Attribute("3000", UIWidgets.Slider, "Maximum damage threshold above which the object is completely destroyed and no effects are played (eg: nuclear bomb damage)", "0.01 50000 0.01", category: "Destruction Setup")]
	float m_fDamageThresholdMaximum;
	[Attribute("0", desc: "Should children destructible objects also receive the damage dealt to this one?", category: "Destruction Setup")]
	bool m_bPassDamageToChildren;
	[Attribute("0", desc: "Should the parent of this object also be destroyed when this object gets destroyed?", category: "Destruction Setup")]
	bool m_bDestroyParentWhenDestroyed;
	[Attribute("0", desc: "Should the children of this object also be destroyed when this object gets destroyed?", category: "Destruction Setup")]
	bool m_bDestroyChildrenWhenDestroyed;

	[Attribute("", UIWidgets.Object, "List of objects (particles, debris, etc) to spawn on destruction of the object", category: "Destruction FX")]
	ref array<ref SCR_BaseSpawnable> m_DestroySpawnObjects;
}

//! Base destruction component, destruction types extend from this
class SCR_DestructionDamageManagerComponent : SCR_DamageManagerComponent
{
#ifdef ENABLE_BASE_DESTRUCTION
	protected static ref ScriptInvoker s_OnDestructibleDestroyed;
	protected static bool s_bReadingInit = false; //Used to determine whether we are in gameplay or synchronization state

#ifdef WORKBENCH
	protected static bool s_bPrintMissingComponent;
	protected static bool s_bPrintMissingPlayerController;
	protected static bool s_bPrintInitializationFailed;
#endif

	protected static int s_iFirstFreeDestructionBaseData = -1;
	protected static ref array<ref SCR_DestructionBaseData> s_aDestructionBaseData = {};

	protected int m_iDestructionBaseDataIndex = -1;
	protected int m_iLastCollisionTime = -1;

	//------------------------------------------------------------------------------------------------
	static bool GetReadingInit(bool readingInit)
	{
		return s_bReadingInit;
	}

	//------------------------------------------------------------------------------------------------
	static void SetReadingInit(bool readingInit)
	{
		s_bReadingInit = readingInit;
	}

	//------------------------------------------------------------------------------------------------
	protected notnull SCR_DestructionBaseData GetDestructionBaseData()
	{
		if (m_iDestructionBaseDataIndex == -1)
			m_iDestructionBaseDataIndex = AllocateDestructionBaseData();

		return s_aDestructionBaseData[m_iDestructionBaseDataIndex];
	}

	//------------------------------------------------------------------------------------------------
	private int AllocateDestructionBaseData()
	{
		if (s_iFirstFreeDestructionBaseData == -1)
			return s_aDestructionBaseData.Insert(new SCR_DestructionBaseData());
		else
		{
			int returnIndex = s_iFirstFreeDestructionBaseData;
			SCR_DestructionBaseData data = s_aDestructionBaseData[returnIndex];
			s_iFirstFreeDestructionBaseData = data.m_iNextFreeIndex;
			data.m_iNextFreeIndex = -1;
			return returnIndex;
		}
	}

	//------------------------------------------------------------------------------------------------
	private void FreeDestructionBaseData(int index)
	{
		s_aDestructionBaseData[index].Reset();
		s_aDestructionBaseData[index].m_iNextFreeIndex = s_iFirstFreeDestructionBaseData;
		s_iFirstFreeDestructionBaseData = index;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns the on destructible destroyed script invoker if one exists, otherwise creates one and returns it
	static ScriptInvoker GetOnDestructibleDestroyedInvoker()
	{
		if (!s_OnDestructibleDestroyed)
			s_OnDestructibleDestroyed = new ScriptInvoker();

		return s_OnDestructibleDestroyed;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns the on damage script invoker if one exists, otherwise creates one and returns it
	ScriptInvoker GetOnDamageInvoker()
	{
		return GetDestructionBaseData().GetOnDamage();
	}

	//------------------------------------------------------------------------------------------------
	SCR_DestructionHitInfo GetDestructionHitInfo(bool createNew = false)
	{
		if (m_iDestructionBaseDataIndex == -1 && !createNew)
			return null;

		return GetDestructionBaseData().GetHitInfo(createNew);
	}

	//------------------------------------------------------------------------------------------------
	bool ShouldDestroyParent()
	{
		SCR_DestructionDamageManagerComponentClass prefabData = SCR_DestructionDamageManagerComponentClass.Cast(GetComponentData(GetOwner()));
		return prefabData && prefabData.m_bDestroyParentWhenDestroyed;
	}

	//------------------------------------------------------------------------------------------------
	bool ShouldDestroyChildren()
	{
		SCR_DestructionDamageManagerComponentClass prefabData = SCR_DestructionDamageManagerComponentClass.Cast(GetComponentData(GetOwner()));
		return prefabData && prefabData.m_bDestroyChildrenWhenDestroyed;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns whether the object should disable physics on destruction (before being deleted or changed)
	bool GetDisablePhysicsOnDestroy()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns whether the object is undamaged
	bool GetUndamaged()
	{
		if (m_iDestructionBaseDataIndex == -1)
			return GetHealth() == 1;

		return !GetDestructionBaseData().GetHitInfo(false) && GetHealth() == 1;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns whether the object is destroyed
	bool GetDestroyed()
	{
		if (m_iDestructionBaseDataIndex == -1)
			return GetHealth() <= 0;

		return GetHealth() <= 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Spawns destroy objects of children as well as own
	[RplRpc(RplChannel.Unreliable, RplRcver.Broadcast)]
	void RPC_DoSpawnAllDestroyEffects()
	{
		SCR_DestructionDamageManagerComponentClass componentData = SCR_DestructionDamageManagerComponentClass.Cast(GetComponentData(GetOwner()));
		if (componentData)
			SCR_DestructionUtility.SpawnDestroyObjects(GetOwner(), componentData.m_DestroySpawnObjects, new SCR_DestructionHitInfo());

		IEntity child = GetOwner().GetChildren();
		while (child)
		{
			SCR_DestructionDamageManagerComponentClass componentDataChild = SCR_DestructionDamageManagerComponentClass.Cast(GetComponentData(child));
			if (!componentDataChild)
			{
				child = child.GetSibling();
				continue;
			}

			SCR_DestructionDamageManagerComponent destructible = SCR_DestructionDamageManagerComponent.Cast(child.FindComponent(SCR_DestructionDamageManagerComponent));
			if (destructible)
				SCR_DestructionUtility.SpawnDestroyObjects(child, componentDataChild.m_DestroySpawnObjects, new SCR_DestructionHitInfo());

			child = child.GetSibling();
		}
	}

	//------------------------------------------------------------------------------------------------
	void DeleteParentWithEffects()
	{
		if (GetDestructionBaseData().GetDestructionQueued())
			return; // Destruction queued already, don't do this.
		
		RPC_DoSpawnAllDestroyEffects();
		Rpc(RPC_DoSpawnAllDestroyEffects);

		DeleteDestructibleDelayed();
	}

	//------------------------------------------------------------------------------------------------
	//! Deletion of the owner of this component which is going to be delayed by a frame to ensure proper excecution of invokers and RPC calls
	void DeleteDestructibleDelayed()
	{
		if (GetOwner().IsDeleted())
			return;

		if (GetDestructionBaseData().GetDestructionQueued())
			return; // Destruction queued already, don't do this.
		
		GetDestructionBaseData().SetDestructionQueued(true);
		GetGame().GetCallqueue().CallLater(DeleteDestructible);
	}

	//------------------------------------------------------------------------------------------------
	//! Deletion of children of owner of this component which is going to be delayed by a frame to ensure proper excecution of invokers and RPC calls
	void DeleteDestructibleChildrenDelayed(bool withEffects = true)
	{
		if (GetOwner().IsDeleted())
			return;

		GetGame().GetCallqueue().CallLater(DeleteDestructibleChildren, param1: withEffects);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Used in cases where we don't care about effects or anything, e. g. building destruction
	//! Use DeleteDestructibleDelayed instead
	private void DeleteDestructible()
	{
		if (GetOwner().IsDeleted())
			return;

		if (ShouldDestroyParent())
		{
			IEntity parent = GetOwner().GetParent();
			if (parent)
			{
				SCR_DestructionDamageManagerComponent destructible = SCR_DestructionDamageManagerComponent.Cast(parent.FindComponent(SCR_DestructionDamageManagerComponent));
				if (destructible)
				{
					destructible.DeleteParentWithEffects();
					return;
				}
			}
		}

		if (ShouldDestroyChildren())
			DeleteDestructibleChildren();

		SCR_DestructionUtility.RegenerateNavmeshDelayed(GetOwner());

		array<HitZone> outHitZones = {};
		if (GetAllHitZones(outHitZones) < 1)
			RplComponent.DeleteRplEntity(GetOwner(), false);

		SCR_DestructibleHitzone destructibleHitZone;
		foreach (HitZone hitZone : outHitZones)
		{
			destructibleHitZone = SCR_DestructibleHitzone.Cast(hitZone);
			if (!destructibleHitZone)
				continue;

			if (destructibleHitZone.GetDestructionHandler())
				return;
		}

		RplComponent.DeleteRplEntity(GetOwner(), false);
	}

	//------------------------------------------------------------------------------------------------
	//! Used in cases where we don't care about effects or anything, e. g. building destruction
	//! Use DeleteDestructibleDelayed instead
	private void DeleteDestructibleChildren(bool withEffects = true)
	{
		IEntity child = GetOwner().GetChildren();
		// Iterate through all siblings
		SCR_DestructionDamageManagerComponent childDestructionComponent;
		while (child)
		{
			// Store the next sibling before potentially destroying the current one
			IEntity nextSibling = child.GetSibling();
			
			// Check if the child has a destruction component
			childDestructionComponent = SCR_DestructionDamageManagerComponent.Cast(child.FindComponent(SCR_DestructionDamageManagerComponent));
			
			if (childDestructionComponent)
			{
				if (withEffects)
					// If it has a destruction component, use it to destroy the child
					childDestructionComponent.DeleteParentWithEffects();
				else
					childDestructionComponent.DeleteDestructibleDelayed();
			}
			
			// Move to the next sibling
			child = nextSibling;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets the base health value of the hit zone (does not trigger destruction)
	void SetHitZoneHealth(float baseHealth, bool clearDamage = true)
	{
		HitZone hitZone = GetDefaultHitZone();
		if (!hitZone)
			return;
		
		hitZone.SetMaxHealth(baseHealth);
		hitZone.SetHealth(baseHealth);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets damage of the hit zone (does not trigger destruction)
	void SetHitZoneDamage(float damage)
	{
		HitZone hitZone = GetDefaultHitZone();
		if (!hitZone)
			return;

		hitZone.SetHealth(hitZone.GetMaxHealth() - damage);
	}

	//------------------------------------------------------------------------------------------------
	//! Creates and fills destruction hit info for the object
	SCR_DestructionHitInfo CreateDestructionHitInfo(bool totalDestruction, float lastHealth, float hitDamage, EDamageType damageType, vector hitPosition, vector hitDirection, vector hitNormal)
	{
		SCR_DestructionHitInfo hitInfo = GetDestructionBaseData().GetHitInfo();

		hitInfo.m_TotalDestruction = totalDestruction;
		hitInfo.m_LastHealth = lastHealth;
		hitInfo.m_HitDamage = hitDamage;
		hitInfo.m_DamageType = damageType;
		hitInfo.m_HitPosition = hitPosition;
		hitInfo.m_HitDirection = hitDirection;
		hitInfo.m_HitNormal = hitNormal;

		return hitInfo;
	}

	//------------------------------------------------------------------------------------------------
	//! Called when the object receives damage, return false if damage should be ignored
	bool GetCanBeDamaged()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Called when the object should be destroyed and various effects or other things need to be performed (actual destruction handled HandleDestruction())
	void QueueDestroy()
	{
	}

	//------------------------------------------------------------------------------------------------
	//! Handle destruction
	void HandleDestruction()
	{
		if (GetOwner().IsDeleted())
			return;
		
		SCR_DestructionDamageManagerComponentClass componentData = SCR_DestructionDamageManagerComponentClass.Cast(GetComponentData(GetOwner()));
		if (!componentData)
			return;
		
		SCR_DestructionUtility.SpawnDestroyObjects(GetOwner(), componentData.m_DestroySpawnObjects, GetDestructionHitInfo());
		DeleteDestructibleDelayed();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Initialize destruction
	void InitDestruction()
	{
	}
	
	//------------------------------------------------------------------------------------------------
	//! Receive encoded hit data from server
	void NetReceiveHitData(int hitIndex, EDamageType damageType, float damage, vector hitPosition, vector hitDirection)
	{
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called when Item is initialized from replication stream. Carries the data from Master.
	void NetReadInit(ScriptBitReader reader)
	{
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called when Item is getting replicated from Master to Slave connection.
	void NetWriteInit(ScriptBitWriter writer)
	{
	}
	
	//------------------------------------------------------------------------------------------------
	void ReplicateDestructibleState(int damagePhase = 0, bool silent = false)
	{
	}
	
	//------------------------------------------------------------------------------------------------
	//! Queue destruction on clients
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	private void RPC_QueueDestroy(bool totalDestruction, float lastHealth, float hitDamage, EDamageType damageType, vector hitPosition, vector hitDirection, vector hitNormal)
	{
		SetHitZoneDamage(GetMaxHealth());
		CreateDestructionHitInfo(totalDestruction, lastHealth, hitDamage, damageType, hitPosition, hitDirection, hitNormal);
		QueueDestroy();
		
		HandleDestruction();
	}
	
	//------------------------------------------------------------------------------------------------
	protected RplComponent FindParentRplComponent()
	{
		IEntity parent = GetOwner().GetParent();
		RplComponent currentRplComponent;
		while (parent)
		{
			currentRplComponent = RplComponent.Cast(parent.FindComponent(RplComponent));
			parent = parent.GetParent();
		}
		
		return currentRplComponent;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void PassDamageToChildren(notnull BaseDamageContext damageContext)
	{
		IEntity child = GetOwner().GetChildren();
		while (child)
		{
			SCR_DestructionDamageManagerComponent destructionComponent = SCR_DestructionDamageManagerComponent.Cast(child.FindComponent(SCR_DestructionDamageManagerComponent));
			if (!destructionComponent)
			{
				child = child.GetSibling();
				continue;
			}
			
			IEntity currentChild = child;
			child = child.GetSibling();
			
			BaseDamageContext childContext = BaseDamageContext.Cast(damageContext.Clone());
			childContext.hitEntity = currentChild;
			childContext.struckHitZone = destructionComponent.GetDefaultHitZone();

			destructionComponent.HandleDamage(childContext);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Checks if this entity is locally owned
	bool IsProxy()
	{
		RplComponent rplComponent = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		return (rplComponent && rplComponent.IsProxy());
	}

	//------------------------------------------------------------------------------------------------
	//! No contacts for chimeraCharacters
	override bool FilterContact(IEntity owner, IEntity other, Contact contact)
	{
		if (!other)
			return false;

		if (!owner.GetPhysics() || !other.GetPhysics())
			return false;

		if(!contact.Physics2.IsDynamic())
			return false;

		if (ChimeraCharacter.Cast(other))
			return false;

		//we call super here, because a) we want to first check for characters as it's cheaper, and b) we need to do other checks after super to not do them unnecessarily
		if (!super.FilterContact(owner, other, contact))
			return false;

		//checks that were previously in OnFilteredContact
		if (GetDestroyed() || !GetCanBeDamaged())
			return false;

		if (other && other.IsInherited(SCR_DebrisSmallEntity)) // Ignore impacts from debris
			return false;

		//this is also called in OnFilteredContact to get the actual component data, but we check here as well to avoid calling script in case they dont exist
		SCR_DestructionDamageManagerComponentClass componentData = SCR_DestructionDamageManagerComponentClass.Cast(GetComponentData(GetOwner()));
		if (!componentData)
			return false;

		// Now get the relative force, which is the impulse divided by the mass of the dynamic object
		float relativeForce = contact.Impulse / contact.Physics2.GetMass();
		if (relativeForce < componentData.m_fRelativeContactForceThresholdMinimum) // Below minimum threshold, ignore
			return false;

		// Check if enough time has passed since the last collision (1 second = 1000 milliseconds)
		int currentTime = GetGame().GetWorld().GetWorldTime();
		if (m_iLastCollisionTime > 0 && currentTime - m_iLastCollisionTime < 1000)
			return false;

		// Update the last collision time
		m_iLastCollisionTime = currentTime;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnFilteredContact(IEntity owner, IEntity other, Contact contact)
	{
		Physics ownerPhysics = owner.GetPhysics();
		Physics otherPhysics = other.GetPhysics();
		float damage = GetMaxHealth();
		EDamageType damageType = EDamageType.TRUE;

		// If vehicle response index is smaller -> deal damage
		// Otherwise -> destroy
		if (otherPhysics.GetResponseIndex() - MIN_MOMENTUM_RESPONSE_INDEX < ownerPhysics.GetResponseIndex() - MIN_DESTRUCTION_RESPONSE_INDEX)
		{
			SCR_DestructionDamageManagerComponentClass componentData = SCR_DestructionDamageManagerComponentClass.Cast(GetComponentData(GetOwner()));
			float momentum = SCR_DestructionUtility.CalculateMomentum(contact, ownerPhysics.GetMass(), otherPhysics.GetMass());
			damage = momentum * componentData.m_fMomentumToDamageScale;
			damageType = EDamageType.COLLISION;
		}

		vector outMat[3];
		vector relVel = contact.VelocityBefore2 - contact.VelocityBefore1;
		outMat[0] = contact.Position; // Hit position
		outMat[1] = relVel.Normalized(); // Hit direction
		outMat[2] = contact.Normal; // Hit normal

		SCR_DamageContext damageContext = new SCR_DamageContext(damageType, damage, outMat, GetOwner(), null,Instigator.CreateInstigator(other), null, -1, -1);
		damageContext.damageEffect = new SCR_CollisionDamageEffect();
		HandleDamage(damageContext);
	}

	//------------------------------------------------------------------------------------------------
	override void OnDamage(notnull BaseDamageContext damageContext)
	{
		super.OnDamage(damageContext);

		SCR_DestructionDamageManagerComponentClass componentData = SCR_DestructionDamageManagerComponentClass.Cast(GetComponentData(GetOwner()));
		if (!componentData)
			return;

		if (componentData.m_bPassDamageToChildren)
			PassDamageToChildren(damageContext);

		if (m_iDestructionBaseDataIndex != -1)
		{
			ScriptInvoker onDamage = GetDestructionBaseData().GetOnDamage(false);
			if (onDamage)
				onDamage.Invoke();
		}

		if (IsProxy())
			return;

		Physics physics = GetOwner().GetPhysics();
		if (physics)
		{
			if (!GetDestroyed())
			{
				GetDestructionBaseData().SetPreviousHealth(GetHealth());
				SCR_DestructionUtility.UpdateResponseIndex(physics, GetHealth(), GetMaxHealth());
				return;
			}

			if (physics.IsDynamic() && !physics.IsActive()) // Wake the object up if sleeping
				physics.ApplyImpulse(vector.Up * physics.GetMass() * 0.001); // Applying impulse instead of SetActive, as we don't want to override ActiveState.ALWAYS_ACTIVE if set

			// Should disable physics on destroy
			if (GetDisablePhysicsOnDestroy())
				physics.SetInteractionLayer(EPhysicsLayerDefs.VehicleCast);
		}

		float previousHealth;
		if (m_iDestructionBaseDataIndex != -1)
			previousHealth = GetDestructionBaseData().GetPreviousHealth();

		CreateDestructionHitInfo(damageContext.damageValue >= componentData.m_fDamageThresholdMaximum, previousHealth, damageContext.damageValue, damageContext.damageType, damageContext.hitPosition, damageContext.hitDirection, damageContext.hitNormal);

		QueueDestroy();

		SCR_DestructionHitInfo destructionHitInfo = GetDestructionHitInfo();
		if (!destructionHitInfo)
			return;

		Rpc(RPC_QueueDestroy, destructionHitInfo.m_TotalDestruction, destructionHitInfo.m_LastHealth, destructionHitInfo.m_HitDamage, destructionHitInfo.m_DamageType, destructionHitInfo.m_HitPosition, destructionHitInfo.m_HitDirection, destructionHitInfo.m_HitNormal);

		if (!IsDestructionQueued())
		{
			HandleDestruction();
			GetDestructionBaseData().SetDestructionQueued(true);
		}
	}

	//------------------------------------------------------------------------------------------------
	bool IsDestructionQueued()
	{
		SCR_DestructionBaseData data = GetDestructionBaseData();
		return data.GetDestructionQueued();
	}

	//------------------------------------------------------------------------------------------------
	float GetTotalDestructionThreshold()
	{
		SCR_DestructionDamageManagerComponentClass componentData = SCR_DestructionDamageManagerComponentClass.Cast(GetComponentData(GetOwner()));
		if (!componentData)
			return 3000;

		return componentData.m_fDamageThresholdMaximum;
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.CONTACT);

		if (!GetDefaultHitZone())
			return;

		SCR_DestructionDamageManagerComponentClass componentData = SCR_DestructionDamageManagerComponentClass.Cast(GetComponentData(owner));
		if (!componentData)
		{
			Print("Component data is null!", LogLevel.ERROR);
			return;
		}

		SetHitZoneHealth(componentData.m_fBaseHealth);
		InitDestruction();
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_DestructionDamageManagerComponent()
	{
#ifdef WORKBENCH
		s_bPrintMissingComponent = false;
		s_bPrintMissingPlayerController = false;
		s_bPrintInitializationFailed = false;
#endif

		if (m_iDestructionBaseDataIndex != -1)
			FreeDestructionBaseData(m_iDestructionBaseDataIndex);
	}
#endif
}
