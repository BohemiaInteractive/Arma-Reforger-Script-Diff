//! User action that ought to be attached to an entity with door component.
//! When performed either opens or closes the door based on the previous state of the door.
class SCR_DoorUserAction : DoorUserAction
{
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{		
		BaseDoorComponent doorComponent = GetDoorComponent();
		if (doorComponent)
		{
			doorComponent.UseDoorAction(pUserEntity);
		}
		
		super.PerformAction(pOwnerEntity, pUserEntity);
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		if (GetDoorComponent())
			return true;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		auto doorComponent = GetDoorComponent();
		// Logic here is flipped since method returns the opposite of what we expect
		if (doorComponent && Math.AbsFloat(doorComponent.GetControlValue()) >= 0.5)
			outName = "#AR-UserAction_Close";
		else
			outName = "#AR-UserAction_Open";
		
		return true;
	}
	
};

class SCR_LadderDoorUserAction : SCR_DoorUserAction
{
	// Kept only for compatibility reasons. It's the same as SCR_DoorUserAction
};