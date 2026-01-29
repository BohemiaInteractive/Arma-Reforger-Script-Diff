[EntityEditorProps(insertable: false)]
class SCR_Tutorial_SW_THROW_GRENADEClass : SCR_BaseTutorialStageClass
{
}

class SCR_Tutorial_SW_THROW_GRENADE : SCR_BaseTutorialStage
{
	protected bool m_bGrenadeInTrigger;

	//------------------------------------------------------------------------------------------------
	override void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();

		if (m_TutorialComponent)
			m_TutorialComponent.EnableArsenal("SW_ARSENAL_GRENADES", true);

		RegisterWaypoint("AR_RANGE_ARSENAL_GRENADES", "", "AMMOGRENADE");

		SCR_BaseTriggerEntity grenadeTrigger = SCR_BaseTriggerEntity.Cast(GetGame().GetWorld().FindEntityByName("GRENADE_TRIGGER"));
		if (!grenadeTrigger)
			return;

		RegisterWaypoint(grenadeTrigger, "", "EXPLOSIVES").SetOffsetVector("0 2 0");

		grenadeTrigger.EnablePeriodicQueries(true);
		grenadeTrigger.SetUpdateRate(0.1);
		grenadeTrigger.GetOnActivate().Insert(OnActivate);
		grenadeTrigger.GetOnDeactivate().Insert(OnDeactivate);

		m_TutorialComponent.SetupTargets("TARGET_GRENADE", null, ETargetState.TARGET_UP, true);

		PlayNarrativeCharacterStage("SPECIALWEAPONS_InstructorM", 2);
	}

	//------------------------------------------------------------------------------------------------
	void OnActivate(IEntity ent)
	{
		m_bGrenadeInTrigger = true;
		m_fDuration = 4; //We need to be sure that grenade didn't hop out
	}

	//------------------------------------------------------------------------------------------------
	void OnDeactivate()
	{
		m_bGrenadeInTrigger = false;
		m_fDuration = 0;
	}

	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		return m_bGrenadeInTrigger;
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_Tutorial_SW_THROW_GRENADE()
	{
		if (m_TutorialComponent)
		{
			m_TutorialComponent.EnableArsenal("SW_ARSENAL_GRENADES", false);
			m_TutorialComponent.SetupTargets("TARGET_GRENADE", null, ETargetState.TARGET_DOWN, false);
		}

		SCR_BaseTriggerEntity grenadeTrigger = SCR_BaseTriggerEntity.Cast(GetGame().GetWorld().FindEntityByName("GRENADE_TRIGGER"));
		if (!grenadeTrigger)
			return;

		grenadeTrigger.EnablePeriodicQueries(false);
		grenadeTrigger.GetOnActivate().Remove(OnActivate);
		grenadeTrigger.GetOnDeactivate().Remove(OnDeactivate);
	}
}
