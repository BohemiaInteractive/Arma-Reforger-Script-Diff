[EntityEditorProps(category: "GameScripted/Triggers", description: "")]
class SCR_ScenarioFrameworkTriggerEntityClass : SCR_BaseTriggerEntityClass
{
}

void ScriptInvokerTriggerUpdated(float activationCountdownTimer, float tempWaitTime, int playersCountByFactionInside, int playersCountByFaction, string playerActivationNotificationTitle, bool triggerConditionsStatus, float minimumPlayersNeededPercentage);
typedef func ScriptInvokerTriggerUpdated;

class SCR_ScenarioFrameworkTriggerEntity : SCR_BaseTriggerEntity
{
	[Attribute(desc: "Faction which is used for area control calculation. Leave empty for any faction.", category: "Trigger")]
	protected FactionKey m_sOwnerFactionKey;

	[Attribute("0", UIWidgets.ComboBox, "By whom the trigger is activated", "", ParamEnumArray.FromEnum(SCR_EScenarioFrameworkTriggerActivation), category: "Trigger")]
	protected SCR_EScenarioFrameworkTriggerActivation m_eActivationPresence;

	[Attribute(desc: "Fill the entity names here for detection. Is combined with other filters using OR.", category: "Trigger")]
	protected ref array<string> m_aSpecificEntityNames;

	[Attribute(desc: "Fill the class names here for detection. Is combined with other filters using OR.", category: "Trigger")]
	protected ref array<string> m_aSpecificClassNames;

	[Attribute(desc: "Which Prefabs and if their children will be detected by the trigger. Is combined with other filters using OR.", category: "Trigger")]
	protected ref array<ref SCR_ScenarioFrameworkPrefabFilter> m_aPrefabFilter;

	[Attribute(desc: "Here you can input custom trigger conditions that you can create by extending the SCR_CustomTriggerConditions", uiwidget: UIWidgets.Object)]
	protected ref array<ref SCR_ScenarioFrameworkActivationConditionBase> m_aCustomTriggerConditions;
	
	[Attribute(defvalue: SCR_EScenarioFrameworkLogicOperators.AND.ToString(), UIWidgets.ComboBox, "Which Boolean Logic will be used for Custom Trigger Conditions.", "", enums: SCR_EScenarioFrameworkLogicOperatorHelper.GetParamInfo(), category: "Trigger")]
	SCR_EScenarioFrameworkLogicOperators m_eCustomTriggerConditionLogic;
	
	[Attribute(defvalue: "1", UIWidgets.CheckBox, desc: "If you set some vehicle to be detected by the trigger, it will also search the inventory for vehicle prefabs/classes that are set", category: "Trigger")]
	protected bool m_bSearchVehicleInventory;

	[Attribute(defvalue: "1", UIWidgets.CheckBox, desc: "Activate the trigger once or everytime the activation condition is true?", category: "Trigger")]
	protected bool m_bOnce;
	
	[Attribute(defvalue: "0", desc: "Activate the trigger once it is empty", category: "Trigger")]
	protected bool m_bActivateOnEmpty;

	[Attribute(defvalue: "0", UIWidgets.Slider, desc: "Minimum players needed to activate this trigger when PLAYER Activation presence is selected", params: "0 1 0.01", precision: 2, category: "Trigger")]
	protected float m_fMinimumPlayersNeededPercentage;

	[Attribute(defvalue: "0", UIWidgets.Slider, desc: "For how long the trigger conditions must be true in order for the trigger to activate. If conditions become false, timer resets", params: "0 86400 1", category: "Trigger")]
	protected float m_fActivationCountdownTimer;

	[Attribute(defvalue: "0", UIWidgets.CheckBox, desc: "Whether or not the notification is allowed to be displayed", category: "Trigger")]
	protected bool m_bNotificationEnabled;

	[Attribute(defvalue: "{47864BB47AB0B1F4}UI/layouts/HUD/CampaignMP/CampaignMainHUD.layout", category: "Trigger")]
	protected ResourceName m_sCountdownHUD;

	[Attribute(desc: "Notification title text that will be displayed when the PLAYER Activation presence is selected", category: "Trigger")]
	protected string m_sPlayerActivationNotificationTitle;

	[Attribute(defvalue: "0", UIWidgets.CheckBox, desc: "Whether or not the audio sound is played and affected by the trigger", category: "Trigger")]
	protected bool m_bEnableAudio;

	[Attribute("", UIWidgets.EditBox, desc: "Audio sound that will be playing when countdown is active.", category: "Trigger")]
	protected string m_sCountdownAudio;
	
	protected ref array<ref SCR_ScenarioFrameworkActionBase> m_aEntityEnteredActions = {};
	protected ref array<ref SCR_ScenarioFrameworkActionBase> m_aEntityLeftActions = {};
	protected ref array<ref SCR_ScenarioFrameworkActionBase> m_aFinishedActions = {};

	protected ref set<BaseContainer> m_aPrefabContainerSet = new set<BaseContainer>();

	protected ref ScriptInvoker m_OnChange;
	protected Faction m_OwnerFaction;
	protected float m_fTempWaitTime = m_fActivationCountdownTimer;
	protected bool m_bInitSequenceDone = false;
	protected bool m_bCountdownMusicPlaying;
	protected ref array<IEntity> m_aEntitiesInside = {};
	protected ref array<IEntity> m_aPlayersInside = {};
	protected MusicManager m_MusicManager;
	protected bool m_bTriggerConditionsStatus;
	protected bool m_bTimerActive;
	protected int m_iCountInsideTrigger;

	static ref ScriptInvokerBase<ScriptInvokerTriggerUpdated> s_OnTriggerUpdated = new ScriptInvokerBase<ScriptInvokerTriggerUpdated>();
	static ref ScriptInvokerInt s_OnTriggerUpdatedPlayerNotPresent = new ScriptInvokerInt();

	//------------------------------------------------------------------------------------------------
	//! Sets Activation Presence
	void SetActivationPresence(SCR_EScenarioFrameworkTriggerActivation EActivationPresence)
	{
		m_eActivationPresence = EActivationPresence;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets specific classnames to be searched in the trigger
	void SetSpecificClassName(notnull array<string> aClassName)
	{
		foreach (ResourceName className : aClassName)
		{
			AddClassType(className.ToType());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Adds specific entity name to be searched in the trigger
	void AddSpecificEntityNameFilter(notnull array<string> aEntityName)
	{
		foreach (ResourceName entityName : aEntityName)
		{
			AddFilterName(entityName);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Sets specific prefab filters to be searched in the trigger
	void SetPrefabFilters(notnull array<ref SCR_ScenarioFrameworkPrefabFilter> aPrefabFilter)
	{
		PrefabFilter prefabFilter;
		foreach (SCR_ScenarioFrameworkPrefabFilter prefabFilterInput : aPrefabFilter)
		{
			prefabFilter = new PrefabFilter();
			prefabFilter.SetPrefab(prefabFilterInput.m_sSpecificPrefabName);
			prefabFilter.SetCheckPrefabHierarchy(prefabFilterInput.m_bIncludeChildren);
			AddPrefabFilter(prefabFilter);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Sets if trigger should also search vehicle inventory when looking for prefabs/classnames inside
	void SetSearchVehicleInventory(bool search)
	{
		m_bSearchVehicleInventory = search;
	}

	//------------------------------------------------------------------------------------------------
	//! Sets if trigger can be finished just once
	void SetOnce(bool bOnce)
	{
		m_bOnce = bOnce;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] activateOnEmpty Sets if trigger can be finished when it is empty
	void SetActivateOnEmpty(bool activateOnEmpty)
	{
		m_bActivateOnEmpty = activateOnEmpty;
	}

	//------------------------------------------------------------------------------------------------
	//! Sets if HUD notifications are enabled
	void SetNotificationEnabled(bool notificationEnabled)
	{
		m_bNotificationEnabled = notificationEnabled;
	}

	//------------------------------------------------------------------------------------------------
	//! Sets trigger conditions status
	void SetTriggerConditionsStatus(bool status)
	{
		m_bTriggerConditionsStatus = status;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets if audio features from this trigger are enabled
	void SetEnableAudio(bool enableAudio)
	{
		m_bEnableAudio = enableAudio;
	}

	//------------------------------------------------------------------------------------------------
	//! Sets minimum player percentage needed to finish this trigger
	void SetMinimumPlayersNeeded(float minimumPlayersNeededPercentage)
	{
		m_fMinimumPlayersNeededPercentage = minimumPlayersNeededPercentage;
	}

	//------------------------------------------------------------------------------------------------
	//! Sets HUD activation notification title
	void SetPlayerActivationNotificationTitle(string sTitle)
	{
		m_sPlayerActivationNotificationTitle = sTitle;
	}

	//------------------------------------------------------------------------------------------------
	//! Sets activation coundown timer
	void SetActivationCountdownTimer(float activationCountdownTimer)
	{
		m_fActivationCountdownTimer = activationCountdownTimer;
	}

	//------------------------------------------------------------------------------------------------
	//! Sets which audio can be played from activating this trigger
	void SetCountdownAudio(string sAudioName)
	{
		m_sCountdownAudio = sAudioName;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] triggerConditions Sets custom trigger conditions for trigger activation.
	void SetCustomTriggerConditions(notnull array<ref SCR_ScenarioFrameworkActivationConditionBase> triggerConditions)
	{
		m_aCustomTriggerConditions = triggerConditions;
		SCR_CustomTriggerConditions triggerCondition;
		foreach (SCR_ScenarioFrameworkActivationConditionBase condition : triggerConditions)
		{
			triggerCondition = SCR_CustomTriggerConditions.Cast(condition);
			if (!triggerCondition)
				continue;
			
			triggerCondition.Prepare(this);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets Custom Trigger Condition Logic
	void SetCustomTriggerConditionLogic(SCR_EScenarioFrameworkLogicOperators customTriggerConditionLogic)
	{
		m_eCustomTriggerConditionLogic = customTriggerConditionLogic;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] entityEnteredActions Array of actions triggered when an entity entered the trigger.
	void SetEntityEnteredActions(array<ref SCR_ScenarioFrameworkActionBase> entityEnteredActions)
	{
		foreach (SCR_ScenarioFrameworkActionBase entityEnteredAction : entityEnteredActions)
		{
			m_aEntityEnteredActions.Insert(entityEnteredAction);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] entityEnteredActions Array of actions triggered when an entity left the trigger.
	void SetEntityLefActions(array<ref SCR_ScenarioFrameworkActionBase> entityLefActions)
	{
		foreach (SCR_ScenarioFrameworkActionBase entityLefAction : entityLefActions)
		{
			m_aEntityLeftActions.Insert(entityLefAction);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] entityEnteredActions Array of actions triggered when trigger is finished.
	void SetFinishedActions(array<ref SCR_ScenarioFrameworkActionBase> finishedActions)
	{
		foreach (SCR_ScenarioFrameworkActionBase finishedAction : finishedActions)
		{
			m_aFinishedActions.Insert(finishedAction);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Sets faction that "owns" this trigger
	void SetOwnerFaction(FactionKey sFaction)
	{
		FactionManager factionManager = GetGame().GetFactionManager();
		if (factionManager)
			m_OwnerFaction = factionManager.GetFactionByKey(sFaction);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns if this is executed in a server environment
	bool IsMaster()
	{
		RplComponent comp = RplComponent.Cast(FindComponent(RplComponent));
		return comp && comp.IsMaster();
	}

	//------------------------------------------------------------------------------------------------
	//! Returns trigger faction owner
	Faction GetOwnerFaction()
	{
		return m_OwnerFaction;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns the countdown HUD resource name
	ResourceName GetCountdownHUD()
	{
		return m_sCountdownHUD;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns activation countdown timer
	float GetActivationCountdownTimer()
	{
		return m_fActivationCountdownTimer;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns activation countdown timer temporary value which is calculated by the trigger but changes over time
	float GetActivationCountdownTimerTemp()
	{
		return m_fTempWaitTime;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns minimum players needed percentage
	float GetMinimumPlayersNeededPercentage()
	{
		return m_fMinimumPlayersNeededPercentage;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns player activation notification title
	string GetPlayerActivationNotificationTitle()
	{
		return m_sPlayerActivationNotificationTitle;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns if HUD notifications are enabled
	bool GetNotificationEnabled()
	{
		return m_bNotificationEnabled;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns trigger conditions status
	bool GetTriggerConditionsStatus()
	{
		return m_bTriggerConditionsStatus;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns number of all the entities that are inside this trigger
	int GetCountInsideTrigger()
	{
		return m_iCountInsideTrigger;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns all entities that are inside this trigger
	int GetCountEntitiesInside()
	{
		GetEntitiesInside(m_aEntitiesInside);
		return m_aEntitiesInside.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! Returns number of players in game by the faction set for this trigger
	int GetPlayersCountByFaction()
	{
		int iCnt = 0;
		array<int> aPlayerIDs = {};
		SCR_PlayerController playerController;
		GetGame().GetPlayerManager().GetPlayers(aPlayerIDs);
		foreach (int iPlayerID : aPlayerIDs)
		{
			if (!m_OwnerFaction)
			{
				iCnt++;			//Faction not set == ANY faction
			}
			else
			{
				playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(iPlayerID));
				if (!playerController)
					continue;

				if (playerController.GetLocalControlledEntityFaction() == m_OwnerFaction)
					iCnt++;
			}
		}

		return iCnt;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns number of specific class that is inside of this trigger
	int GetSpecificClassCountInsideTrigger(string className, int targetCount = -1)
	{
		if (className.IsEmpty())
			return 0;

		if (m_bSearchVehicleInventory)
		{
			array<IEntity> aItemsToAdd = {};
			foreach (IEntity entity : m_aEntitiesInside)
			{
				if (!entity)
					continue;

				if (!Vehicle.Cast(entity))
					continue;

				InventoryStorageManagerComponent inventoryComponent = InventoryStorageManagerComponent.Cast(entity.FindComponent(InventoryStorageManagerComponent));
				if (!inventoryComponent)
					continue;

				array<IEntity> aItems = {};
				inventoryComponent.GetItems(aItems);
				if (!aItems.IsEmpty())
					aItemsToAdd.InsertAll(aItems);
			}

			if (!aItemsToAdd.IsEmpty())
				m_aEntitiesInside.InsertAll(aItemsToAdd);
		}

		int iCnt = 0;
		GetEntitiesInside(m_aEntitiesInside);
		foreach (IEntity entity : m_aEntitiesInside)
		{
			if (!entity)
				continue;

			if (entity.IsInherited(className.ToType()))
				iCnt++;

			if (iCnt == targetCount)
				break;
		}

		return iCnt;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns number of specific prefab that is inside of this trigger
	int GetSpecificPrefabCountInsideTrigger(BaseContainer prefabContainer, int targetCount = -1, bool includeInheritance = false)
	{
		int iCnt;
		GetEntitiesInside(m_aEntitiesInside);

		if (m_bSearchVehicleInventory)
		{
			array<IEntity> aItemsToAdd = {};
			foreach (IEntity entity : m_aEntitiesInside)
			{
				if (!entity)
					continue;

				if (!Vehicle.Cast(entity))
					continue;

				InventoryStorageManagerComponent inventoryComponent = InventoryStorageManagerComponent.Cast(entity.FindComponent(InventoryStorageManagerComponent));
				if (!inventoryComponent)
					continue;

				array<IEntity> aItems = {};
				inventoryComponent.GetItems(aItems);
				if (!aItems.IsEmpty())
					aItemsToAdd.InsertAll(aItems);
			}

			if (!aItemsToAdd.IsEmpty())
				m_aEntitiesInside.InsertAll(aItemsToAdd);
		}

		foreach (IEntity entity : m_aEntitiesInside)
		{
			if (!entity)
				continue;

			EntityPrefabData prefabData = entity.GetPrefabData();
			if (!prefabData)
				continue;

			BaseContainer container = prefabData.GetPrefab();
			if (!container)
				continue;

			if (!includeInheritance)
			{
				if (container != prefabContainer)
					continue;

				iCnt++;
			}
			else
			{
				while (container)
				{
					if (container == prefabContainer)
					{
						iCnt++;
						break;
					}

					container = container.GetAncestor();
				}
			}

			if (iCnt == targetCount)
				break;
		}

		return iCnt;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns number of characters from the selected faction that are inside this trigger
	int GetCharacterCountByFactionInsideTrigger(Faction faction, int targetCount = -1)
	{
		int iCnt = 0;
		SCR_ChimeraCharacter chimeraCharacter;
		GetEntitiesInside(m_aEntitiesInside);
		foreach (IEntity entity : m_aEntitiesInside)
		{
			if (!entity)
				continue;

			chimeraCharacter = SCR_ChimeraCharacter.Cast(entity);
			if (!chimeraCharacter)
				continue;

			if (faction && chimeraCharacter.GetFaction() != faction)
				continue;

			iCnt++;
			if (iCnt == targetCount)
				break;
		}

		return iCnt;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns number of players from the selected faction that are inside this trigger
	int GetPlayersCountByFactionInsideTrigger(Faction faction)
	{
		int iCnt = 0;
		m_aPlayersInside.Clear();
		GetEntitiesInside(m_aEntitiesInside);
		SCR_ChimeraCharacter chimeraCharacter;
		foreach (IEntity entity : m_aEntitiesInside)
		{
			if (!entity)
				continue;

			chimeraCharacter = SCR_ChimeraCharacter.Cast(entity);
			if (!chimeraCharacter)
				continue;

			if (faction && chimeraCharacter.GetFaction() != faction)
				continue;

			if (!EntityUtils.IsPlayer(entity))
				continue;

			iCnt++;
			m_aPlayersInside.Insert(entity);
		}

		return iCnt;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns all the players by the faction set for this trigger
	void GetPlayersByFactionInsideTrigger(notnull out array<IEntity> aOut)
	{
		SCR_ChimeraCharacter chimeraCharacter;
		GetEntitiesInside(m_aEntitiesInside);
		foreach (IEntity entity : m_aEntitiesInside)
		{
			if (!entity)
				continue;

			chimeraCharacter = SCR_ChimeraCharacter.Cast(entity);
			if (!chimeraCharacter)
				continue;

			//Faction not set == ANY faction
			if (!m_OwnerFaction)
			{
				if (EntityUtils.IsPlayer(entity))
					aOut.Insert(entity)
			}
			else
			{
				if (chimeraCharacter.GetFaction() != m_OwnerFaction)
					continue;

				if (EntityUtils.IsPlayer(entity))
					aOut.Insert(entity);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Returns all the players in the game
	void GetPlayersByFaction(notnull out array<IEntity> aOut)
	{
		array<int> aPlayerIDs = {};
		SCR_PlayerController playerController;
		GetGame().GetPlayerManager().GetPlayers(aPlayerIDs);
		foreach (int iPlayerID : aPlayerIDs)
		{
			playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(iPlayerID));
			if (!playerController)
				continue;

			if (playerController.GetLocalControlledEntityFaction() == m_OwnerFaction)
				aOut.Insert(playerController.GetLocalMainEntity());
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Override this method in inherited class to define a new filter.
	override bool ScriptedEntityFilterForQuery(IEntity ent)
	{
		if (m_eActivationPresence == SCR_EScenarioFrameworkTriggerActivation.PLAYER || m_eActivationPresence == SCR_EScenarioFrameworkTriggerActivation.ANY_CHARACTER)
		{
			SCR_ChimeraCharacter chimeraCharacter = SCR_ChimeraCharacter.Cast(ent);
			if (!chimeraCharacter)
				return false;

			if (m_OwnerFaction && chimeraCharacter.GetFaction() != m_OwnerFaction)
				return false;

			if (!IsAlive(ent))
				return false;

			if (m_eActivationPresence == SCR_EScenarioFrameworkTriggerActivation.PLAYER)
				return EntityUtils.IsPlayer(ent);

			return true;
		}

		//In case of vehicle, we first need to check if it is alive and then check the faction
		Vehicle vehicle = Vehicle.Cast(ent);
		if (vehicle)
		{
			if (!IsAlive(ent))
				return false;

			if (!m_OwnerFaction)
				return true;

			return vehicle.GetFaction() == m_OwnerFaction;
		}

		if (!m_OwnerFaction)
			return true;

		FactionAffiliationComponent factionAffiliation = FactionAffiliationComponent.Cast(ent.FindComponent(FactionAffiliationComponent));
		if (!factionAffiliation)
			return true;

		return factionAffiliation.GetAffiliatedFaction() == m_OwnerFaction;
	}

	//------------------------------------------------------------------------------------------------
	//! Finishes trigger
	void FinishTrigger(IEntity ent)
	{
		if (m_bEnableAudio)
			StopMusic(m_sCountdownAudio);

		if (m_bOnce)
			EnablePeriodicQueries(false);

		m_OnActivate.Invoke(ent);
		OnChange(ent);
		
		foreach (SCR_ScenarioFrameworkActionBase finishedActions : m_aFinishedActions)
		{
			finishedActions.OnActivate(this);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Checks activation presesence conditions
	void ActivationPresenceConditions()
	{
		m_bTriggerConditionsStatus = false;
		if (m_eActivationPresence == SCR_EScenarioFrameworkTriggerActivation.PLAYER)
		{
			m_iCountInsideTrigger = GetPlayersCountByFactionInsideTrigger(m_OwnerFaction);
			float minPlayersNeeded = Math.Ceil(GetPlayersCountByFaction() * m_fMinimumPlayersNeededPercentage);
			if (m_iCountInsideTrigger >= minPlayersNeeded && m_iCountInsideTrigger > 0)
				m_bTriggerConditionsStatus = true;

			return;
		}
		
		m_iCountInsideTrigger = GetEntitiesInside(m_aEntitiesInside);
		m_bTriggerConditionsStatus = true;
	}

	//------------------------------------------------------------------------------------------------
	//! This method handles custom conditions
	protected void CustomTriggerConditions()
	{
		if (m_aCustomTriggerConditions.IsEmpty())
			return;
		
		SetTriggerConditionsStatus(SCR_ScenarioFrameworkActivationConditionBase.EvaluateEmptyOrConditions(m_eCustomTriggerConditionLogic, m_aCustomTriggerConditions, this));
	}

	//------------------------------------------------------------------------------------------------
	//! Handles players inside the trigger or those who left it
	protected void HandleNetworkComponentForPlayersInside(IEntity ent)
	{
		if (m_aPlayersInside.IsEmpty())
			m_iCountInsideTrigger = GetPlayersCountByFactionInsideTrigger(m_OwnerFaction);

		foreach (IEntity entity : m_aPlayersInside)
		{
			ProcessPlayerNetworkComponent(entity, false);
		}

		//This handles Network component for players that left the trigger
		if (ent)
			ProcessPlayerNetworkComponent(ent, true);
	}

	//------------------------------------------------------------------------------------------------
	//! Processes the entity and its Network component to replicate only for each player inside the trigger
	protected void ProcessPlayerNetworkComponent(IEntity entity, bool leftTrigger = false)
	{
		PlayerManager playerManager = GetGame().GetPlayerManager();
		int playerId = playerManager.GetPlayerIdFromControlledEntity(entity);
		SCR_PlayerController playerController = SCR_PlayerController.Cast(playerManager.GetPlayerController(playerId));
		if (!playerController)
			return;

		SCR_ScenarioFrameworkTriggerNetworkComponent triggerNetwork = SCR_ScenarioFrameworkTriggerNetworkComponent.Cast(playerController.FindComponent(SCR_ScenarioFrameworkTriggerNetworkComponent));
		if (!triggerNetwork)
			return;

		triggerNetwork.ReplicateTriggerState(this, leftTrigger);
	}

	//------------------------------------------------------------------------------------------------
	//! Handles if timer should be ticking or not
	protected void HandleTimer()
	{
		if (!m_bTimerActive)
		{
			m_bTimerActive = true;
			m_fTempWaitTime = m_fActivationCountdownTimer;
			// Call later in this case will be more efficient than handling it via EOnFrame
			SCR_ScenarioFrameworkSystem.GetCallQueuePausable().CallLater(UpdateTimer, 1000, true);
		}

		if (GetCountInsideTrigger() == 0)
		{
			SCR_ScenarioFrameworkSystem.GetCallQueuePausable().Remove(UpdateTimer);
			m_fTempWaitTime = m_fActivationCountdownTimer;
			m_bTimerActive = false;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Updates the timer value and passes info to HUD
	protected void UpdateTimer()
	{
		if (!m_bTriggerConditionsStatus)
			m_fTempWaitTime = m_fActivationCountdownTimer;
		else
			m_fTempWaitTime--;

		if (!m_bNotificationEnabled)
			return;

		HandleNetworkComponentForPlayersInside(null);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] ent Entity reference for the object being activated.
	override protected event void OnActivate(IEntity ent)
	{
		foreach (SCR_ScenarioFrameworkActionBase enteredActions : m_aEntityEnteredActions)
		{
			enteredActions.OnActivate(this);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] bIsEmpty Checks if query is empty, if true, checks if it should finish it and returns without further action processing the trigger.
	override event protected void OnQueryFinished(bool bIsEmpty)
	{
		super.OnQueryFinished(bIsEmpty);

		if (bIsEmpty)
		{
			if (m_bActivateOnEmpty && m_bInitSequenceDone)
				FinishTrigger(this);
			
			return;
		}
		
		if (m_bActivateOnEmpty)
			return;

		if (!IsMaster())
			return;

		if (!m_bInitSequenceDone)
			return;

		ActivationPresenceConditions();
		
		if (m_bTriggerConditionsStatus)
			CustomTriggerConditions();
		
		HandleTimer();

		if (m_bEnableAudio)
			HandleAudio();

		if (m_bTriggerConditionsStatus && m_fTempWaitTime <= 0)
			FinishTrigger(this);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] ent Entity reference representing the object being deactivated.
	override protected event void OnDeactivate(IEntity ent)
	{
		//This method is triggered when the trigger is deactivated but the said entity is still inside.
		//We need to perform this method after the entity leaves the trigger.
		SCR_ScenarioFrameworkSystem.GetCallQueuePausable().CallLater(OnDeactivateCalledLater, 100, false, ent);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] ent The ent represents an entity inside the method, which is removed from the list of players inside when the method is called.
	void OnDeactivateCalledLater(IEntity ent)
	{
		if (!m_bInitSequenceDone)
			return;

		m_aPlayersInside.RemoveItem(ent);

		ActivationPresenceConditions();
		CustomTriggerConditions();
		HandleTimer();

		if (m_bNotificationEnabled)
			HandleNetworkComponentForPlayersInside(ent);

		if (m_bEnableAudio)
			HandleAudio();

		m_OnDeactivate.Invoke();
		OnChange(ent);
		
		foreach (SCR_ScenarioFrameworkActionBase leftActions : m_aEntityLeftActions)
		{
			leftActions.OnActivate(this);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Checks if trigger conditions are met, plays countdown music if true, stops if false.
	void HandleAudio()
	{
		if (m_bTriggerConditionsStatus)
		{
			if (!m_bCountdownMusicPlaying)
				PlayMusic(m_sCountdownAudio);
		}
		else
		{
			StopMusic(m_sCountdownAudio);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Plays specified audio track, sets countdown music playing flag, and calls RPC.
	//! \param[in] sAudio is the name of the music file to play.
	void PlayMusic(string sAudio)
	{

		if (!m_MusicManager)
		{
			ChimeraWorld world = GetGame().GetWorld();
			if (world)
				m_MusicManager = world.GetMusicManager();
		}

		if (!m_MusicManager)
			return;

		m_bCountdownMusicPlaying = true;
		m_MusicManager.Play(sAudio);

		Rpc(RpcDo_PlayMusic, sAudio);
	}

	//------------------------------------------------------------------------------------------------
	//! Plays specified audio in music manager if available.
	//! \param[in] sAudio is the name of the music file to play.
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_PlayMusic(string sAudio)
	{
		if (!m_MusicManager)
		{
			ChimeraWorld world = GetGame().GetWorld();
			if (world)
				m_MusicManager = world.GetMusicManager();
		}

		if (!m_MusicManager)
			return;

		m_bCountdownMusicPlaying = true;
		m_MusicManager.Play(sAudio);
	}

	//------------------------------------------------------------------------------------------------
	//! Stops music with given audio name, also stops countdown music if playing.
	//! \param[in] sAudio is the name of the music track to stop playing.
	void StopMusic(string sAudio)
	{
		if (!m_MusicManager)
		{
			ChimeraWorld world = GetGame().GetWorld();
			if (world)
				m_MusicManager = world.GetMusicManager();
		}

		if (!m_MusicManager)
			return;

		m_bCountdownMusicPlaying = false;
		m_MusicManager.Stop(sAudio);

		Rpc(RpcDo_StopMusic, sAudio);
	}

	//------------------------------------------------------------------------------------------------
	//! Stops music with given audio name.
	//! \param[in] sAudio is the name of the music track to stop playing.
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_StopMusic(string sAudio)
	{
		if (!m_MusicManager)
		{
			ChimeraWorld world = GetGame().GetWorld();
			if (world)
				m_MusicManager = world.GetMusicManager();
		}

		if (!m_MusicManager)
			return;

		m_bCountdownMusicPlaying = false;
		m_MusicManager.Stop(sAudio);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] ent Entity ent is an input parameter representing an entity in the game world that triggers the method's action when its state changes
	void OnChange(IEntity ent)
	{
		if (m_OnChange)
		{
			SCR_ScenarioFrameworkParam<IEntity> param = new SCR_ScenarioFrameworkParam<IEntity>(ent);
			m_OnChange.Invoke(param);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \return a ScriptInvoker object for handling changes in the script.
	ScriptInvoker GetOnChange()
	{
		if (!m_OnChange)
			m_OnChange = new ScriptInvoker();

		return m_OnChange;
	}

	//------------------------------------------------------------------------------------------------
	//! Sets Init sequence as done or not
	void SetInitSequenceDone(bool init)
	{
		m_bInitSequenceDone = init;
	}

	//------------------------------------------------------------------------------------------------
	//! Initializes the trigger
	override protected void EOnInit(IEntity owner)
	{
		ChimeraWorld world = GetGame().GetWorld();
		if (world)
			m_MusicManager = world.GetMusicManager();
		
		SCR_CustomTriggerConditions triggerCondition;
		foreach (SCR_ScenarioFrameworkActivationConditionBase condition : m_aCustomTriggerConditions)
		{
			triggerCondition = SCR_CustomTriggerConditions.Cast(condition);
			if (!triggerCondition)
				continue;
			
			triggerCondition.Prepare(this);
		}

		PrefabFilter prefabFilter;
		foreach (SCR_ScenarioFrameworkPrefabFilter prefabFilterInput : m_aPrefabFilter)
		{
			prefabFilter = new PrefabFilter();
			prefabFilter.SetPrefab(prefabFilterInput.m_sSpecificPrefabName);
			prefabFilter.SetCheckPrefabHierarchy(prefabFilterInput.m_bIncludeChildren);
			AddPrefabFilter(prefabFilter);
		}

		foreach (ResourceName className : m_aSpecificClassNames)
		{
			AddClassType(className.ToType());
		}
		
		foreach (string entityName : m_aSpecificEntityNames)
		{
			AddFilterName(entityName);
		}
		
		SetOwnerFaction(m_sOwnerFactionKey);
		
		m_bInitSequenceDone = true;
	}
}
