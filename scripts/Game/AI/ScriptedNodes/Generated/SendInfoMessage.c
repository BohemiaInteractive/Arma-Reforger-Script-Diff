//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIMessage_Target
class SCR_AISendInfoMessage_Target : SCR_AISendMessageGenerated
{
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendMessageGenerated.PORT_RECEIVER,
		"TargetInfo"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIMessage_Target msg = new SCR_AIMessage_Target();
		
		msg.SetText(m_sText);
		
		GetVariableIn("TargetInfo", msg.m_TargetInfo);
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	static override bool VisibleInPalette() { return true; }
}

//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIMessage_NoAmmo
class SCR_AISendInfoMessage_NoAmmo : SCR_AISendMessageGenerated
{
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendMessageGenerated.PORT_RECEIVER,
		"EntityToSupply",
		"MagazineWellType"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIMessage_NoAmmo msg = new SCR_AIMessage_NoAmmo();
		
		msg.SetText(m_sText);
		
		GetVariableIn("EntityToSupply", msg.m_entityToSupply);
		
		GetVariableIn("MagazineWellType", msg.m_MagazineWell);
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	static override bool VisibleInPalette() { return true; }
}

//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIMessage_UnderFire
class SCR_AISendInfoMessage_UnderFire : SCR_AISendMessageGenerated
{
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendMessageGenerated.PORT_RECEIVER,
		"Position"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIMessage_UnderFire msg = new SCR_AIMessage_UnderFire();
		
		msg.SetText(m_sText);
		
		GetVariableIn("Position", msg.m_vPosition);
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	static override bool VisibleInPalette() { return true; }
}

//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIMessage_FoundCorpse
class SCR_AISendInfoMessage_FoundCorpse : SCR_AISendMessageGenerated
{
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendMessageGenerated.PORT_RECEIVER,
		"Position"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIMessage_FoundCorpse msg = new SCR_AIMessage_FoundCorpse();
		
		msg.SetText(m_sText);
		
		GetVariableIn("Position", msg.m_vPosition);
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	static override bool VisibleInPalette() { return true; }
}

//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIMessage_Wounded
class SCR_AISendInfoMessage_Wounded : SCR_AISendMessageGenerated
{
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendMessageGenerated.PORT_RECEIVER,
		"Position",
		"WoundedEntity"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIMessage_Wounded msg = new SCR_AIMessage_Wounded();
		
		msg.SetText(m_sText);
		
		GetVariableIn("Position", msg.m_vPosition);
		
		GetVariableIn("WoundedEntity", msg.m_WoundedEntity);
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	static override bool VisibleInPalette() { return true; }
}

//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIMessage_SitRep
class SCR_AISendInfoMessage_SitRep : SCR_AISendMessageGenerated
{
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendMessageGenerated.PORT_RECEIVER
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIMessage_SitRep msg = new SCR_AIMessage_SitRep();
		
		msg.SetText(m_sText);
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	static override bool VisibleInPalette() { return true; }
}

//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIMessage_HealFailed
class SCR_AISendInfoMessage_HealFailed : SCR_AISendMessageGenerated
{
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendMessageGenerated.PORT_RECEIVER,
		"TargetEntity"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIMessage_HealFailed msg = new SCR_AIMessage_HealFailed();
		
		msg.SetText(m_sText);
		
		GetVariableIn("TargetEntity", msg.m_TargetEntity);
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	static override bool VisibleInPalette() { return true; }
}

