class SCR_PlayerXPHandlerComponentClass : ScriptComponentClass
{
}

void OnPlayerXPChangedMethod(int playerId, int currentXP, int XPToAdd, SCR_EXPRewards rewardId);
typedef func OnPlayerXPChangedMethod;
typedef ScriptInvokerBase<OnPlayerXPChangedMethod> OnPlayerXPChanged;

//! Takes care of player-specific XP handling
//! Should be hooked on PlayerController
class SCR_PlayerXPHandlerComponent : ScriptComponent
{
	[RplProp(condition: RplCondition.OwnerOnly)]
	protected int m_iPlayerXP = 0;

	protected int m_iPlayerXPSinceLastSpawn;

	protected float m_fSuicidePenaltyTimestamp;
	
	protected float m_fMedicalAssistanceXPRewardTimestamp;

	protected int m_iSurvivalRewardCycle;

	protected ref ScriptInvoker m_OnXPChanged;

	protected ref OnPlayerXPChanged m_OnPlayerXPChanged;

	protected bool m_bSeizingRewardCycleActive;
	protected SCR_CampaignMilitaryBaseComponent m_SeizedBaseComponent;

	protected ref map<SCR_EXPRewards, WorldTimestamp> m_mRewardCooldowns = new map<SCR_EXPRewards, WorldTimestamp>(); //!< <XPReward type, cooldownTimestamp>

	//------------------------------------------------------------------------------------------------
	//! Getter for player XP
	int GetPlayerXP()
	{
		return m_iPlayerXP;
	}

	//------------------------------------------------------------------------------------------------
	//! Setter for player XP accumulated since last respawn
	//! \param[in] xp
	void SetPlayerXPSinceLastSpawn(int xp)
	{
		m_iPlayerXPSinceLastSpawn = xp;
	}

	//------------------------------------------------------------------------------------------------
	//! Getter for player XP accumulated since last respawn
	int GetPlayerXPSinceLastSpawn()
	{
		return m_iPlayerXPSinceLastSpawn;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] timestamp
	void SetSuicidePenaltyTimestamp(float timestamp)
	{
		m_fSuicidePenaltyTimestamp = timestamp;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetSuicidePenaltyTimestamp()
	{
		return m_fSuicidePenaltyTimestamp;
	}

	//------------------------------------------------------------------------------------------------
	void SetSurvivalRewardCycle(int cycle)
	{
		m_iSurvivalRewardCycle = cycle;
	}

	//------------------------------------------------------------------------------------------------
	int GetSurvivalRewardCycle()
	{
		return m_iSurvivalRewardCycle;
	}

	//------------------------------------------------------------------------------------------------
	//! \return rank
	SCR_ECharacterRank GetPlayerRankByXP()
	{
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return SCR_ECharacterRank.PRIVATE;

		return factionManager.GetRankByXP(m_iPlayerXP);
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvoker GetOnXPChanged()
	{
		if (!m_OnXPChanged)
			m_OnXPChanged = new ScriptInvoker();

		return m_OnXPChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	OnPlayerXPChanged GetOnPlayerXPChanged()
	{
		if (!m_OnPlayerXPChanged)
			m_OnPlayerXPChanged = new OnPlayerXPChanged();

		return m_OnPlayerXPChanged;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool IsProxy()
	{
		RplComponent rpl = RplComponent.Cast(GetOwner().FindComponent(RplComponent));

		return (rpl && rpl.IsProxy());
	}

	//------------------------------------------------------------------------------------------------
	//!
	void OnPlayerKilled()
	{
		m_iPlayerXPSinceLastSpawn = 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Set character rank based on player controller EXP
	//! \param[in] notify
	void UpdatePlayerRank(bool notify = true)
	{
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetOwner());

		if (!playerController)
			return;

		IEntity player = playerController.GetMainEntity();

		if (!player)
			return;

		SCR_CharacterRankComponent comp = SCR_CharacterRankComponent.Cast(player.FindComponent(SCR_CharacterRankComponent));
		if (!comp)
			return;

		auto factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return;

		const SCR_ECharacterRank newRank = factionManager.GetRankByXP(m_iPlayerXP);
		comp.SetCharacterRank(newRank, !notify);
	}

	//------------------------------------------------------------------------------------------------
	//! Cheat method to change player's rank - server side
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_CheatRank(int playerID, bool demote)
	{
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());

		SCR_ECharacterRank rank = factionManager.GetRankByXP(m_iPlayerXP);
		int reqXP;

		if (demote)
		{
			SCR_ECharacterRank newRank = factionManager.GetRankPrev(rank);

			if (newRank == SCR_ECharacterRank.INVALID)
				return;

			reqXP = factionManager.GetRequiredRankXP(newRank) - m_iPlayerXP;
		}
		else
		{
			SCR_ECharacterRank newRank = factionManager.GetRankNext(rank);

			if (newRank == SCR_ECharacterRank.INVALID)
				return;

			reqXP = factionManager.GetRequiredRankXP(newRank) - m_iPlayerXP;
		}

		SCR_XPHandlerComponent comp = SCR_XPHandlerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_XPHandlerComponent));

		if (!comp)
			return;

		comp.AwardXP(playerID, SCR_EXPRewards.CHEAT, reqXP);
	}

	//------------------------------------------------------------------------------------------------
	//! Cheat method to change player's rank
	void CheatRank(bool demote = false)
	{
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetOwner());

		if (!playerController)
			return;

		int playerID = playerController.GetPlayerId();
		Rpc(RpcAsk_CheatRank, playerID, demote);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_OnPlayerXPChanged(int currentXP, int XPToAdd, bool volunteer, SCR_EXPRewards rewardID, bool profileUsed, int skillLevel)
	{
		if (m_OnXPChanged)
			m_OnXPChanged.Invoke(currentXP, rewardID, XPToAdd, volunteer, profileUsed, skillLevel);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Reward the player for using medical items for healing characters other than self
	//! Currently the reward is given even when AI character or enemy character is healed
	//! After receiving XP award for medical assistance, there is a reward cooldown with duration based on parameter m_iMedicalAssistanceAwardCooldown
	void MedicalAssistanceReward()
	{
		// Check if the cooldown period from previous medical assistance award passed
		float curTime = GetGame().GetWorld().GetWorldTime();
		if (curTime <= m_fMedicalAssistanceXPRewardTimestamp)
			return;

		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return;

		SCR_XPHandlerComponent comp = SCR_XPHandlerComponent.Cast(gameMode.FindComponent(SCR_XPHandlerComponent));
		if (!comp)
			return;

		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetOwner());
		if (!playerController)
			return;

		int playerID = playerController.GetPlayerId();

		comp.AwardXP(playerID, SCR_EXPRewards.MEDICAL_ASSISTANCE);

		// When XP awarded, add cooldown to next available medical xp reward
		m_fMedicalAssistanceXPRewardTimestamp = curTime + comp.GetMedicalAssistanceRewardCooldown() * 1000;
	}

	//------------------------------------------------------------------------------------------------
	//!
	void StartSurvivalRewardCycle()
	{
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return;

		SCR_XPHandlerComponent comp = SCR_XPHandlerComponent.Cast(gameMode.FindComponent(SCR_XPHandlerComponent));
		if (!comp)
			return;

		// Call SurvivalReward periodically while the player is alive
		const int survivalRewardCycleDuration = comp.GetSurvivalRewardCooldown();
		if (survivalRewardCycleDuration > 0)
			GetGame().GetCallqueue().CallLater(SurvivalReward, survivalRewardCycleDuration * 1000, true);
	}

	//------------------------------------------------------------------------------------------------
	//!
	void StopSurvivalRewardCycle()
	{
		// Remove SurvivalReward from call queue on player death
		GetGame().GetCallqueue().Remove(SurvivalReward);
		m_iSurvivalRewardCycle = 0;
	}

	//------------------------------------------------------------------------------------------------
	//!
	void SurvivalReward()
	{
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return;

		SCR_XPHandlerComponent comp = SCR_XPHandlerComponent.Cast(gameMode.FindComponent(SCR_XPHandlerComponent));
		if (!comp)
			return;

		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetOwner());
		if (!playerController)
			return;

		int playerID = playerController.GetPlayerId();
		if (playerID == 0)
			return;

		int baseXPReward = comp.GetXPRewardAmount(SCR_EXPRewards.SURVIVAL);
		m_iSurvivalRewardCycle++;
		// Reward XP gets scaled up more the more cycles player survives, the maximum amount of xp gained is limited by survival scale max cycle amount
		int xpReward = Math.Min(m_iSurvivalRewardCycle * baseXPReward, baseXPReward * comp.GetSurvivalScaleMaxCycleAmount());

		if (xpReward == 0)
			return;

		comp.AwardXP(playerID, SCR_EXPRewards.SURVIVAL, xpReward);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] base
	void StartSeizingProgressReward(notnull SCR_CampaignMilitaryBaseComponent base)
	{
		if (m_bSeizingRewardCycleActive)
			return;

		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return;

		SCR_XPHandlerComponent xpHandlerComponent = SCR_XPHandlerComponent.Cast(gameMode.FindComponent(SCR_XPHandlerComponent));
		if (!xpHandlerComponent)
			return;

		int seizingRewardCycleDuration = xpHandlerComponent.GetSeizeBaseProgressionRewardTimer();
		if (seizingRewardCycleDuration == 0)
			return;

		// Seizing progress reward is called periodically while the base is being seized
		GetGame().GetCallqueue().CallLater(SeizingProgressReward, seizingRewardCycleDuration * 1000, true, xpHandlerComponent);

		m_SeizedBaseComponent = base;
		m_bSeizingRewardCycleActive = true;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] base
	void StopSeizingProgressReward(notnull SCR_CampaignMilitaryBaseComponent base)
	{
		if (!m_bSeizingRewardCycleActive)
			return;

		if (!m_SeizedBaseComponent || base != m_SeizedBaseComponent)
			return;

		// Base is no longer being seized, stop calling the seizing progress reward
		GetGame().GetCallqueue().Remove(SeizingProgressReward);

		m_SeizedBaseComponent = null;
		m_bSeizingRewardCycleActive = false;
	}

	//------------------------------------------------------------------------------------------------
	//! Rewards player for progression on seizing of base
	//! Rewarded XP is dependant on the faction affiliation of the seized base
	void SeizingProgressReward(SCR_XPHandlerComponent xpHandlerComponent)
	{
		if (!m_bSeizingRewardCycleActive || !m_SeizedBaseComponent)
			return;

		if (!xpHandlerComponent)
			return;

		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetOwner());
		if (!playerController)
			return;

		// Only players are awarded XP
		int playerID = playerController.GetPlayerId();
		if (playerID == 0)
			return;

		Faction baseFaction = m_SeizedBaseComponent.GetFaction();
		if (!baseFaction)
			return;

		Faction capturingFaction = m_SeizedBaseComponent.GetCapturingFaction();
		if (!capturingFaction)
		{
			StopSeizingProgressReward(m_SeizedBaseComponent);
			return;
		}

		Faction playerFaction = SCR_FactionManager.SGetPlayerFaction(playerID);
		if (!playerFaction || playerFaction != capturingFaction)
			return;

		// XP reward is dependent on base Faction
		int xpReward = xpHandlerComponent.GetSeizeBaseProgressionXP(baseFaction.GetFactionKey());
		if (xpReward == 0)
			return;

		xpHandlerComponent.AwardXP(playerID, SCR_EXPRewards.BASE_SEIZED, 1, false, xpReward);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] rewardID
	//! returns true if player has active reward cooldown
	protected bool HasPlayerRewardCooldown(SCR_EXPRewards rewardID)
	{
		ChimeraWorld world = GetGame().GetWorld();
		if (!world)
			return false;

		WorldTimestamp cooldownTimestamp;
		if (!m_mRewardCooldowns.Find(rewardID, cooldownTimestamp))
			return false;

		return !world.GetServerTimestamp().Greater(cooldownTimestamp);
	}

	//------------------------------------------------------------------------------------------------
	//! Sets to player reward cooldown
	//! \param[in] rewardID
	//! \param[in] cooldown
	protected void SetPlayerRewardCooldown(SCR_EXPRewards rewardID, float cooldown)
	{
		ChimeraWorld world = GetGame().GetWorld();
		if (!world)
			return;

		m_mRewardCooldowns.Set(rewardID, world.GetServerTimestamp().PlusSeconds(cooldown));
	}

	//------------------------------------------------------------------------------------------------
	//! Addition to player XP
	//! \param[in] rewardID
	//! \param[in] multiplier
	//! \param[in] volunteer
	//! \param[in] addDirectly
	void AddPlayerXP(SCR_EXPRewards rewardID, float multiplier = 1.0, bool volunteer = false, int addDirectly = 0)
	{
		if (IsProxy())
			return;
		
		BaseGameMode gameMode = GetGame().GetGameMode();
		
		if (!gameMode)
			return;

		SCR_XPHandlerComponent comp = SCR_XPHandlerComponent.Cast(gameMode.FindComponent(SCR_XPHandlerComponent));
		
		if (!comp)
			return;

		// checks whether the player can receive an XP reward depending on if the reward cooldown has expired
		SCR_XPRewardInfo rewardInfo = comp.GetXpRewardInfo(rewardID);
		if (rewardInfo && rewardInfo.CanUseRewardCooldown())
		{
			if (HasPlayerRewardCooldown(rewardID))
				return;

			SetPlayerRewardCooldown(rewardID, rewardInfo.GetRewardCooldown());
		}

		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetOwner());

		if (addDirectly != 0)
		{
			m_iPlayerXP += (addDirectly);

			if (playerController && m_OnPlayerXPChanged)
				m_OnPlayerXPChanged.Invoke(playerController.GetPlayerId(), m_iPlayerXP, addDirectly, rewardID);

			Replication.BumpMe();
			UpdatePlayerRank(false);
			Rpc(RpcDo_OnPlayerXPChanged, m_iPlayerXP, addDirectly, false, rewardID, false, 0);
			return;
		}

		int XP = comp.GetXPRewardAmount(rewardID);

		if (XP == 0)
			return;

		//EProfileSkillID skillID = comp.GetXPRewardSkill(rewardID);
		//auto profileManager = campaign.FindComponent(SCR_PlayerProfileManagerComponent); Replaced by SCR_PlayerData

		// 35% XP bonus when the player volunteered for the task
		if (volunteer)
			multiplier += 0.35;

		int XPToAdd = Math.Round(XP * multiplier * comp.GetXPMultiplier());

		const int XPToAddBySkill = 0; // TODO: check for good const usage
		bool profileUsed = false;
		const int skillLevel = 0; // TODO: check for good const usage
		/***** Replaced by SCR_PlayerData
		//****
		// Handle skill XP
		if (profileManager && XP > 0)
		{
			SCR_PlayerProfileManagerComponent profileManagerCast = SCR_PlayerProfileManagerComponent.Cast(profileManager);
			CareerBackendData profile = profileManagerCast.GetPlayerProfile(m_PlayerController.GetPlayerId());

			if (profile)
			{
				profileUsed = true;
				skillLevel = Math.Min(Math.Floor(profile.GetSkillXP(skillID) / SCR_GameModeCampaign.SKILL_LEVEL_XP_COST), SCR_GameModeCampaign.SKILL_LEVEL_MAX);
				XPToAddBySkill = Math.Round(XPToAdd * skillLevel * SCR_GameModeCampaign.SKILL_LEVEL_XP_BONUS);
				profile.AddSkillXP(skillID, XP * multiplier);
			}
		}
		//****
		*****/
		if (XPToAdd + XPToAddBySkill == 0)
			return;

		m_iPlayerXP += (XPToAdd + XPToAddBySkill);

		if (rewardID != SCR_EXPRewards.VETERANCY)
			m_iPlayerXPSinceLastSpawn += (XPToAdd + XPToAddBySkill);

		if (playerController && m_OnPlayerXPChanged)
			m_OnPlayerXPChanged.Invoke(playerController.GetPlayerId(), m_iPlayerXP, XPToAdd, rewardID);

		Replication.BumpMe();
		UpdatePlayerRank();

		Rpc(RpcDo_OnPlayerXPChanged, m_iPlayerXP, XPToAdd + XPToAddBySkill, volunteer, rewardID, profileUsed, skillLevel);
	}

	#ifdef ENABLE_DIAG
	//------------------------------------------------------------------------------------------------
	override protected void EOnDiag(IEntity owner, float timeSlice)
	{
		super.EOnDiag(owner, timeSlice);

		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_CHARACTER_SHOW_XP))
		{
			int xp = GetPlayerXP();

			DbgUI.Begin("Player XP Debug", 50, 50);
			DbgUI.Text("Player XP: " + xp);
			DbgUI.End();
		}
	}
	#endif

	//------------------------------------------------------------------------------------------------
	override protected void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		#ifdef ENABLE_DIAG
		ConnectToDiagSystem(owner);
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_CHARACTER_SHOW_XP, "", "Show Player XP", "Character");
		#endif
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnDelete(IEntity owner)
	{
		#ifdef ENABLE_DIAG
		DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_CHARACTER_SHOW_XP);
		#endif

		super.OnDelete(owner);
	}
}
