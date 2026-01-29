class SCR_SeizingComponentClass : SCR_MilitaryBaseLogicComponentClass
{
	[Attribute("{59A6F1EBC6C64F79}Prefabs/Logic/SeizingTrigger.et", UIWidgets.ResourceNamePicker, "", "et")]
	protected ResourceName m_sTriggerPrefab;

	//------------------------------------------------------------------------------------------------
	//! \return
	ResourceName GetTriggerPrefab()
	{
		return m_sTriggerPrefab;
	}
}

void OnTimerChangeFn(WorldTimestamp newStart, WorldTimestamp newEnd);
typedef func OnTimerChangeFn;
typedef ScriptInvokerBase<OnTimerChangeFn> OnTimerChangeInvoker;

class SCR_SeizingComponent : SCR_MilitaryBaseLogicComponent
{
	[Attribute("100")]
	protected int m_iRadius;

	[Attribute("7", desc: "Units in a vehicle (most probably aircraft) above this altitude will be ignored.")]
	protected int m_iMaximumAltitude;

	[Attribute("10")]
	protected float m_fMaximumSeizingTime;

	[Attribute("6")]
	protected float m_fMinimumSeizingTime;

	[Attribute("5", "How many characters need to be seizing at once to achieve the minimum seizing time.")]
	protected int m_iMaximumSeizingCharacters;

	[Attribute("0", desc: "How long after respawn is player able to start seizing or defending.")]
	protected float m_fRespawnCooldownPeriod;

	[Attribute("0", desc: "When checked, the seizing timer will decrease gradually when the seizing is interrupted.")]
	protected bool m_bGradualTimerReset;

	[Attribute("0", desc: "Allow seizing for playable factions only.")]
	protected bool m_bIgnoreNonPlayableAttackers;

	[Attribute("0", desc: "Allow defending for playable factions only.")]
	protected bool m_bIgnoreNonPlayableDefenders;
	
	[Attribute("0", desc: "If enabled, at least one player has to be present in the capture area to progress.")]
	protected bool m_bCapturingRequiresPlayer;

	[Attribute("1")]
	protected bool m_bShowNotifications;

	[Attribute(ENotification.AREA_SEIZING_DONE_FRIENDLIES.ToString(), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ENotification))]
	protected ENotification m_eCapturedByFriendliesNotification;

	[Attribute(ENotification.AREA_SEIZING_DONE_ENEMIES.ToString(), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ENotification))]
	protected ENotification m_eCapturedByEnemiesNotification;

	[RplProp(onRplName: "OnSeizingTimestampChanged")]
	protected WorldTimestamp m_fSeizingStartTimestamp;

	[RplProp(onRplName: "OnSeizingTimestampChanged")]
	protected WorldTimestamp m_fSeizingEndTimestamp;

	protected static const float TRIGGER_CHECK_PERIOD_IDLE = 3;
	protected static const float TRIGGER_CHECK_PERIOD_ACTIVE = 1;

	protected ref ScriptInvoker m_OnCaptureStart;
	protected ref ScriptInvoker m_OnCaptureInterrupt;
	protected ref ScriptInvoker m_OnCaptureFinish;
	protected ref OnTimerChangeInvoker m_OnTimerChange;

	protected WorldTimestamp m_fInterruptedCaptureTimestamp;
	protected float m_fCurrentSeizingTime;
	protected float m_fInterruptedCaptureDuration;
	protected SCR_Faction m_PrevailingFaction;
	protected SCR_Faction m_PrevailingFactionPrevious;
	protected BaseGameTriggerEntity m_Trigger;
	protected bool m_bQueryFinished = true;
	protected bool m_bEnabled = true;
	protected RplComponent m_RplComponent;
	protected bool m_bCharacterPresent;
	protected SCR_FactionAffiliationComponent m_FactionControl;
	protected int m_iSeizingCharacters;
	protected ref map<int, WorldTimestamp> m_mSpawnTimers = new map<int, WorldTimestamp>();
	protected bool m_bDeleteDisabledAIs = false;

	//------------------------------------------------------------------------------------------------
	protected bool IsProxy()
	{
		return (m_RplComponent && m_RplComponent.IsProxy());
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetRadius()
	{
		return m_iRadius;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetMaximumAltitude()
	{
		return m_iMaximumAltitude;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] allow
	void AllowNotifications(bool allow)
	{
		m_bShowNotifications = allow;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	bool NotificationsAllowed()
	{
		return m_bShowNotifications;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvoker GetOnCaptureStart()
	{
		if (!m_OnCaptureStart)
			m_OnCaptureStart = new ScriptInvoker();

		return m_OnCaptureStart;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvoker GetOnCaptureInterrupt()
	{
		if (!m_OnCaptureInterrupt)
			m_OnCaptureInterrupt = new ScriptInvoker();

		return m_OnCaptureInterrupt;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvoker GetOnCaptureFinish()
	{
		if (!m_OnCaptureFinish)
			m_OnCaptureFinish = new ScriptInvoker();

		return m_OnCaptureFinish;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	OnTimerChangeInvoker GetOnTimerChange()
	{
		if (!m_OnTimerChange)
			m_OnTimerChange = new OnTimerChangeInvoker();

		return m_OnTimerChange;
	}

	//------------------------------------------------------------------------------------------------
	protected void EvaluatePrevailingFaction()
	{
		float delay;

		// Switch to idle mode when nobody is in the area
		// Evaluation for all bases should not be done at once, randomize the timer a bit
		if (m_bCharacterPresent)
			delay = Math.RandomFloatInclusive(TRIGGER_CHECK_PERIOD_ACTIVE, TRIGGER_CHECK_PERIOD_ACTIVE + (TRIGGER_CHECK_PERIOD_ACTIVE * 0.2));
		else
			delay = Math.RandomFloatInclusive(TRIGGER_CHECK_PERIOD_IDLE, TRIGGER_CHECK_PERIOD_IDLE + (TRIGGER_CHECK_PERIOD_IDLE * 0.2));

		GetGame().GetCallqueue().CallLater(EvaluatePrevailingFaction, delay * 1000);

		if (!m_bQueryFinished)
			return;

		m_Trigger.GetOnQueryFinished().Insert(OnQueryFinished);
		m_Trigger.QueryEntitiesInside();
		m_bQueryFinished = false;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnQueryFinished(BaseGameTriggerEntity trigger)
	{
		m_bQueryFinished = true;

		array<IEntity> presentEntities = {};
		int presentEntitiesCount = m_Trigger.GetEntitiesInside(presentEntities);
		m_bCharacterPresent = presentEntitiesCount != 0;

		// Nobody is here, no need to evaluate
		if (!m_bCharacterPresent)
		{
			if (m_PrevailingFaction)
			{
				m_PrevailingFactionPrevious = m_PrevailingFaction;
				m_PrevailingFaction = null;
				OnPrevailingFactionChanged();
			}

			return;
		}

		map<SCR_Faction, int> factionsPresence = new map<SCR_Faction, int>();
		map<SCR_Faction, bool> factionsPlayerPresence = new map<SCR_Faction, bool>();
		SCR_Faction evaluatedEntityFaction;
		int factionCount;
		PlayerManager playerManager = GetGame().GetPlayerManager();

		// Go through all entities and check their factions
		for (int i = 0; i < presentEntitiesCount; i++)
		{
			IEntity entity = presentEntities[i];
			
			if (m_bDeleteDisabledAIs && IsDisabledAI(entity))
			{
				RplComponent.DeleteRplEntity(entity, false);	
				continue;
			}				
			
			evaluatedEntityFaction = EvaluateEntityFaction(presentEntities[i]);

			if (!evaluatedEntityFaction)
				continue;

			factionCount = factionsPresence.Get(evaluatedEntityFaction);

			// If faction is not yet registered, do it now - otherwise just increase its presence counter
			if (factionCount == 0)
				factionsPresence.Insert(evaluatedEntityFaction, 1);
			else
				factionsPresence.Set(evaluatedEntityFaction, factionCount + 1);
			
			// Check if there are some players present in case they are required
			if (m_bCapturingRequiresPlayer && playerManager.GetPlayerIdFromControlledEntity(entity) != 0)
				factionsPlayerPresence.Set(evaluatedEntityFaction, true);
		}
		
		m_bDeleteDisabledAIs = false;
		SCR_Faction prevailingFaction;
		int highestAttackingPresence;
		int highestDefendingPresence;
		int curSeizingCharacters;

		// Evaluate the highest attacking presence
		foreach (SCR_Faction faction, int presence : factionsPresence)
		{
			// Non-playable attackers are not allowed
			if (m_bIgnoreNonPlayableAttackers && !faction.IsPlayable())
				continue;
			
			// In case players are required but are not present, ignore this faction for attacking
			if (m_bCapturingRequiresPlayer && !factionsPlayerPresence.Get(faction))
				continue;

			if (presence > highestAttackingPresence)
			{
				highestAttackingPresence = presence;
				prevailingFaction = faction;
			}
			else if (presence == highestAttackingPresence)	// When 2 or more factions have the same presence, none should prevail
			{
				prevailingFaction = null;
			}
		}

		// Evaluate the highest defending presence
		if (prevailingFaction)
		{
			foreach (SCR_Faction faction, int presence : factionsPresence)
			{
				// Non-playable defenders are not allowed
				if (m_bIgnoreNonPlayableDefenders && !faction.IsPlayable())
					continue;

				// This faction is already considered attacking
				if (faction == prevailingFaction)
					continue;

				highestDefendingPresence = Math.Max(presence, highestDefendingPresence);
			}

			// Get net amount of players effectively seizing (clamp for max attackers attribute)
			if (prevailingFaction && highestAttackingPresence > highestDefendingPresence)
			{
				curSeizingCharacters = Math.Min(highestAttackingPresence - highestDefendingPresence, m_iMaximumSeizingCharacters);
			}
			else
			{
				prevailingFaction = null;
				curSeizingCharacters = 0;
			}
		}

		if (prevailingFaction != m_PrevailingFaction)
		{
			m_iSeizingCharacters = curSeizingCharacters;
			m_PrevailingFactionPrevious = m_PrevailingFaction;
			m_PrevailingFaction = prevailingFaction;
			OnPrevailingFactionChanged();
		}
		else if (prevailingFaction && curSeizingCharacters != m_iSeizingCharacters)
		{
			m_iSeizingCharacters = curSeizingCharacters;
			RefreshSeizingTimer();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_Faction EvaluateEntityFaction(IEntity ent)
	{
		SCR_ChimeraCharacter char = SCR_ChimeraCharacter.Cast(ent);
		if (!char)
			return null;

		if (char.IsInVehicle() && SCR_TerrainHelper.GetHeightAboveTerrain(char.GetOrigin()) > m_iMaximumAltitude)
			return null;

		CharacterControllerComponent charControl = char.GetCharacterController();

		if (charControl && charControl.GetLifeState() != ECharacterLifeState.ALIVE)
			return null;

		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(ent);
		if (!playerId && charControl)
		{
			AIControlComponent ctrComp = charControl.GetAIControlComponent();
			if (ctrComp)
			{
				AIAgent ai = ctrComp.GetAIAgent();
				if (ai && ai.GetLOD() == AIAgent.GetMaxLOD())
				{
					return null;
				}
			}
		}

		// Handle after-respawn cooldown
		if (m_fRespawnCooldownPeriod > 0)
		{

			if (playerId != 0 && m_mSpawnTimers.Contains(playerId))
			{
				ChimeraWorld world = GetOwner().GetWorld();
				if (m_mSpawnTimers.Get(playerId).Greater(world.GetServerTimestamp()))
					return null;
				else
					m_mSpawnTimers.Remove(playerId)
			}
		}

		SCR_Faction entityFaction = SCR_Faction.Cast(char.GetFaction());

		return entityFaction;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPrevailingFactionChanged()
	{
		if (!m_PrevailingFaction || m_FactionControl.GetAffiliatedFaction() == m_PrevailingFaction)
		{
			if (m_fSeizingEndTimestamp != 0)
			{
				ChimeraWorld world = GetOwner().GetWorld();
				m_fInterruptedCaptureTimestamp = world.GetServerTimestamp();
				m_fInterruptedCaptureDuration = m_fInterruptedCaptureTimestamp.DiffMilliseconds(m_fSeizingStartTimestamp);
				m_fSeizingEndTimestamp = null;
				m_fSeizingStartTimestamp = null;
				int factionIndex = GetGame().GetFactionManager().GetFactionIndex(m_PrevailingFactionPrevious);
				Rpc(RpcDo_OnCaptureInterrupt, factionIndex);
				RpcDo_OnCaptureInterrupt(factionIndex);
			}
		}
		else
		{
			ChimeraWorld world = GetOwner().GetWorld();
			m_fSeizingStartTimestamp = world.GetServerTimestamp();
			RefreshSeizingTimer();
			int factionIndex = GetGame().GetFactionManager().GetFactionIndex(m_PrevailingFaction);
			Rpc(RpcDo_OnCaptureStart, factionIndex);
			RpcDo_OnCaptureStart(factionIndex);
		}

		OnSeizingTimestampChanged();
		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSeizingTimestampChanged()
	{
		if (m_OnTimerChange)
			m_OnTimerChange.Invoke(m_fSeizingStartTimestamp, m_fSeizingEndTimestamp);

		if (IsProxy())
			return;

		// Run EOnFrame only if timer is actually ticking
		if (m_fSeizingStartTimestamp == 0 && m_fSeizingEndTimestamp == 0)
			ClearEventMask(GetOwner(), EntityEvent.FRAME);
		else
			SetEventMask(GetOwner(), EntityEvent.FRAME);
	}

	//------------------------------------------------------------------------------------------------
	//!
	void RefreshSeizingTimer()
	{
		float seizingTimeVar = m_fMaximumSeizingTime - m_fMinimumSeizingTime;
		float deduct;

		if (m_iMaximumSeizingCharacters > 1)	// Avoid division by 0
		{
			float deductPerPlayer = seizingTimeVar / (m_iMaximumSeizingCharacters - 1);
			deduct = deductPerPlayer * (m_iSeizingCharacters - 1);
		}

		m_fSeizingEndTimestamp = m_fSeizingStartTimestamp.PlusSeconds(m_fMaximumSeizingTime - deduct);

		if (m_bGradualTimerReset && m_fInterruptedCaptureDuration != 0)
			HandleGradualReset();

		Replication.BumpMe();
		OnSeizingTimestampChanged();
	}

	//------------------------------------------------------------------------------------------------
	//! When capture started after getting interrupted, take into account the time spent on it
	protected void HandleGradualReset()
	{
		float timeSinceInterrupt = m_fSeizingStartTimestamp.DiffMilliseconds(m_fInterruptedCaptureTimestamp);

		if (timeSinceInterrupt < m_fInterruptedCaptureDuration)
		{
			float diff = m_fInterruptedCaptureDuration - timeSinceInterrupt;
			m_fSeizingStartTimestamp = m_fSeizingStartTimestamp.PlusMilliseconds(-diff);
			m_fSeizingEndTimestamp = m_fSeizingEndTimestamp.PlusMilliseconds(-diff);
		}

		m_fInterruptedCaptureDuration = 0;
		m_fInterruptedCaptureTimestamp = null;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	WorldTimestamp GetSeizingStartTimestamp()
	{
		return m_fSeizingStartTimestamp;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	WorldTimestamp GetSeizingEndTimestamp()
	{
		return m_fSeizingEndTimestamp;
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_OnCaptureStart(int factionIndex)
	{
		FactionManager factionManager = GetGame().GetFactionManager();

		if (!factionManager)
			return;

		SCR_Faction faction = SCR_Faction.Cast(factionManager.GetFactionByIndex(factionIndex));

		if (!faction)
			return;

		if (m_OnCaptureStart)
			m_OnCaptureStart.Invoke(faction, this);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_OnCaptureInterrupt(int factionIndex)
	{
		FactionManager factionManager = GetGame().GetFactionManager();

		if (!factionManager)
			return;

		SCR_Faction faction = SCR_Faction.Cast(factionManager.GetFactionByIndex(factionIndex));

		if (!faction)
			return;

		if (m_OnCaptureInterrupt)
			m_OnCaptureInterrupt.Invoke(faction, this);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_OnCaptureFinish(int factionIndex)
	{
		m_bDeleteDisabledAIs = true;
		FactionManager factionManager = GetGame().GetFactionManager();

		if (!factionManager)
			return;

		SCR_Faction faction = SCR_Faction.Cast(factionManager.GetFactionByIndex(factionIndex));

		if (!faction)
			return;

		if (m_OnCaptureFinish)
			m_OnCaptureFinish.Invoke(faction, this);

		UpdateFlagsInHierarchy(faction);

		if (m_bShowNotifications)
			NotifyPlayerInRadius(faction);

		if (!IsProxy())
		{
			if (m_FactionControl.GetAffiliatedFaction() != faction)
				m_FactionControl.SetAffiliatedFaction(faction);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlayerSpawned(int playerId, IEntity controlledEntity)
	{
		if (!controlledEntity)
			return;

		// Player did not spawn in the area, ignore them
		if (vector.DistanceSqXZ(controlledEntity.GetOrigin(), GetOwner().GetOrigin()) > (m_iRadius * m_iRadius))
			return;

		ChimeraWorld world = GetOwner().GetWorld();
		m_mSpawnTimers.Set(playerId, world.GetServerTimestamp().PlusSeconds(m_fRespawnCooldownPeriod));
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateFlagsInHierarchy(notnull SCR_Faction faction)
	{
		array<IEntity> queue = {GetOwner()};
		SCR_FlagComponent flag;
		IEntity processedEntity;
		IEntity nextInHierarchy;

		while (!queue.IsEmpty())
		{
			processedEntity = queue[0];
			queue.Remove(0);

			flag = SCR_FlagComponent.Cast(processedEntity.FindComponent(SCR_FlagComponent));
			if (flag)
				flag.ChangeMaterial(faction.GetFactionFlagMaterial());

			nextInHierarchy = processedEntity.GetChildren();

			while (nextInHierarchy)
			{
				queue.Insert(nextInHierarchy);
				nextInHierarchy = nextInHierarchy.GetSibling();
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool GetIsLocalPlayerPresent()
	{
		IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(SCR_PlayerController.GetLocalPlayerId());
		if (!player)
			return false;

		return (vector.DistanceSqXZ(player.GetOrigin(), GetOwner().GetOrigin()) <= (m_iRadius * m_iRadius));
	}

	//------------------------------------------------------------------------------------------------
	protected void NotifyPlayerInRadius(notnull SCR_Faction faction)
	{
		Faction playerFaction = SCR_FactionManager.SGetLocalPlayerFaction();
		if (!playerFaction)
			return;

		if (!GetIsLocalPlayerPresent())
			return;

		if (playerFaction == faction)
			SCR_NotificationsComponent.SendLocal(m_eCapturedByFriendliesNotification);
		else
			SCR_NotificationsComponent.SendLocal(m_eCapturedByEnemiesNotification);
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	Faction GetFaction()
	{
		if (m_FactionControl)
			return m_FactionControl.GetAffiliatedFaction();
		else
			return null;
	}

	//------------------------------------------------------------------------------------------------
	override void OnBaseFactionChanged(Faction faction)
	{
		super.OnBaseFactionChanged(faction);
		m_PrevailingFaction = null;
	}
	//------------------------------------------------------------------------------------------------
	bool IsDisabledAI(IEntity ent)
	{
		
		SCR_ChimeraCharacter char = SCR_ChimeraCharacter.Cast(ent);
		if (!char)
			return false;

		CharacterControllerComponent charControl = char.GetCharacterController();
		if (!charControl)
			return false;
		
		if (charControl.GetLifeState() == ECharacterLifeState.DEAD)
			return false;

		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(ent);
		if (playerId)
			return false;
	
		AIControlComponent ctrComp = charControl.GetAIControlComponent();
		if (!ctrComp)
			return false;
		
		if (ctrComp.IsAIActivated())
			return false;
		return true;
		
	}	
	//------------------------------------------------------------------------------------------------
	//!
	void Disable()
	{
		m_bEnabled = false;

		if (m_Trigger)
			delete m_Trigger;

		ClearEventMask(GetOwner(), EntityEvent.FRAME);

		foreach (SCR_MilitaryBaseComponent base : m_aBases)
		{
			if (!base)
				continue;

			base.UnregisterLogicComponent(this);
		}

		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());

		if (gameMode)
			gameMode.GetOnPlayerSpawned().Remove(OnPlayerSpawned);

		GetGame().GetCallqueue().Remove(EvaluatePrevailingFaction);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));

		// All functionality is server-side
		if (IsProxy())
			return;

		if (!m_bEnabled)
			return;

		// Attributes check
		if (m_iRadius <= 0)
		{
			Print("SCR_SeizingComponent: Invalid area radius (" + m_iRadius + ")! Terminating...", LogLevel.ERROR);
			return;
		}

		if (m_fMaximumSeizingTime < 0 || m_fMinimumSeizingTime < 0 || m_fMaximumSeizingTime < m_fMinimumSeizingTime)
		{
			Print("SCR_SeizingComponent: Invalid seizing time setting (" + m_fMinimumSeizingTime + ", " + m_fMaximumSeizingTime + ")! Terminating...", LogLevel.ERROR);
			return;
		}

		if (m_iMaximumSeizingCharacters <= 0)
		{
			Print("SCR_SeizingComponent: Invalid maximum seizing characters (" + m_iMaximumSeizingCharacters + ")! Terminating...", LogLevel.ERROR);
			return;
		}

		if (!GetGame().InPlayMode())
			return;

		SCR_SeizingComponentClass componentData = SCR_SeizingComponentClass.Cast(GetComponentData(GetOwner()));
		if (!componentData)
			return;

		m_FactionControl = SCR_FactionAffiliationComponent.Cast(owner.FindComponent(SCR_FactionAffiliationComponent));
		if (!m_FactionControl)
		{
			Print("SCR_SeizingComponent: Owner is missing SCR_FactionAffiliationComponent! Terminating...", LogLevel.ERROR);
			return;
		}

		Resource triggerResource = Resource.Load(componentData.GetTriggerPrefab());
		if (!triggerResource)
		{
			Print("SCR_SeizingComponent: Trigger resource failed to load! Terminating...", LogLevel.ERROR);
			return;
		}

		// Spawn the trigger locally on server
		m_Trigger = BaseGameTriggerEntity.Cast(GetGame().SpawnEntityPrefabLocal(triggerResource, GetGame().GetWorld()));
		if (!m_Trigger)
		{
			Print("SCR_SeizingComponent: Trigger failed to spawn! Terminating...", LogLevel.ERROR);
			return;
		}

		m_Trigger.SetSphereRadius(m_iRadius);
		owner.AddChild(m_Trigger, -1);

		// Register after-respawn cooldown method
		if (m_fRespawnCooldownPeriod > 0)
		{
			SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());

			if (gameMode)
				gameMode.GetOnPlayerSpawned().Insert(OnPlayerSpawned);
		}

		GetGame().GetCallqueue().CallLater(EvaluatePrevailingFaction, Math.RandomFloatInclusive(TRIGGER_CHECK_PERIOD_IDLE, TRIGGER_CHECK_PERIOD_IDLE + (TRIGGER_CHECK_PERIOD_IDLE * 0.2)) * 1000);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (m_fSeizingEndTimestamp == 0)
			return;

		ChimeraWorld world = owner.GetWorld();
		if (world.GetServerTimestamp().Less(m_fSeizingEndTimestamp))
			return;

		m_fSeizingEndTimestamp = null;
		m_fSeizingStartTimestamp = null;
		OnSeizingTimestampChanged();

		Replication.BumpMe();

		if (m_FactionControl.GetAffiliatedFaction() != m_PrevailingFaction)
		{
			int factionIndex = GetGame().GetFactionManager().GetFactionIndex(m_PrevailingFaction);
			Rpc(RpcDo_OnCaptureFinish, factionIndex);
			RpcDo_OnCaptureFinish(factionIndex);
		}
		else
		{
			int factionIndex = GetGame().GetFactionManager().GetFactionIndex(m_PrevailingFactionPrevious);
			Rpc(RpcDo_OnCaptureInterrupt, factionIndex);
			RpcDo_OnCaptureInterrupt(factionIndex);
		}
	}

	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_SeizingComponent()
	{
		if (m_Trigger)
			delete m_Trigger;

		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode)
			gameMode.GetOnPlayerSpawned().Remove(OnPlayerSpawned);

		GetGame().GetCallqueue().Remove(EvaluatePrevailingFaction);
	}
}
