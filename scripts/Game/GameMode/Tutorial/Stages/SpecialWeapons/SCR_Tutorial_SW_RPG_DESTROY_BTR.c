[EntityEditorProps(insertable: false)]
class SCR_Tutorial_SW_RPG_DESTROY_BTRClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_SW_RPG_DESTROY_BTR : SCR_BaseTutorialStage
{
	
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		IEntity btr = GetGame().GetWorld().FindEntityByName("TARGET_BTR");
		if (!btr)
		{
			m_TutorialComponent.InsertStage("RPG_TARGET");
			m_bFinished = true;
			return;
		}
		
		SCR_WheeledDamageManagerComponent btrDamageManager = SCR_WheeledDamageManagerComponent.Cast(btr.FindComponent(SCR_WheeledDamageManagerComponent));
		if (!btrDamageManager)
			return;
		
		if (btrDamageManager.GetState() >= EDamageState.INTERMEDIARY)
		{
			m_TutorialComponent.InsertStage("RPG_TARGET");
			m_bFinished = true;
			return;
		}

		PlayNarrativeCharacterStage("SPECIALWEAPONS_InstructorM", 10);

		SCR_Waypoint wp = RegisterWaypoint(btr, string.Empty, "EXPLOSIVES");
		if (!wp)
			return
		
		wp.SetOffsetVector("0 5 0"); 
		wp.ShowDistance(false);
		
		btrDamageManager.GetOnDamageStateChanged().Insert(OnDamageStateChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnDamageStateChanged(EDamageState state)
	{
		IEntity btr = GetGame().GetWorld().FindEntityByName("TARGET_BTR");
		if (!btr)
			return;
		
		SCR_WheeledDamageManagerComponent btrDamageManager = SCR_WheeledDamageManagerComponent.Cast(btr.FindComponent(SCR_WheeledDamageManagerComponent));
		if (!btrDamageManager)
			return;
		
		btrDamageManager.GetOnDamageStateChanged().Remove(OnDamageStateChanged);
		
		m_bFinished = true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		return false;
	}
}