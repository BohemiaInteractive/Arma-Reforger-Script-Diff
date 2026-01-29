void OnSignalChangedDelegate(SCR_CampaignMilitaryBaseComponent base);
void OnAllBasesInitializedDelegate();
void OnLocalPlayerEnteredBaseDelegate(SCR_CampaignMilitaryBaseComponent base);
void OnLocalPlayerLeftBaseDelegate(SCR_CampaignMilitaryBaseComponent base);
void OnLocalFactionCapturedBaseDelegate();
void OnBaseBuiltDelegate(SCR_CampaignMilitaryBaseComponent base, Faction faction);

typedef func OnSignalChangedDelegate;
typedef func OnAllBasesInitializedDelegate;
typedef func OnLocalPlayerEnteredBaseDelegate;
typedef func OnLocalPlayerLeftBaseDelegate;
typedef func OnLocalFactionCapturedBaseDelegate;
typedef func OnBaseBuiltDelegate;

typedef ScriptInvokerBase<OnSignalChangedDelegate> OnSignalChangedInvoker;
typedef ScriptInvokerBase<OnAllBasesInitializedDelegate> OnAllBasesInitializedInvoker;
typedef ScriptInvokerBase<OnLocalPlayerEnteredBaseDelegate> OnLocalPlayerEnteredBaseInvoker;
typedef ScriptInvokerBase<OnLocalPlayerLeftBaseDelegate> OnLocalPlayerLeftBaseInvoker;
typedef ScriptInvokerBase<OnLocalFactionCapturedBaseDelegate> OnLocalFactionCapturedBaseInvoker;
typedef ScriptInvokerBase<OnBaseBuiltDelegate> OnBaseBuiltInvoker;

//------------------------------------------------------------------------------------------------
//! Created in SCR_GameModeCampaign
class SCR_CampaignMilitaryBaseManager
{
	protected static const int PARENT_BASE_DISTANCE_THRESHOLD = 300;			//AI patrols closer than this to a base will couterattack
	protected static const int HQ_NO_REMNANTS_RADIUS = 300;						//AI patrols closer than this to main HQs will be removed
	protected static const int HQ_NO_REMNANTS_PATROL_RADIUS = 600;				//AI patrols with a waypoint which is closer than this to main HQs will be removed
	protected static const int MAX_HQ_SELECTION_ITERATIONS = 20;
	protected static const int DEPOT_PLAYER_PRESENCE_CHECK_INTERVAL = 2000;		//ms
	protected static const float CP_AVG_DISTANCE_TOLERANCE = 0.25;				//highest relative distance tolerance to control points when evaluating main HQs
	protected static const string ICON_NAME_SUPPLIES = "Slot_Supplies";
	protected static const float MAX_DIST_TO_BASE = 300;
	protected static const float PARKED_LIFETIME = 3600;

	protected SCR_GameModeCampaign m_Campaign;

	protected SCR_CampaignFaction m_LocalPlayerFaction;

	protected ref array<SCR_CampaignMilitaryBaseComponent> m_aBases = {};
	protected ref array<SCR_CampaignMilitaryBaseComponent> m_aControlPoints = {};
	protected ref array<SCR_CampaignSuppliesComponent> m_aRemnantSupplyDepots = {};

	protected ref OnSignalChangedInvoker m_OnSignalChanged;
	protected ref OnAllBasesInitializedInvoker m_OnAllBasesInitialized;
	protected ref OnLocalPlayerEnteredBaseInvoker m_OnLocalPlayerEnteredBase;
	protected ref OnLocalPlayerLeftBaseInvoker m_OnLocalPlayerLeftBase;
	protected ref OnBaseBuiltInvoker m_OnBaseBuilt;

	protected int m_iActiveBases;
	protected int m_iTargetActiveBases;
	protected int m_iMaxAvailableCallsignsAmount;

	protected ref map<FactionKey, int> m_mFactionEstablishedBasesAmount = new map<FactionKey, int>();

	protected bool m_bAllBasesInitialized;

	//------------------------------------------------------------------------------------------------
	//! Calculates the maximum amount of available callsigns for establishing of new bases
	protected void CalculateMaxAvailableCallsignAmount()
	{
		FactionManager factionManager = GetGame().GetFactionManager();
		array<Faction> factions = {};
		factionManager.GetFactionsList(factions);
		SCR_CampaignFaction campaignFaction;
		array<int> baseCallsignIndexes = {};

		int minBaseCallsignCount = int.MAX;
		foreach (Faction faction : factions)
		{
			campaignFaction = SCR_CampaignFaction.Cast(faction);
			if (!campaignFaction)
				continue;

			if (!campaignFaction.IsPlayable())
				continue;

			baseCallsignIndexes = campaignFaction.GetBaseCallsignIndexes();

			// Skip faction that does not use any callsigns
			if (baseCallsignIndexes.IsEmpty())
				continue;

			minBaseCallsignCount = Math.Min(minBaseCallsignCount, baseCallsignIndexes.Count());
			m_mFactionEstablishedBasesAmount.Insert(campaignFaction.GetFactionKey(), 0);
		}

		int predefinedBaseCallsignCount;
		foreach (SCR_CampaignMilitaryBaseComponent base : m_aBases)
		{
			// Skip uninitialized bases
			if (!base.IsInitialized())
				continue;

			// Relays do not have callsigns
			if (base.GetType() == SCR_ECampaignBaseType.RELAY)
				continue;

			if (base.GetBuiltByPlayers())
				continue;

			predefinedBaseCallsignCount++;
		}

		m_iMaxAvailableCallsignsAmount = minBaseCallsignCount - predefinedBaseCallsignCount;
	}

	//------------------------------------------------------------------------------------------------
	//! Goes through all bases and counts the amount of established bases for each faction
	protected void CountFactionEstablishedBasesAmount()
	{
		FactionKey baseFactionKey;
		int factionBuiltBases;

		foreach (SCR_CampaignMilitaryBaseComponent base : m_aBases)
		{
			if (!base.GetBuiltByPlayers())
				continue;

			baseFactionKey = base.GetBuiltFaction();
			if (baseFactionKey.IsEmpty())
				continue;

			factionBuiltBases = m_mFactionEstablishedBasesAmount.Get(baseFactionKey) + 1;

			m_mFactionEstablishedBasesAmount.Set(baseFactionKey, factionBuiltBases);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] faction
	//! \return false if no callsigns available or game mode limit for established bases reached, true otherwise
	bool CanFactionBuildNewBase(notnull Faction faction)
	{
		// Establishing bases is not enabled at all
		if (!m_Campaign.GetEstablishingBasesEnabled())
			return false;

		int factionsWithBuiltBases = m_mFactionEstablishedBasesAmount.Count();
		if (factionsWithBuiltBases == 0)
			return true;

		int builtBases = m_mFactionEstablishedBasesAmount.Get(faction.GetFactionKey());
		int gameModeLimit = m_Campaign.GetFactionEstablishBaseLimit();

		// When Gamemode limit is -1, establishing bases is only limited by callsign availability
		if (gameModeLimit == -1)
			return m_iMaxAvailableCallsignsAmount / factionsWithBuiltBases > builtBases;
		else
			return Math.Min(gameModeLimit, m_iMaxAvailableCallsignsAmount / factionsWithBuiltBases) > builtBases;
	}

	//------------------------------------------------------------------------------------------------
	OnBaseBuiltInvoker GetOnBaseBuilt()
	{
		if (!m_OnBaseBuilt)
			m_OnBaseBuilt = new OnBaseBuiltInvoker();

		return m_OnBaseBuilt;
	}

	//------------------------------------------------------------------------------------------------
	int GetBases(notnull out array<SCR_CampaignMilitaryBaseComponent> bases, Faction faction = null)
	{
		bases.Clear();
		foreach (SCR_CampaignMilitaryBaseComponent theBase : m_aBases)
		{
			if (!theBase)
				continue;

			if (theBase.GetType() != SCR_ECampaignBaseType.BASE && theBase.GetType() != SCR_ECampaignBaseType.SOURCE_BASE)
				continue;

			if (faction && theBase.GetFaction() != faction)
				continue;

			bases.Insert(theBase);
		}

		return bases.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! Bases which have been initialized
	int GetActiveBasesCount()
	{
		return m_iActiveBases;
	}

	//------------------------------------------------------------------------------------------------
	//! Total bases expected to be initialized
	int GetTargetActiveBasesCount()
	{
		return m_iTargetActiveBases;
	}

	//------------------------------------------------------------------------------------------------
	void AddActiveBase()
	{
		if (++m_iActiveBases == m_iTargetActiveBases)
			OnAllBasesInitialized();
	}

	//------------------------------------------------------------------------------------------------
	void SetTargetActiveBasesCount(int count)
	{
		m_iTargetActiveBases = count;
	}

	//------------------------------------------------------------------------------------------------
	OnLocalPlayerEnteredBaseInvoker GetOnLocalPlayerEnteredBase()
	{
		if (!m_OnLocalPlayerEnteredBase)
			m_OnLocalPlayerEnteredBase = new OnLocalPlayerEnteredBaseInvoker();

		return m_OnLocalPlayerEnteredBase;
	}

	//------------------------------------------------------------------------------------------------
	OnLocalPlayerLeftBaseInvoker GetOnLocalPlayerLeftBase()
	{
		if (!m_OnLocalPlayerLeftBase)
			m_OnLocalPlayerLeftBase = new OnLocalPlayerLeftBaseInvoker();

		return m_OnLocalPlayerLeftBase;
	}

	//------------------------------------------------------------------------------------------------
	//! Triggered when all bases have been successfully initialized
	OnAllBasesInitializedInvoker GetOnAllBasesInitialized()
	{
		if (!m_OnAllBasesInitialized)
			m_OnAllBasesInitialized = new OnAllBasesInitializedInvoker();

		return m_OnAllBasesInitialized;
	}

	//------------------------------------------------------------------------------------------------
	//! Triggered when a base's radio coverage changes
	OnSignalChangedInvoker GetOnSignalChanged()
	{
		if (!m_OnSignalChanged)
			m_OnSignalChanged = new OnSignalChangedInvoker();

		return m_OnSignalChanged;
	}

	//------------------------------------------------------------------------------------------------
	void OnAllBasesInitialized()
	{
		m_bAllBasesInitialized = true;

		UpdateBases();

		if (m_OnAllBasesInitialized)
			m_OnAllBasesInitialized.Invoke();

		if (RplSession.Mode() != RplMode.Dedicated)
		{
			InitializeSupplyDepotIcons();
			HideUnusedBaseIcons();
		}

		CalculateMaxAvailableCallsignAmount();
		CountFactionEstablishedBasesAmount();

		if (m_Campaign.IsProxy())
			return;

		RecalculateRadioCoverage(m_Campaign.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR));
		RecalculateRadioCoverage(m_Campaign.GetFactionByEnum(SCR_ECampaignFaction.OPFOR));
		EvaluateControlPoints();

		ProcessRemnantsPresence();
	}

	//------------------------------------------------------------------------------------------------
	bool IsBasesInitDone()
	{
		return m_bAllBasesInitialized;
	}

	//------------------------------------------------------------------------------------------------
	protected void DisableExtraSeizingComponents(SCR_MilitaryBaseComponent base, SCR_MilitaryBaseLogicComponent logic)
	{
		if (logic.Type() == SCR_SeizingComponent)
			SCR_SeizingComponent.Cast(logic).Disable();
	}

	//------------------------------------------------------------------------------------------------
	void SetLocalPlayerFaction(notnull SCR_CampaignFaction faction)
	{
		m_LocalPlayerFaction = faction;
	}

	//------------------------------------------------------------------------------------------------
	SCR_CampaignFaction GetLocalPlayerFaction()
	{
		return m_LocalPlayerFaction;
	}

	//------------------------------------------------------------------------------------------------
	//! Update the list of Conflict bases
	int UpdateBases(bool refreshTargetCount = false)
	{
		SCR_RadioCoverageSystem.UpdateAll();
		SCR_MilitaryBaseSystem baseManager = SCR_MilitaryBaseSystem.GetInstance();
		array<SCR_MilitaryBaseComponent> bases = {};
		baseManager.GetBases(bases);

		m_aBases.Clear();
		m_aControlPoints.Clear();

		if (refreshTargetCount)
			m_iTargetActiveBases = 0;

		foreach (SCR_MilitaryBaseComponent base : bases)
		{
			SCR_CampaignMilitaryBaseComponent campaignBase = SCR_CampaignMilitaryBaseComponent.Cast(base);
			if (!campaignBase)
				continue;

			if (refreshTargetCount && campaignBase.IsInitialized())
				++m_iTargetActiveBases;

			m_aBases.Insert(campaignBase);

			if (campaignBase.IsControlPoint())
				m_aControlPoints.Insert(campaignBase);
		}

		return m_aBases.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! Picks Main Operating Bases from a list of candidates by checking average distance to active control points
	void SelectHQs(notnull array<SCR_CampaignMilitaryBaseComponent> candidates, notnull array<SCR_CampaignMilitaryBaseComponent> controlPoints, out notnull array<SCR_CampaignMilitaryBaseComponent> selectedHQs)
	{
		int candidatesCount = candidates.Count();
		if (candidatesCount < 2)
			return;

		// Pick the same HQs every time when debugging
		#ifdef ENABLE_DIAG
		if (SCR_RespawnComponent.Diag_IsCLISpawnEnabled())
		{
			SelectHQsSimple(candidates, selectedHQs);
			return;
		}
		#endif

		// If only two HQs are set up, don't waste time with processing
		if (candidatesCount == 2)
		{
			SelectHQsSimple(candidates, selectedHQs);
			return;
		}

		SCR_CampaignMilitaryBaseComponent bluforHQ;
		SCR_CampaignMilitaryBaseComponent opforHQ;
		array<SCR_CampaignMilitaryBaseComponent> preferredForHQ = {};

		// Pick one of the HQs at random
		bluforHQ = candidates.GetRandomElement();
		candidates.RemoveItem(bluforHQ);

		vector bluforHQPos = bluforHQ.GetOwner().GetOrigin();
		float distanceBetweenHQs;
		float acceptableDistanceBetweenHQs = m_Campaign.GetAcceptableDistanceBetweenFactionHQs() * m_Campaign.GetAcceptableDistanceBetweenFactionHQs();
		float preferredDistanceBetweenHQs = m_Campaign.GetPreferredDistanceBetweenFactionHQs() * m_Campaign.GetPreferredDistanceBetweenFactionHQs();

		foreach (SCR_CampaignMilitaryBaseComponent otherHQ : candidates)
		{
			// Candidates with a distance to first HQ greater than acceptableDistanceBetweenHQs are acceptable to be picked
			distanceBetweenHQs = vector.DistanceSqXZ(bluforHQPos, otherHQ.GetOwner().GetOrigin());
			if (distanceBetweenHQs > acceptableDistanceBetweenHQs)
			{
				preferredForHQ.Insert(otherHQ);

				// Candidates with a distance to first HQ greater than preferredDistanceBetweenHQs have double chance to be picked
				if (distanceBetweenHQs > preferredDistanceBetweenHQs)
					preferredForHQ.Insert(otherHQ);
			}
		}

		// In case none of the candidates are within the acceptable distance, pick any candidate
		if (preferredForHQ.IsEmpty())
			opforHQ = candidates.GetRandomElement();
		else
			opforHQ = preferredForHQ.GetRandomElement();

		// Randomly assign the factions in reverse in case primary selection gets too limited
		if (Math.RandomFloat01() >= 0.5)
			selectedHQs = {bluforHQ, opforHQ};
		else
			selectedHQs = {opforHQ, bluforHQ};
	}

	//------------------------------------------------------------------------------------------------
	//! If there are only two candidates for main HQ or the main process fails, HQs are selected simply and cheaply
	protected void SelectHQsSimple(notnull array<SCR_CampaignMilitaryBaseComponent> candidates, out notnull array<SCR_CampaignMilitaryBaseComponent> selectedHQs)
	{
		// Pick the same HQs every time when debugging
#ifdef ENABLE_DIAG
		if (SCR_RespawnComponent.Diag_IsCLISpawnEnabled())
		{
			selectedHQs = {candidates[0], candidates[1]};
			return;
		}
#endif

		// In Tutorial mode, we always want to use the same HQs
		if (m_Campaign.IsTutorial())
		{
			if (candidates[0].GetFaction(true) == m_Campaign.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR))
				selectedHQs = {candidates[0], candidates[1]};
			else
				selectedHQs = {candidates[1], candidates[0]};

			return;
		}

		SCR_CampaignMilitaryBaseComponent bluforHQ = candidates.GetRandomElement();
		candidates.RemoveItem(bluforHQ);
		SCR_CampaignMilitaryBaseComponent opforHQ = candidates.GetRandomElement();

		if (Math.RandomFloat01() >= 0.5)
			selectedHQs = {bluforHQ, opforHQ};
		else
			selectedHQs = {opforHQ, bluforHQ};
	}

	//------------------------------------------------------------------------------------------------
	void SetHQFactions(notnull array<SCR_CampaignMilitaryBaseComponent> selectedHQs)
	{
		SCR_CampaignFaction factionBLUFOR = m_Campaign.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR);
		SCR_CampaignFaction factionOPFOR = m_Campaign.GetFactionByEnum(SCR_ECampaignFaction.OPFOR);
		SCR_CampaignFaction factionINDFOR = m_Campaign.GetFactionByEnum(SCR_ECampaignFaction.INDFOR);

		if (selectedHQs[0].GetFaction() == selectedHQs[1].GetFaction())
		{
			// Preset owners are the same or null, assign new owners normally
			selectedHQs[0].SetFaction(factionBLUFOR);
			selectedHQs[1].SetFaction(factionOPFOR);
		}
		else
		{
			// Check if one of the preset owners is invalid, if yes, assign a new owner which is not assigned to the other HQ
			if (!selectedHQs[0].GetFaction() || selectedHQs[0].GetFaction() == factionINDFOR)
			{
				if (selectedHQs[1].GetFaction() == factionBLUFOR)
					selectedHQs[0].SetFaction(factionOPFOR);
				else
					selectedHQs[0].SetFaction(factionBLUFOR);
			}
			else if (!selectedHQs[1].GetFaction() || selectedHQs[1].GetFaction() == factionINDFOR)
			{
				if (selectedHQs[0].GetFaction() == factionBLUFOR)
					selectedHQs[1].SetFaction(factionOPFOR);
				else
					selectedHQs[1].SetFaction(factionBLUFOR);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Returns squared average distance to control points - used for starting HQ location calculations
	protected int GetAvgCPDistanceSq(notnull SCR_CampaignMilitaryBaseComponent HQ, notnull array<SCR_CampaignMilitaryBaseComponent> controlPoints)
	{
		int thresholdCP = m_Campaign.GetControlPointTreshold();

		// Avoid division by zero
		if (thresholdCP == 0)
			return 0;

		array<SCR_CampaignMilitaryBaseComponent> nearestControlPoints = {};

		int distanceToHQ;
		int controlPointsCount;
		int arrayIndex;
		int nearestControlPointsCount;

		vector HQPos = HQ.GetOwner().GetOrigin();

		foreach (SCR_CampaignMilitaryBaseComponent controlPoint : controlPoints)
		{
			if (!controlPoint)
				continue;

			distanceToHQ = vector.DistanceSqXZ(controlPoint.GetOwner().GetOrigin(), HQPos);
			controlPointsCount = nearestControlPoints.Count();
			arrayIndex = controlPointsCount;

			for (int i = 0; i < controlPointsCount; i++)
			{
				if (distanceToHQ < vector.DistanceSqXZ(HQPos, nearestControlPoints[i].GetOwner().GetOrigin()))
				{
					arrayIndex = i;
					break;
				}
			}

			nearestControlPointsCount = nearestControlPoints.InsertAt(controlPoint, arrayIndex);
		}

		// Avoid division by zero
		if (nearestControlPointsCount == 0)
			return 0;

		if (thresholdCP < nearestControlPointsCount)
			nearestControlPoints.Resize(thresholdCP);

		int totalDist;

		foreach (SCR_CampaignMilitaryBaseComponent controlPoint : nearestControlPoints)
		{
			totalDist += vector.DistanceSqXZ(HQPos, controlPoint.GetOwner().GetOrigin());
		}

		return totalDist / nearestControlPointsCount;
	}

	//------------------------------------------------------------------------------------------------
	void InitializeBases(notnull array<SCR_CampaignMilitaryBaseComponent> selectedHQs, bool randomizeSupplies)
	{
		array<SCR_CampaignMilitaryBaseComponent> basesSorted = {};
		SCR_CampaignMilitaryBaseComponent baseCheckedAgainst;
		vector originHQ1 = selectedHQs[0].GetOwner().GetOrigin();
		vector originHQ2 = selectedHQs[1].GetOwner().GetOrigin();
		float distanceToHQ;
		bool indexFound;
		int callsignIndex;
		array<int> allCallsignIndexes = {};
		array<int> callsignIndexesBLUFOR = m_Campaign.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR).GetBaseCallsignIndexes();
		SCR_CampaignFaction factionOPFOR = m_Campaign.GetFactionByEnum(SCR_ECampaignFaction.OPFOR);

		// Grab all valid base callsign indexes (if both factions have the index)
		foreach (int indexBLUFOR : callsignIndexesBLUFOR)
		{
			if (factionOPFOR.GetBaseCallsignByIndex(indexBLUFOR))
				allCallsignIndexes.Insert(indexBLUFOR);
		}

		int callsignsCount = allCallsignIndexes.Count();
		Faction defaultFaction;
		BaseRadioComponent radio;
		BaseTransceiver tsv;

		foreach (int iBase, SCR_CampaignMilitaryBaseComponent campaignBase : m_aBases)
		{
			if (!campaignBase.IsInitialized())
				continue;

			defaultFaction = campaignBase.GetFaction(true);

			// Apply default faction set in FactionAffiliationComponent or INDFOR if undefined
			if (!campaignBase.GetFaction())
			{
				if (defaultFaction)
					campaignBase.SetFaction(defaultFaction);
				else
					campaignBase.SetFaction(m_Campaign.GetFactionByEnum(SCR_ECampaignFaction.INDFOR));
			}

			// Assign callsign
			if (campaignBase.GetType() == SCR_ECampaignBaseType.RELAY)
			{
				// Relays use a dummy callsign just so search by callsign is still possible
				campaignBase.SetCallsignIndex(callsignsCount + iBase);
			}
			else
			{
				callsignIndex = allCallsignIndexes.GetRandomIndex();
				campaignBase.SetCallsignIndex(allCallsignIndexes[callsignIndex]);
				allCallsignIndexes.Remove(callsignIndex);
			}

			// Sort bases by distance to a HQ so randomized supplies can be applied fairly (if enabled)
			if (randomizeSupplies && campaignBase.GetType() == SCR_ECampaignBaseType.BASE)
			{
				indexFound = false;
				distanceToHQ = vector.DistanceSqXZ(originHQ1, campaignBase.GetOwner().GetOrigin());

				for (int i = 0, count = basesSorted.Count(); i < count; i++)
				{
					baseCheckedAgainst = basesSorted[i];

					if (distanceToHQ < vector.DistanceSqXZ(originHQ1, baseCheckedAgainst.GetOwner().GetOrigin()))
					{
						basesSorted.InsertAt(campaignBase, i);
						indexFound = true;
						break;
					}
				}

				if (!indexFound)
					basesSorted.Insert(campaignBase);
			}
		}

		if (randomizeSupplies)
			AddRandomSupplies(basesSorted, selectedHQs);
	}

	//------------------------------------------------------------------------------------------------
	//! Add randomized supplies to each base, calculate batches so each side encounters similarly stacked bases
	void AddRandomSupplies(notnull array<SCR_CampaignMilitaryBaseComponent> basesSorted, notnull array<SCR_CampaignMilitaryBaseComponent> selectedHQs)
	{
		array<int> suppliesBufferBLUFOR = {};
		array<int> suppliesBufferOPFOR = {};
		int intervalMultiplier = Math.Floor((m_Campaign.GetMaxStartingSupplies() - m_Campaign.GetMinStartingSupplies()) / m_Campaign.GetStartingSuppliesInterval());
		FactionKey factionToProcess;
		vector basePosition;
		float distanceToHQ1;
		float distanceToHQ2;
		int suppliesToAdd;

		foreach (SCR_CampaignMilitaryBaseComponent base : basesSorted)
		{
			if (base.IsHQ())
				continue;

			basePosition = base.GetOwner().GetOrigin();
			distanceToHQ1 = vector.DistanceSq(basePosition, selectedHQs[0].GetOwner().GetOrigin());
			distanceToHQ2 = vector.DistanceSq(basePosition, selectedHQs[1].GetOwner().GetOrigin());

			if (distanceToHQ1 > distanceToHQ2)
				factionToProcess = selectedHQs[1].GetCampaignFaction().GetFactionKey();
			else
				factionToProcess = selectedHQs[0].GetCampaignFaction().GetFactionKey();

			// Check if we have preset supplies stored in buffer
			if (factionToProcess == m_Campaign.GetFactionKeyByEnum(SCR_ECampaignFaction.BLUFOR) && !suppliesBufferBLUFOR.IsEmpty())
			{
				suppliesToAdd = suppliesBufferBLUFOR[0];
				suppliesBufferBLUFOR.RemoveOrdered(0);
			}
			else if (factionToProcess == m_Campaign.GetFactionKeyByEnum(SCR_ECampaignFaction.OPFOR) && !suppliesBufferOPFOR.IsEmpty())
			{
				suppliesToAdd = suppliesBufferOPFOR[0];
				suppliesBufferOPFOR.RemoveOrdered(0);
			}
			else
			{
				// Supplies from buffer not applied, add random amount, store to opposite faction's buffer
				suppliesToAdd = m_Campaign.GetMinStartingSupplies() + (m_Campaign.GetStartingSuppliesInterval() * Math.RandomIntInclusive(0, intervalMultiplier));

				if (factionToProcess == m_Campaign.GetFactionKeyByEnum(SCR_ECampaignFaction.BLUFOR))
					suppliesBufferOPFOR.Insert(suppliesToAdd);
				else
					suppliesBufferBLUFOR.Insert(suppliesToAdd);
			}

			base.SetStartingSupplies(suppliesToAdd);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Show icons only for supply depots close enough to an active base
	void InitializeSupplyDepotIcons()
	{
		IEntity depot;
		vector origin;
		MapItem item;
		SCR_CampaignMilitaryBaseComponent closestBase;
		MapDescriptorProps props;
		SCR_MapDescriptorComponent mapDescriptorComponent;
		int threshold = m_Campaign.GetSupplyDepotIconThreshold();
		Color colorFIA = m_Campaign.GetFactionByEnum(SCR_ECampaignFaction.INDFOR).GetFactionColor();

		foreach (SCR_CampaignSuppliesComponent comp : m_aRemnantSupplyDepots)
		{
			depot = comp.GetOwner();

			if (!depot)
				continue;

			mapDescriptorComponent = SCR_MapDescriptorComponent.Cast(depot.FindComponent(SCR_MapDescriptorComponent));
			if (!mapDescriptorComponent)
				continue;

			item = mapDescriptorComponent.Item();
			origin = depot.GetOrigin();
			closestBase = FindClosestBase(origin);
			if (!closestBase)
				continue;

			if (vector.Distance(origin, closestBase.GetOwner().GetOrigin()) <= threshold)
			{
				item.SetVisible(true);
				item.SetImageDef(ICON_NAME_SUPPLIES);

				props = item.GetProps();
				props.SetIconSize(32, 0.25, 0.25);
				props.SetFrontColor(colorFIA);
				props.SetTextVisible(false);
				props.Activate(true);
			}
			else
			{
				item.SetVisible(false);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void RegisterRemnantSupplyDepot(notnull SCR_CampaignSuppliesComponent comp)
	{
		m_aRemnantSupplyDepots.Insert(comp);
	}

	//------------------------------------------------------------------------------------------------
	void HideUnusedBaseIcons()
	{
		SCR_MapDescriptorComponent mapDescriptorComponent;
		MapItem item;

		foreach (SCR_CampaignMilitaryBaseComponent base : m_aBases)
		{
			if (base.IsInitialized())
				continue;

			mapDescriptorComponent = SCR_MapDescriptorComponent.Cast(base.GetOwner().FindComponent(SCR_MapDescriptorComponent));

			if (!mapDescriptorComponent)
				continue;

			item = mapDescriptorComponent.Item();

			if (!item)
				continue;

			item.SetVisible(false);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Determine the radio coverage of all bases (no coverage / can be reached / can respond / both ways)
	void RecalculateRadioCoverage(notnull SCR_CampaignFaction faction)
	{
		bool newSettingsDetected = SCR_RadioCoverageSystem.UpdateAll();
		if (newSettingsDetected)
			DelayedEvaluateControlPoints(0);
	}

	//------------------------------------------------------------------------------------------------
	//! Calls Evaluate Control Points with a delay. Should be used in need of a delay of the evalution so that Control Points are properly updated.
	//! \param[in] delay in ms
	void DelayedEvaluateControlPoints(int delay)
	{
		if (delay < 0)
			delay = 0;

		GetGame().GetCallqueue().CallLater(EvaluateControlPoints, delay);
	}

	//------------------------------------------------------------------------------------------------
	// Checks whether some faction is winning the game
	void EvaluateControlPoints()
	{
		FactionManager factionManager = GetGame().GetFactionManager();
		array<Faction> factions = {};
		factionManager.GetFactionsList(factions);

		int controlPointsHeld;
		int controlPointsContested;
		ChimeraWorld world = GetGame().GetWorld();
		WorldTimestamp currentTime = world.GetServerTimestamp();
		WorldTimestamp victoryTimestamp;
		WorldTimestamp blockPauseTimestamp;

		foreach (Faction faction : factions)
		{
			SCR_CampaignFaction fCast = SCR_CampaignFaction.Cast(faction);

			if (!fCast || !fCast.IsPlayable())
				continue;

			controlPointsHeld = 0;
			controlPointsContested = 0;

			// Update amount of control points currently held by this faction
			foreach (SCR_CampaignMilitaryBaseComponent controlPoint : m_aControlPoints)
			{
				if (controlPoint.IsInitialized() && controlPoint.GetFaction() == fCast && controlPoint.IsHQRadioTrafficPossible(fCast, SCR_ERadioCoverageStatus.RECEIVE))
				{
					controlPointsHeld++;

					if (controlPoint.GetCapturingFaction() && controlPoint.GetCapturingFaction() != fCast)
						controlPointsContested++
				}
			}

			m_Campaign.SetControlPointsHeld(fCast, controlPointsHeld);

			victoryTimestamp = fCast.GetVictoryTimestamp();
			blockPauseTimestamp = fCast.GetPauseByBlockTimestamp();
			int controlPointsThreshold = m_Campaign.GetControlPointTreshold();

			// Update timers (if a faction starts winning or a point is contested)
			if (controlPointsHeld >= controlPointsThreshold)
			{
				if ((controlPointsHeld - controlPointsContested) < controlPointsThreshold)
				{
					if (blockPauseTimestamp == 0)
						fCast.SetPauseByBlockTimestamp(currentTime);
				}
				else if (blockPauseTimestamp != 0)
				{
					fCast.SetVictoryTimestamp(currentTime.PlusMilliseconds(victoryTimestamp.DiffMilliseconds(blockPauseTimestamp)));
					fCast.SetPauseByBlockTimestamp(null);
				}

				if (victoryTimestamp == 0)
					fCast.SetVictoryTimestamp(currentTime.PlusSeconds(m_Campaign.GetVictoryTimer()));
			}
			else
			{
				fCast.SetVictoryTimestamp(null);
				fCast.SetPauseByBlockTimestamp(null);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void OnEnemyDetectedByDefenders(SCR_AIGroup group, SCR_AITargetInfo target, AIAgent reporter)
	{
		if (!m_aBases || !target || !target.m_Faction || !group)
			return;

		// Identify the base under attack, notify about it
		foreach (SCR_CampaignMilitaryBaseComponent base : m_aBases)
		{
			if (!base || !base.IsInitialized())
				continue;

			if (base.ContainsGroup(group))
			{
				base.NotifyAboutEnemyAttack(target.m_Faction);
				return;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	SCR_CampaignMilitaryBaseComponent FindClosestBase(vector position)
	{
		SCR_CampaignMilitaryBaseComponent closestBase;
		float closestBaseDistance = float.MAX;

		foreach (SCR_CampaignMilitaryBaseComponent base : m_aBases)
		{
			if (!base.IsInitialized())
				continue;

			float distance = vector.DistanceSq(base.GetOwner().GetOrigin(), position);

			if (distance < closestBaseDistance)
			{
				closestBaseDistance = distance;
				closestBase = base;
			}
		}

		return closestBase;
	}

	//------------------------------------------------------------------------------------------------
	SCR_CampaignMilitaryBaseComponent FindBaseByCallsign(int callsign)
	{
		if (callsign == SCR_MilitaryBaseComponent.INVALID_BASE_CALLSIGN)
			return null;

		foreach (SCR_CampaignMilitaryBaseComponent base : m_aBases)
		{
			if (!base)
				continue;

			if (base.GetCallsign() == callsign)
				return base;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	SCR_CampaignMilitaryBaseComponent FindBaseByPosition(vector position)
	{
		foreach (SCR_CampaignMilitaryBaseComponent base : m_aBases)
		{
			if (!base)
				continue;

			if (base.GetOwner().GetOrigin() == position)
				return base;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	SCR_CampaignSuppliesComponent FindClosestSupplyDepot(vector position)
	{
		SCR_CampaignSuppliesComponent closestDepot;
		float closestDepotDistance = float.MAX;

		foreach (SCR_CampaignSuppliesComponent depot : m_aRemnantSupplyDepots)
		{
			float distance = vector.DistanceSq(depot.GetOwner().GetOrigin(), position);

			if (distance < closestDepotDistance)
			{
				closestDepotDistance = distance;
				closestDepot = depot;
			}
		}

		return closestDepot;
	}

	//------------------------------------------------------------------------------------------------
	bool IsEntityInFactionRadioSignal(notnull IEntity entity, notnull Faction faction)
	{
		SCR_CampaignFaction factionC = SCR_CampaignFaction.Cast(faction);
		if (!factionC)
			return false;

		// Check if the entity is within range of deployed mobile HQ which is able to relay the signal
		SCR_CampaignMobileAssemblyStandaloneComponent mobileHQ = factionC.GetMobileAssembly();

		if (mobileHQ && mobileHQ.GetOwner() != entity && mobileHQ.IsInRadioRange())
		{
			if (vector.DistanceSq(entity.GetOrigin(), mobileHQ.GetOwner().GetOrigin()) < Math.Pow(mobileHQ.GetRadioRange(), 2))
				return true;
		}

		foreach (SCR_CampaignMilitaryBaseComponent base : m_aBases)
		{
			if (!base)
				continue;

			if (faction != base.GetFaction())
				continue;

			if (base.GetIsEntityInMyRange(entity) && base.IsHQRadioTrafficPossible(factionC))
				return true;
		}

		return false;
	}

    //------------------------------------------------------------------------------------------------
	//! \param[in] position
	//! \param[in] faction
	//! \param[in] signalRangeOffset
	//! \return true if a signal from HQ is available at the position
	bool IsPositionInFactionRadioSignal(vector position, notnull Faction faction, float signalRangeOffset = 0)
	{
		SCR_CampaignFaction campaignFaction = SCR_CampaignFaction.Cast(faction);
		if (!campaignFaction)
			return false;

		// Check if the entity is within range of deployed mobile HQ which is able to relay the signal
		SCR_CampaignMobileAssemblyStandaloneComponent mobileHQ = campaignFaction.GetMobileAssembly();
		if (mobileHQ && mobileHQ.IsInRadioRange())
		{
			if (vector.DistanceSq(position, mobileHQ.GetOwner().GetOrigin()) < Math.Pow(mobileHQ.GetRadioRange() + signalRangeOffset, 2))
				return true;
		}

		foreach (SCR_CampaignMilitaryBaseComponent base : m_aBases)
		{
			if (!base)
				continue;

			if (faction != base.GetFaction())
				continue;

			if (vector.DistanceSq(position, base.GetOwner().GetOrigin()) <= Math.Pow(base.GetRadioRange() + signalRangeOffset, 2) && base.IsHQRadioTrafficPossible(campaignFaction))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	//! Clean up ambient patrols around Main Operating Bases, assign parent bases where applicable
	void ProcessRemnantsPresence()
	{
		SCR_AmbientPatrolSystem manager = SCR_AmbientPatrolSystem.GetInstance();
		if (!manager)
			return;

		array<SCR_AmbientPatrolSpawnPointComponent> patrols = {};
		manager.GetPatrols(patrols);

		const int distLimit = Math.Pow(PARENT_BASE_DISTANCE_THRESHOLD, 2);
		float minDistance;
		SCR_CampaignMilitaryBaseComponent nearestBase;
		bool register = true;
		float dist;
		vector center;

		int distLimitHQ = Math.Pow(HQ_NO_REMNANTS_RADIUS, 2);
		int distLimitHQPatrol = Math.Pow(HQ_NO_REMNANTS_PATROL_RADIUS, 2);

		foreach (SCR_AmbientPatrolSpawnPointComponent patrol : patrols)
		{
			minDistance = float.MAX;
			register = true;
			center = patrol.GetOwner().GetOrigin();
			nearestBase = null;

			foreach (SCR_CampaignMilitaryBaseComponent base : m_aBases)
			{
				if (!base.IsInitialized() || base.GetType() == SCR_ECampaignBaseType.RELAY)
					continue;

				dist = vector.DistanceSqXZ(center, base.GetOwner().GetOrigin());

				if (base.IsHQ())
				{
					if (dist < distLimitHQ)
					{
						patrol.SetMembersAlive(0);
						register = false;
						break;
					}
					else if (dist < distLimitHQPatrol)
					{
						AIWaypointCycle waypoint = AIWaypointCycle.Cast(patrol.GetWaypoint());

						if (waypoint)
						{
							patrol.SetMembersAlive(0);
							register = false;
							break;
						}
					}
				}

				if (dist > distLimit || dist > minDistance)
					continue;

				if (!base.IsHQ())
				{
					nearestBase = base;
					minDistance = dist;
				}
				else
				{
					register = false;
					break;
				}
			}

			if (register && nearestBase)
				nearestBase.RegisterRemnants(patrol);
		}
	}

	//------------------------------------------------------------------------------------------------
	void OnPlayerDisconnected(int playerId)
	{
		// If the disconnecting player is currently capturing a base; handle it
		foreach (SCR_CampaignMilitaryBaseComponent base : m_aBases)
		{
			if (!base.IsInitialized())
				continue;

			if (base.GetCapturingFaction() && base.GetReconfiguredByID() == playerId)
			{
				base.EndCapture();
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void OnLocalPlayerPresenceChanged(notnull SCR_CampaignMilitaryBaseComponent base, bool present)
	{
		if (present)
		{
			if (m_OnLocalPlayerEnteredBase)
				m_OnLocalPlayerEnteredBase.Invoke(base);
		}
		else if (m_OnLocalPlayerLeftBase)
		{
			m_OnLocalPlayerLeftBase.Invoke(base);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnBaseFactionChanged(SCR_MilitaryBaseComponent base, Faction newFaction)
	{
		if (!m_Campaign.IsProxy())
			DelayedEvaluateControlPoints(0);
	}

	//------------------------------------------------------------------------------------------------
	void OnServiceBuilt(SCR_EServicePointStatus state, notnull SCR_ServicePointComponent serviceComponent)
	{

	}

	//------------------------------------------------------------------------------------------------
	void OnServiceRemoved(notnull SCR_MilitaryBaseComponent base, notnull SCR_MilitaryBaseLogicComponent service)
	{
		SCR_CampaignMilitaryBaseComponent campaignBase = SCR_CampaignMilitaryBaseComponent.Cast(base);

		if (!campaignBase)
			return;

		campaignBase.OnServiceRemoved(service);

		SCR_CatalogEntitySpawnerComponent spawner = SCR_CatalogEntitySpawnerComponent.Cast(service);

		if (!spawner)
			return;

		spawner.GetOnEntitySpawned().Remove(m_Campaign.OnEntityRequested);
	}

	//------------------------------------------------------------------------------------------------
	//! Called when a new AI group is spawned by Free Roam Building.
	void OnDefenderGroupSpawned(notnull SCR_MilitaryBaseLogicComponent service, notnull SCR_AIGroup group)
	{
		SCR_AIGroupUtilityComponent comp = SCR_AIGroupUtilityComponent.Cast(group.FindComponent(SCR_AIGroupUtilityComponent));

		if (!comp)
			return;

		ScriptInvokerBase<SCR_AIGroupPerceptionOnEnemyDetectedFiltered> onEnemyDetected = comp.m_Perception.GetOnEnemyDetectedFiltered();

		if (!onEnemyDetected)
			return;

		onEnemyDetected.Insert(OnEnemyDetectedByDefenders);

		array<SCR_MilitaryBaseComponent> bases = {};
		service.GetBases(bases);

		foreach (SCR_MilitaryBaseComponent base : bases)
		{
			SCR_CampaignMilitaryBaseComponent campaignBase = SCR_CampaignMilitaryBaseComponent.Cast(base);

			if (!campaignBase)
				continue;

			campaignBase.SetDefendersGroup(group);
		}
	}

	//------------------------------------------------------------------------------------------------
	void OnConflictStarted()
	{
	}

	//------------------------------------------------------------------------------------------------
	SCR_CampaignMilitaryBaseComponent SelectAndReturnPrimaryTarget(notnull SCR_CampaignFaction faction)
	{
		array<SCR_CampaignMilitaryBaseComponent> controlPointsInRange = {};

		// Get all Control Points which are now available for capture
		foreach (SCR_CampaignMilitaryBaseComponent base : m_aControlPoints)
		{
			if (!base.IsInitialized() || base.IsHQ())
				continue;

			if (base.GetFaction() == faction)
				continue;

			if (!base.IsHQRadioTrafficPossible(faction))
				continue;

			controlPointsInRange.Insert(base);
		}

		SCR_CampaignMilitaryBaseComponent target;
		int minDistance = int.MAX;

		// If there are some Control Points in radio range, find the closest one
		if (!controlPointsInRange.IsEmpty())
		{
			array<SCR_CampaignMilitaryBaseComponent> ownedBases = {};

			// Get all bases the given faction currently holds
			foreach (SCR_CampaignMilitaryBaseComponent base : m_aBases)
			{
				if (!base.IsInitialized())
					continue;

				if (base.GetFaction() != faction)
					continue;

				if (!base.IsHQRadioTrafficPossible(faction))
					continue;

				ownedBases.Insert(base);
			}

			foreach (SCR_CampaignMilitaryBaseComponent controlPoint : controlPointsInRange)
			{
				vector positionCP = controlPoint.GetOwner().GetOrigin();

				foreach (SCR_CampaignMilitaryBaseComponent base : ownedBases)
				{
					int distance = vector.DistanceSqXZ(base.GetOwner().GetOrigin(), positionCP);

					if (distance > minDistance)
						continue;

					minDistance = distance;
					target = controlPoint;
				}
			}
		}
		else	// Otherwise, find the Control Point closest to one of the capturable bases
		{
			array<SCR_CampaignMilitaryBaseComponent> basesInRange = {};

			// Get all bases which are now available for capture
			foreach (SCR_CampaignMilitaryBaseComponent base : m_aBases)
			{
				if (!base.IsInitialized() || base.IsHQ())
					continue;

				if (base.GetFaction() == faction)
					continue;

				if (!base.IsHQRadioTrafficPossible(faction))
					continue;

				basesInRange.Insert(base);
			}

			foreach (SCR_CampaignMilitaryBaseComponent controlPoint : m_aControlPoints)
			{
				if (!controlPoint.IsInitialized() || controlPoint.IsHQ())
					continue;

				if (controlPoint.GetFaction() == faction)
					continue;

				vector positionCP = controlPoint.GetOwner().GetOrigin();

				foreach (SCR_CampaignMilitaryBaseComponent base : basesInRange)
				{
					int distance = vector.DistanceSqXZ(base.GetOwner().GetOrigin(), positionCP);
					bool closer = distance < minDistance;
					bool coversControlPoint = base.CanReachByRadio(controlPoint.GetOwner());

					if (!coversControlPoint && !closer)
						continue;

					minDistance = distance;
					target = base;
				}
			}
		}

		return target;
	}

	//------------------------------------------------------------------------------------------------
	protected void DisablePatrolSpawn(IEntity entity)
	{
		if (!entity)
			return;

		array<Managed> outComponents = {};
		IEntity child = entity.GetChildren();
		while (child)
		{
			Managed entityComponent = child.FindComponent(SCR_AmbientPatrolSpawnPointComponent);
			if (entityComponent)
				outComponents.Insert(entityComponent);

			child = child.GetSibling();
		}

		SCR_AmbientPatrolSpawnPointComponent component;
		foreach (Managed outComponent : outComponents)
		{
			component = SCR_AmbientPatrolSpawnPointComponent.Cast(outComponent);
			if (!component)
				continue;

			component.SetMembersAlive(0);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void CreateCampaignMilitaryBase(notnull SCR_MilitaryBaseComponent base)
	{
		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		base.GetOwner().GetTransform(params.Transform);

		const IEntity entity = GetGame().SpawnEntityPrefab(Resource.Load("{1391CE8C0E255636}Prefabs/Systems/MilitaryBase/ConflictMilitaryBase.et"), null, params);
		SCR_CampaignMilitaryBaseComponent campaignBase = SCR_CampaignMilitaryBaseComponent.Cast(entity.FindComponent(SCR_CampaignMilitaryBaseComponent));
		if (campaignBase)
		{
			campaignBase.SetFaction(SCR_CampaignFaction.Cast(base.GetFaction(true)));

			// The base was not linked yet (e.g. player built), attempt to link it now
			const IEntity baseComposition = base.GetOwner().GetParent();
			if (baseComposition)
				campaignBase.SetBaseBuildingComposition(baseComposition);
		}

		DisablePatrolSpawn(entity);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnBaseRegistered(notnull SCR_MilitaryBaseComponent base)
	{
		SCR_CampaignMilitaryBaseComponent campaignBase = SCR_CampaignMilitaryBaseComponent.Cast(base);
		if (campaignBase)
			return;

		if (GetGame().GetWorld().GetWorldTime() < SCR_GameModeCampaign.BACKEND_DELAY)
			return;

		GetGame().GetCallqueue().Call(CreateCampaignMilitaryBase, base);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnBaseUnregistered(SCR_MilitaryBaseComponent base)
	{
		SCR_CampaignMilitaryBaseComponent campaignBase = SCR_CampaignMilitaryBaseComponent.Cast(base);
		if (!campaignBase)
			return;

		// HQ candidates are unregistered before active bases is set
		if (campaignBase.CanBeHQ() && !campaignBase.IsHQ() && (m_iTargetActiveBases <= 0 || m_Campaign.IsProxy()))
			return;

		string factionKey = campaignBase.GetBuiltFaction();
		if (campaignBase.GetBuiltByPlayers() && !factionKey.IsEmpty())
			m_mFactionEstablishedBasesAmount.Set(factionKey, m_mFactionEstablishedBasesAmount.Get(factionKey) - 1);

		m_iActiveBases--;
		m_iTargetActiveBases--;
	}

	//------------------------------------------------------------------------------------------------
	void OnBaseInitialized(notnull SCR_CampaignMilitaryBaseComponent base)
	{
		if (!m_bAllBasesInitialized)
			return;

		if (!m_Campaign.IsProxy())
		{
			array<int> callsignsPool = {};

			array<int> callsignIndexesBLUFOR = m_Campaign.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR).GetBaseCallsignIndexes();
			SCR_CampaignFaction factionOPFOR = m_Campaign.GetFactionByEnum(SCR_ECampaignFaction.OPFOR);

			// Grab all valid base callsign indexes (if both factions have the index)
			foreach (int indexBLUFOR : callsignIndexesBLUFOR)
			{
				if (factionOPFOR.GetBaseCallsignByIndex(indexBLUFOR))
					callsignsPool.Insert(indexBLUFOR);
			}

			// Ignore callsigns which are already assigned to other bases
			foreach (SCR_CampaignMilitaryBaseComponent existingBase : m_aBases)
			{
				if (!existingBase.IsInitialized())
					continue;

				callsignsPool.RemoveItem(existingBase.GetCallsign());
			}

			base.SetCallsignIndex(callsignsPool.GetRandomElement());
			base.OnCallsignAssigned();

			base.SetBuiltByPlayers(true);
			base.SetBuiltFaction(base.GetFaction());
			base.Initialize();

			GetGame().GetCallqueue().CallLater(RecalculateRadioCoverageForced, SCR_GameModeCampaign.MINIMUM_DELAY, false, m_Campaign.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR));
			GetGame().GetCallqueue().CallLater(RecalculateRadioCoverageForced, SCR_GameModeCampaign.MINIMUM_DELAY, false, m_Campaign.GetFactionByEnum(SCR_ECampaignFaction.OPFOR));
		}

		UpdateBases(true);
		SetFactionBaseEstablished(base.GetFaction());

		if (m_OnBaseBuilt)
			m_OnBaseBuilt.Invoke(base, base.GetCampaignFaction());
	}

	//------------------------------------------------------------------------------------------------
	//! Increments faction established bases counter with newly established base
	//! \param[in] faction which built the base
	protected void SetFactionBaseEstablished(Faction faction)
	{
		FactionKey factionKey = faction.GetFactionKey();
		m_mFactionEstablishedBasesAmount.Set(factionKey, m_mFactionEstablishedBasesAmount.Get(factionKey) + 1);
	}

	//------------------------------------------------------------------------------------------------
	//! Determine the radio coverage of all bases (no coverage / can be reached / can respond / both ways)
	void RecalculateRadioCoverageForced(notnull SCR_CampaignFaction faction)
	{
		bool newSettingsDetected = SCR_RadioCoverageSystem.UpdateAll(true);

		if (newSettingsDetected)
			EvaluateControlPoints();
	}

	//------------------------------------------------------------------------------------------------
	void SCR_CampaignMilitaryBaseManager(notnull SCR_GameModeCampaign campaign)
	{
		m_Campaign = campaign;
		SCR_MilitaryBaseSystem baseManager = SCR_MilitaryBaseSystem.GetInstance();

		if (!baseManager)
			return;

		baseManager.GetOnLogicUnregisteredInBase().Insert(OnServiceRemoved);
		baseManager.GetOnBaseFactionChanged().Insert(OnBaseFactionChanged);
		baseManager.GetOnBaseUnregistered().Insert(OnBaseUnregistered);

		m_Campaign.GetOnStarted().Insert(OnConflictStarted);

		if (!Replication.IsServer())
			return;

		const SCR_MilitaryBaseSystem militaryBaseSystem = SCR_MilitaryBaseSystem.GetInstance();
		if (militaryBaseSystem)
		{
			militaryBaseSystem.GetOnLogicRegisteredInBase().Insert(DisableExtraSeizingComponents);
			militaryBaseSystem.GetOnBaseRegistered().Insert(OnBaseRegistered);
		}
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_CampaignMilitaryBaseManager()
	{
		//Unregister from script invokers
		SCR_MilitaryBaseSystem baseManager = SCR_MilitaryBaseSystem.GetInstance();

		if (baseManager)
		{
			baseManager.GetOnLogicUnregisteredInBase().Remove(OnServiceRemoved);
			baseManager.GetOnBaseFactionChanged().Remove(OnBaseFactionChanged);
			baseManager.GetOnBaseUnregistered().Remove(OnBaseUnregistered);
		}

		if (m_Campaign)
			m_Campaign.GetOnStarted().Remove(OnConflictStarted);

		const SCR_MilitaryBaseSystem militaryBaseSystem = SCR_MilitaryBaseSystem.GetInstance();
		if (militaryBaseSystem)
		{
			militaryBaseSystem.GetOnLogicRegisteredInBase().Remove(DisableExtraSeizingComponents);
			militaryBaseSystem.GetOnBaseRegistered().Remove(OnBaseRegistered);
		}
	}
}
