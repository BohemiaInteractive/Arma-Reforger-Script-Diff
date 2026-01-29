class SCR_AICheckEntityStance : AITaskScripted
{
	static const string PORT_ENTITY	= "Entity";
	static const string PORT_STANCE	= "Stance";
	
	[Attribute("0", UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ECharacterStance))]
	protected ECharacterStance m_eStanceDefault;

	[Attribute("10")]
	protected float m_fMaxDistance;
	
	protected ECharacterStance m_eStance;
	protected ECharacterStance m_eStanceLimit = ECharacterStance.STAND;	
	protected bool m_bIsAborted;
	protected AIGroup m_Group;
	protected SCR_AIGroupInfoComponent m_GroupInfo;
	
	protected float m_fNextUpdate_ms;
	[Attribute("500")]
	protected float m_fUpdateInterval_ms;
	
	override void OnInit(AIAgent owner)
	{
		m_bIsAborted = false;
		m_Group = AIGroup.Cast(owner);
		if (!m_Group)
			return SCR_AgentMustBeAIGroup(this, owner);
		m_GroupInfo = SCR_AIGroupInfoComponent.Cast(m_Group.FindComponent(SCR_AIGroupInfoComponent));
		if (!m_GroupInfo)
			return;
		m_eStanceLimit = m_GroupInfo.GetStanceLimit();
		m_eStance = m_eStanceDefault;
	}
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		float currentTime_ms = GetGame().GetWorld().GetWorldTime();
		if (currentTime_ms < m_fNextUpdate_ms)
			return ENodeResult.RUNNING;
		m_fNextUpdate_ms = currentTime_ms + m_fUpdateInterval_ms;
		
		if (!m_Group || !m_GroupInfo)
			return ENodeResult.FAIL;
		
		IEntity entityToCopy;
		if (!GetVariableIn(PORT_ENTITY, entityToCopy))
			return NodeError(this, owner, "No entity provided!");
		
		if (!entityToCopy)
			return ENodeResult.FAIL;
		
		// keep running if agents are too far from entityToCopy
		vector entityPos 	= entityToCopy.GetOrigin();
		vector agentPos 	= owner.GetOrigin();
		float distSq = vector.DistanceSq(entityPos, agentPos); 
		
		if (distSq >= (m_fMaxDistance * m_fMaxDistance))
			return ENodeResult.RUNNING;

		// get stance of entityToCopy
		CharacterControllerComponent charCont = CharacterControllerComponent.Cast(entityToCopy.FindComponent(CharacterControllerComponent));
		if (!charCont)
			return NodeError(this, owner, "No character controller?!");

		ECharacterStance stance = charCont.GetStance();
		if (stance == m_eStance)
			return ENodeResult.RUNNING;
		
		m_GroupInfo.SetStanceLimit(stance);
		m_eStance = stance;
		SetVariableOut(PORT_STANCE, stance);
		return ENodeResult.SUCCESS;
	}

	//------------------------------------------------------------------------------------------------
	override void OnAbort(AIAgent owner, Node nodeCausingAbort)
	{		
		if (m_bIsAborted)
			return;
		if (m_GroupInfo)
			m_GroupInfo.SetStanceLimit(m_eStanceLimit);
		m_bIsAborted = true;
	}
	
	//------------------------------------------------------------------------------------------------
	static override bool VisibleInPalette() {return true;}
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		PORT_STANCE
	};
	override TStringArray GetVariablesOut()
	{
		return s_aVarsOut;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		PORT_ENTITY
	};
	override TStringArray GetVariablesIn()
	{
		return s_aVarsIn;
	}
	
	//------------------------------------------------------------------------------------------------
	static override string GetOnHoverDescription()
	{
		return "SCR_AICheckEntityStance: checks current entity stance. Returns running if stance doesn't change or leading entity is far away.";
	}
	
	static override bool CanReturnRunning()
	{
		return true;
	}
}