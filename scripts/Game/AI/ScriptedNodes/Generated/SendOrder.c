//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIOrder_Stance
class SCR_AISendOrder_Stance : SCR_AISendMessageGenerated
{
	[Attribute("", UIWidgets.ComboBox, enumType: ECharacterStance)]
	ECharacterStance m_eStance;
	
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendOrderGenerated.PORT_RECEIVER,
		"Stance"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIOrder_Stance msg = new SCR_AIOrder_Stance();
		
		msg.SetText(m_sText);
		
		if(!GetVariableIn("Stance", msg.m_eStance))
			msg.m_eStance = m_eStance;
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	override string GetNodeMiddleText()
	{
		string s;
		s = s + string.Format("m_eStance: %1\n", typename.EnumToString(ECharacterStance, m_eStance));
		return s;
	}
	static override bool VisibleInPalette() { return true; }
}

//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIOrder_WeaponRaised
class SCR_AISendOrder_WeaponRaised : SCR_AISendMessageGenerated
{
	[Attribute("")]
	bool m_bWeaponRaised;
	
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendOrderGenerated.PORT_RECEIVER,
		"WeaponRaised"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIOrder_WeaponRaised msg = new SCR_AIOrder_WeaponRaised();
		
		msg.SetText(m_sText);
		
		if(!GetVariableIn("WeaponRaised", msg.m_bWeaponRaised))
			msg.m_bWeaponRaised = m_bWeaponRaised;
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	override string GetNodeMiddleText()
	{
		string s;
		s = s + string.Format("m_bWeaponRaised: %1\n", m_bWeaponRaised);
		return s;
	}
	static override bool VisibleInPalette() { return true; }
}

//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIOrder_MovementType
class SCR_AISendOrder_MovementType : SCR_AISendMessageGenerated
{
	[Attribute("", UIWidgets.ComboBox, enumType: EMovementType)]
	EMovementType m_eMovementType;
	
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendOrderGenerated.PORT_RECEIVER,
		"MovementType"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIOrder_MovementType msg = new SCR_AIOrder_MovementType();
		
		msg.SetText(m_sText);
		
		if(!GetVariableIn("MovementType", msg.m_eMovementType))
			msg.m_eMovementType = m_eMovementType;
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	override string GetNodeMiddleText()
	{
		string s;
		s = s + string.Format("m_eMovementType: %1\n", typename.EnumToString(EMovementType, m_eMovementType));
		return s;
	}
	static override bool VisibleInPalette() { return true; }
}

//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIOrder_ReturnToDefault
class SCR_AISendOrder_ReturnToDefault : SCR_AISendMessageGenerated
{
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendOrderGenerated.PORT_RECEIVER
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIOrder_ReturnToDefault msg = new SCR_AIOrder_ReturnToDefault();
		
		msg.SetText(m_sText);
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	static override bool VisibleInPalette() { return true; }
}

//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIOrder_WeaponType
class SCR_AISendOrder_WeaponType : SCR_AISendMessageGenerated
{
	[Attribute("", UIWidgets.ComboBox, enumType: EWeaponType)]
	EWeaponType m_eWeaponType;
	
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendOrderGenerated.PORT_RECEIVER,
		"WeaponType"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIOrder_WeaponType msg = new SCR_AIOrder_WeaponType();
		
		msg.SetText(m_sText);
		
		if(!GetVariableIn("WeaponType", msg.m_eWeaponType))
			msg.m_eWeaponType = m_eWeaponType;
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	override string GetNodeMiddleText()
	{
		string s;
		s = s + string.Format("m_eWeaponType: %1\n", typename.EnumToString(EWeaponType, m_eWeaponType));
		return s;
	}
	static override bool VisibleInPalette() { return true; }
}

//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIOrder_AIState
class SCR_AISendOrder_AIState : SCR_AISendMessageGenerated
{
	[Attribute("", UIWidgets.ComboBox, enumType: EUnitAIState)]
	EUnitAIState m_eAIState;
	
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendOrderGenerated.PORT_RECEIVER,
		"AIState"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIOrder_AIState msg = new SCR_AIOrder_AIState();
		
		msg.SetText(m_sText);
		
		if(!GetVariableIn("AIState", msg.m_eAIState))
			msg.m_eAIState = m_eAIState;
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	override string GetNodeMiddleText()
	{
		string s;
		s = s + string.Format("m_eAIState: %1\n", typename.EnumToString(EUnitAIState, m_eAIState));
		return s;
	}
	static override bool VisibleInPalette() { return true; }
}

//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIOrder_Unit_State
class SCR_AISendOrder_UnitState : SCR_AISendMessageGenerated
{
	[Attribute("", UIWidgets.ComboBox, enumType: EUnitState)]
	EUnitState m_eUnitState;
	
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendOrderGenerated.PORT_RECEIVER,
		"UnitState"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIOrder_Unit_State msg = new SCR_AIOrder_Unit_State();
		
		msg.SetText(m_sText);
		
		if(!GetVariableIn("UnitState", msg.m_eUnitState))
			msg.m_eUnitState = m_eUnitState;
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	override string GetNodeMiddleText()
	{
		string s;
		s = s + string.Format("m_eUnitState: %1\n", typename.EnumToString(EUnitState, m_eUnitState));
		return s;
	}
	static override bool VisibleInPalette() { return true; }
}

