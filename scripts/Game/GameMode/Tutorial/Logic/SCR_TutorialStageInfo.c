[BaseContainerProps(), SCR_BaseContainerCustomTitleField("m_sStageClassName")]
class SCR_TutorialStageInfo
{
	[Attribute("", UIWidgets.EditBox)]
	protected string m_sStageClassName;
	
	[Attribute("", UIWidgets.EditBox)]
	protected string m_sStageName;

	[Attribute("")]
	protected ref array<ref SCR_HintUIInfo> m_aHints;
	
	[Attribute("")]
	protected string m_sHintSoundEvent;
	
	[Attribute("7")]
	protected float m_fFailsafeHintTimeout;

	[Attribute("0.25")]
	protected float m_fConditionCheckPeriod;
	
	[Attribute("0")]
	protected float m_fDuration;
	
	[Attribute("0")]
	protected float m_fDelay;
	
	[Attribute("1")]
	protected float m_fWaypointCompletionRadius;
	
	[Attribute("1")]
	protected bool m_bShowWaypoint;
	
	[Attribute("1")]
	protected bool m_bCheckWaypoint;
	
	[Attribute("0")]
	protected bool m_bConditionCheckOnSetDuration;
	
	[Attribute("1")]
	protected bool m_bAutoShowHint;
	
	[Attribute("")]
	protected string m_sAreaRestrictionEntityOverride;
	
	[Attribute("-1")]
	protected float m_fAreaRestrictionWarningOverride;
	
	[Attribute("-1")]
	protected float m_fAreaRestrictionBreakingOverride;

	//------------------------------------------------------------------------------------------------
	float GetAreaRestrictionWarningOverride()
	{
		return m_fAreaRestrictionWarningOverride;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetAreaRestrictionBreakingOverride()
	{
		return m_fAreaRestrictionBreakingOverride;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetAreaRestrictionEntityOverride()
	{
		return m_sAreaRestrictionEntityOverride;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	string GetClassName()
	{
		return m_sStageClassName;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	string GetStageName()
	{
		return m_sStageName;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_HintUIInfo GetHint(int index = 0)
	{
		if (!m_aHints.IsIndexValid(index))
			return null;
		
		return m_aHints[index];
	}
	
	//------------------------------------------------------------------------------------------------
	string GetHintSoundEvent()
	{
		return m_sHintSoundEvent;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetFailsafeHintTimeout()
	{
		return m_fFailsafeHintTimeout;
	}

	//------------------------------------------------------------------------------------------------
	float GetConditionCheckPeriod()
	{
		return m_fConditionCheckPeriod;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetDuration()
	{
		return m_fDuration;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetDelay()
	{
		return m_fDelay;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetWaypointCompletionRadius()
	{
		return m_fWaypointCompletionRadius;
	}
	
	//------------------------------------------------------------------------------------------------
	bool ShowWaypoint()
	{
		return m_bShowWaypoint;
	}
	
	//------------------------------------------------------------------------------------------------
	bool CheckWaypoint()
	{
		return m_bCheckWaypoint;
	}
	
	//------------------------------------------------------------------------------------------------
	bool ConditionCheckOnSetDuration()
	{
		return m_bConditionCheckOnSetDuration;
	}
	
	//------------------------------------------------------------------------------------------------
	bool AutoShowHint()
	{
		return m_bAutoShowHint;
	}
}