class SCR_AICheckLowAmmo : AITaskScripted
{
	protected const static string WEAPON_COMPONENT_PORT = "WeaponComponent";	
	protected const static string MUZZLE_ID_PORT = "MuzzleId";
	protected const static string LOW_AMMO_PORT = "LowAmmo";
	
	protected SCR_AICombatComponent m_CombatComp;
	
	protected static ref TStringArray s_aVarsIn = {WEAPON_COMPONENT_PORT, MUZZLE_ID_PORT};
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	
	protected static ref TStringArray s_aVarsOut = {LOW_AMMO_PORT};
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	//-------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		IEntity controlled = owner.GetControlledEntity();
		if (!controlled)
			return;
		m_CombatComp = SCR_AICombatComponent.Cast(controlled.FindComponent(SCR_AICombatComponent));
	}
	
	//-------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		BaseWeaponComponent weaponComp;
		int muzzleId;
		
		// Output
		bool lowAmmo = false;
		
		GetVariableIn(WEAPON_COMPONENT_PORT, weaponComp);
		GetVariableIn(MUZZLE_ID_PORT, muzzleId);
		
		if (weaponComp && muzzleId != -1)
		{
			lowAmmo = m_CombatComp.EvaluateLowAmmo(weaponComp, muzzleId);
		}
		
		SetVariableOut(LOW_AMMO_PORT, lowAmmo);
		return ENodeResult.SUCCESS;
	}
	
	//-------------------------------------------------------------------------------
	static override bool VisibleInPalette() { return true; }
}