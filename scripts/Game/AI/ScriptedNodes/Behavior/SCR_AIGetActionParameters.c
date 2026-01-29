/*!
Base class of node which outputs action parameters to Behavior Tree variables.

If you need to expose action parameters to a BT, you must create a scripted node class which inherits from SCR_AIGetActionParameters.

The action class must use SCR_BTParam for variables which must be exposed to Behavior Trees.

For example:
class SCR_AIGetFlyHelicopterParameters : SCR_AIGetActionParameters
{
	static ref TStringArray s_aVarsOut = (new SCR_AIFlyHelicopterBehavior(null, false)).GetPortNames(); // (1)
	override TStringArray GetVariablesOut() { return s_aVarsOut; }										//
	protected static override bool VisibleInPalette() { return true; }											// (2)
};

(1) We need to create an action object once to get port names. We store the resulting array into a static variable.
Please ensure that constructor does not call any other code which is inaccessible during script compilation!

(2) Remember to override VisibleInPallete!
*/

class SCR_AIGetActionParameters : SCR_AIActionTask
{
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		SCR_AIActionBase action = GetExecutedAction();
		
		if (!action)
			return ENodeResult.FAIL;
		
		action.SetParametersToBTVariables(this);
		
		return ENodeResult.SUCCESS;
	}

	//------------------------------------------------------------------------------------------------
	protected static override bool VisibleInPalette() { return false; }
};



//------------------------------------------------------------------------------------------------
class SCR_AIGetGetInVehicleBehaviorParameters : SCR_AIGetActionParameters
{
	static ref TStringArray s_aVarsOut = (new SCR_AIGetInVehicle(null, null, null, null)).GetPortNames();
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	protected static override bool VisibleInPalette() { return true; }
};

class SCR_AIGetOutVehicleBehaviorParameters : SCR_AIGetActionParameters
{
	static ref TStringArray s_aVarsOut = (new SCR_AIGetOutVehicle(null, null, null)).GetPortNames();
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	protected static override bool VisibleInPalette() { return true; }
};

class SCR_AIGetGetInActivityParameters : SCR_AIGetActionParameters
{
	static ref TStringArray s_aVarsOut = (new SCR_AIGetInActivity(null, null, null)).GetPortNames();
	override TStringArray GetVariablesOut()
	{
		return s_aVarsOut;
	}
	
	protected static override bool VisibleInPalette() { return true; }
};

class SCR_AIGetGetOutActivityParameters : SCR_AIGetActionParameters
{
	static ref TStringArray s_aVarsOut = (new SCR_AIGetOutActivity(null, null, null, null)).GetPortNames();
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	protected static override bool VisibleInPalette() { return true; }
};

class SCR_AIGetMoveActivityParameters : SCR_AIGetActionParameters
{
	static ref TStringArray s_aVarsOut = (new SCR_AIMoveActivity(null, null, vector.Zero, null)).GetPortNames();
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	static override bool VisibleInPalette() { return true; }
};


class SCR_AIGetFollowActivityParameters : SCR_AIGetActionParameters
{
	static ref TStringArray s_aVarsOut = (new SCR_AIFollowActivity(null, null, vector.Zero, null)).GetPortNames();
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	static override bool VisibleInPalette() { return true; }
};

class SCR_AIGetDefendActivityParameters : SCR_AIGetActionParameters
{
	static ref TStringArray s_aVarsOut = (new SCR_AIDefendActivity(null, null, vector.Zero)).GetPortNames();
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	static override bool VisibleInPalette() { return true; }
};

class SCR_AIGetDefendBehaviorParameters : SCR_AIGetActionParameters
{
	static ref TStringArray s_aVarsOut = (new SCR_AIDefendBehavior(null, null, null, vector.Zero, 0)).GetPortNames();
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	static override bool VisibleInPalette() { return true; }
};

class SCR_AIGetAttackBehaviorParameters : SCR_AIGetActionParameters
{
	static ref TStringArray s_aVarsOut = (new SCR_AIAttackBehavior(null, null, null, null)).GetPortNames();
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	static override bool VisibleInPalette() { return true; }
};

class SCR_AIGetPerformActionBehaviorParameters : SCR_AIGetActionParameters
{	
	static ref TStringArray s_aVarsOut = (new SCR_AIPerformActionBehavior(null, null, null)).GetPortNames();
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	static override bool VisibleInPalette() { return true; }
};

class SCR_AIGetPerformActionActivityParameters : SCR_AIGetActionParameters
{	
	static ref TStringArray s_aVarsOut = (new SCR_AIPerformActionActivity(null, null, null, string.Empty)).GetPortNames();
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	static override bool VisibleInPalette() { return true; }
};

class SCR_AIGetResupplyActivityParameters : SCR_AIGetActionParameters
{
	static ref TStringArray s_aVarsOut = (SCR_AIResupplyActivity(null, null, null, Class)).GetPortNames();
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	static override bool VisibleInPalette() { return true; }
};

class SCR_AIGetInvestigateBehaviorParameters : SCR_AIGetActionParameters
{
	static ref TStringArray s_aVarsOut = (new SCR_AIMoveAndInvestigateBehavior(null, null, vector.Zero)).GetPortNames();
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	static override bool VisibleInPalette() { return true; }
};

class SCR_AIGetHealWaitBehaviorParameters : SCR_AIGetActionParameters
{
	static ref TStringArray s_aVarsOut = (new SCR_AIHealWaitBehavior(null, null, null)).GetPortNames();
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	static override bool VisibleInPalette() { return true; }
};

class SCR_AIGetMedicHealBehaviorParameters : SCR_AIGetActionParameters
{
	static ref TStringArray s_aVarsOut = (new SCR_AIMedicHealBehavior(null, null, null, false)).GetPortNames();
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	static override bool VisibleInPalette() { return true; }
};

class SCR_AIGetMoveFromDangerBehaviorParameters  : SCR_AIGetActionParameters
{
	static ref TStringArray s_aVarsOut = (new SCR_AIMoveFromDangerBehavior(null, null, vector.Zero, null)).GetPortNames();
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	static override bool VisibleInPalette() { return true; }
};

class SCR_AIGetMoveFromUnsafeAreaBehaviorParameters  : SCR_AIGetActionParameters
{
	static ref TStringArray s_aVarsOut = (new SCR_AIMoveFromUnsafeAreaBehavior(null, null, vector.Zero, null, 0)).GetPortNames();
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	static override bool VisibleInPalette() { return true; }
};

class SCR_AIGetMoveIndividuallyBehaviorParameters : SCR_AIGetActionParameters
{
	static ref TStringArray s_aVarsOut = (new SCR_AIMoveIndividuallyBehavior(null, null, vector.Zero)).GetPortNames();
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	static override bool VisibleInPalette() { return true; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		SCR_AIActionBase action = GetExecutedAction();
		
		if (!action)
			return ENodeResult.FAIL;
		
		SCR_AIMoveIndividuallyBehavior moveIndBehavior = SCR_AIMoveIndividuallyBehavior.Cast(action);
		
		if (moveIndBehavior)
		{
			SCR_BTParamAssignable<IEntity> paramEntity = moveIndBehavior.m_Entity;
			paramEntity.m_AssignedOut = paramEntity.m_Value != null;
		}
			
		action.SetParametersToBTVariables(this);
		return ENodeResult.SUCCESS;
	} 
};

class SCR_AIGetRetreatWhileLookAtBehaviorParameters : SCR_AIGetActionParameters
{
	static ref TStringArray s_aVarsOut = (new SCR_AIRetreatWhileLookAtBehavior(null, null)).GetPortNames();
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	static override bool VisibleInPalette() { return true; }
};

class SCR_AIGetThrowGrenadeToBehaviorParameters : SCR_AIGetActionParameters
{
	static ref TStringArray s_aVarsOut = (new SCR_AIThrowGrenadeToBehavior(null, null, vector.Zero, EWeaponType.WT_NONE, 0)).GetPortNames();
	
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	static override bool VisibleInPalette() { return true; }
};

class SCR_AIFireIllumFlareBehaviorParameters : SCR_AIGetActionParameters
{
	static ref TStringArray s_aVarsOut = (new SCR_AIFireIllumFlareBehavior(null, null, vector.Zero, null, null, 0)).GetPortNames();
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	static override bool VisibleInPalette() { return true; }
};

class SCR_AIGetSuppressBehaviorParameters : SCR_AIGetActionParameters
{
	static ref TStringArray s_aVarsOut = (new SCR_AISuppressBehavior(null, null, null, 0, 0)).GetPortNames();
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	static override bool VisibleInPalette() { return true; }
};

class SCR_AIGetSuppressActivityParameters : SCR_AIGetActionParameters
{
	static ref TStringArray s_aVarsOut = (new SCR_AISuppressActivity(null, null, null)).GetPortNames();
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	static override bool VisibleInPalette() { return true; }
}