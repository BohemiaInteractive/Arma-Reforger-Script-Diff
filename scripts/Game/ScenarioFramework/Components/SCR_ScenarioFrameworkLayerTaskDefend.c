[EntityEditorProps(category: "GameScripted/ScenarioFramework/Layer", description: "")]
class SCR_ScenarioFrameworkLayerTaskDefendClass : SCR_ScenarioFrameworkLayerTaskClass
{
}

class SCR_ScenarioFrameworkLayerTaskDefend : SCR_ScenarioFrameworkLayerTask
{
	[Attribute(desc: "Will use trigger that is named for Defend params calculations", category: "Task")]
	protected string m_sTriggerName;

	[Attribute(uiwidget: UIWidgets.LocaleEditBox, desc: "Text that will be displayed above the countdown number", category: "Defend params")]
	protected LocalizedString m_sCountdownTitleText;

	[Attribute(defvalue: "-1", UIWidgets.Slider, desc: "For how long you have to Defend the objective of the task. Value -1 is for indefinitely.", params: "-1 86400 1", category: "Defend params")]
	protected float m_fDefendTime;

	[Attribute(defvalue: "1", desc: "When enabled, it will display the text and how much time remains for the Task Defend", category: "Defend params")]
	protected bool m_bDisplayCountdownHUD;

	[Attribute(defvalue: "{47864BB47AB0B1F4}UI/layouts/HUD/CampaignMP/CampaignMainHUD.layout", category: "Defend params")]
	protected ResourceName m_sCountdownHUD;

	[Attribute(category: "Defend params")]
	protected ref array<ref SCR_ScenarioFrameworkTaskDefendFactionSettings> m_aFactionSettings;

	[Attribute(uiwidget: UIWidgets.Slider, desc: "When compared to the number of attackers, minimum of how much of the characters present in the task area must be from defending side to successfully complete the objective on evaluation", params: "0 1 0.01", precision: 2, category: "Defend params")]
	protected float	m_fMinDefenderPercentageRatio;

	[Attribute(uiwidget: UIWidgets.EditBox, desc: "Layer containing attacker forces. Can be more layers, but these layers must include only AI units/groups and nothing else.", category: "Defend params")]
	protected ref array<string>	m_aAttackerLayerNames;

	[Attribute(desc: "When enabled, it will can finish the task earlier than the countdown when all attackers are eliminated", category: "Defend params")]
	protected bool m_bEarlierEvaluation;

	[Attribute(desc: "When enabled, evaluation will be delayed and defenders will need to eliminate all attackers in order for the task to be succesfully completed", category: "Defend params")]
	protected bool m_bDelayedEvaluation;

	[Attribute(desc: "When enabled, it will display the text to inform players that they have to eliminate all attacker units", category: "Defend params")]
	protected bool m_bDisplayDelayedEvaluationText;
	
	[Attribute(uiwidget: UIWidgets.LocaleEditBox, desc: "Text that will be displayed to inform players that they have to eliminate all attacker units", category: "Defend params")]
	protected LocalizedString m_sDelayedEvaluationText;

	protected SCR_CharacterTriggerEntity m_CharacterTriggerEntity;

	protected float m_fTempCountdown = m_fDefendTime;

	protected string m_sFormattedCountdownTitle = string.Format(WidgetManager.Translate("<color rgba=\"226, 168, 80, 255\">%1</color>", m_sCountdownTitleText));
	protected string m_sFormattedDelayedEvaluationText = string.Format(WidgetManager.Translate("<color rgba=\"226, 168, 80, 255\">%1</color>", m_sDelayedEvaluationText));

	protected ref array<SCR_ScenarioFrameworkLayerBase> m_aAttackerLayer = {};

	protected float m_fTempTimeSlice;
	protected bool m_bTaskEvaluated;
	protected bool m_bEvaluationSet;
	protected WorldTimestamp m_fEvaluateTimeStart;
	protected WorldTimestamp m_fEvaluateTimeEnd;

	protected Widget m_wRoot;
	protected Widget m_wInfoOverlay;
	protected Widget m_wCountdownOverlay;
	protected RichTextWidget m_wCountdown;
	protected RichTextWidget m_wFlavour;

	//------------------------------------------------------------------------------------------------
	//! \return Represents time in seconds for defending an object or position.
	float GetDefendTime()
	{
		return m_fDefendTime;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] time Defend time sets duration for defending.
	void SetDefendTime(float time)
	{
		m_fDefendTime = time;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Character trigger entity reference for character interaction.
	SCR_CharacterTriggerEntity GetCharacterTriggerEntity()
	{
		return m_CharacterTriggerEntity;
	}

	//------------------------------------------------------------------------------------------------
	//! Finds character trigger entity by name.
	void FindCharacterTriggerEntity()
	{
		IEntity foundEntity = GetGame().GetWorld().FindEntityByName(m_sTriggerName);
		if (!foundEntity)
			return;

		SCR_ScenarioFrameworkSlotTrigger slotTrigger = SCR_ScenarioFrameworkSlotTrigger.Cast(foundEntity.FindComponent(SCR_ScenarioFrameworkSlotTrigger));
		if (!slotTrigger)
			return;

		SCR_CharacterTriggerEntity trigger = SCR_CharacterTriggerEntity.Cast(slotTrigger.GetSpawnedEntity());
		if (!trigger)
			return;

		m_CharacterTriggerEntity = trigger;
	}

	//------------------------------------------------------------------------------------------------
	//! Counts defenders and attackers inside trigger entity.
	//! \param[out] defenderCount DefenderCount represents the total number of defenders inside the trigger area.
	//! \param[out] attackerCount Represents the total number of attackers inside the trigger entity's area.
	protected void CountAttackerDefenderNumbers(out int defenderCount, out int attackerCount)
	{
		for (int i = 0, count = m_aFactionSettings.Count(); i < count; i++)
		{
			SCR_ScenarioFrameworkTaskDefendDefendingFaction defender = SCR_ScenarioFrameworkTaskDefendDefendingFaction.Cast(m_aFactionSettings[i]);
			if (defender)
			{
				if (defender.GetCountOnlyPlayers())
					defenderCount += m_CharacterTriggerEntity.GetPlayersCountByFactionInsideTrigger(defender.GetFaction());
				else
					defenderCount += m_CharacterTriggerEntity.GetCharacterCountByFactionInsideTrigger(defender.GetFaction());
			}

			SCR_ScenarioFrameworkTaskDefendAttackingFaction attacker = SCR_ScenarioFrameworkTaskDefendAttackingFaction.Cast(m_aFactionSettings[i]);
			if (attacker)
			{
				if (attacker.GetCountOnlyPlayers())
					attackerCount += m_CharacterTriggerEntity.GetPlayersCountByFactionInsideTrigger(attacker.GetFaction());
				else
					attackerCount += m_CharacterTriggerEntity.GetCharacterCountByFactionInsideTrigger(attacker.GetFaction());
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Evaluates task status
	void EvaluateStatus()
	{
		m_bTaskEvaluated = true;

		if (!m_Task)
		{
			SCR_ScenarioFrameworkTask taskToFind;
			array<SCR_Task> activeTasks = {};
			m_TaskSystem.GetTasks(activeTasks);

			foreach (SCR_Task task : activeTasks)
			{
				taskToFind = SCR_ScenarioFrameworkTask.Cast(task);
				if (!taskToFind)
					continue;

				if (taskToFind.GetLayerTask() == this)
				{
					m_Task = taskToFind;
					break;
				}
			}
		}

		if (!m_Task)
		{
			RemovePeriodicUpdates();
			return;
		}

		if (!m_CharacterTriggerEntity && !m_sTriggerName.IsEmpty())
			FindCharacterTriggerEntity();

		if (m_CharacterTriggerEntity)
		{
			int defenderCount = 0;
			int attackerCount = 0;
			CountAttackerDefenderNumbers(defenderCount, attackerCount);

			if (m_fMinDefenderPercentageRatio == 0 || attackerCount == 0)
			{
				ProcessLayerTaskState(SCR_ETaskState.COMPLETED);
				RemovePeriodicUpdates();
				return;
			}

			float defenderRatioEval = defenderCount / attackerCount;
			if (defenderRatioEval < m_fMinDefenderPercentageRatio)
			{
				ProcessLayerTaskState(SCR_ETaskState.FAILED);
				RemovePeriodicUpdates();
				return;
			}
		}

		ProcessLayerTaskState(SCR_ETaskState.COMPLETED);
		RemovePeriodicUpdates();
	}

	//------------------------------------------------------------------------------------------------
	//! Sets up attacker layers from given names in the world.
	void SetupAttackerLayer()
	{
		SCR_ScenarioFrameworkLayerBase attackerLayer;
		IEntity attackerLayerEntity;
		foreach (string layerName : m_aAttackerLayerNames)
		{
			attackerLayerEntity = GetGame().GetWorld().FindEntityByName(layerName);
			if (!attackerLayerEntity)
				continue;

			attackerLayer = SCR_ScenarioFrameworkLayerBase.Cast(attackerLayerEntity.FindComponent(SCR_ScenarioFrameworkLayerBase));
			if (!attackerLayer)
				continue;

			m_aAttackerLayer.Insert(attackerLayer);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Sets up evaluation time, triggers attacker layer, and checks for character trigger entities in scenario layers.
	void SetupEvaluation()
	{
		if (m_fTempCountdown > 0)
		{
			ChimeraWorld world = GetOwner().GetWorld();
			m_fEvaluateTimeStart = world.GetServerTimestamp();
			m_fEvaluateTimeEnd = m_fEvaluateTimeStart.PlusSeconds(m_fTempCountdown);
			m_bEvaluationSet = true;
		}

		if (m_bEarlierEvaluation || m_bDelayedEvaluation)
			SetupAttackerLayer();

		//If trigger is already set via the Trigger Name attribute, we don't need to search for it
		if (m_CharacterTriggerEntity)
			return;

		foreach (SCR_ScenarioFrameworkLayerBase layerBase : m_aChildren)
		{
			if (!layerBase)
				continue;

			SCR_CharacterTriggerEntity charTrigger;
			array<IEntity> spawnedEntities = layerBase.GetSpawnedEntities();
			foreach (IEntity spawnedEntity : spawnedEntities)
			{
				charTrigger = SCR_CharacterTriggerEntity.Cast(spawnedEntity);
				if (charTrigger)
					m_CharacterTriggerEntity = charTrigger; // TODO: break?
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Restores default settings, clears attacker layer, nullifies entities, and calls superclass method.
	//! \param[in] includeChildren Restores default settings, optionally including children entities.
	//! \param[in] reinitAfterRestoration Resets object state after restoration, optionally reinitializing it afterwards.
	//! \param[in] affectRandomization Affects randomization parameters during restoration process.
	override void RestoreToDefault(bool includeChildren = false, bool reinitAfterRestoration = false, bool affectRandomization = true)
	{
		m_CharacterTriggerEntity = null;
		m_fTempCountdown = m_fDefendTime;
		m_fTempTimeSlice = 0;
		m_bTaskEvaluated = false;
		m_bEvaluationSet = false;
		m_wRoot = null;
		m_wInfoOverlay = null;
		m_wCountdownOverlay = null;
		m_wCountdown = null;
		m_wFlavour = null;
		
		m_aAttackerLayer.Clear();
		
		super.RestoreToDefault(includeChildren, reinitAfterRestoration, affectRandomization);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Dynamic Despawn is disabled for Task Defend
	//! \param[in] layer Layer represents the scenario framework layer being dynamically despawned in the method.
	override void DynamicDespawn(SCR_ScenarioFrameworkLayerBase layer)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! Initializes scenario, finds trigger entity, initializes HUD, sets up evaluation, and initializes post-init for owner.
	//! \param[in] layer that is to be used for this method
	override void AfterAllChildrenSpawned(SCR_ScenarioFrameworkLayerBase layer)
	{
		super.AfterAllChildrenSpawned(layer);
		
		if (!m_sTriggerName.IsEmpty())
			FindCharacterTriggerEntity();

		InitHUD();
		SetupEvaluation();
		OnPostInit(GetOwner());
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] owner The owner represents the entity being initialized in the method.
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		if (m_bInitiated)
		{
			SetEventMask(owner, EntityEvent.INIT | EntityEvent.FRAME);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Initializes HUD layout, sets visibility of widgets, sets countdown time, and sets flavor text.
	void InitHUD()
	{
		SCR_HUDManagerComponent hudManager = GetGame().GetHUDManager();
		if (!hudManager)
			return;
		
		m_wRoot = hudManager.CreateLayout(m_sCountdownHUD, EHudLayers.MEDIUM, 0);
		if (!m_wRoot)
			return;
		
		m_wInfoOverlay = m_wRoot.FindAnyWidget("Info");
		m_wCountdownOverlay = m_wRoot.FindAnyWidget("Countdown");
		ImageWidget leftFlag = ImageWidget.Cast(m_wRoot.FindAnyWidget("FlagSideBlue"));
		ImageWidget rightFlag = ImageWidget.Cast(m_wRoot.FindAnyWidget("FlagSideRed"));
		RichTextWidget leftScore = RichTextWidget.Cast(m_wRoot.FindAnyWidget("ScoreBlue"));
		RichTextWidget rightScore = RichTextWidget.Cast(m_wRoot.FindAnyWidget("ScoreRed"));
		RichTextWidget winScore = RichTextWidget.Cast(m_wRoot.FindAnyWidget("TargetScore"));
		m_wCountdown = RichTextWidget.Cast(m_wRoot.FindAnyWidget("CountdownWin"));
		m_wFlavour = RichTextWidget.Cast(m_wRoot.FindAnyWidget("FlavourText"));
		ImageWidget winScoreSideLeft = ImageWidget.Cast(m_wRoot.FindAnyWidget("ObjectiveLeft"));
		ImageWidget winScoreSideRight = ImageWidget.Cast(m_wRoot.FindAnyWidget("ObjectiveRight"));

		m_wInfoOverlay.SetVisible(false);
		leftFlag.SetVisible(false);
		rightFlag.SetVisible(false);
		leftScore.SetVisible(false);
		rightScore.SetVisible(false);
		winScore.SetVisible(false);
		winScoreSideLeft.SetVisible(false);
		winScoreSideRight.SetVisible(false);

		string shownTime = SCR_FormatHelper.GetTimeFormatting(m_fTempCountdown, ETimeFormatParam.DAYS | ETimeFormatParam.HOURS, ETimeFormatParam.DAYS | ETimeFormatParam.HOURS | ETimeFormatParam.MINUTES);
		m_wCountdown.SetText(shownTime);

		m_wFlavour.SetText(m_sFormattedCountdownTitle);

		if (!m_bDisplayCountdownHUD)
			m_wRoot.SetVisible(false);
	}

	//------------------------------------------------------------------------------------------------
	//! Updates HUD based on task state, displays countdown or delayed evaluation text, hides HUD if task is cancelled
	void UpdateHUD()
	{
		m_fTempTimeSlice = 0;
		m_fTempCountdown--;
		
		int taskID = -1;
		if (m_Task)
			taskID = m_Task.GetTaskID();
		
		Rpc(RpcDo_UpdateHUD, m_fTempCountdown, taskID);
		
		if (!m_wRoot)
			return;

		if (m_fTempCountdown < 0 || !m_Task || !m_bDisplayCountdownHUD)
		{
			if (m_bDelayedEvaluation && m_bDisplayDelayedEvaluationText && !m_bTaskEvaluated)
			{
				m_wFlavour.SetText(m_sFormattedDelayedEvaluationText);
				m_wRoot.SetVisible(true);
				m_wCountdownOverlay.SetVisible(false);
				return;
			}

			m_wRoot.SetVisible(false);
			return;
		}

		if (m_Task.GetTaskState() == SCR_ETaskState.FAILED || m_Task.GetTaskState() == SCR_ETaskState.COMPLETED)
		{
			m_wRoot.SetVisible(false);
			return;
		}

		string shownTime = SCR_FormatHelper.GetTimeFormatting(m_fTempCountdown, ETimeFormatParam.DAYS | ETimeFormatParam.HOURS, ETimeFormatParam.DAYS | ETimeFormatParam.HOURS | ETimeFormatParam.MINUTES);
		m_wCountdown.SetText(shownTime);

		m_wFlavour.SetText(m_sFormattedCountdownTitle);
		m_wRoot.SetVisible(true);
	}
	
	//! Updates HUD with countdown or delayed evaluation text based on task state, displays countdown or title, hides or
	//! \param[in] countdown Countdown displays remaining time for active task in HUD.
	//! \param[in] taskID represents the identifier for the current task in the scenario, used for updating HUD based on its state.
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_UpdateHUD(float countdown, int taskID)
	{
		m_fTempCountdown = countdown;
		
		if (!m_wRoot)
		{
			InitHUD();
			if (!m_wRoot)
				return;
		}

		if (m_fTempCountdown < 0 || !m_Task || !m_bDisplayCountdownHUD)
		{
			if (m_bDelayedEvaluation && m_bDisplayDelayedEvaluationText && !m_bTaskEvaluated)
			{
				m_wFlavour.SetText(m_sFormattedDelayedEvaluationText);
				m_wRoot.SetVisible(true);
				m_wCountdownOverlay.SetVisible(false);
				return;
			}

			m_wRoot.SetVisible(false);
			return;
		}

		if (m_Task.GetTaskState() == SCR_ETaskState.FAILED || m_Task.GetTaskState() == SCR_ETaskState.COMPLETED)
		{
			m_wRoot.SetVisible(false);
			return;
		}

		string shownTime = SCR_FormatHelper.GetTimeFormatting(m_fTempCountdown, ETimeFormatParam.DAYS | ETimeFormatParam.HOURS, ETimeFormatParam.DAYS | ETimeFormatParam.HOURS | ETimeFormatParam.MINUTES);
		m_wCountdown.SetText(shownTime);

		m_wFlavour.SetText(m_sFormattedCountdownTitle);
		m_wRoot.SetVisible(true);
	}

	//------------------------------------------------------------------------------------------------
	//! Checks if attacker layers have repeated spawns, then iterates through spawned entities, checking if any character or group is alive or not
	void CheckAttackerLayers()
	{
		m_fTempTimeSlice = 0;
		foreach (SCR_ScenarioFrameworkLayerBase attackerLayer : m_aAttackerLayer)
		{
			if (attackerLayer.GetEnableRepeatedSpawn() && attackerLayer.GetRepeatedSpawnNumber() > 1)
				return;

			array<IEntity> spawnedEntities = attackerLayer.GetSpawnedEntities();
			SCR_ChimeraCharacter character;
			DamageManagerComponent damageManager;
			foreach (IEntity entity : spawnedEntities)
			{
				character = SCR_ChimeraCharacter.Cast(entity);
				if (!character)
				{
					SCR_AIGroup group = SCR_AIGroup.Cast(entity);
					if (group)
						return;
					else
						continue;
				}

				damageManager = character.GetDamageManager();
				if (!damageManager.IsDestroyed())
					return;
			}
		}

		EvaluateStatus();
	}

	//------------------------------------------------------------------------------------------------
	//! Removes periodic updates and clears debug shapes during runtime if not shown.
	void RemovePeriodicUpdates()
	{
		if (!m_bShowDebugShapesDuringRuntime)
			GetOwner().ClearFlags(EntityFlags.ACTIVE);

		UpdateHUD();
	}

	//------------------------------------------------------------------------------------------------
	//! Updates HUD, checks attacker layers based on time slice, evaluates status if not delayed evaluation.
	//! \param[in] owner The owner represents the entity (object) in the game world that this script is attached to, controlling its behavior.
	//! \param[in] timeSlice represents the time interval for each frame update in the method.
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (!m_bInitiated)
			return;
		
		super.EOnFrame(owner, timeSlice);

		m_fTempTimeSlice += timeSlice;
		if (m_fTempTimeSlice >= 1 && m_fTempCountdown >= 0)
		{
			UpdateHUD();
			if (m_bEarlierEvaluation)
				CheckAttackerLayers();
		}
		else if (m_bEarlierEvaluation && m_fTempTimeSlice >= 5)
		{
			CheckAttackerLayers();
		}

		ChimeraWorld world = owner.GetWorld();
		if (!m_bTaskEvaluated && m_bEvaluationSet && world.GetServerTimestamp().GreaterEqual(m_fEvaluateTimeEnd))
		{
			if (m_bDelayedEvaluation && m_fTempTimeSlice >= 5)
				CheckAttackerLayers();

			if (!m_bDelayedEvaluation)
				EvaluateStatus();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	float GetSecondsRemaining()
	{
		return m_fTempCountdown;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetSecondsRemaining(float seconds)
	{	
		m_fTempCountdown = seconds;
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkTaskDefendFactionSettings
{
	[Attribute(defvalue: "", UIWidgets.EditBox, desc: "Faction Name", category: "")]
	protected FactionKey m_sFactionKey;

	[Attribute(defvalue: "0", UIWidgets.EditBox, desc: "When disabled, all units from this faction will be counted with for other Task Defend conditions", category: "")]
	protected bool m_bCountOnlyPlayers;

	//------------------------------------------------------------------------------------------------
	//! \return the Faction object associated with the provided key, or null if not found.
	Faction GetFaction()
	{
		FactionManager factionManager = GetGame().GetFactionManager();
		if (factionManager)
			return factionManager.GetFactionByKey(m_sFactionKey);

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] factionKey Faction key is an identifier for a faction in the game
	void SetFactionKey(FactionKey factionKey)
	{
		m_sFactionKey = factionKey;
	}

	//------------------------------------------------------------------------------------------------
	//! \return Represents whether only players should be counted.
	bool GetCountOnlyPlayers()
	{
		return m_bCountOnlyPlayers;
	}
}

//------------------------------------------------------------------------------------------------
class SCR_ScenarioFrameworkTaskDefendDefendingFactionTitle : BaseContainerCustomTitle
{
	//------------------------------------------------------------------------------------------------
	//! Defines custom title for defending faction.
	//! \param[in] source represents the container holding data.
	//! \param[out] title Defending faction title represents the side being defended.
	//! \return the custom title for the defending faction.
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		title = "Defending faction";
		return true;
	}
}

//------------------------------------------------------------------------------------------------
class SCR_ScenarioFrameworkTaskDefendAttackingFactionTitle : BaseContainerCustomTitle
{
	//------------------------------------------------------------------------------------------------
	//! Sets custom title for attacking faction.
	//! \param[in] source Source represents the attacking faction's data container.
	//! \param[out] title Faction title for attacking side.
	//! \return the custom title for the attacking faction.
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		title = "Attacking faction";
		return true;
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ScenarioFrameworkTaskDefendDefendingFactionTitle()]
class SCR_ScenarioFrameworkTaskDefendDefendingFaction : SCR_ScenarioFrameworkTaskDefendFactionSettings
{
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ScenarioFrameworkTaskDefendAttackingFactionTitle()]
class SCR_ScenarioFrameworkTaskDefendAttackingFaction : SCR_ScenarioFrameworkTaskDefendFactionSettings
{
}
