[EntityEditorProps(insertable: false)]
class SCR_Tutorial_LRS_Targets_LastClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_LRS_Targets_Last : SCR_BaseTutorialStage
{
	protected ref array <SCR_FiringRangeTarget> m_aTargets;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		m_aTargets = {};
		
		m_TutorialComponent.SetupTargets("TARGETS_LAST", m_aTargets, ETargetState.TARGET_UP, false);
		foreach (SCR_FiringRangeTarget target : m_aTargets)
		{
			if (target)
				target.Event_TargetChangeState.Insert(OnTargetStateChanged);
		}
		
		PlayNarrativeCharacterStage("LONGRANGESHOOTING_Instructor", 8);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnTargetStateChanged(ETargetState state, SCR_FiringRangeTarget target)
	{	
		target.Event_TargetChangeState.Remove(OnTargetStateChanged);
		m_aTargets.RemoveItem(target);
		
		if (!m_aTargets || m_aTargets.IsEmpty())
		{
			SCR_HintManagerComponent.HideHint();
			SCR_HintManagerComponent.ClearLatestHint();
			
			PlayNarrativeCharacterStage("LONGRANGESHOOTING_Instructor", 9);
			m_bFinished = true;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		if (m_TutorialComponent && m_TutorialComponent.GetRemainingAmmo() == 0)
			RegisterWaypoint("Ammobox3", "", "AMMO");
		else
			UnregisterWaypoint("Ammobox3");
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_Tutorial_LRS_Targets_Last()
	{
		if (!m_aTargets || m_aTargets.IsEmpty())
			return;
		
		foreach (SCR_FiringRangeTarget target : m_aTargets)
		{
			target.Event_TargetChangeState.Remove(OnTargetStateChanged);
		}
	}
};