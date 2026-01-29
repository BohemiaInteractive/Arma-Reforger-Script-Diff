[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Navigation_COMPASSClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Navigation_COMPASS : SCR_BaseTutorialStage
{
	SCR_CharacterInventoryStorageComponent m_PlayerInventory;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		m_PlayerInventory = SCR_CharacterInventoryStorageComponent.Cast(m_Player.FindComponent(SCR_CharacterInventoryStorageComponent));

		SCR_MapEntity.GetOnMapClose().Remove(m_TutorialComponent.OnMapClose);
		SCR_MapEntity.GetOnMapClose().Insert(m_TutorialComponent.OnMapClose);
		
		PlayNarrativeCharacterStage("NAVIGATION_Instructor", 5);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{	
		if (!m_PlayerInventory)
			return false;
		
		IEntity ent = m_PlayerInventory.GetCurrentItem();
		if (!ent)
			return false;
		
		SCR_CompassComponent compassComp = SCR_CompassComponent.Cast(ent.FindComponent(SCR_CompassComponent));
		if (!compassComp)
			return false;
	
		return compassComp;
	}
};