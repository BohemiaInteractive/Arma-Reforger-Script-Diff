class SCR_AIFindSmartActions: AITaskScripted
{
	static const string PORT_CENTER_OF_SEARCH		= "OriginIn";
	static const string PORT_RADIUS					= "RadiusIn";
	static const string PORT_SA_NUMBER				= "SmartActionNumber";
	static const string PORT_SA_FOUND				= "SmartActionsFound";
	static const string PORT_SA_TAGS				= "SmartActionTags";
	
	protected AISmartActionSystem m_AISmartActionSystem;
	protected SCR_AIGroup m_groupOwner;
	
	//------------------------------------------------------------------------------------------------
	static override bool VisibleInPalette() {return true;}
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		ChimeraWorld myWorld = ChimeraWorld.CastFrom(GetGame().GetWorld());
		if (myWorld)
			m_AISmartActionSystem = AISmartActionSystem.Cast(myWorld.FindSystem(AISmartActionSystem));
		m_groupOwner = SCR_AIGroup.Cast(owner);
		if (!m_groupOwner)
		{
			m_groupOwner = SCR_AIGroup.Cast(owner.GetParentGroup());
			if (!m_groupOwner)
				NodeError(this, owner, "Node is not run on SCR_AIGroup agent or owner is not member of SCR_AIGroup!");
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_AISmartActionSystem)
			return ENodeResult.FAIL;
		
		vector center;
		GetVariableIn(PORT_CENTER_OF_SEARCH, center);
		float radius;
		GetVariableIn(PORT_RADIUS, radius);
		ref array<string> aTags = {};
		GetVariableIn(PORT_SA_TAGS, aTags);
	
		ref array<AISmartActionComponent> outSmartActions = {};
		int numSAFound;
		numSAFound = m_AISmartActionSystem.FindSmartActions(outSmartActions, center, radius, aTags, EAIFindSmartAction_TagTest.InOrder);
		if (numSAFound > 0)
		{
			SetVariableOut(PORT_SA_NUMBER,numSAFound);
			m_groupOwner.AllocateSmartActions(outSmartActions);
		}	
		SetVariableOut(PORT_SA_FOUND, numSAFound > 0);
		return ENodeResult.SUCCESS;		
	}
		
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		PORT_SA_NUMBER,
		PORT_SA_FOUND
	};
	override TStringArray GetVariablesOut()
	{
		return s_aVarsOut;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		PORT_CENTER_OF_SEARCH,
		PORT_RADIUS,
		PORT_SA_TAGS
	};
	override TStringArray GetVariablesIn()
	{
		return s_aVarsIn;
	}
	
	//------------------------------------------------------------------------------------------------
	static override string GetOnHoverDescription()
	{
		return "FindSmartActions: finds all smart actions of given tags within center and radius. Caches result to group properties.";
	}
};