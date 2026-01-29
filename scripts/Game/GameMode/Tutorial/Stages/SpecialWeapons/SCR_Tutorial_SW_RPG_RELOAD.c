[EntityEditorProps(insertable: false)]
class SCR_Tutorial_SW_RPG_RELOADClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_SW_RPG_RELOAD : SCR_BaseTutorialStage
{
	SCR_CharacterInventoryStorageComponent m_StorageComponent;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_TutorialLogic_SW.Cast(m_Logic).m_OnAmmoAddedChanged.Insert(OnAmmoChanged);
		
		m_StorageComponent = SCR_CharacterInventoryStorageComponent.Cast(m_Player.FindComponent(SCR_CharacterInventoryStorageComponent));
		
		RegisterWaypoint("SW_ARSENAL_USSR", "", "AMMO");
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsTargetDestroyed()
	{
		IEntity target = GetGame().GetWorld().FindEntityByName("TARGET_1");
		if (!target)
			return true;
		
		IEntity destructible = target.GetChildren();
		if (!destructible)
			return true;
		
		SCR_DestructionMultiPhaseComponent destructionComp = SCR_DestructionMultiPhaseComponent.Cast(destructible.FindComponent(SCR_DestructionMultiPhaseComponent));
		if (destructionComp)
			return destructionComp.GetState() == EDamageState.DESTROYED;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void OnAmmoChanged()
	{
		SCR_TutorialLogic_SW.Cast(m_Logic).m_OnAmmoAddedChanged.Remove(OnAmmoChanged);
		
		if (!IsTargetDestroyed())
			m_TutorialComponent.SetStage("RPG_FIRE");
		
		m_bFinished = true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		return m_bFinished;
	}
}