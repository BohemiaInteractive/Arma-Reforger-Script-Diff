class SCR_CampaignSeizingComponentClass : SCR_SeizingComponentClass
{
}

void OnCaptureStateChangedDelegate(SCR_EBaseCaptureState spawnpoint);
typedef func OnCaptureStateChangedDelegate;
typedef ScriptInvokerBase<OnCaptureStateChangedDelegate> OnCaptureStateChangedInvoker;

class SCR_CampaignSeizingComponent : SCR_SeizingComponent
{
	[Attribute("60", params: "0 inf 0.1", category: "Campaign")]
	protected float m_fExtraTimePerService;
	
	[Attribute("60", params: "0 inf 0.1", category: "Campaign")]
	protected float m_fExtraTimePerRadioConnection;

	protected SCR_CampaignMilitaryBaseComponent m_Base;
	
	protected SCR_EBaseCaptureState m_eCaptureState;
	protected ref OnCaptureStateChangedInvoker m_OnCaptureStateChanged;

	[RplProp()]
	protected int m_iContestingFactionsCount;

	//------------------------------------------------------------------------------------------------
	protected override SCR_Faction EvaluateEntityFaction(IEntity ent)
	{
		if (!m_Base || m_Base.IsHQ() || !m_Base.IsInitialized())
			return null;
		
		SCR_Faction faction = super.EvaluateEntityFaction(ent);
		
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
			if (m_eCaptureState != SCR_EBaseCaptureState.NONE)
			{
				m_eCaptureState = SCR_EBaseCaptureState.NONE;
				Rpc(RpcDo_OnCaptureStateChanged);
				RpcDo_OnCaptureStateChanged();
			}

			if (m_PrevailingFaction)
			{
				m_PrevailingFactionPrevious = m_PrevailingFaction;
				m_iSeizingCharacters = 0;
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

		int totalFactionPresenceCount, maxFactionPresence;
		m_iContestingFactionsCount = 0;
		bool multipleMaxPresence;
		foreach (SCR_Faction faction, int presence : factionsPresence)
		{
			totalFactionPresenceCount += presence;
			m_iContestingFactionsCount++;

			if (presence > maxFactionPresence)
			{
				maxFactionPresence = presence;
				multipleMaxPresence = false;
			}
			else if (presence == maxFactionPresence)
			{
				multipleMaxPresence = true;
			}
		}
		
		Replication.BumpMe();

		SCR_Faction defenderFaction = m_Base.GetCampaignFaction();
		if (defenderFaction && !factionsPresence.Contains(defenderFaction))
			factionsPresence.Insert(defenderFaction, 0); // defending faction should always be evaluated to make sure contested is registered properly.

		// prevailing faction is the faction that is able to capture the base, if the defenders are winning prevailing faction is null
		m_bDeleteDisabledAIs = false;
		SCR_Faction prevailingFaction;
		SCR_CampaignFaction cFaction;
		int curSeizingCharacters;

		SCR_EBaseCaptureState baseCaptureState = SCR_EBaseCaptureState.NONE;

		// Evaluate if capture, needs more than half the amount of players to capture!
		foreach (SCR_Faction faction, int presence : factionsPresence)
		{
			if (faction == defenderFaction)
			{
				// if its equal, it will go to the defenders!
				if (presence * 2 >= totalFactionPresenceCount)
				{
					prevailingFaction = null;
					curSeizingCharacters = 0;
				}
				else
				{
					// no cap < contested < capture in terms of relevancy so we make sure contested doesnt overwrite capture state
					if (baseCaptureState == SCR_EBaseCaptureState.NONE)
						baseCaptureState = SCR_EBaseCaptureState.CONTESTED;
				}
			}
			else
			{
				if (!faction.CanCaptureBases())
					continue;

				// Non-playable attackers are not allowed
				if (m_bIgnoreNonPlayableAttackers && !faction.IsPlayable())
					continue;

				// In case players are required but are not present, ignore this faction for attacking
				if (m_bCapturingRequiresPlayer && !factionsPlayerPresence.Get(faction))
					continue;

				// Needs to have superior presence in the area, and radio coverage to be able to capture
				cFaction = SCR_CampaignFaction.Cast(faction);
				if (presence == maxFactionPresence && !multipleMaxPresence && cFaction && (!cFaction.IsPlayable() || m_Base.IsHQRadioTrafficPossible(cFaction)))
				{
					prevailingFaction = faction;
					curSeizingCharacters = presence;
					baseCaptureState = SCR_EBaseCaptureState.CAPTURING;
				}
			}
		}

		// Get amount of effectively seizing characters (clamp for max attackers attribute or set to 0 if there is no prevailing faction)
		if (prevailingFaction)
			curSeizingCharacters = Math.Min(totalFactionPresenceCount - curSeizingCharacters, m_iMaximumSeizingCharacters);

		if (baseCaptureState != m_eCaptureState)
		{
			m_eCaptureState = baseCaptureState;
			Rpc(RpcDo_OnCaptureStateChanged);
			RpcDo_OnCaptureStateChanged();
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
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_OnCaptureStateChanged()
	{
		if (m_OnCaptureStateChanged)
			m_OnCaptureStateChanged.Invoke(m_eCaptureState, this);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	OnCaptureStateChangedInvoker GetOnCaptureStateChanged()
	{
		if (!m_OnCaptureStateChanged)
			m_OnCaptureStateChanged = new OnCaptureStateChangedInvoker();

		return m_OnCaptureStateChanged;
	}

	//------------------------------------------------------------------------------------------------
	int GetContestingFactionsCount()
	{
		return m_iContestingFactionsCount;
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

[EnumLinear()]
enum SCR_EBaseCaptureState
{
	// No capture happening
	NONE,
	// Contested = the amount of defenders are less than the combined amount of attackers from other factions
	CONTESTED,
	// Base is being captured by another faction
	CAPTURING
}
