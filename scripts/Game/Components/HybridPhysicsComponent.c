[ComponentEditorProps(category: "GameScripted/Physics", description: "Component that automatically handles switching of between static and dynamic physics of the object")]
class SCR_HybridPhysicsComponentClass : ScriptComponentClass
{
}

//	TODO:
//	- Consider implementing the functionality as an extended version of RigidBody component
//	- Also need API to get damping, sleep thresholds etc

//! Class for storing physics setup info for SCR_HybridPhysicsComponent
class SCR_HybridPhysicsInfo
{
	float m_fMass;
	ref array<int> m_aLayerMasks = new array<int>;
}

//------------------------------------------------------------------------------------------------
//! Component that automatically handles switching of between static and dynamic physics of the object
class SCR_HybridPhysicsComponent : ScriptComponent
{
	[Attribute("50", UIWidgets.EditBox, "How large a contact impulse must be to switch the object into dynamic physics")]
	float m_fDynamicContactImpulse;
	
	private ref SCR_HybridPhysicsInfo m_HybridPhysicsInfo = null;
	
	//------------------------------------------------------------------------------------------------
	//! Clear the dynamic physics info
	private void ClearPhysicsInfo()
	{
		if (!m_HybridPhysicsInfo)
			return;
		
		delete m_HybridPhysicsInfo;
		m_HybridPhysicsInfo = null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Create and store dynamic physics info
	//! \param[in] owner
	private void StorePhysicsInfo(IEntity owner)
	{
		auto physics = owner.GetPhysics();
		if (!physics)
			return;
		
		ClearPhysicsInfo();
		
		m_HybridPhysicsInfo = new SCR_HybridPhysicsInfo;
		m_HybridPhysicsInfo.m_fMass = physics.GetMass();
		
		int numGeoms = physics.GetNumGeoms();
		for (int i = 0; i < numGeoms; i++)
		{
			m_HybridPhysicsInfo.m_aLayerMasks.Insert(physics.GetGeomInteractionLayer(i));
		}
		
		physics.Destroy();
		physics = Physics.CreateStatic(owner, -1);
		
		int numStoredGeoms = m_HybridPhysicsInfo.m_aLayerMasks.Count();
		numGeoms = physics.GetNumGeoms();
		for (int i = 0; i < numStoredGeoms; i++)
		{
			if (i >= numGeoms)
				break;
			
			physics.SetGeomInteractionLayer(i, m_HybridPhysicsInfo.m_aLayerMasks.Get(i));
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Apply stored physics info to dynamic physics object
	//! \param[in] owner
	private void ApplyPhysicsInfo(IEntity owner)
	{
		auto physics = owner.GetPhysics();
		if (!physics)
		{
			ClearPhysicsInfo();
			return;
		}
		
		physics.Destroy();
		physics = Physics.CreateDynamic(owner, m_HybridPhysicsInfo.m_fMass, -1);
		int numStoredGeoms = m_HybridPhysicsInfo.m_aLayerMasks.Count();
		int numGeoms = physics.GetNumGeoms();
		for (int i = 0; i < numStoredGeoms; i++)
		{
			if (i >= numGeoms)
				break;
			
			physics.SetGeomInteractionLayer(i, m_HybridPhysicsInfo.m_aLayerMasks.Get(i));
		}
		
		ClearPhysicsInfo();
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnContact(IEntity owner, IEntity other, Contact contact)
	{
		// We got an impulse large enough to switch us to dynamic physics
		if (contact.Impulse < m_fDynamicContactImpulse)
			return;
		
		auto physics = owner.GetPhysics();
		if (physics.IsDynamic())
			return;
		
		// Clear the contact event (not needed anymore)
		ClearEventMask(owner, EntityEvent.CONTACT);
		
		// Add the frame event (this is where we'll switch to dynamic physics)
		SetEventMask(owner, EntityEvent.FRAME);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		// Dynamic but sleeping, so switch back to static
		auto physics = owner.GetPhysics();
		if (physics.IsDynamic() && !physics.IsActive())
		{
			StorePhysicsInfo(owner);
			SetEventMask(owner, EntityEvent.CONTACT);
			ClearEventMask(owner, EntityEvent.FRAME);
			return;
		}
		
		if (m_HybridPhysicsInfo)
			ApplyPhysicsInfo(owner);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.INIT);
		owner.SetFlags(EntityFlags.ACTIVE);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		if (m_fDynamicContactImpulse <= 0)
			return;
		
		auto physics = owner.GetPhysics();
		if (!physics)
			return;
		
		if (!physics.IsDynamic())
			return;
		
		SetEventMask(owner, EntityEvent.CONTACT);
		
		StorePhysicsInfo(owner);
	}
	
	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_HybridPhysicsComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
	}

	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_HybridPhysicsComponent()
	{
		ClearPhysicsInfo();
	}
}
