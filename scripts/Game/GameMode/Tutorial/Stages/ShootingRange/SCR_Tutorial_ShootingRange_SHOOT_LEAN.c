[EntityEditorProps(insertable: false)]
class SCR_Tutorial_ShootingRange_SHOOT_LEANClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_ShootingRange_SHOOT_LEAN : SCR_BaseTutorialStage
{
	protected ref array <SCR_FiringRangeTarget> m_aTargets;
	
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();

		PlayNarrativeCharacterStage("SHOOTINGRANGE_Instructor", 9);
		RegisterWaypoint("WP_FIREPOZ_2", "", "GUNREADY");
		
		m_aTargets = {};
		
		m_TutorialComponent.SetupTargets("TARGETS_ECHELON_LEFT", m_aTargets, ETargetState.TARGET_UP, true);
		foreach (SCR_FiringRangeTarget target : m_aTargets)
		{
			if (target)
				target.Event_TargetChangeState.Insert(OnTargetStateChanged);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnTargetStateChanged(ETargetState state, SCR_FiringRangeTarget target)
	{	
		if (state != ETargetState.TARGET_DOWN)
			return;
		
		if (!m_Player || !m_Player.GetCharacterController().IsLeaning())
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
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_Tutorial_ShootingRange_SHOOT_LEAN()
	{
		if (!m_aTargets || m_aTargets.IsEmpty())
			return;
		
		foreach (SCR_FiringRangeTarget target : m_aTargets)
		{
			target.Event_TargetChangeState.Remove(OnTargetStateChanged);
		}
	}
};