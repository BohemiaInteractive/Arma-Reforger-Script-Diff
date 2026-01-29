class SCR_AIGetSmartActionSentinelParams : AITaskScripted
{
	static const string SMART_ACTION_GUARD_PORT		= "SmartActionGuard";
	static const string POSITION_PORT				= "OutPos";
	static const string LOOK_DIRECTION_PORT			= "LookPosition";
	static const string LOOK_DIRECTION_RANGE_PORT	= "LookDirectionRange";
	static const string STANCE_PORT					= "Stance";
	static const string USE_BINOCULARS_PORT			= "UseBinoculars";
	static const string LEANING_PORT				= "LeaningType";
		
	protected static override bool VisibleInPalette()
	{
		return true;
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		SMART_ACTION_GUARD_PORT
	};
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }
	
	//-----------------------------------------------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		POSITION_PORT,
		LOOK_DIRECTION_PORT,
		LOOK_DIRECTION_RANGE_PORT,
		STANCE_PORT,
		USE_BINOCULARS_PORT,
		LEANING_PORT
	};
	override TStringArray GetVariablesOut()
    {
        return s_aVarsOut;
    }

	//-----------------------------------------------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		SCR_AISmartActionSentinelComponent smartAction;
		if(!GetVariableIn(SMART_ACTION_GUARD_PORT, smartAction) || !smartAction)
		{
			NodeError(this, owner, "Invalid SCR_AISmartActionSentinelComponent provided!");
			return ENodeResult.FAIL;
		}
		
		IEntity actionOwner = smartAction.GetOwner();
		
		vector worldPosition = smartAction.GetActionOffset(), worldPositionToLook = smartAction.GetLookPosition(), smartObjectPos = actionOwner.GetOrigin();
		vector mat[4];
		actionOwner.GetWorldTransform(mat);
		worldPosition = smartObjectPos + worldPosition.Multiply3(mat);
		if (worldPositionToLook == vector.Zero)
			worldPositionToLook = smartObjectPos + (10 * vector.Forward).Multiply3(mat);
		else 
			worldPositionToLook = smartObjectPos + worldPositionToLook.Multiply3(mat);
		
		SetVariableOut(POSITION_PORT, worldPosition);
		SetVariableOut(LOOK_DIRECTION_PORT, worldPositionToLook);
		SetVariableOut(LOOK_DIRECTION_RANGE_PORT, smartAction.GetLookDirectionRange());
		SetVariableOut(STANCE_PORT, smartAction.GetDesiredStance());
		SetVariableOut(USE_BINOCULARS_PORT, smartAction.GetUseBinoculars());
		switch (smartAction.GetLeaningType())
		{
			case ELeaningType.LEFT : 
			{
				SetVariableOut(LEANING_PORT, "CharacterLeanToggleLeft");
				break;	
			};
			case ELeaningType.RIGHT : 
			{
				SetVariableOut(LEANING_PORT, "CharacterLeanToggleRight");
				break;
			};
			default:
			{
				ClearVariable(LEANING_PORT);
			};			
		};	
		return ENodeResult.SUCCESS;
	}	
};
