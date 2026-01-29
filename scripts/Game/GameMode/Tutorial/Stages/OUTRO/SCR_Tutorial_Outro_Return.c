[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Outro_ReturnClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Outro_Return : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		if (!m_Player)
			m_TutorialComponent.GetOnPlayerSpawned().Insert(DelayedSetup);
		else
			DelayedSetup();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DelayedSetup()
	{
		m_Player = m_TutorialComponent.GetPlayer();
		if (!m_Player)
			return;
		
		//Spawn captain, if he doesn't exist
		ChimeraCharacter outroCaptain =  ChimeraCharacter.Cast(m_TutorialComponent.SpawnAsset("Captain_Table", "{2537DEB68616183E}Prefabs/Characters/Tutorial/Instructors/Finish_Instructor.et"));
		if (outroCaptain)
			SCR_VoiceoverSystem.GetInstance().RegisterActor(outroCaptain);
		
		RegisterWaypoint("SpawnPos_Captain_Table");
		//PlayNarrativeCharacterStage("Captain_Table", 2);
		SCR_VoiceoverSystem.GetInstance().PlaySequence("Outro");
		SetPermanentNarrativeStage("Captain_Table", 3);
		m_fStartTimestamp = float.MAX;
		
		SCR_Task task = m_TutorialComponent.GetCourseTask(SCR_ETutorialCourses.OUTRO);
		if (!task)
			return;
		
		task.SetTaskVisibility(SCR_ETaskVisibility.EVERYONE);
		
		SCR_TutorialLogic_FreeRoam logic = SCR_TutorialLogic_FreeRoam.Cast(m_Logic);
		if (logic)
			logic.PrepareFastTravelSigns();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnVoiceoverEventFinished(string eventName)
	{
		if (eventName == "SOUND_TUTORIAL_AM_CAPTAIN_SPEAKING_ON_PHONE_CAPTAIN_01")
			m_fStartTimestamp = GetGame().GetWorld().GetWorldTime();
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		return m_sLastFinishedEvent == "SOUND_TUTORIAL_AM_CAPTAIN_SPEAKING_ON_PHONE_CAPTAIN_08" || GetDuration() > 35000;
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_Tutorial_Outro_Return()
	{
		if (!m_TutorialComponent)
			return;
		
		//TODO nullchecks
		SCR_Task task = m_TutorialComponent.GetCourseTask(SCR_ETutorialCourses.OUTRO);
		if (!task)
			return;
		
		ArmaReforgerScripted game = GetGame();
		if (!game)
			return;
		
		FactionManager factionMan = game.GetFactionManager();
		if (!factionMan)
			return;
		
		task.SetTaskVisibility(SCR_ETaskVisibility.NONE);
	}
}