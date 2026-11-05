modded enum EOrderType_Character
{
	GESTURE,
	BARK,
}

[EnumLinear()]
enum SCR_ECharacterGestureType
{
	NONE,
	CANCEL,
	SIGN,
	BOO,
	CHEER,
	PUSH,
	PULL,
}

class SCR_AIOrder_Gesture : SCR_AIOrderBase // MESSAGE_CLASS(GenerateSendOrder, SCR_AIOrder_Gesture)
{
	SCR_ECharacterGestureType m_eGestureType; 								 	// VARIABLE(NodePort, GestureType, NodePropertyEnum, m_eGestureType)
	ref ScriptInvokerBase<SCR_AIOnAnimationBehaviorAction> m_RelatedInvoker; 	// VARIABLE(NodePort, RelatedInvoker)

	//------------------------------------------------------------------------------------------------
	void SCR_AIOrder_Gesture()
	{
		m_eOrderType = EOrderType_Character.GESTURE;
	}

	//------------------------------------------------------------------------------------------------
	static SCR_AIOrder_Gesture Create(SCR_ECharacterGestureType gestureType, int mood, ScriptInvokerBase<SCR_AIOnAnimationBehaviorAction> relatedInvoker)
	{
		SCR_AIOrder_Gesture m = new SCR_AIOrder_Gesture();
		int packedInt = SCR_AIIntegerOperations.EncodeTwo(gestureType, mood);
		m.m_eGestureType = packedInt;
		m.m_RelatedInvoker = relatedInvoker;
		return m;
	}

	//------------------------------------------------------------------------------------------------
	override void GetOrderParameters(SCR_AIProcessOrder node)
	{
		node.SetVariableOut(node.ORDER_TYPE, EOrderType_Character.GESTURE);
		node.SetVariableOut(node.SCRIPTED_ORDER, true);
		node.SetVariableOut(node.ORDER_VALUE, m_eGestureType);
		if (node.m_bDebugMe)
			node.m_sDebugString = "Type of animation "+ typename.EnumToString(SCR_ECharacterGestureType, m_eGestureType) + " by " + node.m_Order.GetText();
	}

	//------------------------------------------------------------------------------------------------
	override void SetOrderParameters(SCR_AISendOrder node)
	{
		super.SetOrderParameters(node);
		if (!node.GetVariableIn(node.ORDER_VALUE, m_eGestureType))
			m_eGestureType = node.m_integer;
	}
}

class SCR_AIOrder_Bark : SCR_AIOrderBase // MESSAGE_CLASS(GenerateSendOrder, SCR_AIOrder_Bark)
{
	int m_iBarkMood; // VARIABLE(NodePort, BarkMood, NodePropertyEnum, m_iBarkMood)

	//------------------------------------------------------------------------------------------------
	void SCR_AIOrder_Bark()
	{
		m_eOrderType = EOrderType_Character.BARK;
	}

	//------------------------------------------------------------------------------------------------
	static SCR_AIOrder_Bark Create(int barkMood)
	{
		SCR_AIOrder_Bark m = new SCR_AIOrder_Bark();
		m.m_iBarkMood = barkMood;
		return m;
	}

	//------------------------------------------------------------------------------------------------
	override void GetOrderParameters(SCR_AIProcessOrder node)
	{
		node.SetVariableOut(node.ORDER_TYPE, EOrderType_Character.BARK);
		node.SetVariableOut(node.SCRIPTED_ORDER, true);
		node.SetVariableOut(node.ORDER_VALUE, m_iBarkMood);
		if (node.m_bDebugMe)
			node.m_sDebugString = "Bark with mood "+ m_iBarkMood.ToString() + " by " + node.m_Order.GetText();
	}

	//------------------------------------------------------------------------------------------------
	override void SetOrderParameters(SCR_AISendOrder node)
	{
		super.SetOrderParameters(node);
		if (!node.GetVariableIn(node.ORDER_VALUE, m_iBarkMood))
			m_iBarkMood = node.m_integer;
	}
}
