//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIOrder_Gesture
class SCR_AISendOrder_Gesture : SCR_AISendMessageGenerated
{
	[Attribute("", UIWidgets.ComboBox, enumType: SCR_ECharacterGestureType)]
	SCR_ECharacterGestureType m_eGestureType;
	
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendOrderGenerated.PORT_RECEIVER,
		"GestureType",
		"RelatedInvoker"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIOrder_Gesture msg = new SCR_AIOrder_Gesture();
		
		msg.SetText(m_sText);
		
		GetVariableIn("RelatedInvoker", msg.m_RelatedInvoker);
		
		if(!GetVariableIn("GestureType", msg.m_eGestureType))
			msg.m_eGestureType = m_eGestureType;
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;		
	}
	
	override string GetNodeMiddleText()
	{
		string s;
		s = s + string.Format("m_eGestureType: %1\n", typename.EnumToString(SCR_ECharacterGestureType, m_eGestureType));
		return s;
	}
	static override bool VisibleInPalette() { return true; }
}


