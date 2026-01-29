[EntityEditorProps(insertable: false)]
class SCR_Tutorial_SW_EQUIP_DETONATORClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_SW_EQUIP_DETONATOR: SCR_BaseTutorialStage
{
	protected SCR_CharacterInventoryStorageComponent m_PlayerInventory;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		m_PlayerInventory = SCR_CharacterInventoryStorageComponent.Cast(m_Player.FindComponent(SCR_CharacterInventoryStorageComponent));
		
		PlayNarrativeCharacterStage("SPECIALWEAPONS_InstructorN", 2);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{	
		if (!m_PlayerInventory)
			return false;
		
		IEntity ent = m_PlayerInventory.GetCurrentItem();
		if (!ent)
			return false;
		
		return ent.FindComponent(SCR_DetonatorGadgetComponent);
	}
};