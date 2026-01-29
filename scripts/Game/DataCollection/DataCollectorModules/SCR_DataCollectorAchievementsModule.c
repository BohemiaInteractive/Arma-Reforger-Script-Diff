[BaseContainerProps()]
class SCR_DataCollectorAchievementsModule : SCR_DataCollectorModule
{
	protected const FactionKey FACTION_CIVILIAN = "CIV";
	protected const float DEADEYE_RANGE = 300;
	protected const float DEADEYE_RANGE_SQ = DEADEYE_RANGE * DEADEYE_RANGE;
	protected const int DEADEYE_KILL_COUNT = 10;
	protected const int NIGHT_STALKER_KILL_COUNT = 5;
	protected const int LAST_ONE_STANDING_KILL_COUNT = 10;

	protected ref map<int, int> m_mDeadeyeKillCounter = new map<int, int>();// <playerID, counter>
	protected ref map<int, int> m_mLastOneStandingKillCounter = new map<int, int>();// <playerID, counter>
	protected ref map<int, int> m_mNightStalkerKillCounter = new map<int, int>();// <playerID, counter>
	
	protected TimeAndWeatherManagerEntity m_TimeManager;
	protected SCR_GameModeCampaign m_Conflict;

	//------------------------------------------------------------------------------------------------
	protected override void InitModule()
	{
		// Achievement COMBAT_HYGIENE
		SCR_FlushToilet.GetOnToiletFlushed().Insert(ToiletFlushed);
		
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return;
		
		RplComponent rplComp = RplComponent.Cast(gameMode.FindComponent(RplComponent));
		if (rplComp.IsMaster())
		{
			m_Conflict = SCR_GameModeCampaign.Cast(gameMode);

			// Invoke thhose only as authority
			// Achievement NUTCRACKER
			SCR_VehicleDamageManagerComponent.GetOnVehicleDestroyed().Insert(VehicleDestroyed);
			// Achievement TRUCKER_JOE
			SCR_CampaignNetworkComponent.GetOnSuppliesDelivered().Insert(SuppliesDelivered);
			// Achievements HELPING_HAND, BATTLEFIELD_ANGEL (Support stations)
			SCR_SupportStationManagerComponent supportStationManager = SCR_SupportStationManagerComponent.GetInstance();
			if (supportStationManager)
			{
				supportStationManager.GetOnSupportStationExecutedSuccessfully().Insert(OnSupportStationUsed);
			}

			// allowed only in conflict mode
			if (m_Conflict)
			{
				// Achievement RESCUE_TECHNICIAN
				SCR_RepairSupportStationComponent.GetOnFireExtinguished().Insert(OnFireExtinguished);

				// Achievement CHANGING_CHANNELS
				SCR_CampaignNetworkComponent.GetOnBaseCaptured().Insert(OnBaseCaptured);

				// Achievement NIGHT_STALKER
				ChimeraWorld world = GetGame().GetWorld();
				if (world)
					m_TimeManager = world.GetTimeAndWeatherManager();

				// Achievement REQUISTIONED
				SCR_ResourceEntityRefundAction.GetOnRefundPerformed().Insert(OnEntityRefundPerformed);
			}
		}
		
		// Achievement MAJOR_PROMOTION
		SCR_CharacterRankComponent.s_OnRankChanged.Insert(RankedUp);
		
		// Achievement MINED_OUT
		SCR_MineInventoryItemComponent.GetOnMinePlaced().Insert(MinePlaced);
		
		// Achievement IVORY_TICKLER || PIPING_UP
		SCR_PlayInstrument.GetOnInstrumentPlayed().Insert(InstrumentPlayed);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void AddInvokers(IEntity player)
	{
		// Achievement HELPING_HAND || BATTLEFIELD_ANGEL
		SCR_ChimeraCharacter chimeraPlayer = SCR_ChimeraCharacter.Cast(player);
		if (!chimeraPlayer)
			return;

		SCR_CharacterControllerComponent characterController = SCR_CharacterControllerComponent.Cast(chimeraPlayer.GetCharacterController());
		if (!characterController)
			return;
		
		characterController.m_OnItemUseEndedInvoker.Insert(OnItemUsed);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void RemoveInvokers(IEntity player)
	{
		SCR_ChimeraCharacter chimeraPlayer = SCR_ChimeraCharacter.Cast(player);
		if (!chimeraPlayer)
			return;

		SCR_CharacterControllerComponent characterController = SCR_CharacterControllerComponent.Cast(chimeraPlayer.GetCharacterController());
		if (!characterController)
			return;

		characterController.m_OnItemUseEndedInvoker.Remove(OnItemUsed);
	}
	
	//------------------------------------------------------------------------------------------------
	//Removing the invokers that do not belong to an entity
	protected void ~SCR_DataCollectorAchievementsModule()
	{
		SCR_FlushToilet.GetOnToiletFlushed().Remove(ToiletFlushed);
		SCR_VehicleDamageManagerComponent.GetOnVehicleDestroyed().Remove(VehicleDestroyed);
		SCR_CharacterRankComponent.s_OnRankChanged.Remove(RankedUp);	
		SCR_MineInventoryItemComponent.GetOnMinePlaced().Remove(MinePlaced);
		SCR_PlayInstrument.GetOnInstrumentPlayed().Remove(InstrumentPlayed);
		SCR_CampaignNetworkComponent.GetOnSuppliesDelivered().Remove(SuppliesDelivered);

		SCR_SupportStationManagerComponent supportStationManager = SCR_SupportStationManagerComponent.GetInstance();
		if (supportStationManager)
		{
			supportStationManager.GetOnSupportStationExecutedSuccessfully().Remove(OnSupportStationUsed);
		}

		SCR_ResourceEntityRefundAction.GetOnRefundPerformed().Remove(OnEntityRefundPerformed);
		SCR_RepairSupportStationComponent.GetOnFireExtinguished().Remove(OnFireExtinguished);
		SCR_CampaignNetworkComponent.GetOnBaseCaptured().Remove(OnBaseCaptured);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPlayerKilled(int playerId, IEntity playerEntity, IEntity killerEntity, notnull Instigator instigator, notnull SCR_InstigatorContextData instigatorContextData)
	{
		// Only players are counted towards stats & achievements
		if (instigator.GetInstigatorType() != InstigatorType.INSTIGATOR_PLAYER)
			return;
		
		const int killerId = instigator.GetInstigatorPlayerID();
		
		// zeroing counter for LAST_ONE_STANDING Achievement
		m_mLastOneStandingKillCounter.Set(playerId, 0);

		//~ Not a player kill so ignore (Like suicide)
		if (!instigatorContextData.HasAnyVictimKillerRelation(SCR_ECharacterDeathStatusRelations.KILLED_BY_ENEMY_PLAYER | SCR_ECharacterDeathStatusRelations.KILLED_BY_FRIENDLY_PLAYER))
			return;
		
		// Handle achievement statistic PLAYER_KILLED (achievement TRUE_GRIT)
		// Note: PLAYER_KILLED is counted even for friendlies - oopsies makes fun
		IncrementAchievementProgress(playerId, AchievementStatId.PLAYER_KILLED);
		
		// Teamkills don't count
		if (instigatorContextData.HasAnyVictimKillerRelation(SCR_ECharacterDeathStatusRelations.KILLED_BY_FRIENDLY_PLAYER))
			return;
		
		// Increment stat for the killer
		IncrementAchievementProgress(killerId, AchievementStatId.ENEMIES_NEUTRALIZED);

		// allowed only in conflict mode
		if (m_Conflict)
		{
			// Increment counter for LAST_ONE_STANDING Achievement
			int lastOneStandingKillCount = m_mLastOneStandingKillCounter.Get(killerId) + 1;
			if (lastOneStandingKillCount >= LAST_ONE_STANDING_KILL_COUNT)
			{
				Print("Player with id " + killerId + " Unlocked - Last one standing", LogLevel.DEBUG);
				UnlockAchievement(killerId, AchievementId.LAST_ONE_STANDING);
			}
			m_mLastOneStandingKillCounter.Set(killerId, lastOneStandingKillCount);

			// Increment counter for NIGHT_STALKER Achievement
			if (m_TimeManager && m_TimeManager.IsNightHour(m_TimeManager.GetTimeOfTheDay()))
			{
				int nightStalkerKillCount = m_mNightStalkerKillCounter.Get(killerId) + 1;
				if (nightStalkerKillCount >= NIGHT_STALKER_KILL_COUNT)
				{
					Print("Player with id " + killerId + " Unlocked - Night stalker", LogLevel.DEBUG);
					UnlockAchievement(killerId, AchievementId.NIGHT_STALKER);
				}
				m_mNightStalkerKillCounter.Set(killerId, nightStalkerKillCount);
			}

			if (!playerEntity || !killerEntity)
				return;

			// Increment counter for DEADEYE Achievement
			if (vector.DistanceSq(playerEntity.GetOrigin(), killerEntity.GetOrigin()) >= DEADEYE_RANGE_SQ)
			{
				int count = m_mDeadeyeKillCounter.Get(killerId) + 1;
				if (count >= DEADEYE_KILL_COUNT)
				{
					Print("Player with id " + killerId + " Unlocked - DEADEYE", LogLevel.DEBUG);
					UnlockAchievement(killerId, AchievementId.DEADEYE);
				}

				m_mDeadeyeKillCounter.Set(killerId, count);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnGameModeEnd()
	{
		array<int> players = {};
		GetGame().GetPlayerManager().GetPlayers(players);
		
		SCR_GameModeCombatOpsManager combatOps = SCR_GameModeCombatOpsManager.Cast(GetGame().GetGameMode().FindComponent(SCR_GameModeCombatOpsManager));
		SCR_GameModeCampaign conflict = SCR_GameModeCampaign.Cast(GetGame().GetGameMode());
		
		if (!combatOps && !conflict)
			return;
		
		SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
		if (!taskSystem)
			return;
		
		array<SCR_Task> activeTasks = {};
		array<SCR_Task> finishedTasks = {};
		taskSystem.GetTasksByState(activeTasks, SCR_ETaskState.CREATED);
		taskSystem.GetTasksByState(finishedTasks, SCR_ETaskState.COMPLETED);
		
		if (combatOps)
		{
			/* Achievement CLEAN_SWEEP || PAPER_PUSHER */
			
			bool foundIntel, cleanSweep;
			foreach (SCR_Task task : finishedTasks)
			{
				if (!task)
					continue;
				
				if (task.GetTaskName().Contains("Intel"))
				{
					foundIntel = true;
					break;
				}
			}
			
			if (activeTasks.IsEmpty()) // all tasks finished
				cleanSweep = true;
			
			if (finishedTasks.Count() > activeTasks.Count()) // mission successful
			{	
				foreach(int player: players)
				{
					if (cleanSweep)
						CleanSweepCombatOps(player);
					
					if (foundIntel)
						SecureIntelCombatOps(player);
				}
			}
			else // mission lost
			{
				foreach (SCR_Task task : activeTasks)
				{
					if (!task || task.GetTaskState() != SCR_ETaskState.COMPLETED)
						continue;
					
					if (task.GetTaskName().Contains("Intel")) // should be in finished tasks, no?!
					{
						foreach(int player: players)
						{
							SecureIntelCombatOps(player);
						}
						break;
					}
				}
			}
		}
		else if (conflict && conflict.IsTutorial())
		{
			/* Achievement SWEAT_SAVES_BLOOD */
			if (finishedTasks.Count() >= activeTasks.Count())
			{	
				foreach(int player: players)
				{
					CleanSweepTutorial(player);
				}
				return;
			}
			/* Achievement SWEAT_SAVES_BLOOD */
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnItemUsed(IEntity item, bool actionCompleted, ItemUseParameters animParams)
	{
		// This is invoked for multiple items being used, including mines, but mines are handled separately
		if (!item || !actionCompleted)
		{
			return;
		}

		// We are interested only in consumables
		SCR_ConsumableItemComponent consumableComp = SCR_ConsumableItemComponent.Cast(item.FindComponent(SCR_ConsumableItemComponent));
		if (!consumableComp)
		{
			return;
		}

		// We are interested only in medical consumables
		SCR_EConsumableType consumType = consumableComp.GetConsumableType();

		// All consumables are medical at this time, but I try to make check, that would skip any newly added consumables
		// and should produce error, if the existing medical consumable is removed/changed in SCR_EConsumableType
		// TODO: Turn into something more serious when general "medical" type is available
		if (consumType != SCR_EConsumableType.BANDAGE &&
			consumType != SCR_EConsumableType.HEALTH &&
			consumType != SCR_EConsumableType.TOURNIQUET &&
			consumType != SCR_EConsumableType.SALINE &&
			consumType != SCR_EConsumableType.MORPHINE &&
			consumType != SCR_EConsumableType.MED_KIT)
		{
			return;
		}

		IEntity userCharacter = consumableComp.GetCharacterOwner();
		if (!userCharacter)
		{
			return;
		}

		IEntity targetCharacter = consumableComp.GetTargetCharacter();

		OnHealItemUsed(consumType, userCharacter, targetCharacter);

		if (!targetCharacter)
		{
			return;
		}

		// Self usage doesn't count
		if (userCharacter == targetCharacter)
		{
			return;
		}

		// Only player as a user is counted
		int userPlayerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(userCharacter);
		if (userPlayerId == 0)
		{
			return;
		}

		// Only player as a target is counted
		int targetPlayerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(targetCharacter);
		if (targetPlayerId == 0)
		{
			return;
		}

		// Tracking for achievements HELPING_HAND, BATTLEFIELD_ANGEL
		IncrementAchievementProgress(userPlayerId, AchievementStatId.MEDICAL_ASSISTS);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnHealItemUsed(SCR_EConsumableType consumType, IEntity userCharacter, IEntity targetCharacter)
	{
		if (!userCharacter)
			return;

		// Only player as a user is counted
		int userPlayerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(userCharacter);
		if (userPlayerId == 0)
		{
			return;
		}

		// if targetCharacter is null, he used the item on himself, and this is allowed
		if (targetCharacter)
		{
			// Only player as a target is counted
			int targetPlayerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(targetCharacter);
			if (targetPlayerId == 0)
			{
				return;
			}
		}

		// allowed only in conflict mode
		if (!m_Conflict)
			return;

		// Achievement READY_SALTED
		if (consumType == SCR_EConsumableType.SALINE)
		{
			Print("Player with id " + userPlayerId + " used a saline solution!", LogLevel.DEBUG);
			IncrementAchievementProgress(userPlayerId, AchievementStatId.SALINE_SOLUTION_USED);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSupportStationUsed(SCR_BaseSupportStationComponent supportStation, ESupportStationType supportStationType, IEntity actionTarget, IEntity actionUser, SCR_BaseUseSupportStationAction action)
	{
		// Achievements are interested only in HEAL type
		if (supportStationType != ESupportStationType.HEAL)
		{
			return;
		}
		
		// Not interested in self aplication
		if (actionTarget == actionUser)
		{
			return;
		}
		
		// Only player as a user is counted
		int userPlayerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(actionUser);
		if (userPlayerId == 0)
		{
			return;
		}

		// Only player as a target is counted
		int targetPlayerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(actionTarget);
		if (targetPlayerId == 0)
		{
			return;
		}
		
		// Increment the stat of HELPING_HAND, BATTLEFIELD_ANGEL achievement
		IncrementAchievementProgress(userPlayerId, AchievementStatId.MEDICAL_ASSISTS);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void InstrumentPlayed(int playerId, SCR_EInstrumentType instrumentType)
	{
		// Achievement IVORY_TICKLER || PIPING_UP
		
		switch (instrumentType)
		{
			case SCR_EInstrumentType.PIANO: 
				PianoPlayed(playerId);
			break;
			case SCR_EInstrumentType.ORGAN:
				OrganPlayed(playerId);
			break;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UnlockAchievement(int playerId, AchievementId achievementId)
	{
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);
		if (!playerController)
			return;
		
		SCR_AchievementsHandler handler = SCR_AchievementsHandler.Cast(playerController.FindComponent(SCR_AchievementsHandler));
		if (!handler)
			return;
		
		handler.UnlockAchievement(achievementId);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void IncrementAchievementProgress(int playerId, AchievementStatId achievementStatId)
	{
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);
		if (!playerController)
			return;
		
		SCR_AchievementsHandler handler = SCR_AchievementsHandler.Cast(playerController.FindComponent(SCR_AchievementsHandler));
		if (!handler)
			return;
		
		handler.IncrementAchievementProgress(achievementStatId);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ToiletFlushed(int playerId)
	{		
		// Achievement COMBAT_HYGIENE
		Print("Player with id " + playerId + " flushed a toilet!", LogLevel.DEBUG);
		UnlockAchievement(playerId, AchievementId.COMBAT_HYGIENE);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void VehicleDestroyed(int playerId)
	{
		// Achievement NUTCRACKER
		IncrementAchievementProgress(playerId, AchievementStatId.VEHICLES_DESTROYED);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RankedUp(SCR_ECharacterRank prevRank, SCR_ECharacterRank newRank, IEntity playerEntity, bool silent)
	{
		// Achievement MAJOR_PROMOTION
		if (newRank != SCR_ECharacterRank.MAJOR || prevRank > newRank)
			return;
		
		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(playerEntity);
		if (playerId <= 0)
			return;
		
		Print("Player with id " + playerId + " ranked up to major!", LogLevel.DEBUG);
		UnlockAchievement(playerId, AchievementId.MAJOR_PROMOTION);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SuppliesDelivered(int playerId, int suppliesDelivered, int totalSuppliesDelivered)
	{
		// Achievement TRUCKER_JOE
		const int TARGET_AMOUNT = 5000;

		if (totalSuppliesDelivered < TARGET_AMOUNT)
			return;

		Print("Player with id " + playerId + " delivered lots of supplies!", LogLevel.DEBUG);
		UnlockAchievement(playerId, AchievementId.TRUCKER_JOE);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CleanSweepCombatOps(int playerId)
	{
		// Achievement CLEAN_SWEEP
		Print("Player with id " + playerId + " cleanswept combat ops!", LogLevel.DEBUG);
		UnlockAchievement(playerId, AchievementId.CLEAN_SWEEP);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SecureIntelCombatOps(int playerId)
	{
		// Achievement PAPER_PUSHER
		Print("Player with id " + playerId + " has secured the intel!", LogLevel.DEBUG);
		UnlockAchievement(playerId, AchievementId.PAPER_PUSHER);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CleanSweepTutorial(int playerId)
	{
		//Achievement SWEAT_SAVES_BLOOD
		Print("Player with id " + playerId + " has cleanswept the tutorial!", LogLevel.DEBUG);
		UnlockAchievement(playerId, AchievementId.SWEAT_SAVES_BLOOD);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void MinePlaced(int playerId)
	{
		// Achievement MINED_OUT
		Print("Player with id " + playerId + " placed a mine!", LogLevel.DEBUG);
		IncrementAchievementProgress(playerId, AchievementStatId.MINES_PLACED);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void PianoPlayed(int playerId)
	{
		// Achievement IVORY_TICKLER
		Print("Player with id " + playerId + " played a Piano!", LogLevel.DEBUG);
		UnlockAchievement(playerId, AchievementId.IVORY_TICKLER);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OrganPlayed(int playerId)
	{
		//Achievement PIPING_UP
		Print("Player with id " + playerId + " played an Organ!", LogLevel.DEBUG);
		UnlockAchievement(playerId, AchievementId.PIPING_UP);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnEntityRefundPerformed(IEntity refundedEntity, IEntity playerEntity)
	{
		// Achievement REQUISTIONED
		if (!refundedEntity || !playerEntity)
			return;

		SCR_VehicleFactionAffiliationComponent factionComponent = SCR_VehicleFactionAffiliationComponent.Cast(refundedEntity.FindComponent(SCR_VehicleFactionAffiliationComponent));
		if (!factionComponent)
			return;

		if (factionComponent.GetDefaultFactionKey() != FACTION_CIVILIAN)
			return;

		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(playerEntity);

		Print("Player with id " + playerId + " refunded a civilian vehicle!", LogLevel.DEBUG);
		IncrementAchievementProgress(playerId, AchievementStatId.CIV_VEHICLE_DECOMMISSIONED);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnFireExtinguished(IEntity playerEntity)
	{
		// Achievement RESCUE_TECHNICIAN
		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(playerEntity);
		Print("Player with id " + playerId + " extinguished a fire!", LogLevel.DEBUG);
		IncrementAchievementProgress(playerId, AchievementStatId.FIRE_EXTINGUISHED);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnBaseCaptured(SCR_CampaignMilitaryBaseComponent base, int playerId)
	{
		// Achievement CHANGING_CHANNELS
		if (!base)
			return;

		if (base.GetType() != SCR_ECampaignBaseType.RELAY)
			return;

		if (playerId == 0) // is AI
			return;

		Print("Player with id " + playerId + " reconfigured a radio relay!", LogLevel.DEBUG);
		IncrementAchievementProgress(playerId, AchievementStatId.RADIO_RELAY_RECONFIGURED);
	}
};
