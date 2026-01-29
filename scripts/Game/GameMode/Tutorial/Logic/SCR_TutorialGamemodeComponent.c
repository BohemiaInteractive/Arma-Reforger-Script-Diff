class SCR_TutorialGamemodeComponentClass : SCR_BaseGameModeComponentClass
{
}

class SCR_TutorialGamemodeComponent : SCR_BaseGameModeComponent
{	
	[Attribute("{14FDB620B5FF18BC}Configs/Tutorial/TutorialStagesConfig.conf", UIWidgets.ResourceNamePicker, "", "conf")]
	protected ResourceName m_sCoursesConfigResourceName;
	
	[Attribute("{86D830868F026D54}Prefabs/Vehicles/Wheeled/M151A2/M151A2_MERDC.et", UIWidgets.ResourceNamePicker, "", "conf")]
	protected ResourceName m_sPlayerVehicleResourceName;
	
	protected const int GARBAGE_DESPAWN_DISTANCE_SQ = 200*200;
	protected const int PUNISHMENT_TIMEOUT = 5000;
	protected const string FADEOUT_LAYOUT = "{265245C299401BF6}UI/layouts/Menus/ContentBrowser/DownloadManager/ScrollBackground.layout";
	protected const string TASK_COMPLETED_SUFFIX = "#AR-Tutorial_TaskCompletedSuffix";
	
	protected ChimeraCharacter m_Player;
	protected ref SCR_TutorialCoursesConfig m_CoursesConfig;
	protected ref array<ref SCR_TutorialStageInfo> m_aStageInfos = {};
	protected ref array<ref SCR_MapMarkerBase> m_aMarkers = {};
	protected int m_iActiveStage;
	protected SCR_BaseTutorialStage m_Stage;
	protected ImageWidget m_wFadeOut;
	protected bool m_bFirstStageSet;
	
	protected bool m_bIsMapOpen = false;
	protected bool m_bUsed3PV = false;
	
	protected bool m_fDelayedControlSchemeChangeRunning = false;
	protected bool m_bIsUsingGamepad;
	protected bool m_bProtectionWarning;
	protected SCR_TutorialCourse m_ActiveConfig;
	protected SCR_PlacingEditorComponent m_PlacingComponent;
	protected int m_iFreeRoamActivations;
	protected bool m_bStagesComplete = false;
	protected SCR_VoiceoverSystem m_VoiceoverSystem;
	protected IEntity m_FastTravelPosition;
	protected WidgetAnimationOpacity m_OngoingFadeAnimation;
	protected float m_fSavedTime;
	
	protected ref array <SCR_TutorialInstructorComponent> m_aInstructors = {};
	
	protected ref ScriptInvoker m_OnPlayerSpawned = new ScriptInvoker();
	protected ref ScriptInvoker m_OnEntitySpawned = new ScriptInvoker();
	protected ref ScriptInvoker m_OnFastTravelDone = new ScriptInvoker();
	
	protected bool m_bIsFastTraveling;
	protected bool m_bCourseBreaking;
	protected SCR_WaypointDisplay m_WaypointDisplay;

	protected SCR_ETutorialCourses m_eFinishedCourses;
	
	protected ref array <IEntity> m_aGarbage;
	protected ref array <SCR_VehicleDamageManagerComponent> m_aDamagedVehicles;
	protected ref array <SCR_NarrativeComponent> m_aNarratedCharacters;
	
	protected SCR_ETutorialBreakType m_iBreakType;
	protected Widget m_wPunishmentScreen;
	protected bool m_bOutroBreak;
	
	protected IEntity m_AreaRestrictionCenter;
	
	protected float m_fSavedViewDistance;
	
	// Sound fade
	protected float m_fSoundInterpolationSpeed;
	protected float m_fSoundVolume;
	protected bool m_bApplySoundFade;
	
	
	#ifdef ENABLE_DIAG
		bool m_bIgnoreCourseRequirements;
		bool m_bSkipIntro;
		protected bool m_bDrawAreaRestrictions;
	#endif
	
	//------------------------------------------------------------------------------------------------
	void BlockBuildingModeAccess(notnull IEntity providerEntity, bool block)
	{
		ActionsManagerComponent actionsMan = ActionsManagerComponent.Cast(providerEntity.FindComponent(ActionsManagerComponent));
		if (!actionsMan)
			return;
		
		array <BaseUserAction> outActions = {};
		actionsMan.GetActionsList(outActions);
		if (!outActions || outActions.IsEmpty())
			return;
		
		SCR_TutorialBuildingStartUserAction buildingAction;
		foreach (BaseUserAction userAction : outActions)
		{
			buildingAction = SCR_TutorialBuildingStartUserAction.Cast(userAction);
			if (!buildingAction)
				continue;
			
			buildingAction.SetCanShow(!block);
			
			return;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void EnableRefunding(IEntity ent, bool enable)
	{
		if (!ent)
			return;
		
		BaseInventoryStorageComponent storageComp = BaseInventoryStorageComponent.Cast(ent.FindComponent(BaseInventoryStorageComponent));
		if (!storageComp)
			return;
		
		SCR_ItemAttributeCollection attributeCollection = SCR_ItemAttributeCollection.Cast(storageComp.GetAttributes());
		if (!attributeCollection)
			return;
		
		attributeCollection.SetRefundable(enable);
	}
	
	//------------------------------------------------------------------------------------------------
	void SaveViewDistance(float distance)
	{
		m_fSavedViewDistance = distance;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetSavedViewDistance()
	{
		return m_fSavedViewDistance;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetupTargets(string parentTargetName, out array <SCR_FiringRangeTarget> targets = null, ETargetState setupState = -1, bool setupAutoReset = true)
	{
		SCR_FiringRangeTarget target = SCR_FiringRangeTarget.Cast(GetGame().GetWorld().FindEntityByName(parentTargetName));
		if (!target)
			return;
		
		target.SetAutoResetTarget(setupAutoReset);
		target.SetState(setupState);
		
		if (targets)
			targets.Insert(target);
		
		target = SCR_FiringRangeTarget.Cast(target.GetChildren());
		
		while (target)
		{
			if (targets)
				targets.Insert(target);
			
			target.SetAutoResetTarget(setupAutoReset);
			target.SetState(setupState);
			
			target = SCR_FiringRangeTarget.Cast(target.GetSibling());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void RepairWorldVehicles()
	{
		if (!m_aDamagedVehicles || m_aDamagedVehicles.IsEmpty())
			return;
		
		string name;
		foreach (SCR_VehicleDamageManagerComponent damageManager : m_aDamagedVehicles)
		{
			
			if (!damageManager)
				continue;
			
			name = damageManager.GetOwner().GetName();
			if (name == "RespawnAmbulance" || name == "UH1COURSE" || name == "SmallJeep")
				continue;
			
			damageManager.FullHeal();
		}
		
		m_aDamagedVehicles = null;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnVehicleDamaged(SCR_VehicleDamageManagerComponent damageManager)
	{
		//Vehicle was repaired.
		if (damageManager.GetState() == EDamageState.UNDAMAGED)
			return;
		
		IEntity owner = damageManager.GetOwner();
		if (owner.GetName() == "TARGET_BTR")
			return;
		
		if (!m_aDamagedVehicles)
			m_aDamagedVehicles = {};
			
		m_aDamagedVehicles.Insert(damageManager);
		
		Instigator instigator = damageManager.GetInstigator();
		if (!instigator)
			return;
		
		IEntity instigatorEntity = instigator.GetInstigatorEntity();
		if (instigatorEntity == m_Player && m_Player.GetParent() != owner)
			RequestBreakCourse(SCR_ETutorialBreakType.PLAYER_GRIEFING_VEHICLE);
	}
	
	//------------------------------------------------------------------------------------------------
	void HandleGarbage(bool force = false)
	{
		if (!m_aGarbage)
			return;
		
		vector distance;
		for (int i = m_aGarbage.Count()-1; i>=0; i--)
		{
			if (m_aGarbage[i] == null)
			{
				m_aGarbage.Remove(i);
				continue;
			}
			
			if (!force && vector.DistanceSq(m_Player.GetOrigin(), m_aGarbage[i].GetOrigin()) < GARBAGE_DESPAWN_DISTANCE_SQ)
				continue;
			
			SCR_EntityHelper.DeleteEntityAndChildren(m_aGarbage[i]);
			m_aGarbage.Remove(i);
		}
		
		if (m_aGarbage.IsEmpty())
			m_aGarbage = null;
	}
	
	//------------------------------------------------------------------------------------------------
	void RemoveFromGarbage(IEntity ent)
	{
		if (!m_aGarbage)
			return;
		
		m_aGarbage.RemoveItem(ent);
		if (m_aGarbage.IsEmpty())
			m_aGarbage = null;
	}
	
	//------------------------------------------------------------------------------------------------
	void InsertIntoGarbage(string entityName)
	{
		IEntity ent = GetGame().GetWorld().FindEntityByName(entityName);
		
		if (ent)
			InsertIntoGarbage(ent);
	}
	
	//------------------------------------------------------------------------------------------------
	void InsertIntoGarbage(IEntity ent)
	{
		if (!m_aGarbage)
			m_aGarbage = {};
		
		m_aGarbage.Insert(ent);
	}
	
	//------------------------------------------------------------------------------------------------
	void ChangeVehicleLockState(notnull IEntity vehicle, bool lock)
	{
		SCR_BaseLockComponent lockComp = SCR_BaseLockComponent.Cast(vehicle.FindComponent(SCR_BaseLockComponent));
		if (!lockComp)
			return;
		
		lockComp.SetLocked(lock);
	}
	
	//------------------------------------------------------------------------------------------------
	int GetRemainingAmmo()
	{
		BaseWeaponManagerComponent weaponManager = BaseWeaponManagerComponent.Cast(m_Player.FindComponent(BaseWeaponManagerComponent));
		if (!weaponManager)
			return 0;
		
		array<IEntity> weapons = {};
		weaponManager.GetWeaponsList(weapons);
		
		if (!weapons || weapons.IsEmpty())
			return 0;
		
		int ammoCount;
		foreach (IEntity weapon : weapons)
		{
			ammoCount += GetAmmoCountForWeapon(weapon);
		}
		
		return ammoCount;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetAmmoCountForWeapon(IEntity weapon)
	{
		if (!weapon)
			return 0;
		
		BaseWeaponComponent	weaponComp = BaseWeaponComponent.Cast(weapon.FindComponent(BaseWeaponComponent));
		if (!weaponComp)
			return 0;
		
		int ammoCount = 0;
		BaseMuzzleComponent muzzle = weaponComp.GetCurrentMuzzle();
		if (muzzle)
			ammoCount = muzzle.GetAmmoCount();
		
		BaseMagazineWell magWell = muzzle.GetMagazineWell();
		if (magWell)
		{
			const typename magazineType = magWell.Type();
			
			array<IEntity> foundItems = {};
			GetPlayerInventory().FindItemsWithComponents(foundItems, {BaseMagazineComponent}, EStoragePurpose.PURPOSE_ANY);
			foreach (IEntity item : foundItems)
			{
				auto magazine = BaseMagazineComponent.Cast(item.FindComponent(BaseMagazineComponent));
				if (!magazine)
					continue;

				auto magazineWell = magazine.GetMagazineWell();
				if (!magazineWell)
					continue;

				if (magazineWell.Type() == magazineType)
					ammoCount += magazine.GetAmmoCount();
			}
		}
		
		return ammoCount;
	}
	
	//------------------------------------------------------------------------------------------------
	void LoadProgress(SCR_ETutorialCourses completedCourses)
	{
		m_eFinishedCourses = completedCourses;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetSavedTime(float time)
	{
		m_fSavedTime = time;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetSavedTime()
	{
		return m_fSavedTime;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_ETutorialCourses GetFinishedCourses()
	{
		return m_eFinishedCourses;
	}
	
	//------------------------------------------------------------------------------------------------
	void MoveCharacterIntoVehicle(notnull ChimeraCharacter character, notnull Vehicle vehicle, ECompartmentType compartmentType, int compartmentIndex = 0)
	{
		CompartmentAccessComponent compartmentAccess = character.GetCompartmentAccessComponent();
		if (!compartmentAccess)
			return;
		
		SCR_BaseCompartmentManagerComponent compartmentManager = SCR_BaseCompartmentManagerComponent.Cast(vehicle.FindComponent(SCR_BaseCompartmentManagerComponent));
		if (!compartmentManager)
			return;
		
		array<BaseCompartmentSlot> outCompartments = {};
		compartmentManager.GetCompartmentsOfType(outCompartments, compartmentType);
			
		if (outCompartments)
			compartmentAccess.GetInVehicle(vehicle, outCompartments[compartmentIndex], true, 0, ECloseDoorAfterActions.CLOSE_DOOR, false);
		
		SCR_NarrativeComponent narrativeComponent = SCR_NarrativeComponent.Cast(character.FindComponent(SCR_NarrativeComponent));
		
		if (!SCR_NarrativeComponent)
			return;
		
		narrativeComponent.Initialize(character);
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnPlayerSpawned()
	{
		return m_OnPlayerSpawned;
	}
	
	//------------------------------------------------------------------------------------------------
	void HealPlayer()
	{
		if (!m_Player)
			return;
		
		SCR_CharacterDamageManagerComponent damageMan = SCR_CharacterDamageManagerComponent.Cast(m_Player.FindComponent(SCR_CharacterDamageManagerComponent));
		if (!damageMan)
			return;
		
		damageMan.FullHeal();
	}

	//------------------------------------------------------------------------------------------------
	void RequestBreakCourse(SCR_ETutorialBreakType breakType)
	{
		if (m_bCourseBreaking)
			return;
		
		m_bCourseBreaking = true;
		
		SCR_ETutorialCourses course = m_ActiveConfig.GetCourseType();
		SCR_AnalyticsApplication.GetInstance().TutorialCourseEnds(course, breakType);

		m_bOutroBreak = course == SCR_ETutorialCourses.OUTRO;
		
		GetGame().GetMenuManager().CloseAllMenus();
		
		m_iBreakType = breakType;
		
		// User or Course induced break, resulting into Fast travel
		if (breakType == SCR_ETutorialBreakType.FORCED)
		{
			m_OnFastTravelDone.Insert(FinalizeBreak);
			FastTravel(m_ActiveConfig.GetFastTravelPosition());
			return;
		}
		else
		{
			ChimeraWorld world = GetGame().GetWorld();
			
			if (world)
				world.GetMusicManager().Play("SOUND_END_BAD");
		}
		
		// Player death of griefing, resulting into Respawn
		EnableControls(false);
		
		if (breakType != SCR_ETutorialBreakType.PLAYER_DEATH)
			CreateWarningOverlay(breakType == SCR_ETutorialBreakType.PLAYER_GRIEFING_NPC);
		
		Fadeout(true).GetOnCompleted().Insert(HandleRespawnBreak);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void HandleRespawnBreak()
	{
		if (m_OngoingFadeAnimation)
			m_OngoingFadeAnimation.GetOnCompleted().Remove(HandleRespawnBreak);
		
		m_OnPlayerSpawned.Insert(FinalizeBreak);
		
		// If player died, respawn him as soon as possible
		if (m_iBreakType == SCR_ETutorialBreakType.PLAYER_DEATH)
		{
			SpawnPlayer("Respawn_Ambulance");
			return;
		}
		
		GetGame().GetCallqueue().CallLater(SpawnPlayer, PUNISHMENT_TIMEOUT, false, "FT_HUB");
	}
	
	//------------------------------------------------------------------------------------------------
	void FinalizeBreak()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (editorManager && editorManager.IsOpened())
			editorManager.Toggle();
		
		if (m_aNarratedCharacters)
		foreach (SCR_NarrativeComponent narratedCharacter : m_aNarratedCharacters)
		{
			narratedCharacter.Initialize(narratedCharacter.GetOwner());
		}
		
		SetCourseConfig(SCR_ETutorialCourses.FREE_ROAM);
		HandleGarbage(true);
		
		if (m_OnFastTravelDone)
			m_OnFastTravelDone.Remove(FinalizeBreak);
		
		if (m_OnPlayerSpawned)
			m_OnPlayerSpawned.Remove(FinalizeBreak);
		
		if (m_OngoingFadeAnimation)
			m_OngoingFadeAnimation.GetOnCompleted().Remove(FinalizeBreak);
		
		if (m_iBreakType == SCR_ETutorialBreakType.PLAYER_DEATH)
		{
			if (m_Stage)
				GetGame().GetCallqueue().CallLater(	m_Stage.PlayNarrativeCharacterStage, 2000, false, "Ambulance_Doctor", 1);
		}
		
		m_bCourseBreaking = false;
		
		if (m_wPunishmentScreen)
			m_wPunishmentScreen.RemoveFromHierarchy();
		
		Fadeout(false);
		
		if (m_bOutroBreak)
		{
			SetCourseConfig(SCR_ETutorialCourses.OUTRO);
			m_bOutroBreak = false;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsCourseBreaking()
	{
		return m_bCourseBreaking;
	}
	
	//------------------------------------------------------------------------------------------------
	//TODO: Look into gamemode, if it could be fixed
	void EnableControls(bool enable)
	{
		if (!m_Player)
			return;
		
		CharacterControllerComponent characterController = m_Player.GetCharacterController();
		if (!characterController)
			return;

		characterController.SetDisableViewControls(!enable);
		characterController.SetDisableWeaponControls(!enable);
		characterController.SetDisableMovementControls(!enable)
	}
	
	//------------------------------------------------------------------------------------------------
	Widget CreateWarningOverlay(bool killedNPC)
	{
		SCR_HUDManagerComponent hudManager = GetGame().GetHUDManager();
		if (!hudManager)
			return null;
		
		m_wPunishmentScreen = hudManager.CreateLayout("{33548E2300BFF61C}UI/layouts/HUD/GameOver/Tutorial_Punishment.layout", EHudLayers.ALWAYS_TOP, 0);
		if (!m_wPunishmentScreen)
			return null;
		
		TextWidget text = TextWidget.Cast(m_wPunishmentScreen.FindAnyWidget("GameOver_Description"));
		if (text)
		{
			if (killedNPC)
				text.SetText("#AR-Tutorial_Hint_CourseTerminatedByFriendlyFire");
			else
				text.SetText("#AR-Tutorial_Hint_CourseTerminatedByMisbehaving");
		}
		
		return m_wPunishmentScreen;
	}
	
	//------------------------------------------------------------------------------------------------
	bool CanBreakCourse()
	{
		if (!m_ActiveConfig || m_bCourseBreaking)
			return false;
		
		SCR_ETutorialArlandStageMasters courseType = m_ActiveConfig.GetCourseType();
		
		return courseType != SCR_ETutorialCourses.FREE_ROAM && courseType != SCR_ETutorialCourses.INTRO && courseType != SCR_ETutorialCourses.OUTRO;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_BaseTutorialStage GetCurrentStage()
	{
		return m_Stage;
	}
	
	//------------------------------------------------------------------------------------------------
	WidgetAnimationOpacity GetOngoingFadeAnimation()
	{
		return m_OngoingFadeAnimation;
	}
	
	//------------------------------------------------------------------------------------------------
	void PlayNarrativeCharacterStageDelayed(string characterName, int stage)
	{
		IEntity entity = GetGame().GetWorld().FindEntityByName(characterName);
		
		if (!entity)
			return;
	
		SCR_NarrativeComponent narrative = SCR_NarrativeComponent.Cast(entity.FindComponent(SCR_NarrativeComponent));
		
		if (!narrative)
			return;
		
		if (!m_aNarratedCharacters)
			m_aNarratedCharacters = {};
		
		m_aNarratedCharacters.Insert(narrative);
		
		narrative.PlayAnimation(true, "CMD_NarrativeStage", stage);
	}
	
	//------------------------------------------------------------------------------------------------
	void RegisterMarker(notnull SCR_MapMarkerBase marker)
	{
		m_aMarkers.Insert(marker);
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_TutorialGamemodeComponent GetInstance()
	{
		BaseGameMode gamemode = GetGame().GetGameMode();
		if (!gamemode)
			return null;
		
		return SCR_TutorialGamemodeComponent.Cast(gamemode.FindComponent(SCR_TutorialGamemodeComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnFastTravelDone()
	{
		return m_OnFastTravelDone;
	}
	
	//------------------------------------------------------------------------------------------------
	//TODO> Perhaps simply going through all entities in inventory would be better
	IEntity FindPrefabInPlayerInventory(ResourceName prefabName)
	{
		SCR_InventoryStorageManagerComponent inventory = GetPlayerInventory();
		if (!inventory)
			return null;
		
		SCR_TutorialInventoryPrefabSearchPredicate predicate = new SCR_TutorialInventoryPrefabSearchPredicate;
		predicate.m_sSeachPrefab = prefabName;
		
		IEntity item = inventory.FindItem(predicate);
		if (item)
			return item;
		
		SCR_CharacterInventoryStorageComponent storageComp = SCR_CharacterInventoryStorageComponent.Cast(m_Player.FindComponent(SCR_CharacterInventoryStorageComponent));
		if (!storageComp)
			return null;
		
		item = storageComp.GetCurrentItem();
		if (item && item.GetPrefabData().GetPrefabName() == prefabName)
			return item;
		
		array <IEntity> entities = storageComp.GetQuickSlotEntitiesOnly();
		foreach (IEntity ent : entities)
		{
			if (!ent)
				continue;
			
			if (ent.GetPrefabData().GetPrefabName() == prefabName)
				return ent;
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsEntityInPlayerInventory(string entityName)
	{
		IEntity ent = GetGame().GetWorld().FindEntityByName(entityName);
		if (!ent)
			return false;
		
		return IsEntityInPlayerInventory(ent);
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsEntityInPlayerInventory(IEntity entity)
	{
		SCR_InventoryStorageManagerComponent inventory = GetPlayerInventory();
		if (!inventory)
			return false;
		
		if (inventory.Contains(entity))
			return true;
		
		SCR_TutorialInventoryItemSearchPredicate predicate = new SCR_TutorialInventoryItemSearchPredicate;
		predicate.m_SearchEntity = entity;
		
		IEntity item = inventory.FindItem(predicate);
		if (item)
			return true;
		
		SCR_CharacterInventoryStorageComponent storageComp = SCR_CharacterInventoryStorageComponent.Cast(m_Player.FindComponent(SCR_CharacterInventoryStorageComponent));
		if (!storageComp)
			return false;
		
		item = storageComp.GetCurrentItem();
		if (item == entity)
			return true;
		
		array <IEntity> entities = storageComp.GetQuickSlotEntitiesOnly();
		foreach (IEntity ent : entities)
		{
			if (ent == entity)
				return true;
		}

		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_InventoryStorageManagerComponent GetPlayerInventory()
	{
		if (!m_Player)
			return null;
		
		return SCR_InventoryStorageManagerComponent.Cast(m_Player.FindComponent(SCR_InventoryStorageManagerComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	void HandlePlayerGroup()
	{
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!playerController)
			return;
		
		SCR_PlayerControllerGroupComponent playerGroupComp = SCR_PlayerControllerGroupComponent.Cast(playerController.FindComponent(SCR_PlayerControllerGroupComponent));
		if (!playerGroupComp)
			return;
		
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;
		
		SCR_FactionAffiliationComponent factionComp = SCR_FactionAffiliationComponent.Cast(playerController.FindComponent(SCR_FactionAffiliationComponent));
		
		SCR_AIGroup group = groupsManager.FindGroup(playerGroupComp.GetGroupID());
		//create group, if there is none
		if (!group)
			group = SCR_GroupsManagerComponent.GetInstance().CreateNewPlayableGroup(factionComp.GetAffiliatedFaction());
		
		playerGroupComp.RequestJoinGroup(group.GetGroupID());
		groupsManager.SetGroupLeader(group.GetGroupID(), 1);
		
		SCR_GadgetManagerComponent gMan = SCR_GadgetManagerComponent.Cast(m_Player.FindComponent(SCR_GadgetManagerComponent));
		group.OnControllableEntitySpawned(m_Player, gMan);
		
		//SCR_GadgetManagerComponent.GetOnGadgetInitDoneInvoker().Insert(OnControllableEntitySpawned);
		SCR_CommandingManagerComponent.GetInstance().ResetSlaveGroupWaypoints(1, m_Player);
	}
	
	//------------------------------------------------------------------------------------------------
	void EnableArsenal(string entityName, bool enable)
	{
		IEntity entity = GetGame().GetWorld().FindEntityByName(entityName);
		if (!entity)
			return;
		
		SCR_ArsenalComponent arsenalComp = SCR_ArsenalComponent.FindArsenalComponent(entity);
		if (arsenalComp)
			arsenalComp.SetArsenalEnabled(enable);	
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_TutorialCoursesConfig GetCoursesConfig()
	{
		return m_CoursesConfig;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_Task GetCourseTask(SCR_ETutorialCourses course)
	{
		SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
		if (!taskSystem)
			return null;
		
		array <SCR_Task> tasks = {};
		taskSystem.GetTasks(tasks);
		
		string taskId;
		
		foreach (SCR_Task task : tasks)
		{
			if (!task)
				continue;
			
			taskId = task.GetTaskID();
			if (!SCR_StringHelper.IsEmptyOrWhiteSpace(taskId) && taskId.ToInt() == course)
				return task;
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateCourseTask(SCR_ETutorialCourses courseType)
	{
		if (!m_CoursesConfig)
			return;
		
		if (GetCourseTask(courseType))
			return;
		
		SCR_TutorialCourse course = m_CoursesConfig.GetCourse(courseType);
		if (!course)
			return;
		
		SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
		if (!taskSystem)
			return;
		
		vector position;
		if (courseType == SCR_ETutorialCourses.OUTRO)
		{
			IEntity posEnt = GetGame().GetWorld().FindEntityByName("SpawnPos_Captain_Table");
			if (posEnt)
				position = posEnt.GetOrigin();
		}
		else
		{
			foreach (SCR_TutorialInstructorComponent instructor : m_aInstructors)
			{
				if (!instructor || instructor.GetCourseType() != courseType)
					continue;
			
				position = instructor.GetOwner().GetOrigin();
				break;
			}
		}
		
		if (position == vector.Zero)
		{
			IEntity positionEntity = course.GetFastTravelPosition();
			if (positionEntity)
				position = positionEntity.GetOrigin();
		}
		
		SCR_Task task = taskSystem.CreateTask("{1D0F815858EE24AD}Prefabs/Tasks/BaseTask.et", course.GetCourseType().ToString(), course.GetTaskTitle(), course.GetTaskDescription(), position);
		if (!task)
			return;
		
		task.SetTaskIconPath("{D43428C02A644C1B}UI/Imagesets/Tutorial-Courses/Tutorial-Courses.imageset");
		task.SetTaskIconSetName(course.GetTaskIcon());
		task.SetTaskVisibility(SCR_ETaskVisibility.NONE);
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsFastTraveling()
	{
		return m_bIsFastTraveling;
	}
	
	//------------------------------------------------------------------------------------------------
	void SpawnPlayer(string spawnPositionName)
	{	
		SCR_MapEntity mapEnt = SCR_MapEntity.Cast(GetGame().GetMapManager());
		if (mapEnt)
			mapEnt.CloseMap();
		
		MenuManager menuManager = GetGame().GetMenuManager();
		if (menuManager)
		{
			MenuBase mapMenu = menuManager.FindMenuByPreset(ChimeraMenuPreset.MapMenu);
			if (mapMenu)
				menuManager.CloseMenu(mapMenu);
		}
		
		SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!pc)
			return;
		
		pc.SetGadgetFocus(false);
		
		IEntity spawnpos = GetGame().GetWorld().FindEntityByName(spawnPositionName);
		if (!spawnpos)
			return;
		
		SCR_PlayerLoadoutComponent playerLoadoutComp = SCR_PlayerLoadoutComponent.Cast(pc.FindComponent(SCR_PlayerLoadoutComponent));
		if (!playerLoadoutComp)
			return;
		
		SCR_BasePlayerLoadout loadout = playerLoadoutComp.GetLoadout();
		if (!loadout)
			return;
		
		SCR_FreeSpawnData data = new SCR_FreeSpawnData(loadout.GetLoadoutResource(), spawnpos.GetOrigin(), spawnpos.GetAngles());
		SCR_RespawnComponent.Cast(pc.FindComponent(SCR_RespawnComponent)).RequestSpawn(data);
		
		spawnpos = spawnpos.GetChildren();
		if (spawnpos)
			SpawnAsset("PlayerVehicle", m_sPlayerVehicleResourceName, spawnpos);
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_TutorialInstructorComponent GetCourseInstructor(SCR_ETutorialCourses course)
	{
		foreach (SCR_TutorialInstructorComponent instructor : m_aInstructors)
		{
			if (!instructor)
				continue;
			
			if (instructor.GetCourseType() == course)
				return instructor;
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	void GetInstructors(notnull array<SCR_TutorialInstructorComponent> instructors)
	{
		instructors.Copy(m_aInstructors);
	}
	
	//------------------------------------------------------------------------------------------------
	void UnregisterInstructor(notnull SCR_TutorialInstructorComponent instructor)
	{	
		if (m_aInstructors)
			m_aInstructors.RemoveItem(instructor);
	}
	
	//------------------------------------------------------------------------------------------------
	void RegisterInstructor(notnull SCR_TutorialInstructorComponent instructor)
	{	
		SCR_Task task = GetCourseTask(instructor.GetCourseType());
		if (task)
			task.SetOrigin(instructor.GetOwner().GetOrigin());
		
		if (m_aInstructors)
			m_aInstructors.Insert(instructor);
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsCourseAvailable(SCR_ETutorialCourses courseType)
	{
		if (!m_CoursesConfig)
			return false;
		
		#ifdef ENABLE_DIAG
		if (m_bIgnoreCourseRequirements && courseType != SCR_ETutorialCourses.OUTRO)
			return true;
		#endif
		
		SCR_TutorialCourse course = m_CoursesConfig.GetCourse(courseType);
		if (!course)
			return false;
		
		return SCR_Enum.HasFlag(m_eFinishedCourses, course.GetRequiredCourses());
	}
	
	//------------------------------------------------------------------------------------------------
	void FastTravelToCourse(SCR_ETutorialCourses course)
	{
		if (!m_CoursesConfig)
			return;
		
		SCR_TutorialCourse courseConf = m_CoursesConfig.GetCourse(course);
		if (!courseConf)
			return;
		
		IEntity playerPos, vehiclePos;
		playerPos = courseConf.GetFastTravelPosition();
		
		while (playerPos)
		{
			if (playerPos.IsInherited(SCR_Position))
			{
				vehiclePos = playerPos.GetChildren();
				break;
			}
			
			playerPos = playerPos.GetSibling();
		}
		
		if (playerPos)
			FastTravel(playerPos);
		
		if (vehiclePos)
			SpawnAsset("PlayerVehicle", m_sPlayerVehicleResourceName, vehiclePos);
	}
	
	//------------------------------------------------------------------------------------------------
	// Fast traveling 
	void FastTravel(notnull IEntity toEnt)
	{	
		m_bIsFastTraveling = true;
		m_FastTravelPosition = toEnt;
		
		if (m_ActiveConfig.GetCourseType() == SCR_ETutorialCourses.FREE_ROAM)
			m_Stage.UnregisterWaypoint("CampGreenhornSign");
		
		Fadeout(true).GetOnCompleted().Insert(OnFastTravelFadeoutCompleted);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnFastTravelFadeoutCompleted()
	{	
		SCR_BaseGameMode gamemode = GetGameMode();
		if (!gamemode)
			return;
		
		if (m_Player && m_Player.IsInVehicle())
		{
			CompartmentAccessComponent compartmentComp = m_Player.GetCompartmentAccessComponent();
			if (compartmentComp)
			{
				compartmentComp.GetOutVehicle(EGetOutType.TELEPORT, -1, false, false);
				
				//Call later for repeated attempt to finalize Fast Travel, after player leaves the vehicle
				GetGame().GetCallqueue().CallLater(OnFastTravelFadeoutCompleted, 1000);
				return;
			}
		}
		
		gamemode.GetOnPreloadFinished().Insert(OnFastTravelPreloadFinished);
		gamemode.StartSpawnPreload(m_FastTravelPosition.GetOrigin()); 
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnFastTravelPreloadFinished()
	{
		SCR_BaseGameMode gamemode = GetGameMode();
		if (!gamemode)
			return;
		
		gamemode.GetOnPreloadFinished().Remove(OnFastTravelPreloadFinished);
		
		vector transformation[4];
		m_FastTravelPosition.GetTransform(transformation);
			
		ChimeraCharacter player = ChimeraCharacter.Cast(GetGame().GetPlayerController().GetControlledEntity());
		if (!player)
			return;
		
		ChimeraWorld world = GetGame().GetWorld();
		if (!world)
			return;
				
		TimeAndWeatherManagerEntity tmwManager = world.GetTimeAndWeatherManager();
		if (!tmwManager)
			return;
		
		tmwManager.SetTimeOfTheDay(tmwManager.GetTimeOfTheDay()+0.2);
		
		if (!player.GetDamageManager().IsDestroyed())
			player.Teleport(transformation);
		else
			SpawnPlayer(m_FastTravelPosition.GetName());
		
		m_bIsFastTraveling = false;
		m_FastTravelPosition = null;
		Fadeout(false);
		HealPlayer();
		
		SCR_HUDManagerComponent hudManager = GetGame().GetHUDManager();

		if (hudManager)
		{
			SCR_VoiceoverSubtitles subtitlesDisplay = SCR_VoiceoverSubtitles.Cast(hudManager.FindInfoDisplay(SCR_VoiceoverSubtitles));

			if (subtitlesDisplay)
				subtitlesDisplay.Show(false);
		}
		
		m_OnFastTravelDone.Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_TutorialCourse GetActiveConfig()
	{
		return m_ActiveConfig;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetupVoiceSystem()
	{
		if (!m_VoiceoverSystem)
			m_VoiceoverSystem = SCR_VoiceoverSystem.GetInstance();
		
		if (!m_ActiveConfig)
			return;
		
		ResourceName voiceDataConfig = m_ActiveConfig.GetVoiceOverDataConfig();
		if (voiceDataConfig.IsEmpty())
			return;
		
		m_VoiceoverSystem.SetData(voiceDataConfig);
	}

	//------------------------------------------------------------------------------------------------
	void RemovePlayerMapMarkers()
	{
		SCR_MapMarkerManagerComponent mapMarkerManager = SCR_MapMarkerManagerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_MapMarkerManagerComponent));
		if (!mapMarkerManager)
			return;
		
		array <SCR_MapMarkerBase> markerArray = {};		
		markerArray = mapMarkerManager.GetStaticMarkers();
		for (int index = markerArray.Count()-1; index >= 0; index--)
		{
			if (!markerArray.IsIndexValid(index) || !markerArray[index])
				continue;
			
			mapMarkerManager.RemoveStaticMarker(markerArray[index]);
		}
		
		// TODO: Fix the following:
		// Without keeping markers in this array the marker system loses last pointer during RemoveStaticMarker and produces a VME
		m_aMarkers.Clear();
	}
		
	//------------------------------------------------------------------------------------------------
	//!
	void HandleAchievement()
	{
		/* Achievement SWEAT_SAVES_BLOOD */
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!playerController)
			return;
		
		SCR_AchievementsHandler handler = SCR_AchievementsHandler.Cast(playerController.FindComponent(SCR_AchievementsHandler));
		if (!handler)
			return;
	
		handler.UnlockAchievement(AchievementId.SWEAT_SAVES_BLOOD);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] descriptorOwnerName
	//! \param[in] enable
	void ShowMapDescriptor(string descriptorOwnerName, bool enable)
	{
		IEntity ent = GetGame().GetWorld().FindEntityByName(descriptorOwnerName);
		if (!ent)
			return;

		SCR_MapDescriptorComponent descr = SCR_MapDescriptorComponent.Cast(ent.FindComponent(SCR_MapDescriptorComponent));
		if (!descr)
			return;

		descr.Item().SetVisible(enable);
	}

	//------------------------------------------------------------------------------------------------
	bool IsPlayerAimingInAngle(float angle, int tolerance)
	{
		if (!m_Player)
			return false;
		
		CharacterControllerComponent charController = m_Player.GetCharacterController();
		if (!charController)
			return false;
		
		CharacterInputContext inputContext = charController.GetInputContext();
		if (!inputContext)
			return false;
		
		float aimAngle = Math.MapAngle(Math.RAD2DEG * inputContext.GetAimingAngles()[0], 360, 0, 360);
		
		return Math.IsInRange(aimAngle, angle-tolerance, angle+tolerance);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] modeEntity
	void OnModeAdded(SCR_EditorModeEntity modeEntity)
	{
		if (modeEntity.GetModeType() != EEditorMode.BUILDING)
			return;
		
		m_PlacingComponent = SCR_PlacingEditorComponent.Cast(modeEntity.FindComponent(SCR_PlacingEditorComponent));
		if (!m_PlacingComponent)
			return;
		
		m_PlacingComponent.GetOnPlaceEntityServer().Insert(OnEntitySpawned);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	SCR_PreviewEntityEditorComponent GetPreviewEntityComponent()
	{
		if (!m_PlacingComponent)
			return null;
		
		IEntity ent = m_PlacingComponent.GetOwner();
		if (!ent)
			return null;
		
		return SCR_PreviewEntityEditorComponent.Cast(ent.FindComponent(SCR_PreviewEntityEditorComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void SetupEditorModeListener()
	{	
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core)
			return;
		
		SCR_EditorManagerEntity editorManager = core.GetEditorManager();
		if (!editorManager)
			return;
		
		editorManager.GetOnModeAdd().Insert(OnModeAdded);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	protected SCR_PlacingEditorComponent GetPlacingComponent()
	{
		return m_PlacingComponent;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnEntitySpawned(int prefabID, SCR_EditableEntityComponent ent)
	{
		if (!ent)
			return;
	
		m_OnEntitySpawned.Invoke(ent.GetOwner());
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnEntitySpawned()
	{
		return m_OnEntitySpawned;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnObjectDestroyed(EDamageState state)
	{
		if (state == EDamageState.DESTROYED)
			SetCourseConfig(SCR_ETutorialCourses.FREE_ROAM);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnInputDeviceIsGamepad(bool isGamepad)
	{
		if (!SCR_HintManagerComponent.GetInstance())
			return;
		
		if (!SCR_HintManagerComponent.GetInstance().IsShown())
			return;
		
		m_bIsUsingGamepad = isGamepad;
		
		if (m_fDelayedControlSchemeChangeRunning)
			return;
		
		m_fDelayedControlSchemeChangeRunning = true;
		GetGame().GetCallqueue().CallLater(OnInputDeviceChangedDelayed, 250);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnInputDeviceChangedDelayed()
	{
		m_fDelayedControlSchemeChangeRunning = false;
		bool switchedToKeyboard = !m_bIsUsingGamepad;
		m_Stage.OnInputDeviceChanged(switchedToKeyboard);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	ChimeraCharacter GetPlayer()
	{
		return m_Player;
	}	
	
	//------------------------------------------------------------------------------------------------
	bool GamepadUsed()
	{
		return m_bIsUsingGamepad;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] fadeOut - if true, screen will fadeout to black, false does the opposite
	WidgetAnimationOpacity Fadeout(bool fadeOut, float duration = 1, float soundDuration = duration)
	{
		if (!m_wFadeOut)
		{
			SCR_HUDManagerComponent hudManager = GetGame().GetHUDManager();
			if (!hudManager)
				return null;
			
			m_wFadeOut = ImageWidget.Cast(hudManager.CreateLayout(FADEOUT_LAYOUT, EHudLayers.OVERLAY));
			
			if (!m_wFadeOut)
				return null;
		}
		
		if (!m_OngoingFadeAnimation)
		{
			m_OngoingFadeAnimation = AnimateWidget.Opacity(m_wFadeOut, fadeOut, duration);
		}
		else
		{
			m_OngoingFadeAnimation.SetTargetValue(fadeOut);
			m_OngoingFadeAnimation.SetSpeed(duration);
		}
		
		SetSoundFade(fadeOut, soundDuration);
		
		return m_OngoingFadeAnimation;
		
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] name
	//! \param[in] type
	//! \param[in] spawnpoint
	//! \return
	IEntity SpawnAsset(string name, ResourceName type, IEntity spawnpoint = null, bool respawn = true)
	{
		if (!spawnpoint)
			spawnpoint = GetGame().GetWorld().FindEntityByName(string.Format("SpawnPos_%1", name));
		
		if (!spawnpoint)
			return null;
		
		IEntity oldEntity;
		if (name != string.Empty)
			oldEntity = GetGame().GetWorld().FindEntityByName(name);

		if (oldEntity && respawn)
		{
			if (m_Player && m_Player.GetParent() == oldEntity)
			{
				oldEntity.SetName("OldPlayerVeh");
				
				//Fix for edge case, where player leaves vehicle during induced course break. As vehicle got removed, so did the player...
				GetGame().GetCallqueue().CallLater(SCR_EntityHelper.DeleteEntityAndChildren, 1000, false, oldEntity);
			}
			else
			{
				SCR_EntityHelper.DeleteEntityAndChildren(oldEntity);
			}
		}
		else if (oldEntity)
		{
			return oldEntity;
		}
		
		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		spawnpoint.GetWorldTransform(params.Transform);
		Resource res = Resource.Load(type);
		
		if (!res)
			return null;
		
		IEntity newEntity = GetGame().SpawnEntityPrefab(res, null, params);
		if (!newEntity)
			return null;
		
		newEntity.SetName(name);
		
		Vehicle veh = Vehicle.Cast(newEntity);
		if (veh)
		{
			Physics physicsComponent = veh.GetPhysics();
			
			if (physicsComponent)
				physicsComponent.SetVelocity("0 -1 0");
		}
		
		if (newEntity && newEntity.IsInherited(ChimeraCharacter))
		{
			SCR_DamageManagerComponent damManager = SCR_DamageManagerComponent.Cast(newEntity.FindComponent(SCR_DamageManagerComponent));
			if (!damManager)
				return newEntity;
				
			damManager.GetOnDamage().Insert(OnCharacterDamaged);
		}
		
		EnableRefunding(newEntity, false);
		
		return newEntity;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] config
	void OnMapOpen(MapConfiguration config)
	{
		m_bIsMapOpen = true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] config
	void OnMapClose(MapConfiguration config)
	{
		m_bIsMapOpen = false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	bool GetIsMapOpen()
	{
		return m_bIsMapOpen;
	}
	
	//------------------------------------------------------------------------------------------------
	//! TODO> MOVE TO OBSTACLE COURSE LOGIC
	//! \param[in] command
	//! \return
	bool CheckCharacterStance(ECharacterCommandIDs command)
	{
		CharacterAnimationComponent comp = CharacterAnimationComponent.Cast(m_Player.FindComponent(CharacterAnimationComponent));
		bool ret = false;
					
		if (!comp)
			return false;
		
		CharacterMovementState mState = new CharacterMovementState();
		comp.GetMovementState(mState);
		
		if (mState)
			ret = mState.m_CommandTypeId == command;
	
		delete mState;
		return ret;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	int GetFreeRoamActivations()
	{
		return m_iFreeRoamActivations;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetFreeRoamActivation(int activations)
	{
		m_iFreeRoamActivations = activations;
	}
	
	//------------------------------------------------------------------------------------------------
	void IncrementFreeRoamActivation()
	{
		m_iFreeRoamActivations++;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void Check3rdPersonViewUsed()
	{
		if (!m_Player)
			return;
		
		CameraHandlerComponent comp = CameraHandlerComponent.Cast(m_Player.FindComponent(CameraHandlerComponent));
		if (!comp)
			return;
		
		if (comp.IsInThirdPerson())
		{
			m_bUsed3PV = true;
			GetGame().GetCallqueue().Remove(Check3rdPersonViewUsed);
			
			/*if (m_eStage == SCR_ECampaignTutorialArlandStage.WALL)
				GetGame().GetCallqueue().Remove(DelayedPopup);*/
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	bool GetWas3rdPersonViewUsed()
	{
		return m_bUsed3PV;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] text
	//! \param[in] subtitle
	//! \param[in] duration
	//! \param[in] param1
	//! \param[in] param2
	//! \param[in] subtitleParam1
	//! \param[in] subtitleParam2
	void DelayedPopup(string text = "", string subtitle = "", float duration = SCR_PopUpNotification.DEFAULT_DURATION, string param1 = "", string param2 = "", string subtitleParam1 = "", string subtitleParam2 = "")
	{
		SCR_PopUpNotification.GetInstance().PopupMsg(text, duration, text2: subtitle, param1: param1, param2: param2, text2param1: subtitleParam1, text2param2: subtitleParam2, category: SCR_EPopupMsgFilter.TUTORIAL);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] base
	//! \param[in] structure
	void OnStructureBuilt(SCR_CampaignMilitaryBaseComponent base, IEntity structure)
	{
		if (m_Stage)
			m_Stage.OnStructureBuilt(base, structure);
	}
	
	//------------------------------------------------------------------------------------------------
	//WAYPOINT LOGIC
	//------------------------------------------------------------------------------------------------
	SCR_WaypointDisplay GetWaypointDisplay()
	{
		if (!m_WaypointDisplay)
		{
			SCR_HUDManagerComponent hudManager = GetGame().GetHUDManager();
			if (!hudManager)
				return null;
			
			 m_WaypointDisplay = SCR_WaypointDisplay.Cast(hudManager.FindInfoDisplay(SCR_WaypointDisplay));
		}
		
		return m_WaypointDisplay;
	}
	
	//------------------------------------------------------------------------------------------------
	//STAGE LOGIC
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] stageName - name of stage in config
	//! \param[in] position - position where stage should be put in order. If -1, stage is put in front of current stage.
	void InsertStage(string stageName, int position = -1)
	{
		SCR_TutorialStageInfo stageToAdd = m_ActiveConfig.GetStageByName(stageName);
		if (!stageToAdd)
			return;
		
		if (position > -1)
			m_aStageInfos.InsertAt(stageToAdd, position);
		else
			m_aStageInfos.InsertAt(stageToAdd, m_iActiveStage+1);
		
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	int GetActiveStageIndex()
	{
		return m_iActiveStage;
	}
	
	//------------------------------------------------------------------------------------------------
	void FinishStage()
	{	
		m_iActiveStage++;
		SCR_ETutorialCourses finishedCourse = m_ActiveConfig.GetCourseType();		

		if (!m_aStageInfos.IsIndexValid(m_iActiveStage))
		{
			array <SCR_ETutorialCourses> finishedCourses = {};
			SCR_Enum.GetEnumValues(SCR_ETutorialCourses, finishedCourses);
			
			for (int i = finishedCourses.Count()-1; i >= 0; i--)
			{
				if (IsCourseAvailable(finishedCourses[i]))
					finishedCourses.Remove(i);
			}
			
			//TODO: NULLCHECK m_ActiveConfig
			m_eFinishedCourses = SCR_Enum.SetFlag(m_eFinishedCourses, m_ActiveConfig.GetCourseType());
			
			for (int i = finishedCourses.Count()-1; i >= 0; i--)
			{
				if (!IsCourseAvailable(finishedCourses[i]))
					finishedCourses.Remove(i);
			}
			
			SCR_HintManagerComponent.HideHint();
			SCR_HintManagerComponent.ClearLatestHint();
						
			GenerateTasks();
			
			string description;
			foreach (SCR_ETutorialCourses course : finishedCourses)
			{
				description += m_CoursesConfig.GetCourse(course).GetTaskTitle() + "<br/>";
			}
			
			if (!description.IsEmpty())
			{
				SCR_HintUIInfo hintInfo = SCR_HintUIInfo.CreateInfo(description, "#AR-Tutorial_Hint_CoursesUnlocked", 7, EHint.UNDEFINED, EFieldManualEntryId.NONE, false);
				SCR_HintManagerComponent.ShowHint(hintInfo);
			}
			
			if (m_ActiveConfig && m_ActiveConfig.GetCourseType() != SCR_ETutorialCourses.INTRO && m_ActiveConfig.GetCourseType() != SCR_ETutorialCourses.OUTRO)
			{
				SCR_PopUpNotification.GetInstance().PopupMsg("#AR-Tutorial_Popup_CourseFinished-UC", text2: m_ActiveConfig.GetTaskTitle(), duration: 7);
				
				ChimeraWorld world = GetGame().GetWorld();
				MusicManager manager = world.GetMusicManager();
				
				if (manager)
					manager.Play(SCR_SoundEvent.SOUND_ONBASECAPTURE);
				
			}
			
			if (m_ActiveConfig && m_ActiveConfig.GetCourseType() != SCR_ETutorialCourses.INTRO)
				SCR_AnalyticsApplication.GetInstance().TutorialCourseEnds(finishedCourse, SCR_EAnalyticsCourseEndReason.COMPLETED);
			
			m_iActiveStage = 0;
			
			if (!SCR_Enum.HasFlag(m_eFinishedCourses, SCR_ETutorialCourses.OUTRO) && IsCourseAvailable(SCR_ETutorialCourses.OUTRO))
			{
				SetCourseConfig(SCR_ETutorialCourses.OUTRO);
				
				if (!m_bStagesComplete)
				{
					m_bStagesComplete = true;
					HandleAchievement();
				}
			}
			else
			{
				SetCourseConfig(SCR_ETutorialCourses.FREE_ROAM);
			}
			
			ChimeraWorld world = GetGame().GetWorld();
			
			if (world)
			{
				if (finishedCourse == SCR_ETutorialCourses.OUTRO)
					world.GetMusicManager().Play("SOUND_END_GOOD");
				else if (finishedCourse != SCR_ETutorialCourses.INTRO && finishedCourse != SCR_ETutorialCourses.FREE_ROAM)
					world.GetMusicManager().Play("SOUND_COURSE_DONE");
			}
			
			GetGame().GetSaveGameManager().RequestSavePoint(ESaveGameType.AUTO);
			return;
		}
			
		SetStage(m_iActiveStage);
	}
	
	//------------------------------------------------------------------------------------------------
	int GetStageIndexByName(string name)
	{
		if (!m_aStageInfos || m_aStageInfos.IsEmpty())
			return -1;
		
		foreach (int i, SCR_TutorialStageInfo info : m_aStageInfos)
		{
			if (info.GetStageName() == name)
				return i; 
		}
		
		return -1;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetStage(string stageName)
	{
		SetStage(GetStageIndexByName(stageName));
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] stage
	void SetStage(int stage)
	{	
		if (stage <= -1)
			return;
		
		delete m_Stage;
		m_AreaRestrictionCenter = null;
		
		// Run the next stage
		if (!m_aStageInfos)
		{
			SetCourseConfig(SCR_ETutorialCourses.FREE_ROAM);
			return;
		}
		
		if (!m_aStageInfos.IsIndexValid(stage))
			return;
		
		m_Stage = SCR_BaseTutorialStage.Cast(GetGame().SpawnEntity(m_aStageInfos[stage].GetClassName().ToType()));
		m_Stage.Init(m_aStageInfos[stage]);
		
		m_iActiveStage = stage;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] config
	void SetCourseConfig(SCR_ETutorialCourses config, string stageName = string.Empty, bool spawnAssets = true)
	{			
		if (!m_CoursesConfig)
			return;
		
		RemovePlayerMapMarkers();
		
		//Finish any current course 
		if (m_ActiveConfig)
			m_ActiveConfig.OnFinish();
		
		//Clear old stages
		m_aStageInfos.Clear();
		delete m_Stage;
		
		m_aNarratedCharacters = null;
		
		DespawnCourseAssets();

		m_ActiveConfig = m_CoursesConfig.GetCourse(config);
		if (!m_ActiveConfig)
			return;
				
		m_ActiveConfig.GetStages(m_aStageInfos);
		
		if (spawnAssets)
			SpawnCourseAssets();
		
		if (m_ActiveConfig.GetCourseType() != SCR_ETutorialCourses.FREE_ROAM && m_ActiveConfig.GetCourseType() != SCR_ETutorialCourses.INTRO && m_ActiveConfig.GetCourseType() != SCR_ETutorialCourses.OUTRO)
		{
			IEntity position = m_ActiveConfig.GetFastTravelPosition();
			if (position)
			{
				position = position.GetChildren();
				IEntity playerVehicle = GetGame().GetWorld().FindEntityByName("PlayerVehicle");
				if (!playerVehicle || (vector.Distance(playerVehicle.GetOrigin(), position.GetOrigin()) >= 15))
					SpawnAsset("PlayerVehicle", m_sPlayerVehicleResourceName, position);
			}
		}
		
		SetupVoiceSystem();
		
		m_iActiveStage = 0;
		m_ActiveConfig.OnStart();
		
		if (!SCR_StringHelper.IsEmptyOrWhiteSpace(stageName))
			InsertStage(stageName, 0);
		
		SetStage(0);

		if (m_OngoingFadeAnimation)
			m_OngoingFadeAnimation.Stop();
		
		HideTasks(m_ActiveConfig.GetCourseType() != SCR_ETutorialCourses.FREE_ROAM);
		
		if (config != SCR_ETutorialCourses.INTRO && config != SCR_ETutorialCourses.FREE_ROAM && config != SCR_ETutorialCourses.OUTRO)
		{
			ChimeraWorld world = GetGame().GetWorld();
			
			if (world)
				world.GetMusicManager().Play("SOUND_COURSE_START");
		}
		
		if (config != SCR_ETutorialCourses.INTRO && config != SCR_ETutorialCourses.FREE_ROAM)
			SCR_AnalyticsApplication.GetInstance().TutorialCourseStarts(config);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnCharacterDamaged(BaseDamageContext damageContext)
	{
		if (!m_Stage || m_bCourseBreaking)
			return;
		
		Instigator instigator = damageContext.instigator;
		if (!instigator)
			return;
		
		//Handling of situation, where player crashes vehicle as driver and AI is passanger
		if (damageContext.damageType == EDamageType.COLLISION)
		{
			ChimeraCharacter instigatorEnt = ChimeraCharacter.Cast(instigator.GetInstigatorEntity());
			if (instigatorEnt && instigatorEnt.IsInVehicle() && instigatorEnt.GetParent() == damageContext.hitEntity.GetParent())
			{
				//No damage manager == no damage, so no need to punish
				SCR_CharacterDamageManagerComponent damManager = SCR_CharacterDamageManagerComponent.Cast(damageContext.hitEntity.FindComponent(SCR_CharacterDamageManagerComponent));
				if (!damManager)
					return;
				
				//Do nothing unless character is bleeding and alive
				if (!damManager.IsBleeding() && !damManager.IsDestroyed())
					return;
				
				RequestBreakCourse(SCR_ETutorialBreakType.FORCED);
				return;
			}
		}
		
		if (damageContext.damageSource)
			RequestBreakCourse(SCR_ETutorialBreakType.PLAYER_GRIEFING_NPC);
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsDamagedOrDestroyed(notnull IEntity ent)
	{
		SCR_DamageManagerComponent damManager = SCR_DamageManagerComponent.Cast(ent.FindComponent(SCR_DamageManagerComponent));
		if (!damManager)
			return false;
		
		return damManager.IsDestroyed() || damManager.GetHealth() != damManager.GetMaxHealth();
	}
	
	//------------------------------------------------------------------------------------------------
	void SpawnCourseAssets(SCR_ETutorialCourses courseType = -1)
	{
		SCR_DamageManagerComponent damManager;
		IEntity position, entity;
		
		array <SCR_TutorialCoursePrefabInfo> prefabInfos = {};
		
		if (courseType != -1)
		{
			SCR_TutorialCourse course = m_CoursesConfig.GetCourse(courseType);
			if (!course)
				return;
			
			course.GetPrefabs(prefabInfos);
		}
		else
		{
			m_ActiveConfig.GetPrefabs(prefabInfos);
		}
		
		foreach (SCR_TutorialCoursePrefabInfo prefabInfo : prefabInfos)
		{
			entity = GetGame().GetWorld().FindEntityByName(prefabInfo.m_sEntityName);
			position = GetGame().GetWorld().FindEntityByName(prefabInfo.m_sSpawnPosName);

			if ((entity && !prefabInfo.m_bRespawn) || !position)
				continue;
			
			if (entity && (prefabInfo.m_bRespawnDamaged && !IsDamagedOrDestroyed(entity)))
				continue;
			
			if (entity && prefabInfo.m_bRespawnDistance && (vector.Distance(entity.GetOrigin(), position.GetOrigin()) < prefabInfo.m_fRespawnMinimumDistance))
			{
				if (!IsDamagedOrDestroyed(entity))
					continue;
			}
				
			SpawnAsset(prefabInfo.m_sEntityName, prefabInfo.m_sPrefabResourceName, position);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void DespawnCourseAssets()
	{
		if (!m_ActiveConfig)
			return;
		
		array <SCR_TutorialCoursePrefabInfo> prefabInfos = {};
		m_ActiveConfig.GetPrefabs(prefabInfos);
		
		IEntity ent;
		foreach (SCR_TutorialCoursePrefabInfo prefabInfo : prefabInfos)
		{
			if (!prefabInfo.m_bDespawnAfterCourse)
				continue;
			
			if (prefabInfo.m_sEntityName == string.Empty)
				continue;
			
			ent = GetGame().GetWorld().FindEntityByName(prefabInfo.m_sEntityName);
			if (!ent)
				continue;
			
			SCR_EntityHelper.DeleteEntityAndChildren(ent);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//OVERRIDE EVENT AND METHODS RespawnPlayer	
	//------------------------------------------------------------------------------------------------
	override void OnPlayerRegistered(int playerId)
	{		
		super.OnPlayerRegistered(playerId);
		
		// Attempt to spawn the player automatically, cease after spawn is successful in OnPlayerSpawned
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);
		
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		SCR_PlayerFactionAffiliationComponent factionComp = SCR_PlayerFactionAffiliationComponent.Cast(playerController.FindComponent(SCR_PlayerFactionAffiliationComponent));
		factionComp.RequestFaction(factionManager.GetFactionByKey("US"));

		SCR_LoadoutManager loadoutManager = GetGame().GetLoadoutManager();
		SCR_PlayerLoadoutComponent loadoutComp = SCR_PlayerLoadoutComponent.Cast(playerController.FindComponent(SCR_PlayerLoadoutComponent));
		loadoutComp.RequestLoadout(loadoutManager.GetLoadoutByIndex(0));

		if (!m_bFirstStageSet)
		{
			m_bFirstStageSet = true;
			
			//TODO: @yuri pls fix for the future, very dirty indeed.
			GetGame().GetCallqueue().CallLater(Check3rdPersonViewUsed, 500, true);
		}

		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (editorManager)
		{
			editorManager.SetCanOpen(true, EEditorCanOpen.ALIVE);
			editorManager.AutoInit();
		}		
		
		AudioSystem.SetMasterVolume(AudioSystem.SFX, 0);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPlayerSpawnFinalize_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnHandlerComponent handlerComponent, SCR_SpawnData data, IEntity entity)
	{
		m_Player = ChimeraCharacter.Cast(entity);
		HandlePlayerGroup();
		
		ChimeraWorld world = GetGame().GetWorld();
		if (!world)
			return;
		
		GarbageSystem garbageSys = GarbageSystem.Cast(world.FindSystem(GarbageSystem));
		if (garbageSys)
			garbageSys.Flush(0);
		
		if (m_ActiveConfig && m_ActiveConfig.GetCourseType() != SCR_ETutorialCourses.INTRO)
			Fadeout(false);
		
		m_OnPlayerSpawned.Invoke();
		
		if (world)
			world.GetMusicManager().Play("SOUND_ONSPAWN");
	}
	
	//------------------------------------------------------------------------------------------------
	void PlayBedAnimation(bool fast)
	{		
		SCR_CharacterControllerComponent characterController = SCR_CharacterControllerComponent.Cast(m_Player.FindComponent(SCR_CharacterControllerComponent));
		if (!characterController)
			return;
		
		IEntity position = GetGame().GetWorld().FindEntityByName("Respawn_Start");
		if (!position)
			return;
		
		vector bedTransformation[4];
		position.GetTransform(bedTransformation);
		
		if (fast)	
			characterController.StartLoitering(8, false, true, true, bedTransformation, false);
		else
			characterController.StartLoitering(7, true, true, true, bedTransformation, false);
		
 		GetGame().GetCallqueue().CallLater(Fadeout, 1500, false, false, 1, 0);
	}
	
	//------------------------------------------------------------------------------------------------
	//override void OnControllableDestroyed(IEntity entity, IEntity killerEntity, notnull Instigator killer)
	override void OnControllableDestroyed(notnull SCR_InstigatorContextData instigatorContextData)
	{
		IEntity victim = instigatorContextData.GetVictimEntity();
		
		if (!victim)
			return;
		
		//Played died
		if (victim == m_Player)
		{	
			RequestBreakCourse(SCR_ETutorialBreakType.PLAYER_DEATH);
			return;
		}
		
		//Character died
		ChimeraCharacter character = ChimeraCharacter.Cast(victim);
		if (character)
		{
			IEntity killer = instigatorContextData.GetKillerEntity();
			//Ignore punishment if character is sitting in same vehicle as player.
			if ((character.IsInVehicle() && killer && character.GetParent() == killer.GetParent() && CanBreakCourse()) || m_ActiveConfig.GetCourseType() == SCR_ETutorialCourses.HELICOPTERS)
				RequestBreakCourse(SCR_ETutorialBreakType.FORCED);
			else
				RequestBreakCourse(SCR_ETutorialBreakType.PLAYER_GRIEFING_NPC);
			
			return;
		}
		
		//Vehicle destroyed (and player is not sitting in it)
		SCR_VehicleDamageManagerComponent vehDamManager = SCR_VehicleDamageManagerComponent.Cast(victim.FindComponent(SCR_VehicleDamageManagerComponent));
		if (vehDamManager)
			OnVehicleDamaged(vehDamManager);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnGameModeStart()
	{
		SCR_HUDManagerComponent hudManager = GetGame().GetHUDManager();
		if (hudManager)
		{
			m_wFadeOut = ImageWidget.Cast(hudManager.CreateLayout(FADEOUT_LAYOUT, EHudLayers.OVERLAY));
			if (m_wFadeOut)
				m_wFadeOut.SetOpacity(1);
		}

		GenerateTasks();
		
		IEntity position = GetGame().GetWorld().FindEntityByName("FT_DRIVING");
		if (position)
		{
			position = position.GetChildren();
			if (position)
				SpawnAsset("PlayerVehicle", m_sPlayerVehicleResourceName, position);
		}
		
		IEntity door = GetGame().GetWorld().FindEntityByName("TUT_BAR_DOOR_01");
		if (door)
			door.SetOrigin("0 0 0");
		
		door = GetGame().GetWorld().FindEntityByName("TUT_BAR_DOOR_02");
		if (door)
			door.SetOrigin("0 0 0");
		
		if (SCR_Enum.HasFlag(m_eFinishedCourses, SCR_ETutorialCourses.INTRO))
			SpawnPlayer("Respawn_Start_Load");
		else
			SpawnPlayer("Respawn_Start");
		
		#ifdef ENABLE_DIAG
			if (!SCR_Enum.HasFlag(m_eFinishedCourses, SCR_ETutorialCourses.OUTRO) && IsCourseAvailable(SCR_ETutorialCourses.OUTRO))
				SetCourseConfig(SCR_ETutorialCourses.OUTRO);
			else if (m_bSkipIntro || SCR_Enum.HasFlag(m_eFinishedCourses, SCR_ETutorialCourses.INTRO))
				SetCourseConfig(SCR_ETutorialCourses.FREE_ROAM);
			else
				SetCourseConfig(SCR_ETutorialCourses.INTRO);
		#else
			if (!SCR_Enum.HasFlag(m_eFinishedCourses, SCR_ETutorialCourses.OUTRO) && IsCourseAvailable(SCR_ETutorialCourses.OUTRO))
				SetCourseConfig(SCR_ETutorialCourses.OUTRO);
			else if (SCR_Enum.HasFlag(m_eFinishedCourses, SCR_ETutorialCourses.INTRO))
				SetCourseConfig(SCR_ETutorialCourses.FREE_ROAM);
			else
				SetCourseConfig(SCR_ETutorialCourses.INTRO);
		#endif
	}
	
	//------------------------------------------------------------------------------------------------
	void HideTasks(bool hide)
	{
		SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
		if (!taskSystem)
			return;
		
		array <SCR_Task> tasks = {};
		taskSystem.GetTasks(tasks);
		
		string taskId;
		
		foreach (SCR_Task task : tasks)
		{
			if (!task)
				continue;
			
			taskId = task.GetTaskID();
			if (SCR_StringHelper.IsEmptyOrWhiteSpace(taskId) || task.GetTaskID().ToInt() == SCR_ETutorialCourses.OUTRO)
				continue;
			
			if (!IsCourseAvailable(taskId.ToInt()))
				continue;
			
			if (hide)
				task.SetTaskVisibility(SCR_ETaskVisibility.NONE);
			else
				task.SetTaskVisibility(SCR_ETaskVisibility.EVERYONE);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void GenerateTasks()
	{
		array <ref SCR_TutorialCourse> courses = {};
		m_CoursesConfig.GetConfigs(courses);
		
		int courseType;
		SCR_Task task;
		string taskName;
		
		foreach (SCR_TutorialCourse course : courses)
		{
			if (!course)
				continue;
			
			courseType = course.GetCourseType();
			
			if (course.CanCreateTask())
				CreateCourseTask(courseType);
			
			if (SCR_Enum.HasFlag(m_eFinishedCourses, courseType))
			{
				task = SCR_TaskSystem.GetTaskFromTaskID(courseType.ToString());
				if (!task)
					continue;
				
				taskName = task.GetTaskName();
				if (!taskName.Contains(TASK_COMPLETED_SUFFIX))
					task.SetTaskName(taskName + " " + TASK_COMPLETED_SUFFIX);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void CheckAreaRestrictions()
	{
		//TODO> Mouthful. Circumisse
		if (!m_ActiveConfig || m_ActiveConfig.GetCourseType() == SCR_ETutorialCourses.FREE_ROAM || !m_Player || m_bCourseBreaking)
			return;
		
		SCR_TutorialStageInfo stageInfo = m_Stage.GetStageInfo();
		if (!stageInfo)
			return;
		
		string areaRestrictionEntName = stageInfo.GetAreaRestrictionEntityOverride();
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(areaRestrictionEntName))
			areaRestrictionEntName = m_ActiveConfig.GetCourseAreaCenterEntityName();
		
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(areaRestrictionEntName))
			return;
		
		m_AreaRestrictionCenter = GetGame().GetWorld().FindEntityByName(areaRestrictionEntName);
		if (!m_AreaRestrictionCenter)
		{
			m_bProtectionWarning = false;
			return;
		}
		
		float warningDist = stageInfo.GetAreaRestrictionWarningOverride();
		if (warningDist == -1)
			warningDist = m_ActiveConfig.GetWarningDistance();
		
		float breakDist = stageInfo.GetAreaRestrictionBreakingOverride();
		if (breakDist == -1)
			breakDist = m_ActiveConfig.GetCourseBreakDistance(); 
		
		if (warningDist == -1 || breakDist == -1)
			return;
		
		vector playerPos = m_Player.GetOrigin();
		if (vector.DistanceSq(playerPos, m_AreaRestrictionCenter.GetOrigin()) > (Math.Pow(warningDist, 2)))
		{
			if (vector.DistanceSq(playerPos, m_AreaRestrictionCenter.GetOrigin()) > (Math.Pow(breakDist, 2)))
			{
				m_bProtectionWarning = false;
				if (m_ActiveConfig.GetCourseType() == SCR_ETutorialCourses.INTRO)
				{
					m_eFinishedCourses = SCR_Enum.SetFlag(m_eFinishedCourses, SCR_ETutorialCourses.INTRO);
					SetCourseConfig(SCR_ETutorialCourses.FREE_ROAM);
					return;
				}
				
				RequestBreakCourse(SCR_ETutorialBreakType.FORCED);
				
				ChimeraWorld world = GetGame().GetWorld();
			
				if (world)
					world.GetMusicManager().Play("SOUND_LEAVING_COURSE");
			}
			else if (!m_bProtectionWarning)
			{
				m_bProtectionWarning = true;
				
				SCR_HintUIInfo hintInfo = SCR_HintUIInfo.CreateInfo("#AR-Tutorial_Hint_LeavingCourseArea", "#AR-Tutorial_Hint_Warning", 20, EHint.UNDEFINED, EFieldManualEntryId.NONE, true);
				if (hintInfo)
					SCR_HintManagerComponent.ShowHint(hintInfo);
			}
		}
		else if (m_bProtectionWarning)
		{
			SCR_HintManagerComponent.HideHint();
			SCR_HintManagerComponent.ClearLatestHint();
			
			if (m_Stage)
			{
				SCR_HintUIInfo hintInfo = m_Stage.GetStageInfo().GetHint();
				if (hintInfo)
					SCR_HintManagerComponent.ShowHint(hintInfo);
			}
			
			m_bProtectionWarning = false;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetSoundFade(bool fadeOut, float duration = 0)
	{
		if (duration == 0)
		{
			AudioSystem.SetMasterVolume(AudioSystem.SFX, !fadeOut);
			AudioSystem.SetMasterVolume(AudioSystem.Dialog, !fadeOut);
			
			m_fSoundInterpolationSpeed = 0;
			m_bApplySoundFade = false;
		}
		else
		{
			m_fSoundInterpolationSpeed = 1 / duration;
		
			if (fadeOut)
				m_fSoundInterpolationSpeed *= -1;
			
			m_bApplySoundFade = true;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void HandleSoundFade(float timeSlice)
	{
		if (!m_bApplySoundFade)
			return;
		
		m_fSoundVolume += m_fSoundInterpolationSpeed * timeSlice;
		
		m_fSoundVolume = Math.Clamp(m_fSoundVolume, 0, 1);
		
		AudioSystem.SetMasterVolume(AudioSystem.SFX, m_fSoundVolume);
		AudioSystem.SetMasterVolume(AudioSystem.Dialog, m_fSoundVolume);
		
		if (m_fSoundVolume == 1 || m_fSoundVolume == 0)
			m_bApplySoundFade = false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void EOnFrame(IEntity owner, float timeSlice)
	{
		//TODO: Optimize
		CheckAreaRestrictions();
		
		HandleSoundFade(timeSlice);
		
		if (m_aGarbage)
			HandleGarbage();
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnPostInit(IEntity owner)
	{
#ifdef ENABLE_DIAG
		// Cheat menu
		DiagMenu.RegisterMenu(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_MENU, "Tutorial", "");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_DISABLE_COURSE_REQUIREMENTS, "", "Disable course reqs ", "Tutorial");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_SKIP_INTRO, "", "Skip intro sequence", "Tutorial");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_REGENERATE_TASKS, "", "Regenerate course tasks", "Tutorial");
		DiagMenu.RegisterMenu(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_COURSE_LOCATIONS, "Course locations", "Tutorial");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_COURSE_SKIP_AI_DRIVING, "", "Skip AI Driving", "Tutorial");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_COURSE_FINISH_STAGE, "", "Finish current stage", "Tutorial");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_COURSE_MOVE_TO_WP, "", "Move to waypoint", "Tutorial");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_SET_EVENING_TIME, "", "Set evening time", "Tutorial");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_DUMP_STAGES_INTO_LOG, "", "Dump stages into log", "Tutorial");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_FINISH_EVERYTHING, "", "Finish all courses", "Tutorial");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_DRAW_AREA_RESTRICTIONS, "", "Draw Area Restrictions", "Tutorial");
				
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_COURSE_LOCATION_OBSTACLE, "", "OBSTACLE COURSE", "Course locations");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_COURSE_LOCATION_FIRING, "", "FIRING RANGE", "Course locations");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_COURSE_LOCATION_ENGINEERING, "", "COMBAT ENGINEERING", "Course locations");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_COURSE_LOCATION_SEIZING, "", "SEIZING", "Course locations");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_COURSE_LOCATION_AID, "", "FIRST AID", "Course locations");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_COURSE_LOCATION_DRIVING, "", "DRIVING", "Course locations");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_COURSE_LOCATION_NAVIGATION, "", "NAVIGATION", "Course locations");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_COURSE_LOCATION_HELICOPTERS, "", "HELICOPTER PILOTING", "Course locations");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_COURSE_LOCATION_SPECIAL, "", "SPECIAL WEAPONS", "Course locations");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_COURSE_LOCATION_LONGRANGE, "", "LONG-DISTANCE SHOOTING", "Course locations");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_COURSE_LOCATION_FIRESUPPORT, "", "FIRE SUPPORT COURSE", "Course locations");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_COURSE_LOCATION_LEADERSHIP, "", "SQUAD LEADERSHIP", "Course locations");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_COURSE_LOCATION_MAINTENANCE, "", "VEHICLE MAINTENANCE", "Course locations");
		
		ConnectToDiagSystem(owner);
#endif
		if (!GetGame().InPlayMode())
			return;
		
		LoadCoursesConfig();
		if (!m_CoursesConfig)
			return;
		
		GetGame().OnInputDeviceIsGamepadInvoker().Insert(OnInputDeviceIsGamepad);
		SetEventMask(owner, EntityEvent.FRAME);
		
		SCR_VoiceoverSubtitles.SetLingerDuration(0.5);
		
		ScriptInvoker onVehicleDamageStateChanged = SCR_VehicleDamageManagerComponent.GetOnVehicleDamageStateChanged();
		if (onVehicleDamageStateChanged)
			onVehicleDamageStateChanged.Insert(OnVehicleDamaged);
		
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (core)
			core.Event_OnEditorManagerCreatedServer.Insert(SetupEditorModeListener);
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode)
			gameMode.PauseGame(false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void LoadCoursesConfig()
	{
		Resource holder = BaseContainerTools.LoadContainer(m_sCoursesConfigResourceName);
		if (!holder)
			return;
		
		BaseContainer container = holder.GetResource().ToBaseContainer();
		if (!container)
			return;
		
		m_CoursesConfig = SCR_TutorialCoursesConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(container));
	}
	
#ifdef ENABLE_DIAG
	//------------------------------------------------------------------------------------------------
	void DrawAreaRestrictions()
	{
		if (!m_AreaRestrictionCenter)
			return;
		
		SCR_TutorialStageInfo stageInfo = m_Stage.GetStageInfo();
		float warningDist = stageInfo.GetAreaRestrictionWarningOverride();
		if (warningDist == -1)
			warningDist = m_ActiveConfig.GetWarningDistance();
		
		Shape.CreateSphere(Color.YELLOW, ShapeFlags.ONCE | ShapeFlags.NOZWRITE | ShapeFlags.WIREFRAME, m_AreaRestrictionCenter.GetOrigin(), warningDist);
		
		float breakDist = stageInfo.GetAreaRestrictionBreakingOverride();
		if (breakDist == -1)
			breakDist = m_ActiveConfig.GetCourseBreakDistance();
		
		Shape.CreateSphere(Color.RED, ShapeFlags.ONCE | ShapeFlags.NOZWRITE | ShapeFlags.WIREFRAME, m_AreaRestrictionCenter.GetOrigin(), breakDist);
	}
	
	//------------------------------------------------------------------------------------------------
	void SkipAIDrive()
	{
		Vehicle jeep = Vehicle.Cast(GetGame().GetWorld().FindEntityByName("COMMANDING_JEEP"));
		if (!jeep)
			return;
		
		IEntity newPos = GetGame().GetWorld().FindEntityByName("COMMANDING_CROSSROAD_STOP");
		if (!newPos)
			return;
		
		vector transformation[4];
		newPos.GetTransform(transformation);
		jeep.SetTransform(transformation);
		
		if (!jeep.GetPilot())
			SetCourseConfig(SCR_ETutorialCourses.FREE_ROAM);
	}
	
	//------------------------------------------------------------------------------------------------
	void TeleportToInstructor(SCR_ETutorialCourses course)
	{
		IEntity instructorEnt;
		foreach (SCR_TutorialInstructorComponent instructor : m_aInstructors)
		{
			if (!instructor || instructor.GetCourseType() != course)
				continue;
			
			instructorEnt = instructor.GetOwner();
		}
		
		if (!instructorEnt || !m_Player)
			return;
		
		m_Player.SetOrigin(instructorEnt.GetOrigin());
	}
	
	//------------------------------------------------------------------------------------------------
	void SetCourseCompleted(SCR_ETutorialCourses course)
	{
		m_eFinishedCourses = SCR_Enum.SetFlag(m_eFinishedCourses, course);
		GenerateTasks();
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnDiag(IEntity owner, float timeSlice)
	{
		super.EOnDiag(owner, timeSlice);
		
		// Cheat menu
		m_bIgnoreCourseRequirements = DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_DISABLE_COURSE_REQUIREMENTS);
		m_bSkipIntro = DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_SKIP_INTRO);
		
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_DUMP_STAGES_INTO_LOG))
		{
			DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_DUMP_STAGES_INTO_LOG, 0);
			
			Print("***STAGE LIST***");
			foreach (int i, SCR_TutorialStageInfo stageInfo : m_aStageInfos)
			{
				if (i == GetActiveStageIndex())
					PrintFormat(">> %1 - %2 - %3", i, stageInfo.GetClassName(), stageInfo.GetStageName());
				else
					PrintFormat("|| %1 - %2 - %3", i, stageInfo.GetClassName(), stageInfo.GetStageName());
			}
			Print("***-***");
		}
		
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_REGENERATE_TASKS))
		{
			DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_REGENERATE_TASKS, 0);
			HideTasks(false);
		}
		
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_COURSE_FINISH_STAGE))
		{
			DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_COURSE_FINISH_STAGE, 0);
			m_Stage.FinishStage();
		}
		
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_COURSE_MOVE_TO_WP))
		{
			DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_COURSE_MOVE_TO_WP, 0);
			
			if (!m_Stage)
				return;
			
			SCR_Waypoint wp = m_Stage.GetWaypoint();
			if (wp)
			{
				m_Player.SetOrigin(wp.GetPosition());
			}
		}
		
		// COURSE TELEPORTS
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_COURSE_LOCATION_OBSTACLE))
		{
			DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_COURSE_LOCATION_OBSTACLE, 0);
			TeleportToInstructor(SCR_ETutorialCourses.OBSTACLE_COURSE);
		}
		
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_COURSE_LOCATION_FIRING))
		{
			DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_COURSE_LOCATION_FIRING, 0);
			TeleportToInstructor(SCR_ETutorialCourses.SHOOTING_RANGE);
		}
		
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_COURSE_LOCATION_ENGINEERING))
		{
			DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_COURSE_LOCATION_ENGINEERING, 0);
			TeleportToInstructor(SCR_ETutorialCourses.COMBAT_ENGINEERING);
		}
		
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_COURSE_LOCATION_SEIZING))
		{
			DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_COURSE_LOCATION_SEIZING, 0);
			TeleportToInstructor(SCR_ETutorialCourses.CONFLICT);
		}
		
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_COURSE_LOCATION_AID))
		{
			DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_COURSE_LOCATION_AID, 0);
			TeleportToInstructor(SCR_ETutorialCourses.FIRST_AID);
		}
		
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_COURSE_LOCATION_DRIVING))
		{
			DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_COURSE_LOCATION_DRIVING, 0);
			TeleportToInstructor(SCR_ETutorialCourses.DRIVING);
		}
		
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_COURSE_LOCATION_NAVIGATION))
		{
			DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_COURSE_LOCATION_NAVIGATION, 0);
			TeleportToInstructor(SCR_ETutorialCourses.NAVIGATION);
		}
		
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_COURSE_LOCATION_HELICOPTERS))
		{
			DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_COURSE_LOCATION_HELICOPTERS, 0);
			TeleportToInstructor(SCR_ETutorialCourses.HELICOPTERS);
		}
		
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_COURSE_LOCATION_SPECIAL))
		{
			DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_COURSE_LOCATION_SPECIAL, 0);
			TeleportToInstructor(SCR_ETutorialCourses.SPECIAL_WEAPONS);
		}
		
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_COURSE_LOCATION_LONGRANGE))
		{
			DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_COURSE_LOCATION_LONGRANGE, 0);
			TeleportToInstructor(SCR_ETutorialCourses.LONG_RANGE_SHOOTING);
		}
		
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_COURSE_LOCATION_FIRESUPPORT))
		{
			DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_COURSE_LOCATION_FIRESUPPORT, 0);
			TeleportToInstructor(SCR_ETutorialCourses.FIRE_SUPPORT);
		}
		
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_COURSE_LOCATION_LEADERSHIP))
		{
			DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_COURSE_LOCATION_LEADERSHIP, 0);
			TeleportToInstructor(SCR_ETutorialCourses.SQUAD_LEADERSHIP);
		}
		
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_COURSE_LOCATION_MAINTENANCE))
		{
			DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_COURSE_LOCATION_MAINTENANCE, 0);
			TeleportToInstructor(SCR_ETutorialCourses.VEHICLE_MAINTENANCE);
		}
		
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_COURSE_SKIP_AI_DRIVING))
		{
			DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_COURSE_SKIP_AI_DRIVING, 0);
			SkipAIDrive();
		}
		
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_FINISH_EVERYTHING))
		{
			DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_FINISH_EVERYTHING, 0);
			
			array <int> courses = {};
			SCR_Enum.GetEnumValues(SCR_ETutorialCourses, courses);
			
			foreach (int course : courses)
			{
				if (course == SCR_ETutorialCourses.OUTRO)
					continue;
				
				SetCourseCompleted(course);
			}
			
			if (!m_bStagesComplete)
			{
				m_bStagesComplete = true;
				HandleAchievement();
			}
			
			SetCourseConfig(SCR_ETutorialCourses.OUTRO);
		}
		
		m_bDrawAreaRestrictions = DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_NEWTUTORIAL_DRAW_AREA_RESTRICTIONS);
		if (m_bDrawAreaRestrictions)
			DrawAreaRestrictions();
	}
#endif
	
	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		super.OnDelete(owner);
		DisconnectFromDiagSystem(owner);
		
		SetSoundFade(false);
	}
	
	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_TutorialGamemodeComponent()
	{	
		if (m_wPunishmentScreen)
			m_wPunishmentScreen.RemoveFromHierarchy();
		
		if (m_wFadeOut)
			m_wFadeOut.RemoveFromHierarchy();
		
		ScriptInvoker onVehicleDamageStateChanged = SCR_VehicleDamageManagerComponent.GetOnVehicleDamageStateChanged();
		if (onVehicleDamageStateChanged)
			onVehicleDamageStateChanged.Remove(OnVehicleDamaged);
		
		GetGame().OnInputDeviceIsGamepadInvoker().Remove(OnInputDeviceIsGamepad);
		
		if (m_fSavedViewDistance != 0)
			GetGame().SetViewDistance(m_fSavedViewDistance);
	}
}