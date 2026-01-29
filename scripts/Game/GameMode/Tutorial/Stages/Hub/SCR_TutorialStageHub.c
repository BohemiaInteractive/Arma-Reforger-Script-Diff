[EntityEditorProps(insertable: false)]
class SCR_TutorialStageHubClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialStageHub : SCR_BaseTutorialStage
{	
	protected const int INSTRUCTOR_MAXIMUM_DISTANCE_LONG = 75;
	protected const int INSTRUCTOR_MAXIMUM_DISTANCE_SHORT = 10;
	
	protected ref array<SCR_TutorialInstructorComponent> m_aInstructors;
	protected SCR_Waypoint m_VehicleWP;
	
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		if (!m_Player)
			m_TutorialComponent.GetOnPlayerSpawned().Insert(OnPlayerSpawned);
		else
			OnPlayerSpawned();
		
		GetGame().GetCallqueue().CallLater(PlayLoudspeaker, Math.RandomInt(5, 30) * 1000);
		
		// Pilot in the hub should change his lines when heli course has been finished
		if (SCR_Enum.HasFlag(m_TutorialComponent.GetFinishedCourses(), SCR_ETutorialCourses.HELICOPTERS))
			SetPermanentNarrativeStage("Soldier_Friend", 2);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void PrepareWorldAssets()
	{
		SCR_ETutorialCourses finishedCourses = m_TutorialComponent.GetFinishedCourses();
		finishedCourses = SCR_Enum.RemoveFlag(finishedCourses, SCR_ETutorialCourses.INTRO);
		
		//This will happen after first course is finished (Excluding INTRO and possible FREE_ROAM reactivation)
		if (finishedCourses > SCR_ETutorialCourses.FREE_ROAM)
		{
			BaseWorld world = GetGame().GetWorld();
			if (!world)
				return;
			
			SCR_EntityHelper.DeleteEntityAndChildren(world.FindEntityByName("Soldier_DoorGuard"));
			SCR_EntityHelper.DeleteEntityAndChildren(world.FindEntityByName("Captain_Outside"));
			SCR_EntityHelper.DeleteEntityAndChildren(world.FindEntityByName("Soldier_Outside_01"));
			SCR_EntityHelper.DeleteEntityAndChildren(world.FindEntityByName("Soldier_Outside_02"));
			
			SCR_EntityHelper.DeleteEntityAndChildren(world.FindEntityByName("WCDrill"));
			SCR_EntityHelper.DeleteEntityAndChildren(world.FindEntityByName("WCMaggot"));
			SCR_EntityHelper.DeleteEntityAndChildren(world.FindEntityByName("PukeStain"));
			
			SCR_EntityHelper.DeleteEntityAndChildren(world.FindEntityByName("Briefing_Officer"));
			SCR_EntityHelper.DeleteEntityAndChildren(world.FindEntityByName("Line_Soldier_1"));
			SCR_EntityHelper.DeleteEntityAndChildren(world.FindEntityByName("Line_Soldier_2"));
			SCR_EntityHelper.DeleteEntityAndChildren(world.FindEntityByName("Line_Soldier_3"));
			SCR_EntityHelper.DeleteEntityAndChildren(world.FindEntityByName("Line_Soldier_4"));
			SCR_EntityHelper.DeleteEntityAndChildren(world.FindEntityByName("Line_Soldier_5"));
			SCR_EntityHelper.DeleteEntityAndChildren(world.FindEntityByName("Line_Soldier_6"));
			SCR_EntityHelper.DeleteEntityAndChildren(world.FindEntityByName("Line_Soldier_7"));
			
			ChimeraCharacter outroCaptain = ChimeraCharacter.Cast(GetGame().GetWorld().FindEntityByName("Captain_Table"));
			SCR_DamageManagerComponent damMan; 
			if (outroCaptain)
				damMan =  outroCaptain.GetDamageManager();
			
			if (!outroCaptain || damMan.IsDestroyed() || (damMan.GetHealth() < damMan.GetMaxHealth()))
			{
				outroCaptain =  ChimeraCharacter.Cast(m_TutorialComponent.SpawnAsset("Captain_Table", "{2537DEB68616183E}Prefabs/Characters/Tutorial/Instructors/Finish_Instructor.et"));
				SCR_VoiceoverSystem.GetInstance().RegisterActor(outroCaptain);
			}
		}
		else if (m_TutorialComponent.GetFreeRoamActivations() > 1)
		{
			m_TutorialComponent.SpawnCourseAssets(SCR_ETutorialCourses.INTRO);
			SCR_EntityHelper.DeleteEntityAndChildren(GetGame().GetWorld().FindEntityByName("Soldier_DoorGuard"));
			SCR_VoiceoverSystem.GetInstance().RegisterActor(GetGame().GetWorld().FindEntityByName("Captain_Outside"));
			SetPermanentNarrativeStage("Captain_Outside", -1);
		}
		
		//Raise targets, if Shooting course is finished
		ETargetState state;
		bool autoRaise = SCR_Enum.HasFlag(finishedCourses, SCR_ETutorialCourses.SHOOTING_RANGE);
		if (autoRaise)
			state = ETargetState.TARGET_UP;
		else
			state = ETargetState.TARGET_DOWN;
		
		m_TutorialComponent.SetupTargets("TARGETS_BACK", null, state, autoRaise);
		m_TutorialComponent.SetupTargets("TARGETS_ECHELON_LEFT", null, state, autoRaise);
		m_TutorialComponent.SetupTargets("TARGETS_ECHELON_RIGHT", null, state, autoRaise);
		m_TutorialComponent.SetupTargets("TARGETS_FRONT", null, state, autoRaise);
		m_TutorialComponent.SetupTargets("TARGETS_MIDDLE", null, state, autoRaise);
		m_TutorialComponent.SetupTargets("TARGETS_WRECK_CAR", null, state, autoRaise);
		m_TutorialComponent.SetupTargets("TARGETS_WRECK_TRUCK", null, state, autoRaise);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnVehicleEntered( IEntity vehicle, BaseCompartmentManagerComponent manager, int mgrID, int slotID )
	{
		if (vehicle == GetGame().GetWorld().FindEntityByName("SmallJeep"))
			m_TutorialComponent.SetCourseConfig(SCR_ETutorialCourses.DRIVING);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnTaskAssigned(SCR_Task task, SCR_TaskExecutor executor, int requesterID)
	{
		SCR_WaypointDisplay display = m_TutorialComponent.GetWaypointDisplay();
		if (!display)
			return;
		
		IEntity instructorEnt;
		SCR_Waypoint wp;
		foreach (SCR_TutorialInstructorComponent instructor : m_aInstructors)
		{
			if (!instructor || !instructor.IsEnabled())
				continue;
			
			instructorEnt = instructor.GetOwner();
			if (!instructorEnt)
				continue;
			
			wp = display.FindWaypointByEntity(instructorEnt);
			if (!wp)
				continue;
			
			if (instructor.GetCourseType() == task.GetTaskID().ToInt())
				wp.m_iMaximumDrawDistance = -1;
			else
				wp.m_iMaximumDrawDistance = INSTRUCTOR_MAXIMUM_DISTANCE_SHORT;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void OnTaskUnassigned(SCR_Task task, SCR_TaskExecutor executor, int requesterID)
	{
		SCR_WaypointDisplay display = m_TutorialComponent.GetWaypointDisplay();
		if (!display)
			return;
		
		IEntity instructorEnt;
		SCR_Waypoint wp;
		foreach (SCR_TutorialInstructorComponent instructor : m_aInstructors)
		{
			if (!instructor)
				continue;
			
			instructorEnt = instructor.GetOwner();
			if (!instructorEnt)
				continue;
			
			wp = display.FindWaypointByEntity(instructorEnt);
			if (!wp)
				continue;
			
			wp.m_iMaximumDrawDistance = INSTRUCTOR_MAXIMUM_DISTANCE_LONG;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPlayerSpawned()
	{
		if (!m_Player)
			m_Player = m_TutorialComponent.GetPlayer();
		
		m_TutorialComponent.GetOnPlayerSpawned().Remove(OnPlayerSpawned);
		
		m_TutorialComponent.IncrementFreeRoamActivation();
		int freeRoamActivations = m_TutorialComponent.GetFreeRoamActivations();
		
		ChimeraWorld world = GetGame().GetWorld();
		SCR_VoiceoverSystem systemVO = SCR_VoiceoverSystem.GetInstance();
		
		systemVO.RegisterActor(world.FindEntityByName("Soldier_Friend"));
		systemVO.RegisterActor(world.FindEntityByName("radioCommander"));
		systemVO.RegisterActor(world.FindEntityByName("radioWriter"));
		systemVO.RegisterActor(world.FindEntityByName("mechanic"));
		
		if (freeRoamActivations == 1)
		{
			GetGame().GetCallqueue().CallLater(ShowHint, 5000, false, 2);
			world.GetMusicManager().Play("SOUND_FREE_ROAM");
		}
		else if (freeRoamActivations == 2)
		{
			GetGame().GetCallqueue().CallLater(ShowHint, 5000, false, 0);
		}

		m_TutorialComponent.HideTasks(false);
		m_TutorialComponent.RepairWorldVehicles();
		
		SCR_TutorialLogic_FreeRoam logic = SCR_TutorialLogic_FreeRoam.Cast(m_Logic);
		if (logic)
		{
			logic.SetUpAmbulance();
			logic.PrepareDrivingCourseInstructor();
			logic.PrepareFastTravelSigns();
			logic.PrepareHelicopter();
			MoveCopilotIntoHelicopter();
		}
		
		PrepareWorldAssets();
		
		//TODO> Could be better
		m_aInstructors = {};
		m_TutorialComponent.GetInstructors(m_aInstructors);
		
		IEntity instructorEnt;
		SCR_Waypoint waypoint;
		
		foreach (SCR_TutorialInstructorComponent instructor : m_aInstructors)
		{
			if (!instructor || !instructor.IsEnabled())
				continue;
			
			instructorEnt = instructor.GetOwner();
			if (!instructorEnt)
				continue;
			
			if (!m_TutorialComponent.IsCourseAvailable(instructor.GetCourseType()))
				continue;
			
			waypoint = RegisterWaypoint(instructorEnt, instructor.GetAssignedCourseName());
			if (!waypoint)
				continue;
			
			waypoint.EnableFading(true);
			waypoint.SetOffsetVector("0 1.9 0");
			waypoint.m_iMaximumDrawDistance = INSTRUCTOR_MAXIMUM_DISTANCE_LONG;
		}
		
		//TODO: Sort out the offset and implement better icon
		m_VehicleWP = RegisterWaypoint("PlayerVehicle", "", "MOUNTCAR");
		if (m_VehicleWP)
		{
			m_VehicleWP.SetOffsetVector("0 1.5 0");
			m_VehicleWP.EnableFading(true);
			m_VehicleWP.m_iMaximumDrawDistance = INSTRUCTOR_MAXIMUM_DISTANCE_SHORT;
		}
		
		SCR_Task.GetOnTaskAssigneeAdded().Insert(OnTaskAssigned);
		SCR_Task.GetOnTaskAssigneeRemoved().Insert(OnTaskUnassigned);
		
		if (m_Player)
		{
			SCR_CompartmentAccessComponent accessComp = SCR_CompartmentAccessComponent.Cast(m_Player.FindComponent(SCR_CompartmentAccessComponent));
			
			if (accessComp)
				accessComp.GetOnCompartmentEntered().Insert(OnVehicleEntered);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void PlayLoudspeaker()
	{
		IEntity loudspeaker = GetGame().GetWorld().FindEntityByName("AirfieldLoudspeaker");
		
		if (loudspeaker && m_Player && vector.Distance(loudspeaker.GetOrigin(), m_Player.GetOrigin()) < 300)
		{
			string variants[4] = {"AirfieldLoudspeaker_01", "AirfieldLoudspeaker_02", "AirfieldLoudspeaker_03", "AirfieldLoudspeaker_04"};
			string variant = variants[Math.RandomIntInclusive(0, 3)];
			
			SCR_VoiceoverSystem.GetInstance().PlaySequence(variant, loudspeaker, playImmediately: false);
		}

		GetGame().GetCallqueue().CallLater(PlayLoudspeaker, Math.RandomInt(30, 60) * 1000);
	}
	
	//------------------------------------------------------------------------------------------------
	void MoveCopilotIntoHelicopter()
	{
		Vehicle helicopter = Vehicle.Cast(GetGame().GetWorld().FindEntityByName("UH1COURSE"));
		if (!helicopter)
			return;
		
		ChimeraCharacter copilot = ChimeraCharacter.Cast(GetGame().GetWorld().FindEntityByName("HELICOPTER_Copilot"));
		if (copilot)
			m_TutorialComponent.MoveCharacterIntoVehicle(copilot, helicopter, ECompartmentType.PILOT, 1);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (m_VehicleWP && m_Player)
		{
			if (m_Player.IsInVehicle())
				m_VehicleWP.m_iMinimumDrawDistance = 10;
			else
				m_VehicleWP.m_iMinimumDrawDistance = -1;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_TutorialStageHub()
	{
		GetGame().GetCallqueue().Remove(SCR_HintManagerComponent.ShowHint);
		GetGame().GetCallqueue().Remove(PlayLoudspeaker);
		
		SCR_Task.GetOnTaskAssigneeAdded().Remove(OnTaskAssigned);
		SCR_Task.GetOnTaskAssigneeRemoved().Remove(OnTaskUnassigned);
		
		if (m_Player)
		{
			SCR_CompartmentAccessComponent accessComp = SCR_CompartmentAccessComponent.Cast(m_Player.FindComponent(SCR_CompartmentAccessComponent));
			
			if (accessComp)
				accessComp.GetOnCompartmentEntered().Remove(OnVehicleEntered);
		}
	}
};
