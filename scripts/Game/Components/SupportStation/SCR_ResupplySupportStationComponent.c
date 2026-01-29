[ComponentEditorProps(category: "GameScripted/SupportStation", description: "")]
class SCR_ResupplySupportStationComponentClass : SCR_BaseItemSupportStationComponentClass
{
}

class SCR_ResupplySupportStationComponent : SCR_BaseItemSupportStationComponent
{
	//------------------------------------------------------------------------------------------------
	protected override bool InitValidSetup()
	{		
		//~ Resupply ammo does not support range as only players can resupply themselves or others at the moment
		if (UsesRange())
		{
			Print("'SCR_ResupplySupportStationComponent' does not support range. Make sure m_fRange is set to -1", LogLevel.ERROR);
			return false;
		}
		
		return super.InitValidSetup();
	}
	
	//------------------------------------------------------------------------------------------------
	override ESupportStationType GetSupportStationType()
	{
		return ESupportStationType.RESUPPLY_AMMO;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool IsValid(IEntity actionOwner, IEntity actionUser, SCR_BaseUseSupportStationAction action, vector actionPosition, out ESupportStationReasonInvalid reasonInvalid, out int supplyAmount)
	{
		if (!SCR_BaseResupplySupportStationAction.Cast(action))
		{
			Debug.Error2("SCR_ResupplySupportStationComponent", "'IsValid' fails as the resupply support station is executed with a non SCR_BaseResupplySupportStationAction action! This is will break the support station!");
			return false;
		}
		
		return super.IsValid(actionOwner, actionUser, action, actionPosition, reasonInvalid, supplyAmount);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnExecutedServer(notnull IEntity actionOwner, notnull IEntity actionUser, notnull SCR_BaseUseSupportStationAction action)
	{
		SCR_BaseResupplySupportStationAction resupplyAction = SCR_BaseResupplySupportStationAction.Cast(action);
		
		SCR_InventoryStorageManagerComponent inventoryManager = resupplyAction.GetTargetInventory();
		if (!inventoryManager)
			return;
		
		//~ Consume supplies
		if (AreSuppliesEnabled())
		{
			if (!OnConsumeSuppliesServer(GetSupplyAmountAction(actionOwner, actionUser, action)))
				return;
		}

		//~ Consume Military Supply Allocation if enabled
		if (SCR_ArsenalManagerComponent.IsMilitarySupplyAllocationEnabled())
			OnConsumeMilitarySupplyAllocationServer(actionUser, GetMilitarySupplyAllocationCostOfAction(action));

		map<ResourceName, int> itemsToResupply = new map<ResourceName, int>;
		itemsToResupply.Insert(resupplyAction.GetItemPrefab(), 1);
		inventoryManager.ResupplyMagazines(itemsToResupply);
		
		super.OnExecutedServer(actionOwner, actionUser, action);
	}	
	
	//------------------------------------------------------------------------------------------------
	//~ Called by OnExecuteBroadcast and is executed both on server and on client
	//~ playerId can be -1 if the user was not a player
	protected override void OnExecute(IEntity actionOwner, IEntity actionUser, int playerId, SCR_BaseUseSupportStationAction action)
	{
		//~ On succesfully executed
		OnSuccessfullyExecuted(actionOwner, actionUser, action);
		
		if (!actionUser || !actionOwner)
			return;
		
		SCR_BaseResupplySupportStationAction resupplyAction = SCR_BaseResupplySupportStationAction.Cast(action);
		
		SCR_InventoryStorageManagerComponent targetInventory = resupplyAction.GetTargetInventory();
		if (!targetInventory)
			return;
		
		//~ Player resupplied themselves
		if (targetInventory.GetOwner() == actionUser)
		{
			//~  Play sound on use
			PlaySoundEffect(GetOnUseAudioConfig(), actionOwner, action);
		
			//~ Play full done voice event (if any)
			PlayCharacterVoiceEvent(actionOwner);
			
			if (GetSendNotificationOnUse() && resupplyAction.GetNotificationOnUse() != ENotification.UNKNOWN)
			{
				//~ ID is not the local player so do not send notification
				playerId = SCR_PossessingManagerComponent.GetPlayerIdFromControlledEntity(actionUser);
				if (SCR_PlayerController.GetLocalPlayerId() == playerId)
					SCR_NotificationsComponent.SendLocal(resupplyAction.GetNotificationOnUse());
			}
		}
		//~ Player resupplied other	
		else 
		{			
			//~  Play sound on use
			PlaySoundEffect(GetOnUseAudioConfig(), actionUser, action);
		
			//~ Play full done voice event (if any)
			PlayCharacterVoiceEvent(actionUser);
			
			if (GetSendNotificationOnUse() && resupplyAction.GetNotificationOnUse() != ENotification.UNKNOWN)
			{
				//~ ID is not the local player so do not send notification
				playerId = SCR_PossessingManagerComponent.GetPlayerIdFromControlledEntity(actionOwner);
				if (SCR_PlayerController.GetLocalPlayerId() == playerId)
				{
					RplId ownerId;
					RplId userId;
					FindEntityIds(actionOwner, actionUser, ownerId, userId, playerId);
					SCR_NotificationsComponent.SendLocal(resupplyAction.GetNotificationOnUse(), userId);
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] actionUser
	//! \param[in] amount
	protected void OnConsumeMilitarySupplyAllocationServer(IEntity actionUser, int amount)
	{
		if (amount == 0)
			return;

		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return;

		PlayerManager playerManager = GetGame().GetPlayerManager();
		int playerId = playerManager.GetPlayerIdFromControlledEntity(actionUser);

		// Do not consume Military Supply Allocation if resupplying on enemy support station
		SCR_Faction playerFaction = SCR_Faction.Cast(factionManager.GetPlayerFaction(playerId));
		if (playerFaction && playerFaction != GetFaction())
			return;

		SCR_PlayerController playerController = SCR_PlayerController.Cast(playerManager.GetPlayerController(playerId));
		if (!playerController)
			return;

		SCR_PlayerSupplyAllocationComponent playerSupplyAllocationComponent = SCR_PlayerSupplyAllocationComponent.Cast(playerController.FindComponent(SCR_PlayerSupplyAllocationComponent));
		if (!playerSupplyAllocationComponent)
			return;

		playerSupplyAllocationComponent.AddPlayerAvailableAllocatedSupplies(-1 * amount);
	}

	//------------------------------------------------------------------------------------------------
	//! \return the Military Supply Allocation cost of action based on item supply cost
	//! \param[in] action
	protected int GetMilitarySupplyAllocationCostOfAction(SCR_BaseUseSupportStationAction action)
	{
		if (!SCR_ArsenalManagerComponent.IsMilitarySupplyAllocationEnabled())
			return 0;

		SCR_BaseItemHolderSupportStationAction itemHolder = SCR_BaseItemHolderSupportStationAction.Cast(action);
		if (!itemHolder)
			return 0;

		SCR_EntityCatalogEntry catalogEntry = m_EntityCatalogManager.GetEntryWithPrefabFromAnyCatalog(EEntityCatalogType.ITEM, itemHolder.GetItemPrefab(), GetFaction());
		if (!catalogEntry)
			return 0;

		SCR_ArsenalItem arsenalData = SCR_ArsenalItem.Cast(catalogEntry.GetEntityDataOfType(SCR_ArsenalItem));
		if (!arsenalData)
			return 0;

		if (arsenalData.GetUseMilitarySupplyAllocation())
			return Math.ClampInt(arsenalData.GetSupplyCost(m_eSupplyCostType) + m_iBaseSupplyCostOnUse, 0, int.MAX);

		return 0;
	}
}
