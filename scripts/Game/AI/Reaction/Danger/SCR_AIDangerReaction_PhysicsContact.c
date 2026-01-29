[BaseContainerProps()]
class SCR_AIDangerReaction_PhysicsContact : SCR_AIDangerReaction
{
	override bool PerformReaction(notnull SCR_AIUtilityComponent utility, notnull SCR_AIThreatSystem threatSystem, AIDangerEvent dangerEvent, int dangerEventCount)
	{
		IEntity target = dangerEvent.GetObject();
		
		if (!target)
			return true;
		
		// Ignore if we are in vehicle, we can't do anything
		SCR_AIInfoComponent infoComp = utility.m_AIInfo;
		if (infoComp)
		{
			if (infoComp.HasUnitState(EUnitState.IN_VEHICLE) || infoComp.HasUnitState(EUnitState.IN_TURRET))
				return true;
		}
		
		// Ignore non characters
		CharacterControllerComponent targetCharacterController = CharacterControllerComponent.Cast(target.FindComponent(CharacterControllerComponent));
		if (!targetCharacterController)
			return true;
		
		// Ignore unconscious and dead
		if (targetCharacterController.GetLifeState() != ECharacterLifeState.ALIVE)
			return true;
		
		// Ignore if it's another AI who is activated. We only care about players colliding into us.
		AIControlComponent targetAiControlComp = AIControlComponent.Cast(target.FindComponent(AIControlComponent));
		if (targetAiControlComp && targetAiControlComp.IsAIActivated())
			return true;
				
		if (utility.HasActionOfType(SCR_AIAvoidCharacterBehavior))
			return true;
		
		vector targetVelocity = vector.Zero;
		Physics phy = target.GetPhysics();
		if (phy)
			targetVelocity = phy.GetVelocity();
		
		SCR_AIAvoidCharacterBehavior action = new SCR_AIAvoidCharacterBehavior(utility, null, target.GetOrigin(), targetVelocity);
		utility.AddAction(action);
		
		return true;
	}
}