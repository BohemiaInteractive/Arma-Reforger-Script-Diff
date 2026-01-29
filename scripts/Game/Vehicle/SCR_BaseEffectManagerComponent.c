//class SCR_BaseEffectManagerComponentClass : MotorExhaustEffectComponentClass
class SCR_BaseEffectManagerComponentClass : ScriptComponentClass
{
};

//------------------------------------------------------------------------------------------------
//TODO: Get rid of MotorExhaustEffectComponent typename
//this class must be independent on vehicles
//the base class is meant to work with any type of object
//class SCR_BaseEffectManagerComponent : MotorExhaustEffectComponent
class SCR_BaseEffectManagerComponent : ScriptComponent
{
	[Attribute()]
	protected ref array<ref SCR_EffectModule> m_aEffectModules;			//array of all effect modules
	protected ref array<ref SCR_EffectModule> m_aEffectModulesActive = {};		//array of all ACTIVATED effect modules
	protected IEntity m_pOwner;													//the entity which is owner of this component
	protected bool m_bInitiated = false;
	protected const bool ON_PHYSIC_ONLY = false;
	protected const bool ON_FRAME = true;
	
	//------------------------------------------------------------------------------------------------
	//USER METHODS
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	protected void DeactivateModule(SCR_EffectModule pModule)
	{
		m_aEffectModulesActive.RemoveItem(pModule);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ActivateModule(SCR_EffectModule pModule)
	{
		if (m_aEffectModulesActive.Find(pModule) == -1)
		{
			pModule.Init(this);
			m_aEffectModulesActive.Insert(pModule);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Activate all modules which should be activated. Parameter decides if it should be activated on physics only or tick each frame
	protected void ActivateAllModules(bool bPhysics)
	{
		foreach (SCR_EffectModule pModule : m_aEffectModules)
		{
			if (!pModule)
				continue;
			if (pModule.GetTickOnFrame() == bPhysics)
				ActivateModule(pModule);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Deactivate all modules which should be deactivated. Parameter decides if it should be activated on physics only or tick each frame
	protected void DeactivateAllModules(bool bPhysics)
	{
		for (int i = 0; i < m_aEffectModulesActive.Count(); i++)
		{
			if (!m_aEffectModulesActive[i])
				continue;
			if (m_aEffectModulesActive[i].GetTickOnFrame() == bPhysics)
				DeactivateModule(m_aEffectModulesActive[i]);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void InitModules(bool bTickOnFrame = false)
	{
		foreach (SCR_EffectModule pModule : m_aEffectModules)
		{
			if (!pModule)
				continue;
			if (m_aEffectModulesActive.Find(pModule) != -1)
				continue;	//this module is already activated
			if (pModule.GetTickOnFrame() == bTickOnFrame)
			{
				pModule.Init(this);
				ActivateModule(pModule);	//add it to the array of active modules
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//STANDARD METHODS
	//------------------------------------------------------------------------------------------------
	#ifdef WORKBENCH
		//------------------------------------------------------------------------------------------------------------
		override int _WB_GetAfterWorldUpdateSpecs(IEntity owner, IEntitySource src)
		{
			return EEntityFrameUpdateSpecs.CALL_WHEN_ENTITY_VISIBLE;
		}

		//------------------------------------------------------------------------------------------------------------
		override void _WB_AfterWorldUpdate(IEntity owner, float timeSlice)
		{	
			foreach (SCR_EffectModule pModule : m_aEffectModules)
		{
			if (!pModule)
				continue;
			pModule.WB_Update(owner, timeSlice);
		}
		}
	#endif
	
	//------------------------------------------------------------------------------------------------
	void UpdateModules(float timeSlice)
	{
		foreach (SCR_EffectModule pModule : m_aEffectModulesActive)
		{
			if (!pModule)
				continue;
			pModule.Update(m_pOwner, timeSlice);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		ActivateAllModules(ON_FRAME);
	}
	
	override void EOnPhysicsActive(IEntity owner, bool activeState)
	{
		if (System.IsConsoleApp())	//don't run it on DS
			return;
		
		if (activeState)
		{
			m_pOwner = owner;
			ActivateAllModules(ON_PHYSIC_ONLY);
		
		}
		else
		{
			DeactivateAllModules(ON_PHYSIC_ONLY);
		}
	}
	
	override void EOnDeactivate(IEntity owner)
	{
		super.EOnDeactivate(owner);
		
		DisconnectFromEffectManagerSystem();
	}
	
	override void EOnActivate(IEntity owner)
	{
		super.EOnActivate(owner);
		
		ConnectToEffectManagerSystem();
	}
		
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		m_pOwner = owner;
		
		if (System.IsConsoleApp())	//don't run it on DS
			return;
		
		SetEventMask(owner, EntityEvent.INIT | EntityEvent.PHYSICSACTIVE);
		ConnectToEffectManagerSystem();
	}
	
	override void OnDelete(IEntity owner)
	{
		DisconnectFromEffectManagerSystem();
		
		super.OnDelete(owner);
	}
	
	private void ConnectToEffectManagerSystem()
	{
		World world = m_pOwner.GetWorld();
		EffectManagerSystem updateSystem = EffectManagerSystem.Cast(world.FindSystem(EffectManagerSystem));
		if (!updateSystem)
			return;
		
		updateSystem.Register(this);
	}
	
	private void DisconnectFromEffectManagerSystem()
	{
		World world = m_pOwner.GetWorld();
		EffectManagerSystem updateSystem = EffectManagerSystem.Cast(world.FindSystem(EffectManagerSystem));
		if (!updateSystem)
			return;
		
		updateSystem.Unregister(this);
	}
	
	#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	override void _WB_OnInit(IEntity owner, inout vector mat[4], IEntitySource src)
	{
		foreach (SCR_EffectModule pModule : m_aEffectModules)
		{
			if (!pModule)
				continue;
			pModule.WB_OnInit(owner, mat, src);
		}
	}
	#endif	
};

//------------------------------------------------------------------------------------------------
class SCR_HelicopterEffectManagerComponentClass : SCR_BaseEffectManagerComponentClass
{
};

//------------------------------------------------------------------------------------------------
//Helicopter specific effect class
class SCR_HelicopterEffectManagerComponent : SCR_BaseEffectManagerComponent
{
	//------------------------------------------------------------------------------------------------
	SCR_HelicopterControllerComponent GetHelicopterController()
	{
		if (!m_pOwner)
			return null;		//we don't have owner
		return SCR_HelicopterControllerComponent.Cast(m_pOwner.FindComponent(SCR_HelicopterControllerComponent));
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
	}
};







