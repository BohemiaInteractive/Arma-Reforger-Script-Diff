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
	
	protected SCR_Faction m_Faction;
	protected SCR_EntityCatalogManagerComponent m_EntityCatalogManager;
	protected SCR_CatalogEntitySpawnerComponent m_CatalogEntitySpawnerComponent;
	protected RplComponent m_ReplicationComponent;
	protected SCR_ResourceGenerator m_ResourceGenerator;
	protected float m_fResourceCost = -1;
	
	protected bool m_bIsResourceEnabled = true;
	
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

		//~ Refund if resource is enabled
		if (m_bIsResourceEnabled)
		{
			if (m_CatalogEntitySpawnerComponent.IsInGracePeriod(GetOwner()))
				m_ResourceGenerator.RequestGeneration(SCR_ResourceSystemHelper.RoundRefundSupplyAmount(m_fResourceCost * m_ResourceGenerator.GetResourceMultiplier()));
			else
				m_ResourceGenerator.RequestGeneration(SCR_ResourceSystemHelper.RoundRefundSupplyAmount(m_fResourceCost * m_CatalogEntitySpawnerComponent.GetPostGracePeriodRefundMultiplier()));
		}
		
		if (s_OnRefundPerformed)
			s_OnRefundPerformed.Invoke(pOwnerEntity, pUserEntity);

		RplComponent.DeleteRplEntity(GetOwner(), false);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		if (!m_CatalogEntitySpawnerComponent)
			return false;
		
		if (m_bIsResourceEnabled)
		{
			float resourceCost;
			
			if (m_CatalogEntitySpawnerComponent.IsInGracePeriod(GetOwner()))
				resourceCost = SCR_ResourceSystemHelper.RoundRefundSupplyAmount(m_fResourceCost * m_ResourceGenerator.GetResourceMultiplier());
			else
				resourceCost = SCR_ResourceSystemHelper.RoundRefundSupplyAmount(m_fResourceCost * m_CatalogEntitySpawnerComponent.GetPostGracePeriodRefundMultiplier());
		
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
		
		return true;
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
