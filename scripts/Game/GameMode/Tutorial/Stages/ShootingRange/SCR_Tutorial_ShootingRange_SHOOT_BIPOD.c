[EntityEditorProps(insertable: false)]
class SCR_Tutorial_ShootingRange_SHOOT_BIPODClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_ShootingRange_SHOOT_BIPOD : SCR_BaseTutorialStage
{
	protected ref array <SCR_FiringRangeTarget> m_aTargets;
	protected bool m_bDeployedCommented;
	
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
	
		RegisterWaypoint("WP_FIREPOZ_4", "", "GUNREADY");
		
		m_aTargets = {};
		
		m_TutorialComponent.SetupTargets("TARGETS_BACK", m_aTargets, ETargetState.TARGET_UP, true);
		foreach (SCR_FiringRangeTarget target : m_aTargets)
		{
			if (target)
				target.Event_TargetChangeState.Insert(OnTargetStateChanged);
		}

		PlayNarrativeCharacterStage("SHOOTINGRANGE_Instructor", 11);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnTargetStateChanged(ETargetState state, SCR_FiringRangeTarget target)
	{	
		if (state != ETargetState.TARGET_DOWN)
			return;
		
		if (!m_Player || !m_Player.GetCharacterController().GetIsWeaponDeployedBipod())
			return;
		
		target.SetAutoResetTarget(false);
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
		
		if (!m_bDeployedCommented && m_Player && m_Player.GetCharacterController().GetIsWeaponDeployedBipod())
		{
			m_bDeployedCommented = true;
			PlayNarrativeCharacterStage("SHOOTINGRANGE_Instructor", 12);
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_Tutorial_ShootingRange_SHOOT_BIPOD()
	{
		if (!m_aTargets || m_aTargets.IsEmpty())
			return;
		
		foreach (SCR_FiringRangeTarget target : m_aTargets)
		{
			target.Event_TargetChangeState.Remove(OnTargetStateChanged);
		}
	}
};