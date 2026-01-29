[BaseContainerProps(configRoot: true), BaseContainerCustomTitleField("m_sLoadoutName")]
class SCR_PlayerArsenalLoadout : SCR_FactionPlayerLoadout
{
	static const string ARSENALLOADOUT_FACTIONKEY_NONE = "none";
	static const string ARSENALLOADOUT_KEY = "arsenalLoadout";
	static const string ARSENALLOADOUT_FACTION_KEY = "faction";
	static ref array<typename> ARSENALLOADOUT_COMPONENTS_TO_CHECK;

	//------------------------------------------------------------------------------------------------
	override bool IsLoadoutAvailable(int playerId)
	{
		SCR_ArsenalManagerComponent arsenalManager;
		if (SCR_ArsenalManagerComponent.GetArsenalManager(arsenalManager))
		{
			SCR_ArsenalPlayerLoadout arsenalLoadout;
			return arsenalManager.GetPlayerArsenalLoadout(SCR_PlayerIdentityUtils.GetPlayerIdentityId(playerId), arsenalLoadout);
		}
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool IsLoadoutAvailableClient()
	{
		SCR_ArsenalManagerComponent arsenalManager;
		if (SCR_ArsenalManagerComponent.GetArsenalManager(arsenalManager))
			return arsenalManager.GetLocalPlayerLoadoutAvailable();

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] playerUID
	//! \return the cost of the provided id player's loadout or 0 on error or player not found
	static float GetLoadoutSuppliesCost(string playerUID)
	{
		SCR_ArsenalManagerComponent arsenalManager;
		if (SCR_ArsenalManagerComponent.GetArsenalManager(arsenalManager))
		{
			float supplyCostMulti = arsenalManager.GetCalculatedLoadoutSpawnSupplyCostMultiplier();

			//~ No need to get supply cost if multiplier is 0
			if (supplyCostMulti <= 0)
				return 0;

			SCR_ArsenalPlayerLoadout arsenalLoadout;
			if (arsenalManager.GetPlayerArsenalLoadout(playerUID, arsenalLoadout))
				return arsenalLoadout.suppliesCost * supplyCostMulti;
		}

		return 0;
	}

		//------------------------------------------------------------------------------------------------
	//! \param[in] playerId
	//! \return the cost of the provided id player's loadout or 0 on error or player not found
	static float GetLoadoutSuppliesCost(int playerID)
	{
		const UUID playerUID = SCR_PlayerIdentityUtils.GetPlayerIdentityId(playerID);
		return GetLoadoutSuppliesCost(playerUID);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] playerUID
	//! \return the Military Supply Allocation cost of the provided id player's loadout or 0 on error or player not found
	static float GetLoadoutMilitarySupplyAllocationCost(string playerUID)
	{
		SCR_ArsenalManagerComponent arsenalManager;
		if (SCR_ArsenalManagerComponent.GetArsenalManager(arsenalManager))
		{
			SCR_ArsenalPlayerLoadout arsenalLoadout;
			if (arsenalManager.GetPlayerArsenalLoadout(playerUID, arsenalLoadout))
				return arsenalLoadout.m_fMilitarySupplyAllocationCost;
		}

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] playerId
	//! \return required rank of player arsenal loadout
	SCR_ECharacterRank GetRequiredRank(int playerId)
	{
		SCR_ArsenalManagerComponent arsenalManager;
		if (!SCR_ArsenalManagerComponent.GetArsenalManager(arsenalManager))
			return SCR_ECharacterRank.PRIVATE;

		const UUID playerUID = SCR_PlayerIdentityUtils.GetPlayerIdentityId(playerId);
		SCR_ArsenalPlayerLoadout arsenalLoadout;
		if (!arsenalManager.GetPlayerArsenalLoadout(playerUID, arsenalLoadout))
			return SCR_ECharacterRank.PRIVATE;

		return arsenalLoadout.m_eRequiredRank;
	}

	//------------------------------------------------------------------------------------------------
	override void OnLoadoutSpawned(GenericEntity pOwner, int playerId)
	{
		super.OnLoadoutSpawned(pOwner, playerId);
		SCR_ArsenalManagerComponent arsenalManager;
		if (!SCR_ArsenalManagerComponent.GetArsenalManager(arsenalManager))
			return;

		const UUID playerUID = SCR_PlayerIdentityUtils.GetPlayerIdentityId(playerId);
		GameEntity playerEntity = GameEntity.Cast(pOwner);
		SCR_ArsenalPlayerLoadout playerArsenalItems;
		if (!playerEntity || !arsenalManager.GetPlayerArsenalLoadout(playerUID, playerArsenalItems))
			return;

		FactionAffiliationComponent factionComponent = FactionAffiliationComponent.Cast(playerEntity.FindComponent(FactionAffiliationComponent));
		if (!factionComponent)
			return;

		SCR_JsonLoadContext context = new SCR_JsonLoadContext();
		bool loadSuccess = true;
		loadSuccess &= context.ImportFromString(playerArsenalItems.loadout);
		// Read faction key and ensure same faction, otherwise delete saved arsenal loadout
		string factionKey;
		loadSuccess &= context.ReadValue(ARSENALLOADOUT_FACTION_KEY, factionKey) && factionKey != ARSENALLOADOUT_FACTIONKEY_NONE;
		loadSuccess &= factionKey == factionComponent.GetAffiliatedFaction().GetFactionKey();
		loadSuccess &= ApplyLoadoutString(playerEntity, context);

		// Deserialisation failed, delete saved arsenal loadout
		if (!loadSuccess)
			arsenalManager.SetPlayerArsenalLoadout(playerId, null, null, SCR_EArsenalSupplyCostType.RESPAWN_COST);
	}

	//------------------------------------------------------------------------------------------------
	static bool ReadLoadoutString(IEntity owner, BaseSerializationSaveContext context)
	{
		if (!context.StartObject(ARSENALLOADOUT_KEY))
			return false;

		if (!ReadEntityLoadoutString(owner, context))
			return false;

		if (!ReadCharacterDataLoadoutString(owner, context))
			return false;

		return context.EndObject();
	}

	//------------------------------------------------------------------------------------------------
	protected static bool ReadEntityLoadoutString(IEntity owner, BaseSerializationSaveContext context)
	{
		context.WriteValue("prefab", SCR_ResourceNameUtils.GetPrefabName(owner));

		if (!ReadEntityCustomDataString(owner, context))
			return false;

		return ReadEntityStorageString(owner, context);
	}

	//------------------------------------------------------------------------------------------------
	//! Add your modded logic here for custom components to store their data and read back later. Call super for multi mod compatiblity!
	protected static bool ReadEntityCustomDataString(IEntity owner, BaseSerializationSaveContext context)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	static void FindStorageComponents(IEntity owner, notnull set<BaseInventoryStorageComponent> storages)
	{
		array<Managed> outComponents = {};
		owner.FindComponents(BaseInventoryStorageComponent, outComponents);
		foreach (Managed component : outComponents)
		{
			BaseInventoryStorageComponent storage = BaseInventoryStorageComponent.Cast(component);
			storages.Insert(storage);
			FindStorageComponents(storage, storages);
		}
	}

	//------------------------------------------------------------------------------------------------
	static void FindStorageComponents(GenericComponent parentComponent, notnull set<BaseInventoryStorageComponent> storages)
	{
		array<GenericComponent> outComponents = {};
		parentComponent.FindComponents(BaseInventoryStorageComponent, outComponents);
		foreach (GenericComponent component : outComponents)
		{
			BaseInventoryStorageComponent storage = BaseInventoryStorageComponent.Cast(component);
			storages.Insert(storage);
			FindStorageComponents(storage, storages);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static bool ShouldSaveStorage(BaseInventoryStorageComponent candidate)
	{
		foreach (typename toCheck : ARSENALLOADOUT_COMPONENTS_TO_CHECK)
		{
			if (candidate.Type() == toCheck)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool ReadEntityStorageString(IEntity owner, BaseSerializationSaveContext context)
	{
		set<BaseInventoryStorageComponent> storageCandiates();
		FindStorageComponents(owner, storageCandiates);

		array<BaseInventoryStorageComponent> storages = {};
		foreach (BaseInventoryStorageComponent candidate : storageCandiates)
		{
			if (ShouldSaveStorage(candidate))
				storages.Insert(candidate);
		}

		int storageCount = storages.Count();
		if (storageCount == 0)
			return true;

		context.StartArray("storages", storageCount);
		foreach (BaseInventoryStorageComponent storage : storages)
		{
			context.StartObject();
			context.WriteValue("id", GetComponentIdentifier(owner, storage));
			const array<InventoryItemComponent> slotItems = GetSlotItems(storage);
			int slotCount = slotItems.Count();
			context.StartMap("slots", slotCount);
			foreach (InventoryItemComponent item : slotItems)
			{
				context.WriteMapKey(item.GetParentSlot().GetID().ToString());
				context.StartObject();

				if (!ReadEntityLoadoutString(item.GetOwner(), context))
					return false;

				context.EndObject();
			}
			context.EndMap();
			context.EndObject();
		}
		return context.EndArray();
	}

	//------------------------------------------------------------------------------------------------
	//! Get all slots in any relevant order from the storage. Default is simply all slots in default order.
	//! Default order is simply by slot index in ASC order
	protected static array<InventoryItemComponent> GetSlotItems(const BaseInventoryStorageComponent storage)
	{
		array<InventoryItemComponent> outItemsComponents = {};
		storage.GetOwnedItems(outItemsComponents, false);

		// For weapon attachments we need to load back slots without requirements first and those who rely on them later, regardless of slot index they are in
		// Example: Bayonet needs muzzle flash hider.
		const SCR_WeaponAttachmentsStorageComponent weaponAttachments = SCR_WeaponAttachmentsStorageComponent.Cast(storage);
		if (weaponAttachments)
		{
			array<ref SCR_SortableItem<InventoryItemComponent>> sortSlots = {};
			foreach (InventoryItemComponent item : outItemsComponents)
			{
				int numRequirements = 0;
				const SCR_WeaponAttachmentObstructionAttributes obstructionAttributes = SCR_WeaponAttachmentObstructionAttributes.Cast(item.FindAttribute(SCR_WeaponAttachmentObstructionAttributes));
				if (obstructionAttributes)
					numRequirements = obstructionAttributes.GetRequiredAttachmentTypes().Count();

				SCR_SortableItem<InventoryItemComponent> sortable(item, numRequirements);
				sortSlots.Insert(sortable);
			}
			sortSlots.Sort();
			outItemsComponents.Clear();
			foreach (SCR_SortableItem<InventoryItemComponent> sortedSlot : sortSlots)
			{
				outItemsComponents.Insert(sortedSlot.m_Item);
			}
		}

		return outItemsComponents;
	}

	//------------------------------------------------------------------------------------------------
	//! Add any custom logic that should be saved for the main character entity the loadout is for.
	//! Is written and read back after all components and storage slots are handled so it allows interacting with otherwise finished char.
	protected static bool ReadCharacterDataLoadoutString(IEntity owner, BaseSerializationSaveContext context)
	{
		// By default just the chars active weapon is stored as additional meta data
		int activeWeaponIdx = -1;
		const BaseWeaponManagerComponent weaponManager = BaseWeaponManagerComponent.Cast(owner.FindComponent(BaseWeaponManagerComponent));
		if (weaponManager)
		{
			const WeaponSlotComponent currentSlot = weaponManager.GetCurrentSlot();
			if (currentSlot)
				activeWeaponIdx = currentSlot.GetWeaponSlotIndex();
		}
		return context.WriteDefault(activeWeaponIdx, -1);
	}

	//------------------------------------------------------------------------------------------------
	protected static string GetComponentIdentifier(IEntity owner, GenericComponent component)
	{
		const BaseContainer source = component.GetComponentSource(owner);
		const string storeName = source.GetResourceName();
		return component.ClassName() + ":" + SCR_ResourceNameUtils.GetPrefabGUID(storeName);
	}

	//------------------------------------------------------------------------------------------------
	static bool ApplyLoadoutString(IEntity owner, BaseSerializationLoadContext context)
	{
		if (!context.StartObject(ARSENALLOADOUT_KEY))
			return false;

		InventoryStorageManagerComponent manager = InventoryStorageManagerComponent.Cast(owner.FindComponent(InventoryStorageManagerComponent));
		if (!manager || !ApplyEntityLoadoutString(owner, context, manager))
			return false;

		if (!ApplyCharacterDataLoadoutString(owner, context))
			return false;

		return context.EndObject();
	}

	//------------------------------------------------------------------------------------------------
	protected static bool ApplyEntityLoadoutString(
		IEntity owner,
		BaseSerializationLoadContext context,
		InventoryStorageManagerComponent manager,
		BaseInventoryStorageComponent parentStorage = null,
		int slotId = -1)
	{
		if (parentStorage && slotId != -1)
		{
			ResourceName prefab;
			if (!context.Read(prefab))
				return false;

			const ResourceName currentPrefab = SCR_ResourceNameUtils.GetPrefabName(owner);
			if (prefab != currentPrefab)
			{
				if (owner && !manager.TryDeleteItem(owner))
					return false;

				if (!manager.TrySpawnPrefabToStorage(prefab, parentStorage, slotId))
					return false;

				owner = parentStorage.Get(slotId);
				if (!owner)
					return false;
			}
		}

		if (!ApplyEntityCustomDataString(owner, context))
			return false;

		return ApplyEntityStorageString(owner, context, manager, parentStorage, slotId);
	}

	//------------------------------------------------------------------------------------------------
	// Read back any custom data per entity that was written by ReadEntityCustomDataString
	protected static bool ApplyEntityCustomDataString(IEntity owner, BaseSerializationLoadContext context)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool ApplyEntityStorageString(
		IEntity owner,
		BaseSerializationLoadContext context,
		InventoryStorageManagerComponent manager,
		BaseInventoryStorageComponent parentStorage = null,
		int slotId = -1)
	{
		int storageCount;
		context.StartArray("storages", storageCount);
		if (storageCount == 0)
			return true; // No substorages so we are done here

		set<BaseInventoryStorageComponent> storageCandiates();
		FindStorageComponents(owner, storageCandiates);

		for (int nStorage = 0; nStorage < storageCount; ++nStorage)
		{
			if (!context.StartObject())
				return false;

			string id;
			if (!context.Read(id))
				return false;

			// Find the right storage by it's id
			BaseInventoryStorageComponent storage;
			foreach (BaseInventoryStorageComponent candidate : storageCandiates)
			{
				const string componentId = GetComponentIdentifier(owner, candidate);
				if (componentId == id)
				{
					storage = candidate;
					break;
				}
			}
			if (!storage)
				return false;

			// Quick access to all currently stored items to apply data and remove those who are not part of loadodut at the end
			map<int, IEntity> slottetItems = new map<int, IEntity>();
			array<InventoryItemComponent> outItemsComponents = {};
			storage.GetOwnedItems(outItemsComponents, false);
			foreach (InventoryItemComponent item : outItemsComponents)
			{
				slottetItems.Insert(item.GetParentSlot().GetID(), item.GetOwner());
			}

			int slotCount = 0;
			if (!context.StartMap("slots", slotCount))
				return false;

			for (int i = 0; i < slotCount; ++i)
			{
				// Read back the order they were written in.
				// NOTE: This may not be in numerical ascending order but logic for e.g. attachment dependency
				string idxStr;
				if (!context.ReadMapKey(i, idxStr))
					return false;

				const int childSlotId = idxStr.ToInt(-1);
				if (childSlotId == -1)
					return false;

				IEntity existing;
				slottetItems.Take(childSlotId, existing);

				if (!context.StartObject(idxStr))
					return false;

				if (!ApplyEntityLoadoutString(existing, context, manager, storage, childSlotId))
					return false;

				if (!context.EndObject())
					return false;
			}
			if (!context.EndMap())
				return false;

			// Remove any existing entity whos slot was not mentioned in the loadout data.
			foreach (int idx, IEntity entity : slottetItems)
			{
				if (entity && !manager.TryDeleteItem(entity))
					return false;
			}

			if (!context.EndObject())
				return false;
		}

		return context.EndArray();
	}

	//------------------------------------------------------------------------------------------------
	protected static bool ApplyCharacterDataLoadoutString(IEntity owner, BaseSerializationLoadContext context)
	{
		int activeWeaponIdx;
		context.ReadDefault(activeWeaponIdx, -1);
		const BaseWeaponManagerComponent weaponManager = BaseWeaponManagerComponent.Cast(owner.FindComponent(BaseWeaponManagerComponent));
		const CharacterControllerComponent charController = CharacterControllerComponent.Cast(owner.FindComponent(CharacterControllerComponent));
		if (!weaponManager || !charController) // If neither is present we have some special char who apparently does not need the data
			return true;

		if (activeWeaponIdx != -1)
		{
			array<WeaponSlotComponent> outSlots = {};
			if (weaponManager.GetWeaponsSlots(outSlots))
			{
				WeaponSlotComponent desiredSlot, fallbackSlot;
				foreach (WeaponSlotComponent slot : outSlots)
				{
					const int slotIdx = slot.GetWeaponSlotIndex();
					if (slotIdx == activeWeaponIdx)
					{
						desiredSlot = slot;
						break;
					}

					if (!fallbackSlot && slotIdx < 3 && slot.GetWeaponEntity())
						fallbackSlot = slot;
				}

				IEntity weaponEntity;
				if (desiredSlot)
				{
					weaponEntity = desiredSlot.GetWeaponEntity();
				}
				else if (fallbackSlot)
				{
					weaponEntity = fallbackSlot.GetWeaponEntity();
				}

				charController.TryEquipRightHandItem(weaponEntity, EEquipItemType.EEquipTypeWeapon, true);
			}
		}
		else
		{
			charController.TryEquipRightHandItem(null, EEquipItemType.EEquipTypeUnarmedDeliberate, true);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	static void ComputeSuppliesCost(
		BaseSerializationLoadContext context,
		SCR_Faction faction,
		SCR_ArsenalPlayerLoadout playerLoadout,
		SCR_EArsenalSupplyCostType arsenalSupplyType,
		int depth = 0)
	{
		ResourceName prefab;
		if (!context.Read(prefab))
			return;

		if (depth > 0)
		{
			SCR_EntityCatalogManagerComponent entityCatalogManager = SCR_EntityCatalogManagerComponent.GetInstance();
			if (entityCatalogManager)
			{
				SCR_EntityCatalogEntry entry = entityCatalogManager.GetEntryWithPrefabFromGeneralOrFactionCatalog(EEntityCatalogType.ITEM, prefab, faction);
				if (entry)
				{
					SCR_ArsenalItem data = SCR_ArsenalItem.Cast(entry.GetEntityDataOfType(SCR_ArsenalItem));
					if (data)
					{
						const float cost = data.GetSupplyCost(arsenalSupplyType);
						playerLoadout.suppliesCost += cost;
						playerLoadout.m_eRequiredRank = Math.Max(playerLoadout.m_eRequiredRank, data.GetRequiredRank());
						if (data.GetUseMilitarySupplyAllocation())
							playerLoadout.m_fMilitarySupplyAllocationCost += cost;
					}
				}
			}
		}

		int storageCount;
		context.StartArray("storages", storageCount);
		if (storageCount == 0)
			return; // No substorages so we are done here

		for (int nStorage = 0; nStorage < storageCount; ++nStorage)
		{
			if (!context.StartObject())
				return;

			string id;
			if (!context.Read(id))
				return;

			int slotCount = 0;
			context.StartMap("slots", slotCount);
			for (int i = 0; i < slotCount; ++i)
			{
				string idxStr;
				if (!context.ReadMapKey(i, idxStr))
					return;

				const int childSlotId = idxStr.ToInt(-1);
				if (childSlotId == -1)
					return;

				context.StartObject(idxStr);
				ComputeSuppliesCost(context, faction, playerLoadout, arsenalSupplyType, depth + 1);
				context.EndObject();
			}
			context.EndMap();
			context.EndObject();
		}
		context.EndArray();
	}
}
