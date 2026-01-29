class SCR_AIDecoMachinegunnerBipod : DecoratorScripted
{
	protected static const string PORT_TARGET_VISIBLE = "TargetVisible";
	protected static const string PORT_TARGET_POSITION = "TargetPos";
	
	[Attribute("10", UIWidgets.EditBox)]
	protected float m_fDistanceThreshold;
	
	ref TStringArray s_aVarsIn = { PORT_TARGET_VISIBLE, PORT_TARGET_POSITION };
	
	protected IEntity m_ControlledEntity;
	
	override TStringArray GetVariablesIn() { return s_aVarsIn; };
	
	override void OnInit(AIAgent owner)
	{
		m_ControlledEntity = owner.GetControlledEntity();
	}
	
	override bool TestFunction(AIAgent owner)
	{	
		vector targetPos;
		bool targetVisible;
		
		if (!m_ControlledEntity ||
			!GetVariableIn(PORT_TARGET_POSITION, targetPos) ||
			!GetVariableIn(PORT_TARGET_VISIBLE, targetVisible))
			return false;
		
		float distance = vector.Distance(targetPos, m_ControlledEntity.GetOrigin());
		
		return distance > m_fDistanceThreshold && targetVisible;
	}
	
	static override string GetOnHoverDescription() { return "Tests conditions specific for machinegunner bopod deployment behavior. This node is not intended for generic usage."; }
}
