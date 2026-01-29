// if you want this node to abort subtree when conditions change, you must specify abort condition!
// normal mode: smaller than low distance -> Condition TRUE
// negate mode: bigger than high distance -> Condition TRUE
class SCR_AIDecoDistanceHysteresis : DecoratorScripted
{
	static const string PORT_ENTITY_IN = "EntityIn";
	static const string PORT_POSITION_IN = "PositionIn";
	
	[Attribute("", UIWidgets.EditBox, "Low boundary distance")]
	private float m_lowBoundary;
	
	[Attribute("", UIWidgets.EditBox, "High boundary distance")]
	private float m_highBoundary;
	
	private bool m_lastValueOfGate = false;
	
/*
	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{	
		// make sure it works when uncomenting
		
		if (GetVariableType(true, PORT_POSITION_IN) == vector || GetVariableType(true, PORT_ENTITY_IN) == IEntity)
		{
			NodeError(this, owner, PORT_POSITION_IN + "(vector) or " + PORT_ENTITY_IN + "(IEntity) has to be provided");
		}
		
		//TODO uncoment OnInit
	}
*/
	
	//------------------------------------------------------------------------------------------------
	protected override bool TestFunction(AIAgent owner)
	{
		vector ownerPos,destinationPos;
		IEntity entity;
		bool openGate = false;
		
		AIGroup ownerIsGroup = AIGroup.Cast(owner);
		if (ownerIsGroup)
			ownerPos = ownerIsGroup.GetLeaderEntity().GetOrigin();
		else
			ownerPos = owner.GetControlledEntity().GetOrigin();
 	
		
		if (!GetVariableIn(PORT_ENTITY_IN, entity))
		{
			if (!GetVariableIn(PORT_POSITION_IN,destinationPos))
				return false;
		}
		else if (!entity)
			return false;
		else		 
			destinationPos = entity.GetOrigin();	
				
		float dist = vector.Distance(destinationPos, ownerPos);
		if ( dist < m_lowBoundary)
		{
			openGate = true;
			m_lastValueOfGate = openGate;								
		}	
		else if ( dist > m_highBoundary)
		{
			openGate = false;
			m_lastValueOfGate = openGate;								
		}					
		else
		{
			openGate = m_lastValueOfGate;					
		}								
				
		return openGate;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static override bool VisibleInPalette()
	{
		return true;
	}	
	
	//------------------------------------------------------------------------------------------------
	protected static override string GetOnHoverDescription()
	{
		return "SCR_AIDecoDistanceHysteresis: Checks distance to entity within boundaries and in circumference it returns value according to history of moving";
	}
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		PORT_ENTITY_IN,
		PORT_POSITION_IN
	};
	protected override TStringArray GetVariablesIn()
	{
		return s_aVarsIn;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override string GetNodeMiddleText()
	{
		return "Low boundary: " + m_lowBoundary.ToString() + "\n" + "High boundary: " + m_highBoundary.ToString();
	}
};
