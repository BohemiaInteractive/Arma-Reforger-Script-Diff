[EntityEditorProps(category: "GameScripted/Building", description: "Component attached to a provider, responsible for basic provider behaviour.")]
class SCR_CampaignBuildingProviderComponentClass : SCR_MilitaryBaseLogicComponentClass
{
}

class SCR_CampaignBuildingProviderComponent : SCR_MilitaryBaseLogicComponent
{
	[Attribute("", UIWidgets.EditBox, "Name of provider shown in provider interface", "")]
	protected string m_sProviderDisplayName;
	
	[Attribute("0", UIWidgets.CheckBox, "Can the building mode at this provider executed only via user action?")]
	protected bool m_bUserActionActivationOnly;
	
	[Attribute("0", UIWidgets.CheckBox, "Can be used by any faction")]
	protected bool m_bAnyFactionCanUse;

	[Attribute("0", UIWidgets.CheckBox, "Master provider is for example HQ with other service (providers) in vicinity.")]
	protected bool m_bIsMasterProvider;
	
	[Attribute("0", UIWidgets.CheckBox, "When opening a Free Roam Building mode, try to use master provider. Master provider is HQ where the service is registered in.")]
	protected bool m_bUseMasterProvider;

	[Attribute("0", UIWidgets.CheckBox, "Reads data(EEditableEntityLabel) from available providers as and extends the building option.")]
	protected bool m_bUseAllAvailableProviders;
		
	[Attribute("0", UIWidgets.CheckBox, "Register at nearby base, if available.")]
	protected bool m_bRegisterAtBase;
	
	[Attribute("0", UIWidgets.CheckBox, "If set, player can command friendly AI in building radius.")]
	protected bool m_bCanCommandAI;
	
	[Attribute("1", UIWidgets.CheckBox, "Obstruct view when an enemy is present within the radius of the building.")]
	protected bool m_bObstructViewWhenEnemyInRange;

	[Attribute("50", UIWidgets.EditBox, "Building radius")]
	protected float m_fBuildingRadius;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.ComboBox, desc: "Minimal rank that allows player to use the provider to build structures.", enums: ParamEnumArray.FromEnum(SCR_ECharacterRank))]
	protected SCR_ECharacterRank m_iRank;
	
	[Attribute(desc: "Fill in the budgets to be used with this provider")]
	protected ref array<ref SCR_CampaignBuildingBudgetToEvaluateData> m_aBudgetsToEvaluate;

	[Attribute(desc: "Traits this provider will provide. Each trait represents a tab in building interface. The tabs have to be defined in building mode's SCR_ContentBrowserEditorComponent.", UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EEditableEntityLabel))]
	protected ref array<EEditableEntityLabel> m_aAvailableTraits;

	//! Current props Value represents, how many entities with set prop budget can be spawned with this provider. The max number is limited by Prop budget.
	[RplProp()]
	protected int m_iCurrentPropValue;
	
	//! Current AI Value represents, how many AI is currently spawned with this provider. The max number is limited by AI budget.
	[RplProp()]
	protected int m_iCurrentAIValue;

	protected SCR_ResourceComponent m_ResourceComponent;

	protected ref array<int> m_aActiveUsersIDs = {};
	protected ref array<int> m_aAvailableUsersIDs = {};
	protected ref array<SCR_CampaignBuildingBudgetToEvaluateData> m_aShownBudget = {};

	protected static ref ScriptInvokerVoid s_OnProviderCreated = new ScriptInvokerVoid();
	protected ref ScriptInvokerVoid m_OnCooldownLockUpdated;

	protected const int MOVING_CHECK_PERIOD = 1000;
	protected const int PROVIDER_SPEED_TO_REMOVE_BUILDING_SQ = 1;
	
	protected ref array<ref Tuple2<int, WorldTimestamp>> m_aPlacingCooldown = {};
	protected bool m_bCooldownClientLock;
	protected bool m_bUseAllAvailableProvidersByPlayer;
	
	SCR_CampaignBuildingProviderComponent m_MasterProviderComponent;
	
	private ref map<EEditableEntityBudget, int> m_accumulatedBudgetChanges = new map<EEditableEntityBudget, int>;
	bool m_changesAccumulated = false;
	
	//------------------------------------------------------------------------------------------------
	void AccumulateBudgetChange(EEditableEntityBudget budgetType, int amount)
	{
		m_accumulatedBudgetChanges[budgetType] = m_accumulatedBudgetChanges[budgetType] + amount;
		
		if(!m_changesAccumulated)
		{
			GetGame().GetCallqueue().CallLater(ClearAccumulatedBudgetChanges);
			m_changesAccumulated = true;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	int GetAccumulatedBudgetChanges(EEditableEntityBudget type)
	{
		int value = 0;
		bool found = m_accumulatedBudgetChanges.Find(type, value);
		
		if(found)
			return value;
		
		return -1;
	}
	
	//------------------------------------------------------------------------------------------------
	void ClearAccumulatedBudgetChanges()
	{
		m_accumulatedBudgetChanges.Clear();
		m_changesAccumulated = false;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsThereEnoughSupplies(int availableSupplies, int supplyCost, int accumulatedSupplyCost)
	{
		int totalSupplyCost = supplyCost;
		
		if(accumulatedSupplyCost != -1)
			totalSupplyCost += accumulatedSupplyCost;
		
		return totalSupplyCost <= availableSupplies;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsThereEnoughBudgetToSpawn(notnull array<ref SCR_EntityBudgetValue> budgetCosts)
	{
		if(budgetCosts.IsEmpty())
			return true;
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		
		foreach(SCR_EntityBudgetValue budget : budgetCosts)
		{
			const EEditableEntityBudget budgetType = budget.GetBudgetType();

			SCR_CampaignBuildingBudgetToEvaluateData data = GetBudgetData(budgetType);
		
			//budget shouldn't be evaluated, so we don't care
			if(!data)
				continue;
			
			SCR_CampaignBuildingProviderComponent realProvider = this;
			const int currentBudgetValue = GetBudgetValue(budgetType, realProvider);
			
			const int budgetIncrease = budget.GetBudgetValue();
			const int accumulatedBudgetChanges = realProvider.GetAccumulatedBudgetChanges(budgetType);

			if(budgetType == EEditableEntityBudget.CAMPAIGN)	
			{
				//if supplies budget is disabled by GM we dont care
				if(!gameMode.IsResourceTypeEnabled(EResourceType.SUPPLIES))
					continue;
				
				bool enoughSupplies = realProvider.IsThereEnoughSupplies(currentBudgetValue, budgetIncrease, accumulatedBudgetChanges);
				
				//not enough supplies, we return false
				if(!enoughSupplies)
					return false;
				
				//enough supplies, continue checking for other budgets
				continue;
			}
			
			//max budget is unlimited, always allowed
			const int maxBudgetValue = GetMaxBudgetValueFromMasterIfNeeded(budgetType);
			if(maxBudgetValue == -1)
				continue;

			if(budgetIncrease + accumulatedBudgetChanges + currentBudgetValue > maxBudgetValue)
				return false;
		}
		
		foreach(SCR_EntityBudgetValue budget : budgetCosts)
		{
			const EEditableEntityBudget budgetType = budget.GetBudgetType();
			SCR_CampaignBuildingProviderComponent realProvider = this;
			
			//get real provider if needed
			const int currentBudgetValue = GetBudgetValue(budgetType, realProvider);
			
			realProvider.AccumulateBudgetChange(budgetType, budget.GetBudgetValue());
		}
		
		//we accumulate supplies changes here
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetBudgetValue(EEditableEntityBudget type, out SCR_CampaignBuildingProviderComponent componentToUse)
	{
		bool useMaster = UseMasterProviderBudget(EEditableEntityBudget.PROPS, componentToUse);
		
		if(type == EEditableEntityBudget.PROPS)
			return GetCurrentPropValue();
		
		if(type == EEditableEntityBudget.AI)
			return GetCurrentAIValue();
		
		//if its not supplies I have no idea how are we suppossed to handle it xdd
		if(type != EEditableEntityBudget.CAMPAIGN)
			return -1;
		
		//check supplies
		SCR_ResourceComponent resource = componentToUse.GetResourceComponent();
		
		if(!resource)
			return false;
		
		SCR_ResourceConsumer consumer = resource.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);
		//Get number of supplies
		
		float currentSupplies = consumer.GetAggregatedResourceValue();
		return currentSupplies;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return name of the provider
	string GetProviderDisplayName()
	{
		return m_sProviderDisplayName;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Should this provider use a master provider?
	bool UseMasterProvider()
	{
		return m_bUseMasterProvider;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Master provider can be for an example HQ where there are other services that can serve as providers (living area for an example) but master provider counts budget limits or define a building area.
	bool IsMasterProvider()
	{
		return m_bIsMasterProvider;
	}
	
	//------------------------------------------------------------------------------------------------
	bool UseAllAvailableProviders()
	{
		return m_bUseAllAvailableProviders || m_bUseAllAvailableProvidersByPlayer;
	}

	//------------------------------------------------------------------------------------------------
	void SetUseAllAvailableProvidersByPlayer(bool useByPlayer)
	{
		m_bUseAllAvailableProvidersByPlayer = useByPlayer;
	}

	//------------------------------------------------------------------------------------------------
	//! \return Can player from this provider control friendly AI in building radius.
	bool CanCommandAI()
	{
		return m_bCanCommandAI;
	}

	//------------------------------------------------------------------------------------------------
	//! \return bool When true, view will be obstructed when an enemy is present within the radius of the building.
	bool ObstrucViewWhenEnemyInRange()
	{
		return m_bObstructViewWhenEnemyInRange;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns master provider entity of this provider if any. If not, return itself.
	IEntity GetMasterProviderEntity()
	{		
		// Check bases this service is registered to. If non, it's standalone, return itself.
		array<SCR_MilitaryBaseComponent> bases = {};
		array<SCR_CampaignBuildingProviderComponent> providers = {};
		
		GetBases(bases);
		
		foreach (SCR_MilitaryBaseComponent base : bases)
		{
			base.GetBuildingProviders(providers);
			
			foreach (SCR_CampaignBuildingProviderComponent provider : providers)
			{
				if (provider.IsMasterProvider())
					return provider.GetOwner();	
			}
		}
		
		return GetOwner();
	}

	//------------------------------------------------------------------------------------------------
	//! Check if for this budget should be used budget of master provider.
	bool UseMasterProviderBudget(EEditableEntityBudget budget, out SCR_CampaignBuildingProviderComponent masterProviderComponent)
	{
		if (!m_aBudgetsToEvaluate || m_aBudgetsToEvaluate.IsEmpty())
			return false;
		
		IEntity masterProvider;
		
		foreach (SCR_CampaignBuildingBudgetToEvaluateData budgetData : m_aBudgetsToEvaluate)
		{
			if (budgetData.GetBudget() == budget && budgetData.UseMasterProviderBudget())
			{
				masterProvider = GetMasterProviderEntity();
				if (masterProvider == GetOwner())
					return false;
				
				masterProviderComponent = SCR_CampaignBuildingProviderComponent.Cast(masterProvider.FindComponent(SCR_CampaignBuildingProviderComponent));
				return true;
			}
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetClientLock(bool lock, IEntity provider, int playerId)
	{
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);
		if (!playerController)
			return;
		
		SCR_CampaignBuildingNetworkComponent buildingNetworkComponent = SCR_CampaignBuildingNetworkComponent.Cast(playerController.FindComponent(SCR_CampaignBuildingNetworkComponent));
		if (!buildingNetworkComponent)
			return;
		
		buildingNetworkComponent.SetClientLock(lock, provider);
	}
			
	//------------------------------------------------------------------------------------------------
	void SetCooldownClientLock(bool val)
	{
		m_bCooldownClientLock = val;
		
		if (m_OnCooldownLockUpdated)
			m_OnCooldownLockUpdated.Invoke();
	}	
	
	//------------------------------------------------------------------------------------------------
	//! Set the cooldown time. First calculate if there is a potential adjustment of the time, based on the budget setting of provider. Make loop running on server.
	void SetPlayerCooldown(int playerId, int cooldownTime)
	{
		ChimeraWorld world = GetOwner().GetWorld();
		if (!world)
			return;

 		m_aPlacingCooldown.Insert(new Tuple2<int, WorldTimestamp>(playerId,  world.GetServerTimestamp().PlusMilliseconds(CalculateCooldownTime(playerId, cooldownTime) * 1000)));
		
		SetClientLock(true, GetOwner(), playerId);
		GetGame().GetCallqueue().CallLater(UpdateCooldownTimer, 250, true, null);
	}	
	
	//------------------------------------------------------------------------------------------------
	//! Remove all cooldown times for player
	void RemovePlayerCooldowns(int playerId)
	{
		if (!m_aPlacingCooldown || m_aPlacingCooldown.IsEmpty())
			return;
		
		for (int i = m_aPlacingCooldown.Count() -1 ; i >= 0; i--)
		{
			if (m_aPlacingCooldown[i].param1 == playerId)
				m_aPlacingCooldown.Remove(i);
		}
		
		SetClientLock(false, GetOwner(), playerId);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Periodically called method to evaluate a current status of the cooldown.
	void UpdateCooldownTimer()
	{
		ChimeraWorld world = GetOwner().GetWorld();
		if (!world)
			return;
		
		bool cooldownChange;
		
		for (int i = m_aPlacingCooldown.Count() - 1; i >= 0; i--)
		{
			if (!m_aPlacingCooldown[i].param2.Greater(world.GetServerTimestamp()))
			{
				SetClientLock(false, GetOwner(), m_aPlacingCooldown[i].param1);
				m_aPlacingCooldown.Remove(i);
				cooldownChange = true;
			}
		}
		
		if (m_OnCooldownLockUpdated && cooldownChange)
				m_OnCooldownLockUpdated.Invoke();
		
		if (m_aPlacingCooldown.IsEmpty())
			GetGame().GetCallqueue().Remove(UpdateCooldownTimer);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Is placing allowed for this player?
	bool HasCooldownSet(int playerId)
	{		
		return m_bCooldownClientLock || !m_aPlacingCooldown.IsEmpty();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return current value of the cooldown time for a given player.
	float GetCooldownValue(int playerId)
	{
		float cooldown = 0.0;
		
		ChimeraWorld world = GetOwner().GetWorld();
		if (!world)
			return cooldown;
			
		foreach (Tuple2<int, WorldTimestamp> cooldownEntry : m_aPlacingCooldown)
		{
			if (cooldownEntry.param1 == playerId)
			{
				cooldown = cooldownEntry.param2.DiffMilliseconds(world.GetServerTimestamp()) * 0.001;
				return cooldown;
			}
		}
		
		return cooldown;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnCooldownLockUpdated()
	{
		if (!m_OnCooldownLockUpdated)
			m_OnCooldownLockUpdated = new ScriptInvokerVoid();

		return m_OnCooldownLockUpdated;
	}

	//------------------------------------------------------------------------------------------------
	override void RegisterBase(notnull SCR_MilitaryBaseComponent base)
	{
		if (!m_bRegisterAtBase)
			return;

		super.RegisterBase(base);
	}

	//------------------------------------------------------------------------------------------------
	//! \return if the provider can be registered at base or not.
	bool CanRegisterAtMilitaryBase()
	{
		return m_bRegisterAtBase;
	}

	//------------------------------------------------------------------------------------------------
	//! \return a military base component of the base this provider is registered to. If is registered to more than one, it returns the first one.
	SCR_MilitaryBaseComponent GetMilitaryBaseComponent()
	{
		array<SCR_MilitaryBaseComponent> bases = {};
		GetBases(bases);

		if (bases.IsEmpty())
			return null;

		return bases[0];
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return a Campaign military base component of the base this provider is registered to.
	SCR_CampaignMilitaryBaseComponent GetCampaignMilitaryBaseComponent()
	{
		array<SCR_MilitaryBaseComponent> bases = {};
		GetBases(bases);

		SCR_CampaignMilitaryBaseComponent campaignBase;
		foreach (SCR_MilitaryBaseComponent base : bases)
		{
			campaignBase = SCR_CampaignMilitaryBaseComponent.Cast(base);
			if (campaignBase)
				return campaignBase;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_ECharacterRank GetAccessRank()
	{
		return m_iRank;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetBuildingRadius()
	{
		return m_fBuildingRadius;
	}

	//------------------------------------------------------------------------------------------------
	//! Return max value of the given budget if this budget is added to be evaluated with this provider and has max value set.
	int GetMaxBudgetValue(EEditableEntityBudget budget)
	{
		SCR_CampaignBuildingBudgetToEvaluateData budgetData = GetBudgetData(budget);
		if (!budgetData)
			return -1;
		
		SCR_CampaignBuildingMaxValueBudgetToEvaluateData maxValueBudgetData = SCR_CampaignBuildingMaxValueBudgetToEvaluateData.Cast(budgetData);
		if (maxValueBudgetData)
			return maxValueBudgetData.GetMaxValue();
			
		return -1;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return max value of the given budget if this budget is added to be evaluated with this provider and has max value set.
	int GetMaxBudgetValueFromMasterIfNeeded(EEditableEntityBudget budget)
	{
		SCR_CampaignBuildingBudgetToEvaluateData budgetData = GetBudgetData(budget);
		if (!budgetData)
			return -1;
		
		SCR_CampaignBuildingProviderComponent masterProviderComponent;
		
		//if we should use the master's budget, also check master's budget no?
		if (UseMasterProviderBudget(budgetData.GetBudget(), masterProviderComponent))
			return masterProviderComponent.GetMaxBudgetValue(budget);
		
		//otherwise return normal max budget
		SCR_CampaignBuildingMaxValueBudgetToEvaluateData maxValueBudgetData = SCR_CampaignBuildingMaxValueBudgetToEvaluateData.Cast(budgetData);
		if (maxValueBudgetData)
			return maxValueBudgetData.GetMaxValue();
			
		return -1;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	int GetCurrentPropValue()
	{
		return m_iCurrentPropValue;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] value
	void SetPropValue(int value)
	{
		m_iCurrentPropValue = value;
		
		SCR_CampaignBuildingProviderComponent masterProviderComponent;
		
		if (UseMasterProviderBudget(EEditableEntityBudget.PROPS, masterProviderComponent))
			masterProviderComponent.SetPropValue(value);
		
		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] value
	void AddPropValue(int value)
	{
		m_iCurrentPropValue += value;
		
		SCR_CampaignBuildingProviderComponent masterProviderComponent;
		
		if (UseMasterProviderBudget(EEditableEntityBudget.PROPS, masterProviderComponent))
			masterProviderComponent.AddPropValue(value);
		
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	int GetCurrentAIValue()
	{
		SCR_CampaignBuildingProviderComponent masterProviderComponent;
		
		if (UseMasterProviderBudget(EEditableEntityBudget.AI, masterProviderComponent))
			return masterProviderComponent.GetCurrentAIValue();
		
		return m_iCurrentAIValue;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetAIValue(int value)
	{
		m_iCurrentAIValue = value;
		
		SCR_CampaignBuildingProviderComponent masterProviderComponent;
		
		if (UseMasterProviderBudget(EEditableEntityBudget.AI, masterProviderComponent))
			masterProviderComponent.SetAIValue(value);
		
		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	void AddAIValue(int value)
	{
		m_iCurrentAIValue += value;
		
		SCR_CampaignBuildingProviderComponent masterProviderComponent;
		
		if (UseMasterProviderBudget(EEditableEntityBudget.AI, masterProviderComponent))
			masterProviderComponent.AddAIValue(value);
		
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	//! An event called when AI spawned by this provider is killed or deleted
	protected void OnAIRemoved(IEntity ent)
	{
		RemoveEvents(ent);
		AddAIValue(-1);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set an event called when entity spawned by this provider get killed
	void SetOnEntityKilled(IEntity ent)
	{
		SCR_EditableCharacterComponent editableCharacter = SCR_EditableCharacterComponent.Cast(ent.FindComponent(SCR_EditableCharacterComponent));
		if (!editableCharacter)
			return;
		
		editableCharacter.GetOnDestroyed().Insert(OnAIRemoved);
		editableCharacter.GetOnDeleted().Insert(OnAIRemoved);
	}
	
	//------------------------------------------------------------------------------------------------
	//! remove all methods invoked to entity and related to it's dead or delete.
	protected void RemoveEvents(IEntity ent)
	{
		SCR_EditableCharacterComponent editableCharacter = SCR_EditableCharacterComponent.Cast(ent.FindComponent(SCR_EditableCharacterComponent));
		if (!editableCharacter)
			return;
		
		editableCharacter.GetOnDestroyed().Remove(OnAIRemoved);
		editableCharacter.GetOnDeleted().Remove(OnAIRemoved);
	}
	
	//------------------------------------------------------------------------------------------------
	array<EEditableEntityLabel> GetAvailableTraits()
	{
		array<EEditableEntityLabel> availableTraits = {};
		availableTraits.Copy(m_aAvailableTraits);

		// if Establishing Bases is disabled, remove Base building trait
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.Cast(GetGame().GetGameMode());
		if (campaign && !campaign.GetEstablishingBasesEnabled())
			availableTraits.RemoveItem(EEditableEntityLabel.TRAIT_BASE);

		// extends available traits from other providers traits
		if (!UseAllAvailableProviders())
			return availableTraits;

		array<SCR_MilitaryBaseComponent> bases = {};
		int count = GetBases(bases);
		if (count == 0)
			return availableTraits;

		array<SCR_CampaignBuildingProviderComponent> campaignBuildingProvides = {};
		int providerCount = bases[0].GetBuildingProviders(campaignBuildingProvides);
		bool isVehicle;
		array<EEditableEntityLabel> otherProviderAvailableTraits;
		int providerAvailableTraitCount;

		for (int i = 0; i < providerCount; i++)
		{
			if (campaignBuildingProvides[i].UseAllAvailableProviders())
				continue;

			// checks if the provider is in the vehicle and skips them, we need only providers from compositions
			isVehicle = Vehicle.Cast(campaignBuildingProvides[i].GetOwner().GetParent());
			if (isVehicle)
				continue;

			otherProviderAvailableTraits = campaignBuildingProvides[i].GetAvailableTraits();
			providerAvailableTraitCount = otherProviderAvailableTraits.Count();

			for (int j = 0; j < providerAvailableTraitCount; j++)
			{
				if (!availableTraits.Contains(otherProviderAvailableTraits[j]))
				{
					availableTraits.Insert(otherProviderAvailableTraits[j]);
				}
			}
		}

		return availableTraits;
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);

		super.OnPostInit(owner);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		if (System.IsConsoleApp() || !GetGame().GetPlayerController())
			SetOnProviderFactionChangedEvent();
	}

	//------------------------------------------------------------------------------------------------
	//! Add an ID of a user on the list of those who are currently in use of provider
	//! \param[in] userID
	void AddNewActiveUser(int userID)
	{
		m_aActiveUsersIDs.Insert(userID);
		SetOnPlayerDeathActiveUserEvent(userID);
	}

	//------------------------------------------------------------------------------------------------
	//! Remove an ID of a user from the list of those who are currently in use of provider
	//! \param[in] userID
	void RemoveActiveUser(int userID)
	{
		m_aActiveUsersIDs.RemoveItem(userID);
	}

	//------------------------------------------------------------------------------------------------
	//! Return the array of users id and it's count.
	//! \param[out] users
	int GetActiveUsers(out notnull array<int> users)
	{
		return users.Copy(m_aActiveUsersIDs);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] playerId
	//! \return
	bool ContainActiveUsers(int playerId)
	{
		return m_aActiveUsersIDs.Contains(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] userID
	void AddNewAvailableUser(int userID)
	{
		m_aAvailableUsersIDs.Insert(userID);
		SetOnPlayerDeathAvailableUserEvent(userID);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] userID
	void RemoveAvailableUser(int userID)
	{
		m_aAvailableUsersIDs.RemoveItem(userID);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[out] users
	//! \return
	int GetAvailableUsers(out array<int> users)
	{
		return users.Copy(m_aAvailableUsersIDs);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] playerId
	//! \return
	bool ContainAvailableUsers(int playerId)
	{
		return m_aAvailableUsersIDs.Contains(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! Requesting a building mode. If trigger exist (was spawned with provider, because "Y" can be used to enter the mode, it will open the mode directly. If not, it 1st spawn the building area trigger.
	//! \param[in] playerID
	//! \param[in] userActionUsed
	void RequestBuildingMode(int playerID, bool userActionUsed)
	{
		RequestEnterBuildingMode(playerID, userActionUsed);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] playerID
	//! \param[in] userActionUsed
	void RequestEnterBuildingMode(int playerID, bool userActionUsed)
	{
		SCR_CampaignBuildingNetworkComponent networkComponent = GetNetworkManager();
		if (!networkComponent)
			return;

		SCR_EditorManagerEntity editorManager = GetEditorManager();
		if (!editorManager)
			return;

		SetOnPlayerConsciousnessChanged();
		SetOnPlayerTeleported(playerID);
		editorManager.GetOnOpenedServer().Insert(BuildingModeCreated);
		editorManager.GetOnClosed().Insert(OnModeClosed);
		networkComponent.RequestEnterBuildingMode(GetOwner(), playerID, m_bUserActionActivationOnly, userActionUsed);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] playerID
	void SetOnPlayerTeleported(int playerID)
	{
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;

		SCR_PlayerTeleportedFeedbackComponent playerTeleportComponent = SCR_PlayerTeleportedFeedbackComponent.Cast(playerController.FindComponent(SCR_PlayerTeleportedFeedbackComponent));
		if (!playerTeleportComponent)
			return;

		playerTeleportComponent.GetOnPlayerTeleported().Insert(PlayerTeleported);
	}

	//------------------------------------------------------------------------------------------------
	//! Set even when player consciousness changed.
	void SetOnPlayerConsciousnessChanged()
	{
		ChimeraCharacter player = ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		if (!player)
			return;

		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(player.GetCharacterController());
		if (controller)
			controller.m_OnLifeStateChanged.Insert(OnLifeStateChanged);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] conscious
	void OnLifeStateChanged(ECharacterLifeState previousLifeState, ECharacterLifeState newLifeState)
	{
		if (newLifeState == ECharacterLifeState.ALIVE)
			return;

		SCR_CampaignBuildingNetworkComponent networkComponent = GetNetworkManager();
		if (!networkComponent)
			return;

		networkComponent.RemoveEditorMode(SCR_PlayerController.GetLocalPlayerId(), GetOwner());
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] character
	//! \param[in] isLongFade
	//! \param[in] teleportReason
	void PlayerTeleported(SCR_EditableCharacterComponent character, bool isLongFade, SCR_EPlayerTeleportedReason teleportReason)
	{
		SCR_CampaignBuildingNetworkComponent networkComponent = GetNetworkManager();
		if (!networkComponent)
			return;

		networkComponent.RemoveEditorMode(SCR_PlayerController.GetLocalPlayerId(), GetOwner());

	}

	//------------------------------------------------------------------------------------------------
	//!
	void RemoveOnLifeStateChanged()
	{
		ChimeraCharacter player = ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		if (!player)
			return;

		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(player.GetCharacterController());
		if (controller)
			controller.m_OnLifeStateChanged.Remove(OnLifeStateChanged);
	}

	//------------------------------------------------------------------------------------------------
	//! Insert a method called when the provider faction is changed, e.g a base is taken by an enemy.
	void SetOnProviderFactionChangedEvent()
	{
		SCR_FactionAffiliationComponent factionComponent = SCR_FactionAffiliationComponent.Cast(GetOwner().FindComponent(SCR_FactionAffiliationComponent));
		if (!factionComponent)
			return;

		factionComponent.GetOnFactionChanged().Insert(OnBaseOwnerChanged);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] userID
	void SetOnPlayerDeathActiveUserEvent(int userID)
	{
		SCR_CharacterControllerComponent comp = GetCharacterControllerComponent(userID);
		if (!comp)
			return;

		comp.GetOnPlayerDeathWithParam().Insert(OnActiveUserDeath);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] userID
	void SetOnPlayerDeathAvailableUserEvent(int userID)
	{
		SCR_CharacterControllerComponent comp = GetCharacterControllerComponent(userID);
		if (!comp)
			return;

		comp.GetOnPlayerDeathWithParam().Insert(OnAvailableUserDeath);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] characterControllerComponent
	//! \param[in] instigatorEntity
	//! \param[in] instigator
	void OnActiveUserDeath(SCR_CharacterControllerComponent characterControllerComponent, IEntity instigatorEntity, notnull Instigator instigator)
	{
		RemoveActiveUser(GetPlayerIdFromCharacterController(characterControllerComponent));
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] characterControllerComponent
	//! \param[in] instigatorEntity
	//! \param[in] instigator
	void OnAvailableUserDeath(SCR_CharacterControllerComponent characterControllerComponent, IEntity instigatorEntity, notnull Instigator instigator)
	{
		RemoveAvailableUser(GetPlayerIdFromCharacterController(characterControllerComponent));
	}

	//------------------------------------------------------------------------------------------------
	//!
	void BuildingModeCreated()
	{
		SCR_EditorManagerEntity editorManager = GetEditorManager();
		if (!editorManager)
			return;

		editorManager.GetOnOpenedServer().Remove(BuildingModeCreated);
				
		SCR_EditorModeEntity editorMode = editorManager.GetCurrentModeEntity();
		if (!editorMode)
			return;
		
		SCR_PlacingEditorComponent placingComponent = SCR_PlacingEditorComponent.Cast(editorMode.FindComponent(SCR_PlacingEditorComponent));
		if (!placingComponent)
			return;
		
		placingComponent.GetOnPlaceEntityServer().Insert(EntitySpawnedByProvider);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Event rised when the entity is spawned by this provider.
	void EntitySpawnedByProvider(int prefabID, SCR_EditableEntityComponent editableEntity)
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		if (!campaign)
			return;
		
		if (m_aActiveUsersIDs.IsEmpty())
			return;
		
		IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(m_aActiveUsersIDs[0]);
		campaign.OnEntityRequested(editableEntity.GetOwner(), player, SCR_Faction.Cast(GetEntityFaction(GetOwner())), this);
	}

	//------------------------------------------------------------------------------------------------
	//!
	void OnModeClosed()
	{
		RemoveOnModeClosed();
		RemoveOnLifeStateChanged();
		RemoveOnPlayerTeleported();
		
		m_aShownBudget.Clear();

		SCR_CampaignBuildingNetworkComponent networkComponent = GetNetworkManager();
		if (!networkComponent)
			return;

		networkComponent.RemoveEditorMode(SCR_PlayerController.GetLocalPlayerId(), GetOwner());
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	static ScriptInvokerVoid GetOnProviderCreated()
	{
		return s_OnProviderCreated;
	}

	//------------------------------------------------------------------------------------------------
	//!
	void RemoveOnModeClosed()
	{
		SCR_EditorManagerEntity editorManager = GetEditorManager();
		if (!editorManager)
			return;

		editorManager.GetOnClosed().Remove(OnModeClosed);
	}

	//------------------------------------------------------------------------------------------------
	//!
	void RemoveOnPlayerTeleported()
	{
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;

		SCR_PlayerTeleportedFeedbackComponent playerTeleportComponent = SCR_PlayerTeleportedFeedbackComponent.Cast(playerController.FindComponent(SCR_PlayerTeleportedFeedbackComponent));
		if (!playerTeleportComponent)
			return;

		playerTeleportComponent.GetOnPlayerTeleported().Remove(PlayerTeleported);
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool GetUserActionInitOnly()
	{
		return m_bUserActionActivationOnly;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	bool CanBeUsedByAnyFaction()
	{
		return m_bAnyFactionCanUse;
	}

	//------------------------------------------------------------------------------------------------
	//! \return Network Manager
	protected SCR_CampaignBuildingNetworkComponent GetNetworkManager()
	{
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return null;

		return SCR_CampaignBuildingNetworkComponent.Cast(playerController.FindComponent(SCR_CampaignBuildingNetworkComponent));
	}

	//------------------------------------------------------------------------------------------------
	//! \return local Editor Manager
	protected SCR_EditorManagerEntity GetEditorManager()
	{
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core)
			return null;

		return core.GetEditorManager();
	}

	//------------------------------------------------------------------------------------------------
	//! \return Editor manager by player ID
	protected SCR_EditorManagerEntity GetEditorManagerByID(int playerId)
	{
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core)
			return null;

		return core.GetEditorManager(playerId);
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_CharacterControllerComponent GetCharacterControllerComponent(int playerID)
	{
		IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
		if (!player)
			return null;

		return SCR_CharacterControllerComponent.Cast(player.FindComponent(SCR_CharacterControllerComponent));
	}

	//------------------------------------------------------------------------------------------------
	protected int GetPlayerIdFromCharacterController(SCR_CharacterControllerComponent characterControllerComponent)
	{
		if (!characterControllerComponent)
			return -1;

		IEntity ent = characterControllerComponent.GetOwner();
		if (!ent)
			return -1;

		return GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(ent);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Does entity faction one match faction of entity two.
	//! \param[in] entity
	//! \return
	bool IsEntityFactionSame(notnull IEntity ent1, notnull IEntity ent2)
	{
		Faction ent1Faction = GetEntityFaction(ent1);
		if (!ent1Faction)
			return false;

		Faction ent2Faction = GetEntityFaction(ent2);
		if (!ent2Faction)
			return false;

		return ent1Faction == ent2Faction;
	}

	//------------------------------------------------------------------------------------------------
	protected Faction GetEntityFaction(notnull IEntity ent)
	{
		FactionAffiliationComponent factionComp = FactionAffiliationComponent.Cast(ent.FindComponent(FactionAffiliationComponent));

		// Seacrch for the faction component on parent entities as not always is it on the same component as this one (vehicle for an example)
		while (!factionComp && ent)
		{
			ent = ent.GetParent();
			if (ent)
				factionComp = FactionAffiliationComponent.Cast(ent.FindComponent(FactionAffiliationComponent));
		}

		if (!factionComp)
			return null;

		Faction faction = factionComp.GetAffiliatedFaction();
		if (!faction)
			faction = factionComp.GetDefaultAffiliatedFaction();

		return faction;
	}

	//------------------------------------------------------------------------------------------------
	//! Method triggered when owning faction of provider has changed.
	//! \param[in] owner
	//! \param[in] previousFaction
	//! \param[in] newFaction
	protected void OnBaseOwnerChanged(FactionAffiliationComponent owner, Faction previousFaction, Faction newFaction)
	{
		// ToDo: In the future if the trigger activation will be enabled again, here I have to use a trigger range to find all entities which should be added and remove those on the list now.
		RemoveActiveUsers();
	}

	//------------------------------------------------------------------------------------------------
	//! Caches and returns the resource component.
	SCR_ResourceComponent GetResourceComponent()
	{
		if (!m_ResourceComponent)
			m_ResourceComponent = SCR_ResourceComponent.FindResourceComponent(GetOwner());
		
		return m_ResourceComponent;
	}

	//------------------------------------------------------------------------------------------------
	//! \return provider supply component - obsolete
	[Obsolete("SCR_CampaignBuildingProviderComponent.GetResourceComponent() should be used instead.")]
	SCR_CampaignSuppliesComponent GetSuppliesComponent()
	{
		return SCR_CampaignSuppliesComponent.Cast(GetOwner().FindComponent(SCR_CampaignSuppliesComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get data of given budget.
	SCR_CampaignBuildingBudgetToEvaluateData GetBudgetData(EEditableEntityBudget budget)
	{
		foreach (SCR_CampaignBuildingBudgetToEvaluateData budgetData : m_aBudgetsToEvaluate)
		{
			if (budgetData.GetBudget() == budget)
				return budgetData;
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns all budget types to evaluate with this provider
	int GetBudgetTypesToEvaluate(notnull out array<ref EEditableEntityBudget> budgets)
	{
		foreach (SCR_CampaignBuildingBudgetToEvaluateData budgetData : m_aBudgetsToEvaluate)
		{
			budgets.Insert(budgetData.GetBudget());
		}
		
		return budgets.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] blockingBudget
	//! \return whether or not this budget is suppose to be taken into account with this provider.
	bool IsBudgetToEvaluate(EEditableEntityBudget blockingBudget)
	{
		if (!m_aBudgetsToEvaluate)
			return false;
		
		foreach (SCR_CampaignBuildingBudgetToEvaluateData budgetData : m_aBudgetsToEvaluate)
		{
			if (budgetData.GetBudget() == blockingBudget && budgetData.CanBeUsed())
				return true;
		}

		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Evaluate all set budgets with this provider and return first one that is marked as to be shown in UI.
	EEditableEntityBudget GetShownBudget()
	{
		foreach (SCR_CampaignBuildingBudgetToEvaluateData budgetData : m_aBudgetsToEvaluate)
		{
			if (budgetData.CanShowBudgetInUI() && !m_aShownBudget.Contains(budgetData))
			{
				m_aShownBudget.Insert(budgetData);
				return budgetData.GetBudget();
			}	
		}
		
		return -1;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Checks the budget setting and calculate final cooldown time based on it.
	int CalculateCooldownTime(int playerId, int cooldownTime)
	{
		SCR_CampaignBuildingCooldownWithRankBudgetToEvaluateData cooldownBudgetData;
		
		foreach (SCR_CampaignBuildingBudgetToEvaluateData budgetData : m_aBudgetsToEvaluate)
		{
			if (budgetData.GetBudget() == EEditableEntityBudget.COOLDOWN)
			{
				cooldownBudgetData = SCR_CampaignBuildingCooldownWithRankBudgetToEvaluateData.Cast(budgetData);
				if (!cooldownBudgetData)
					continue;
				
				return cooldownBudgetData.CooldownTimeModifier(playerId, cooldownTime);
			}
		}
		
		return cooldownTime;
	}

	//------------------------------------------------------------------------------------------------
	//! Method called when the provider was destroyed or deleted to remove a provider.
	void RemoveActiveUsers()
	{
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return;

		SCR_CampaignBuildingManagerComponent buildingManagerComponent = SCR_CampaignBuildingManagerComponent.Cast(gameMode.FindComponent(SCR_CampaignBuildingManagerComponent));
		if (!buildingManagerComponent)
			return;

		bool isActiveUser;
		foreach (int playerId : m_aAvailableUsersIDs)
		{
			isActiveUser = false;

			SCR_EditorManagerEntity editorManager = GetEditorManagerByID(playerId);
			if (!editorManager)
				return;

			SCR_EditorModeEntity modeEntity = editorManager.FindModeEntity(EEditorMode.BUILDING);
			if (!modeEntity)
				return;

			if (m_aActiveUsersIDs.Contains(playerId))
			{
				RemoveActiveUser(playerId);
				isActiveUser = true;
			}

			buildingManagerComponent.RemoveProvider(playerId, this, isActiveUser);
			RemoveAvailableUser(playerId);
		}

		RemoveCheckProviderMove();
	}

	//------------------------------------------------------------------------------------------------
	//!
	void RemoveCheckProviderMove()
	{
		if (IsProviderDynamic())
			GetGame().GetCallqueue().Remove(CheckProviderMove);
	}

	//------------------------------------------------------------------------------------------------
	//!
	void SetCheckProviderMove()
	{
		if (IsProviderDynamic())
			GetGame().GetCallqueue().CallLater(CheckProviderMove, MOVING_CHECK_PERIOD, true);
	}

	//------------------------------------------------------------------------------------------------
	//! Check if the provider velocity in meters per second is faster then allowed. If so, terminate  a building mode.
	private void CheckProviderMove()
	{
		IEntity mainParent = SCR_EntityHelper.GetMainParent(GetOwner(), true);
		if (!mainParent)
			return;
		
		Physics providerPhysics = mainParent.GetPhysics();
		if (!providerPhysics)
			return;
		
		vector velocity = providerPhysics.GetVelocity();

		if ((velocity.LengthSq()) > PROVIDER_SPEED_TO_REMOVE_BUILDING_SQ)
			RemoveActiveUsers();
	}

	//------------------------------------------------------------------------------------------------
	//! Check if the provider is dynamic.
	private bool IsProviderDynamic()
	{
		IEntity mainParent = SCR_EntityHelper.GetMainParent(GetOwner(), true);
		if (!mainParent)
			return false;

		Physics providerPhysics = mainParent.GetPhysics();
		if (!providerPhysics)
			return false;

		return providerPhysics.IsDynamic();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check if given character faction is a hostile to player, or not.
	//! \param[in] char Character to be evaluated if is enemy or not.
	bool IsEnemyFaction(notnull ChimeraCharacter char)
	{
		FactionAffiliationComponent factionComponent = FactionAffiliationComponent.Cast(char.FindComponent(FactionAffiliationComponent));
		if (!factionComponent)
			return false;
		
		Faction faction = factionComponent.GetAffiliatedFaction();
		if (!faction )
			return false;
		
		SCR_CampaignFactionManager factionManager = SCR_CampaignFactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return false;
		
		Faction playerFaction = factionManager.GetLocalPlayerFaction();
		if (!playerFaction)
			return false;
		
		return playerFaction.IsFactionEnemy(faction);
	}

	//------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		s_OnProviderCreated.Invoke();
		return true;
	}

	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_CampaignBuildingProviderComponent()
	{
		if (!m_aAvailableUsersIDs.IsEmpty())
			RemoveActiveUsers();
	}
}
