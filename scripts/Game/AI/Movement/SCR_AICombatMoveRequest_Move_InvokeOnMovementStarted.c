class SCR_AICombatMoveRequest_Move_InvokeOnMovementStarted : AITaskScripted
{
	protected static const string PORT_REQUEST = "Request";
	protected static const string PORT_POSITION = "Position";
	protected static const string PORT_IS_COVER = "IsCover";
	
	
	//------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		SCR_AICombatMoveRequestBase rqBase;
		GetVariableIn(PORT_REQUEST, rqBase);
		SCR_AICombatMoveRequest_Move rq = SCR_AICombatMoveRequest_Move.Cast(rqBase);
		if (!rq)
			return SCR_AIErrorMessages.NodeErrorCombatMoveRequest(this, owner, rq);
		
		vector pos;
		GetVariableIn(PORT_POSITION, pos);
		
		bool isCover;
		GetVariableIn(PORT_IS_COVER, isCover);
		
		ScriptInvokerBase<SCR_AICombatMoveRequest_Move_MovementEvent> invoker = rq.GetOnMovementStarted(false);
		
		if (invoker)
		{
			SCR_AIUtilityComponent utility = SCR_AIUtilityComponent.Cast(owner.FindComponent(SCR_AIUtilityComponent));
			invoker.Invoke(utility, rq, pos, isCover);
		}
			
		return ENodeResult.SUCCESS;
	}
	
	
	//------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		PORT_REQUEST,
		PORT_POSITION,
		PORT_IS_COVER
	};
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	
	static override bool VisibleInPalette() { return true; }
}