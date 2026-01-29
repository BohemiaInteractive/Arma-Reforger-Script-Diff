[EntityEditorProps(insertable: false)]
class SCR_Tutorial_ShootingRange_SHOOTClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_ShootingRange_SHOOT : SCR_BaseTutorialStage
{
	protected ref array <SCR_FiringRangeTarget> m_aTargets;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();

		m_aTargets = {};
		
		m_TutorialComponent.SetupTargets("TARGETS_FRONT", m_aTargets, ETargetState.TARGET_UP, false);
		foreach (SCR_FiringRangeTarget target : m_aTargets)
		{
			if (target)
				target.Event_TargetChangeState.Insert(OnTargetStateChanged);
		}

		PlayNarrativeCharacterStage("SHOOTINGRANGE_Instructor", 6);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnTargetStateChanged(ETargetState state, SCR_FiringRangeTarget target)
	{	
		target.Event_TargetChangeState.Remove(OnTargetStateChanged);
		m_aTargets.RemoveItem(target);
		
		if (!m_aTargets || m_aTargets.IsEmpty())
			m_bFinished = true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		if (m_TutorialComponent.GetRemainingAmmo() == 0)
			RegisterWaypoint("Ammobox", "", "AMMO");
		else
			UnregisterWaypoint("Ammobox");
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_Tutorial_ShootingRange_SHOOT()
	{
		if (!m_aTargets || m_aTargets.IsEmpty())
			return;
		
		foreach (SCR_FiringRangeTarget target : m_aTargets)
		{
			target.Event_TargetChangeState.Remove(OnTargetStateChanged);
		}
	}
};