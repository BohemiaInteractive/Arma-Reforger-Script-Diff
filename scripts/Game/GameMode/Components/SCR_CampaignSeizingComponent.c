class SCR_CampaignSeizingComponentClass : SCR_SeizingComponentClass
{
}

class SCR_CampaignSeizingComponent : SCR_SeizingComponent
{
	[Attribute("60", params: "0 inf 0.1", category: "Campaign")]
	protected float m_fExtraTimePerService;
	
	[Attribute("60", params: "0 inf 0.1", category: "Campaign")]
	protected float m_fExtraTimePerRadioConnection;

	protected SCR_CampaignMilitaryBaseComponent m_Base;
	
	//------------------------------------------------------------------------------------------------
	protected override SCR_Faction EvaluateEntityFaction(IEntity ent)
	{
		if (!m_Base || m_Base.IsHQ() || !m_Base.IsInitialized())
			return null;
		
		SCR_Faction faction = super.EvaluateEntityFaction(ent);
		
		if (!faction)
			return null;
		
		// Players of faction not covering this base with radio signal should not be able to capture or prevent capture
		SCR_CampaignFaction cFaction = SCR_CampaignFaction.Cast(faction);
		
		if (!cFaction)
			return null;
		
		if (faction.IsPlayable() && !m_Base.IsHQRadioTrafficPossible(cFaction))
			return null;
		
		return faction;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnQueryFinished(BaseGameTriggerEntity trigger)
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
			// FIA is not allowed
			if (faction == SCR_GameModeCampaign.GetInstance().GetFactionByEnum(SCR_ECampaignFaction.INDFOR))
				continue;
			
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
	override void RefreshSeizingTimer()
	{
		if (!m_fSeizingStartTimestamp)
			return;

		int servicesCount;

		if (m_Base)
		{
			array<SCR_EServicePointType> checkedTypes = {
				SCR_EServicePointType.ARMORY,
				SCR_EServicePointType.HELIPAD,
				SCR_EServicePointType.BARRACKS,
				SCR_EServicePointType.RADIO_ANTENNA,
				SCR_EServicePointType.FIELD_HOSPITAL,
				SCR_EServicePointType.LIGHT_VEHICLE_DEPOT,
				SCR_EServicePointType.HEAVY_VEHICLE_DEPOT
			};

			foreach (SCR_EServicePointType type : checkedTypes)
			{
				if (m_Base.GetServiceDelegateByType(type))
					servicesCount++;
			}
		}
		
		int radioConnectionsCount;
		SCR_CoverageRadioComponent comp = SCR_CoverageRadioComponent.Cast(m_Base.GetOwner().FindComponent(SCR_CoverageRadioComponent));
		
		if (comp)
		{
			SCR_CampaignFaction faction = m_Base.GetCampaignFaction();
			
			if (faction && faction.IsPlayable())
				radioConnectionsCount = comp.GetRadiosInRangeOfCount(faction.GetFactionRadioEncryptionKey());
		}

		float seizingTimeVar = m_fMaximumSeizingTime - m_fMinimumSeizingTime;
		float deduct;

		if (m_iMaximumSeizingCharacters > 1)	// Avoid division by 0
		{
			float deductPerPlayer = seizingTimeVar / (m_iMaximumSeizingCharacters - 1);
			deduct = deductPerPlayer * (m_iSeizingCharacters - 1);
		}

		float multiplier = 1;
		
		if ((m_fMaximumSeizingTime - m_fMinimumSeizingTime) > 0)
		{
			multiplier += (servicesCount * (m_fExtraTimePerService / (m_fMaximumSeizingTime - m_fMinimumSeizingTime)));
			multiplier += (radioConnectionsCount * (m_fExtraTimePerRadioConnection / (m_fMaximumSeizingTime - m_fMinimumSeizingTime)));
		}

		m_fSeizingEndTimestamp = m_fSeizingStartTimestamp.PlusSeconds(multiplier * (m_fMaximumSeizingTime - deduct));

		ChimeraWorld world = GetGame().GetWorld();
		WorldTimestamp currentTime = world.GetServerTimestamp();

		// Add a tiny delay if removing a service would cause immediate capture
		if (m_fSeizingEndTimestamp.LessEqual(currentTime))
			m_fSeizingEndTimestamp = currentTime.PlusMilliseconds(SCR_GameModeCampaign.DEFAULT_DELAY);

		if (m_bGradualTimerReset && m_fInterruptedCaptureDuration != 0)
			HandleGradualReset();
		
		Replication.BumpMe();
		OnSeizingTimestampChanged();
	}
	
//	//------------------------------------------------------------------------------------------------
//	//!
//	//! \param[in] maxSeizingTime
//	//! \param[in] minSeizingTime
//	//! \param[in] extraTime
//	//! \param[in] players
//	//! \param[in] services
//	static void TestValues(float maxSeizingTime, float minSeizingTime, float extraTime, int players, int services)
//	{
//		float seizingTimeVar = maxSeizingTime - minSeizingTime;
//		float deductPerPlayer = seizingTimeVar / 11;
//		float deduct = deductPerPlayer * (players - 1);
//		float servicesMultiplier = 1 + (services * (extraTime / (maxSeizingTime - minSeizingTime)));
//		Print(servicesMultiplier * (maxSeizingTime - deduct));
//	}
	
	//------------------------------------------------------------------------------------------------
	override void OnBaseRegistered(notnull SCR_MilitaryBaseComponent base)
	{
		super.OnBaseRegistered(base);
		
		SCR_CampaignMilitaryBaseComponent campaignBase = SCR_CampaignMilitaryBaseComponent.Cast(base);
		
		if (!campaignBase)
		{
			UnregisterBase(base);
			base.UnregisterLogicComponent(this);
		}
		
		if (!campaignBase || campaignBase.IsHQ())
			return;
		
		m_Base = campaignBase;
	}
}
