//! Returns a position of entity with local space offset
class SCR_AIGetEntityPosition : AITaskScripted
{
	[Attribute("0 0 0", UIWidgets.Auto)]
	protected vector m_fOffsetLocal;
	
	[Attribute("0 0 0", UIWidgets.Auto)]
	protected vector m_fOffsetWorld;
	
	protected static const string ENTITY_PORT = "Entity";
	protected static const string POSITION_PORT = "Position";
	
	static override bool VisibleInPalette() { return true; }
	
	static override string GetOnHoverDescription()
	{
		return "Returns position of entity with local and world space offset.";
	}
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		IEntity myEntity = owner.GetControlledEntity();
		
		IEntity entity;
		GetVariableIn(ENTITY_PORT, entity);
		if (!entity && !myEntity)
			return ENodeResult.FAIL;
		
		if (!entity)
			entity = myEntity;
		
		vector posOut = entity.CoordToParent(m_fOffsetLocal);
		posOut = posOut + m_fOffsetWorld;
		SetVariableOut(POSITION_PORT, posOut);
		
		return ENodeResult.SUCCESS;
	}
	
	protected static ref TStringArray s_aVarsIn = { ENTITY_PORT };
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	
	protected static ref TStringArray s_aVarsOut = { POSITION_PORT };
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
}