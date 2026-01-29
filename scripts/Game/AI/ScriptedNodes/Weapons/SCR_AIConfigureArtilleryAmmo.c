class SCR_AIConfigureArtilleryAmmo : AITaskScripted
{
	// Inputs
	protected static const string PORT_AMMO_ENTITY = "AmmoEntity";
	protected static const string PORT_INIT_SPEED_ID = "InitSpeedId";
	
	
	//---------------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		IEntity ammoEntity;
		int initSpeedId = 0;
		if (!GetVariableIn(PORT_AMMO_ENTITY, ammoEntity))
			return ENodeResult.FAIL;
		if (!ammoEntity)
			return ENodeResult.FAIL;
		
		if (!GetVariableIn(PORT_INIT_SPEED_ID, initSpeedId))
			return ENodeResult.FAIL;
		
		//Print(string.Format("SCR_AIConfigureArtilleryAmmo: Setting initial speed index %1 for ammo entity %2", initSpeedId, ammoEntity));
		ConfigureAmmoEntity(ammoEntity, initSpeedId);
		
		return ENodeResult.SUCCESS;
	}
	
	//---------------------------------------------------------------------------------------------------------
	protected bool ConfigureAmmoEntity(notnull IEntity ammoEntity, int configurationId)
	{
		SCR_MortarShellGadgetComponent shellComp = SCR_MortarShellGadgetComponent.Cast(ammoEntity.FindComponent(SCR_MortarShellGadgetComponent));
		
		if (!shellComp)
			return false;
		
		shellComp.SetChargeRingConfig(configurationId);
		
		return true;
	}
	
	//---------------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = { PORT_AMMO_ENTITY, PORT_INIT_SPEED_ID };
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	
	override static string GetOnHoverDescription() { return "Configures artillery ammo entity. Now it only sets initial speed configuration."; }
	
	override static bool VisibleInPalette() { return true; }
}