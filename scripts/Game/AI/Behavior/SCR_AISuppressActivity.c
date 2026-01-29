class SCR_AISuppressActivity : SCR_AIActivityBase
{
	protected ref SCR_BTParamRef<SCR_AISuppressionVolumeBase> m_Volume = new SCR_BTParamRef<SCR_AISuppressionVolumeBase>("SuppressionVolume");
	
	void SCR_AISuppressActivity(SCR_AIGroupUtilityComponent utility, AIWaypoint relatedWaypoint, SCR_AISuppressionVolumeBase suppressionVolume, float priorityLevel = PRIORITY_LEVEL_NORMAL)
	{
		SetPriority(PRIORITY_ACTIVITY_SUPPRESS);
		
		m_sBehaviorTree = "{0191A917B6BD723F}AI/BehaviorTrees/Chimera/Group/ActivitySuppress.bt";
		
		m_fPriorityLevel.Init(this, priorityLevel);
		m_Volume.Init(this, suppressionVolume);
	}
	
	//-------------------------------------------------------------------------------------------------
	override void OnActionFailed()
	{
		super.OnActionFailed();
		SendCancelMessagesToAllAgents();
	}
	
	//-------------------------------------------------------------------------------------------------
	override void OnActionCompleted()
	{
		super.OnActionCompleted();
		SendCancelMessagesToAllAgents();
	}
	
	//-------------------------------------------------------------------------------------------------
	override void OnActionDeselected()
	{
		super.OnActionDeselected();
		SendCancelMessagesToAllAgents();
	}
}