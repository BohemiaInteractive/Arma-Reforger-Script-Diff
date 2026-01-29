enum EItemType
{
	IT_NONE = 16385
}

[ComponentEditorProps(category: "GameScripted/Inventory", description: "Inventory 2.0", icon: HYBRID_COMPONENT_ICON)]
class SCR_CharacterInventoryStorageComponentClass: CharacterInventoryStorageComponentClass
{
}

class SCR_InvEquipCB : SCR_InvCallBack
{
	CharacterControllerComponent m_Controller;

	//------------------------------------------------------------------------------------------------
	protected override void OnComplete()
	{
		m_Controller.TryEquipRightHandItem(m_pItem, EEquipItemType.EEquipTypeWeapon, false);
		m_pItem = null;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnFailed()
	{
		m_pItem = null;
	}
}

class SCR_EquipNextGrenadeCB : SCR_InvCallBack
{
	SCR_InventoryStorageManagerComponent m_InvMan;

	//------------------------------------------------------------------------------------------------
	protected override void OnComplete()
	{
		if (m_pItem)
			m_InvMan.EquipWeapon(m_pItem, null, false);
		
		m_pItem = null;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnFailed()
	{
		m_pItem = null;
	}
}

class SCR_EquipGearCB : SCR_InvCallBack
{
	protected override void OnComplete()
	{
		if (m_pMenu)
		{
			m_pMenu.ShowStoragesList();
			m_pMenu.ShowAllStoragesInList();
		}
	}
}

class SCR_InvEquipAnyItemCB : SCR_InvCallBack
{
	SCR_CharacterInventoryStorageComponent m_CharacterStorage;

	//------------------------------------------------------------------------------------------------
	protected override void OnComplete()
	{
		SCR_CharacterInventoryStorageComponent characterStorage = SCR_CharacterInventoryStorageComponent.Cast(m_pStorageToPickUp);
		if (!characterStorage || !m_pItem)
			return;

		if (m_iSlotToFocus > -1)
		{
			characterStorage.RemoveItemFromQuickSlotAtIndex(m_iSlotToFocus);
			characterStorage.StoreItemToQuickSlot(m_pItem, m_iSlotToFocus);
			SCR_WeaponSwitchingBaseUI.HighlightQuickSlot(m_iSlotToFocus);
		}

		if (m_bShouldEquip)
			// This must be delayed because inventory... Otherwise its possible that item will be permanently system locked, as host might still not know that item transfer was finished
			GetGame().GetCallqueue().CallLater(characterStorage.UseItem, 250, false, m_pItem, ESlotFunction.TYPE_GENERIC, SCR_EUseContext.FROM_INVENTORY);

		super.OnComplete();
		m_pItem = null;
		m_pStorageToPickUp = null;
	}
}

class SCR_CharacterInventoryStorageComponent : CharacterInventoryStorageComponent
{
	//TODO: define this on loadout level. This is temporary and will be removed!
	[Attribute( "0", UIWidgets.EditBox, "How much weight the character can carry")]
	protected float m_fMaxWeight;
	
	//TODO: define this on loadout level. This is temporary and will be removed!
	[Attribute( "0", UIWidgets.EditBox, "How much volume the character can carry")]
	protected float m_fMaxVolume;
	
	#ifndef DISABLE_INVENTORY
	
	private BaseInventoryStorageComponent				m_LootStorage;
	protected ref array<ref SCR_QuickslotBaseContainer>	m_aQuickSlots = { null, null, null, null, null, null, null, null, null, null };
	protected ref map<IEntity, int> 					m_mSlotHistory = new map<IEntity, int>();
	protected ref array<IEntity>						m_aWeaponQuickSlotsStorage = {}; //Is used to store first four quickslots of turrets.
	protected ref array<int>							m_aQuickSlotsHistory = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};	//here we'll be remembering the items stored
	protected ref set<int>								m_aQuickSlotsInitalized = new set<int>();	
//	protected ref array<EntityComponentPrefabData>		m_aPrefabsData = { null, null, null, null, null, null, null, null, null, null }; // todo: figure out the intentions
	protected static const int							GADGET_OFFSET = 9999;	//EWeaponType && EGadgetType might be have the same number, offset it ( not nice (i agree) )
	protected static const int							TURRET_WEAPON_SWITCH_SLOTS = 4; //How many slots can player cycle between using SelectNextWeapon method by default when in turret
	protected static const int							INFANTRY_WEAPON_SWITCH_SLOTS = 2; //How many slots can player cycle between using SelectNextWeapon method by default when not in turret

//	protected ref array<ref array<int>>					m_aDefaultRiflemanQuickSlots = 	{	{ EWeaponType.WT_RIFLE, EWeaponType.WT_SNIPERRIFLE, EWeaponType.WT_MACHINEGUN },
//																							{ EWeaponType.WT_HANDGUN, EWeaponType.WT_ROCKETLAUNCHER, EWeaponType.WT_GRENADELAUNCHER, ( EGadgetType.BINOCULARS ) + GADGET_OFFSET  },
//																							{ ( EGadgetType.FLASHLIGHT ) + GADGET_OFFSET },
//																							{ EWeaponType.WT_FRAGGRENADE, EWeaponType.WT_SMOKEGRENADE },
//																							{ ( EGadgetType.MAP ) + GADGET_OFFSET },
//																							{ ( EGadgetType.COMPASS ) + GADGET_OFFSET }
//																						};

	
	//---- REFACTOR NOTE START: Not the best way to do this, should be more configurable by players ----
	
	protected static const ref array<ref array<int>>	DEFAULT_QUICK_SLOTS =			{	{ EWeaponType.WT_RIFLE, EWeaponType.WT_SNIPERRIFLE, EWeaponType.WT_MACHINEGUN },
																							{ EWeaponType.WT_RIFLE, EWeaponType.WT_ROCKETLAUNCHER, EWeaponType.WT_GRENADELAUNCHER, EWeaponType.WT_SNIPERRIFLE, EWeaponType.WT_MACHINEGUN },
																							{ EWeaponType.WT_HANDGUN },
																							{ EWeaponType.WT_FRAGGRENADE },
																							{ EWeaponType.WT_SMOKEGRENADE },
																							{ EGadgetType.CONSUMABLE + GADGET_OFFSET + SCR_EConsumableType.BANDAGE },
																							{ EGadgetType.CONSUMABLE + GADGET_OFFSET + SCR_EConsumableType.TOURNIQUET },
																							{ EGadgetType.CONSUMABLE + GADGET_OFFSET + SCR_EConsumableType.MORPHINE, EGadgetType.CONSUMABLE + GADGET_OFFSET + SCR_EConsumableType.SALINE },
																							{ EGadgetType.RADIO + GADGET_OFFSET }, // Preferably as GadgetRadio action, then it can be saline
																							{ EGadgetType.BUILDING_TOOL + GADGET_OFFSET } // To be replaced with engineering tool
																						};

	//---- REFACTOR NOTE END ----
	
	protected ref array<typename> m_aBlockedSlots = {};
	protected ref array<BaseInventoryStorageComponent> m_aStoragesInStorageList = {};		//here we remember the opened storages in the Inventory menu ( in the Storages list area )
	protected SCR_CompartmentAccessComponent m_CompartmentAccessComp;
	protected BaseInventoryStorageComponent m_WeaponStorage;
	protected SCR_CharacterFactionAffiliationComponent m_CharacterAffiliationComponent;
	
	protected ref SCR_InvEquipCB m_Callback = new SCR_InvEquipCB();

	protected static const ref array<EWeaponType> WEAPON_TYPES_THROWABLE = { EWeaponType.WT_FRAGGRENADE, EWeaponType.WT_SMOKEGRENADE };
	
	//------------------------------------------------------------------------ USER METHODS ------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! \return
	//TODO: define this on loadout level. This is temporary and will be removed!
	float GetMaxLoad()
	{
		return m_fMaxWeight;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	BaseInventoryStorageComponent GetWeaponStorage()
	{
		if (!m_WeaponStorage)
			m_WeaponStorage = BaseInventoryStorageComponent.Cast(GetOwner().FindComponent(EquipedWeaponStorageComponent));
		
		return m_WeaponStorage;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] eSlot
	//! \return
	InventoryItemComponent GetItemFromLoadoutSlot( LoadoutAreaType eSlot )
	{
		// Not all attached Entities to slots are storages.
		// For instance boots - even though attached to character storage slot by themselves represent item, not storage
		// However if entity attached to slot it is guaranteed to have InventoryItemComponent
		InventoryStorageSlot slot = GetSlotFromArea(eSlot.Type());
		if (!slot)
			return null;
		
		IEntity entity = slot.GetAttachedEntity();
		if (!entity)
			return null;
		
		return InventoryItemComponent.Cast(entity.FindComponent(InventoryItemComponent));
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] eSlot
	//! \return
	BaseInventoryStorageComponent GetStorageFromLoadoutSlot( LoadoutAreaType eSlot )
	{
		return BaseInventoryStorageComponent.Cast( GetItemFromLoadoutSlot(eSlot) );
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool HasStorageComponent( IEntity pEntity )
	{
		return GetStorageComponentFromEntity( pEntity ) != null;	
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[out] storagesInInventory
	//! \return all topmost storages
	void GetStorages( out notnull array<SCR_UniversalInventoryStorageComponent> storagesInInventory )
	{
		array<IEntity> pEntities = {};
		int iNrOfStorages = GetAll( pEntities );
		
		SCR_UniversalInventoryStorageComponent pUniComp;
		foreach ( IEntity pEntity: pEntities )
		{
			pUniComp = GetStorageComponentFromEntity(pEntity);
			if( pUniComp )
				storagesInInventory.Insert( pUniComp );
		}
	}
		
	//------------------------------------------------------------------------------------------------
	//! \param[out] blockedSlots
	void GetBlockedSlots(out notnull array<typename> blockedSlots)
	{
		blockedSlots.Copy(m_aBlockedSlots);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] pEntity
	//! \return the item inventory component
	SCR_UniversalInventoryStorageComponent GetStorageComponentFromEntity( IEntity pEntity )
	{
		if ( pEntity == null )
			return null;
		
		return SCR_UniversalInventoryStorageComponent.Cast(pEntity.FindComponent( SCR_UniversalInventoryStorageComponent ));	
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] pOwner
	void SetLootStorage( IEntity pOwner )
	{
		if( !pOwner )
		{
			m_LootStorage = null;
			return;
		}

		m_LootStorage = BaseInventoryStorageComponent.Cast(pOwner.FindComponent(BaseInventoryStorageComponent));
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	BaseInventoryStorageComponent GetLootStorage()
	{
		return m_LootStorage;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] pStorage
	//! \return the visibility state of the adequate UI container - if the storage was previously shown in the inventory
	bool GetIsStorageShown( notnull BaseInventoryStorageComponent pStorage )
	{
		return m_aStoragesInStorageList.Find( pStorage ) != -1;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] pStorage
	void SetStorageAsShown( notnull BaseInventoryStorageComponent pStorage )
	{
		if ( !GetIsStorageShown( pStorage ) )
			m_aStoragesInStorageList.Insert( pStorage );
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] pStorage
	void SetStorageAsHidden( notnull BaseInventoryStorageComponent pStorage )
	{
		m_aStoragesInStorageList.RemoveItem( pStorage );
	}
	
	//---- REFACTOR NOTE START: Ideally should be linked to components and rather should work based on item config instead ----
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] pItem
	//! \return
	static int GetItemType(notnull IEntity pItem)
	{
		int iItemType = -1;
		//Weapons:
		BaseWeaponComponent weaponComponent = BaseWeaponComponent.Cast( pItem.FindComponent( BaseWeaponComponent ) );
		if ( weaponComponent )
		{
			iItemType = weaponComponent.GetWeaponType();
		}
		else
		{
			//Gadgets:
			SCR_GadgetComponent gadgetComponent = SCR_GadgetComponent.Cast( pItem.FindComponent( SCR_GadgetComponent ) );
			if ( gadgetComponent )
			{
				EGadgetType gadgetType = gadgetComponent.GetType();
				int consumableOffset = 0;
				if (gadgetType == EGadgetType.CONSUMABLE)
				{
					SCR_ConsumableItemComponent consumable = SCR_ConsumableItemComponent.Cast(gadgetComponent);
					if (consumable)
						consumableOffset = consumable.GetConsumableType();
				}
				iItemType = gadgetComponent.GetType() + GADGET_OFFSET + consumableOffset;
			}
		}
				
		return iItemType;
	}
	
	//---- REFACTOR NOTE END ----
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] iItemType
	//! \param[in] iSlotIndex
	//! \return
	bool ItemBelongsToSlot( int iItemType, int iSlotIndex )
	{
		return DEFAULT_QUICK_SLOTS[iSlotIndex].Contains(iItemType);
	}

	//------------------------------------------------------------------------------------------------
	bool SetQuickSlotInitialized(int iSlotIndex)
	{
		return m_aQuickSlotsInitalized.Insert(iSlotIndex);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] ent
	//! \return
	int GetLastQuickSlotId(IEntity ent)
	{
		int result = m_mSlotHistory.Get(ent);
		m_mSlotHistory.Remove(ent);
		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] item
	//! \return
	static int GetDefaultQuickSlot(notnull IEntity item)
	{
		int itemType = GetItemType(item);
		foreach (int i, ref array<int> allowedTypes : DEFAULT_QUICK_SLOTS)
		{
			if (allowedTypes.Contains(itemType))
				return i;
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] item
	//! \return
	bool IsInDefaultQuickSlot(notnull IEntity item)
	{
		int itemType = GetItemType(item);
		if (itemType < 0)
			return false;

		int slotId = GetDefaultQuickSlot(item);
		if (slotId < 0)
			return false;

		return GetEntityIndexInQuickslots(item) == slotId;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] pItem
	//! \param[in] iSlotIndex
	//! \param[in] isForced
	//! \return
	int StoreItemToQuickSlot(notnull IEntity pItem, int iSlotIndex = -1, bool isForced = false)
	{
		int iItemType = GetItemType( pItem );
		if ( iSlotIndex == -1 ) //we don't know what slot we put the item into. Check first if we remember the type of the item
		{	
			InventoryItemComponent itemComp = InventoryItemComponent.Cast(pItem.FindComponent(InventoryItemComponent));
			InventoryStorageSlot parentSlot;
			if (itemComp)
				parentSlot = itemComp.GetParentSlot();
			
			if (parentSlot && EquipedWeaponStorageComponent.Cast(parentSlot.GetStorage()))
			{
				iSlotIndex = parentSlot.GetID();
			}
			else
			{
				foreach (int i, SCR_QuickslotBaseContainer quickSlot : m_aQuickSlots)
				{
					if (quickSlot)
						continue;//do it only for the empty slots

					if (m_aQuickSlotsHistory.IsIndexValid(i) && iItemType == m_aQuickSlotsHistory[i])
					{
						iSlotIndex = i;
						break;
					}

					if (ItemBelongsToSlot(iItemType, i))
					{
						iSlotIndex = i;
						break;
					}
				}
			}
		}
				
		if ( iSlotIndex == -1 )	//any suitable slot not found, do not insert into quick slot
			return -1;
		
		if (!isForced)
		{
			TurretCompartmentSlot turretCompartment = TurretCompartmentSlot.Cast(GetCurrentCompartment());
			if (turretCompartment)
			{
				if (iSlotIndex < SCR_InventoryMenuUI.WEAPON_SLOTS_COUNT)
					return -1;
				
				array<IEntity> turretWeapons = {};
				GetTurretWeaponsList(turretCompartment, turretWeapons);
				if (turretWeapons.Contains(pItem))
					return -1;
			}
		}
		
		SCR_QuickslotEntityContainer entityContainer = SCR_QuickslotEntityContainer.Cast(m_aQuickSlots[iSlotIndex]);
		if ( entityContainer && pItem == entityContainer.GetEntity() )
			return iSlotIndex;	
		
		int iOldIndex = RemoveItemFromQuickSlot( pItem );
		if ( 0 <= iOldIndex && iOldIndex < m_aQuickSlotsHistory.Count() )
			m_aQuickSlotsHistory[ iOldIndex ] = 0;	//in case the item is already in slot and we shift the item into a different slot
		
		SCR_QuickslotEntityContainer inventoryContainer = new SCR_QuickslotEntityContainer(pItem);
		InsertContainerIntoQuickslot(inventoryContainer, iSlotIndex);

		m_aQuickSlotsHistory[ iSlotIndex ] = iItemType; // remember it
		return iSlotIndex;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void InsertContainerIntoQuickslot(SCR_QuickslotBaseContainer container, int slotIndex)
	{
		//clear the array in case something reacts to it
		m_aQuickSlots.Set(slotIndex, null);
		m_aQuickSlots.Set(slotIndex, container);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] pItem
	//! \return
	int RemoveItemFromQuickSlot( IEntity pItem )
	{
		int index = GetEntityIndexInQuickslots(pItem);
	
		RemoveItemFromQuickSlotAtIndex(index);
		
		return index;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] index
	void RemoveItemFromQuickSlotAtIndex(int index)
	{
		if (m_aQuickSlots.IsIndexValid(index))
			m_aQuickSlots.Set(index, null);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	array<ref SCR_QuickslotBaseContainer> GetQuickSlotItems()
	{ 
		return m_aQuickSlots; 
	}
	
	//------------------------------------------------------------------------------------------------
	//! gets all entities in quickslots, and nulls in arrays for non-entity or empty slots
	array<IEntity> GetQuickSlotEntitiesOnly()
	{
		array<IEntity> entities = {};
		
		SCR_QuickslotEntityContainer entityContainer;
		foreach(SCR_QuickslotBaseContainer container : m_aQuickSlots)
		{
			entityContainer = SCR_QuickslotEntityContainer.Cast(container);
			if (!entityContainer)
			{
				entities.Insert(null);
				continue;
			}
			
			entities.Insert(entityContainer.GetEntity());
		}
		
		return entities;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] index
	//! \return
	IEntity GetItemFromQuickSlot(int index)
	{
		if (!m_aQuickSlots.IsIndexValid(index))
			return null;

		SCR_QuickslotEntityContainer entityContainer = SCR_QuickslotEntityContainer.Cast(m_aQuickSlots[index]);
		if (!entityContainer)
			return null;
		
		return entityContainer.GetEntity();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] index
	//! \return
	SCR_QuickslotBaseContainer GetContainerFromQuickslot(int index)
	{
		if (!m_aQuickSlots.IsIndexValid(index))
			return null;

		return m_aQuickSlots[index];
	}

	//------------------------------------------------------------------------------------------------
	//! \return currently held item. If character holds gadget, gadget is returned, otherwise current weapon.
	IEntity GetCurrentItem()
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(GetOwner());
		if (!character)
			return null;
		
		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return null;
		
		IEntity gadget = controller.GetAttachedGadgetAtLeftHandSlot();
		if (gadget)
			return gadget;
		
		BaseWeaponComponent weapon = GetCurrentWeapon();
		if (weapon)
			return weapon.GetOwner();
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return selected item. If there is no active switching, currently held item is returned instead.
	IEntity GetSelectedItem()
	{
		// Selection is in progress
		if (m_Callback.m_pItem)
			return m_Callback.m_pItem;
	
		return GetCurrentItem();	
	}
	
	//------------------------------------------------------------------------------------------------
	//! Unequip currently held item. Not allowed while switching to another item.
	void UnequipCurrentItem()
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(GetOwner());
		if (!character)
			return;
		
		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return;
		
		if (controller.IsChangingItem())
			return;
		
		if (controller.IsGadgetInHands())
			controller.RemoveGadgetFromHand();
		else
			controller.SelectWeapon(null);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Drop currently held item. Not allowed while switching to another item.
	void DropCurrentItem()
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(GetOwner());
		if (!character)
			return;
		
		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return;
		
		if (controller.IsChangingItem())
			return;
		
		SCR_InventoryStorageManagerComponent storageManager = SCR_InventoryStorageManagerComponent.Cast(controller.GetInventoryStorageManager());
		if (!storageManager)
			return;
		
		IEntity itemEnt;
		
		// TODO: Also drop hidden sticky gadget
		if (controller.IsGadgetInHands())
		{
			// TODO: Equip another gadget or consumable of same type
			itemEnt = controller.GetAttachedGadgetAtLeftHandSlot();
			storageManager.TryRemoveItemFromInventory(itemEnt);
			return;
		}
		
		BaseWeaponManagerComponent weaponManager = controller.GetWeaponManagerComponent();
		if (!weaponManager)
			return;
		
		WeaponSlotComponent currentSlot = weaponManager.GetCurrentSlot();
		if (!currentSlot)
			return;
		
		SCR_EquipNextGrenadeCB callback = new SCR_EquipNextGrenadeCB();
		
		EWeaponType type = currentSlot.GetWeaponType();
		itemEnt = currentSlot.GetWeaponEntity();
		
		if (!storageManager.CanMoveItem(itemEnt))
			return;
		
		callback.m_InvMan = storageManager;		
		
		if (WEAPON_TYPES_THROWABLE.Contains(type))
			callback.m_pItem = storageManager.FindNextWeaponOfType(type, itemEnt, true);
		
		storageManager.SetInventoryLocked(true);
		controller.DropWeapon(currentSlot);
		storageManager.SetInventoryLocked(false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called when item is added to slot, update armored attributes when item with armorData is equipped
	//! \param[in] item
	//! \param[in] slotID
	protected override void OnAddedToSlot(IEntity item, int slotID)
	{
		super.OnAddedToSlot(item, slotID);
		
		UpdateBlockedSlots(item, slotID, true);
		
		EditArmoredAttributes(item, slotID);
		
		//~ Update perceived faction
		UpdatePerceivedFaction(true, item);

		#ifdef DEBUG_INVENTORY20
		// Loadout manager is taking care of this since there are some items that shouldn't be visible when attached to slot, some have different meshes for different states.
		// Consider glasses in first person view - they deffinitely should be disabled
		// it is slightly more complex then this
			
		InventoryItemComponent itemComponent = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
		if (!itemComponent) 
			return;
		
		SCR_UniversalInventoryStorageComponent storageComponent = GetStorageComponentFromEntity(item);
		if (!storageComponent)
			return;
		
		SCR_ItemAttributeCollection attr = SCR_ItemAttributeCollection.Cast(storageComponent.GetAttributes());
		if( !attr )
			return;
		
		UIInfo UIinfoItem = attr.GetUIInfo();
		if( !UIinfoItem )
			return;
		
		Print(string.Format("INV: item %1 was added. It's weight is: %2, and total weight of item/storage is: %3", UIinfoItem.GetName(), attr.GetWeight(), storageComponent.GetTotalWeight()), LogLevel.NORMAL);
		#endif
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnRemovedFromSlot(IEntity item, int slotID)
	{
		super.OnRemovedFromSlot(item, slotID);
			
		UpdateBlockedSlots(item, slotID, false);
		
		EditArmoredAttributes(item, slotID, true);
		
		//~ Update perceived faction
		UpdatePerceivedFaction(false, item);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateBlockedSlots(IEntity item, int slotID, bool added)
	{
		BaseLoadoutClothComponent loadoutComp = BaseLoadoutClothComponent.Cast(item.FindComponent(BaseLoadoutClothComponent));
		if (!loadoutComp)
			return;
		
		array<typename> blockedSlots = {};
		
		loadoutComp.GetBlockedSlots(blockedSlots);
		
		if (blockedSlots.IsEmpty())
			return;
		
		if (added)
		{
			foreach (typename blockedSlot: blockedSlots)
			{
				m_aBlockedSlots.Insert(blockedSlot);
			}
		}
		else
		{
			foreach (typename blockedSlot: blockedSlots)
			{
				m_aBlockedSlots.RemoveItem(blockedSlot);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] areaType
	//! \return
	bool IsAreaBlocked(typename areaType)
	{
		return m_aBlockedSlots.Contains(areaType);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Take the data from the armor attribute, and store them in map on damagemanager
	protected void EditArmoredAttributes(IEntity item, int slotID, bool remove = false)
	{
		InventoryItemComponent itemComponent = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
		if (!itemComponent) 
			return;
		
		SCR_ItemAttributeCollection attributes = SCR_ItemAttributeCollection.Cast(itemComponent.GetAttributes());
		if (!attributes)
			return;
		
		SCR_ArmoredClothItemData armorAttr = SCR_ArmoredClothItemData.Cast(attributes.FindAttribute(SCR_ArmoredClothItemData));
		SCR_CharacterDamageManagerComponent damageMgr = SCR_CharacterDamageManagerComponent.Cast(GetOwner().FindComponent(SCR_CharacterDamageManagerComponent));
		if (armorAttr && damageMgr)
			damageMgr.UpdateArmorDataMap(armorAttr, remove);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] item
	//! \param[in] storageOwner
	void HandleOnItemAddedToInventory( IEntity item, BaseInventoryStorageComponent storageOwner )
	{
		int targetQuickSlot = StoreItemToQuickSlot(item);
		if (targetQuickSlot > -1 && SCR_WeaponSwitchingBaseUI.s_bOpened)
			SCR_WeaponSwitchingBaseUI.RefreshQuickSlots(targetQuickSlot);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] item
	//! \param[in] storageOwner
	void HandleOnItemRemovedFromInventory( IEntity item, BaseInventoryStorageComponent storageOwner )
	{
		int itemIndex = GetEntityIndexInQuickslots(item);
		m_mSlotHistory.Set(item, itemIndex);
		RemoveItemFromQuickSlot( item );
	}
	
	//------------------------------------------------------------------------------------------------
	//! Checks if provided item can be equiped while in the inventory menu
	//! \return true if this item can be equiped, otherwise false
	bool CanEquipItem_Inventory(notnull IEntity item)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(GetOwner());
		if (!character)
			return false;

		CharacterControllerComponent charCtrl = character.GetCharacterController();
		if (!charCtrl)
			return false;

		if (charCtrl.GetCurrentItemInHands() == item || charCtrl.GetInputContext().GetLeftHandGadgetEntity() == item)
			return false;

		GenericComponent component = GenericComponent.Cast(item.FindComponent(SCR_GadgetComponent));
		if (SCR_GadgetComponent.Cast(component))
			return CanUseItem(item, ESlotFunction.TYPE_GADGET);

		component = GenericComponent.Cast(item.FindComponent(MagazineComponent));
		if (MagazineComponent.Cast(component))
			return false;

		component = GenericComponent.Cast(item.FindComponent(BaseLoadoutClothComponent));
		if (BaseLoadoutClothComponent.Cast(component))
		{
			BaseLoadoutClothComponent clothComp = BaseLoadoutClothComponent.Cast(component);
			LoadoutAreaType loadoutArea = clothComp.GetAreaType();
			return loadoutArea && item != GetClothFromArea(loadoutArea.Type());//prevent reequipping of an item
		}

		component = GenericComponent.Cast(item.FindComponent(BaseWeaponComponent));
		if (component)
		{
			if (charCtrl.IsChangingItem())
				return false;

			BaseWeaponComponent weapon = BaseWeaponComponent.Cast(component);
			return weapon && weapon.CanBeEquipped(charCtrl) == ECanBeEquippedResult.OK;//f.e. mines cannot be equipped while prone
		}

		return CanUseItem(item);
	}

	//---- REFACTOR NOTE START: Ideally should be linked to components and rather should work based on item config instead ----
	
	// For use from inventory menu	
	bool CanUseItem_Inventory(notnull IEntity item, ESlotFunction slotFunction = ESlotFunction.TYPE_GENERIC)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(GetOwner());
		if (!character)
			return false;

		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return false;

		if (slotFunction == ESlotFunction.TYPE_GENERIC)
		{
			if (item.FindComponent(BaseLoadoutClothComponent))
				slotFunction = ESlotFunction.TYPE_CLOTHES;
			else if (item.FindComponent(SCR_GadgetComponent))
				slotFunction = ESlotFunction.TYPE_GADGET;
			else if (item.FindComponent(MagazineComponent))
				slotFunction = ESlotFunction.TYPE_MAGAZINE;
			else if (item.FindComponent(BaseWeaponComponent))
				slotFunction = ESlotFunction.TYPE_WEAPON;
		}

		switch (slotFunction)
		{
			case ESlotFunction.TYPE_CLOTHES:
			{
				return false;
			} break;

			case ESlotFunction.TYPE_MAGAZINE:
			{
				if (!character.IsInVehicle())
					return CanReloadCurrentWeapon(item);

				return false;
			} break;

			case ESlotFunction.TYPE_WEAPON:
			{
				return false;
			} break;

			case ESlotFunction.TYPE_GADGET:
			{
				SCR_ConsumableItemComponent consumableComp = SCR_ConsumableItemComponent.Cast(item.FindComponent(SCR_ConsumableItemComponent));
				if (consumableComp)
				{
					return (consumableComp 
						&& consumableComp.GetConsumableEffect() 
						&& consumableComp.GetConsumableEffect().CanApplyEffect(character, character));
				}
				else
				{
					SCR_GadgetComponent gadgetComp = SCR_GadgetComponent.Cast(item.FindComponent(SCR_GadgetComponent));
					return (gadgetComp.GetMode() == EGadgetMode.IN_HAND && gadgetComp.GetUseMask() & SCR_EUseContext.FROM_INVENTORY);
				}
			} break;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------	
	//!
	//! \param[in] item
	//! \param[in] slotFunction
	//! \return
	bool CanUseItem(notnull IEntity item, ESlotFunction slotFunction = ESlotFunction.TYPE_GENERIC)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(GetOwner());
		if (!character)
			return false;
		
		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return false;
		
		// Autodetect slot function if not provided
		if (slotFunction == ESlotFunction.TYPE_GENERIC)
		{
			if (item.FindComponent(SCR_GadgetComponent))
				slotFunction = ESlotFunction.TYPE_GADGET;
			else if (item.FindComponent(MagazineComponent))
				slotFunction = ESlotFunction.TYPE_MAGAZINE;
			else if (item.FindComponent(BaseWeaponComponent))
				slotFunction = ESlotFunction.TYPE_WEAPON;
		}
		
		switch (slotFunction)
		{
			case ESlotFunction.TYPE_MAGAZINE:
			{
				if (!character.IsInVehicle())
					return CanReloadCurrentWeapon(item);
				
				return false;
			}
			
			case ESlotFunction.TYPE_WEAPON:
			{
				if (TurretCompartmentSlot.Cast(GetCurrentCompartment()))
				{
					BaseWeaponComponent currentWeapon = GetCurrentTurretWeapon();
					return currentWeapon && currentWeapon.GetOwner(); // TODO: != item
				}
				else
				{
					return controller.GetCanFireWeapon(); // TODO: Has multiple muzzles or has next grenade type
				}
				
				return false;
			}
			
			case ESlotFunction.TYPE_GADGET:
			{
				return controller.CanEquipGadget(item);
			}
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------	
	//!
	//! \param[in] item
	//! \param[in] slotFunction
	//! \return
	bool UseItem(notnull IEntity item, ESlotFunction slotFunction = ESlotFunction.TYPE_GENERIC, SCR_EUseContext context = SCR_EUseContext.FROM_QUICKSLOT)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(GetOwner());
		if (!character)
			return false;
		
		// Autodetect slot function
		if (slotFunction == ESlotFunction.TYPE_GENERIC)
		{
			if (item.FindComponent(SCR_GadgetComponent))
				slotFunction = ESlotFunction.TYPE_GADGET;
			else if (item.FindComponent(BaseLoadoutClothComponent))
				slotFunction = ESlotFunction.TYPE_CLOTHES;
			else if (item.FindComponent(MagazineComponent))
				slotFunction = ESlotFunction.TYPE_MAGAZINE;
			else if (item.FindComponent(BaseWeaponComponent))
				slotFunction = ESlotFunction.TYPE_WEAPON;
		}
		
		SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INV_VICINITY_EQUIP_CLICK);
		
		switch (slotFunction)
		{
			case ESlotFunction.TYPE_MAGAZINE:
			{
				RemoveItemFromQuickSlot(item);

				SCR_MagazinePredicate predicate = new SCR_MagazinePredicate();
				predicate.magWellType = MagazineComponent.Cast(item.FindComponent(MagazineComponent)).GetMagazineWell().Type();
				array<IEntity> magazines = {};

				InventoryStorageManagerComponent invMan = InventoryStorageManagerComponent.Cast(GetOwner().FindComponent(InventoryStorageManagerComponent));
				if (invMan)
					invMan.FindItems(magazines, predicate);

				foreach (IEntity nextMag : magazines)
				{
					if (nextMag != item)
					{
						StoreItemToQuickSlot(nextMag);
						break;
					}
				}

				return ReloadCurrentWeapon(item);
			}
			
			case ESlotFunction.TYPE_WEAPON:
			{
				CharacterControllerComponent controller = character.GetCharacterController();
				if (!controller)
					return false;
				
				m_Callback.m_pItem = item;
				m_Callback.m_Controller = controller;
				
				SCR_GadgetManagerComponent gadgetMgr = SCR_GadgetManagerComponent.GetGadgetManager(character);
				if (gadgetMgr)
					gadgetMgr.RemoveHeldGadget();
				
				TurretCompartmentSlot turretCompartment = TurretCompartmentSlot.Cast(GetCurrentCompartment());
				if (turretCompartment)
				{	
					TurretControllerComponent turretController = TurretControllerComponent.Cast(turretCompartment.GetController());
					if (!turretController)
						return false;
					
					array<WeaponSlotComponent> turretWeaponSlots = {};		
					GetTurretWeaponSlots(turretCompartment, turretWeaponSlots);
					foreach (WeaponSlotComponent weaponSlot: turretWeaponSlots)
					{
						if (weaponSlot.GetWeaponEntity() == item)
							return turretController.SelectWeapon(character, weaponSlot);
					}
				}
				else
				{
					SCR_InventoryStorageManagerComponent inventoryManager = SCR_InventoryStorageManagerComponent.Cast(controller.GetInventoryStorageManager());
					if (!inventoryManager)
						return false;
					
					BaseWeaponComponent currentWeapon;
					BaseWeaponManagerComponent manager = controller.GetWeaponManagerComponent();
					if (manager)
						currentWeapon = manager.GetCurrentWeapon();
					
					// Swap grenade type only if reselecting the same grenade slot
					BaseWeaponComponent itemWeapon = BaseWeaponComponent.Cast(item.FindComponent(BaseWeaponComponent));
					if (itemWeapon && itemWeapon.CanBeEquipped(controller) != ECanBeEquippedResult.OK)
					{
						return false;
					}
					else if (currentWeapon && itemWeapon && currentWeapon.GetWeaponType() == itemWeapon.GetWeaponType() && WEAPON_TYPES_THROWABLE.Contains(itemWeapon.GetWeaponType()))
					{
						// Equip different type of grenade
						IEntity nextGrenade = inventoryManager.FindNextWeaponOfType(itemWeapon.GetWeaponType(), currentWeapon.GetOwner());
						if (nextGrenade)
							m_Callback.m_pItem = nextGrenade;
					}
					// Currently selected weapon can have alternative muzzle
					else if (currentWeapon && currentWeapon.GetOwner() == item && !controller.IsGadgetInHands())
					{
						// Select next muzzle of a selected weapon
						int nextMuzzleID = SCR_WeaponLib.GetNextMuzzleID(currentWeapon); 
						if (nextMuzzleID != -1)
							return controller.SetMuzzle(nextMuzzleID);
						
						return false;
					}
					
					// TODO: Interrupt current equipping process now
					if (GetWeaponStorage() && GetWeaponStorage().Contains(m_Callback.m_pItem))
					{
						bool result = controller.TryEquipRightHandItem(m_Callback.m_pItem, EEquipItemType.EEquipTypeWeapon, false);
						m_Callback.m_pItem = null;
						return result;
					}

					return inventoryManager.EquipWeapon(m_Callback.m_pItem, m_Callback, false);
				}
				break;
			}
			
			case ESlotFunction.TYPE_GADGET:
			{
				// need to run through manager
				// TODO kamil: this doesnt call setmode when switching to other item from gadget (no direct call to scripted togglefocused for example, possibly other issues?)
				SCR_GadgetManagerComponent gadgetMgr = SCR_GadgetManagerComponent.GetGadgetManager(character);
				if (gadgetMgr)
				{
					SCR_GadgetComponent gadgetComp = SCR_GadgetComponent.Cast(item.FindComponent(SCR_GadgetComponent));
					if (gadgetComp.GetMode() == EGadgetMode.IN_HAND && gadgetComp.GetUseMask() != SCR_EUseContext.NONE)
					{
						gadgetComp.ToggleActive(!gadgetComp.IsToggledOn(), context);
					}
					else
					{
						if (gadgetMgr.GetHeldGadget() && gadgetMgr.GetHeldGadget() != item)
							gadgetMgr.RemoveHeldGadget();

						gadgetMgr.SetGadgetMode(item, EGadgetMode.IN_HAND);
					}
				}
				else
				{
					return false;
				}

				return true;
			}

			case ESlotFunction.TYPE_CLOTHES:
			{
				SCR_EquipGearCB cb = new SCR_EquipGearCB();
				cb.m_pMenu = SCR_InventoryMenuUI.GetInventoryMenu();

				SCR_InventoryStorageManagerComponent storageMgr = SCR_InventoryStorageManagerComponent.Cast(character.GetCharacterController().GetInventoryStorageManager());
				if (storageMgr)
					storageMgr.EquipAny(this, item, -1, cb);
			} break;

			case ESlotFunction.TYPE_STORAGE:
			{
				SCR_EquipGearCB cb = new SCR_EquipGearCB();
				cb.m_pMenu = SCR_InventoryMenuUI.GetInventoryMenu();

				SCR_InventoryStorageManagerComponent storageMgr = SCR_InventoryStorageManagerComponent.Cast(character.GetCharacterController().GetInventoryStorageManager());
				if (storageMgr)
					storageMgr.EquipAny(this, item, -1, cb);
			} break;
		} 
		return false;
	}
	
	//---- REFACTOR NOTE END ----
	
	//------------------------------------------------------------------------------------------------	
	protected bool CanReloadCurrentWeapon(notnull IEntity item)
	{
		BaseWeaponComponent currentWeapon = GetCurrentWeapon();
		if (!currentWeapon)
			return false;
		
		if (!currentWeapon.IsReloadPossible())
			return false;
		
		BaseMuzzleComponent currentMuzzle = currentWeapon.GetCurrentMuzzle();
		if (!currentMuzzle)
			return false;
		
		BaseMagazineWell currentMagWell = currentMuzzle.GetMagazineWell();
		if (!currentMagWell)
			return false;
		
		MagazineComponent magazine = MagazineComponent.Cast(item.FindComponent(MagazineComponent));
		if (!magazine)
			return false;
		
		BaseMagazineWell magazineWell = magazine.GetMagazineWell();
		if (!magazineWell)
			return false;
		
		if (magazineWell.Type() == currentMagWell.Type())
			return true;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------	
	protected bool ReloadCurrentWeapon(IEntity item)
	{
		if (!item)
			return false;
		
		ChimeraCharacter character = ChimeraCharacter.Cast(GetOwner());
		if (!character)
			return false;
		
		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return false;
		
		return controller.ReloadWeaponWith(item);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void GetPlayersWeapons( notnull inout array<IEntity> outWeapons )
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(GetOwner());
		if (!character)
			return;
		
		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return;
		
		BaseWeaponManagerComponent weaponManager = controller.GetWeaponManagerComponent();
		if (weaponManager)
			weaponManager.GetWeaponsList(outWeapons);
	}
	
	//------------------------------------------------------------------------------------------------
	protected BaseCompartmentSlot GetCurrentCompartment()
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(GetOwner());
		if (!character)
			return null;
		
		CompartmentAccessComponent compAccess = character.GetCompartmentAccessComponent();
		if (!compAccess || !compAccess.IsInCompartment())
			return null;
		
		return compAccess.GetCompartment();
	}
	
	//------------------------------------------------------------------------------------------------
	protected BaseWeaponComponent GetCurrentTurretWeapon()
	{
		BaseCompartmentSlot compartment = GetCurrentCompartment();
		if (!compartment)
			return null;
		
		TurretControllerComponent controller = TurretControllerComponent.Cast(compartment.GetController());
		if (!controller)
			return null;
		
		BaseWeaponManagerComponent weaponManager = controller.GetWeaponManager();
		if (!weaponManager)
			return null;
		
		return weaponManager.GetCurrentWeapon();
	}

	//------------------------------------------------------------------------------------------------
	protected BaseWeaponComponent GetCurrentCharacterWeapon()
	{
		BaseWeaponComponent turretWeapon = GetCurrentTurretWeapon();
		if (turretWeapon)
			return turretWeapon;

		ChimeraCharacter character = ChimeraCharacter.Cast(GetOwner());
		if (!character)
			return null;

		CharacterControllerComponent controller = character.GetCharacterController();	
		if (!controller)
			return null;

		BaseWeaponManagerComponent weaponManager = controller.GetWeaponManagerComponent();
		if (!weaponManager)
			return null;

		return weaponManager.GetCurrentWeapon();
	}

	//------------------------------------------------------------------------------------------------
	protected BaseWeaponComponent GetCurrentWeapon()
	{
		TurretCompartmentSlot compartment = TurretCompartmentSlot.Cast(GetCurrentCompartment());
		if (compartment)
			return GetCurrentTurretWeapon();
		else
			return GetCurrentCharacterWeapon();
	}

	//------------------------------------------------------------------------------------------------
	//! Select next weapon
	IEntity SelectNextWeapon(int maxSlot = -1)
	{
		IEntity nextWeapon = GetNextWeapon(maxSlot);
		if (!nextWeapon)
			return null;

		if (CanUseItem(nextWeapon))
		{
			UseItem(nextWeapon);

			int id = GetEntityIndexInQuickslots(nextWeapon);
			SCR_WeaponSwitchingBaseUI.HighlightQuickSlot(id);
		}

		return nextWeapon;
	}

	//------------------------------------------------------------------------------------------------
	//! Get next weapon up to specified maximum
	IEntity GetNextWeapon(int maxSlots = -1)
	{
		if (maxSlots < 0)
		{
			if (TurretCompartmentSlot.Cast(GetCurrentCompartment()))
				maxSlots = TURRET_WEAPON_SWITCH_SLOTS;
			else
				maxSlots = INFANTRY_WEAPON_SWITCH_SLOTS;
		}

		maxSlots = Math.Min(maxSlots, m_aQuickSlots.Count());

		IEntity currentWeapon;
		BaseWeaponComponent currentWeaponComponent = GetCurrentWeapon();
		if (currentWeaponComponent)
			currentWeapon = currentWeaponComponent.GetOwner();

		// If not currently equipped, select weapon again
		if (currentWeapon && currentWeapon == GetCurrentItem())
		{
			// Find current slot
			int currentSlot = -1;
			for (int i; i < maxSlots; i++)
			{
				if (GetItemFromQuickSlot(i) != currentWeapon)
					continue;

				currentSlot = i;
				break;
			}

			// Find next weapon within limit
			IEntity item;
			int nextSlot = -1;
			for (int i; i < maxSlots; i++)
			{
				nextSlot = (currentSlot + i + 1) % maxSlots;

				if (nextSlot == currentSlot)
					continue;

				item = GetItemFromQuickSlot(nextSlot);
				if (item)
					return item;
			}
		}
		else
		{
			// Get first valid item
			IEntity item;
			for (int i; i < maxSlots; i++)
			{
				item = GetItemFromQuickSlot(i);
				if (item)
					return item;
			}
		}

		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] pOwner
	//! \param[in] pControlled
	void InitAsPlayer(IEntity pOwner, bool pControlled)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(pOwner);
		if (!character)
			return;
		
		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return;
		
		SCR_InventoryStorageManagerComponent pInventoryManager = SCR_InventoryStorageManagerComponent.Cast(controller.GetInventoryStorageManager());
		if ( !pInventoryManager )
			return;
			
		if (pControlled)
		{
			array<IEntity> initSlotItems = {};
			pInventoryManager.GetItems(initSlotItems);
			foreach (IEntity item : initSlotItems)
			{
				if (!item)
					continue;

				const int slotId = GetDefaultQuickSlot(item);
				if (!SetQuickSlotInitialized(slotId))
					continue; // Already inited by external logic (e.g. save-game)

				if (GetEntityIndexInQuickslots(item) == slotId)
					continue; // Already in default quick slot

				StoreItemToQuickSlot(item);
			}

			pInventoryManager.m_OnItemAddedInvoker.Insert( HandleOnItemAddedToInventory );
			pInventoryManager.m_OnItemRemovedInvoker.Insert( HandleOnItemRemovedFromInventory );

			SCR_CharacterControllerComponent charController = SCR_CharacterControllerComponent.Cast(controller);
			if (charController)
				charController.m_OnItemUseEndedInvoker.Insert(OnItemUsed);
			
			m_CompartmentAccessComp = SCR_CompartmentAccessComponent.Cast(character.GetCompartmentAccessComponent());
			if (!m_CompartmentAccessComp)
				return;
			
			m_CompartmentAccessComp.GetOnCompartmentEntered().Insert(OnCompartmentEntered);
			m_CompartmentAccessComp.GetOnCompartmentLeft().Insert(OnCompartmentLeft);
			
			BaseCompartmentSlot compartmentSlot = m_CompartmentAccessComp.GetCompartment();
			if (compartmentSlot)
			{
				// If the character was already in a compartment slot before the character was initialized as a player, we need to handle it.
				OnCompartmentSlotEntered(compartmentSlot);
			}
		}
		else
		{
			pInventoryManager.m_OnItemAddedInvoker.Remove( HandleOnItemAddedToInventory );
			pInventoryManager.m_OnItemRemovedInvoker.Remove( HandleOnItemRemovedFromInventory );
			
			if (m_CompartmentAccessComp)
			{
				m_CompartmentAccessComp.GetOnCompartmentEntered().Remove(OnCompartmentEntered);
				m_CompartmentAccessComp.GetOnCompartmentLeft().Remove(OnCompartmentLeft);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	// Called when item is used by the player
	protected void OnItemUsed(IEntity item, bool successful, ItemUseParameters animParams)
	{
		if (!item)
			return;

		// If the item isn't consumable, return
		if (!SCR_ConsumableItemComponent.Cast(item.FindComponent(SCR_ConsumableItemComponent)))
			return;
		
		// restock used medical item back to its quick slot
		int quickSlot = GetEntityIndexInQuickslots(item);
		if (quickSlot == -1)
			return;

		RemoveItemFromQuickSlotAtIndex(quickSlot);
		int itemType = GetItemType(item);

		typename t = BaseWeaponComponent;
		if (itemType > GADGET_OFFSET)
			t = SCR_GadgetComponent;

		SCR_ItemTypeSearchPredicate itemSearch = new SCR_ItemTypeSearchPredicate(t, itemType, item);
		array<IEntity> items = {};

		InventoryStorageManagerComponent invMan = InventoryStorageManagerComponent.Cast(GetOwner().FindComponent(InventoryStorageManagerComponent));
		if (!invMan)
			return;

		invMan.FindItems(items, itemSearch);

		if (!items.IsEmpty())
		{
			foreach (IEntity itm : items)
			{
				if (itm == item)
					continue;
				StoreItemToQuickSlot(itm, quickSlot);
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	private void OnCompartmentSlotEntered(BaseCompartmentSlot compartment)
	{
		if (m_CompartmentAccessComp)
		{
			m_aWeaponQuickSlotsStorage.Clear();
			IEntity entity;
			SCR_QuickslotEntityContainer entityContainer;
			for (int i, count = m_aQuickSlots.Count(); i < SCR_InventoryMenuUI.WEAPON_SLOTS_COUNT && i < count; i++)
			{
				entityContainer = SCR_QuickslotEntityContainer.Cast(m_aQuickSlots[i]);
				if (!entityContainer)
				{
					m_aWeaponQuickSlotsStorage.Insert(null);
					continue;
				}
				
				m_aWeaponQuickSlotsStorage.Insert(entityContainer.GetEntity());
			}
		}
		
		if (!TurretCompartmentSlot.Cast(compartment) && !m_aWeaponQuickSlotsStorage.IsEmpty())
		{
			RemoveItemsFromWeaponQuickSlots();
			
			IEntity quickSlotEntity;
			SCR_InventoryStorageManagerComponent pInventoryManager = SCR_InventoryStorageManagerComponent.Cast(GetOwner().FindComponent( SCR_InventoryStorageManagerComponent));
			
			for (int i; i < m_aWeaponQuickSlotsStorage.Count(); i++)
			{
				quickSlotEntity = m_aWeaponQuickSlotsStorage[i];
				
				if (!quickSlotEntity)
					continue;
				
				if (!pInventoryManager.Contains(quickSlotEntity))
					continue;
				
				StoreItemToQuickSlot(quickSlotEntity, i, true);
			}
			
			SCR_WeaponSwitchingBaseUI.RefreshQuickSlots();
			
			return;
		}

		array<WeaponSlotComponent> turretWeaponSlots = {};
		if (GetTurretWeaponSlots(compartment, turretWeaponSlots) > 0)
		{
			RemoveItemsFromWeaponQuickSlots();
			
			foreach (int i, WeaponSlotComponent weaponSlot: turretWeaponSlots)
			{
				IEntity weaponSlotEntity = weaponSlot.GetWeaponEntity();
				if (!weaponSlotEntity)
					continue;

				StoreItemToQuickSlot(weaponSlotEntity, i, true);
			}
		}
		
		SCR_WeaponSwitchingBaseUI.RefreshQuickSlots();
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_CompartmentAccessComponent event
	//! \param[in] targetEntity
	//! \param[in] manager
	//! \param[in] mgrID
	//! \param[in] slotID
	//! \param[in] move
	protected void OnCompartmentEntered(IEntity targetEntity, BaseCompartmentManagerComponent manager, int mgrID, int slotID, bool move)
	{
		BaseCompartmentSlot compartment = manager.FindCompartment(slotID, mgrID);
		
		OnCompartmentSlotEntered(compartment);
	}

	//------------------------------------------------------------------------------------------------
	//! SCR_CompartmentAccessComponent event
	//! \param[in] targetEntity
	//! \param[in] manager
	//! \param[in] mgrID
	//! \param[in] slotID
	//! \param[in] move
	protected void OnCompartmentLeft(IEntity targetEntity, BaseCompartmentManagerComponent manager, int mgrID, int slotID, bool move)
	{
		BaseCompartmentSlot compartment = manager.FindCompartment(slotID, mgrID);
		
		if (!TurretCompartmentSlot.Cast(compartment))
		{
			m_aWeaponQuickSlotsStorage.Clear();
			IEntity entity;
			SCR_QuickslotEntityContainer entityContainer;
			for (int i, count = m_aQuickSlots.Count(); i < SCR_InventoryMenuUI.WEAPON_SLOTS_COUNT && i < count; i++)
			{
				entityContainer = SCR_QuickslotEntityContainer.Cast(m_aQuickSlots[i]);
				if (!entityContainer)
				{
					m_aWeaponQuickSlotsStorage.Insert(null);
					continue;
				}
				
				m_aWeaponQuickSlotsStorage.Insert(entityContainer.GetEntity());
			}
		}
		
		if (m_CompartmentAccessComp)
		{
			RemoveItemsFromWeaponQuickSlots();
			
			IEntity quickSlotEntity;
			SCR_InventoryStorageManagerComponent pInventoryManager = SCR_InventoryStorageManagerComponent.Cast(GetOwner().FindComponent( SCR_InventoryStorageManagerComponent));
			
			for (int i; i < m_aWeaponQuickSlotsStorage.Count(); i++)
			{
				quickSlotEntity = m_aWeaponQuickSlotsStorage[i];
				
				if (!quickSlotEntity)
					continue;
				
				if (!pInventoryManager.Contains(quickSlotEntity))
					continue;
				
				StoreItemToQuickSlot(quickSlotEntity, i, true);
			}
			
			m_aWeaponQuickSlotsStorage.Clear();
		}
		
		SCR_WeaponSwitchingBaseUI.RefreshQuickSlots();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RemoveItemsFromWeaponQuickSlots()
	{
		for (int i = 0; i < SCR_InventoryMenuUI.WEAPON_SLOTS_COUNT; i++)
		{
			RemoveItemFromQuickSlotAtIndex(i);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected int GetTurretWeaponsList(BaseCompartmentSlot compartment, out array<IEntity> weaponsList)
	{
		TurretControllerComponent turretController = TurretControllerComponent.Cast(compartment.GetController());
		if (!turretController)
			return -1;
		
		BaseWeaponManagerComponent weaponManager = turretController.GetWeaponManager();
		if (weaponManager)
			return weaponManager.GetWeaponsList(weaponsList);
		
		return -1;
	}
	
	//------------------------------------------------------------------------------------------------
	protected int GetTurretWeaponSlots(BaseCompartmentSlot compartment, out array<WeaponSlotComponent> weaponSlots)
	{
		TurretControllerComponent turretController = TurretControllerComponent.Cast(compartment.GetController());
		if (!turretController)
			return -1;
		
		BaseWeaponManagerComponent weaponManager = turretController.GetWeaponManager();
		if (weaponManager)
			return weaponManager.GetWeaponsSlots(weaponSlots);
		
		return -1;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	int GetEntityIndexInQuickslots(notnull IEntity entity)
	{
		int index = -1;
		SCR_QuickslotEntityContainer entityContainer;
		foreach (int i, SCR_QuickslotBaseContainer container : m_aQuickSlots)
		{
			entityContainer = SCR_QuickslotEntityContainer.Cast(container);
			if (entityContainer && entityContainer.GetEntity() == entity)
				index = i;
		} 
		
		return index;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	int GetFirstSlotIndexOf(ECommonItemType type)
	{
		int index = -1;
		SCR_QuickslotEntityContainer entityContainer;
		IEntity testedEntity;
		InventoryItemComponent itemComponent;
		foreach (int i, SCR_QuickslotBaseContainer container : m_aQuickSlots)
		{
			entityContainer = SCR_QuickslotEntityContainer.Cast(container);
			if (entityContainer && entityContainer.GetEntity())
			{
				testedEntity = entityContainer.GetEntity();
				itemComponent = InventoryItemComponent.Cast(testedEntity.FindComponent(InventoryItemComponent));
				if (!itemComponent)
					continue;;
				
				if (itemComponent.GetAttributes().GetCommonType() != type)
					continue;
				
				index = i;
			}
		} 
		
		return index;
	}
	
	//------------------------------------------------------------------------ PERCEIVED FACTION ----------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	//! Called by SCR_CharacterFactionAffiliationComponent on init. Sets the m_CharacterAffiliationComponent as well as making sure that the items are added to the outfit values of the faction affiliation comp
	//! \param[in] characterAffiliationComponent Character Faction Affiliation component to set as a reference 
	void InitCharacterPerceivedOutfitData(notnull SCR_CharacterFactionAffiliationComponent characterAffiliationComponent)
	{
		m_CharacterAffiliationComponent = characterAffiliationComponent;
		
		array<IEntity> allItems = {};
		GetAll(allItems);
		
		SCR_ItemOutfitFactionComponent outfitComponent;
		
		//~ Update the perceived faction. Does not replicate as that is called when the init is done by the SCR_CharacterFactionAffiliationComponent
		foreach (IEntity item : allItems)
		{
			//~ Only check items that are in slots
			if (!FindItemSlot(item))
				continue;
			
			UpdatePerceivedFaction(true, item, false);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Called on init and when an item is added or removed from the character
	protected void UpdatePerceivedFaction(bool addedToSlot, IEntity item, bool updateFaction = true)
	{
		if (!m_CharacterAffiliationComponent || !m_CharacterAffiliationComponent.HasPerceivedFaction())
			return;
		
		//~ Send update to character affiliation comp that a piece of clothing was added/removed even if it has no faction
		SCR_ItemOutfitFactionComponent outfitComponent = SCR_ItemOutfitFactionComponent.Cast(item.FindComponent(SCR_ItemOutfitFactionComponent));
		if (!outfitComponent)
		{
			m_CharacterAffiliationComponent.OnNoFactionSlottedItemChanged(addedToSlot, item, updateFaction);
			return;
		}
			
		if (addedToSlot)
			outfitComponent.OnAddedToSlot(m_CharacterAffiliationComponent, updateFaction);
		else
			outfitComponent.OnRemovedFromSlot(m_CharacterAffiliationComponent, updateFaction);
	}	
	
	//------------------------------------------------------------------------ COMMON METHODS ----------------------------------------------------------------------
	
	#else

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetMaxLoad();

	//------------------------------------------------------------------------------------------------
	//! \return
	BaseInventoryStorageComponent GetWeaponStorage();

	//------------------------------------------------------------------------------------------------
	//! \param[in] eSlot
	//! \return
	InventoryItemComponent GetItemFromLoadoutSlot( ELoadoutArea eSlot );

	//------------------------------------------------------------------------------------------------
	//! \param[in] eSlot
	//! \return
	BaseInventoryStorageComponent GetStorageFromLoadoutSlot( ELoadoutArea eSlot );
	protected bool HasStorageComponent( IEntity pEntity );

	//------------------------------------------------------------------------------------------------
	//! \param[out] storagesInInventory
	void GetStorages( out notnull array<SCR_UniversalInventoryStorageComponent> storagesInInventory );

	//------------------------------------------------------------------------------------------------
	//! \param[in] pEntity
	//! \return
	SCR_UniversalInventoryStorageComponent GetStorageComponentFromEntity( IEntity pEntity );

	//------------------------------------------------------------------------------------------------
	//! \param[in] pOwner
	void SetLootStorage( IEntity pOwner );

	//------------------------------------------------------------------------------------------------
	//! \return
	BaseInventoryStorageComponent GetLootStorage();
	//override bool CanStoreItem(IEntity item, int slotID);
	//override bool CanRemoveItem(IEntity item);
	//protected override void OnRemovedFromSlot(IEntity item, int slotID);
	//override InventoryStorageSlot GetEmptySlotForItem( IEntity item );
	//protected override void OnAddedToSlot(IEntity item, int slotID);
	#endif
}
