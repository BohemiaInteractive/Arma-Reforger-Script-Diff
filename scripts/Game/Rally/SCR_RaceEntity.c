class SCR_RaceEntityClass : GenericEntityClass
{
}

class SCR_RaceEntity : GenericEntity
{
	/*
		Race
	*/

	//! Medals, best first. In SP the first (gold) time is replaced by the finish time of the ghost the player races against
	//! (their best run or the default ghost, whichever is faster) and the others keep their offset from it, see GetMedalTimeForGhost().
	//! The authored times are used as they are in MP or when there is no ghost.
	[Attribute(category: "Race")]
	ref array<ref SCR_RaceRecord> m_aRaceRecords;

	[Attribute(desc: "Starting position of race", category: "Race")]
	protected ref PointInfo m_raceStartingPoint;

	[Attribute(desc: "Camera position used for the RACE view in the customization menu, track-specific", category: "Race")]
	protected ref SCR_CustomizationCameraPointInfo m_raceCamPosition;

	[Attribute("0", desc: "Amount of laps if the race is circuit", params: "0 inf", category: "Race")]
	protected int m_iLapCount;

	[Attribute(category: "Race")]
	protected string m_sRaceName;

	[Attribute(category: "Race")]
	protected string m_sRaceDescription;

	[Attribute(category: "Race", params: "edds")]
	protected ResourceName m_sRaceImage;

	/*
		Ghost
	*/

	[Attribute(category: "Ghost", params: "emat")]
	ResourceName m_ghostMaterial;

	//! Track-authored default ghost, loaded straight from this .conf resource - completely independent
	//! of SCR_PersistentRaceData/SaveGameManager. SP only; see StartPlayingGhost().
	[Attribute(category: "Ghost", params: "conf class=SCR_GhostSave")]
	ResourceName m_sDefaultGhostFile;

	protected ref array<SCR_RaceCheckpoint> m_aRaceCheckpoints = {};
	protected float m_fTime;
	protected ref array<ref SCR_RacerData> m_aRacers = {};
	protected int m_iRaceCheckpointCount;

	protected ref ScriptInvokerVoid m_OnRacePrepared;

	// MPREWORK::we support only one race per map, in other case this will need to be changed
	static ref ScriptInvokerInt s_OnRaceStarted = new ScriptInvokerInt();
	static ref ScriptInvokerVoid s_OnRaceFinished = new ScriptInvokerVoid();

	protected ref ScriptInvokerEntity m_OnCheckpointsUpdated;
	protected ref ScriptInvokerEntity m_OnCheckpointMissed;
	
	protected ref ScriptInvokerVehicle m_OnVehicleRecovery;
	protected ref ScriptInvokerVehicle m_OnVehicleFlipped;
	
	protected SCR_RaceTimeInfo m_Display;
	protected bool m_bIsRaceRunning;
	protected bool m_bIsRacePrepared;
	protected SCR_RaceSave m_RecordRace;
	protected float m_fLockedGhostFinishTime = -1;	//!< ghost finish time the current/last run's medals are based on, see LockMedalTimes()

	protected ref array<ref Shape> m_aGhostDebugShapes = {};
	protected ref SCR_GhostSave m_GhostData;
	protected IEntity m_GhostVehicle;
	protected ParametricMaterialInstanceComponent m_GhostMaterialComponent;
	protected int m_iGhostSaveTimer;

	protected static const ResourceName GHOST_VEHICLE_PREFAB = "{C4336BA903093340}Prefabs/Props/RallyDLC/GhostVehicle_Base.et";
	
	protected SCR_RaceCheckpoint m_NextCheckpoint;
	protected SCR_RaceCheckpoint m_StartCheckpoint;
	protected WorldTimestamp m_LastMissedCheckpointCheck;
	protected ref array<vector> m_RaceTrack = {};
	protected int m_iCircularity = -1;
	protected bool m_bOnClosingLeg;
	protected float m_fTrackLength;

	protected WorldTimestamp m_LastFlippedCheck;
	protected ref array<Vehicle> m_aVehicleFlippedRaisedEvent = {};

	protected TimeAndWeatherManagerEntity m_TimeWeatherManager;

	protected Widget m_wRestartFadeWidget;
	protected SCR_FadeUIComponent m_RestartFadeComponent;

	static const int RACE_CHECKPOINT_START_ID = int.MIN;
	static const int RACE_CHECKPOINT_END_ID = int.MAX;

	static const int GHOST_SAVE_FREQUENCY = 100; //!< milliseconds

	//! Ghost is at GHOST_FADE_MAX_ALPHA opacity at/beyond this distance from the player's vehicle, GHOST_FADE_MIN_ALPHA at/below GHOST_FADE_NEAR_DIST - it never fully disappears.
	protected static const float GHOST_FADE_FAR_DIST = 15;
	protected static const float GHOST_FADE_NEAR_DIST = 3;
	protected static const float GHOST_FADE_MIN_ALPHA = 0.15;
	protected static const float GHOST_FADE_MAX_ALPHA = 0.9;

	protected static const int FLIPPED_CHECK_FREQUENCY = 500;	//!< milliseconds
	protected static const int MISSED_CHECK_FREQUENCY = 2000;	//!< milliseconds
	protected static const int FLIPPED_TIME_TRIGGER = 1500;		//!< milliseconds
	protected static const float FLIPPED_MAX_SPEED = 1.5;		//!< km/h
	protected static const float FLIPPED_MAX_SPEED_SQ = Math.Pow(FLIPPED_MAX_SPEED / 3.6, 2); //!< m/s²

	protected static const int TELEPORT_DELAY = 100;		//!< ms
	protected static const int WAKE_UP_DELAY = 1500;		//!< ms, time given to an unconscious racer to wake up before a restart puts them into a car

	protected static const int BLACKIN_RACE_START = 1500;	//!< ms
	protected static const int BLACKIN_RACE_RESTART = 500;	//!< ms

	protected static const ResourceName BLACK_OUT_IN_LAYOUT = "{4F40BB35BC44C946}UI/layouts/HUD/TeleportFeedback/HUD_GM_TeleportingFeedback.layout";

	//------------------------------------------------------------------------------------------------
	void CheckpointsUpdated(IEntity checkpoint)
	{
		SCR_RaceCheckpoint thisCheckpoint = SCR_RaceCheckpoint.Cast(checkpoint);
		if (!thisCheckpoint)
		{
			Print("SCR_RaceEntity::CheckpointsUpdated: Race checkpoint has a wrong class!", LogLevel.ERROR);
			return;
		}
		
		m_NextCheckpoint = FindNextCheckpoint(thisCheckpoint);
		if (IsCircuit())
		{
			m_bOnClosingLeg = m_NextCheckpoint == GetStartCheckpoint();
			if (m_bOnClosingLeg)
				m_NextCheckpoint.SetDistanceFromStart(m_fTrackLength);
		}
		
		if (m_OnCheckpointsUpdated)
			m_OnCheckpointsUpdated.Invoke(checkpoint);
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerEntity GetOnCheckpointsUpdated()
	{
		if (!m_OnCheckpointsUpdated)
			m_OnCheckpointsUpdated = new ScriptInvokerEntity();

		return m_OnCheckpointsUpdated;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerEntity GetOnCheckpointMissed()
	{
		if (!m_OnCheckpointMissed)
			m_OnCheckpointMissed = new ScriptInvokerEntity();

		return m_OnCheckpointMissed;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerVehicle GetOnVehicleRecovery()
	{
		if (!m_OnVehicleRecovery)
			m_OnVehicleRecovery = new ScriptInvokerVehicle();

		return m_OnVehicleRecovery;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerVehicle GetOnVehicleFlipped()
	{
		if (!m_OnVehicleFlipped)
			m_OnVehicleFlipped = new ScriptInvokerVehicle();

		return m_OnVehicleFlipped;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnRacePrepared()
	{
		if (!m_OnRacePrepared)
			m_OnRacePrepared = new ScriptInvokerVoid();

		return m_OnRacePrepared;
	}

	//------------------------------------------------------------------------------------------------
	void RaceStarted(int playerID)
	{
		if (s_OnRaceStarted)
			s_OnRaceStarted.Invoke(playerID);
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerInt GetOnRaceStarted()
	{
		if (!s_OnRaceStarted)
			s_OnRaceStarted = new ScriptInvokerInt();

		return s_OnRaceStarted;
	}

	//------------------------------------------------------------------------------------------------
	void RaceFinished()
	{
		if (s_OnRaceFinished)
			s_OnRaceFinished.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnRaceFinished()
	{
		if (!s_OnRaceFinished)
			s_OnRaceFinished = new ScriptInvokerVoid();

		return s_OnRaceFinished;
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		IEntity entity = GetChildren();
		SCR_RaceCheckpoint checkpoint = SCR_RaceCheckpoint.Cast(GetChildren());

		while (checkpoint)
		{
			//all checkpoints get disabled at beginning so we can later activate them in correct order
			checkpoint.DisableCheckpoint();
			m_aRaceCheckpoints.Insert(checkpoint);
			checkpoint = SCR_RaceCheckpoint.Cast(checkpoint.GetSibling());
		}

		m_iRaceCheckpointCount = m_aRaceCheckpoints.Count();

		//guard against SFX/Dialog/Music master volume being left faded out by a finish-screen
		//fade that never got restored (e.g. session stopped mid-fade in a previous scenario run)
		AudioSystem.SetMasterVolume(AudioSystem.SFX, 1);
		AudioSystem.SetMasterVolume(AudioSystem.Dialog, 1);
		AudioSystem.SetMasterVolume(AudioSystem.Music, 1);
		AudioSystem.SetMasterVolume(AudioSystem.UI, 1);
		AudioSystem.SetMasterVolume(AudioSystem.VoiceChat, 1);

		ChimeraWorld world = ChimeraWorld.CastFrom(GetGame().GetWorld());
		if (!world)
			return;

		m_TimeWeatherManager = world.GetTimeAndWeatherManager();
		if (!m_TimeWeatherManager)
			return;

		//add check to verify if the race has START and END
		if (PrepareRaceSpline(m_fTrackLength))
			PrecomputeCheckpointDistances();
		
		EnableRace();
		Activate();
	}

	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		// save ghost only once per 0.1s to reduce the save size significantly, then do transposition when spawning ghost

		m_iGhostSaveTimer = m_iGhostSaveTimer + timeSlice * 1000;

		WorldTimestamp timeStamp = owner.GetWorld().GetTimestamp();
		if (!m_LastFlippedCheck)
			m_LastFlippedCheck = timeStamp;
		
		if (!m_LastMissedCheckpointCheck)
			m_LastMissedCheckpointCheck = timeStamp;

		bool checkFlipped = timeStamp.DiffMilliseconds(m_LastFlippedCheck) >= FLIPPED_CHECK_FREQUENCY;
		bool checkMissed = timeStamp.DiffMilliseconds(m_LastMissedCheckpointCheck) >= MISSED_CHECK_FREQUENCY && !m_RaceTrack.IsEmpty();

		// foreach in frame, but the number of racers will not reach big numbers, so it won't cycle much
		foreach (SCR_RacerData racer : m_aRacers)
		{
			if (!racer)
				continue;
			
			Vehicle vehicle = racer.GetVehicle();
			//
			// ghost
			//

			if (m_bIsRaceRunning)
			{
				//opacity fade doesn't need per-frame precision, so it piggybacks on this same throttle
				bool updateGhostFade = m_iGhostSaveTimer > GHOST_SAVE_FREQUENCY;

				if (updateGhostFade)
				{
					racer.WriteFrameGhostData();
					m_iGhostSaveTimer = 0;

					racer.RecordMaximumSpeed();
				}

				if (m_GhostData && m_GhostData.m_aPositions)
				{
//					foreach (SCR_TimedPositionSave position : m_GhostData.m_aPositions)
//					{
//						if (m_TimeWeatherManager.GetEngineTime() - racer.GetStartTime() < position.m_fTime)
//						{
//							m_GhostVehicle.SetTransform(position.m_vTransform);
//							break;
//						}
//					}

					SCR_TimedPositionSave prevPos;
					foreach (int i, SCR_TimedPositionSave currPos : m_GhostData.m_aPositions)
					{
						if (i != 0 && m_TimeWeatherManager.GetEngineTime() - racer.GetStartTime() < currPos.m_fTime)
						{
							float value = Math.InverseLerp(
								m_GhostData.m_aPositions[i - 1].m_fTime,
								currPos.m_fTime,
								m_TimeWeatherManager.GetEngineTime() - racer.GetStartTime());

							vector finalTransform[4] = {
								vector.Lerp(prevPos.m_aTransform[0], currPos.m_aTransform[0], value),
								vector.Lerp(prevPos.m_aTransform[1], currPos.m_aTransform[1], value),
								vector.Lerp(prevPos.m_aTransform[2], currPos.m_aTransform[2], value),
								vector.Lerp(prevPos.m_aTransform[3], currPos.m_aTransform[3], value),
							};

							m_GhostVehicle.SetTransform(finalTransform);

							if (vehicle && updateGhostFade)
							{
								float distToPlayer = vector.Distance(finalTransform[3], vehicle.GetOrigin());
								float t = Math.Clamp(Math.InverseLerp(GHOST_FADE_NEAR_DIST, GHOST_FADE_FAR_DIST, distToPlayer), 0, 1);
								SetGhostOpacity(Math.Lerp(GHOST_FADE_MIN_ALPHA, GHOST_FADE_MAX_ALPHA, t));
							}

							break;
						}

						prevPos = currPos;
					}
				}
			}

			// flipped state test
			if (checkFlipped && vehicle)
				FlippedVehicleTest(vehicle, racer);
			
			// missed checkpoint test
			if (checkMissed && m_NextCheckpoint && vehicle)
				MissedCheckpointTest(vehicle);
		}
		
		if (checkFlipped)
			m_LastFlippedCheck = timeStamp;
		
		if (checkMissed)
			m_LastMissedCheckpointCheck = timeStamp;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool FlippedVehicleTest(notnull Vehicle vehicle, notnull SCR_RacerData racer)
	{
		vector angles = vehicle.GetAngles();
		bool isFlipped = !float.AlmostEqual(0, angles[0], 45) || !float.AlmostEqual(0, angles[2], 45);
		bool isMarkedAsFlipped = racer.IsVehicleFlipped();
		if (isMarkedAsFlipped)
		{
			if (isFlipped)
			{
				if (!m_aVehicleFlippedRaisedEvent.Contains(vehicle) && racer.GetTimeSinceFlippedVehicle() >= FLIPPED_TIME_TRIGGER)
				{
					m_aVehicleFlippedRaisedEvent.Insert(vehicle);
					if (m_OnVehicleFlipped)
						m_OnVehicleFlipped.Invoke(vehicle);
				}
			}
			else
			{
				racer.SetVehicleFlipped(false);
				m_aVehicleFlippedRaisedEvent.RemoveItem(vehicle);
				if (m_OnVehicleRecovery)
					m_OnVehicleRecovery.Invoke(vehicle);
			}
		}
		else
		if (isFlipped)
		{
			// must be flipped -and- stopped so AI does not run towards a rooftop-sliding car
			bool isStopped = !vehicle.GetPhysics() || vehicle.GetPhysics().GetVelocity().LengthSq() < FLIPPED_MAX_SPEED_SQ;
			if (isStopped)
				racer.SetVehicleFlipped(true);
		}
		return isFlipped;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool MissedCheckpointTest(notnull Vehicle vehicle)
	{
		vector racerPosition = vehicle.GetOrigin();
		float distToTrack, distToStart;
		
		distToStart = GetDistanceAlongSpline(m_RaceTrack, racerPosition, distToTrack, IsCircuit(), m_bOnClosingLeg);
		
		bool isRacerFurtherThanCheckpoint = distToStart > m_NextCheckpoint.GetDistanceFromStart();
		
		if (isRacerFurtherThanCheckpoint && m_OnCheckpointMissed)
		{
			m_OnCheckpointMissed.Invoke(m_NextCheckpoint);
			m_NextCheckpoint = null; // to fire the invoker only once, TODO: more racers must have different NextCheckpoints.
			return true;
		}
		
		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected void EnableRace()
	{
		if (m_aRaceCheckpoints.IsEmpty())
			return;

		//temporary, use specific start entity next time
		SCR_RaceCheckpoint first = FindCheckpointByID(RACE_CHECKPOINT_START_ID);
		if (first)
			first.EnableCheckpoint();

		if (m_OnRacePrepared)
			m_OnRacePrepared.Invoke();

		m_bIsRacePrepared = true;
	}

	//------------------------------------------------------------------------------------------------
	protected Vehicle SpawnRaceVehicle(SCR_RallyCustomizationData customizationData = null, ResourceName vehiclePrefab = string.Empty)
	{
		ResourceName carPath = vehiclePrefab;
		if (carPath == string.Empty)
			carPath = customizationData.GetBaseVehicle();

		Resource carPrefab = Resource.Load(carPath);
		if (!carPrefab.IsValid())
			return null;

		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;

		vector mat[4];

		m_raceStartingPoint.Init(this);
		m_raceStartingPoint.GetWorldTransform(mat);

		params.Transform = mat;

		Vehicle car = Vehicle.Cast(GetGame().SpawnEntityPrefabLocal(carPrefab, null, params));
		if (!car)
			return null;

		ApplyCustomizationToVehicle(car, customizationData);

		return car;
	}

	//------------------------------------------------------------------------------------------------
	protected void ApplyCustomizationToVehicle(notnull Vehicle car, notnull SCR_RallyCustomizationData customizationData)
	{
		ResourceName partPrefab;

		partPrefab = customizationData.GetVehiclePart(SCR_ERallyCustomizationParts.SPOILER);
		if (!partPrefab.IsEmpty())
		{
			SetVehiclePartByPrefab(SCR_ERallyCustomizationParts.SPOILER, car, partPrefab);
			partPrefab = string.Empty;
		}

		partPrefab = customizationData.GetVehiclePart(SCR_ERallyCustomizationParts.BODYKIT);
		if (!partPrefab.IsEmpty())
		{
			SetVehiclePartByPrefab(SCR_ERallyCustomizationParts.BODYKIT, car, partPrefab);
			partPrefab = string.Empty;
		}

		partPrefab = customizationData.GetVehiclePart(SCR_ERallyCustomizationParts.LIGHTS);
		if (!partPrefab.IsEmpty())
		{
			SetVehiclePartByPrefab(SCR_ERallyCustomizationParts.LIGHTS, car, partPrefab);
			partPrefab = string.Empty;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Visualizes the ghost using the vehicle it was actually recorded with (m_GhostData.m_sVehiclePrefab),
	//! when known. Legacy ghosts (recorded before this field existed) have it empty, so we fall back to
	//! carTemplate - the live player's current vehicle - which is the old behavior.
	void SpawnGhost(notnull Vehicle carTemplate)
	{
		//check if the ghost SHOULD be spawned, coz there can be no previous record
		EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;
		GetWorldTransform(spawnParams.Transform);
		spawnParams.Transform[3] = vector.Zero; // spawning at 0,0,0 as otherwise the ghost will be visible in the wrong location for a split second when starting

		Resource ghostVehicleTemplateRes = Resource.Load(GHOST_VEHICLE_PREFAB);
		if (ghostVehicleTemplateRes.IsValid())
			m_GhostVehicle = GetGame().SpawnEntityPrefabLocal(ghostVehicleTemplateRes, GetGame().GetWorld(), spawnParams);
		else
			m_GhostVehicle = GetGame().SpawnEntity(GenericEntity, GetGame().GetWorld(), spawnParams);

		m_GhostVehicle.SetFlags(EntityFlags.ACTIVE, true);

		m_GhostMaterialComponent = ParametricMaterialInstanceComponent.Cast(m_GhostVehicle.FindComponent(ParametricMaterialInstanceComponent));

		Vehicle sourceVehicle = carTemplate;
		IEntity tempGhostSourceVehicle;

		if (m_GhostData && !m_GhostData.m_sVehiclePrefab.IsEmpty())
		{
			Resource ghostVehicleRes = Resource.Load(m_GhostData.m_sVehiclePrefab);
			if (ghostVehicleRes.IsValid())
			{
				EntitySpawnParams tempParams = new EntitySpawnParams();
				tempParams.TransformMode = ETransformMode.WORLD;
				GetWorldTransform(tempParams.Transform);
				tempParams.Transform[3] = vector.Zero;

				tempGhostSourceVehicle = GetGame().SpawnEntityPrefabLocal(ghostVehicleRes, GetGame().GetWorld(), tempParams);

				Vehicle castVehicle = Vehicle.Cast(tempGhostSourceVehicle);
				if (castVehicle)
					sourceVehicle = castVehicle;
			}
		}

		VObject obj = sourceVehicle.GetVObject();

		string materials[256];
		string remap;
		for (int i, numMats = obj.GetMaterials(materials); i < numMats; i++)
		{
			remap += string.Format("$remap '%1' '%2';", materials[i], m_ghostMaterial);
		}

		m_GhostVehicle.SetObject(obj, remap);

		if (tempGhostSourceVehicle)
			delete tempGhostSourceVehicle;

		SetGhostOpacity(GHOST_FADE_MIN_ALPHA);

		return;
	}

	//------------------------------------------------------------------------------------------------
	//! Sets the ghost's opacity at runtime via its ParametricMaterialInstanceComponent (see GhostVehicle_Base.et).
	//! \param alpha 0 (invisible) to 1 (fully opaque)
	void SetGhostOpacity(float alpha)
	{
		if (!m_GhostMaterialComponent)
			return;

		int alphaByte = Math.Round(Math.Clamp(alpha, 0, 1) * 255);
		m_GhostMaterialComponent.SetColor(ARGB(alphaByte, 255, 255, 255));
	}

	//------------------------------------------------------------------------------------------------
	protected void SetVehiclePartByPrefab(SCR_ERallyCustomizationParts part, notnull Vehicle car, ResourceName prefab = string.Empty)
	{
		string partName;

		switch (part)
		{
			case SCR_ERallyCustomizationParts.SPOILER:
				partName = "Spoiler";
				break;

			case SCR_ERallyCustomizationParts.BODYKIT:
				partName = "BodyKit";
				break;

			case SCR_ERallyCustomizationParts.LIGHTS:
				partName = "RallyLights";
				break;
		}

		SlotManagerComponent slotManager = SlotManagerComponent.Cast(car.FindComponent(SlotManagerComponent));
		if (!slotManager)
			return;

		array<EntitySlotInfo> slots = {};
		slotManager.GetSlotInfos(slots);

		EntitySlotInfo foundSlot;
		foreach (EntitySlotInfo slot : slots)
		{
			if (slot.GetSourceName() == partName)
			{
				foundSlot = slot;
				break;
			}
		}

		if (!foundSlot)
			return;

		IEntity attachedEntity = foundSlot.GetAttachedEntity();
		if (attachedEntity)
		{
			foundSlot.DetachEntity();
			delete attachedEntity;
		}

		if (prefab.IsEmpty())
			return;

		Resource partPrefab = Resource.Load(prefab);
		if (!partPrefab.IsValid())
			return;

		//change this if going to MP?
		IEntity spawnedPart = GetGame().SpawnEntityPrefabLocal(partPrefab, null, null);

		foundSlot.AttachEntity(spawnedPart);
	}

	//------------------------------------------------------------------------------------------------
	void StartRace(int playerID, ResourceName vehiclePrefab = string.Empty)
	{
		//if player is already in race, its impossible to start race again
		SCR_RacerData data = GetRacerData(playerID);
		if (data)
			return;

		data = new SCR_RacerData(playerID);
		m_aRacers.Insert(data);

		Vehicle car = SpawnRaceVehicle(null, vehiclePrefab);
		if (!car)
			return;

		data.SetSpawnInfo(null, vehiclePrefab);

		PlayFadeFeedback(BLACKIN_RACE_START);

		//The compartment of vehicle is not initialised quickly enough, so teleporting has to be delayed a bit
		GetGame().GetCallqueue().CallLater(TeleportPlayerToVehicle, TELEPORT_DELAY, false, playerID, car);

		ShowRaceInfoDisplay(true);
	}

	//------------------------------------------------------------------------------------------------
	void StartRaceCustomized(int playerID, notnull SCR_RallyCustomizationData customizationData)
	{
		//if player is already in race, its impossible to start race again
		SCR_RacerData data = GetRacerData(playerID);
		if (data)
			return;

		data = new SCR_RacerData(playerID);
		m_aRacers.Insert(data);

		Vehicle car = SpawnRaceVehicle(customizationData, customizationData.GetBaseVehicle());
		if (!car)
		{
			Print(string.Format("SCR_RaceEntity::StartRaceCustomized: SpawnRaceVehicle failed for prefab '%1'", customizationData.GetBaseVehicle()), LogLevel.ERROR);
			return;
		}

		data.SetSpawnInfo(customizationData, customizationData.GetBaseVehicle());

		PlayFadeFeedback(BLACKIN_RACE_START);

		//The compartment of vehicle isnt initialized quickly enough, so teleporting has to be delayed one frame
		GetGame().GetCallqueue().CallLater(TeleportPlayerToVehicle, TELEPORT_DELAY, false, playerID, car);

		ShowRaceInfoDisplay(true);
	}

	//------------------------------------------------------------------------------------------------
	//! \return the race assigned to the map's customization point (one race per map), or null when there is none
	static SCR_RaceEntity GetActiveRace()
	{
		SCR_AdditionalGameModeSettingsComponent settings = SCR_AdditionalGameModeSettingsComponent.GetInstance();
		if (!settings)
			return null;

		SCR_RaceCustomizationPointEntity point = settings.GetCustomizationEntity();
		if (!point)
			return null;

		return SCR_RaceEntity.Cast(GetGame().GetWorld().FindEntityByName(point.GetAssignedRace()));
	}

	//------------------------------------------------------------------------------------------------
	//! Restart entry point for the pause menu and the game over screen. Unlike RestartRace() it also copes with a racer
	//! who is no longer in a state to simply be put into a new car:
	//! - alive: restarted right away
	//! - incapacitated (unconscious): healed and woken up first, restarted after WAKE_UP_DELAY
	//! - dead / no controlled character: respawned through SCR_RaceSpawnLogic, which restarts the race once the new character exists
	//! The chosen outfit is re-applied to a respawned character by SCR_RaceCustomizationPointEntity.OnPlayerSpawn.
	void RequestRestart(int playerID)
	{
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID));

		CharacterControllerComponent controller;
		if (character)
			controller = character.GetCharacterController();

		if (!controller || controller.GetLifeState() == ECharacterLifeState.DEAD)
		{
			RespawnAndRestart(playerID);
			return;
		}

		if (controller.GetLifeState() == ECharacterLifeState.INCAPACITATED)
		{
			SCR_CharacterDamageManagerComponent damageManager = SCR_CharacterDamageManagerComponent.Cast(SCR_DamageManagerComponent.GetDamageManager(character));
			if (damageManager)
				damageManager.FullHeal();

			controller.SetUnconscious(false);

			// leave time for the wake-up animation, getting into a vehicle is not possible while it plays
			GetGame().GetCallqueue().CallLater(RestartRace, WAKE_UP_DELAY, false, playerID);
			return;
		}

		RestartRace(playerID);
	}

	//------------------------------------------------------------------------------------------------
	protected void RespawnAndRestart(int playerID)
	{
		SCR_RespawnSystemComponent respawnSystem = SCR_RespawnSystemComponent.GetInstance();
		if (!respawnSystem)
		{
			Print("SCR_RaceEntity::RespawnAndRestart: no SCR_RespawnSystemComponent", LogLevel.ERROR);
			return;
		}

		SCR_RaceSpawnLogic spawnLogic = SCR_RaceSpawnLogic.Cast(respawnSystem.GetSpawnLogic());
		if (!spawnLogic)
		{
			Print("SCR_RaceEntity::RespawnAndRestart: spawn logic is not SCR_RaceSpawnLogic", LogLevel.ERROR);
			return;
		}

		// Stay fully black until RestartRace() (called after the respawn) fades back in, so the
		// freshly spawned character standing on the road is never visible
		HoldBlackout();
		spawnLogic.RespawnForRaceRestart_S(playerID);
	}

	//------------------------------------------------------------------------------------------------
	//! Restarts the race for a racer in-place, without reloading the world: resets timers, checkpoints,
	//! speed stats and ghost recording, and gives them a fresh vehicle at the start line. The Start
	//! checkpoint is re-armed but not auto-triggered - the player has to physically drive through it again.
	void RestartRace(int playerID)
	{
		SCR_RacerData data = GetRacerData(playerID);
		if (!data)
			return;

		// Spawn the new car first: if that fails the running race is left exactly as it was
		Vehicle newCar = SpawnRaceVehicle(data.GetCustomizationData(), data.GetVehiclePrefab());
		if (!newCar)
		{
			Print(string.Format("SCR_RaceEntity::RestartRace: SpawnRaceVehicle failed for prefab '%1', race not restarted", data.GetVehiclePrefab()), LogLevel.ERROR);
			return;
		}

		m_NextCheckpoint = null;
		m_bOnClosingLeg = false;

		PlayFadeFeedback(BLACKIN_RACE_RESTART);

		data.ResetForRestart();

		// reset every checkpoint back to "only Start reachable" (mirrors EOnInit)
		foreach (SCR_RaceCheckpoint checkpoint : m_aRaceCheckpoints)
		{
			checkpoint.DisableCheckpoint();
		}

		m_bIsRaceRunning = false;
		m_bIsRacePrepared = false;

		// drop the old car, spawn a fresh one with the same customization/prefab as before
		Vehicle oldCar = data.GetVehicle();
		if (oldCar)
			delete oldCar;
		
		data.SetVehicle(newCar);

		// drop the old ghost visualization; a fresh one spawns once the player crosses Start again
		if (m_GhostVehicle)
		{
			delete m_GhostVehicle;
			m_GhostVehicle = null;
		}

		//The compartment of vehicle isnt initialized quickly enough, so teleporting has to be delayed one frame
		GetGame().GetCallqueue().CallLater(TeleportPlayerToVehicle, TELEPORT_DELAY, false, playerID, newCar);

		// put spectators back at the start line, same as a genuinely fresh race
		if (m_OnRacePrepared)
			m_OnRacePrepared.Invoke();

		// re-arm the Start checkpoint - the player must physically drive through it to begin timing
		SCR_RaceCheckpoint start = FindCheckpointByID(RACE_CHECKPOINT_START_ID);
		if (start)
			start.EnableCheckpoint();

		ShowRaceInfoDisplay(true);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] show Show or Hide RaceTimeInfo Info Display, true to show info display, false to hide
	void ShowRaceInfoDisplay(bool show)
	{
		SCR_HUDManagerComponent hud = GetGame().GetHUDManager();
		if (!hud)
			return;

		m_Display = SCR_RaceTimeInfo.Cast(hud.FindInfoDisplay(SCR_RaceTimeInfo));
		if (!m_Display)
			return;

		m_Display.SetContentVisibility(show);
	}

	//------------------------------------------------------------------------------------------------
	//! Fades the screen to black and back in (reusing the base game's teleport-feedback fade widget,
	//! created directly here) so the restart doesn't read as an instant jump cut. Created on demand
	//! rather than relying on the PlayerController prefab, since the mission's active PlayerController
	//! may not have the base game's SCR_PlayerTeleportedFeedbackComponent/HUD entry wired up.
	//! \param[in] durationMS
	protected void PlayFadeFeedback(int durationMS)
	{
		if (durationMS < 1)
			return;

		if (!PrepareBlackout())
			return;

		m_RestartFadeComponent.DelayedFadeOut(durationMS, false);
	}

	//------------------------------------------------------------------------------------------------
	//! Covers the screen with black and keeps it there until the next PlayFadeFeedback() fades it out
	protected void HoldBlackout()
	{
		PrepareBlackout();
	}

	//------------------------------------------------------------------------------------------------
	//! Creates the fade widget if needed and sets it to fully opaque
	//! \return false if the widget could not be created
	protected bool PrepareBlackout()
	{
		SCR_HUDManagerComponent hudManager = GetGame().GetHUDManager();
		if (!hudManager)
			return false;

		if (!m_wRestartFadeWidget)
		{
			m_wRestartFadeWidget = hudManager.CreateLayout(BLACK_OUT_IN_LAYOUT, EHudLayers.ALWAYS_TOP, 1000);
			if (!m_wRestartFadeWidget)
				return false;

			m_RestartFadeComponent = SCR_FadeUIComponent.Cast(m_wRestartFadeWidget.FindHandler(SCR_FadeUIComponent));
		}

		if (!m_RestartFadeComponent)
			return false;

		m_RestartFadeComponent.CancelFade(false);
		m_wRestartFadeWidget.SetOpacity(1);
		m_wRestartFadeWidget.SetVisible(true);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void TeleportPlayerToVehicle(int playerID, Vehicle car)
	{
		SCR_RacerData data = GetRacerData(playerID);
		if (!data)
		{
			Print(string.Format("SCR_RaceEntity::TeleportPlayerToVehicle: no SCR_RacerData for playerID %1", playerID), LogLevel.ERROR);
			return;
		}

		IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
		if (!player)
		{
			Print(string.Format("SCR_RaceEntity::TeleportPlayerToVehicle: GetPlayerControlledEntity(%1) returned null", playerID), LogLevel.ERROR);
			return;
		}

		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(player);
		if (!character)
		{
			Print(string.Format("SCR_RaceEntity::TeleportPlayerToVehicle: controlled entity '%1' is not a SCR_ChimeraCharacter", player.GetName()), LogLevel.ERROR);
			return;
		}

		SCR_CompartmentAccessComponent compartmentComponent = SCR_CompartmentAccessComponent.Cast(character.GetCompartmentAccessComponent());
		if (!compartmentComponent)
		{
			Print("SCR_RaceEntity::TeleportPlayerToVehicle: character has no SCR_CompartmentAccessComponent", LogLevel.ERROR);
			return;
		}

		if (!car)
		{
			Print("SCR_RaceEntity::TeleportPlayerToVehicle: car is null", LogLevel.ERROR);
			return;
		}

		bool gotIn = compartmentComponent.GetInVehicle(car, car.GetPilotCompartmentSlot(), true, -1, ECloseDoorAfterActions.INVALID, true);
		Print(string.Format("SCR_RaceEntity::TeleportPlayerToVehicle: GetInVehicle returned %1", gotIn), LogLevel.WARNING);
		data.SetVehicle(car);
	}

	//------------------------------------------------------------------------------------------------
	SCR_RacerData GetRacerData(int playerID)
	{
		foreach (SCR_RacerData data : m_aRacers)
		{
			if (data.GetPlayerID() == playerID)
				return data;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_RaceCheckpoint FindCheckpointByID(int id)
	{
		foreach (SCR_RaceCheckpoint checkpoint : m_aRaceCheckpoints)
		{
			if (checkpoint.GetCheckpointID() == id)
				return checkpoint;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	SCR_RaceCheckpoint GetStartCheckpoint()
	{
		if (m_StartCheckpoint)
			return m_StartCheckpoint;
		
		foreach (SCR_RaceCheckpoint checkpoint : m_aRaceCheckpoints)
		{
			if (checkpoint.GetCheckpointID() == RACE_CHECKPOINT_START_ID)
			{
				m_StartCheckpoint = checkpoint;
				return checkpoint;
			}
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	SCR_RaceCheckpoint GetEndCheckpoint()
	{
		foreach (SCR_RaceCheckpoint checkpoint : m_aRaceCheckpoints)
		{
			if (checkpoint.GetCheckpointID() == RACE_CHECKPOINT_END_ID)
				return checkpoint;
		}

		if (IsCircuit())
			return GetStartCheckpoint();
		else
			return null;
	}

	//------------------------------------------------------------------------------------------------
	SCR_RaceCheckpoint FindNextCheckpoint(SCR_RaceCheckpoint currentCheckpoint)
	{
		int currentCheckpointID = currentCheckpoint.GetCheckpointID();
		int nextCheckpointID = RACE_CHECKPOINT_END_ID;

		//if the current checkpoint is end, there is no next checkpoint
		if (currentCheckpointID == nextCheckpointID)
			return null;

		foreach (SCR_RaceCheckpoint checkpoint : m_aRaceCheckpoints)
		{
			int tempCheckpointID = checkpoint.GetCheckpointID();

			if (tempCheckpointID < nextCheckpointID && tempCheckpointID > currentCheckpointID)
				nextCheckpointID = tempCheckpointID;
		}

		//if the race is circuit and no checkpoint with higher ID was found, we go back to start
		if (nextCheckpointID == RACE_CHECKPOINT_END_ID && IsCircuit())
			nextCheckpointID = RACE_CHECKPOINT_START_ID;

		return FindCheckpointByID(nextCheckpointID);
	}

	//------------------------------------------------------------------------------------------------
	PointInfo GetStartPosition()
	{
		return m_raceStartingPoint;
	}

	//------------------------------------------------------------------------------------------------
	SCR_CustomizationCameraPointInfo GetRaceCamPosition()
	{
		if (m_raceCamPosition)
			m_raceCamPosition.Init(this);

		return m_raceCamPosition;
	}

	//------------------------------------------------------------------------------------------------
	int GetRaceCheckpointCount()
	{
		return m_iRaceCheckpointCount;
	}

	//------------------------------------------------------------------------------------------------
	bool IsRacePrepared()
	{
		return m_bIsRacePrepared;
	}

	//------------------------------------------------------------------------------------------------
	bool IsRaceRunning()
	{
		return m_bIsRaceRunning;
	}

	//------------------------------------------------------------------------------------------------
	void SetRaceRunning(bool running)
	{
		m_bIsRaceRunning = running;
	}

	//------------------------------------------------------------------------------------------------
	bool IsCircuit()
	{
		if (m_iCircularity == -1)
			m_iCircularity = FindCheckpointByID(SCR_RaceEntity.RACE_CHECKPOINT_START_ID) && !FindCheckpointByID(SCR_RaceEntity.RACE_CHECKPOINT_END_ID);
		return m_iCircularity > 0;
	}

	//------------------------------------------------------------------------------------------------
	int GetMaxLaps()
	{
		return m_iLapCount;
	}

#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	override void _WB_AfterWorldUpdate(float timeSlice)
	{
		DrawDebug();
	}

	//------------------------------------------------------------------------------------------------
	protected void DrawDebug()
	{
		// do not draw debug if child of SCR_RaceEntity AND grandchild of SCR_RaceTrackGeneratorEntity;
		// SCR_RaceTrackGeneratorEntity takes care of its own debug shapes
		IEntity parent = GetParent();
		if (!parent || !parent.Type() || !parent.Type().IsInherited(SCR_RaceTrackGeneratorEntity))
			Shape.CreateSphere(0x7DFFFFCC, ShapeFlags.ONCE | ShapeFlags.TRANSP | ShapeFlags.WIREFRAME | ShapeFlags.NOCULL, GetOrigin(), 2);
	}
#endif // WORKBENCH

	//------------------------------------------------------------------------------------------------
	//! Loads the track-authored default ghost from m_sDefaultGhostFile. Pure content resource -
	//! never touches PersistentState/SaveGameManager, so it's identical for every player/session.
	protected SCR_GhostSave LoadDefaultGhost()
	{
		if (m_sDefaultGhostFile.IsEmpty())
			return null;

		Resource res = Resource.Load(m_sDefaultGhostFile);
		if (!res.IsValid())
			return null;

		BaseContainer container = res.GetResource().ToBaseContainer();
		if (!container)
			return null;

		SCR_GhostSave ghost = new SCR_GhostSave();
		container.Get("m_sName", ghost.m_sName);
		container.Get("m_sVehiclePrefab", ghost.m_sVehiclePrefab);
		if (!container.Get("m_aPositions", ghost.m_aPositions))
			return null;

		ghost.RebuildTransforms();
		return ghost;
	}

	//------------------------------------------------------------------------------------------------
	//! Picks the ghost to race against: the player's best run or the track default, whichever is faster.
	//! Ghosts are single-player only, in MP nothing is picked.
	//! \param[out] ghostTime finish time of the picked ghost, -1 if none
	//! \param[out] recordRace player's saved best run, null if none
	//! \return picked ghost, null if none
	protected SCR_GhostSave SelectGhost(out float ghostTime, out SCR_RaceSave recordRace)
	{
		ghostTime = -1;
		recordRace = null;

		if (RplSession.Mode() != RplMode.None)
			return null;

		SCR_GhostSave defaultGhost = LoadDefaultGhost();
		SCR_GhostSave playerGhost;
		float playerTime;

		PersistenceSystem persistence = PersistenceSystem.GetInstance();
		SCR_PersistentRaceData raceData;
		if (persistence)
			raceData = SCR_PersistentRaceData.Cast(persistence.GetPersistentState(SCR_PersistentRaceData));

		if (raceData)
		{
			array<SCR_RaceSave> races = raceData.GetRaces();
			//we store just one ghost per race so we can always get first
			if (races && !races.IsEmpty())
			{
				recordRace = races.Get(0);
				if (recordRace && recordRace.m_aGhost && !recordRace.m_aGhost.IsEmpty())
				{
					playerGhost = recordRace.m_aGhost.Get(0);
					if (playerGhost)
						playerGhost.RebuildTransforms();

					playerTime = recordRace.m_fGoalReachedTime;
				}
			}
		}

		//default ghost carries no separate finish time field - its last recorded sample is its finish time
		float defaultTime = -1;
		if (defaultGhost && defaultGhost.m_aPositions && !defaultGhost.m_aPositions.IsEmpty())
			defaultTime = defaultGhost.m_aPositions[defaultGhost.m_aPositions.Count() - 1].m_fTime;

		//use whichever is faster; player's best only exists once a run is recorded, default only once authored
		if (playerGhost && (!defaultGhost || playerTime <= defaultTime))
		{
			ghostTime = playerTime;
			return playerGhost;
		}

		ghostTime = defaultTime;
		return defaultGhost;
	}

	//------------------------------------------------------------------------------------------------
	//! Ghosts are single-player only. In MP this is a no-op - m_GhostData stays null and no ghost
	//! vehicle is spawned (see SCR_RaceStartCheckpoint, which also gates SpawnGhost() on RplSession.Mode()).
	void StartPlayingGhost(SCR_RacerData racerData)
	{
		if (RplSession.Mode() != RplMode.None)
			return;

		float ghostTime;
		m_GhostData = SelectGhost(ghostTime, m_RecordRace);

//		foreach (SCR_TimedPositionSave point : m_GhostData.m_aPositions)
//		{
//			m_aGhostDebugShapes.Insert(Shape.CreateSphere(ARGB(125, 255, 255, 255), ShapeFlags.WIREFRAME, point.m_aTransform[3], 0.1));
//		}
	}

	//------------------------------------------------------------------------------------------------
	//! \return finish time of the ghost the player races against (their best run or the track default, whichever is faster), -1 if none (MP or no ghost)
	float GetGhostFinishTime()
	{
		float ghostTime;
		SCR_RaceSave recordRace;
		SelectGhost(ghostTime, recordRace);
		return ghostTime;
	}

	//------------------------------------------------------------------------------------------------
	//! Snapshot of the medal times for the run that is starting, so a new best saved at its finish doesn't change the medals it is judged by.
	//! Call once at the start of a run, regardless of whether the ghost is shown.
	void LockMedalTimes()
	{
		m_fLockedGhostFinishTime = GetGhostFinishTime();
	}

	//------------------------------------------------------------------------------------------------
	//! \return number of medals (m_aRaceRecords), ordered best first
	int GetMedalCount()
	{
		if (!m_aRaceRecords)
			return 0;

		return m_aRaceRecords.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! \return localized medal name
	string GetMedalText(int index)
	{
		if (!m_aRaceRecords || !m_aRaceRecords.IsIndexValid(index))
			return string.Empty;

		return m_aRaceRecords[index].GetWinningText();
	}

	//------------------------------------------------------------------------------------------------
	//! Medal time based on a ghost finish time: the first medal (gold) equals the ghost time, the others keep their
	//! authored offset from gold in m_aRaceRecords. Without a ghost (ghostFinishTime < 0) the authored times are used.
	float GetMedalTimeForGhost(int index, float ghostFinishTime)
	{
		if (!m_aRaceRecords || !m_aRaceRecords.IsIndexValid(index))
			return 0;

		if (ghostFinishTime < 0)
			return m_aRaceRecords[index].GetFinalTime();

		return ghostFinishTime + m_aRaceRecords[index].GetFinalTime() - m_aRaceRecords[0].GetFinalTime();
	}

	//------------------------------------------------------------------------------------------------
	//! \return medal time the last started run is judged by (locked by LockMedalTimes())
	float GetLockedMedalTime(int index)
	{
		return GetMedalTimeForGhost(index, m_fLockedGhostFinishTime);
	}

	//------------------------------------------------------------------------------------------------
	string GetRaceName()
	{
		return m_sRaceName;
	}

	//------------------------------------------------------------------------------------------------
	string GetRaceDescription()
	{
		return m_sRaceDescription;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetRaceImage()
	{
		return m_sRaceImage;
	}

	//------------------------------------------------------------------------------------------------
	void SCR_RaceEntity(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT | EntityEvent.FRAME);
		Activate();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Tries to find a race spline - a root parent of the race entity and calculates data points in world coords.
	//! Returns true, if Race has a spline.
	bool PrepareRaceSpline(out float aproxTrackLength)
	{
		m_RaceTrack = {};
		aproxTrackLength = 0;
		ShapeEntity raceTrack = ShapeEntity.Cast(GetRootParent());
		if (!raceTrack)
		{
			Print("Cannot find Race Track spline!", LogLevel.WARNING);
			return false;
		}
		
		raceTrack.GetPointsPositions(m_RaceTrack);
		int count = m_RaceTrack.Count();
		if (count < 2)
		{
			Print("Track spline is not set properly!", LogLevel.ERROR);
			m_RaceTrack.Clear();
			return false;
		} 
		
		vector splineOrigin = raceTrack.GetOrigin();
		m_RaceTrack[0] = splineOrigin + m_RaceTrack[0];
		
		for (int i = 1; i < count; i++)
		{
			m_RaceTrack[i] = splineOrigin + m_RaceTrack[i];
			aproxTrackLength += (m_RaceTrack[i] - m_RaceTrack[i-1]).Length(); 
		}
		
		if (IsCircuit())
			aproxTrackLength += (m_RaceTrack[0] - m_RaceTrack[count - 1]).Length();
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	// Returns array of spline points that are an approximation of the racetrack
	array<vector> GetRaceSpline()
	{
		if (m_RaceTrack.IsEmpty())
			Print("Track has not a valid spline defined.", LogLevel.WARNING);    
		
		array<vector> copy = {};
		copy.Copy(m_RaceTrack);
		return copy;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Projects 'point' onto the polyline 'spline' and returns the arc-length
	//! (cumulative distance from spline[0]) of that projection.
	//! \param[in] spline world-space track points, minimum 2
	//! \param[in] point world position to project (e.g. entity origin)
	//! \param[in] isCircle for closed splines (where spline[0] is connected to spline[count - 1]
	//! \param[in] isNextLoop we are not on the beginning of the spline, where distance to spline[0] matters on the first segment
	//! \param[out] outPerpDistSq squared perpendicular distance to the spline
	//! \return distance along the spline, or -1 if spline has < 2 points
	protected float GetDistanceAlongSpline(notnull array<vector> spline, vector point, out float outPerpDistSq, bool isCircle = false, bool isNextLoop = false)
	{
		int count = spline.Count();
		if (count < 2)
		{
			outPerpDistSq = float.MAX;
			return -1;
		}
		
		float bestPerpSq  = float.MAX;
		float bestArcLen  = 0;
		float accumulated = 0; // arc length up to segmentStart
		
		vector segmentStart = spline[0];
		int lastIndex = count;
		if (!isCircle || !isNextLoop)
		{
			lastIndex = count - 1; 
		}
		
		for (int i = 1; i <= lastIndex; i++)
		{
			vector segmentEnd = spline[i % count]; // spline[0] for i==count
			vector seg        = segmentEnd - segmentStart;
			float  segLenSq   = seg.LengthSq();
			float  segLen     = seg.Length();
			
			float t = 0;
			if (segLenSq > 0.00001)
				t = Math.Clamp(vector.Dot(point - segmentStart, seg) / segLenSq, 0, 1);
			
			vector proj   = segmentStart + seg * t;
			float  perpSq = vector.DistanceSq(point, proj);
			
			if (perpSq < bestPerpSq)
			{
				bestPerpSq = perpSq;
				bestArcLen = accumulated + segLen * t;
			}
			
			accumulated += segLen;
			segmentStart = segmentEnd;
		}
		
		outPerpDistSq = bestPerpSq;
		return bestArcLen;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Projects every checkpoint onto m_RaceTrack once and caches its arc-length
	//! into SCR_RaceCheckpoint.m_fDistanceFromStart.
	//! MUST be called AFTER m_RaceTrack has been populated (world-space) and
	//! after m_aRaceCheckpoints is filled in EOnInit.
	//! \return true on success, false if the spline is not ready
	protected bool PrecomputeCheckpointDistances()
	{
		if (!m_RaceTrack || m_RaceTrack.Count() < 2)
			return false;
		
		foreach (SCR_RaceCheckpoint checkpoint : m_aRaceCheckpoints)
		{
			if (!checkpoint)
				continue;
			
			float perpSq;
			float dist = GetDistanceAlongSpline(m_RaceTrack, checkpoint.GetOrigin(), perpSq);
			checkpoint.SetDistanceFromStart(dist);
		}
		
		return true;
	}
}

void ScriptInvokerVehicleMethod(notnull Vehicle vehicle);
typedef func ScriptInvokerVehicleMethod;
typedef ScriptInvokerBase<ScriptInvokerVehicleMethod> ScriptInvokerVehicle;

[BaseContainerProps()]
class SCR_RaceRecord
{
	[Attribute()]
	float m_fFinalTime;

	[Attribute()]
	string m_sWinningText;

	//------------------------------------------------------------------------------------------------
	float GetFinalTime()
	{
		return m_fFinalTime;
	}

	//------------------------------------------------------------------------------------------------
	string GetWinningText()
	{
		return m_sWinningText;
	}
}
