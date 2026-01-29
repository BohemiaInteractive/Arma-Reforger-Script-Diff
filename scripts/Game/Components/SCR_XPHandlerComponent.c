class SCR_XPHandlerComponentClass : SCR_BaseGameModeComponentClass
{
}

class SCR_XPHandlerComponent : SCR_BaseGameModeComponent
{
	[Attribute("{E6FC4537B53EA00B}Configs/Campaign/XPRewards.conf", params: "conf class=SCR_XPRewardList")]
	private ResourceName m_sXPRewardsConfig;

	[Attribute("300", UIWidgets.EditBox, "How many XP a player needs to gain in a single life to get the Veterancy award. 0 = disabled.", params: "0 inf 1")]
	protected int m_iVeterancyXPAwardThreshold;

	[Attribute("1800", UIWidgets.EditBox, "If suicide is committed more than once in this time (seconds), a penalty is issued.", params: "0 inf 1")]
	protected int m_iSuicidePenaltyCooldown;
	
	[Attribute("180", UIWidgets.EditBox, "Player cannot be awarded medical assistance xp reward until this many seconds pass since the previous medical assistance reward", params: "0 inf 1")]
	protected int m_iMedicalAssistanceRewardCooldown;

	[Attribute("0.1", UIWidgets.EditBox, "Fraction of the vehicle cost to be awarded as XP to player destroying it", params: "0 inf 1")]
	protected float m_fEnemyVehicleDestroyXPMultiplier;

	[Attribute("180", UIWidgets.EditBox, "Player is being rewarded xp for survival each time this amount of seconds pass since spawning", params: "0 inf 1")]
	protected int m_iSurvivalRewardCooldown;

	[Attribute("60", UIWidgets.EditBox, "Maximum amount of survival reward cycles with xp rewards being scaled", params: "0 inf 1")]
	protected int m_iSurvivalScaleMaxCycleAmount;

	[Attribute("1", UIWidgets.EditBox, "Ratio of XP awarded to supplies spent in repair action (ratio of 0.5: 10 supplies spent on repair rewards 5 XP", params: "0 inf 1")]
	protected float m_fVehicleRepairXPMultiplier;

	[Attribute(desc: "Seize XP reward config")]
	protected ref SCR_SeizeXpRewardConfig m_mSeizeXPRewardConfig;

	static protected bool s_bXpSystemEnabled;
	
	//static const int SKILL_LEVEL_MAX = 10;
	//static const int SKILL_LEVEL_XP_COST = 1000;				// how much XP is needed for new level

	//static const float SKILL_LEVEL_XP_BONUS = 0.1;

	protected static const float TRANSPORT_POINTS_TO_XP_RATIO = 0.01;
	protected static const float TRANSPORT_HELI_MULTIPLIER = 0.25;
	protected static const int TRANSPORT_XP_PAYOFF_THRESHOLD = 15;

	protected ref array<ref SCR_XPRewardInfo> m_aXPRewardList = {};

	protected ref map<int, float> m_mPlayerTransportPoints = new map<int, float>();

	protected ref array<SCR_CampaignMilitaryBaseComponent> m_aBasesBeingSeized = {};
	protected float m_fBaseSeizingCheckTimer;

	protected float m_fXpMultiplier = 1;

	protected const int BASE_SEIZING_CHECK_INTERVAL = 5;
	protected const int RELAY_SEIZING_RADIUS = 50;

	//------------------------------------------------------------------------------------------------
	//! Returns true when Xp handling system is operational
	static bool IsXpSystemEnabled()
	{
		return s_bXpSystemEnabled;
	}

	//------------------------------------------------------------------------------------------------
	int GetMedicalAssistanceRewardCooldown()
	{
		return m_iMedicalAssistanceRewardCooldown;
	}

	//------------------------------------------------------------------------------------------------
	int GetSurvivalRewardCooldown()
	{
		return m_iSurvivalRewardCooldown;
	}

	//------------------------------------------------------------------------------------------------
	int GetSurvivalScaleMaxCycleAmount()
	{
		return m_iSurvivalScaleMaxCycleAmount;
	}

	//------------------------------------------------------------------------------------------------
	int GetSeizeBaseProgressionRewardTimer()
	{
		if (!m_mSeizeXPRewardConfig)
			return 0;

		return m_mSeizeXPRewardConfig.GetSeizeProgressionTimer();
	}

	//------------------------------------------------------------------------------------------------
	int GetSeizeBaseProgressionXP(string baseFactionKey)
	{
		if (!m_mSeizeXPRewardConfig)
			return 0;

		return m_mSeizeXPRewardConfig.GetSeizeProgressionXpReward(baseFactionKey);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerSpawnFinalize_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnHandlerComponent handlerComponent, SCR_SpawnData data, IEntity entity)
	{
		super.OnPlayerSpawnFinalize_S(requestComponent, handlerComponent, data, entity);
		
		if (!requestComponent || !entity)
			return;

		if (!IsProxy())
		{
			SCR_CompartmentAccessComponent compartmentAccessComponent = SCR_CompartmentAccessComponent.Cast(entity.FindComponent(SCR_CompartmentAccessComponent));

			if (compartmentAccessComponent)
				compartmentAccessComponent.GetOnCompartmentLeft().Insert(OnCompartmentLeft);
		}

		PlayerController pc = requestComponent.GetPlayerController();
		if (!pc)
			return;

		SCR_PlayerXPHandlerComponent compXP = SCR_PlayerXPHandlerComponent.Cast(pc.FindComponent(SCR_PlayerXPHandlerComponent));
		if (compXP)
		{
			compXP.UpdatePlayerRank(false);
			compXP.StartSurvivalRewardCycle();
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerDisconnected(int playerId, KickCauseCode cause, int timeout)
	{
		super.OnPlayerDisconnected(playerId, cause, timeout);

		m_mPlayerTransportPoints.Remove(playerId);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerKilled(notnull SCR_InstigatorContextData instigatorContextData)
	{
		super.OnPlayerKilled(instigatorContextData);
		
		if (IsProxy())
			return;
		
		int playerID = instigatorContextData.GetVictimPlayerID();
		
		//~ If player killed self (Admins and GM are not punished)
		if (instigatorContextData.HasAnyVictimKillerRelation(SCR_ECharacterDeathStatusRelations.SUICIDE))
			ProcessSuicide(playerID);

		PlayerController pc = GetGame().GetPlayerManager().GetPlayerController(playerID);

		if (pc)
		{
			SCR_PlayerXPHandlerComponent compXP = SCR_PlayerXPHandlerComponent.Cast(pc.FindComponent(SCR_PlayerXPHandlerComponent));

			if (compXP)
			{
				compXP.OnPlayerKilled();
				compXP.StopSurvivalRewardCycle();
			}
		}

		AwardTransportXP(playerID);
		
		SCR_CompartmentAccessComponent compartmentAccessComponent = SCR_CompartmentAccessComponent.Cast(instigatorContextData.GetVictimEntity().FindComponent(SCR_CompartmentAccessComponent));
		if (!compartmentAccessComponent)
			return;

		compartmentAccessComponent.GetOnCompartmentLeft().Remove(OnCompartmentLeft);
	}

	//------------------------------------------------------------------------------------------------
	override void OnControllableDestroyed(notnull SCR_InstigatorContextData instigatorContextData)
	{
		super.OnControllableDestroyed(instigatorContextData);

		IEntity victim = instigatorContextData.GetVictimEntity();
		if (!victim)
			return;

		// Vehicle destroyed, award XP for enemy vehicle destroy
		if (victim.IsInherited(Vehicle))
			OnVehicleDestroyed(instigatorContextData);

		// Character killed, award XP for killing a character
		if (ChimeraCharacter.Cast(victim))
			OnCharacterKilled(instigatorContextData);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnVehicleDestroyed(notnull SCR_InstigatorContextData instigatorContextData)
	{
		if (instigatorContextData.GetInstigator().GetInstigatorType() != InstigatorType.INSTIGATOR_PLAYER)
			return;

		Vehicle vehicle = Vehicle.Cast(instigatorContextData.GetVictimEntity());
		if (!vehicle)
			return;

		int playerID = instigatorContextData.GetInstigator().GetInstigatorPlayerID();
		if (playerID == 0)
			return;

		Faction playerFaction = SCR_FactionManager.SGetPlayerFaction(playerID);
		if (!playerFaction)
			return;

		Faction vehicleFaction = vehicle.GetFaction();

		// If vehicle does not have any affiliated faction, use it's default faction
		if (!vehicleFaction)
			vehicleFaction = vehicle.GetDefaultFaction();

		// Continue only if destroyed vehicle's faction is not friendly to player's faction
		if (!vehicleFaction || playerFaction.IsFactionFriendly(vehicleFaction))
			return;

		SCR_EditableEntityComponent editableEntityComponent = SCR_EditableEntityComponent.GetEditableEntity(vehicle);
		if (!editableEntityComponent)
			return;

		array<ref SCR_EntityBudgetValue> budgets = {};
		editableEntityComponent.GetEntityBudgetCost(budgets, vehicle);

		int xpToAward;
		foreach (SCR_EntityBudgetValue budget : budgets)
		{
			// We only care about the supply cost in Conflict
			if (budget.GetBudgetType() != EEditableEntityBudget.CAMPAIGN)
				continue;

			// XP Reward should be a fraction of the cost of the vehicle
			xpToAward = budget.GetBudgetValue() * m_fEnemyVehicleDestroyXPMultiplier;
			AwardXP(playerID, SCR_EXPRewards.ENEMY_VEHICLE_DESTRUCTION, xpToAward);

			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCharacterKilled(notnull SCR_InstigatorContextData instigatorContextData)
	{	
		// Handle XP for kills of players
		if (instigatorContextData.GetInstigator().GetInstigatorType() != InstigatorType.INSTIGATOR_PLAYER)
			return;
		
		SCR_ECharacterControlType killerControlType = instigatorContextData.GetKillerCharacterControlType();
		int killerId = instigatorContextData.GetKillerPlayerID();
		
		//~ Punish for teamkilling if teamkill punishment is enabled. GM and Admin are never punished
		if (instigatorContextData.HasAnyVictimKillerRelation(SCR_ECharacterDeathStatusRelations.KILLED_BY_FRIENDLY_PLAYER))
		{
			if (instigatorContextData.DoesPlayerKillCountAsTeamKill())
				AwardXP(killerId, SCR_EXPRewards.FRIENDLY_KILL);
			
			return;
		}
		//~ Killed by Enemy player
		if (instigatorContextData.HasAnyVictimKillerRelation(SCR_ECharacterDeathStatusRelations.KILLED_BY_ENEMY_PLAYER))
		{
			//~ Possessed AI will not get any XP to hide that the GM is possessing
			if (killerControlType == SCR_ECharacterControlType.POSSESSED_AI)
				return;
			
			//~ The kill was illegal so punish the player instead
			if (instigatorContextData.IsEnemyKillPunished(SCR_EDisguisedKillingPunishment.WARCRIME | SCR_EDisguisedKillingPunishment.XP_LOSS, true, true))
			{
				AwardXP(killerId, SCR_EXPRewards.WARCRIME);
				return;
			}
			else if (instigatorContextData.IsEnemyKillPunished(SCR_EDisguisedKillingPunishment.XP_LOSS, true))
			{
				AwardXP(killerId, SCR_EXPRewards.KILLING_WHILE_DISGUISED);
				return;
			}
			
			//~ Killer gets bigger XP reward for killing enemy commander
			int victimID = instigatorContextData.GetVictimPlayerID();
			SCR_Faction victimFaction = SCR_Faction.Cast(SCR_FactionManager.SGetPlayerFaction(victimID));
			if (victimFaction && victimFaction.GetCommanderId() == victimID)
			{
				AwardXP(killerId, SCR_EXPRewards.ENEMY_COMMANDER_KILL);
				return;
			}
			
			//~ Check if player is in vehicle
			//~ TODO: This logic is incomplete. If the player places a mine then gets into a vehicle and kills a enemy with the mine they will get the vehicle XP
			SCR_ChimeraCharacter instigatorChar = SCR_ChimeraCharacter.Cast(instigatorContextData.GetKillerEntity());
			if (instigatorChar && instigatorChar.IsInVehicle())
				AwardXP(killerId, SCR_EXPRewards.ENEMY_KILL_VEH);
			//~ Character not in vehicle so give normal reward
			else 
				AwardXP(killerId, SCR_EXPRewards.ENEMY_KILL);
			
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCompartmentLeft(IEntity targetEntity, BaseCompartmentManagerComponent manager, int mgrID, int slotID, bool move)
	{
		BaseCompartmentSlot compartment = manager.FindCompartment(slotID, mgrID);

		if (!compartment)
			return;

		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(compartment.GetOccupant());

		if (playerId == 0)
			return;

		AwardTransportXP(playerId);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnStatPointsAdded(int playerId, SCR_EDataStats stat, float amount, bool temp)
	{
		if (!temp)
			return;

		if (stat == SCR_EDataStats.POINTS_AS_DRIVER_OF_PLAYERS)
			OnDrivingStatPointsAdded(playerId, amount);
		else if (IsHealingFriendlyStatPoint(stat))
			OnHealingStatPointsAdded(playerId);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDrivingStatPointsAdded(int playerId, float amount)
	{
		SCR_ChimeraCharacter player = SCR_ChimeraCharacter.Cast(GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId));
		IEntity vehicle = CompartmentAccessComponent.GetVehicleIn(player);

		if (vehicle)
		{
			VehicleHelicopterSimulation heliSim = VehicleHelicopterSimulation.Cast(vehicle.FindComponent(VehicleHelicopterSimulation));
			
			// Award lower XP per distance travelled in a helicopter
			if (heliSim)
				amount *= TRANSPORT_HELI_MULTIPLIER;
		}

		int newValue = m_mPlayerTransportPoints.Get(playerId) + amount;
		m_mPlayerTransportPoints.Set(playerId, newValue);

		if (newValue * TRANSPORT_POINTS_TO_XP_RATIO >= TRANSPORT_XP_PAYOFF_THRESHOLD)
			AwardTransportXP(playerId);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnHealingStatPointsAdded(int playerId)
	{
		PlayerController pc = GetGame().GetPlayerManager().GetPlayerController(playerId);
		if (!pc)
			return;

		SCR_PlayerXPHandlerComponent compXP = SCR_PlayerXPHandlerComponent.Cast(pc.FindComponent(SCR_PlayerXPHandlerComponent));
		if (!compXP)
			return;

		compXP.MedicalAssistanceReward();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns true if stat corresponds to healing of friendly characters by any medical item
	//! \param[in] stat
	protected bool IsHealingFriendlyStatPoint(SCR_EDataStats stat)
	{
		return stat == SCR_EDataStats.MORPHINE_FRIENDLIES
			|| stat == SCR_EDataStats.BANDAGE_FRIENDLIES
			|| stat == SCR_EDataStats.SALINE_FRIENDLIES;
	}

	//------------------------------------------------------------------------------------------------
	//! Support station was executed
	//! \param[in] supportStation
	//! \param[in] supportStationType
	//! \param[in] actionTarget
	//! \param[in] actionUser
	//! \param[in] action
	protected void OnSupportStationExecuted(SCR_BaseSupportStationComponent supportStation, ESupportStationType supportStationType, IEntity actionTarget, IEntity actionUser, SCR_BaseUseSupportStationAction action)
	{
		// Vehicle repaired
		if (supportStationType == ESupportStationType.REPAIR)
			OnVehicleRepaired(supportStation, supportStationType, actionTarget, actionUser, action);

		// Character healed
		if (supportStationType == ESupportStationType.HEAL)
			OnCharacterHealed(supportStation, supportStationType, actionTarget, actionUser, action);
	}

	//------------------------------------------------------------------------------------------------
	//! Award XP when repair action is finished
	//! \param[in] supportStation
	//! \param[in] supportStationType
	//! \param[in] actionTarget
	//! \param[in] actionUser
	//! \param[in] action
	protected void OnVehicleRepaired(SCR_BaseSupportStationComponent supportStation, ESupportStationType supportStationType, IEntity actionTarget, IEntity actionUser, SCR_BaseUseSupportStationAction action)
	{
		// Only player as a user is counted
		int userPlayerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(actionUser);
		if (userPlayerId == 0 || !action)
			return;

		// Amount of awarded XP depends on cost of the repair action
		int xpToAward = action.GetSupportStationSuppliesOnUse() * m_fVehicleRepairXPMultiplier;
		if (xpToAward == 0)
			return;

		AwardXP(userPlayerId, SCR_EXPRewards.SUPPORT_REPAIR_VEHICLE, xpToAward);
	}

	//------------------------------------------------------------------------------------------------
	//! Award XP when heal action is finished
	//! \param[in] supportStation
	//! \param[in] supportStationType
	//! \param[in] actionTarget
	//! \param[in] actionUser
	//! \param[in] action
	protected void OnCharacterHealed(SCR_BaseSupportStationComponent supportStation, ESupportStationType supportStationType, IEntity actionTarget, IEntity actionUser, SCR_BaseUseSupportStationAction action)
	{
		// Only player as a user is counted
		int userPlayerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(actionUser);
		if (userPlayerId == 0 || !action)
			return;

		// Only healing other characters is counted
		int targetPlayerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(actionTarget);
		if (targetPlayerId == userPlayerId)
			return;

		PlayerController pc = GetGame().GetPlayerManager().GetPlayerController(userPlayerId);
		if (!pc)
			return;

		SCR_PlayerXPHandlerComponent compXP = SCR_PlayerXPHandlerComponent.Cast(pc.FindComponent(SCR_PlayerXPHandlerComponent));
		if (!compXP)
			return;

		compXP.MedicalAssistanceReward();
	}

	//------------------------------------------------------------------------------------------------
	void OnBaseSeized(SCR_CampaignMilitaryBaseComponent baseComponent)
	{
		SCR_ECampaignBaseType baseType = baseComponent.GetType();
		SCR_ECampaignSeizingBaseType seizingBaseType;

		if (baseComponent.IsControlPoint())
			seizingBaseType = SCR_ECampaignSeizingBaseType.CONTROL_POINT;
		else if (baseType == SCR_ECampaignBaseType.SOURCE_BASE)
			seizingBaseType = SCR_ECampaignSeizingBaseType.SOURCE_BASE;
		else if (baseType == SCR_ECampaignBaseType.BASE)
			seizingBaseType = SCR_ECampaignSeizingBaseType.FOB;

		int xpReward = m_mSeizeXPRewardConfig.GetSeizeCompletionXpReward(seizingBaseType);

		int radius;
		vector baseOrigin = baseComponent.GetOwner().GetOrigin();
		Faction baseFaction = baseComponent.GetFaction();

		if (baseType == SCR_ECampaignBaseType.RELAY)
			radius = RELAY_SEIZING_RADIUS;
		else
			radius = baseComponent.GetRadius();

		PlayerManager playerManager = GetGame().GetPlayerManager();
		array<int> players = {};
		playerManager.GetPlayers(players);
		SCR_ChimeraCharacter playerEntity;

		foreach (int playerId : players)
		{
			playerEntity = SCR_ChimeraCharacter.Cast(playerManager.GetPlayerControlledEntity(playerId));

			if (!playerEntity || playerEntity.GetFactionKey() != baseFaction.GetFactionKey())
				continue;

			if (vector.DistanceSq(playerEntity.GetOrigin(), baseOrigin) >= radius * radius)
				continue;

			if (baseType == SCR_ECampaignBaseType.RELAY)
				AwardXP(playerId, SCR_EXPRewards.RELAY_RECONFIGURED, 1);
			else
				AwardXP(playerId, SCR_EXPRewards.BASE_SEIZED, 1, false, xpReward);
		}
	}

	//------------------------------------------------------------------------------------------------
	void OnBaseAttackStarted(notnull SCR_CampaignMilitaryBaseComponent base, Faction defendingFaction, Faction attackingFaction)
	{
		if (m_aBasesBeingSeized.Contains(base))
			return;

		m_aBasesBeingSeized.Insert(base);
	}

	//------------------------------------------------------------------------------------------------
	void OnBaseAttackEnded(notnull SCR_CampaignMilitaryBaseComponent base)
	{
		if (!m_aBasesBeingSeized.Contains(base))
			return;

		m_aBasesBeingSeized.RemoveItem(base);

		PlayerManager playerManager = GetGame().GetPlayerManager();
		array<int> players = {};
		playerManager.GetPlayers(players);
		PlayerController playerController;
		SCR_PlayerXPHandlerComponent playerXpHandlerComponent;

		foreach (int playerId : players)
		{
			playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);
			if (!playerController)
				continue;

			playerXpHandlerComponent = SCR_PlayerXPHandlerComponent.Cast(playerController.FindComponent(SCR_PlayerXPHandlerComponent));
			if (!playerXpHandlerComponent)
				continue;

			playerXpHandlerComponent.StopSeizingProgressReward(base);
		}
	}

	//------------------------------------------------------------------------------------------------
	void EvaluateAllAttackedBasesAttackers()
	{
		foreach (SCR_CampaignMilitaryBaseComponent base : m_aBasesBeingSeized)
		{
			EvaluateBaseAttackers(base);
		}
	}

	//------------------------------------------------------------------------------------------------
	void EvaluateBaseAttackers(SCR_CampaignMilitaryBaseComponent base)
	{
		Faction baseFaction = base.GetFaction();
		if (!baseFaction)
			return;

		PlayerManager playerManager = GetGame().GetPlayerManager();
		array<int> players = {};
		playerManager.GetPlayers(players);
		vector baseOrigin = base.GetOwner().GetOrigin();
		int radius = base.GetRadius();

		SCR_ChimeraCharacter playerEntity;
		PlayerController playerController;
		SCR_PlayerXPHandlerComponent playerXpHandlerComponent;

		foreach (int playerId : players)
		{
			playerEntity = SCR_ChimeraCharacter.Cast(playerManager.GetPlayerControlledEntity(playerId));

			if (!playerEntity || playerEntity.GetFactionKey() == baseFaction.GetFactionKey())
				continue;

			playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);
			if (!playerController)
				continue;

			playerXpHandlerComponent = SCR_PlayerXPHandlerComponent.Cast(playerController.FindComponent(SCR_PlayerXPHandlerComponent));
			if (!playerXpHandlerComponent)
				continue;

			if (vector.DistanceSq(playerEntity.GetOrigin(), baseOrigin) < radius * radius)
				playerXpHandlerComponent.StartSeizingProgressReward(base);
			else
				playerXpHandlerComponent.StopSeizingProgressReward(base);
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] playerId
	void ProcessSuicide(int playerId)
	{
#ifdef NO_SUICIDE_PENALTY
		return;
#endif
		PlayerController pc = GetGame().GetPlayerManager().GetPlayerController(playerId);
		if (!pc)
			return;

		SCR_PlayerXPHandlerComponent compXPPlayer = SCR_PlayerXPHandlerComponent.Cast(pc.FindComponent(SCR_PlayerXPHandlerComponent));
		if (!compXPPlayer)
			return;

		SCR_FactionManager fm = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!fm)
			return;

		// Check for cooldown on the penalty
		float curTime = GetGame().GetWorld().GetWorldTime();
		float penaltyTimestamp = compXPPlayer.GetSuicidePenaltyTimestamp();
		compXPPlayer.SetSuicidePenaltyTimestamp(curTime + (m_iSuicidePenaltyCooldown * 1000.0));
		if (curTime > penaltyTimestamp)
			return;

		// Don't make player renegade just by suiciding
		int penalty = GetXPRewardAmount(SCR_EXPRewards.SUICIDE);
		int playerXPWithPenalty = compXPPlayer.GetPlayerXP() + penalty;
		SCR_ECharacterRank newRank = fm.GetRankByXP(playerXPWithPenalty);
		if (fm.IsRankRenegade(newRank))
			return;

		AwardXP(pc.GetPlayerId(), SCR_EXPRewards.SUICIDE);
	}

	//------------------------------------------------------------------------------------------------
	protected void AwardTransportXP(int playerId)
	{
		float toAward = m_mPlayerTransportPoints.Get(playerId) * TRANSPORT_POINTS_TO_XP_RATIO;

		if (toAward <= 0)
			return;

		PlayerController controller = GetGame().GetPlayerManager().GetPlayerController(playerId);

		if (!controller)
			return;

		m_mPlayerTransportPoints.Set(playerId, 0);

		AwardXP(controller, SCR_EXPRewards.TASK_TRANSPORT, toAward);
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsProxy()
	{
		RplComponent rplComponent = RplComponent.Cast(GetOwner().FindComponent(RplComponent));

		return rplComponent && rplComponent.IsProxy();
	}

	//------------------------------------------------------------------------------------------------
	//! Add XP to given playerId
	//! \param[in] playerId
	//! \param[in] rewardID
	//! \param[in] multiplier
	//! \param[in] volunteer
	//! \param[in] customXP
	void AwardXP(int playerId, SCR_EXPRewards rewardID, float multiplier = 1.0, bool volunteer = false, int customXP = 0)
	{
		if (IsProxy())
			return;

		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);

		if (playerController)
			AwardXP(playerController, rewardID, multiplier, volunteer, customXP);
	}

	//------------------------------------------------------------------------------------------------
	//! Add XP to given controller
	//! \param[in] controller
	//! \param[in] rewardID
	//! \param[in] multiplier
	//! \param[in] volunteer
	//! \param[in] customXP
	void AwardXP(notnull PlayerController controller, SCR_EXPRewards rewardID, float multiplier = 1.0, bool volunteer = false, int customXP = 0)
	{
		if (IsProxy())
			return;

		SCR_PlayerXPHandlerComponent comp = SCR_PlayerXPHandlerComponent.Cast(controller.FindComponent(SCR_PlayerXPHandlerComponent));

		if (!comp)
			return;

		comp.AddPlayerXP(rewardID, multiplier, volunteer, customXP);

		// Do not handle veterancy award if it's being processed already or disabled
		if (rewardID == SCR_EXPRewards.VETERANCY || m_iVeterancyXPAwardThreshold == 0 || customXP != 0)
			return;

		int singleLifeXP = comp.GetPlayerXPSinceLastSpawn();
		float veterancyAwards = Math.Floor(singleLifeXP / m_iVeterancyXPAwardThreshold);

		if (veterancyAwards < 1)
			return;

		int leftoverXP = singleLifeXP % m_iVeterancyXPAwardThreshold;
		comp.SetPlayerXPSinceLastSpawn(leftoverXP);

		// Award veterancy bonus with a delay so the UI isn't overwritten immediately
		GetGame().GetCallqueue().CallLater(VeterancyAward, 2000, false, controller, veterancyAwards);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] controller
	//! \param[in] multiplier
	void VeterancyAward(notnull PlayerController controller, float multiplier)
	{
		AwardXP(controller, SCR_EXPRewards.VETERANCY, multiplier);
	}

	//------------------------------------------------------------------------------------------------
	SCR_XPRewardInfo GetXpRewardInfo(SCR_EXPRewards reward)
	{
		foreach (SCR_XPRewardInfo info : m_aXPRewardList)
		{
			if (info.GetRewardID() == reward)
				return info;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns XP reward amount
	//! \param[in] reward
	//! \return
	int GetXPRewardAmount(SCR_EXPRewards reward)
	{
		int rewardsCnt = m_aXPRewardList.Count();

		for (int i = 0; i < rewardsCnt; i++)
		{
			if (m_aXPRewardList[i].GetRewardID() == reward)
				return m_aXPRewardList[i].GetRewardXP();
		}

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns XP reward amount
	//! \return
	float GetXPMultiplier()
	{
		return m_fXpMultiplier;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns XP reward name
	//! \param[in] reward
	//! \return XP reward name
	string GetXPRewardName(SCR_EXPRewards reward)
	{
		int rewardsCnt = m_aXPRewardList.Count();

		for (int i = 0; i < rewardsCnt; i++)
		{
			if (m_aXPRewardList[i].GetRewardID() == reward)
				return m_aXPRewardList[i].GetRewardName();
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] reward
	//! \return
	bool AllowNotification(SCR_EXPRewards reward)
	{
		int rewardsCnt = m_aXPRewardList.Count();

		for (int i = 0; i < rewardsCnt; i++)
		{
			if (m_aXPRewardList[i].GetRewardID() == reward)
				return m_aXPRewardList[i].AllowNotification();
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns XP reward skill
	//! \param[in] reward
	//! \return
//	EProfileSkillID GetXPRewardSkill(SCR_EXPRewards reward)
//	{
//		int rewardsCnt = m_aXPRewardList.Count();
//
//		for (int i = 0; i < rewardsCnt; i++)
//		{
//			if (m_aXPRewardList[i].GetRewardID() == reward)
//				return m_aXPRewardList[i].GetRewardSkill();
//		}
//
//		return EProfileSkillID.GLOBAL;
//	}

	//------------------------------------------------------------------------------------------------
	protected override void EOnFrame(IEntity owner, float timeSlice)
	{
		m_fBaseSeizingCheckTimer += timeSlice;

		if (m_fBaseSeizingCheckTimer > BASE_SEIZING_CHECK_INTERVAL)
		{
			m_fBaseSeizingCheckTimer = 0;
			EvaluateAllAttackedBasesAttackers();
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		//Parse & register XP reward list
		Resource container = BaseContainerTools.LoadContainer(m_sXPRewardsConfig);
		SCR_XPRewardList list = SCR_XPRewardList.Cast(BaseContainerTools.CreateInstanceFromContainer(container.GetResource().ToBaseContainer()));
		list.GetRewardList(m_aXPRewardList);

		SCR_MissionHeader header = SCR_MissionHeader.Cast(GetGame().GetMissionHeader());

		if (header)
			m_fXpMultiplier = header.m_fXpMultiplier;

		SCR_PlayerData.s_OnStatAdded.Insert(OnStatPointsAdded);

		SCR_SupportStationManagerComponent supportStationManager = SCR_SupportStationManagerComponent.GetInstance();
		if (supportStationManager)
		{
			supportStationManager.GetOnSupportStationExecutedSuccessfully().Insert(OnSupportStationExecuted);
		}

		SCR_CampaignMilitaryBaseComponent.GetOnBaseUnderAttack().Insert(OnBaseAttackStarted);
		SCR_CampaignMilitaryBaseComponent.GetOnBaseAttackEnd().Insert(OnBaseAttackEnded);

		SetEventMask(GetOwner(), EntityEvent.FRAME);

		s_bXpSystemEnabled = true;
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_XPHandlerComponent()
	{
		SCR_SupportStationManagerComponent supportStationManager = SCR_SupportStationManagerComponent.GetInstance();
		if (supportStationManager)
		{
			supportStationManager.GetOnSupportStationExecutedSuccessfully().Remove(OnSupportStationExecuted);
		}

		SCR_CampaignMilitaryBaseComponent.GetOnBaseUnderAttack().Remove(OnBaseAttackStarted);
		SCR_CampaignMilitaryBaseComponent.GetOnBaseAttackEnd().Remove(OnBaseAttackEnded);

		s_bXpSystemEnabled = false;
	}
}

enum SCR_EXPRewards
{
	UNDEFINED,
	CHEAT,
	ENEMY_KILL,
	ENEMY_KILL_VEH,
	FRIENDLY_KILL,
	RELAY_DISCOVERED,
	RELAY_RECONFIGURED,
	BASE_SEIZED,
	BASE_DEFENDED,
	SUPPLIES_DELIVERED,
	SUPPORT_EVAC,
	SUPPORT_FUEL,
	TASK_DEFEND,
	TASK_TRANSPORT,
	SERVICE_BUILD,
	SUICIDE,
	VETERANCY,
	SPAWN_PROVIDER,
	FREE_ROAM_BUILDING_BUILT,
	CUSTOM_1,
	CUSTOM_2,
	CUSTOM_3,
	CUSTOM_4,
	CUSTOM_5,
	CUSTOM_6,
	CUSTOM_7,
	CUSTOM_8,
	CUSTOM_9,
	CUSTOM_10,
	CUSTOM_11,
	CUSTOM_12,
	CUSTOM_13,
	CUSTOM_14,
	CUSTOM_15,
	CUSTOM_16,
	CUSTOM_17,
	CUSTOM_18,
	CUSTOM_19,
	CUSTOM_20,
	
	VALUABLE_INTEL_HANDIN_SMALL,
	VALUABLE_INTEL_HANDIN_MEDIUM,
	VALUABLE_INTEL_HANDIN_LARGE,
	KILLING_WHILE_DISGUISED,
	WARCRIME,
	SUPPORT_REPAIR_VEHICLE,
	MEDICAL_ASSISTANCE,
	ENEMY_COMMANDER_KILL,
	ENEMY_VEHICLE_DESTRUCTION,
	SURVIVAL,
	GROUP_COHESION,

	REARM_TASK_COMPLETED,
	ATTACK_TASK_COMPLETED,
	ATTACK_TASK_FAILED,
	REPAIR_TASK_COMPLETED,
	REINFORCE_TASK_COMPLETED,
	ESTABLISH_BASE_COMPLETED,
	DISMANTLE_BASE_COMPLETED,
	SEIZE_TASK_COMPLETED,
	COMMANDER_TASK_COMPLETED,
	RECON_TASK_COMPLETED,
	HOLD_TASK_COMPLETED,

	STARTING_RANK,
}
