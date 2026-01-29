/*!
Evaluates an expected weapon at Weapon&Target selector and returns the weapon, magazine and muzzle.
*/
class SCR_AIEvaluateExpectedWeapon : AITaskScripted
{
	// Output ports
	protected static const string PORT_WEAPON_COMPONENT = "WeaponComponent";
	protected static const string PORT_MAGAZINE_COMPONENT = "MagazineComponent";
	protected static const string PORT_MUZZLE_ID = "MuzzleId";
	
	protected SCR_AICombatComponent m_CombatComponent;
	
	//--------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		IEntity controlledEnt = owner.GetControlledEntity();
		m_CombatComponent = SCR_AICombatComponent.Cast(controlledEnt.FindComponent(SCR_AICombatComponent));
		
		if (!m_CombatComponent)
		{
			NodeError(this, owner, "SCR_AIEvaluateExpectedWeapon didn't find necessary components!");
		}
	}
	
	//--------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{	
		if (!m_CombatComponent)
			return ENodeResult.FAIL;
		
		BaseWeaponComponent weaponComp;
		BaseMagazineComponent magazineComp;
		int muzzleId;
		
		m_CombatComponent.EvaluateExpectedWeapon(weaponComp, muzzleId, magazineComp);
		
		if (!weaponComp)
			return ENodeResult.FAIL;
		
		SetVariableOut(PORT_WEAPON_COMPONENT, weaponComp);
		SetVariableOut(PORT_MAGAZINE_COMPONENT, magazineComp);
		SetVariableOut(PORT_MUZZLE_ID, muzzleId);
		
		return ENodeResult.SUCCESS;
	}
	
	//--------------------------------------------------------------------------------------------
	static override bool VisibleInPalette() { return true; }
	
	protected static ref TStringArray s_aVarsOut = {
		PORT_WEAPON_COMPONENT,
		PORT_MUZZLE_ID,
		PORT_MAGAZINE_COMPONENT
	};
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
}