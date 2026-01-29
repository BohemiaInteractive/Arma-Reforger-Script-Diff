[BaseContainerProps()]
class SCR_AIDangerReaction_UnsafeArea : SCR_AIDangerReaction
{
	override bool PerformReaction(notnull SCR_AIUtilityComponent utility, notnull SCR_AIThreatSystem threatSystem, AIDangerEvent dangerEvent, int dangerEventCount)
	{	
		SCR_AIDangerEvent_UnsafeArea unsafeAreaEvent = SCR_AIDangerEvent_UnsafeArea.Cast(dangerEvent);
		if (!unsafeAreaEvent)
			return false;
		
		vector unsafePos = unsafeAreaEvent.GetPosition();
		float unsafeRadius = unsafeAreaEvent.GetRadius();
		float distance = vector.Distance(unsafePos, utility.GetOrigin());
		
		if (distance > unsafeRadius)
			return false;
		
		float distanceToMove = (unsafeRadius - distance) * -1;		
		
		SCR_AIMoveFromUnsafeAreaBehavior moveBehavior = new SCR_AIMoveFromUnsafeAreaBehavior(utility, null, unsafePos, null, distanceToMove);
		utility.AddAction(moveBehavior);		
		
		return true;
	}
};
