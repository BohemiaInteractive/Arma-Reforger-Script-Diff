class SCR_AIGetCombatComponentWeaponProperties : AITaskScripted
{
	// Output ports
	protected static const string PORT_MIN_DISTANCE = "MinDistance";
	protected static const string PORT_MAX_DISTANCE = "MaxDistance";
	protected static const string PORT_DIRECT_DAMAGE = "IsDirectDamage";

	protected SCR_AICombatComponent m_CombatComponent;
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		IEntity controlledEnt = owner.GetControlledEntity();
		m_CombatComponent = SCR_AICombatComponent.Cast(controlledEnt.FindComponent(SCR_AICombatComponent));
		
		if (!m_CombatComponent)
		{
			NodeError(this, owner, "SCR_AIGetCombatComponentWeaponProperties didn't find necessary components!");
		}
	}
	
	//------------------------------------------------------------------------------------------------
	/*
	It's not very nice that we are moving weapon properties data all the way through weapon selection, combat component, and then this node.
	Ideally we should be able to get all this from weapon, muzzle and magazine directly.
	But ATM this is not possible, therefore it's done this way so that the values are consistent with what weapon selector is using
	to evaluate weapons and targets.
	*/
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{	
		if (!m_CombatComponent)
			return ENodeResult.FAIL;
		
		BaseWeaponComponent weaponComp;
		BaseMagazineComponent magazineComp;
		int muzzleId;
		
		m_CombatComponent.GetSelectedWeaponAndMagazine(weaponComp, muzzleId, magazineComp);
		
		float minDistance;
		float maxDistance;
		bool directDamage;
		m_CombatComponent.GetSelectedWeaponProperties(minDistance, maxDistance, directDamage);
		
		SetVariableOut(PORT_DIRECT_DAMAGE, directDamage);
		SetVariableOut(PORT_MIN_DISTANCE, minDistance);
		SetVariableOut(PORT_MAX_DISTANCE, maxDistance);
		
		return ENodeResult.SUCCESS;
	}
	
	//------------------------------------------------------------------------------------------------
	static override bool VisibleInPalette()
	{
		return true;
	}
	
	protected static ref TStringArray s_aVarsOut = {
		PORT_MIN_DISTANCE,
		PORT_MAX_DISTANCE,
		PORT_DIRECT_DAMAGE
	};
	
	//------------------------------------------------------------------------------------------------
	override TStringArray GetVariablesOut()
	{
		return s_aVarsOut;
	}
}
