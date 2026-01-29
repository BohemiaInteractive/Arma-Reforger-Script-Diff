[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Seizing_MoveClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Seizing_Move : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	void OnMapOpen(MapConfiguration config)
	{
		ShowHint(1);
		
		SCR_MapEntity.GetOnMapOpen().Remove(OnMapOpen);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnMapClosed(MapConfiguration config)
	{
		ShowHint(2);
		
		SCR_MapEntity.GetOnMapClose().Remove(OnMapClosed);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DelayedSpeech()
	{
		if (vector.Distance(m_Player.GetOrigin(), GetGame().GetWorld().FindEntityByName("SpawnPos_CONFLICT_InstructorB").GetOrigin()) < 50)
			return;
		
		SCR_VoiceoverSystem.GetInstance().PlaySequence("OneMoreThing");
		GetGame().GetCallqueue().Remove(DelayedSpeech);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_SEIZING_1");
		CreateMarkerPath("SEIZING_PATH_DOTS");

		GetGame().GetCallqueue().CallLater(DelayedSpeech, 1000, true);

		SCR_MapEntity.GetOnMapOpen().Insert(OnMapOpen);
		SCR_MapEntity.GetOnMapClose().Insert(OnMapClosed);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_Tutorial_Seizing_Move()
	{
		GetGame().GetCallqueue().Remove(DelayedSpeech);
	}
};