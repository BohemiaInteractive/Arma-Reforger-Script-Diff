class SCR_AIGetCoverParameters : AITaskScripted
{
	// Inputs
	protected static const string PORT_COVER_LOCK = "CoverLock";
	
	// Outputs
	protected static const string PORT_POS = "Pos";
	protected static const string PORT_STANCE = "Stance";
	protected static const string PORT_STANCE_HIDE = "StanceHide";
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		SCR_AICoverLock coverLock;
		GetVariableIn(PORT_COVER_LOCK, coverLock);
		if (!coverLock || !coverLock.IsValid())
			return ENodeResult.FAIL;
		
		float coverHeight = coverLock.m_vCoverTallestPos[1] - coverLock.m_vCoverPos[1];
		ECharacterStance stance, stanceHide;
		
		CalculateCoverStance(coverHeight, stance, stanceHide);
		
		SetVariableOut(PORT_POS, coverLock.m_vCoverPos);
		SetVariableOut(PORT_STANCE, stance);
		SetVariableOut(PORT_STANCE_HIDE, stanceHide);
		
		return ENodeResult.SUCCESS;
	}
	
	//------------------------------------------------------------------------------------------------
	void CalculateCoverStance(float height, out ECharacterStance outStance, out ECharacterStance outStanceHide)
	{
		if (height > 0.85)
		{
			outStance = ECharacterStance.STAND;
			outStanceHide = ECharacterStance.CROUCH;
		}
		else
		{
			outStance = ECharacterStance.CROUCH;
			outStanceHide = ECharacterStance.PRONE;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		PORT_POS,
		PORT_STANCE,
		PORT_STANCE_HIDE
	};
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	protected static ref TStringArray s_aVarsIn = {
		PORT_COVER_LOCK
	};
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	
	static override bool VisibleInPalette() { return true; }
}