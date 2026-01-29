/*!
Base class of node which gets action parameters from a BT node.

Please refer to SCR_AIGetActionParameters documentation to understand how this class works.
*/

class SCR_AISetActionParameters : SCR_AIActionTask
{
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		SCR_AIActionBase action = GetExecutedAction();
		
		if (!action)
			return ENodeResult.FAIL;
		
		action.GetParametersFromBTVariables(this);
		return ENodeResult.SUCCESS;
	}

	//------------------------------------------------------------------------------------------------
	static override bool VisibleInPalette() { return false; }
};


class SCR_AISetMoveActivityParameters : SCR_AISetActionParameters
{
	protected static ref TStringArray s_aVarsIn = (new SCR_AIMoveActivity(null, null, vector.Zero, null)).GetPortNames();	
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	static override bool VisibleInPalette() { return true; }
};

class SCR_AISetGetInActivityParameters : SCR_AISetActionParameters
{
	protected static ref TStringArray s_aVarsIn = (new SCR_AIGetInActivity(null, null, null)).GetPortNames();
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	static override bool VisibleInPalette() { return true; }
};

class SCR_AISetDefendActivityParameters : SCR_AISetActionParameters
{
	protected static ref TStringArray s_aVarsIn = (new SCR_AIDefendActivity(null, null, vector.Zero)).GetPortNames();	
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	static override bool VisibleInPalette() { return true; }
};

class SCR_AISetAttackBehaviorParameters : SCR_AISetActionParameters
{
	protected static ref TStringArray s_aVarsIn = (new SCR_AIAttackBehavior(null, null, null, null)).GetPortNames();
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	static override bool VisibleInPalette() { return true; }
};