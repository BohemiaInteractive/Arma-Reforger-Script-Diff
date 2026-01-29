//! Waits for a change in combat component's selected or expected weapon value.
//! Returns Running until the change is detected.
//! Returns Success on every change and outputs data to the output port.
//! Also returns Success and outputs data on first run.
class SCR_AIWaitForNewCombatComponentWeapon : AITaskScripted
{
	// Output ports
	protected static const string PORT_WEAPON_COMPONENT = "WeaponComponent";
	protected static const string PORT_MAGAZINE_COMPONENT = "MagazineComponent";
	protected static const string PORT_MUZZLE_ID = "MuzzleId";
	protected static const string PORT_HANDLE_TREE = "WeaponHandlingTree";
	protected static const string PORT_WEAPON_OR_MUZZLE = "WeaponOrMuzzleSwitch";
	
	protected SCR_AICombatComponent m_CombatComponent;
	
	protected BaseWeaponComponent m_WeaponComp;
	protected int m_MuzzleId;
	protected BaseMagazineComponent m_MagazineComp;
	
	bool m_bFirstRun = true;
	
	//--------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		IEntity controlledEnt = owner.GetControlledEntity();
		m_CombatComponent = SCR_AICombatComponent.Cast(controlledEnt.FindComponent(SCR_AICombatComponent));
		
		if (!m_CombatComponent)
		{
			NodeError(this, owner, "SCR_AIGetCombatComponentWeapon didn't find necessary components!");
		}
	}
	
	//--------------------------------------------------------------------------------------------
	override void OnAbort(AIAgent owner, Node nodeCausingAbort)
	{
		m_bFirstRun = true;
	}
	
	//--------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{	
		if (!m_CombatComponent)
			return ENodeResult.FAIL;
		
		BaseWeaponComponent weaponComp;
		BaseMagazineComponent magazineComp;
		int muzzleId;
		
		// Resolve in what mode we operate
		m_CombatComponent.GetSelectedWeaponAndMagazine(weaponComp, muzzleId, magazineComp);
		
		bool weaponEvent = weaponComp != m_WeaponComp || magazineComp != m_MagazineComp || muzzleId != m_MuzzleId;
		bool weaponOrMuzzleSwitch = weaponComp != m_WeaponComp || muzzleId != m_MuzzleId;
		
		if (weaponEvent || m_bFirstRun)
		{
			m_WeaponComp = weaponComp;
			m_MagazineComp = magazineComp;
			m_MuzzleId = muzzleId;
			m_bFirstRun = false;
			
			SetVariableOut(PORT_WEAPON_COMPONENT, weaponComp);
			SetVariableOut(PORT_MAGAZINE_COMPONENT, magazineComp);
			SetVariableOut(PORT_MUZZLE_ID, muzzleId);
			SetVariableOut(PORT_HANDLE_TREE, m_CombatComponent.GetSelectedWeaponResource());
			if (weaponOrMuzzleSwitch)
				SetVariableOut(PORT_WEAPON_OR_MUZZLE, weaponOrMuzzleSwitch);
			return ENodeResult.SUCCESS;
		}
		else
		{
			return ENodeResult.RUNNING;
		}
	}
	
	//--------------------------------------------------------------------------------------------
	static override bool VisibleInPalette()
	{
		return true;
	}
	
	protected static ref TStringArray s_aVarsOut = {
		PORT_WEAPON_COMPONENT,
		PORT_MUZZLE_ID,
		PORT_MAGAZINE_COMPONENT,
		PORT_HANDLE_TREE,
		PORT_WEAPON_OR_MUZZLE
	};

	//------------------------------------------------------------------------------------------------
	override TStringArray GetVariablesOut()
	{
		return s_aVarsOut;
	}
}
