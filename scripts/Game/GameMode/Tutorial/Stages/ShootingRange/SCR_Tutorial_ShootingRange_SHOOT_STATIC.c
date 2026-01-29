[EntityEditorProps(insertable: false)]
class SCR_Tutorial_ShootingRange_SHOOT_STATICClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_ShootingRange_SHOOT_STATIC : SCR_BaseTutorialStage
{
	protected ref array <SCR_FiringRangeTarget> m_aTargets;
	protected BaseMuzzleComponent m_MuzzleComponent;
	
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();

		m_aTargets = {};
		
		m_TutorialComponent.SetupTargets("TARGETS_WRECK_TRUCK", m_aTargets, ETargetState.TARGET_UP, false);
		foreach (SCR_FiringRangeTarget target : m_aTargets)
		{
			if (target)
				target.Event_TargetChangeState.Insert(OnTargetStateChanged);
		}
		
		PlayNarrativeCharacterStage("SHOOTINGRANGE_Instructor", 14);
		
		IEntity ent = GetGame().GetWorld().FindEntityByName("M2");
		if (!ent)
			return;
		
		WeaponSlotComponent weaponSlot = WeaponSlotComponent.Cast(ent.FindComponent(WeaponSlotComponent));
		if (!weaponSlot)
			return;
		
		ent = weaponSlot.GetWeaponEntity();
		if (!ent)
			return;
		
		WeaponComponent weaponComp = WeaponComponent.Cast(ent.FindComponent(WeaponComponent));
		if (weaponComp)
			m_MuzzleComponent = weaponComp.GetCurrentMuzzle();
	
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
		if (m_MuzzleComponent && m_MuzzleComponent.GetAmmoCount() == 0)
			return true;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_Tutorial_ShootingRange_SHOOT_STATIC()
	{
		if (!m_aTargets || m_aTargets.IsEmpty())
			return;
		
		foreach (SCR_FiringRangeTarget target : m_aTargets)
		{
			target.Event_TargetChangeState.Remove(OnTargetStateChanged);
		}
	}
};