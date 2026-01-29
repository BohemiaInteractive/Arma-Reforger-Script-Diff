class SCR_AIGetPositionAndRotation: AITaskScripted
{
    static override bool VisibleInPalette() {return true;}
	
	protected static ref TStringArray s_aVarsIn = {
		"Entity"
	};
    override array<string> GetVariablesIn()
    {
        return s_aVarsIn;
    }
    
	protected static ref TStringArray s_aVarsOut = {
		"Position",
		"Rotation"
	};
    override array<string> GetVariablesOut()
    {
        return s_aVarsOut;
    }

    override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
    {
		IEntity entity;
		GetVariableIn("Entity",entity);
		if (!entity)
			entity = owner.GetControlledEntity();
		
		if (!entity)
			return ENodeResult.FAIL;
		
		SetVariableOut("Position",entity.GetOrigin());
		SetVariableOut("Rotation",entity.GetAngles());

		return ENodeResult.SUCCESS;
    }
};