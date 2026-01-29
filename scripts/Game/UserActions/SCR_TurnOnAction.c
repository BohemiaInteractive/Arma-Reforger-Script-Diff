class SCR_TurnOnAction : TurnOnUserAction
{			
	//---------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{		
		RadioBroadcastComponent broadcastComponent = GetRadioBCComponent();
		if (broadcastComponent)
		{	
			if(broadcastComponent.GetState())
				broadcastComponent.EnableRadio(false);
			else
				broadcastComponent.EnableRadio(true);
		}	
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		RadioBroadcastComponent broadcastComponent = GetRadioBCComponent();
		if (broadcastComponent)
			return true;
		
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBroadcastScript()
	{		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		RadioBroadcastComponent broadcastComponent = GetRadioBCComponent();
		if (!broadcastComponent)
		 return false;

		if (broadcastComponent.GetState())
			outName = "#AR-UserAction_TurnOff";
		else
			outName = "#AR-UserAction_TurnOn";

		return true;
	}
};