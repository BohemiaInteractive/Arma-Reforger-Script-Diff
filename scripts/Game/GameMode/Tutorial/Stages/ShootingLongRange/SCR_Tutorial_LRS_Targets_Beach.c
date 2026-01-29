[EntityEditorProps(insertable: false)]
class SCR_Tutorial_LRS_Targets_BeachClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_LRS_Targets_Beach : SCR_BaseTutorialStage
{
	protected ref array <SCR_FiringRangeTarget> m_aTargets;
	protected SCR_Waypoint m_Waypoint;
	
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		m_aTargets = {};
		
		m_Waypoint = RegisterWaypoint("WP_TARGETS_CLOSER", "", "TARGETCHAR");
		
		m_TutorialComponent.SetupTargets("TARGETS_BEACH", m_aTargets, ETargetState.TARGET_UP, false);
		foreach (SCR_FiringRangeTarget target : m_aTargets)
		{
			if (target)
				target.Event_TargetChangeState.Insert(OnTargetStateChanged);
		}
		
		PlayNarrativeCharacterStage("LONGRANGESHOOTING_Instructor", 6);
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
		if (m_TutorialComponent && m_TutorialComponent.GetRemainingAmmo() == 0)
			RegisterWaypoint("Ammobox3", "", "AMMO");
		else
			UnregisterWaypoint("Ammobox3");
		
		CharacterControllerComponent charController = m_Player.GetCharacterController();
		
		if (charController)
		{
			if (charController.IsWeaponADS())
				m_Waypoint.SetOffsetVector("0 20 0");
			else
				m_Waypoint.SetOffsetVector("0 0 0");
		}

		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_Tutorial_LRS_Targets_Beach()
	{
		if (!m_aTargets || m_aTargets.IsEmpty())
			return;
		
		foreach (SCR_FiringRangeTarget target : m_aTargets)
		{
			target.Event_TargetChangeState.Remove(OnTargetStateChanged);
		}
	}
};