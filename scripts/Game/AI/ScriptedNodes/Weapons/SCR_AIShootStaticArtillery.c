class SCR_AIShootStaticArtillery : AITaskScripted
{
	// Inputs
	protected static const string PORT_ARTILLERY_ENTITY = "ArtilleryEntity";
	protected static const string PORT_AMMO_ENTITY = "AmmoEntity";
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		IEntity myEntity = owner.GetControlledEntity();
		if (!myEntity)
			return ENodeResult.FAIL;
		
		IEntity ammoEntity, artilleryEntity;
		GetVariableIn(PORT_ARTILLERY_ENTITY, artilleryEntity);
		GetVariableIn(PORT_AMMO_ENTITY, ammoEntity);
		
		if (!ammoEntity || !artilleryEntity)
			return ENodeResult.FAIL;
		
		ActionsManagerComponent actionsManager = ActionsManagerComponent.Cast(artilleryEntity.FindComponent(ActionsManagerComponent));
		if (!actionsManager)
			return ENodeResult.FAIL;
		
		array<BaseUserAction> acts = {};
		actionsManager.GetActionsList(acts);
		
		foreach (BaseUserAction act : acts) 
		{
			SCR_MortarShootAction shootAction = SCR_MortarShootAction.Cast(act);
			
			if (!shootAction)
				continue;
			
			bool canBeShown = shootAction.CanBeShownScript(myEntity);
			bool canBePerformed = shootAction.CanBePerformedScript(myEntity);
			
			if (canBeShown && canBePerformed)
			{
				shootAction.PerformAction(ammoEntity, myEntity);
				return ENodeResult.SUCCESS;
			}
		}
		
		return ENodeResult.FAIL;
	};
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		PORT_ARTILLERY_ENTITY,
        PORT_AMMO_ENTITY
	};
	override array<string> GetVariablesIn()
    {
        return s_aVarsIn;
    }
	//------------------------------------------------------------------------------------------------
	override static string GetOnHoverDescription()
	{
		return "Use the provided mortar shell entity on mortar entity. Used for firing the mortar.";
	}
	protected static override bool VisibleInPalette(){ return true; }
};