class SCR_CampaignMilitaryBaseComponentClass : SCR_MilitaryBaseComponentClass
{
	[Attribute("{F7E8D4834A3AFF2F}UI/Imagesets/Conflict/conflict-icons-bw.imageset", UIWidgets.ResourceNamePicker, "Imageset with foreground textures", "imageset", category: "Campaign")]
	ResourceName m_sBuildingIconImageset;

	//------------------------------------------------------------------------------------------------
	//! \return
	ResourceName GetBuildingIconImageset()
	{
		return m_sBuildingIconImageset;
	}
}

void OnSpawnPointAssignedDelegate(SCR_SpawnPoint spawnpoint);
typedef func OnSpawnPointAssignedDelegate;
typedef ScriptInvokerBase<OnSpawnPointAssignedDelegate> OnSpawnPointAssignedInvoker;

void OnBaseStateChangedDelegate(SCR_CampaignMilitaryBaseComponent base, Faction defendingFaction, Faction attackingFaction);
typedef func OnBaseStateChangedDelegate;
typedef ScriptInvokerBase<OnBaseStateChangedDelegate> OnBaseStateChangedInvoker;

void OnSupplyLimitChanged(SCR_CampaignMilitaryBaseComponent campaignBase, float supplyLimit);
typedef func OnSupplyLimitChangedDelegate;
typedef ScriptInvokerBase<OnSupplyLimitChangedDelegate> OnSupplyLimitChangedInvoker;

void OnBaseAttackEndDelegate(SCR_CampaignMilitaryBaseComponent base);
typedef func OnBaseAttackEndDelegate;
typedef ScriptInvokerBase<OnBaseAttackEndDelegate> OnBaseAttackEndInvoker;

class SCR_CampaignMilitaryBaseComponent : SCR_MilitaryBaseComponent
{
	[Attribute("0", category: "Campaign"), RplProp()]
	protected bool m_bIsControlPoint;

	[Attribute("0", desc: "Can this base be picked as a faction's main base?", category: "Campaign")]
	protected bool m_bCanBeHQ;

	[Attribute("1", category: "Campaign")]
	protected bool m_bDisableWhenUnusedAsHQ;

	[Attribute("0", desc: "If enabled, this base will be treated as an HQ when handling supply income.", category: "Campaign")]
	protected bool m_bIsSupplyHub;

	[Attribute("#AR-MapSymbol_Installation", desc: "The display name of this base.", category: "Campaign")]
	protected string m_sBaseName;

	[Attribute("#AR-MapSymbol_Installation", desc: "The display name of this base, in upper case.", category: "Campaign")]
	protected string m_sBaseNameUpper;

	[Attribute("", desc: "Name of associated map location (to hide its label)", category: "Campaign")]
	protected string m_sMapLocationName;

	[Attribute(defvalue: "0", uiwidget: UIWidgets.ComboBox, desc: "Type", category: "Campaign", enumType:SCR_ECampaignBaseType)]
	protected SCR_ECampaignBaseType m_eType;

	[Attribute("true", desc: "Enables or disables the automatic creation of resupply tasks upon reaching the supply limit threshold.", category: "Campaign")]
	protected bool m_bIsResupplyTaskCreationEnabled;

	[Attribute("60", desc: "Defines how much time must elapse before a player can spawn at this base after it has been captured. [s]", category: "Campaign", params: "0 inf")]
	protected float m_fRespawnDelayAfterCaptureSeconds;

	[Attribute("-1", desc: "How many supplies are periodically replenished. If value is -1, default Gamemode value will be used.", params: "-1 inf 1", category: "Campaign")]
	protected int m_iRegularSuppliesIncomeBase;

	[Attribute("-1", desc: "How often are supplies replenished in bases [s]. If value is -1, default Gamemode value will be used.", params: "-1 inf 1", category: "Campaign")]
	protected int m_iSuppliesArrivalInterval;

	[RplProp(), Attribute("0.2", UIWidgets.Slider, "Defines the supply limit as a percentage of total storage capacity (TSC). If the supply falls below this threshold, a resupply request is triggered.", params: "0, 0.9, 0.01", category: "Campaign")]
	protected float m_fSupplyLimit;

	[RplProp(), Attribute("0.5", UIWidgets.Slider, "Defines the amount of supplies that are reserved to be stored inside the base.", params: "0.1 0.9 0.1", category: "Campaign")]
	protected float m_fReservedSupplyAmount;

	[RplProp(onRplName: "SupplyRequestExecutionPriorityReplicated"), Attribute(defvalue: SCR_Enum.GetDefault(SCR_ESupplyRequestExecutionPriority.MEDIUM), uiwidget: UIWidgets.ComboBox, desc: "Indicates the priority of resupply task for base when resupply request is evaluated.", enumType: SCR_ESupplyRequestExecutionPriority, category: "Campaign")]
	protected SCR_ESupplyRequestExecutionPriority m_eSupplyRequestExecutionPriority;

	protected ref map<int, WorldTimestamp> m_mDefendersData = new map<int, WorldTimestamp>();

	static const float RADIO_RECONFIGURATION_DURATION = 10.0;
	static const float BARRACKS_REDUCED_DEPLOY_COST = 0.5;
	static const int UNDER_ATTACK_WARNING_PERIOD = 60;
	static const int INVALID_PLAYER_INDEX = -1;
	static const int INVALID_FACTION_INDEX = -1;
	static const int RESPAWN_DELAY_AFTER_CAPTURE = 180000;
	static const int RELAY_BASE_RADIUS = 100;

	static const int SUPPLY_DEPOT_CAPACITY = 4500;
	static const int DEFENDERS_CHECK_PERIOD = 30000;
	static const int DEFENDERS_REWARD_PERIOD = 120000;
	static const int DEFENDERS_REWARD_MULTIPLIER = 3;
	static const float SUPPLY_LIMIT_MIN = 0;
	static const float SUPPLY_LIMIT_MAX = 0.9;

	protected static const int TICK_TIME = 1;
	protected static const int CONTESTED_RESPAWN_DELAY = 35;
	protected static const int BASES_IN_RANGE_RESUPPLY_THRESHOLD = 5;	// max bases to be taken into account when calculating supply income
	protected static const float HQ_VEHICLE_SPAWN_RADIUS = 40;
	protected static const float HQ_VEHICLE_QUERY_SPACE = 6;

	protected ref OnSpawnPointAssignedInvoker m_OnSpawnPointAssigned;
	protected static ref OnBaseStateChangedInvoker m_OnFactionChangedExtended;
	protected static ref OnBaseStateChangedInvoker m_OnBaseUnderAttack;
	protected static ref OnBaseAttackEndInvoker m_OnBaseAttackEnd;

	protected ref array<SCR_AmbientPatrolSpawnPointComponent> m_aRemnants = {};
	protected ref array<IEntity> m_aStartingVehicles = {};
	protected ref array<SCR_ERadioMsg> m_aServiceBuiltMsgQueue = {};

	protected float m_fStartingSupplies;
	protected float m_fNextFrameCheck;
	protected float m_fTimer;
	protected float m_fRadioRangeDefault;
	protected WorldTimestamp m_fLastEnemyContactTimestamp;

	protected bool m_bLocalPlayerPresent;
	protected bool m_bWasHQSet;

	protected SCR_CampaignMilitaryBaseMapDescriptorComponent m_MapDescriptor;

	protected SCR_SpawnPoint m_SpawnPoint;

	protected SCR_CampaignFaction m_CapturingFaction;
	protected SCR_CampaignFaction m_OwningFactionPrevious;

	protected SCR_CoverageRadioComponent m_RadioComponent;

	protected SCR_CampaignMapUIBase m_UIElement;

	protected SCR_TimedWaypoint m_SeekDestroyWP;

	protected ref array<SCR_AIGroup> m_aDefendersGroups = {};

	protected IEntity m_BaseBuildingComposition;

	protected ref OnSupplyLimitChangedInvoker m_OnSupplyLimitChanged;
	protected ref ScriptInvokerInt m_OnSupplyRequestExecutionPriorityChanged;
	protected ref ScriptInvokerFloat m_OnReservedSupplyAmountChanged;
	protected ref ScriptInvokerVoid m_SuppliesArrivalInvoker;

	[RplProp(onRplName: "OnHQSet")]
	protected bool m_bIsHQ;

	[RplProp(onRplName: "OnInitialized")]
	protected bool m_bInitialized;

	[RplProp()]
	protected int m_iSupplyRegenAmount;

	[RplProp(onRplName: "OnCapturingFactionChanged")]
	protected int m_iCapturingFaction = INVALID_FACTION_INDEX;

	[RplProp()]
	protected int m_iReconfiguredBy = INVALID_PLAYER_INDEX;

	[RplProp(onRplName: "OnRadioRangeChanged")]
	protected float m_fRadioRange;

	[RplProp(onRplName: "OnRespawnCooldownChanged")]
	protected WorldTimestamp m_fRespawnAvailableSince;

	[RplProp(onRplName: "OnAttackingFactionChanged")]
	protected int m_iAttackingFaction = -1;

	[RplProp()]
	protected float m_fSuppliesArrivalTime = float.MAX;

	[RplProp()]
	protected bool m_bBuiltByPlayers;

	[RplProp()]
	protected FactionKey m_sBuiltFaction;

	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnSuppliesArrivalInvoker()
	{
		if (!m_SuppliesArrivalInvoker)
			m_SuppliesArrivalInvoker = new ScriptInvokerVoid();

		return m_SuppliesArrivalInvoker;
	}

	//------------------------------------------------------------------------------------------------
	OnSupplyLimitChangedInvoker GetOnSupplyLimitChanged()
	{
		if (!m_OnSupplyLimitChanged)
			m_OnSupplyLimitChanged = new OnSupplyLimitChangedInvoker();

		return m_OnSupplyLimitChanged;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerInt GetOnSupplyRequestExecutionPriorityChanged()
	{
		if (!m_OnSupplyRequestExecutionPriorityChanged)
			m_OnSupplyRequestExecutionPriorityChanged = new ScriptInvokerInt();

		return m_OnSupplyRequestExecutionPriorityChanged;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerFloat GetOnReservedSupplyAmountChanged()
	{
		if (!m_OnReservedSupplyAmountChanged)
			m_OnReservedSupplyAmountChanged = new ScriptInvokerFloat();

		return m_OnReservedSupplyAmountChanged;
	}

	//------------------------------------------------------------------------------------------------
	IEntity GetBaseBuildingComposition()
	{
		return m_BaseBuildingComposition;
	}

	//------------------------------------------------------------------------------------------------
	void SetBaseBuildingComposition(IEntity building)
	{
		m_BaseBuildingComposition = building;
	}

	//------------------------------------------------------------------------------------------------
	bool IsResupplyTaskCreationEnabled()
	{
		return m_bIsResupplyTaskCreationEnabled;
	}

	//------------------------------------------------------------------------------------------------
	bool IsResupplyNeeded()
	{
		return GetScarcityLevel() > 0;
	}

	//------------------------------------------------------------------------------------------------
	float GetScarcityLevel()
	{
		SCR_ResourceConsumer resourceConsumer = GetResourceConsumer();
		if (!resourceConsumer)
			return 0;

		float aggregatedResources = resourceConsumer.GetAggregatedResourceValue();
		float supplyLimitValue = GetSuppliesMax() * m_fSupplyLimit;

		return supplyLimitValue - aggregatedResources;
	}

	//------------------------------------------------------------------------------------------------
	float GetSupplyLimit()
	{
		return m_fSupplyLimit;
	}

	//------------------------------------------------------------------------------------------------
	void SetSupplyLimit(float value)
	{
		if (m_fSupplyLimit == value)
			return;

		m_fSupplyLimit = value;
		Replication.BumpMe();

		if (m_OnSupplyLimitChanged)
			m_OnSupplyLimitChanged.Invoke(this, m_fSupplyLimit);
	}

	//------------------------------------------------------------------------------------------------
	float GetReservedSupplyAmount()
	{
		return m_fReservedSupplyAmount;
	}

	//------------------------------------------------------------------------------------------------
	void SetReservedSupplyAmount(float value)
	{
		if (m_fReservedSupplyAmount == value)
			return;

		m_fReservedSupplyAmount = value;
		Replication.BumpMe();

		if (m_OnReservedSupplyAmountChanged)
			m_OnReservedSupplyAmountChanged.Invoke(m_fReservedSupplyAmount);
	}

	//------------------------------------------------------------------------------------------------
	float GetSuppliesArrivalTime()
	{
		return m_fSuppliesArrivalTime;
	}

	//------------------------------------------------------------------------------------------------
	SCR_ESupplyRequestExecutionPriority GetSupplyRequestExecutionPriority()
	{
		return m_eSupplyRequestExecutionPriority;
	}

	//------------------------------------------------------------------------------------------------
	void SetSupplyRequestExecutionPriority(SCR_ESupplyRequestExecutionPriority priority)
	{
		int minPriority, maxPriority, newPriority;
		SCR_Enum.GetRange(SCR_ESupplyRequestExecutionPriority, minPriority, maxPriority);

		newPriority = Math.ClampInt(priority, minPriority, maxPriority);
		if (newPriority == m_eSupplyRequestExecutionPriority)
			return;

		m_eSupplyRequestExecutionPriority = newPriority;
		Replication.BumpMe();

		if (m_OnSupplyRequestExecutionPriorityChanged)
			m_OnSupplyRequestExecutionPriorityChanged.Invoke(m_eSupplyRequestExecutionPriority);
	}

	//------------------------------------------------------------------------------------------------
	SCR_ResourceConsumer GetResourceConsumer()
	{
		SCR_ResourceComponent resourceComponent = GetResourceComponent();
		if (!resourceComponent)
			return null;

		return resourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] faction
	//! \return Formatted base name with callsign in brackets
	string GetFormattedBaseNameWithCallsign(Faction faction)
	{
		if (GetType() == SCR_ECampaignBaseType.RELAY)
			return GetBaseNameUpperCase();

		if (!GetCallsignDisplayNameOnlyUC().IsEmpty())
			return string.Format("%1 (%2)", GetBaseNameUpperCase(), GetCallsignDisplayNameOnlyUC());

		// it looks like it has not set callsign data, maybe it is dedicated server
		SCR_Faction scrFaction = SCR_Faction.Cast(faction);
		if (!scrFaction)
			return GetBaseNameUpperCase();

		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		if (!campaign)
			return GetBaseNameUpperCase();

		if (m_iCallsign == INVALID_BASE_CALLSIGN)
			return GetBaseNameUpperCase();

		int callsignOffset = campaign.GetCallsignOffset();
		if (callsignOffset == INVALID_BASE_CALLSIGN)
			return GetBaseNameUpperCase();

		SCR_MilitaryBaseCallsign callsignInfo;

		// Use index offset for OPFOR so callsigns are not comparable (i.e. Alabama will not always mean Avrora etc.)
		if (scrFaction == campaign.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR))
			callsignInfo = scrFaction.GetBaseCallsignByIndex(m_iCallsign);
		else
			callsignInfo = scrFaction.GetBaseCallsignByIndex(m_iCallsign, callsignOffset);

		if (!callsignInfo)
			return GetBaseNameUpperCase();

		return string.Format("%1 (%2)", GetBaseNameUpperCase(), callsignInfo.GetCallsignUpperCase());
	}

	//------------------------------------------------------------------------------------------------
	bool IsValidTarget(notnull SCR_CampaignFaction faction)
	{
		if (GetFaction() == faction)
			return false;

		if (!IsHQRadioTrafficPossible(faction))
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	void SetBuiltByPlayers(bool builtByPlayers)
	{
		if (m_bBuiltByPlayers == builtByPlayers)
			return;

		m_bBuiltByPlayers = builtByPlayers;
		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	bool GetBuiltByPlayers()
	{
		return m_bBuiltByPlayers;
	}

    //------------------------------------------------------------------------------------------------
	void SetBuiltFaction(notnull Faction faction)
	{
		FactionKey factionKey = faction.GetFactionKey();
		if (m_sBuiltFaction == factionKey)
			return;

		m_sBuiltFaction = factionKey;
		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	FactionKey GetBuiltFaction()
	{
		return m_sBuiltFaction;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	bool IsControlPoint()
	{
		return m_bIsControlPoint;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	bool CanBeHQ()
	{
		return m_bCanBeHQ;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	bool DisableWhenUnusedAsHQ()
	{
		return m_bDisableWhenUnusedAsHQ;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	WorldTimestamp GetRespawnTimestamp()
	{
		return m_fRespawnAvailableSince;
	}

	//------------------------------------------------------------------------------------------------
	//!
	void Initialize()
	{
		if (m_bInitialized || IsProxy())
			return;

		SCR_CoverageRadioComponent radio = SCR_CoverageRadioComponent.Cast(GetOwner().FindComponent(SCR_CoverageRadioComponent));
		if (radio)
			radio.SetPower(true);

		m_bInitialized = true;
		Replication.BumpMe();
		OnInitialized();
	}

	//------------------------------------------------------------------------------------------------
	//!
	void Disable()
	{
		if (IsProxy())
			return;

		SCR_CoverageRadioComponent radio = SCR_CoverageRadioComponent.Cast(GetOwner().FindComponent(SCR_CoverageRadioComponent));
		if (radio)
			radio.SetPower(false);

		GetGame().GetCallqueue().Remove(SpawnBuilding);
		GetGame().GetCallqueue().Remove(SpawnStartingVehicles);
		GetGame().GetCallqueue().Remove(EvaluateDefenders);
		GetGame().GetCallqueue().Remove(SupplyIncomeTimer);
		GetGame().GetCallqueue().Remove(HandleSpawnPointFaction);
		DeleteStartingVehicles();

		m_FactionComponent.SetAffiliatedFaction(null);

		RplComponent.DeleteRplEntity(m_BaseBuildingComposition, false);

		m_bInitialized = false;
		Replication.BumpMe();
		OnDisabled();
	}

	//------------------------------------------------------------------------------------------------
	//!
	void OnInitialized()
	{
		if (!m_bInitialized)
			return;

		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		if (!campaign)
			return;

		// Register default base spawnpoint
		SCR_SpawnPoint spawnpoint;
		IEntity child = GetOwner().GetChildren();
		while (child)
		{
			spawnpoint = SCR_SpawnPoint.Cast(child);

			if (spawnpoint)
			{
				m_SpawnPoint = spawnpoint;

				SCR_CampaignBuildingProviderComponent buildingProvider = SCR_CampaignBuildingProviderComponent.Cast(GetOwner().FindComponent(SCR_CampaignBuildingProviderComponent));
				if (buildingProvider)
					m_SpawnPoint.SetSpawnPositionRange(buildingProvider.GetBuildingRadius());
				else
					m_SpawnPoint.SetSpawnPositionRange(m_iRadius);

				if (m_OnSpawnPointAssigned)
					m_OnSpawnPointAssigned.Invoke(spawnpoint);

				break;
			}

			child = child.GetSibling();
		}

#ifdef ENABLE_DIAG
		if (SCR_RespawnComponent.Diag_IsCLISpawnEnabled())
			HandleSpawnPointFaction();
#endif

		// Initialize registered services
		array<SCR_ServicePointComponent> services = {};
		GetServices(services);

		foreach (SCR_ServicePointComponent service : services)
		{
			OnServiceBuilt(service);
		}

		if (RplSession.Mode() != RplMode.Dedicated)
		{
			ConnectToCampaignBasesSystem();

			Faction playerFaction = SCR_FactionManager.SGetLocalPlayerFaction();
			if (!playerFaction)
				playerFaction = campaign.GetBaseManager().GetLocalPlayerFaction();

			if (playerFaction)
				OnLocalPlayerFactionAssigned(playerFaction);
			else
				campaign.GetOnFactionAssignedLocalPlayer().Insert(OnLocalPlayerFactionAssigned);
		}

		campaign.GetBaseManager().GetOnAllBasesInitialized().Insert(OnAllBasesInitialized);

		if (IsProxy())
		{
			campaign.GetBaseManager().AddActiveBase();
			return;
		}
		else
		{
			if (!m_bIsHQ && SCR_XPHandlerComponent.IsXpSystemEnabled())
			{
				GetGame().GetCallqueue().CallLater(EvaluateDefenders, DEFENDERS_CHECK_PERIOD + Math.RandomIntInclusive(0, DEFENDERS_CHECK_PERIOD * 0.1), true);
			}
		}

		SCR_CampaignSeizingComponent seizingComponent = SCR_CampaignSeizingComponent.Cast(GetOwner().FindComponent(SCR_CampaignSeizingComponent));
		if (seizingComponent)
		{
			seizingComponent.GetOnCaptureStart().Insert(OnCaptureStart);
			seizingComponent.GetOnCaptureInterrupt().Insert(EndCapture);
		}

		bool isSupportedBaseType = m_eType == SCR_ECampaignBaseType.BASE || m_eType == SCR_ECampaignBaseType.SOURCE_BASE;
		if (isSupportedBaseType && !campaign.IsTutorial())
		{
			// Supplies autoregen loop
			GetGame().GetCallqueue().CallLater(SupplyIncomeTimer, SCR_GameModeCampaign.UI_UPDATE_DELAY, true, false);

			// Refresh spawnpoint faction after loaded data has been applied
			GetGame().GetCallqueue().CallLater(HandleSpawnPointFaction, SCR_GameModeCampaign.BACKEND_DELAY);
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] settings
	void ApplyHeaderSettings(notnull SCR_CampaignCustomBase settings)
	{
		m_bCanBeHQ = settings.GetCanBeHQ();
		m_bDisableWhenUnusedAsHQ = settings.GetDisableWhenUnusedAsHQ();
		m_bIsControlPoint = settings.IsControlPoint();

		SCR_CoverageRadioComponent coverageComp = SCR_CoverageRadioComponent.Cast(GetOwner().FindComponent(SCR_CoverageRadioComponent));

		if (coverageComp)
			coverageComp.SetIsSource(m_bCanBeHQ);

		float radioRange = settings.GetRadioRange();

		if (radioRange < 0)
			return;

		BaseRadioComponent radio = BaseRadioComponent.Cast(GetOwner().FindComponent(BaseRadioComponent));

		if (!radio)
			return;

		BaseTransceiver transceiver = radio.GetTransceiver(0);

		if (!transceiver)
			return;

		m_fRadioRangeDefault = radioRange;
		transceiver.SetRange(radioRange);
		RecalculateRadioRange();
	}

	//------------------------------------------------------------------------------------------------
	override void OnServiceStateChanged(SCR_EServicePointStatus state, notnull SCR_ServicePointComponent serviceComponent)
	{
		switch (state)
		{
			case SCR_EServicePointStatus.UNDER_CONSTRUCTION:
			{
				return;
			}

			case SCR_EServicePointStatus.ONLINE:
			{
				OnServiceBuilt(serviceComponent);

				SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
				if (!campaign)
					return;

				SCR_CampaignMilitaryBaseManager baseManager = campaign.GetBaseManager();
				if (!baseManager)
					return;

				baseManager.OnServiceBuilt(state, serviceComponent);
				return;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] service
	void OnServiceBuilt(notnull SCR_ServicePointComponent service)
	{
		bool duringInit = (GetGame().GetWorld().GetWorldTime() <= 0);

		// Delayed call so the composition has time to properly load and register its entire hierarchy on clients as well
		GetGame().GetCallqueue().CallLater(OnServiceBuilt_AfterInit, SCR_GameModeCampaign.MEDIUM_DELAY, false, service);

		EEditableEntityLabel serviceType = service.GetLabel();
		SCR_ERadioMsg radio = SCR_ERadioMsg.NONE;
		SCR_CampaignFaction owner = GetCampaignFaction();

		switch (serviceType)
		{
			case EEditableEntityLabel.SERVICE_SUPPLY_STORAGE:
			{
				radio = SCR_ERadioMsg.BUILT_SUPPLY;
				break;
			}

			case EEditableEntityLabel.SERVICE_FUEL:
			{
				radio = SCR_ERadioMsg.BUILT_FUEL;
				break;
			}

			case EEditableEntityLabel.SERVICE_ARMORY:
			{
				radio = SCR_ERadioMsg.BUILT_ARMORY;
				break;
			}

			case EEditableEntityLabel.SERVICE_VEHICLE_DEPOT_LIGHT:
			{
				radio = SCR_ERadioMsg.BUILT_VEHICLES_LIGHT;
				break;
			}

			case EEditableEntityLabel.SERVICE_VEHICLE_DEPOT_HEAVY:
			{
				radio = SCR_ERadioMsg.BUILT_VEHICLES_HEAVY;
				break;
			}

			case EEditableEntityLabel.SERVICE_ANTENNA:
			{
				radio = SCR_ERadioMsg.BUILT_ANTENNA;

				if (!IsProxy())
					RecalculateRadioRange();

				break;
			}

			case EEditableEntityLabel.SERVICE_LIVING_AREA:
			{
				radio = SCR_ERadioMsg.BUILT_BARRACKS;
				break;
			}

			case EEditableEntityLabel.SERVICE_FIELD_HOSPITAL:
			{
				radio = SCR_ERadioMsg.BUILT_FIELD_HOSPITAL;
				break;
			}

			case EEditableEntityLabel.SERVICE_HELIPAD:
			{
				radio = SCR_ERadioMsg.BUILT_HELIPAD;
				break;
			}
		}

		if (IsProxy())
			return;

		GetGame().GetCallqueue().CallLater(SetProviderEntity, 1, false, service); // call later to give a time to a composition to properly register in replication.

		if (radio != SCR_ERadioMsg.NONE && owner && IsHQRadioTrafficPossible(GetCampaignFaction(), SCR_ERadioCoverageStatus.BOTH_WAYS) && !duringInit)
		{
			m_aServiceBuiltMsgQueue.Insert(radio);
			GetGame().GetCallqueue().Remove(SendHighestPriorityMessage);

			// Delayed call so we have all messages gathered
			GetGame().GetCallqueue().CallLater(SendHighestPriorityMessage, SCR_GameModeCampaign.UI_UPDATE_DELAY, false, owner);
		}

		RefreshSeizingTimers();
	}

	//------------------------------------------------------------------------------------------------
	protected void SupplyRequestExecutionPriorityReplicated()
	{
		if (m_OnSupplyRequestExecutionPriorityChanged)
			m_OnSupplyRequestExecutionPriorityChanged.Invoke(m_eSupplyRequestExecutionPriority);
	}

	//------------------------------------------------------------------------------------------------
	protected int GetSuppliesIncomeAmount()
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		if (!campaign)
			return 0;

		// For HQs and Supply Hubs
		if (m_bIsHQ || m_bIsSupplyHub)
		{
			// If base has custom value, use it, otherwise use gamemode's regular supplies income
			if (m_iRegularSuppliesIncomeBase > -1)
				return m_iRegularSuppliesIncomeBase;
			else
				return campaign.GetRegularSuppliesIncome();
		}

		// For Forward Operating Bases when supply regeneration is enabled
		if (campaign.GetSuppliesAutoRegenerationEnabled())
			return campaign.GetRegularSuppliesIncomeBase();

		// In case of supply regeneration is disabled, no income for FOBs
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	protected int GetSuppliesArrivalTimer()
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		if (!campaign)
			return int.MAX;

		// If Supply Regeneration is enabled, non-supply-hub bases use regular arrival interval
		if (!m_bIsSupplyHub && campaign.GetSuppliesAutoRegenerationEnabled())
			return campaign.GetSuppliesArrivalInterval();

		// Supply regeneration is disabled or base is a supply hub, use custom value if set or arrival interval for source bases
		if (m_iSuppliesArrivalInterval > -1)
			return m_iSuppliesArrivalInterval;
		else
			return campaign.GetSuppliesArrivalIntervalSource();
	}

	//------------------------------------------------------------------------------------------------
	protected void SetProviderEntity(notnull SCR_ServicePointComponent service)
	{
		SCR_CampaignBuildingCompositionComponent buildingComponent = SCR_CampaignBuildingCompositionComponent.Cast(SCR_EntityHelper.GetMainParent(service.GetOwner(), true).FindComponent(SCR_CampaignBuildingCompositionComponent));

		if (buildingComponent && !buildingComponent.GetProviderEntity())
		{
			array<SCR_CampaignBuildingProviderComponent> buildingProviders = {};
			GetBuildingProviders(buildingProviders);

			foreach (SCR_CampaignBuildingProviderComponent provider : buildingProviders)
			{
				if (provider && provider.IsMasterProvider())
				{
					buildingComponent.SetProviderEntityServer(provider.GetOwner());
					break;
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SendHighestPriorityMessage(notnull SCR_CampaignFaction faction)
	{
		if (m_aServiceBuiltMsgQueue.IsEmpty())
			return;

		SCR_ERadioMsg topPriorityEnum = m_aServiceBuiltMsgQueue[0];

		foreach (SCR_ERadioMsg msg : m_aServiceBuiltMsgQueue)
		{
			if (msg < topPriorityEnum)
				topPriorityEnum = msg;
		}

		m_aServiceBuiltMsgQueue.Clear();
		faction.SendHQMessage(topPriorityEnum, m_iCallsign);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] service
	void OnServiceRemoved(notnull SCR_MilitaryBaseLogicComponent service)
	{
		SCR_ServicePointComponent serviceCast = SCR_ServicePointComponent.Cast(service);

		if (!serviceCast || serviceCast.GetServiceState() != SCR_EServicePointStatus.ONLINE)
			return;

		if (IsProxy())
			return;

		if (serviceCast.GetType() == SCR_EServicePointType.RADIO_ANTENNA)
			RecalculateRadioRange();

		RefreshSeizingTimers();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] service
	void OnServiceBuilt_AfterInit(SCR_ServicePointComponent service)
	{
		// In case the composition has been deleted in the meantime
		if (!service)
			return;

		if (!IsProxy())
		{
			ResourceName delegatePrefab = service.GetDelegatePrefab();

			if (delegatePrefab.IsEmpty())
				return;

			Resource delegateResource = Resource.Load(delegatePrefab);

			if (!delegateResource || !delegateResource.IsValid())
				return;

			EntitySpawnParams params = EntitySpawnParams();
			params.TransformMode = ETransformMode.WORLD;
			service.GetOwner().GetTransform(params.Transform);
			IEntity delegateEntity = GetGame().SpawnEntityPrefab(delegateResource, null, params);

			if (!delegateEntity)
				return;

			SCR_ServicePointDelegateComponent delegateComponent = SCR_ServicePointDelegateComponent.Cast(delegateEntity.FindComponent(SCR_ServicePointDelegateComponent));

			if (!delegateComponent)
				return;

			service.SetDelegate(delegateComponent);
			delegateComponent.SetParentBaseId(Replication.FindId(this));
		}

		// Delayed call so clients know about the new delegate
		if (RplSession.Mode() != RplMode.Dedicated)
			GetGame().GetCallqueue().CallLater(m_MapDescriptor.HandleMapInfo, SCR_GameModeCampaign.DEFAULT_DELAY, false, null);
	}

	//------------------------------------------------------------------------------------------------
	//!
	void OnDisabled()
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		if (!campaign)
			return;

		if (m_SpawnPoint)
		{
			m_SpawnPoint.SetFactionKey(string.Empty);
			m_SpawnPoint = null;
		}

		if (m_MapDescriptor)
			m_MapDescriptor.Item().SetVisible(false);

		if (m_UIElement)
			m_UIElement.SetVisible(false);

		campaign.GetOnFactionAssignedLocalPlayer().Remove(OnLocalPlayerFactionAssigned);

		if (RplSession.Mode() != RplMode.Dedicated)
			DisconnectFromCampaignBasesSystem();

		campaign.GetBaseManager().GetOnAllBasesInitialized().Remove(OnAllBasesInitialized);

		SCR_CampaignSeizingComponent seizingComponent = SCR_CampaignSeizingComponent.Cast(GetOwner().FindComponent(SCR_CampaignSeizingComponent));
		if (seizingComponent)
		{
			seizingComponent.GetOnCaptureStart().Remove(OnCaptureStart);
			seizingComponent.GetOnCaptureInterrupt().Remove(EndCapture);
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	bool IsInitialized()
	{
		return m_bInitialized;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] isHQ
	void SetAsHQ(bool isHQ)
	{
		if (IsProxy())
			return;

		m_bIsHQ = isHQ;
		if (m_bIsHQ)
		{
			SCR_CampaignFaction faction = GetCampaignFaction();
			SCR_CampaignMilitaryBaseComponent previousHQ = faction.GetMainBase();
			SCR_CampaignMilitaryBaseComponent enemyHQ = SCR_CampaignFactionManager.Cast(GetGame().GetFactionManager()).GetEnemyFaction(faction).GetMainBase();
			if (previousHQ && previousHQ != this && previousHQ != enemyHQ)
			{
				if (previousHQ.GetDisableWhenUnusedAsHQ())
					previousHQ.Disable();
			}
		}

		Replication.BumpMe();
		OnHQSet();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnHQSet()
	{
		SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(GetFaction());
		if (faction && m_bIsHQ)
		{
			faction.SetMainBase(this);
			m_bWasHQSet = true;
		}

		if (IsProxy())
			return;

		HandleSpawnPointFaction();

		GetGame().GetCallqueue().Remove(SpawnStartingVehicles);

		if (m_bIsHQ && !SCR_PersistenceSystem.IsLoadInProgress())
		{
			SupplyIncomeTimer(true);
			GetGame().GetCallqueue().CallLater(SpawnStartingVehicles, 1500, false); // Delay so we don't spawn stuff during init
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	bool IsHQ()
	{
		return m_bIsHQ;
	}

	//------------------------------------------------------------------------------------------------
	//! \return If it cost supplies to spawn on this base
	bool CostSuppliesToSpawn()
	{
		return !m_bIsHQ;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	OnSpawnPointAssignedInvoker GetOnSpawnPointAssigned()
	{
		if (!m_OnSpawnPointAssigned)
			m_OnSpawnPointAssigned = new OnSpawnPointAssignedInvoker();

		return m_OnSpawnPointAssigned;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	static OnBaseStateChangedInvoker GetOnBaseUnderAttack()
	{
		if (!m_OnBaseUnderAttack)
			m_OnBaseUnderAttack = new OnBaseStateChangedInvoker();

		return m_OnBaseUnderAttack;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	static OnBaseStateChangedInvoker GetOnFactionChangedExtended()
	{
		if (!m_OnFactionChangedExtended)
			m_OnFactionChangedExtended = new OnBaseStateChangedInvoker();

		return m_OnFactionChangedExtended;
	}

	//------------------------------------------------------------------------------------------------
	static OnBaseAttackEndInvoker GetOnBaseAttackEnd()
	{
		if (!m_OnBaseAttackEnd)
			m_OnBaseAttackEnd = new OnBaseAttackEndInvoker();

		return m_OnBaseAttackEnd;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetRadioRange()
	{
		return m_fRadioRange;
	}

	//------------------------------------------------------------------------------------------------
	protected float GetRelayRadioRange(notnull BaseRadioComponent radio)
	{
		float range;
		RelayTransceiver transceiver;
		float thisRange;

		for (int i = 0, count = radio.TransceiversCount(); i < count; i++)
		{
			transceiver = RelayTransceiver.Cast(radio.GetTransceiver(i));

			if (!transceiver)
				continue;

			thisRange = transceiver.GetRange();

			if (thisRange <= range)
				continue;

			range = thisRange;
		}

		return range;
	}

	//------------------------------------------------------------------------------------------------
	bool CanReachByRadio(notnull IEntity entity)
	{
		SCR_CoverageRadioComponent radio = SCR_CoverageRadioComponent.Cast(entity.FindComponent(SCR_CoverageRadioComponent));

		if (!radio)
			return false;

		return m_RadioComponent.ConnectedRadiosContain(radio, true);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] faction
	//! \return
	SCR_ERadioCoverageStatus GetHQRadioCoverage(notnull SCR_CampaignFaction faction)
	{
		SCR_CampaignMilitaryBaseComponent hq = faction.GetMainBase();

		if (!hq)
			return SCR_ERadioCoverageStatus.NONE;

		SCR_CoverageRadioComponent radioHQ = SCR_CoverageRadioComponent.Cast(hq.GetOwner().FindComponent(SCR_CoverageRadioComponent));

		if (!radioHQ)
			return SCR_ERadioCoverageStatus.NONE;

		string encryption = radioHQ.GetEncryptionKey();

		SCR_CoverageRadioComponent radio = SCR_CoverageRadioComponent.Cast(GetOwner().FindComponent(SCR_CoverageRadioComponent));

		if (!radio)
			return SCR_ERadioCoverageStatus.NONE;

		return radio.GetCoverageByEncryption(encryption);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] faction
	//! \param[in] direction
	//! \return
	bool IsHQRadioTrafficPossible(notnull SCR_CampaignFaction faction, SCR_ERadioCoverageStatus direction = SCR_ERadioCoverageStatus.RECEIVE)
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();

		if (campaign && faction == campaign.GetFactionByEnum(SCR_ECampaignFaction.INDFOR))
			return true;

		SCR_ERadioCoverageStatus status = GetHQRadioCoverage(faction);

		switch (direction)
		{
			case SCR_ERadioCoverageStatus.RECEIVE:
			{
				return (status == SCR_ERadioCoverageStatus.RECEIVE || status == SCR_ERadioCoverageStatus.BOTH_WAYS);
			}

			case SCR_ERadioCoverageStatus.SEND:
			{
				return (status == SCR_ERadioCoverageStatus.SEND || status == SCR_ERadioCoverageStatus.BOTH_WAYS);
			}
		}

		return (status == direction);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnLocalPlayerFactionAssigned(Faction assignedFaction)
	{
		m_MapDescriptor.MapSetup(assignedFaction);
		m_MapDescriptor.HandleMapInfo(SCR_CampaignFaction.Cast(assignedFaction));
		HideMapLocationLabel();
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetBaseSpawnCostFactor()
	{
		if (GetServiceDelegateByType(SCR_EServicePointType.BARRACKS))
			return BARRACKS_REDUCED_DEPLOY_COST;

		return 1.0;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCaptureStart(SCR_Faction faction)
	{
		SCR_CampaignFaction factionC = SCR_CampaignFaction.Cast(faction);

		if (!factionC)
			return;

		BeginCapture(factionC);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] attackingFaction
	void NotifyAboutEnemyAttack(notnull Faction attackingFaction)
	{
		SCR_CampaignFaction campaignFaction = GetCampaignFaction();
		if (!campaignFaction)
			return;

		if (!IsHQRadioTrafficPossible(campaignFaction))
			return;

		ChimeraWorld world = GetOwner().GetWorld();

		if (!world)
			return;

		WorldTimestamp curTime = world.GetServerTimestamp();
		if (m_fLastEnemyContactTimestamp != 0)
		{
			if (m_fLastEnemyContactTimestamp.PlusSeconds(SCR_CampaignMilitaryBaseComponent.UNDER_ATTACK_WARNING_PERIOD).Greater(curTime))
				return;
		}

		campaignFaction.SendHQMessage(SCR_ERadioMsg.BASE_UNDER_ATTACK, GetCallsign());
		m_fLastEnemyContactTimestamp = curTime;

		if (GetCapturingFaction())
			return;

		FactionManager fManager = GetGame().GetFactionManager();

		if (!fManager)
			return;

		SetAttackingFaction(fManager.GetFactionIndex(attackingFaction));
	}

	//------------------------------------------------------------------------------------------------
	//! Capturing has been terminated
	void EndCapture()
	{
		if (IsProxy())
			return;

		if (!m_CapturingFaction)
			return;

		m_CapturingFaction = null;
		m_sCapturingFaction = FactionKey.Empty;

		m_iCapturingFaction = INVALID_FACTION_INDEX;
		Replication.BumpMe();
		OnCapturingFactionChanged();

		if (m_OnBaseAttackEnd)
			m_OnBaseAttackEnd.Invoke(this);
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetReconfiguredByID()
	{
		return m_iReconfiguredBy;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool GetDisableWhenUnusedAsHQ()
	{
		return m_bDisableWhenUnusedAsHQ;
	}

	//------------------------------------------------------------------------------------------------
	//! Capturing has begun
	bool BeginCapture(SCR_CampaignFaction faction, int playerID = INVALID_PLAYER_INDEX)
	{
		if (IsProxy() || !faction)
			return false;

		// The capturing faction already owns this base, return
		if (faction == GetFaction())
			return false;

		// Change the capturing faction
		m_CapturingFaction = faction;

		m_iCapturingFaction = SCR_CampaignFactionManager.Cast(GetGame().GetFactionManager()).GetFactionIndex(faction);
		m_sCapturingFaction = faction.GetFactionKey();

		m_iReconfiguredBy = playerID;
		Replication.BumpMe();
		OnCapturingFactionChanged();
		NotifyAboutEnemyAttack(faction);

		if (m_OnBaseUnderAttack)
			m_OnBaseUnderAttack.Invoke(this, GetFaction(true), faction);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Changes the faction which can spawn on spawn point groups owned by this base
	protected void HandleSpawnPointFaction()
	{
		if (!m_SpawnPoint)
			return;

		SCR_CampaignFaction owner = GetCampaignFaction();
		FactionKey currentKey = m_SpawnPoint.GetFactionKey();
		FactionKey ownerKey;
		FactionKey finalKey;

		if (owner)
		{
			ownerKey = owner.GetFactionKey();
			finalKey = ownerKey;

			SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();

			if (campaign && owner == campaign.GetFactionByEnum(SCR_ECampaignFaction.INDFOR) && !IsHQRadioTrafficPossible(campaign.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR)) && !IsHQRadioTrafficPossible(campaign.GetFactionByEnum(SCR_ECampaignFaction.OPFOR)))
				finalKey = FactionKey.Empty;
		}

		if (ownerKey == FactionKey.Empty)
		{
			if (currentKey != FactionKey.Empty)
				m_SpawnPoint.SetFactionKey(FactionKey.Empty);

			return;
		}

		if (owner && !m_bIsHQ && !IsHQRadioTrafficPossible(owner, SCR_ERadioCoverageStatus.BOTH_WAYS))
			finalKey = FactionKey.Empty;

		ChimeraWorld world = GetOwner().GetWorld();
		if (world.GetServerTimestamp().Less(m_fRespawnAvailableSince) && !m_bIsHQ)
			finalKey = FactionKey.Empty;

		if (finalKey == currentKey)
			return;

		m_SpawnPoint.SetFactionKey(finalKey);
	}
	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_SpawnPoint GetSpawnPoint()
	{
		return m_SpawnPoint;
	}

	//------------------------------------------------------------------------------------------------
	//! Event which is triggered when the capturing faction changes
	override void OnCapturingFactionChanged()
	{
		super.OnCapturingFactionChanged();

		m_CapturingFaction = SCR_CampaignFactionManager.Cast(GetGame().GetFactionManager()).GetCampaignFactionByIndex(m_iCapturingFaction);

		if (!IsProxy())
		{
			SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();

			if (campaign)
				campaign.GetBaseManager().EvaluateControlPoints();
		}

		if (RplSession.Mode() != RplMode.Dedicated)
		{
			if (m_CapturingFaction && GetFaction() == SCR_FactionManager.SGetLocalPlayerFaction())
				FlashBaseIcon(faction: m_CapturingFaction, infiniteTimer: true);
			else
				FlashBaseIcon(changeToDefault: true);
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	void FlashBaseIcon(float remainingTime = 0, Faction faction = null, bool changeToDefault = false, bool infiniteTimer = false)
	{
		GetGame().GetCallqueue().Remove(FlashBaseIcon);

		if (m_UIElement)
			m_UIElement.FlashBaseIcon(faction, changeToDefault);

		remainingTime -= SCR_CampaignFeedbackComponent.ICON_FLASH_PERIOD;

		if (infiniteTimer || remainingTime > 0 || !changeToDefault)
			GetGame().GetCallqueue().CallLater(FlashBaseIcon, SCR_CampaignFeedbackComponent.ICON_FLASH_PERIOD * 1000, false, remainingTime, faction, !changeToDefault, infiniteTimer);
	}

	//------------------------------------------------------------------------------------------------
	//!
	void OnHasSignalChanged()
	{
		if (!IsProxy())
		{
			HandleSpawnPointFaction();
			SupplyIncomeTimer(true);
		}

		if (m_MapDescriptor && RplSession.Mode() != RplMode.Dedicated)
			m_MapDescriptor.HandleMapInfo();

		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();

		if (campaign)
			campaign.GetBaseManager().GetOnSignalChanged().Invoke(this);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSuppliesChanged()
	{
		if (!IsProxy() && !m_bIsHQ)
			HandleSpawnPointFaction();

		if (RplSession.Mode() != RplMode.Dedicated && m_UIElement)
			m_UIElement.SetIconInfoText();
	}

	//------------------------------------------------------------------------------------------------
	//! Event which is triggered when the owning faction changes
	override protected void OnFactionChanged(FactionAffiliationComponent owner, Faction previousFaction, Faction faction)
	{
		super.OnFactionChanged(owner, previousFaction, faction);

		if (m_OnFactionChangedExtended)
			m_OnFactionChangedExtended.Invoke(this, previousFaction, faction);

		if (!GetGame().InPlayMode())
			return;

		SCR_CampaignFaction newCampaignFaction = SCR_CampaignFaction.Cast(faction);

		if (!newCampaignFaction)
			return;

		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();

		if (!campaign)
			return;

		float curTime = GetGame().GetWorld().GetWorldTime();

		if (!IsProxy())
		{
			bool wasCaptured = IsBeingCaptured();
			EndCapture();
			m_mDefendersData.Clear();
			m_iSupplyRegenAmount = 0;
			Replication.BumpMe();

			ChangeRadioSettings(newCampaignFaction);

			BaseGameMode gameMode = GetGame().GetGameMode();
			if (gameMode && wasCaptured && previousFaction && previousFaction != faction)
			{
				SCR_XPHandlerComponent xpHandlerComponent = SCR_XPHandlerComponent.Cast(gameMode.FindComponent(SCR_XPHandlerComponent));
				if (xpHandlerComponent)
					xpHandlerComponent.OnBaseSeized(this);
			}

			// Update signal coverage only if the base was seized during normal play, not at the start
			if (curTime > 10000)
			{
				campaign.GetBaseManager().RecalculateRadioCoverage(campaign.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR));
				campaign.GetBaseManager().RecalculateRadioCoverage(campaign.GetFactionByEnum(SCR_ECampaignFaction.OPFOR));
			}

			// Reset timer for reinforcements
			SupplyIncomeTimer(true);

			// CallLater for the starting bases of factions, where the faction is set before resources are properly initialized, resulting in incorrect Supply Regeneration calculation
			float currentSupplies = GetSupplies();
			float maxSupplies = GetSuppliesMax();
			GetGame().GetCallqueue().CallLater(CalculateSupplyRegenerationAmount, 1, false, currentSupplies, maxSupplies, newCampaignFaction);

			// Delay respawn possibility at newly-captured bases
			if (!m_bIsHQ && m_bInitialized && newCampaignFaction.IsPlayable() && campaign.GetBaseManager().IsBasesInitDone() && curTime > SCR_GameModeCampaign.BACKEND_DELAY)
			{
				ChimeraWorld world = GetOwner().GetWorld();
				m_fRespawnAvailableSince = world.GetServerTimestamp().PlusMilliseconds(m_fRespawnDelayAfterCaptureSeconds * 1000);
				OnRespawnCooldownChanged();
				Replication.BumpMe();
			}

			HandleSpawnPointFaction();
			SendHQMessageBaseCaptured();

			// If some Remnants live, send them to recapture
			if (newCampaignFaction.IsPlayable() && !m_bIsHQ)
			{
				foreach (SCR_AmbientPatrolSpawnPointComponent remnants : m_aRemnants)
				{
					AIGroup grp = remnants.GetSpawnedGroup();

					// Make sure groups which are not spawned at this point don't spawn in later
					if (grp && !remnants.IsGroupActive())
					{
						remnants.SetMembersAlive(0);
						RplComponent.DeleteRplEntity(grp, false);
						continue;
					}
					else if (!grp)
					{
						continue;
					}

					EntitySpawnParams params = EntitySpawnParams();
					params.TransformMode = ETransformMode.WORLD;
					params.Transform[3] = GetOwner().GetOrigin();
					m_SeekDestroyWP = SCR_TimedWaypoint.Cast(GetGame().SpawnEntityPrefabLocal(Resource.Load(SCR_GameModeCampaign.GetInstance().GetSeekDestroyWaypointPrefab()), null, params));
					m_SeekDestroyWP.SetHoldingTime(60);

					if (m_SeekDestroyWP)
						grp.AddWaypointAt(m_SeekDestroyWP, 0);
				}
			}

			// Do an auto save every time a base was captured
			if (!SCR_PersistenceSystem.IsLoadInProgress() && (GetGame().GetWorld().GetWorldTime() > campaign.BACKEND_DELAY))
				GetGame().GetSaveGameManager().RequestSavePoint(ESaveGameType.AUTO);
		}
		else
		{
			if (newCampaignFaction && m_bIsHQ && !m_bWasHQSet)
			{
				newCampaignFaction.SetMainBase(this);
				m_bWasHQSet = true;
			}
		}

		if (RplSession.Mode() != RplMode.Dedicated)
		{
			if (m_UIElement)
				m_UIElement.FlashBaseIcon(changeToDefault: true);

			if (m_MapDescriptor)
				m_MapDescriptor.HandleMapInfo();

			SCR_CampaignFaction playerFaction = SCR_CampaignFaction.Cast(SCR_FactionManager.SGetLocalPlayerFaction());
			IEntity player = SCR_PlayerController.GetLocalControlledEntity();

			// TODO: Move this to PlayRadioMsg so it is checked for player being inside radio range
			if (campaign)
			{
				if (player && playerFaction && playerFaction != GetCampaignFaction() && IsHQRadioTrafficPossible(playerFaction))
				{
					if (GetCampaignFaction())
						SCR_PopUpNotification.GetInstance().PopupMsg("#AR-Campaign_BaseSeized-UC", prio: SCR_ECampaignPopupPriority.BASE_LOST, param1: GetCampaignFaction().GetFactionNameUpperCase(), param2: GetBaseNameUpperCase());
					else
					{
						SCR_CampaignFaction factionR = campaign.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR);

						if (factionR)
							SCR_PopUpNotification.GetInstance().PopupMsg("#AR-Campaign_BaseSeized-UC", prio: SCR_ECampaignPopupPriority.BASE_LOST, param1: factionR.GetFactionNameUpperCase(), param2: GetBaseNameUpperCase());
					}

					/*if (playerFaction == m_OwningFactionPrevious)
						campaign.ShowHint(SCR_ECampaignHints.BASE_LOST); */
				}
			}
		}

		m_OwningFactionPrevious = newCampaignFaction;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] faction
	void OnSpawnPointFactionAssigned(FactionKey faction)
	{
		if (m_MapDescriptor && (RplSession.Mode() != RplMode.Dedicated))
			m_MapDescriptor.HandleMapInfo();
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] faction
	void ChangeRadioSettings(notnull SCR_Faction faction)
	{
		if (!m_RadioComponent || m_RadioComponent.TransceiversCount() == 0)
			return;

		BaseTransceiver tsv = m_RadioComponent.GetTransceiver(0);

		if (!tsv)
			return;

		m_RadioComponent.SetEncryptionKey(faction.GetFactionRadioEncryptionKey());

		int factionFrequency = faction.GetFactionRadioFrequency();

		// Setting frequency outside of limits causes a VME
		if (factionFrequency < tsv.GetMinFrequency() || factionFrequency > tsv.GetMaxFrequency())
			return;

		tsv.SetFrequency(factionFrequency);

		SCR_RadioCoverageSystem.UpdateAll();

		array<SCR_CoverageRadioComponent> radios = {};
		m_RadioComponent.GetRadiosInRange(radios);

		foreach (SCR_CoverageRadioComponent radio : radios)
		{
			if (!radio)
				continue;

			SCR_CampaignMilitaryBaseComponent base = SCR_CampaignMilitaryBaseComponent.Cast(radio.GetOwner().FindComponent(SCR_CampaignMilitaryBaseComponent));

			if (base)
				base.RefreshSeizingTimers();
		}
	}

	//------------------------------------------------------------------------------------------------
	void RefreshSeizingTimers()
	{
		foreach (SCR_MilitaryBaseLogicComponent logic : m_aSystems)
		{
			SCR_CampaignSeizingComponent seizingComponent = SCR_CampaignSeizingComponent.Cast(logic);

			if (!seizingComponent)
				continue;

			seizingComponent.RefreshSeizingTimer();
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] remnants
	void RegisterRemnants(notnull SCR_AmbientPatrolSpawnPointComponent remnants)
	{
		m_aRemnants.Insert(remnants);
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	ResourceName GetBuildingIconImageset()
	{
		SCR_CampaignMilitaryBaseComponentClass componentData = SCR_CampaignMilitaryBaseComponentClass.Cast(GetComponentData(GetOwner()));

		if (!componentData)
			return ResourceName.Empty;

		return componentData.GetBuildingIconImageset();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] grp
	void SetDefendersGroup(SCR_AIGroup grp)
	{
		// event is called for every spawned entity of the AI group. Prevent adding duplicates.
		if (m_aDefendersGroups.Contains(grp))
			return;

		m_aDefendersGroups.Insert(grp);
		grp.GetOnEmpty().Insert(RemoveGroup);
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	void RemoveGroup(SCR_AIGroup grp)
	{
		m_aDefendersGroups.RemoveItem(grp);
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool ContainsGroup(SCR_AIGroup grp)
	{
		return m_aDefendersGroups.Contains(grp);
	}

	//------------------------------------------------------------------------------------------------
	override void SetCallsign(notnull SCR_Faction faction)
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		if (!campaign)
			return;

		if (GetType() == SCR_ECampaignBaseType.RELAY) // Ignore relays as they only have dummy callsigns
			return;
		
		if (m_iCallsign == INVALID_BASE_CALLSIGN)
			return;

		int callsignOffset = campaign.GetCallsignOffset();
		campaign.GetOnCallsignOffsetChanged().Remove(OnCallsignAssigned);

		if (callsignOffset == INVALID_BASE_CALLSIGN)
		{
			campaign.GetOnCallsignOffsetChanged().Insert(OnCallsignAssigned);
			return;
		}

		// Use index offset for OPFOR so callsigns are not comparable (i.e. Alabama will not always mean Avrora etc.)
		SCR_MilitaryBaseCallsign callsignInfo;
		if (faction == campaign.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR))
			callsignInfo = faction.GetBaseCallsignByIndex(m_iCallsign);
		else
			callsignInfo = faction.GetBaseCallsignByIndex(m_iCallsign, callsignOffset);

		if (!callsignInfo)
			return;

		m_sCallsign = callsignInfo.GetCallsign();
		m_sCallsignNameOnly = callsignInfo.GetCallsignShort();
		m_sCallsignNameOnlyUC = callsignInfo.GetCallsignUpperCase();
		m_iCallsignSignal = callsignInfo.GetSignalIndex();
	}

	//------------------------------------------------------------------------------------------------
	override void SetCallsignIndexAutomatic(int index)
	{
		// Handled separately in SCR_CampaignMilitaryBaseManager.InitializeBases()
		return;
	}

	//------------------------------------------------------------------------------------------------
	//! Handles the timer for reinforcements arrival at this base
	//! \param reset Resets the current timer
	protected void SupplyIncomeTimer(bool reset = false)
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();

		if (!campaign)
			return;

		ChimeraWorld world = GetOwner().GetWorld();

		if (!world)
		{
			m_iSupplyRegenAmount = 0;
			Replication.BumpMe();
			return;
		}

		float curTime = world.GetWorldTime();

		if (reset)
		{
			SetSuppliesArrivalTime(curTime + (GetSuppliesArrivalTimer() * 1000));
			return;
		}

		SCR_CampaignFaction owner = GetCampaignFaction();

		// Add supplies only to captured bases in HQ radio range which are not under attack
		if (!owner || !m_SpawnPoint || (m_eType != SCR_ECampaignBaseType.SOURCE_BASE && !IsHQRadioTrafficPossible(owner, SCR_ERadioCoverageStatus.BOTH_WAYS)) || (m_CapturingFaction && m_CapturingFaction != owner) || world.GetServerTimestamp().Less(m_fRespawnAvailableSince))
		{
			m_iSupplyRegenAmount = 0;
			Replication.BumpMe();
			return;
		}

		if (curTime >= m_fSuppliesArrivalTime)
		{
			AddRegularSupplyPackage(owner);
			SetSuppliesArrivalTime(curTime + (GetSuppliesArrivalTimer() * 1000));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Reinforcements timer has finished, send in reinforcements
	protected void AddRegularSupplyPackage(notnull SCR_CampaignFaction faction)
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		float currentSupplies = GetSupplies();
		float maxSupplies = GetSuppliesMax();
		
        CalculateSupplyRegenerationAmount(currentSupplies, maxSupplies, faction);
        
        float spaceLimit;

		if (m_bIsHQ)
			spaceLimit = float.MAX;
		else
			spaceLimit = campaign.GetSuppliesReplenishThreshold() - currentSupplies;

		spaceLimit = Math.Min(spaceLimit, maxSupplies - currentSupplies);
		if (spaceLimit <= 0 || currentSupplies >= maxSupplies)
		{
			m_iSupplyRegenAmount = 0;
			Replication.BumpMe();
			return;
		}

        AddSupplies(Math.Min(m_iSupplyRegenAmount, spaceLimit));
	}

	//------------------------------------------------------------------------------------------------
	//! Calculates the supply regeneration amount based on input parameters. Calculated value saves to m_iSupplyRegenAmount
	//! \param[in] currentSupplies
	//! \param[in] maxSupplies
	//! \param[in] faction
	void CalculateSupplyRegenerationAmount(float currentSupplies, float maxSupplies, notnull SCR_CampaignFaction faction)
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();

		int quickReplenishThreshold = campaign.GetQuickSuppliesReplenishThreshold();
		float quickReplenishMultiplier = campaign.GetQuickSuppliesReplenishMultiplier();
		int income = GetSuppliesIncomeAmount();
		int toAdd;

		if (currentSupplies < quickReplenishThreshold)
		{
			int incomeHQQuick = income * quickReplenishMultiplier;
			incomeHQQuick = Math.Min(incomeHQQuick, quickReplenishThreshold - currentSupplies);
			toAdd = Math.Max(incomeHQQuick, income);
		}
		else
		{
			toAdd = income;
		}

		int extraBasesCount;

		if (!m_bIsHQ && campaign.GetSuppliesAutoRegenerationEnabled())
		{
			SCR_CampaignMilitaryBaseComponent base;
			array<SCR_CoverageRadioComponent> radios = {};
			m_RadioComponent.GetRadiosInRange(radios);

			foreach (SCR_CoverageRadioComponent radio : radios)
			{
				base = SCR_CampaignMilitaryBaseComponent.Cast(radio.GetOwner().FindComponent(SCR_CampaignMilitaryBaseComponent));

				if (!base)
					continue;

				if (base.GetType() != SCR_ECampaignBaseType.BASE)
					continue;

				if (base.GetCampaignFaction() != faction)
					continue;

				if (!base.IsHQRadioTrafficPossible(faction))
					continue;

				extraBasesCount++;

				if (extraBasesCount == BASES_IN_RANGE_RESUPPLY_THRESHOLD)
					break;
			}
		}

		toAdd += (SCR_GameModeCampaign.GetInstance().GetRegularSuppliesIncomeExtra() * extraBasesCount);

		m_iSupplyRegenAmount = toAdd;
		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_CampaignMilitaryBaseMapDescriptorComponent GetMapDescriptor()
	{
		return m_MapDescriptor;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_CampaignMapUIBase GetMapUI()
	{
		return m_UIElement;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] entity
	//! \return
	bool GetIsEntityInMyRange(notnull IEntity entity)
	{
		return vector.Distance(entity.GetOrigin(), GetOwner().GetOrigin()) <= GetRadioRange();
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetSupplies()
	{
		SCR_ResourceComponent resourceComponent = GetResourceComponent();

		if (!resourceComponent)
			return 0.0;

		SCR_ResourceConsumer resourceConsumer = resourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);

		if (!resourceConsumer)
			return 0.0;

		if (!IsProxy())
			GetGame().GetResourceGrid().UpdateInteractor(resourceConsumer);

		return resourceConsumer.GetAggregatedResourceValue();
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetSuppliesMax()
	{
		SCR_ResourceComponent resourceComponent = GetResourceComponent();

		if (!resourceComponent)
			return 0.0;

		SCR_ResourceConsumer resourceConsumer = resourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);

		if (!resourceConsumer)
			return 0.0;

		if (!IsProxy())
			GetGame().GetResourceGrid().UpdateInteractor(resourceConsumer);

		return resourceConsumer.GetAggregatedMaxResourceValue();
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetSuppliesIncome()
	{
		return m_iSupplyRegenAmount;
	}

	//------------------------------------------------------------------------------------------------
	override void RegisterLogicComponent(notnull SCR_MilitaryBaseLogicComponent component)
	{
		super.RegisterLogicComponent(component);

		SCR_FlagComponent flag = SCR_FlagComponent.Cast(component);

		// This is indeed a repeated call from super
		// Sandbox mil bases spawned via HQ tents are interfering with shown flag as they have different faction
		// It's a temporary fix until we switch to full free building and bases on top of each other will no longer be a thing
		if (flag && GetFaction())
			GetGame().GetCallqueue().CallLater(UpdateFlags, 1000, false);
	}

	//------------------------------------------------------------------------------------------------
	//!
	void HideMapLocationLabel()
	{
		if (m_sMapLocationName.IsEmpty() || !GetGame().GetWorld())
			return;

		GenericEntity ent = GenericEntity.Cast(GetGame().GetWorld().FindEntityByName(m_sMapLocationName));

		if (!ent)
			return;

		MapDescriptorComponent comp = MapDescriptorComponent.Cast(ent.FindComponent(MapDescriptorComponent));

		if (!comp)
			return;

		MapItem item = comp.Item();

		if (!item)
			return;

		item.SetVisible(false);
	}

	//------------------------------------------------------------------------------------------------
	//! return ResourceComponent of base
	SCR_ResourceComponent GetResourceComponent()
	{
		return SCR_ResourceComponent.FindResourceComponent(GetOwner());
	}

	//------------------------------------------------------------------------------------------------
	//! Add delivered supplies
	//! \param[in] suppliesCount
	//! \param[in] replicate
	void AddSupplies(int suppliesCount, bool replicate = true)
	{
		if (suppliesCount == 0)
			return;

		SCR_ResourceComponent resourceComponent = GetResourceComponent();

		if (!resourceComponent)
			return;

		if (suppliesCount > 0)
		{
			SCR_ResourceGenerator resourceGenerator = resourceComponent.GetGenerator(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);

			if (!resourceGenerator)
				return;

			resourceGenerator.RequestGeneration(suppliesCount);
		}
		else
		{
			SCR_ResourceConsumer resourceConsumer = resourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);

			if (!resourceConsumer)
				return;

			resourceConsumer.RequestConsumtion(-suppliesCount);
		}

		HandleSpawnPointFaction();

		if (m_SuppliesArrivalInvoker)
			m_SuppliesArrivalInvoker.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] suppliesCount
	void SetSupplies(float suppliesCount)
	{
		SCR_ResourceComponent resourceComponent = GetResourceComponent();

		if (!resourceComponent)
			return;

		SCR_ResourceConsumer resourceConsumer = resourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);

		if (!resourceConsumer)
			return;

		if (!IsProxy())
			GetGame().GetResourceGrid().UpdateInteractor(resourceConsumer);

		AddSupplies(suppliesCount - resourceConsumer.GetAggregatedResourceValue());
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] suppliesCount
	void SetInitialSupplies(float suppliesCount)
	{
		SCR_ResourceComponent resourceComponent = GetResourceComponent();

		if (!resourceComponent)
			return;

		SCR_ResourceConsumer resourceConsumer = resourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);

		if (!resourceConsumer)
			return;

		if (!IsProxy())
			GetGame().GetResourceGrid().UpdateInteractor(resourceConsumer);

		// Set starting supplies only if no bigger supply caches are already in the vicinity
		if (suppliesCount > resourceConsumer.GetAggregatedResourceValue())
			SetSupplies(suppliesCount);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] suppliesCount
	void SetStartingSupplies(float suppliesCount)
	{
		m_fStartingSupplies = suppliesCount;
	}

	//------------------------------------------------------------------------------------------------
	//! Change reinforcements timer
	//! \param time This number is added to the timer
	void AlterSupplyIncomeTimer(float time)
	{
		SetSuppliesArrivalTime(m_fSuppliesArrivalTime + time);
	}

	//------------------------------------------------------------------------------------------------
	void SetSuppliesArrivalTime(float time)
	{
		if (time == m_fSuppliesArrivalTime)
			return;

		m_fSuppliesArrivalTime = time;
		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] faction
	void SetFaction(SCR_CampaignFaction faction)
	{
		if (IsProxy())
			return;

		if (!m_FactionComponent)
			return;

		if(m_FactionComponent.GetAffiliatedFaction() == faction)
			return;
		
		if(!faction || m_FactionComponent.GetAffiliatedFactionKey() != faction.GetFactionKey())
			m_FactionComponent.SetAffiliatedFaction(faction);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns the owning faction
	SCR_CampaignFaction GetCampaignFaction()
	{
		return SCR_CampaignFaction.Cast(GetFaction());
	}

	//------------------------------------------------------------------------------------------------
	//! Returns type of this base
	SCR_ECampaignBaseType GetType()
	{
		return m_eType;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns the name of this base
	string GetBaseName()
	{
		return m_sBaseName;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns the upper-case name of this base
	string GetBaseNameUpperCase()
	{
		return m_sBaseNameUpper;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns whether this base is being captured
	bool IsBeingCaptured()
	{
		// The capturing faction exists, return true
		if (m_CapturingFaction)
			return true;

		// No faction is capturing this base, return false
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] enemyFaction
	void SetAttackingFaction(int enemyFaction)
	{
		m_iAttackingFaction = enemyFaction;

		if (m_iAttackingFaction < 0)
			return;

		Replication.BumpMe();
		OnAttackingFactionChanged();
	}

	//------------------------------------------------------------------------------------------------
	//!
	void OnAttackingFactionChanged()
	{
		Faction enemyFaction = GetGame().GetFactionManager().GetFactionByIndex(m_iAttackingFaction);
		Faction playerFaction = SCR_FactionManager.SGetLocalPlayerFaction();

		if (!IsProxy())
			GetGame().GetCallqueue().CallLater(SetAttackingFaction, 1000, false, -1);
		else
			m_iAttackingFaction = -1;

		if (RplSession.Mode() != RplMode.Dedicated && enemyFaction != playerFaction && GetFaction() == playerFaction)
			FlashBaseIcon(SCR_CampaignFeedbackComponent.ICON_FLASH_DURATION, enemyFaction);
	}

	//------------------------------------------------------------------------------------------------
	//!
	void OnRespawnCooldownChanged()
	{
		ChimeraWorld world = GetOwner().GetWorld();
		WorldTimestamp curTime = world.GetServerTimestamp();

		// Make sure the spawnpoint becomes available after timer runs out
		if (!IsProxy())
			GetGame().GetCallqueue().CallLater(HandleSpawnPointFaction, m_fRespawnAvailableSince.DiffMilliseconds(curTime) + SCR_GameModeCampaign.MEDIUM_DELAY);

		// Handle respawn cooldown UI
		if (RplSession.Mode() != RplMode.Dedicated)
			UpdateRespawnCooldown();
	}

	//------------------------------------------------------------------------------------------------
	//!
	void UpdateRespawnCooldown()
	{
		ChimeraWorld world = GetOwner().GetWorld();
		WorldTimestamp curTime = world.GetServerTimestamp();

		if (m_UIElement && SCR_FactionManager.SGetLocalPlayerFaction() == GetFaction())
			m_UIElement.SetIconInfoText();

		GetGame().GetCallqueue().Remove(UpdateRespawnCooldown);

		if (curTime.Less(m_fRespawnAvailableSince))
			GetGame().GetCallqueue().CallLater(UpdateRespawnCooldown, SCR_GameModeCampaign.UI_UPDATE_DELAY);
	}

	//------------------------------------------------------------------------------------------------
	void OnAllBasesInitialized()
	{
		if (IsProxy())
		{
			// Delayed call so all radios are powered on for clients
			GetGame().GetCallqueue().CallLater(SCR_RadioCoverageSystem.UpdateAll, SCR_GameModeCampaign.DEFAULT_DELAY, false, false);
			return;
		}

		// Spawn building only if not already set (from e.g. savegame)
		if (m_BaseBuildingComposition)
			return;

		ResourceName buildingPrefab;
		EEditableEntityLabel label;

		switch (GetType())
		{
			case SCR_ECampaignBaseType.BASE:
				label = EEditableEntityLabel.SERVICE_HQ;
				break;
			case SCR_ECampaignBaseType.SOURCE_BASE:
				label = EEditableEntityLabel.SERVICE_SOURCE_BASE;
				break;
			default:
				return;
		}

		buildingPrefab = GetCampaignFaction().GetBuildingPrefab(label);
		if (!buildingPrefab)
		{
			SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
			
			if (Math.RandomFloat01() >= 0.5)
				buildingPrefab = campaign.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR).GetBuildingPrefab(label);
			else
				buildingPrefab = campaign.GetFactionByEnum(SCR_ECampaignFaction.OPFOR).GetBuildingPrefab(label);
		}

		if (buildingPrefab)
		{
			// Delay so we don't spawn stuff during init
			GetGame().GetCallqueue().CallLater(	SpawnBuilding, 1000, false, buildingPrefab, GetOwner().GetOrigin(), GetOwner().GetYawPitchRoll(), true);
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	void RecalculateRadioRange()
	{
		float range = m_fRadioRangeDefault;
		float thisRange;
		array<SCR_ServicePointComponent> antennas = {};
		GetServicesByType(antennas, SCR_EServicePointType.RADIO_ANTENNA);
		BaseRadioComponent radio;

		// Find antenna services, read max radio range from the radio component on their owners
		foreach (SCR_ServicePointComponent service : antennas)
		{
			SCR_AntennaServicePointComponent antenna = SCR_AntennaServicePointComponent.Cast(service);
			radio = BaseRadioComponent.Cast(antenna.GetOwner().FindComponent(BaseRadioComponent));

			if (!radio)
				continue;

			// Turn off the radio so we don't hit performance too much with every antenna built
			if (radio.IsPowered())
				radio.SetPower(false);

			thisRange = GetRelayRadioRange(radio);

			if (thisRange > range)
				range = thisRange;
		}

		if (m_fRadioRange == range)
			return;

		// Instead of relying on antenna radio which has been turned off, apply the antenna's signal range to the radio component on the base itself
		if (m_RadioComponent)
		{
			RelayTransceiver transceiver;

			for (int i = 0, count = m_RadioComponent.TransceiversCount(); i < count; i++)
			{
				transceiver = RelayTransceiver.Cast(m_RadioComponent.GetTransceiver(i));

				if (!transceiver)
					continue;

				transceiver.SetRange(range);
			}
		}

		m_fRadioRange = range;
		Replication.BumpMe();
		OnRadioRangeChanged();
	}

	//------------------------------------------------------------------------------------------------
	//!
	void OnRadioRangeChanged()
	{
		// Delayed call so transceiver ranges on radios are properly updated for clients as well
		GetGame().GetCallqueue().CallLater(SCR_RadioCoverageSystem.UpdateAll, SCR_GameModeCampaign.DEFAULT_DELAY, false, false);

		if (IsProxy())
			return;

		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();

		if (!campaign)
			return;

		SCR_CampaignMilitaryBaseManager bManager = campaign.GetBaseManager();

		if (!bManager)
			return;

		if (GetGame().GetWorld().GetWorldTime() > campaign.BACKEND_DELAY)
		{
			// Process recalculation immediately unless we're still within save loading period
			bManager.RecalculateRadioCoverage(GetCampaignFaction());
		}
		else
		{
			// Otherwise process for both factions only once so we're not doing it for each antenna loaded
			GetGame().GetCallqueue().Remove(bManager.RecalculateRadioCoverage);
			GetGame().GetCallqueue().CallLater(bManager.RecalculateRadioCoverage, campaign.DEFAULT_DELAY, false, campaign.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR));
			GetGame().GetCallqueue().CallLater(bManager.RecalculateRadioCoverage, campaign.DEFAULT_DELAY, false, campaign.GetFactionByEnum(SCR_ECampaignFaction.OPFOR));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Called from SCR_MapCampaignUI when base UI elements are initialized
	void SetBaseUI(SCR_CampaignMapUIBase base)
	{
		m_UIElement = base;
	}

	//------------------------------------------------------------------------------------------------
	//!
	void SendHQMessageBaseCaptured()
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		if (!campaign)
			return;

		SCR_CampaignFaction newOwner = GetCampaignFaction();
		if (!newOwner)
			return;

		int newOwnerPoints = newOwner.GetControlPointsHeld();

		if (newOwnerPoints == campaign.GetControlPointTreshold() && IsControlPoint())
		{
			newOwner.SendHQMessage(SCR_ERadioMsg.WINNING);

			SCR_CampaignFaction losingFaction;
			FactionManager factionManager = GetGame().GetFactionManager();
			if (!factionManager)
				return;

			if (newOwner.GetFactionKey() == campaign.GetFactionKeyByEnum(SCR_ECampaignFaction.BLUFOR))
				losingFaction = SCR_CampaignFaction.Cast(factionManager.GetFactionByKey(campaign.GetFactionKeyByEnum(SCR_ECampaignFaction.OPFOR)));
			else
				losingFaction = SCR_CampaignFaction.Cast(factionManager.GetFactionByKey(campaign.GetFactionKeyByEnum(SCR_ECampaignFaction.BLUFOR)));

			if (losingFaction)
				losingFaction.SendHQMessage(SCR_ERadioMsg.LOSING);
		}
		else
		{
			if (m_iCallsign == INVALID_BASE_CALLSIGN)
				return;

			if (IsHQ())
				newOwner.SendHQMessage(SCR_ERadioMsg.SEIZED_MAIN, m_iCallsign);
			else if (IsControlPoint())
				newOwner.SendHQMessage(SCR_ERadioMsg.SEIZED_MAJOR, m_iCallsign);
			else if (GetType() == SCR_ECampaignBaseType.BASE)
				newOwner.SendHQMessage(SCR_ERadioMsg.SEIZED_SMALL, m_iCallsign);

			if (m_OwningFactionPrevious)
			{
				if (GetType() == SCR_ECampaignBaseType.RELAY)
					m_OwningFactionPrevious.SendHQMessage(SCR_ERadioMsg.RELAY_LOST, m_iCallsign);
				else
					m_OwningFactionPrevious.SendHQMessage(SCR_ERadioMsg.BASE_LOST, m_iCallsign);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SpawnStartingVehicles()
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();

		if (!campaign)
			return;

		if (campaign.IsTutorial())
			return;

		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		vector pos, oldPos;
		IEntity vehicleEntity;
		Physics physicsComponent;

		array<ResourceName> prefabNames = {};
		GetCampaignFaction().GetStartingVehiclePrefabs(prefabNames);

		foreach (ResourceName prefabName : prefabNames)
		{
			oldPos = pos;
			SCR_WorldTools.FindEmptyTerrainPosition(pos, GetOwner().GetOrigin(), HQ_VEHICLE_SPAWN_RADIUS, HQ_VEHICLE_QUERY_SPACE);

			//Should found empty terrain position be too close to last one, do another search with previous position as areaCenter.
			if (vector.DistanceSqXZ(pos, oldPos) < 1)
				SCR_WorldTools.FindEmptyTerrainPosition(pos, oldPos, HQ_VEHICLE_SPAWN_RADIUS, HQ_VEHICLE_QUERY_SPACE);

			params.Transform[3] = pos;

			vehicleEntity = GetGame().SpawnEntityPrefabEx(prefabName, false, params: params);
			if (vehicleEntity)
			{
				physicsComponent = vehicleEntity.GetPhysics();
				if (physicsComponent)
					physicsComponent.SetVelocity("0 -0.1 0"); // Make vehicle copy the terrain properly

				m_aStartingVehicles.Insert(vehicleEntity);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void DeleteStartingVehicles()
	{
		foreach (IEntity veh : m_aStartingVehicles)
		{
			if (veh)
				RplComponent.DeleteRplEntity(veh, false);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void RemoveFortifications()
	{
		IEntity child = GetOwner().GetChildren();
		IEntity prevChild;

		while (child)
		{
			if (child.FindComponent(SCR_CampaignBuildingCompositionComponent))
				prevChild = child;

			child = child.GetSibling();

			if (prevChild)
				SCR_EntityHelper.DeleteEntityAndChildren(prevChild);
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] prefab
	//! \param[in] position
	//! \param[in] rotation
	//! \param[in] isMainTent
	void SpawnBuilding(ResourceName prefab, vector position, vector rotation, bool isMainTent = false)
	{
		if (prefab.IsEmpty())
			return;

		if (position == vector.Zero)
			return;

		EntitySpawnParams params = EntitySpawnParams();
		GetOwner().GetWorldTransform(params.Transform);
		params.TransformMode = ETransformMode.WORLD;
		Math3D.AnglesToMatrix(rotation, params.Transform);
		params.Transform[3] = position;

		IEntity composition = GetGame().SpawnEntityPrefab(Resource.Load(prefab), null, params);
		if (!composition)
			return;

		m_BaseBuildingComposition = composition;

		if (isMainTent)
		{
			// Delayed call so the supplies system has time to process all the containers first
			if (m_fStartingSupplies >= 0)
				GetGame().GetCallqueue().CallLater(SetInitialSupplies, SCR_GameModeCampaign.MEDIUM_DELAY, false, m_fStartingSupplies);
		}

		SCR_AIWorld aiWorld = SCR_AIWorld.Cast(GetGame().GetAIWorld());

		if (aiWorld)
			aiWorld.RequestNavmeshRebuildEntity(composition);

		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(composition.FindComponent(SCR_EditableEntityComponent));
		vector transform[4];

		if (!editableEntity)
		{
			GetOwner().GetTransform(transform);
			SCR_TerrainHelper.SnapToTerrain(transform, composition.GetWorld());
			composition.SetTransform(transform);
			return;
		}

		editableEntity.GetTransform(transform);

		if (!SCR_TerrainHelper.SnapToTerrain(transform, composition.GetWorld()))
			return;

		editableEntity.SetTransformWithChildren(transform);
	}

	//------------------------------------------------------------------------------------------------
	//! Periodically add XP to players defending their base
	void EvaluateDefenders()
	{
		SCR_CampaignFaction baseFaction = GetCampaignFaction();

		if (!baseFaction.IsPlayable())
			return;

		SCR_CampaignFactionManager factionManager = SCR_CampaignFactionManager.Cast(GetGame().GetFactionManager());

		if (!factionManager)
			return;

		if (GetHQRadioCoverage(factionManager.GetEnemyFaction(baseFaction)) == SCR_ERadioCoverageStatus.NONE)
			return;

		PlayerManager playerManager = GetGame().GetPlayerManager();
		array<int> playerIds = {};
		array<int> playerIdsPresent = {};
		playerManager.GetPlayers(playerIds);
		int radiusSq;
		vector basePos = GetOwner().GetOrigin();
		SCR_XPHandlerComponent compXP = SCR_XPHandlerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_XPHandlerComponent));
		bool enemiesPresent;

		if (m_eType == SCR_ECampaignBaseType.BASE)
			radiusSq = m_iRadius * m_iRadius;
		else
			radiusSq = RELAY_BASE_RADIUS * RELAY_BASE_RADIUS;

		foreach (int playerId : playerIds)
		{
			SCR_ChimeraCharacter player = SCR_ChimeraCharacter.Cast(playerManager.GetPlayerControlledEntity(playerId));

			if (!player)
				continue;

			CharacterControllerComponent charController = player.GetCharacterController();

			if (charController.IsDead())
				continue;

			if (vector.DistanceSqXZ(player.GetOrigin(), basePos) > radiusSq)
				continue;

			if (player.GetFaction() == baseFaction)
				playerIdsPresent.Insert(playerId);
			else
				enemiesPresent = true;
		}

		ChimeraWorld world = GetOwner().GetWorld();
		WorldTimestamp curTime = world.GetServerTimestamp();
		foreach (int playerId : playerIdsPresent)
		{
			WorldTimestamp startedDefendingAt = m_mDefendersData.Get(playerId);
			if (startedDefendingAt == 0)
			{
				m_mDefendersData.Set(playerId, curTime)
			}
			else if (curTime.DiffMilliseconds(startedDefendingAt) >= DEFENDERS_REWARD_PERIOD)
			{
				m_mDefendersData.Set(playerId, curTime);

				if (enemiesPresent)
					compXP.AwardXP(playerManager.GetPlayerController(playerId), SCR_EXPRewards.BASE_DEFENDED, DEFENDERS_REWARD_MULTIPLIER);
				else
					compXP.AwardXP(playerManager.GetPlayerController(playerId), SCR_EXPRewards.BASE_DEFENDED);
			}
		}

		// Clean up non-present players from the list
		for (int i = m_mDefendersData.Count() - 1; i >= 0; i--)
		{
			int playerId = m_mDefendersData.GetKey(i);

			if (playerIdsPresent.Contains(playerId))
				continue;

			m_mDefendersData.Remove(playerId);
		}
	}

	protected void ConnectToCampaignBasesSystem()
	{
		World world = GetOwner().GetWorld();
		CampaignBasesSystem updateSystem = CampaignBasesSystem.Cast(world.FindSystem(CampaignBasesSystem));
		if (!updateSystem)
			return;

		updateSystem.Register(this);
	}

	protected void DisconnectFromCampaignBasesSystem()
	{
		World world = GetOwner().GetWorld();
		CampaignBasesSystem updateSystem = CampaignBasesSystem.Cast(world.FindSystem(CampaignBasesSystem));
		if (!updateSystem)
			return;

		updateSystem.Unregister(this);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] timeSlice
	void Update(float timeSlice)
	{
		m_fTimer += timeSlice;

		// Periodic calls, add random delay to avoid spikes
		if (m_fTimer < m_fNextFrameCheck)
			return;

		m_fTimer = 0;
		m_fNextFrameCheck = TICK_TIME + (Math.RandomFloat01() * TICK_TIME * 0.5);
		bool playerPresentPreviously = m_bLocalPlayerPresent;
		m_bLocalPlayerPresent = GetIsLocalPlayerPresent();

		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();

		if (campaign && m_bLocalPlayerPresent != playerPresentPreviously)
			campaign.GetBaseManager().OnLocalPlayerPresenceChanged(this, m_bLocalPlayerPresent);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		if (!IsProxy())
		{
			SCR_ResourceComponent resourceComponent = GetResourceComponent();

			if (resourceComponent)
			{
				SCR_ResourceConsumer consumerComponent = resourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);

				if (consumerComponent)
					consumerComponent.GetOnResourcesChanged().Insert(HandleSpawnPointFaction);
			}
		}

		if (m_RadioComponent)
			m_RadioComponent.GetOnCoverageChanged().Insert(OnHasSignalChanged);

		if (GetGame().GetWorld().GetWorldTime() >= SCR_GameModeCampaign.BACKEND_DELAY)
		{
			SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
			SCR_CampaignMilitaryBaseManager manager = campaign.GetBaseManager();

			if (manager)
				GetGame().GetCallqueue().CallLater(manager.OnBaseInitialized, 0, false, this);
		}

		if (RplSession.Mode() == RplMode.Dedicated)
			return;

		m_MapDescriptor = SCR_CampaignMilitaryBaseMapDescriptorComponent.Cast(GetOwner().FindComponent(SCR_CampaignMilitaryBaseMapDescriptorComponent));
		if (!m_MapDescriptor)
			return;

		m_MapDescriptor.SetParentBase(this);
		m_MapDescriptor.Item().SetVisible(true);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		m_RadioComponent = SCR_CoverageRadioComponent.Cast(GetOwner().FindComponent(SCR_CoverageRadioComponent));

		if (m_fRadioRange == 0 && m_RadioComponent)
		{
			m_fRadioRange = GetRelayRadioRange(m_RadioComponent);
			m_fRadioRangeDefault = m_fRadioRange;
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		DisconnectFromCampaignBasesSystem();

		SCR_CampaignSeizingComponent seizingComponent = SCR_CampaignSeizingComponent.Cast(GetOwner().FindComponent(SCR_CampaignSeizingComponent));
		if (seizingComponent)
		{
			seizingComponent.GetOnCaptureStart().Remove(OnCaptureStart);
			seizingComponent.GetOnCaptureInterrupt().Remove(EndCapture);
		}

		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		if (campaign)
		{
			campaign.GetOnFactionAssignedLocalPlayer().Remove(OnLocalPlayerFactionAssigned);
			campaign.GetBaseManager().GetOnAllBasesInitialized().Remove(OnAllBasesInitialized);
			campaign.GetOnCallsignOffsetChanged().Remove(OnCallsignAssigned);

			SCR_CampaignMilitaryBaseManager manager = campaign.GetBaseManager();
			if (manager)
				GetGame().GetCallqueue().CallLater(manager.UpdateBases, 0, false, false);
		}

		SCR_ResourceComponent resourceComponent = GetResourceComponent();
		if (resourceComponent)
		{
			SCR_ResourceConsumer consumerComponent = resourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);
			if (consumerComponent)
			{
				consumerComponent.GetOnResourcesChanged().Remove(HandleSpawnPointFaction);
			}
		}

		super.OnDelete(owner);
	}

	//------------------------------------------------------------------------------------------------
	//! Checks if this base is eligible for a Seize task.
	//! \return True if valid, false otherwise.
	bool IsValidForSeizeTask()
	{
		if (IsProxy())
			return false;

		if (m_iCallsign == INVALID_BASE_CALLSIGN)
			return false;

		if (!m_bInitialized)
			return false;

		if (m_bIsHQ || (m_bCanBeHQ && m_bDisableWhenUnusedAsHQ))
			return false;

		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();

		if (!campaign || campaign.GetCallsignOffset() == SCR_MilitaryBaseComponent.INVALID_BASE_CALLSIGN)
			return false;

		return true;
	}
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleResourceName("m_sBaseName", true)]
class SCR_CampaignCustomBase
{
	[Attribute("", UIWidgets.EditBox, "Base entity name as set up in World Editor.")]
	protected string m_sBaseName;

	[Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox)]
	protected bool m_bIsControlPoint;

	[Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox)]
	protected bool m_bCanBeHQ;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox)]
	protected bool m_bDisableWhenUnusedAsHQ;

	[Attribute("-1", desc: "Use -1 for default base setting.")]
	protected float m_fRadioRange;

	//------------------------------------------------------------------------------------------------
	//! \return
	string GetBaseName()
	{
		return m_sBaseName;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	bool IsControlPoint()
	{
		return m_bIsControlPoint;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool GetCanBeHQ()
	{
		return m_bCanBeHQ;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool GetDisableWhenUnusedAsHQ()
	{
		return m_bDisableWhenUnusedAsHQ;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetRadioRange()
	{
		return m_fRadioRange;
	}
}

enum SCR_ECampaignBaseType
{
	BASE,
	RELAY,
	SOURCE_BASE
}

enum EFactionMapID
{
	UNKNOWN = 0,
	EAST = 1,
	WEST = 2,
	FIA = 3
}
