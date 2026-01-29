[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Obstacle_TIMEClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Obstacle_TIME : SCR_BaseTutorialStage
{
	vector m_vInstructorPos;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		ChimeraWorld world = GetGame().GetWorld();
			
		if (world)
			world.GetMusicManager().Stop("SOUND_OBSTACLE_COURSE");
		
		float currentTime = GetGame().GetWorld().GetWorldTime();
		float originalTime = m_TutorialComponent.GetSavedTime();
		
		int finalTimeMS = Math.Floor(currentTime - originalTime);
		int finalTimeS = Math.Floor(finalTimeMS / 1000);
		int ms = (finalTimeMS % (finalTimeS * 1000));
		
		string time;
		
		int hour;
		int minute;
		int second;
		
		SCR_DateTimeHelper.GetHourMinuteSecondFromSeconds(finalTimeS, hour, minute, second);
		
		string buffer;
		
		if (hour > 0)
		{
			buffer = hour.ToString();
			time += buffer + ":";
		}
		
		buffer = minute.ToString();
		
		if (minute > 0 && minute < 10)
			time += "0";
		
		time += buffer + ":";
		
		buffer = second.ToString();
		
		if (second < 10)
			time += "0";
		
		time += buffer + ":";
		
		buffer = ms.ToString();
		
		if (ms < 10)
			time += "00";
		else if (ms < 100)
			time += "0";
		
		time += buffer;

		SCR_HintUIInfo info = m_StageInfo.GetHint();
		
		if (!info)
			return;
		
		LocalizedString stringToEdit = info.GetDescription();
		
		if (!stringToEdit)
			return;

		stringToEdit = stringToEdit + " " + time;
		
		SCR_HintUIInfo hintInfo = SCR_HintUIInfo.CreateInfo(stringToEdit, string.Empty, -1, EHint.UNDEFINED, EFieldManualEntryId.NONE, false);
		SCR_HintManagerComponent.ShowHint(hintInfo);

		if (finalTimeMS < 73000)
			PlayNarrativeCharacterStage("OBSTACLECOURSE_Instructor", 17);	// Good
		else if (finalTimeMS < 77000)
			PlayNarrativeCharacterStage("OBSTACLECOURSE_Instructor", 16);	// Average
		else
			PlayNarrativeCharacterStage("OBSTACLECOURSE_Instructor", 18);	// Bad
		
		IEntity ent = GetGame().GetWorld().FindEntityByName("SpawnPos_OBSTACLECOURSE_Instructor");
		if (ent)
			m_vInstructorPos = ent.GetOrigin();
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		if (vector.Distance(m_vInstructorPos, m_Player.GetOrigin()) > 40)
			return true;
		
		return m_sLastFinishedEvent == "SOUND_TUTORIAL_EVE_END_COURSE_INSTRUCTOR_A_02" || GetDuration() > 25000;
	}
};