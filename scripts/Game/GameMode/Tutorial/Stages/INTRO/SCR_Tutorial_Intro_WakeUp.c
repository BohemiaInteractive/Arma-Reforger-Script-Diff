[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Intro_WakeUpClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Intro_WakeUp : SCR_BaseTutorialStage
{
	SCR_CharacterControllerComponent m_CharController;
	
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		if (!m_Player)
			m_TutorialComponent.GetOnPlayerSpawned().Insert(OnPlayerSpawned);
		else
			OnPlayerSpawned();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPlayerSpawned()
	{
		m_fStartTimestamp = GetGame().GetWorld().GetWorldTime();
		
		GetGame().GetCallqueue().Remove(ShowHint);
		GetGame().GetCallqueue().CallLater(ShowHint, m_StageInfo.GetFailsafeHintTimeout() * 1000, false, 0);
		
		if (!m_Player)
			m_Player = m_TutorialComponent.GetPlayer();
		
		m_CharController = SCR_CharacterControllerComponent.Cast(m_Player.FindComponent(SCR_CharacterControllerComponent));

		m_TutorialComponent.GetOnPlayerSpawned().Remove(OnPlayerSpawned);
		m_TutorialComponent.PlayBedAnimation(true);
		
		//Delete items from player inventory for intro reasons
		const SCR_InventoryStorageManagerComponent inventory = m_TutorialComponent.GetPlayerInventory();
		IEntity binoculars = inventory.FindItem(SCR_ResourceNamePredicate("{0CF54B9A85D8E0D4}Prefabs/Items/Equipment/Binoculars/Binoculars_M22/Binoculars_M22.et"), EStoragePurpose.PURPOSE_ANY);
		SCR_EntityHelper.DeleteEntityAndChildren(binoculars);
		
		IEntity gloves = inventory.FindItem(SCR_ResourceNamePredicate("{D19C474FA0F42CC4}Prefabs/Characters/Handwear/Gloves_Pilot_US_01/Gloves_Pilot_US_01.et"), EStoragePurpose.PURPOSE_ANY);
		SCR_EntityHelper.DeleteEntityAndChildren(gloves);

		SCR_VoiceoverSystem.GetInstance().RegisterActor(GetGame().GetWorld().FindEntityByName("Soldier_DoorGuard"));

		GetGame().GetCallqueue().CallLater(PlayNarrativeCharacterStage, 1000, false, "Soldier_DoorGuard", 3);
		
		SCR_TutorialLogic_FreeRoam logic = SCR_TutorialLogic_FreeRoam.Cast(m_Logic);
		if (!logic)
			return;
		
		logic.SetUpAmbulance();
		logic.PrepareDrivingCourseInstructor();
	}

	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (!m_CharController)
			return false;

		return !m_CharController.IsLoitering() && GetDuration() > 2000;
	}
}