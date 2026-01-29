[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Seizing_ShootTargetsClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Seizing_ShootTargets : SCR_BaseTutorialStage
{
	protected ref array <SCR_FiringRangeTarget> m_aTargets;
	protected bool m_bComment;
	
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		if (m_TutorialComponent && m_TutorialComponent.GetRemainingAmmo() < 5)
		{
			m_bFinished = true;
			return;
		}
		
		
		m_aTargets = {};
		m_TutorialComponent.SetupTargets("base_target1", m_aTargets, ETargetState.TARGET_UP, false);
		
		foreach (SCR_FiringRangeTarget target : m_aTargets)
		{
			RegisterWaypoint(target, "", "TARGETCHAR").SetOffsetVector("0 1.7 0");;
			target.Event_TargetChangeState.Insert(OnTargetStateChanged);
		}
		
		SCR_VoiceoverSystem.GetInstance().RegisterActor(GetGame().GetWorld().FindEntityByName("CONFLICT_Instructor_C"));
		
		PlayNarrativeCharacterStage("CONFLICT_Instructor_C", 1);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnTargetStateChanged(ETargetState state, SCR_FiringRangeTarget target)
	{
		if (state != ETargetState.TARGET_DOWN)
			return;
		
		SCR_WaypointDisplay wpDisplay = m_TutorialComponent.GetWaypointDisplay();
		if (!wpDisplay)
			return;
		
		SCR_Waypoint wp = wpDisplay.FindWaypointByEntity(target);
		if (!wp)
			return;

		wpDisplay.DeleteWaypoint(wp);
		m_aStageWaypoints.RemoveItem(wp);
				
		m_aTargets.RemoveItem(target);
		target.Event_TargetChangeState.Remove(OnTargetStateChanged);
		
		m_bComment = !m_bComment;

		if (m_bComment && !m_aTargets.IsEmpty())
			GetGame().GetCallqueue().CallLater(PlayNarrativeCharacterStage, 500, false, "CONFLICT_Instructor_C", Math.RandomInt(2,5));
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnStageFinished()
	{
		if (m_TutorialComponent)
			m_TutorialComponent.SetupTargets("base_target1", null, ETargetState.TARGET_DOWN, false);
		
		super.OnStageFinished();
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		if (m_TutorialComponent && m_TutorialComponent.GetRemainingAmmo() < m_aTargets.Count())
			return true;
		
		return m_aTargets.IsEmpty();
	}
};