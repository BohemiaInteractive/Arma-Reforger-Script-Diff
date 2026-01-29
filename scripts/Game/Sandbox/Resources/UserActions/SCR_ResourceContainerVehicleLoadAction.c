class SCR_ResourceContainerVehicleLoadAction : SCR_ScriptedUserAction
{
	[Attribute(defvalue: EResourceType.SUPPLIES.ToString(), uiwidget: UIWidgets.ComboBox, desc: "Sets the type of Resource to be used.\nOnly a transaction matching Resource types can be successfully concluded.", enums: ParamEnumArray.FromEnum(EResourceType))]
	protected EResourceType m_eResourceType;
	
	[Attribute("2000", "Amount of resources transfered on each action execute, -1 means max is transfer", params: "-1 inf")]
	protected float m_fTransferAmount;
	
	protected const LocalizedString OCCUPIED_BY_CHARACTER = "#AR-UserAction_SeatOccupied";
	
	protected SCR_ResourceSystemSubscriptionHandleBase m_ResourceSubscriptionHandleConsumer;
	protected SCR_ResourceSystemSubscriptionHandleBase m_ResourceSubscriptionHandleGenerator;
	protected SCR_ResourceComponent m_ResourceComponent;
	protected SCR_ResourceGenerator m_ResourceGenerator;
	protected SCR_ResourceConsumer m_ResourceConsumer;
	protected RplId m_ResourceInventoryPlayerComponentRplId;
	protected float m_fMaxStoredResource;
	protected float m_fCurrentResource;
	protected float m_fCurrentTransferValue;
	protected bool m_bCanPerform;
	
	protected SCR_BaseCompartmentManagerComponent m_CompartmentManager;
	
	//------------------------------------------------------------------------------------------------
	//~ If continues action it will only execute everytime the duration is done
	override void PerformContinuousAction(IEntity pOwnerEntity, IEntity pUserEntity, float timeSlice)
	{			
		if (!LoopActionUpdate(timeSlice))
			return; 
		
		PerformAction(pOwnerEntity, pUserEntity);
	}
	
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
 	{
		SCR_BaseGameMode gamemode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gamemode || !gamemode.IsMaster())
			return;
		
		if (!m_ResourceComponent 
		||	!m_ResourceGenerator	&& !m_ResourceComponent.GetGenerator(EResourceGeneratorID.VEHICLE_LOAD, m_eResourceType, m_ResourceGenerator)
		||	!m_ResourceConsumer		&& !m_ResourceComponent.GetConsumer(EResourceGeneratorID.VEHICLE_LOAD, m_eResourceType, m_ResourceConsumer))
			return;
		
		const int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(pUserEntity);
		const PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);
		
		if (!playerController)
			return;
		
		SCR_ResourcePlayerControllerInventoryComponent resourcePlayerControllerComponent = SCR_ResourcePlayerControllerInventoryComponent.Cast(playerController.FindComponent(SCR_ResourcePlayerControllerInventoryComponent));
		
		if (!resourcePlayerControllerComponent)
			return;
		
		GetResourceValues(m_fCurrentResource, m_fMaxStoredResource, m_fCurrentTransferValue);
		resourcePlayerControllerComponent.OnBeforePlayerInteraction(EResourcePlayerInteractionType.VEHICLE_LOAD, m_ResourceConsumer.GetComponent(), m_ResourceGenerator.GetComponent(), m_eResourceType, m_fCurrentTransferValue);
		m_ResourceConsumer.RequestConsumtion(m_fCurrentTransferValue);
		m_ResourceGenerator.RequestGeneration(m_fCurrentTransferValue);
		resourcePlayerControllerComponent.OnPlayerInteraction(EResourcePlayerInteractionType.VEHICLE_LOAD, m_ResourceConsumer.GetComponent(), m_ResourceGenerator.GetComponent(), m_eResourceType, m_fCurrentTransferValue);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void GetResourceValues(out float currentResource, out float maxResource, out float transferAmount)
	{
		currentResource = m_ResourceConsumer.GetAggregatedResourceValue();
		maxResource = Math.Min(m_ResourceGenerator.GetAggregatedMaxResourceValue() - m_ResourceGenerator.GetAggregatedResourceValue(), currentResource);
	
		if (m_fTransferAmount <= 0 || m_fTransferAmount > maxResource)
			transferAmount = maxResource;
		else 
			transferAmount = m_fTransferAmount;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		if (!m_bCanPerform
		||	!m_ResourceComponent 
		||	!m_ResourceInventoryPlayerComponentRplId.IsValid()
		||	!m_ResourceGenerator	&& !m_ResourceComponent.GetGenerator(EResourceGeneratorID.VEHICLE_LOAD, m_eResourceType, m_ResourceGenerator)
		||	!m_ResourceConsumer		&& !m_ResourceComponent.GetConsumer(EResourceGeneratorID.VEHICLE_LOAD, m_eResourceType, m_ResourceConsumer))
			return false;
		
		outName = WidgetManager.Translate(GetUIInfo().GetDescription(), m_fCurrentTransferValue, m_ResourceGenerator.GetAggregatedResourceValue(), m_ResourceGenerator.GetAggregatedMaxResourceValue());
		outName = WidgetManager.Translate("#AR-ActionFormat_SupplyCost", outName, m_fCurrentResource, m_ResourceConsumer.GetAggregatedMaxResourceValue());
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{	
		if (!super.CanBeShownScript(user))
			return false;

		if (!m_ResourceComponent || !m_ResourceComponent.IsResourceTypeEnabled(m_eResourceType))
			return false;

		Vehicle vehicle = Vehicle.Cast(GetOwner());
		if (!vehicle)
		{
			vehicle = Vehicle.Cast(GetOwner().GetRootParent());
			if (!vehicle)
				return false;
		}

		SCR_DamageManagerComponent damageMgr = vehicle.GetDamageManager();
		if (!damageMgr || damageMgr.GetState() == EDamageState.DESTROYED)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override event bool CanBePerformedScript(IEntity user)
	{	
		m_bCanPerform = false;
		
		//~ TODO: Hotfix until proper solution, only blocks player does not block AI or Editor actions
		if (m_CompartmentManager && m_CompartmentManager.BlockSuppliesIfOccupied() && m_CompartmentManager.GetOccupantCount() > 0)
		{
			SetCannotPerformReason(OCCUPIED_BY_CHARACTER);
			return false;
		}
		
		if (!m_ResourceInventoryPlayerComponentRplId || !m_ResourceInventoryPlayerComponentRplId.IsValid())
			m_ResourceInventoryPlayerComponentRplId = Replication.FindId(SCR_ResourcePlayerControllerInventoryComponent.Cast(GetGame().GetPlayerController().FindComponent(SCR_ResourcePlayerControllerInventoryComponent)));
		
		if (!m_ResourceComponent 
		||	!m_ResourceInventoryPlayerComponentRplId.IsValid()
		||	!m_ResourceGenerator	&& !m_ResourceComponent.GetGenerator(EResourceGeneratorID.VEHICLE_LOAD, m_eResourceType, m_ResourceGenerator)
		||	!m_ResourceConsumer		&& !m_ResourceComponent.GetConsumer(EResourceGeneratorID.VEHICLE_LOAD, m_eResourceType, m_ResourceConsumer))
		{
			SetCannotPerformReason("#AR-Supplies_CannotPerform_Generic");
			
			return false;
		}
		
		if (m_ResourceSubscriptionHandleConsumer)
			m_ResourceSubscriptionHandleConsumer.Poke();
		else
			m_ResourceSubscriptionHandleConsumer = GetGame().GetResourceSystemSubscriptionManager().RequestSubscriptionListenerHandleGraceful(m_ResourceConsumer, m_ResourceInventoryPlayerComponentRplId);
		
		if (m_ResourceSubscriptionHandleGenerator)
			m_ResourceSubscriptionHandleGenerator.Poke();
		else
			m_ResourceSubscriptionHandleGenerator = GetGame().GetResourceSystemSubscriptionManager().RequestSubscriptionListenerHandleGraceful(m_ResourceGenerator, m_ResourceInventoryPlayerComponentRplId);
		
		GetResourceValues(m_fCurrentResource, m_fMaxStoredResource, m_fCurrentTransferValue);
		
		if (m_ResourceConsumer.GetAggregatedMaxResourceValue() == 0.0)
			SetCannotPerformReason("#AR-Supplies_CannotPerform_Vehicle_NoStorage");
		else if (m_fCurrentResource == 0.0)
			SetCannotPerformReason("#AR-Supplies_CannotPerform_Generic");
		else if (m_fMaxStoredResource == 0.0)
			SetCannotPerformReason("#AR-Supplies_CannotPerform_Vehicle_StorageFull");
		
		m_bCanPerform = m_fMaxStoredResource > 0.0 && m_fMaxStoredResource <= m_fCurrentResource;
		
		return m_bCanPerform;
	}
	
	//------------------------------------------------------------------------------------------------
	override event void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_ResourceComponent = SCR_ResourceComponent.Cast(pOwnerEntity.FindComponent(SCR_ResourceComponent));
		
		if (!m_ResourceComponent)
			return;
		
		m_ResourceGenerator	= m_ResourceComponent.GetGenerator(EResourceGeneratorID.VEHICLE_LOAD, m_eResourceType);
		m_ResourceConsumer	= m_ResourceComponent.GetConsumer(EResourceGeneratorID.VEHICLE_LOAD, m_eResourceType);
		m_CompartmentManager = SCR_BaseCompartmentManagerComponent.Cast(pOwnerEntity.FindComponent(SCR_BaseCompartmentManagerComponent));
	}
}