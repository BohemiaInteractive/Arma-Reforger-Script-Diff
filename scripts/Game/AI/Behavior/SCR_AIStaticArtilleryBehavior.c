typedef func SCR_AIStaticArtilleryBehavior_OnArtilleryFired;
void SCR_AIStaticArtilleryBehavior_OnArtilleryFired(SCR_AIStaticArtilleryBehavior behavior, AIAgent agent);

/*
Behavior to operate a static artillery (like a mortar).
Aims artillery, finds nearby arsenal with shells, then performs loop of taking shell and firing.
*/
class SCR_AIStaticArtilleryBehavior : SCR_AIBehaviorBase
{	
	protected ref SCR_BTParam<IEntity> m_ArtilleryEntity = new SCR_BTParam<IEntity>("ArtilleryEntity");
	protected ref SCR_BTParam<vector> m_vTargetPos = new SCR_BTParam<vector>("TargetPos");
	
	// Inventory entity where we will take ammo from. It is found automatically by the behavior
	protected ref SCR_BTParam<IEntity> m_InventoryEntity = new SCR_BTParam<IEntity>("InventoryEntity");
	
	// Ammo prefab, found automatically.
	protected ref SCR_BTParam<ResourceName> m_AmmoPrefab = new SCR_BTParam<ResourceName>("AmmoPrefab");
	
	// Counter of how many shots were made by this behavior
	protected int m_iShotCount = 0;
	
	// Ammo type which we were requested to use
	protected SCR_EAIArtilleryAmmoType m_eAmmoType;
	
	protected ref ScriptInvokerBase<SCR_AIStaticArtilleryBehavior_OnArtilleryFired> m_OnArtilleryFired;
	
	//------------------------------------------------------------------------------------------------------------------------------------------
	void InitParameters(IEntity artilleryEntity, vector targetPos)
	{
		m_ArtilleryEntity.Init(this, artilleryEntity);
		m_vTargetPos.Init(this, targetPos);
		m_InventoryEntity.Init(this, null);
		m_AmmoPrefab.Init(this, string.Empty);
	}
	//------------------------------------------------------------------------------------------------------------------------------------------
	void SCR_AIStaticArtilleryBehavior(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity, IEntity artilleryEntity, vector targetPos, SCR_EAIArtilleryAmmoType ammoType, float priorityLevel = PRIORITY_LEVEL_NORMAL)
	{
		InitParameters(artilleryEntity, targetPos);
		m_eAmmoType = ammoType;
		
		m_sBehaviorTree = "{2CB767812037EDDE}AI/BehaviorTrees/Chimera/Soldier/StaticArtilleryBehavior.bt";
		SetPriority(PRIORITY_BEHAVIOR_STATIC_ARTILLERY);
		m_fPriorityLevel.m_Value = priorityLevel;
		m_bAllowLook = false; // Looking elsewhere is forbidden since it interferes with mortar aiming
		
		if (artilleryEntity)
		{
			// Subscribe to deletion event. It can be deleted if destroyed.
			SCR_AIVehicleUsageComponent vehicleUsageComp = SCR_AIVehicleUsageComponent.Cast(m_ArtilleryEntity.m_Value.FindComponent(SCR_AIVehicleUsageComponent));
			if (vehicleUsageComp)
				vehicleUsageComp.GetOnDeleted().Insert(OnArtilleryEntityDeleted);
		}
	}
	
	//------------------------------------------------------------------------------------------------------------------------------------------
	//! Returns event which is invoked every time AI who executes this behavior fires an artillery
	ScriptInvokerBase<SCR_AIStaticArtilleryBehavior_OnArtilleryFired> GetOnArtilleryFired()
	{
		if (!m_OnArtilleryFired)
			m_OnArtilleryFired = new ScriptInvokerBase<SCR_AIStaticArtilleryBehavior_OnArtilleryFired>();
		return m_OnArtilleryFired;
	}
	
	//------------------------------------------------------------------------------------------------------------------------------------------
	int GetShotCount()
	{
		return m_iShotCount;
	}
	
	//------------------------------------------------------------------------------------------------------------------------------------------
	//! Called by the SCR_AIStaticArtilleryBehavior_ShootingLogic node from behavior tree
	void Internal_OnArtilleryFired()
	{
		m_iShotCount++;
		
		if (m_OnArtilleryFired)
			m_OnArtilleryFired.Invoke(this, m_Utility.GetAIAgent());
	}
	
	//------------------------------------------------------------------------------------------------------------------------------------------
	override void OnActionSelected()
	{
		if (!InitAmmoPrefabValue())
		{
			Print(string.Format("SCR_AIStaticArtilleryBehavior: Failed to resolve ammo prefab of type %1 for artillery entity: %2",
				typename.EnumToString(SCR_EAIArtilleryAmmoType, m_eAmmoType), m_ArtilleryEntity.m_Value), LogLevel.ERROR);
			Fail();
			return;
		}
		
		// Temporary, prevent max LOD to keep the agent operate the artillery
		AIAgent agent = m_Utility.GetOwner();
		agent.PreventMaxLOD();
	}
	
	//------------------------------------------------------------------------------------------------------------------------------------------
	override void OnActionDeselected()
	{
		AIAgent agent = m_Utility.GetOwner();
		agent.AllowMaxLOD();
	}
	
	//------------------------------------------------------------------------------------------------------------------------------------------
	void OnArtilleryEntityDeleted(SCR_AIVehicleUsageComponent comp)
	{
		SetFailReason(EAIActionFailReason.ENTITY_DELETED);
		Fail();
	}
	
	//------------------------------------------------------------------------------------------------------------------------------------------
	//! Finds which prefab is used by the muzzle
	bool InitAmmoPrefabValue()
	{
		if (!m_ArtilleryEntity.m_Value)
			return false;
		
		// The artillery gun should have this component, which will resolve ammo type
		SCR_AIStaticArtilleryVehicleUsageComponent artilleryComp = SCR_AIStaticArtilleryVehicleUsageComponent.Cast(m_ArtilleryEntity.m_Value.FindComponent(SCR_AIStaticArtilleryVehicleUsageComponent));
		if (!artilleryComp)
			return false;
		
		ResourceName ammoPrefab = artilleryComp.GetAmmoResourceName(m_eAmmoType);
		if (ammoPrefab.IsEmpty())
			return false;
		
		m_AmmoPrefab.m_Value = ammoPrefab;
		return true;
	}
	
	//------------------------------------------------------------------------------------------------------------------------------------------
	override string GetActionDebugInfo()
	{
		return this.ToString() + " Operating " + m_ArtilleryEntity.m_Value.ToString();
	}
};

class SCR_AIGetStaticArtilleryBehaviorParameters : SCR_AIGetActionParameters
{
	static ref TStringArray s_aVarsOut = (new SCR_AIStaticArtilleryBehavior(null, null, null, vector.Zero, 0)).GetPortNames();
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	protected static override bool VisibleInPalette() { return true; }
};

class SCR_AISetStaticArtilleryBehaviorParameters : SCR_AISetActionParameters
{
	static ref TStringArray s_aVarsIn = (new SCR_AIStaticArtilleryBehavior(null, null, null, vector.Zero, 0)).GetPortNames();
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	
	protected static override bool VisibleInPalette() { return true; }
}

/*
Special node which is executed in static artillery behavior right after gunshot.
*/
class SCR_AIStaticArtilleryBehavior_ShootingLogic : AITaskScripted
{
	// Outputs
	protected static const string PORT_AIMING_NEEDED = "AimingNeeded";
	
	protected SCR_AIUtilityComponent m_Utility;
	
	[Attribute("1", UIWidgets.EditBox, desc: "How many shots will be made before aiming the mortar again")]
	protected int m_iShotsBetweenAiming;
	
	//-----------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		m_Utility = SCR_AIUtilityComponent.Cast(owner.FindComponent(SCR_AIUtilityComponent));
	}
	
	//-----------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_Utility)
			return ENodeResult.FAIL;
		
		SCR_AIStaticArtilleryBehavior behavior = SCR_AIStaticArtilleryBehavior.Cast(m_Utility.GetExecutedAction());
		if (!behavior)
			return ENodeResult.FAIL;
		
		// Should we aim it again?
		int shotCount = behavior.GetShotCount();
		bool aimingNeeded = (shotCount > 0) &&
								(m_iShotsBetweenAiming > 0) &&
								((shotCount % m_iShotsBetweenAiming) == 0);
		
		SetVariableOut(PORT_AIMING_NEEDED, aimingNeeded);
		
		// Notify behavior
		behavior.Internal_OnArtilleryFired();
		
		return ENodeResult.SUCCESS;
	}
	
	//-----------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = { PORT_AIMING_NEEDED };
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	override static bool VisibleInPalette() { return true; }
}