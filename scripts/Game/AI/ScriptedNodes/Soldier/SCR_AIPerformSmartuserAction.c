class SCR_AIPerformSmartUserAction : AITaskScripted
{
	[Attribute( defvalue: "", uiwidget: UIWidgets.EditBox, desc: "Insert UserAction class name" )]
	protected string m_sUserAction;
	
	[Attribute( defvalue: "0", uiwidget: UIWidgets.CheckBox, desc: "Perform OnAbort? If false, performs action OnSimulate. If true OnSimulate returns RUNNING" )]
	protected bool m_bPerformOnAbort;
	
	protected bool m_bHasAborted;
	
	static const string USER_ACTION_PORT = "UserAction";
	static const string TARGET_ENTITY_PORT = "TargetEntity";
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		USER_ACTION_PORT,
		TARGET_ENTITY_PORT
	};
	
	//------------------------------------------------------------------------------------------------
	override TStringArray GetVariablesIn()
	{
		return s_aVarsIn;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
			m_bHasAborted = false;
	}

	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
			if (m_bPerformOnAbort)
				return ENodeResult.RUNNING;
			return PerformAction(owner, true);
	}
	
	//------------------------------------------------------------------------------------------------
	private ENodeResult PerformAction(AIAgent owner, bool turnActionOn)
	{
		IEntity targetEntity;
		string userActionString;
		typename userAction;
		GetVariableIn(TARGET_ENTITY_PORT, targetEntity);
		if (!GetVariableIn(USER_ACTION_PORT, userActionString))
			userActionString = m_sUserAction;

		IEntity controlledEntity = owner.GetControlledEntity();
		if (!controlledEntity)
			return ENodeResult.FAIL;

		if (!targetEntity)
			return ENodeResult.FAIL;
		
		userAction = userActionString.ToType();
		if (!userAction)
			return ENodeResult.FAIL;

		array<BaseUserAction> outActions = {};
		ScriptedUserAction action;
		GetActions(targetEntity, outActions);
		foreach (BaseUserAction baseAction : outActions)
		{
			action = ScriptedUserAction.Cast(baseAction);
			if (action && userAction == action.Type() && action.CanBePerformedScript(controlledEntity))
			{
				SCR_UserActionWithOccupancy actionWithOccupancy = SCR_UserActionWithOccupancy.Cast(action);
				if (actionWithOccupancy)
				{
					if (turnActionOn && actionWithOccupancy.IsOccupied()) // action is already ON and we wanted to turn on 
						return ENodeResult.FAIL;
					else if (!turnActionOn && !actionWithOccupancy.IsOccupied()) //  action is not ON and we wanted to turn it off
						return ENodeResult.FAIL;
				};	
				action.PerformAction(targetEntity, controlledEntity);
				return ENodeResult.SUCCESS;
			}
		}
		return ENodeResult.FAIL;
	}
	
	override void OnAbort(AIAgent owner, Node nodeCausingAbort)
	{
		if (m_bPerformOnAbort && !m_bHasAborted)
		{
			ENodeResult result = PerformAction(owner, false);
			m_bHasAborted = true;
			if (result != ENodeResult.SUCCESS)
				ForceAbortAllActions(owner);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ForceAbortAllActions(AIAgent owner)
	{
		// Right now it only aborts loitering; If other smart actions cause the AI to get stuck, they should be added here too
		IEntity controlledEntity = owner.GetControlledEntity();
		if (!controlledEntity)
			return;
		
		SCR_CharacterCommandHandlerComponent cmd = SCR_CharacterCommandHandlerComponent.Cast(controlledEntity.FindComponent(SCR_CharacterCommandHandlerComponent));
		if (cmd && cmd.IsLoitering())
		{
			cmd.StopLoitering(false);
		}
	}
	//-----------------------------------------------------------------------------------------------
	private void GetActions(IEntity targetEntity, notnull out array<BaseUserAction> outActions)
	{
		if (!targetEntity)
			return;
		
		ActionsManagerComponent actionOnEntity = ActionsManagerComponent.Cast(targetEntity.FindComponent(ActionsManagerComponent));
		
		if (!actionOnEntity)
			return;
		
		actionOnEntity.GetActionsList(outActions);
	}
	
	//------------------------------------------------------------------------------------------------
	protected static override bool VisibleInPalette()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static override bool CanReturnRunning()
	{
		return true;
	}
	
	// -----------------------------------------------------------------------------------------------
	protected static override string GetOnHoverDescription()
	{
		return "Performs specified smart action on target entity. Both must be specified. It can either perform it OnSimulate (default) or OnAbort.";
	}
};