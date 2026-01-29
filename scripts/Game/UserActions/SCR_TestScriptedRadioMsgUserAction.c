class SCR_TestScriptedRadioMsgUserAction : ScriptedUserAction
{
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{		
		GenericEntity owner = GenericEntity.Cast(pOwnerEntity);
		BaseRadioComponent radioComp = BaseRadioComponent.Cast(owner.FindComponent(BaseRadioComponent));
		if (!radioComp)
			return;

		BaseTransceiver transmitter = radioComp.GetTransceiver(0);
		if (!transmitter)
			return;

		//ScriptedRadioMessage msg = new ScriptedRadioMessage();
		//SCR_RequestTransportMessage msg = new SCR_RequestTransportMessage();
		//transmitter.BeginTransmission(msg);
	}
}
