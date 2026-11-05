class SCR_ResourceEntityRefundAction : SCR_ScriptedUserAction
{
	[Attribute(defvalue: EResourceType.SUPPLIES.ToString(), uiwidget: UIWidgets.ComboBox, desc: "Sets the type of Resource to be used.\nOnly a transaction matching Resource types can be successfully concluded.", enums: ParamEnumArray.FromEnum(EResourceType))]
	protected EResourceType m_eResourceType;
	
	[Attribute(defvalue: EResourceGeneratorID.DEFAULT_STORAGE.ToString(), uiwidget: UIWidgets.ComboBox, desc: "Identifier for the generator used for storage", enums: ParamEnumArray.FromEnum(EResourceGeneratorID))]
	protected EResourceGeneratorID m_eGeneratorIdentifier;
	
	[Attribute("#AR-Supplies_Refund_Action_Object_NoSupplies", desc: "If no supplies are returned or supplies are disabled action name")]
	protected LocalizedString m_sNoSuppliesActionName;
	
	[Attribute("#AR-Supplies_Refund_Action_Vehicle_NoDepot", desc: "Invalid reason shown when there is no valid refund point")]
	protected LocalizedString m_sInvalidNoValidRefundPoint;
	
	[Attribute("#AR-Supplies_Refund_Action_Vehicle_Occupied", desc: "Invalid reason shown when entity is a vehicle and it is occupied")]
	protected LocalizedString m_sInvalidIsOccupied;

	[Attribute("#AR-ActionInvalid_HostileFaction", desc: "Invalid reason shown when the user is not friendly with the base the entity is in")]
	protected LocalizedString m_sInvalidEnemyBase;

	[Attribute(defvalue: "1", desc: "Determines if supplies that are still in this object should be redistributed when object is refunded.")]
	protected bool m_bAccountForStoredSupplies;

	[Attribute(defvalue: "0", desc: "Determines if overall damage state of the object should play a role in calculation of the refund price.")]
	protected bool m_bEvaluateDamageWhenCalculatingPrice;

	[Attribute(defvalue: "1", desc: "Factor used for calculating the overall state of the vehicle when computing the impact of undamaged hit zone.\nFactor of individual hit zone is calculated by multipling damageStateFactor by MaxHtiZoneHealth.\nFactors from all physical hit zones are added and then divided by the total amount of max health of all these hit zones.", params: "0 1 0.01")]
	protected float m_fUndamagedStateFactor;

	[Attribute(defvalue: "0.6", desc: "Factor used for calculating the overall state of the vehicle when computing the impact of damaged hit zone.\nFactor of individual hit zone is calculated by multipling damageStateFactor by MaxHtiZoneHealth.\nFactors from all physical hit zones are added and then divided by the total amount of max health of all these hit zones.", params: "0 1 0.01")]
	protected float m_fDamagedStateFactor;
	
	[Attribute(defvalue: "0.1", desc: "Factor used for calculating the overall state of the vehicle when computing the impact of destroyed hit zone.\nFactor of individual hit zone is calculated by multipling damageStateFactor by MaxHtiZoneHealth.\nFactors from all physical hit zones are added and then divided by the total amount of max health of all these hit zones.", params: "0 1 0.01")]
	protected float m_fDestroyedStateFactor;

	protected SCR_Faction m_Faction;
	protected SCR_EntityCatalogManagerComponent m_EntityCatalogManager;
	protected SCR_CatalogEntitySpawnerComponent m_CatalogEntitySpawnerComponent;
	protected RplComponent m_ReplicationComponent;
	protected SCR_ResourceGenerator m_ResourceGenerator;
	protected SCR_ResourceComponent m_OwnerResourceComp;
	protected SCR_DamageManagerComponent m_DamageManagerComp;
	protected float m_fResourceCost = -1;
	protected float m_fTimeUntilNextDamageCheck;
	protected float m_fLastOverallDamageState = 1;
	
	protected bool m_bIsResourceEnabled = true;
	
	protected static const int DAMAGE_STATE_EVALUATION_COOLDOWN_SECONDS = 5;
	protected static ref ScriptInvokerEntity2 s_OnRefundPerformed;//Entity refundedEntity, Entity playerEntity

	//~ For vehicles it sets the compartment manager to check if it is occupied
	protected SCR_BaseCompartmentManagerComponent m_CompartmentManager;
	
	//! Get event called when player refunds some entity.
	//! Invoker params are: Entity refundedEntity, Entity playerEntity
	//! \return ScriptInvokerEntity2
	static ScriptInvokerEntity2 GetOnRefundPerformed()
	{
		if (!s_OnRefundPerformed)
			s_OnRefundPerformed = new ScriptInvokerEntity2();

		return s_OnRefundPerformed;
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
 	{
		if (!m_ReplicationComponent || !m_ReplicationComponent.IsMaster())
			return;

		if (!SelectSuitableResourceGenerator(pUserEntity) || !m_CatalogEntitySpawnerComponent)
			return;

		//~ Vehicle is in use
		if (m_CompartmentManager && m_CompartmentManager.AnyCompartmentsOccupiedOrLocked())
			return;

		//~ Server-side guard: user must be friendly with the base the entity is in
		if (!IsUserFriendlyToBase(pUserEntity))
			return;

		//~ Refund if resource is enabled
		if (m_bIsResourceEnabled)
			m_ResourceGenerator.RequestGeneration(CalculateObjectRefundValue());
		
		if (s_OnRefundPerformed)
			s_OnRefundPerformed.Invoke(pOwnerEntity, pUserEntity);

		RplComponent.DeleteRplEntity(GetOwner(), false);
	}

	//------------------------------------------------------------------------------------------------
	protected float GetAmountOfStoredSupplies()
	{
		if (!m_bAccountForStoredSupplies)
			return 0;

		if (!m_OwnerResourceComp)
		{
			m_OwnerResourceComp = SCR_ResourceComponent.FindResourceComponent(GetOwner());
			if (!m_OwnerResourceComp)
				return 0;
		}

		float storedResources;
		SCR_ResourceSystemHelper.GetStoredResources(m_OwnerResourceComp, storedResources, m_eResourceType);
		return storedResources;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CalculateOverallDamageState()
	{
		if (!m_bEvaluateDamageWhenCalculatingPrice)
			return;

		const IEntity owner = GetOwner();
		if (!owner)
			return;

		const ChimeraWorld world = ChimeraWorld.CastFrom(owner.GetWorld());
		if (!world)
			return;

		const TimeAndWeatherManagerEntity timeAndWeatherManager = world.GetTimeAndWeatherManager();
		if (!timeAndWeatherManager)
			return;

		const float currentTime = timeAndWeatherManager.GetEngineTime();
		if (currentTime < m_fTimeUntilNextDamageCheck)
			return;

		m_fTimeUntilNextDamageCheck = currentTime + DAMAGE_STATE_EVALUATION_COOLDOWN_SECONDS;
		m_fLastOverallDamageState = 1; // reset to pristine state to not penalize if damage manager is not present
		if (!m_DamageManagerComp)
		{
			m_DamageManagerComp = SCR_DamageManagerComponent.GetDamageManager(owner);
			if (!m_DamageManagerComp)
				return;
		}

		array<HitZone> physicalHitZones();
		m_DamageManagerComp.GetPhysicalHitZones(physicalHitZones);
		float totalMaxHealth;
		float totalFactor;
		foreach (HitZone hz : physicalHitZones)
		{
			// Health scaled cannot be used for this, as there is no guarantee that it will be correct on all clients, thus states must be used 
			EDamageState state = hz.GetDamageState();
			float maxHealth = hz.GetMaxHealth();
			float factor;
			switch (state)
			{
				case EDamageState.DESTROYED:
					factor = m_fDestroyedStateFactor;
					break;

				case EDamageState.INTERMEDIARY:
					factor = m_fDamagedStateFactor;
					break;

				default:
					factor = m_fUndamagedStateFactor;
			}

			totalFactor += factor * maxHealth;
			totalMaxHealth += maxHealth;
		}

		if (totalMaxHealth > 0)
			totalFactor /= totalMaxHealth;
		else
			totalFactor = 1;

		m_fLastOverallDamageState = totalFactor;
	}

	//------------------------------------------------------------------------------------------------
	protected float CalculateObjectRefundValue()
	{
		if (!m_CatalogEntitySpawnerComponent)
			return 0;

		const IEntity owner = GetOwner();
		float resourceCost = GetAmountOfStoredSupplies();
		CalculateOverallDamageState();

		if (m_CatalogEntitySpawnerComponent.IsInGracePeriod(owner))
			resourceCost += m_fResourceCost * m_ResourceGenerator.GetResourceMultiplier();
		else
		{
			float priceFactor = m_CatalogEntitySpawnerComponent.GetRefundPriceFactor(m_fLastOverallDamageState);
			resourceCost += m_fResourceCost * m_CatalogEntitySpawnerComponent.GetPostGracePeriodRefundMultiplier() * priceFactor;
		}

		return SCR_ResourceSystemHelper.RoundRefundSupplyAmount(resourceCost);
	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		if (!m_CatalogEntitySpawnerComponent)
			return false;
		
		if (m_bIsResourceEnabled)
		{
			float resourceCost = CalculateObjectRefundValue();

			if (resourceCost > 0)
			{
				ActionNameParams[0] = SCR_ResourceSystemHelper.SuppliesToString(resourceCost);
			}
			else 
			{
				outName = m_sNoSuppliesActionName;
				return true;
			}
		}
		else 
		{
			outName = m_sNoSuppliesActionName;
			return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override event bool CanBePerformedScript(IEntity user)
	{
		/*if (!SelectSuitableResourceGenerator(user))
		{
			m_sCannotPerformReason = m_sInvalidNoValidRefundPoint;
			return false;
		}*/
		
		//~ Additional settings has disabled the action so do not perform and show disabled reason
		SCR_AdditionalGameModeSettingsComponent additionalSettings = SCR_AdditionalGameModeSettingsComponent.GetInstance();
		if (additionalSettings && !additionalSettings.IsEntityRefundingActionAllowed())
		{
			m_sCannotPerformReason = additionalSettings.GetEntityRefundingDisabledReason();
			return false;
		}
			
		//~ Check if vehicle is in use
		if (m_CompartmentManager && m_CompartmentManager.AnyCompartmentsOccupiedOrLocked())
		{
			m_sCannotPerformReason = m_sInvalidIsOccupied;
			return false;
		}

		//~ Prevent griefing: only users friendly to the base controlling this area may refund entities inside it
		if (!IsUserFriendlyToBase(user))
		{
			m_sCannotPerformReason = m_sInvalidEnemyBase;
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! True if the entity is outside every base, or the user's faction is friendly to the base that currently controls the area.
	protected bool IsUserFriendlyToBase(notnull IEntity user)
	{
		IEntity owner = GetOwner();
		if (!owner)
			return false;

		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		if (!campaign)
			return true;

		SCR_CampaignMilitaryBaseManager baseManager = campaign.GetBaseManager();
		if (!baseManager)
			return true;

		vector ownerPos = owner.GetOrigin();
		SCR_CampaignMilitaryBaseComponent nearestBase = baseManager.FindClosestBase(ownerPos);
		if (!nearestBase)
			return true;

		if (vector.DistanceXZ(ownerPos, nearestBase.GetOwner().GetOrigin()) > nearestBase.GetRadius())
			return true;

		Faction baseFaction = nearestBase.GetFaction();
		if (!baseFaction)
			return true;

		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(user);
		if (!character)
			return false;
		
		Faction userFaction = character.GetFaction();
		if (!userFaction)
			return false;

		return baseFaction.IsFactionFriendly(userFaction);
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Finds and selects a suitable resource generator and return if it was found or not.
	\return true if a resource generator was found, false otherwise.
	*/
	bool SelectSuitableResourceGenerator(IEntity user)
	{
		SCR_ResourceComponent resourceComponent;
		SCR_ResourceGenerator resourceGeneratorCompare;
		m_ResourceGenerator = null;
		m_CatalogEntitySpawnerComponent = null;
		vector ownerOrigin = GetOwner().GetOrigin();
		float distanceSq = float.MAX;
		float distanceSqCompare = float.MAX;
		
		foreach (SCR_CatalogEntitySpawnerComponent component : SCR_CatalogEntitySpawnerComponent.INSTANCES)
		{
			distanceSqCompare = vector.DistanceSq(ownerOrigin, component.GetOwner().GetOrigin());
			
			if (distanceSq <= distanceSqCompare && !component.CanRefund(GetOwner(), user))
				continue;
			
			resourceComponent = SCR_ResourceComponent.Cast(component.GetOwner().FindComponent(SCR_ResourceComponent));
			
			if (resourceComponent && resourceComponent.GetGenerator(EResourceGeneratorID.DEFAULT, m_eResourceType, resourceGeneratorCompare) && distanceSqCompare < resourceGeneratorCompare.GetStorageRange() * resourceGeneratorCompare.GetStorageRange())
			{
				distanceSq = distanceSqCompare;
				m_CatalogEntitySpawnerComponent = component;
				m_ResourceGenerator = resourceGeneratorCompare;
				
				continue;
			}
			
			resourceGeneratorCompare = null;
		}
		
		//~ Is resource enabled
		if (resourceComponent)
			m_bIsResourceEnabled = resourceComponent.IsResourceTypeEnabled(m_eResourceType);
		else
			m_bIsResourceEnabled = SCR_ResourceSystemHelper.IsGlobalResourceTypeEnabled(m_eResourceType);
		
		return m_ResourceGenerator;
	}
	
	//------------------------------------------------------------------------------------------------
	override event bool CanBeShownScript(IEntity user)
	{
		if (m_fResourceCost <= 0 || !m_ReplicationComponent)
			return false;
		
		if (!SelectSuitableResourceGenerator(user))
			return false;
		
		//~ Additional settings has disabled the action and the reason for disabling is set empty so hide the action
		SCR_AdditionalGameModeSettingsComponent additionalSettings = SCR_AdditionalGameModeSettingsComponent.GetInstance();
		if (additionalSettings && (!additionalSettings.IsEntityRefundingActionAllowed() && additionalSettings.GetEntityRefundingDisabledReason().IsEmpty()))
			return false;
		
		return super.CanBeShownScript(user);
	}
	
	//------------------------------------------------------------------------------------------------
	override event void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		//~ Call a frame later so the catalog is initialized correctly
		GetGame().GetCallqueue().CallLater(DelayedInit, param1: pOwnerEntity, param2: pManagerComponent);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DelayedInit(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_fResourceCost = -1;
		
		if (!pOwnerEntity)
			return;
		
		m_ReplicationComponent = RplComponent.Cast(pOwnerEntity.FindComponent(RplComponent));
		m_EntityCatalogManager = SCR_EntityCatalogManagerComponent.GetInstance();
		
		if (!m_EntityCatalogManager)
			return;
		
		SCR_EntityCatalogEntry entry;
		ResourceName prefabName = pOwnerEntity.GetPrefabData().GetPrefabName();
		
		SCR_FactionAffiliationComponent factionComponent = SCR_FactionAffiliationComponent.Cast(GetOwner().FindComponent(SCR_FactionAffiliationComponent));
		if (factionComponent)
			m_Faction = SCR_Faction.Cast(factionComponent.GetDefaultAffiliatedFaction());
		
		//~ Has faction get it from faction
		if (m_Faction)
		{
			entry = m_EntityCatalogManager.GetEntryWithPrefabFromGeneralOrFactionCatalog(EEntityCatalogType.VEHICLE, prefabName, m_Faction);
		
			if (!entry)
			{
				entry = m_EntityCatalogManager.GetEntryWithPrefabFromGeneralOrFactionCatalog(EEntityCatalogType.CHARACTER, prefabName, m_Faction);
				
				if (!entry)
					entry = m_EntityCatalogManager.GetEntryWithPrefabFromGeneralOrFactionCatalog(EEntityCatalogType.GROUP, prefabName, m_Faction);
			}
		}
		//~ Does not have faction so get factionless
		else 
		{
			entry = m_EntityCatalogManager.GetEntryWithPrefabFromCatalog(EEntityCatalogType.VEHICLE, prefabName);
		
			if (!entry)
			{
				entry = m_EntityCatalogManager.GetEntryWithPrefabFromCatalog(EEntityCatalogType.CHARACTER, prefabName);
			
				if (!entry)
					entry = m_EntityCatalogManager.GetEntryWithPrefabFromCatalog(EEntityCatalogType.GROUP, prefabName);
			}
		}
		
		//~ No entry found
		if (!entry)
			return;
		
		//~ Get compartment manager if vehicle
		m_CompartmentManager = SCR_BaseCompartmentManagerComponent.Cast(pOwnerEntity.FindComponent(SCR_BaseCompartmentManagerComponent));
		m_OwnerResourceComp = SCR_ResourceComponent.FindResourceComponent(pOwnerEntity);
		m_DamageManagerComp = SCR_DamageManagerComponent.GetDamageManager(pOwnerEntity);
		
		SCR_EditableEntityUIInfo uiInfo = SCR_EditableEntityUIInfo.Cast(entry.GetEntityUiInfo());
		if (uiInfo)
		{
			array<ref SCR_EntityBudgetValue> budgets = {};
			uiInfo.GetEntityAndChildrenBudgetCost(budgets);
			
			foreach (SCR_EntityBudgetValue budget : budgets)
			{
				if (budget.GetBudgetType() == EEditableEntityBudget.CAMPAIGN)
				{
					m_fResourceCost = budget.GetBudgetValue();
					return;
				}
			}
		}
		
		//~ Could not get budget value from editable entity so use spawner data instead if any
		SCR_EntityCatalogSpawnerData data = SCR_EntityCatalogSpawnerData.Cast(entry.GetEntityDataOfType(SCR_EntityCatalogSpawnerData));
		if (!data)
			return;
		
		m_fResourceCost = data.GetSupplyCost();
	}
}
