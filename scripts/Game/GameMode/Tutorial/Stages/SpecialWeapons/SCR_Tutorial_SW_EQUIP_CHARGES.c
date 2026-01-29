[EntityEditorProps(insertable: false)]
class SCR_Tutorial_SW_EQUIP_CHARGESClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_SW_EQUIP_CHARGES: SCR_BaseTutorialStage
{
	protected SCR_CharacterInventoryStorageComponent m_PlayerInventory;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		m_PlayerInventory = SCR_CharacterInventoryStorageComponent.Cast(m_Player.FindComponent(SCR_CharacterInventoryStorageComponent));
		
		SCR_VoiceoverSystem.GetInstance().RegisterActor(GetGame().GetWorld().FindEntityByName("SPECIALWEAPONS_InstructorN"));
		PlayNarrativeCharacterStage("SPECIALWEAPONS_InstructorN", 1);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{	
		if (!m_PlayerInventory)
			return false;
		
		IEntity ent = m_PlayerInventory.GetCurrentItem();
		if (!ent)
			return false;
		
		return ent.FindComponent(SCR_ExplosiveChargeComponent);
	}
};