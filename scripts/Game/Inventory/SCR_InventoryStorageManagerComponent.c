[EntityEditorProps(category: "GameScripted/UI/Inventory", description: "[MOD] Inventory Slot UI class")]
class SCR_InventoryStorageManagerComponentClass : ScriptedInventoryStorageManagerComponentClass
{
}

enum EInventoryRetCode
{
	RETCODE_OK = 0,
	RETCODE_ITEM_TOO_BIG = 2,
	RETCODE_ITEM_TOO_HEAVY = 4,
	RETCODE_DEFAULT_STATE = 0xFFFF
}

enum ECallbackState
{
	DROP = 0,
	INSERT = 1,
	MOVE = 2,
	FINAL = 3
}

enum EResupplyUnavailableReason
{
	//~ If multiple reasons for Resupply Unavailable than the Highst enum will be used
	NONE,
	NO_VALID_WEAPON = 10,
	ENOUGH_ITEMS = 20,
	NOT_IN_GIVEN_STORAGE = 30,
	INVENTORY_FULL = 40,
	RANK_TOO_LOW = 50,
	NOT_ENOUGH_AVAILABLE_ALLOCATED_SUPPLIES = 60,

	//~ Resupply was valid. Add invalid reasons above
	RESUPPLY_VALID = 99999,
}

class SCR_HoldableItemPredicate : InventorySearchPredicate
{
    ECommonItemType wanted;

	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_HoldableItemPredicate()
	{
        QueryAttributeTypes.Insert(SCR_ItemOfInterestAttribute);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool IsMatch(BaseInventoryStorageComponent storage, IEntity item, array<GenericComponent> queriedComponents, array<BaseItemAttributeData> queriedAttributes)
	{
        SCR_ItemOfInterestAttribute optionalAttribute = SCR_ItemOfInterestAttribute.Cast(queriedAttributes[0]);
		return optionalAttribute.GetInterestType() == wanted;
	}
}

class SCR_BandagePredicate : InventorySearchPredicate
{
	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_BandagePredicate()
	{
		QueryComponentTypes.Insert(SCR_ConsumableItemComponent);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool IsMatch(BaseInventoryStorageComponent storage, IEntity item, array<GenericComponent> queriedComponents, array<BaseItemAttributeData> queriedAttributes)
	{		
		return (SCR_ConsumableItemComponent.Cast(queriedComponents[0])).GetConsumableType() == SCR_EConsumableType.BANDAGE;
	}
}

class SCR_ApplicableMedicalItemPredicate : InventorySearchPredicate
{
	IEntity characterEntity;
	ECharacterHitZoneGroup hitZoneGroup;

	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_ApplicableMedicalItemPredicate()
	{
		QueryComponentTypes.Insert(SCR_ConsumableItemComponent);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool IsMatch(BaseInventoryStorageComponent storage, IEntity item, array<GenericComponent> queriedComponents, array<BaseItemAttributeData> queriedAttributes)
	{
		SCR_EConsumableType type = SCR_ConsumableItemComponent.Cast(queriedComponents[0]).GetConsumableType();
		bool isMatch = (type == SCR_EConsumableType.BANDAGE)
			|| (type == SCR_EConsumableType.HEALTH)
			|| (type == SCR_EConsumableType.TOURNIQUET)
			|| (type == SCR_EConsumableType.SALINE)
			|| (type == SCR_EConsumableType.MORPHINE);

		if (!isMatch)
			return false;

		SCR_ConsumableItemComponent medicalItem = SCR_ConsumableItemComponent.Cast(item.FindComponent(SCR_ConsumableItemComponent));
		SCR_ConsumableEffectHealthItems effect = SCR_ConsumableEffectHealthItems.Cast(medicalItem.GetConsumableEffect());
		if (!effect)
			return false;

		return effect.CanApplyEffectToHZ(characterEntity, characterEntity, hitZoneGroup);
	}
}

class SCR_ItemTypeSearchPredicate : InventorySearchPredicate
{
	int m_iItemType = -1;
	IEntity m_iOriginalItem;
	
	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] type
	//! \param[in] wantedItemType
	//! \param[in] originalItem
	void SCR_ItemTypeSearchPredicate(typename type, int wantedItemType, IEntity originalItem)
	{
		QueryComponentTypes.Insert(type);
		m_iItemType = wantedItemType;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool IsMatch(BaseInventoryStorageComponent storage, IEntity item, array<GenericComponent> queriedComponents, array<BaseItemAttributeData> queriedAttributes)
	{
		return (item != m_iOriginalItem) && (SCR_CharacterInventoryStorageComponent.GetItemType(item) == m_iItemType);
	}
}

class SCR_CommonItemTypeSearchPredicate : InventorySearchPredicate
{
	ECommonItemType m_eItemType = -1;
	IEntity m_OriginalItem;

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] wantedItemType
	//! \param[in] originalItem
	void SCR_CommonItemTypeSearchPredicate(ECommonItemType wantedItemType, IEntity originalItem)
	{
		m_eItemType = wantedItemType;
		m_OriginalItem = originalItem;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool IsMatch(BaseInventoryStorageComponent storage, IEntity item, array<GenericComponent> queriedComponents, array<BaseItemAttributeData> queriedAttributes)
	{
		InventoryItemComponent iic = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
		if (!iic || !iic.GetAttributes())
			return false;

		return (item != m_OriginalItem) && (iic.GetAttributes().GetCommonType() == m_eItemType);
	}
}

//! Searches for attachments of the defined atttachmentType
class SCR_CompatibleAttachmentPredicate : InventorySearchPredicate
{
	typename attachmentType;

	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_CompatibleAttachmentPredicate()
	{
		QueryComponentTypes.Insert(InventoryItemComponent);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool IsMatch(BaseInventoryStorageComponent storage, IEntity item, array<GenericComponent> queriedComponents, array<BaseItemAttributeData> queriedAttributes)
	{
		InventoryItemComponent itemComp = InventoryItemComponent.Cast(queriedComponents[0]);

		if (!itemComp)
			return false;

		ItemAttributeCollection itemAttributes = itemComp.GetAttributes();
		if (!itemAttributes)
			return false;

		WeaponAttachmentAttributes itemAttribute = WeaponAttachmentAttributes.Cast(itemAttributes.FindAttribute(WeaponAttachmentAttributes));
		if (!itemAttribute)
			return false;

		BaseAttachmentType itemAttachmentType = itemAttribute.GetAttachmentType();
		if (!itemAttachmentType)
			return false;

		typename itemAttachmentTypename = itemAttachmentType.Type();
		if (!itemAttachmentTypename)
			return false;

		return itemAttachmentTypename.IsInherited(attachmentType); // Check if attachment types match
	}
}

class SCR_SalinePredicate : InventorySearchPredicate
{
	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_SalinePredicate()
	{
		QueryComponentTypes.Insert(SCR_ConsumableItemComponent);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool IsMatch(BaseInventoryStorageComponent storage, IEntity item, array<GenericComponent> queriedComponents, array<BaseItemAttributeData> queriedAttributes)
	{
		if (storage.Type().IsInherited(EquipmentStorageComponent))
			return true;
		
		return false;
	}
}

//! Searches for magazines with certain mag well
class SCR_MagazinePredicate : InventorySearchPredicate
{
	typename magWellType;
	
	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_MagazinePredicate()
	{
		QueryComponentTypes.Insert(BaseMagazineComponent);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool IsMatch(BaseInventoryStorageComponent storage, IEntity item, array<GenericComponent> queriedComponents, array<BaseItemAttributeData> queriedAttributes)
	{
		BaseMagazineComponent iMag = BaseMagazineComponent.Cast(queriedComponents[0]);
		if (!iMag)
			return false;
		
		BaseMagazineWell iMagWell = iMag.GetMagazineWell();
		if (!iMagWell)
			return false;
		
		return (iMagWell.IsInherited(magWellType)); // Check if magwells match
	}
}

//! Searches for items with same prefab name
class SCR_PrefabNamePredicate : InventorySearchPredicate
{
	string prefabName;
	
	//------------------------------------------------------------------------------------------------
	override protected bool IsMatch(BaseInventoryStorageComponent storage, IEntity item, array<GenericComponent> queriedComponents, array<BaseItemAttributeData> queriedAttributes)
	{
		EntityPrefabData pd = item.GetPrefabData();
		return pd.GetPrefabName() == this.prefabName;
	}
}

//! Searches for items with same prefab data (prefer this to prefab name if you have prefab data already)
class SCR_PrefabDataPredicate : InventorySearchPredicate
{
	EntityPrefabData prefabData;
	
	//------------------------------------------------------------------------------------------------
	override protected bool IsMatch(BaseInventoryStorageComponent storage, IEntity item, array<GenericComponent> queriedComponents, array<BaseItemAttributeData> queriedAttributes)
	{
		return item.GetPrefabData() == this.prefabData;
	}
}

//! Searches for items with same prefab name by comparing prefab pointers rather than strings
class SCR_ResourceNamePredicate : InventorySearchPredicate
{
	//! either set this manually or pass ResourceName to the constructor 
	BaseContainer m_PrefabContainer;
	//! Entity which is going to be ignored
	IEntity m_ExcludedItem;

	//------------------------------------------------------------------------------------------------
	//! \param[in] rn ResourceName of the prefab that will be used for matching
	void SCR_ResourceNamePredicate(ResourceName rn = string.Empty, IEntity excludedItem = null)
	{
		m_ExcludedItem = excludedItem;
		Resource resource = Resource.Load(rn);
		if (resource.IsValid())
			m_PrefabContainer = resource.GetResource().ToBaseContainer();
	}

	//------------------------------------------------------------------------------------------------
	override protected bool IsMatch(BaseInventoryStorageComponent storage, IEntity item, array<GenericComponent> queriedComponents, array<BaseItemAttributeData> queriedAttributes)
	{
		return item != m_ExcludedItem && item.GetPrefabData().GetPrefab() == m_PrefabContainer;
	}
}

class DropAndMoveOperationCallback : ScriptedInventoryOperationCallback
{
	InventoryItemComponent m_ItemBefore;
	InventoryItemComponent m_ItemAfter;
	InventoryStorageSlot m_TargetSlot;
	SCR_InventoryStorageManagerComponent m_Manager;
	SCR_InvCallBack m_FinalCB;
	bool m_bIstakenFromArsenal;
	ref array<IEntity> m_aItemsToMove = {};
	ECallbackState m_ECurrentState = 0; // 0 - drop, 1 - insert, 2 - move, 3 - final
	
	//------------------------------------------------------------------------------------------------
	override protected void OnComplete()
	{
		switch (m_ECurrentState)
		{
			case ECallbackState.DROP:
			{
				OnDropComplete();
			}
			break;

			case ECallbackState.INSERT:
			{
				OnInsertComplete();
			}
			break;

			case ECallbackState.MOVE:
			{
				OnMoveComplete();
			}
			break;

			case ECallbackState.FINAL:
			{
				OnFinalState();
			}
			break;
		}
	} 

	//------------------------------------------------------------------------------------------------
	protected void OnDropComplete()
	{
		m_ECurrentState++;
		m_Manager.TryMoveItemToStorage(m_ItemAfter.GetOwner(), m_TargetSlot.GetStorage(), m_TargetSlot.GetID(), this);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnInsertComplete()
	{
		if (!m_bIstakenFromArsenal)
		{
			m_ECurrentState = ECallbackState.FINAL;
			OnFinalState();
			return;
		} 
		
		m_ECurrentState++;
		//BaseInventoryStorageComponent itemIsStorage = BaseInventoryStorageComponent.Cast(m_ItemBefore);
		//m_Manager.GetAllItems(m_aItemsToMove, itemIsStorage);
		OnMoveComplete();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnMoveComplete()
	{
		// Temporarily disabled for performance reasons. Once it gets fixed the transfer of items can be reenabled.
//		if (m_aItemsToMove.Count() == 0)
//		{
//			m_ECurrentState++;
//			OnDeleteComplete();
//			return;
//		}
//		IEntity item = m_aItemsToMove[m_aItemsToMove.Count() - 1];
//		m_aItemsToMove.Resize(m_aItemsToMove.Count() - 1);
//		BaseInventoryStorageComponent storage = m_Manager.FindStorageForItem(item, EStoragePurpose.PURPOSE_ANY);
//		if (!m_Manager.TryMoveItemToStorage(item, storage, -1, this))
//			OnMoveComplete();

		m_ECurrentState = ECallbackState.FINAL;
		OnFinalState();
		return;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnFinalState()	
	{
		if (m_FinalCB)
			m_FinalCB.InternalComplete();
	}
}

class SCR_ResupplyMagazinesCallback : ScriptedInventoryOperationCallback
{
	protected SCR_InventoryStorageManagerComponent m_Manager;
	protected ref map<ResourceName, int> m_MagazinesToSpawn = new map<ResourceName, int>();
	protected BaseInventoryStorageComponent m_LastTargetStorage;
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] prefab
	//! \param[in] count
	void Insert(ResourceName prefab, int count)
	{
		int currentCount;
		m_MagazinesToSpawn.Find(prefab, currentCount);
		m_MagazinesToSpawn.Insert(prefab, currentCount + count);
	}

	//------------------------------------------------------------------------------------------------
	//!
	void Start()
	{
		OnComplete();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnComplete()
	{
		if (!m_Manager)
			return;

		if (m_LastTargetStorage)
		{//if we spawned something in hand slot then we need to skip the wait for the animation as there is no animation in this case
			SCR_HandSlotStorageComponent handSlotStorage = SCR_HandSlotStorageComponent.Cast(m_LastTargetStorage);
			if (handSlotStorage)
				handSlotStorage.SkipAnimation_S();
		}

		if (!m_MagazinesToSpawn.IsEmpty())
		{
			//--- Next item to process
			ResourceName prefab = m_MagazinesToSpawn.GetKey(0);
			int count = m_MagazinesToSpawn.GetElement(0);
			count--;
			
			if (count == 0)
				m_MagazinesToSpawn.Remove(prefab);
			else
				m_MagazinesToSpawn.Set(prefab, count);
			
			m_LastTargetStorage = m_Manager.FindStorageForResource(prefab);
			m_Manager.TrySpawnPrefabToStorage(prefab, m_LastTargetStorage, cb: this);
		}
		else
		{
			//--- All processed
			OnFailed();
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnFailed()
	{
		//--- Delete itself (after a delay - can't delete itself at this frame)
		GetGame().GetCallqueue().CallLater(m_Manager.EndResupplyMagazines, 1, false);
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] manager
	void SCR_ResupplyMagazinesCallback(SCR_InventoryStorageManagerComponent manager)
	{
		m_Manager = manager;
	}
}

class SCR_InventoryStorageManagerComponent : ScriptedInventoryStorageManagerComponent
{
	private SCR_CharacterInventoryStorageComponent				m_Storage;
	private SCR_CharacterControllerComponent					m_CharacterController;
	private	ref SCR_BandagePredicate 							m_BandagePredicate = new SCR_BandagePredicate();
	protected EInventoryRetCode									m_ERetCode;
	protected int												m_iHealthEquipment	=	0;
	protected bool 												m_bIsInventoryLocked = false;
	protected ref SCR_WeaponSwitchingBaseUI						m_pWeaponSwitchingUI;
	private bool												m_bWasRaised;
	private IEntity 											m_StorageToOpen;
	protected ref SCR_ResupplyMagazinesCallback						m_ResupplyMagazineCallback;
	
	ref ScriptInvokerBool 									m_OnInventoryOpenInvoker	= new ScriptInvokerBool();
	ref ScriptInvokerBool 									m_OnQuickBarOpenInvoker		= new ScriptInvokerBool();

	//------------------------------------------------------------------------------------------------
	//! Get an array of all root items in the inventory storage.
	//! \param[out] rootItems All root items without going in the sub inventory of the items or attachments
	//! \return Count of root items
	[Obsolete("Use GetItems or a custom predicate with Find instead.")]
	int GetAllRootItems(out notnull array<IEntity> rootItems)
	{
		rootItems.Clear();
		array<BaseInventoryStorageComponent> storages = {};
		
		//~ Get deposits like backpacks and jackets as well as any held weapons
		GetStorages(storages, EStoragePurpose.PURPOSE_DEPOSIT);
		GetStorages(storages, EStoragePurpose.PURPOSE_WEAPON_PROXY);
		
		array<IEntity> items = {};
		foreach (BaseInventoryStorageComponent storage : storages)
		{
			//~ If backpack or jacked or any other cloth storage only get what is inside the storage
			if (ClothNodeStorageComponent.Cast(storage))
			{
				if (!storage)
					continue;
				
				array<BaseInventoryStorageComponent> clothStorages = {};
				storage.GetOwnedStorages(clothStorages, 1, false);
			
				foreach (BaseInventoryStorageComponent clothStorage : clothStorages)
				{
					if (!clothStorage)
						continue;
					
					clothStorage.GetAll(items);
					rootItems.Copy(items);
					items.Clear();
				}
				
				continue;
			}

			storage.GetAll(items);
			rootItems.Copy(items);
			items.Clear();
		}
		
		return rootItems.Count();
	}

	//------------------------------------------------------------------------------------------------
	// Callback when item is added (will be performed locally after server completed the Insert/Move operation)
	override protected void OnItemAdded(BaseInventoryStorageComponent storageOwner, IEntity item)
	{		
		super.OnItemAdded(storageOwner, item);

		SCR_ConsumableItemComponent consumable = SCR_ConsumableItemComponent.Cast(item.FindComponent(SCR_ConsumableItemComponent));
		if ( consumable && consumable.GetConsumableType() == SCR_EConsumableType.BANDAGE )
			m_iHealthEquipment++;	//store count of the health components
	}

	//------------------------------------------------------------------------------------------------
	// Callback when item is removed (will be performed locally after server completed the Remove/Move operation)
	override protected void OnItemRemoved(BaseInventoryStorageComponent storageOwner, IEntity item)
	{
		super.OnItemRemoved(storageOwner, item);

		SCR_ConsumableItemComponent consumable = SCR_ConsumableItemComponent.Cast(item.FindComponent(SCR_ConsumableItemComponent));
		if ( consumable && consumable.GetConsumableType() == SCR_EConsumableType.BANDAGE )
			m_iHealthEquipment--;	//store count of the health components
	}

	//------------------------------------------------------------------------------------------------
	override protected bool ShouldForbidRemoveByInstigator(InventoryStorageManagerComponent instigatorManager, BaseInventoryStorageComponent fromStorage, IEntity item)
	{
		//in case of health items, permit medics to donate healing items to targets
		SCR_ConsumableItemComponent consumableItemComp = SCR_ConsumableItemComponent.Cast(item.FindComponent(SCR_ConsumableItemComponent));
		if (consumableItemComp)
			return false;
		
		if (m_CharacterController && m_CharacterController.GetLifeState() == ECharacterLifeState.ALIVE)
			return true;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] entity
	//! \param[in] soundEvent
	void PlayItemSound(IEntity entity, string soundEvent)
	{	
		if (!entity)
			return;

		RplComponent rplComp = RplComponent.Cast(entity.FindComponent(RplComponent));
		if (rplComp)
		{
			Rpc(RpcAsk_PlaySound, rplComp.Id(), soundEvent);
		}
		else
		{
			SoundComponent soundComp = SoundComponent.Cast(entity.FindComponent(SoundComponent));
			if (soundComp && soundComp.GetEventIndex(soundEvent) != -1)
			{
				soundComp.SoundEvent(soundEvent);
			}
			else
			{			
				SCR_SoundManagerModule.CreateAndPlayAudioSource(entity, soundEvent);	
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] targetRplId
	//! \param[in] soundAction
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_PlaySound(RplId targetRplId, string soundAction)
	{
		Rpc(RpcDo_PlaySound, targetRplId, soundAction);
		RpcDo_PlaySound(targetRplId, soundAction);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] targetRplId
	//! \param[in] soundAction
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_PlaySound(RplId targetRplId, string soundAction)
	{
		RplComponent rplComp = RplComponent.Cast(Replication.FindItem(targetRplId));
		if (!rplComp)
			return;
		
		IEntity entity = rplComp.GetEntity();
		if (!entity)
			return;
		
		SoundComponent soundComp = SoundComponent.Cast(entity.FindComponent(SoundComponent));
		if (soundComp && soundComp.GetEventIndex(soundAction) != -1)
		{
			soundComp.SoundEvent(soundAction);
		}
		else
		{			
			SCR_SoundManagerModule.CreateAndPlayAudioSource(entity, soundAction);	
		}
	}
	
#ifndef DISABLE_INVENTORY

	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_CharacterInventoryStorageComponent GetCharacterStorage()
	{
		return m_Storage;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] item
	//! \param[in] storage
	//! \param[in] slotID
	//! \return
	// TODO: make this method as native (cannot override the proto native CanMoveItemToStorage)
	bool CanInsertItemInActualStorage(IEntity item, BaseInventoryStorageComponent storage, int slotID = -1)
	{
		if (!IsAnimationReady() || IsInventoryLocked())
			return false;

		array<BaseInventoryStorageComponent> pStorages = {};
		storage.GetOwnedStorages( pStorages, 1, false );
		pStorages.Insert( storage );

		foreach ( BaseInventoryStorageComponent pStorage : pStorages )
		{	
			bool bCanInsert = CanInsertItemInStorage( item, pStorage, -1 ); //split because of debug purposes
			bool bCanMove = CanMoveItemToStorage( item, pStorage, -1 );
			if ( bCanInsert || bCanMove )
				return true;
		}
		return false;
	}
	
	BaseInventoryStorageComponent FindActualStorageForItemResource(ResourceName itemResource, BaseInventoryStorageComponent storage, int slotID = -1)
	{
		array<BaseInventoryStorageComponent> storages = {};
		storage.GetOwnedStorages(storages, 1, false);
		storages.Insert(storage);

		foreach (BaseInventoryStorageComponent actualStorage : storages)
		{
			if (CanInsertResourceInStorage(itemResource, actualStorage, slotID))
				return actualStorage;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	// ! The return code informs about the state of the operation ( i.e. cannot insert item, since it is too large )
	// ! it clear the flag
	//! \param[in] ERetCode
	void SetReturnCode( EInventoryRetCode ERetCode ) 
	{ 
		m_ERetCode &= ~ERetCode; 
	}
	
	//------------------------------------------------------------------------------------------------
	// ! The return code informs about the state of the operation ( i.e. cannot insert item, since it is too large )
	// ! it clear the flag
	void SetReturnCodeDefault() 
	{ 
		m_ERetCode = EInventoryRetCode.RETCODE_DEFAULT_STATE;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	EInventoryRetCode GetReturnCode()
	{
		return m_ERetCode;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	float GetTotalWeightOfAllStorages()
	{
		array<BaseInventoryStorageComponent> storages = {};
		float fTotalWeight = 0.0;

		//TODO: actually not a very good way how to get storages, but using the GetStorages() method causes the weight being doubled. We need to get just the "parent" storages
		//Also if someone ever fixes this let character anim progs know to fix it in CharacterControllerComponent.CalculateSpeedLimits.
		storages.Insert(m_Storage.GetWeaponStorage());
		storages.Insert(m_Storage);
				
		foreach (BaseInventoryStorageComponent storage : storages)
		{
			fTotalWeight += storage.GetTotalWeight();
		}

		return fTotalWeight;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] pOwner
	void SetLootStorage( IEntity pOwner )
	{
		if (m_Storage)
			m_Storage.SetLootStorage(pOwner);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Try to insert the item into the storage (not slot)
	//! \param[in] pItem
	//! \param[in] pStorageTo
	//! \param[in] pStorageFrom
	//! \param[in] cb
	void InsertItem( IEntity pItem, BaseInventoryStorageComponent pStorageTo = null, BaseInventoryStorageComponent pStorageFrom = null, SCR_InvCallBack cb = null  )
	{
		if (!pItem || !IsAnimationReady() || IsInventoryLocked())
			return;
		
		SetInventoryLocked(true);

		bool canInsert = true;
		if ( !pStorageTo ) // no storage selected, put it into best fitting storage
		{
			string soundEvent = SCR_SoundEvent.SOUND_EQUIP;
			//TryInsertItem( pItem, EStoragePurpose.PURPOSE_DEPOSIT);	// works for the owned storages ( not for the vicinity storages )
			if ( !TryInsertItem( pItem, EStoragePurpose.PURPOSE_WEAPON_PROXY, cb ) )
			{
				if ( !TryInsertItem( pItem, EStoragePurpose.PURPOSE_EQUIPMENT_ATTACHMENT, cb ) )
				{
					if ( !TryInsertItem( pItem, EStoragePurpose.PURPOSE_DEPOSIT, cb ) )
					{
						if ( !TryMoveItemToStorage( pItem, FindStorageForItem( pItem, EStoragePurpose.PURPOSE_ANY ), -1, cb ) )
							canInsert = TryMoveItemToStorage(pItem, m_Storage, -1, cb); 				// clothes from storage in vicinity
						else
							soundEvent = SCR_SoundEvent.SOUND_PICK_UP;	// play pick up sound for everything else
					
						SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INV_HOTKEY_CONFIRM);
					}
					else
						soundEvent = SCR_SoundEvent.SOUND_PICK_UP;
				}
			}
			
			if (canInsert)
				PlayItemSound(pItem, soundEvent);
		}
		else
		{
			if (pStorageTo == m_Storage)
			{
				canInsert = TryReplaceItem( pStorageTo, pItem, 0, cb );
				if (canInsert)
				{
					SetInventoryLocked(false);
					return;
				}
			}
			
			//~ Find a valid storage to insert item in
			BaseInventoryStorageComponent validStorage = FindStorageForInsert(pItem, pStorageTo, EStoragePurpose.PURPOSE_EQUIPMENT_ATTACHMENT);
			
			if (!validStorage)
				validStorage = FindStorageForInsert(pItem, pStorageTo, EStoragePurpose.PURPOSE_ANY);
			
			if (validStorage)
			{
				pStorageTo = validStorage;
			}
			//~ Check if item can be inserted in linked storages
			else 
			{
				//~ Find valid storage in linked storages
				SCR_UniversalInventoryStorageComponent universalStorage = SCR_UniversalInventoryStorageComponent.Cast(pStorageTo);
				if (universalStorage)
				{
					array<BaseInventoryStorageComponent> linkedStorages = {};
					universalStorage.GetLinkedStorages(linkedStorages);
					
					foreach(BaseInventoryStorageComponent linkedStorage : linkedStorages)
					{
						//~ Valid linked storage found
						if (FindStorageForInsert(pItem, linkedStorage, EStoragePurpose.PURPOSE_ANY))
						{
							pStorageTo = linkedStorage;
							break;
						}
					}
				}
			}
			
			if ( !pStorageFrom )
				canInsert = TryInsertItemInStorage( pItem, pStorageTo, -1, cb );	// if we move item from ground to opened storage
			else
				canInsert = TryMoveItemToStorage( pItem, pStorageTo, -1, cb );		// if we move item between storages
		}
		
		if (!canInsert)
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INV_DROP_ERROR);
		else
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INV_CONTAINER_DIFR_DROP);
		
		if (m_CharacterController && canInsert && !pStorageFrom)
			m_CharacterController.TryPlayItemGesture(EItemGesture.EItemGesturePickUp);

		SetInventoryLocked(false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Locks the inventory for the duration of the item removal process
	//! \param[in] pItem
	//! \param[in] storage
	//! \param[in] cb
	//! \return
	bool TryRemoveItemFromInventory(IEntity pItem, BaseInventoryStorageComponent storage = null, InventoryOperationCallback cb = null)
	{
		if (!CanMoveItem(pItem))
			return false;

		if (!storage)
		{
			InventoryItemComponent itemComp = InventoryItemComponent.Cast(pItem.FindComponent(InventoryItemComponent));
			if (!itemComp)
				return false;
			
			InventoryStorageSlot parentSlot = itemComp.GetParentSlot();
			if (parentSlot)
				storage = parentSlot.GetStorage();
		}
		
		SetInventoryLocked(true);
		bool result = TryRemoveItemFromStorage(pItem, storage, cb);
		SetInventoryLocked(false);

		return result;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Checks whether it is possible to move the item
	//! \param[in] item
	//! \return
	bool CanMoveItem(IEntity item)
	{
		if (!item || item.IsDeleted() || !IsAnimationReady() || IsInventoryLocked())
			return false;
		
		return true;
	}
	
//	//------------------------------------------------------------------------------------------------
//	override bool TryInsertItemInStorageScr( IEntity pItem, BaseInventoryStorageComponent pStorageTo, int slotID = -1, InventoryOperationCallback cb = null )
//	{
//
//		if ( !pStorageTo )
//			return;
//
//		array<BaseInventoryStorageComponent> pStorages = {};
//		pStorage.GetOwnedStorages( pStorages, 1, false );	// get all the storages, the storage has attached to it
//		pStorages.Insert( pStorage );						// and put there also the storage
//		foreach ( BaseInventoryStorageComponent tmpStorage : pStorages )
//		{
//			if ( MoveOperation( itemComponent, tmpStorage ) )
//			{
//				bRet = true;
//				break;
//			}
//		}
//	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] pOwnerEntity
	//! \param[in] pStorageTo
	//! \param[in] cb
	//! \return
	bool TrySwapItems( IEntity pOwnerEntity, BaseInventoryStorageComponent pStorageTo, SCR_InvCallBack cb = null )
	{
		if ( !pStorageTo )
			return false;
						
		InventoryStorageSlot slot =  pStorageTo.FindSuitableSlotForItem( pOwnerEntity );
		if ( !slot )
			return false;

		if ( slot.GetAttachedEntity() )
		{
			if (!TrySwapItemStorages( pOwnerEntity, slot.GetAttachedEntity(), cb ))
			{
				CharacterHandWeaponSlotComponent handWeaponSlot = CharacterHandWeaponSlotComponent.Cast(slot.GetParentContainer());
				//Move 
				if (handWeaponSlot)
				{
					if (TryRemoveItemFromInventory(slot.GetAttachedEntity()))
					{
						TryInsertItem(pOwnerEntity, EStoragePurpose.PURPOSE_ANY, cb);
						return true;
					}
				}
				
				return false;
			}
			
			return true;
		}
		else
		{
			return TryMoveItemToStorage( pOwnerEntity, pStorageTo, slot.GetID(), cb );
		}

		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Try to equip the item into the slot (weapon)
	//! \param[in] pOwnerEntity
	//! \param[in] cb
	//! \param[in] bFromVicinity
	bool EquipWeapon( IEntity pOwnerEntity, SCR_InvCallBack cb = null, bool bFromVicinity = true )
	{
		if ( !bFromVicinity )
		{
			bool result = TrySwapItems(pOwnerEntity, m_Storage.GetWeaponStorage(), cb);
			if (result)
				SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INV_CONTAINER_DIFR_DROP);
			else
				SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INV_DROP_ERROR);

			return result;
		}
		
		IEntity user = GetOwner();
		if (!user)
			return false;
		
		BaseWeaponManagerComponent weaponManager = BaseWeaponManagerComponent.Cast(user.FindComponent(BaseWeaponManagerComponent));
		if (!weaponManager)
			return false;
	
		WeaponSlotComponent slot = weaponManager.GetCurrentSlot();
		int preferred = 0;
		if ( slot )
			preferred = slot.GetWeaponSlotIndex();

		bool result = EquipAny(m_Storage.GetWeaponStorage(), pOwnerEntity, preferred, cb);
		if (!result)
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INV_DROP_ERROR);
		
		if (cb && cb.m_pStorageFrom != cb.m_pStorageTo)
				SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INV_CONTAINER_DIFR_DROP);
			else
				SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INV_CONTAINER_SAME_DROP);
		
		// Play sound
		PlayItemSound(pOwnerEntity, SCR_SoundEvent.SOUND_EQUIP);
		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Try to equip the item into the slot (weapon)
	//! \param[in] pOwnerEntity
	//! \param[in] pUserEntity
	//! \param[in] cb
	void EquipWeaponAttachment( IEntity pOwnerEntity, IEntity pUserEntity, SCR_InvCallBack cb = null )
	{
		BaseWeaponManagerComponent weaponManager = BaseWeaponManagerComponent.Cast(pUserEntity.FindComponent(BaseWeaponManagerComponent));
		if (!weaponManager)
			return;

		WeaponSlotComponent slot = weaponManager.GetCurrentSlot();
		if (!slot)
			return;

		IEntity weaponEntity = slot.GetWeaponEntity();
		if (!weaponEntity)
			return;

		BaseInventoryStorageComponent storage = BaseInventoryStorageComponent.Cast(weaponEntity.FindComponent(BaseInventoryStorageComponent));
		if (!storage)
			return;
		
		EquipAny(storage, pOwnerEntity, -1, cb );
		
		// Play sound
		PlayItemSound(pOwnerEntity, SCR_SoundEvent.SOUND_EQUIP);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Try to equip the item into the slot (gadget)
	//! \param[in] pOwnerEntity
	//! \param[in] cb
	void EquipGadget( IEntity pOwnerEntity, SCR_InvCallBack cb = null )
	{
		//(kamil) the gadget slots are now present directly on individual clothing items - will have to revise logic here if swapping is wanted
		BaseInventoryStorageComponent storageComp = FindStorageForItem(pOwnerEntity, EStoragePurpose.PURPOSE_EQUIPMENT_ATTACHMENT);
		if (storageComp)
		{
			EquipAny( storageComp, pOwnerEntity, -1, cb );
			
			// Play sound
			PlayItemSound(pOwnerEntity, SCR_SoundEvent.SOUND_EQUIP);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Try to equip the item into the slot (cloth)
	//! \param[in] pOwnerEntity
	void EquipCloth( IEntity pOwnerEntity )
	{
		// m_pStorage because character storage is inherited from SCR_EquipedLoadoutStorageComponent
		EquipAny(m_Storage, pOwnerEntity);
		
		// Play sound
		PlayItemSound(pOwnerEntity, SCR_SoundEvent.SOUND_EQUIP);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Try to equip the item into the storage at provided slot
	//! \param[in] storage
	//! \param[in] item
	//! \param[in] preferred
	//! \param[in] cb
	//! \return
	bool EquipAny(BaseInventoryStorageComponent storage, IEntity item, int preferred = -1, SCR_InvCallBack cb = null)
	{
		if (!storage || !item)
			return false;
		InventoryItemComponent itemComp = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
		if (!itemComp)
			return false;
		
		InventoryStorageSlot m_TargetSlot = storage.FindSuitableSlotForItem(item);
		// Storage doesn't have suitable slot for item (therefore any future opearation would fail)
		if (!m_TargetSlot)
			return false;

		if (preferred > 0 && preferred < storage.GetSlotsCount())
			m_TargetSlot = storage.GetSlot(preferred);
		else
			preferred = m_TargetSlot.GetID();

		InventoryStorageSlot sourceSlot = itemComp.GetParentSlot();
		// Item is on the ground as it does not belong to any storage (eg is not in the slot)
		if (!sourceSlot || !sourceSlot.GetStorage())
		{
			// we are picking up item from ground
			// if target slot is not empty return the result of replace operation
			if (m_TargetSlot.GetAttachedEntity())
				return TryReplaceItem(item, storage, preferred, cb);

			// we are picking up item from ground into empty slot, simply return result of the insert operation
			return TryInsertItemInStorage(item, storage, preferred, cb);
		}

		// our target slot is empty and we moving item from another storage
		// simply return the result of move operation
		if (!m_TargetSlot.GetAttachedEntity())
			return TryMoveItemToStorage(item, storage, preferred, cb);

		BaseInventoryStorageComponent sourceStorage = sourceSlot.GetStorage();
		bool isTakenFromArsenal = sourceStorage.GetOwner().FindComponent(SCR_ArsenalComponent);
		bool isTakenFromBody = false;
		
		ChimeraCharacter lootedBodyCharacter = ChimeraCharacter.Cast(sourceStorage.GetOwner());
		DamageManagerComponent lootedBodyDamageManager;
		
		if (lootedBodyCharacter)
			lootedBodyDamageManager = lootedBodyCharacter.GetDamageManager();
		
		if (lootedBodyDamageManager)
			isTakenFromBody = lootedBodyDamageManager.GetState() == EDamageState.DESTROYED;
		
		bool performDropOfOriginalItem = isTakenFromBody || isTakenFromArsenal || sourceStorage.GetOwner().FindComponent(SCR_CampaignArmoryStorageComponent) ;
		
		// If we don't want to drop item
		if (performDropOfOriginalItem)
		{
			// if we want to drop originally equipped item
			// here sequence would be as follows:
			// 1 - drop original item
			// 2 - insert item to target storage
			// 3 - try move as many items as possible from dropped item back to inventory
			// 4 - delete dropped item
	
			// At first - let's validate if this is even possible
			if (!CanSwapItemStorages(item, m_TargetSlot.GetAttachedEntity()))
				return false;
	
			DropAndMoveOperationCallback chainedCallback = new DropAndMoveOperationCallback();
			chainedCallback.m_Manager = this;
			chainedCallback.m_ItemAfter = itemComp;
			chainedCallback.m_ItemBefore = InventoryItemComponent.Cast(m_TargetSlot.GetAttachedEntity().FindComponent(InventoryItemComponent));
			chainedCallback.m_TargetSlot = m_TargetSlot;
			chainedCallback.m_FinalCB = cb;
			chainedCallback.m_bIstakenFromArsenal = isTakenFromArsenal;
	
			return TryRemoveItemFromStorage(m_TargetSlot.GetAttachedEntity(), m_TargetSlot.GetStorage(), chainedCallback);
		}
		
		// we return the result of swap opoeration were item from slotA will be transfered to slotB and item from slotB to slotA
		return TrySwapItemStorages(item, m_TargetSlot.GetAttachedEntity(), cb);
	} 

	//------------------------------------------------------------------------------------------------
	//! Method that attempts to assign to the quick slot a different instance of the same prefab or a different item with the same common item type
	//! \param[in] currentItem Item that is going to be replaced.
	//! \param[in] quickSlotId Slot ID which is going to be used for choosing to which slot it should be assigned. If not provided, the game will try to find in which slot the provided item is.
	//! \param[in] mustBeSamePrefab If the found item can be a different prefab of the same ECommonItemType. Even when this is true, the game will first try to find the same prefab.
	//! \param[in] equipCallback Provide one if you would like to wait for this, e.g., until item transfer is finished.
	//! \param[in] equipNewItem If you want to equip the found item.
	//! \return true if script was able to find a fitting item, otherwise false.
	bool TryAssigningNextItemToQuickSlot(notnull IEntity currentItem, int quickSlotId = -1, bool mustBeSamePrefab = true, out SCR_InvEquipAnyItemCB equipCallback = null, bool equipNewItem = false)
	{
		if (quickSlotId < 0)
		{
			quickSlotId = m_Storage.GetEntityIndexInQuickslots(currentItem);
			if (quickSlotId < 0)
				return false;
		}

		SCR_ResourceNamePredicate specificItemSearch = new SCR_ResourceNamePredicate(excludedItem: currentItem);
		specificItemSearch.m_PrefabContainer = currentItem.GetPrefabData().GetPrefab();
		array<IEntity> foundItems = {};
		if (FindItems(foundItems, specificItemSearch) <= 0)
		{
			if (mustBeSamePrefab)
				return false;

			foundItems.Clear();
			InventoryItemComponent iic = InventoryItemComponent.Cast(currentItem.FindComponent(InventoryItemComponent));
			if (!iic)
				return false;

			ItemAttributeCollection attributes = iic.GetAttributes();
			if (!attributes)
				return false;

			ECommonItemType itemType = attributes.GetCommonType();
			if (itemType == ECommonItemType.NONE)
				return false;

			SCR_CommonItemTypeSearchPredicate itemTypeSearch = new SCR_CommonItemTypeSearchPredicate(itemType, currentItem);
			if (FindItems(foundItems, itemTypeSearch) <= 0)
				return false;
		}

		IEntity selectedItem = foundItems[0];
		if (equipCallback)
		{
			equipCallback.m_pItem = selectedItem;
			equipCallback.m_pStorageToPickUp = m_Storage;
			equipCallback.m_iSlotToFocus = quickSlotId;
			equipCallback.m_bShouldEquip = equipNewItem;
			return true;
		}

		m_Storage.RemoveItemFromQuickSlotAtIndex(quickSlotId);
		m_Storage.StoreItemToQuickSlot(selectedItem, quickSlotId);
		SCR_WeaponSwitchingBaseUI.HighlightQuickSlot(quickSlotId);
		if (equipNewItem)
			m_Storage.UseItem(selectedItem);
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Try to drop the original item and replace it by itemToReplace at the slot specified by slotID
	//! \param[in] storage
	//! \param[in] item
	//! \param[in] slotID
	//! \param[in] cb
	//! \param[in] isTakenFromArsenal
	//! \param[in] deleteOriginalItemIfEmpty
	//! \return
	bool TryReplaceAndDropItemAtSlot(BaseInventoryStorageComponent storage, IEntity item, int slotID, SCR_InvCallBack cb = null, bool isTakenFromArsenal = false)
	{
		if (!storage || !item)
			return false;
		InventoryItemComponent itemComp = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
		if (!itemComp)
			return false;
		
		InventoryStorageSlot m_TargetSlot = storage.GetSlot(slotID);
		// Storage doesn't have suitable slot for item (therefore any future opearation would fail)
		if (!m_TargetSlot)
			return false;		

		InventoryStorageSlot sourceSlot = itemComp.GetParentSlot();
		// Item is on the ground as it does not belong to any storage (eg is not in the slot)
		if (!sourceSlot || !sourceSlot.GetStorage())
		{
			// we are picking up item from ground
			// if target slot is not empty return the result of replace operation
			if (m_TargetSlot.GetAttachedEntity())
				return TryReplaceItem(item, storage, slotID, cb);
			// we are picking up item from ground into empty slot, simply return result of the insert operation
			return TryInsertItemInStorage(item, storage, slotID, cb);
		}

		// our target slot is empty and we moving item from another storage
		// simply return the result of move operation
		if (!m_TargetSlot.GetAttachedEntity())
			return TryMoveItemToStorage(item, storage, slotID, cb);

		// if we want to drop originally equipped item
		// here sequence would be as follows:
		// 1 - drop original item
		// 2 - insert item to target storage
		// 3 - try move as many items as possible from dropped item back to inventory
		// 4 - delete dropped item

		// At first - let's validate if this is even possible
		if (!CanSwapItemStorages(item, m_TargetSlot.GetAttachedEntity()))
			return false;

		DropAndMoveOperationCallback chainedCallback = new DropAndMoveOperationCallback();
		chainedCallback.m_Manager = this;
		chainedCallback.m_ItemAfter = itemComp;
		chainedCallback.m_ItemBefore = InventoryItemComponent.Cast(m_TargetSlot.GetAttachedEntity().FindComponent(InventoryItemComponent));
		chainedCallback.m_TargetSlot = m_TargetSlot;
		chainedCallback.m_bIstakenFromArsenal = isTakenFromArsenal;
		chainedCallback.m_FinalCB = cb;
		
		return TryRemoveItemFromStorage(m_TargetSlot.GetAttachedEntity(), m_TargetSlot.GetStorage(), chainedCallback);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Try to replace at preferred slot
	//! \param[in] storage
	//! \param[in] item
	//! \param[in] preferred
	//! \param[in] cb
	//! \return
	bool TryReplaceItem( BaseInventoryStorageComponent storage, IEntity item, int preferred, SCR_InvCallBack cb )
	{
		int slotCount = storage.GetSlotsCount();
		
		for ( int i = 0; i < slotCount; i++ )
		{
			int j = ( i + preferred ) % slotCount;
			if ( CanReplaceItem( item, storage, j ) )
			{
				if ( TryReplaceItem( item, storage, j, cb ) )
					return true;
			}
		}

		return false;
	} 
	
	//------------------------------------------------------------------------------------------------
	//! Try to equip the item into the slot (weapon)
	//! \param[in] weaponStorage
	//! \param[in] weapon
	void EquipItem( EquipedWeaponStorageComponent weaponStorage, IEntity weapon )
	{
		// There are empty suitable slots at weapon storage
		if ( CanInsertItemInStorage( weapon, weaponStorage, -1) )
		{
			TryInsertItemInStorage( weapon, weaponStorage, -1 );
			return;
		}

		// Otherwise try replace weapon at suitable slot
		BaseWeaponManagerComponent weaponManager = BaseWeaponManagerComponent.Cast( GetOwner().FindComponent(BaseWeaponManagerComponent) );
		if (!weaponManager)
			return;

		WeaponSlotComponent slot = weaponManager.GetCurrentSlot();
		int slotCount = weaponStorage.GetSlotsCount();
		int preferred = 0;
		if ( slot )
			preferred = slot.GetWeaponSlotIndex();

		for ( int i = 0; i < slotCount; i++ )
		{
			int j = ( i + preferred ) % slotCount;
			if ( CanReplaceItem( weapon, weaponStorage, j ) )
			{
				TryReplaceItem( weapon, weaponStorage, j );
				return;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get if resupply magazines action is available with the given magazine count.
	//! Note it does not check if inventory has free space
	//! \param[in] resupplyMagazineCount How many magazines will be resupplied
	//! \param[out] resupplyUnavailableReason The reason why the Resupplied failed if it failed
	//! \param[in] muzzleType (optional) If you want the Resupply to only check specific muzzle types. -1 to Ignore
	//! \param[in] mustBeInStorage (Optional) If storage is given than it will check if the item you try to resupply is in the storage. Used in tadum with Arsenal to give player magazines that are in that arsenal.
	//! \return true if resupply is available.
	bool IsResupplyMagazinesAvailable(int resupplyMagazineCount = 4, out EResupplyUnavailableReason resupplyUnavailableReason = EResupplyUnavailableReason.NONE, EMuzzleType muzzleType = -1, InventoryStorageManagerComponent mustBeInStorage = null)
	{
		BaseWeaponManagerComponent weaponsManager = BaseWeaponManagerComponent.Cast(GetOwner().FindComponent(BaseWeaponManagerComponent));
		if (!weaponsManager)
			return false;

		array<IEntity> weaponList = {};
		weaponsManager.GetWeaponsList(weaponList);

		bool foundValidWeapon = false;
		ResourceName magazineOrProjectilePrefab;
		IEntity spawnedMagazine;

		BaseWeaponComponent comp;
		array<BaseMuzzleComponent> muzzles;
		SCR_MuzzleInMagComponent inMagMuzzle;
		SCR_ArsenalInventoryStorageManagerComponent arsenalStorage;
		foreach (IEntity weapon : weaponList)
		{
			comp = BaseWeaponComponent.Cast(weapon.FindComponent(BaseWeaponComponent));
			if (!comp)
				continue;

			string weaponSlotType = comp.GetWeaponSlotType();

			// Only refill primary and secondary weapons
			if ((weaponSlotType != "primary" && weaponSlotType != "secondary"))
				continue;

			muzzles = {};

			//~ Get base muzzle to only supply magazines
			comp.GetMuzzlesList(muzzles);
			foreach (BaseMuzzleComponent muzzle : muzzles)
			{
				if (muzzleType != -1 && muzzle.GetMuzzleType() != muzzleType)
					continue;

				inMagMuzzle = SCR_MuzzleInMagComponent.Cast(muzzle);
				if (inMagMuzzle && !inMagMuzzle.CanBeReloaded())
					continue;

				magazineOrProjectilePrefab = muzzle.GetDefaultMagazineOrProjectileName();
				if (SCR_StringHelper.IsEmptyOrWhiteSpace(magazineOrProjectilePrefab))
					continue;

				//~ At least one valid weapon was found
				foundValidWeapon = true;

				//~ If storage is given check if magazine or projectile is in storage and only allow resupply if it is (Does not care for amount and intended use is with Arsenal)
				if (mustBeInStorage)
				{
					arsenalStorage = SCR_ArsenalInventoryStorageManagerComponent.Cast(mustBeInStorage);
					if ((arsenalStorage && !arsenalStorage.IsPrefabInArsenalStorage(magazineOrProjectilePrefab)) || (!arsenalStorage && mustBeInStorage.GetDepositItemCountByResource(GetOwner(), magazineOrProjectilePrefab) < 1))
					{
						if (resupplyUnavailableReason < EResupplyUnavailableReason.NOT_IN_GIVEN_STORAGE)
							resupplyUnavailableReason = EResupplyUnavailableReason.NOT_IN_GIVEN_STORAGE;

						continue;
					}
				}

				//~ Check if there are already enough magazines
				if (resupplyMagazineCount - GetMagazineCountByMuzzle(null,muzzle) <= 0)
				{
					if (resupplyUnavailableReason < EResupplyUnavailableReason.ENOUGH_ITEMS)
						resupplyUnavailableReason = EResupplyUnavailableReason.ENOUGH_ITEMS;

					continue;
				}
				
				//~ If it can be stored
				if (!FindStorageForResourceInsert(magazineOrProjectilePrefab, m_Storage))
				{
					if (resupplyUnavailableReason < EResupplyUnavailableReason.INVENTORY_FULL)
						resupplyUnavailableReason = EResupplyUnavailableReason.INVENTORY_FULL;
					
					continue;
				}

				//~ Passes all the checks
				//~ Set reason none just in case as resupply is available
				resupplyUnavailableReason = EResupplyUnavailableReason.NONE;

				//~ All checks passed and at least one magazine can be added so function returns true
				return true;
			}
		}

		//~ Did not have any valid weapons so set unavailable reason to no valid weapon
		if (!foundValidWeapon)
			resupplyUnavailableReason = EResupplyUnavailableReason.NO_VALID_WEAPON;

		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get map of all items (ResourceName) and count to add to inventory on resupply
	//! \param[out] validResupplyItems
	//! \param[in] maxMagazineCount Desired number of magazines
	//! \param[in] muzzleType (optional) If you want to Resupply only check specific muzzle types. -1 to Ignore
	//! \param[in] mustBeInStorage (Optional) If storage is given than it will check if the item you try to check is in the storage. Used in tadum with Arsenal to give player magazines that are in that arsenal.
	void GetValidResupplyItemsAndCount(out notnull map<ResourceName, int> validResupplyItems, int maxMagazineCount = 4, EMuzzleType muzzleType = -1, InventoryStorageManagerComponent mustBeInStorage = null)
	{
		validResupplyItems.Clear();

		BaseWeaponManagerComponent weaponsManager = BaseWeaponManagerComponent.Cast(GetOwner().FindComponent(BaseWeaponManagerComponent));
		if (!weaponsManager)
			return;
		
		array<IEntity> weaponList = {};
		weaponsManager.GetWeaponsList(weaponList);

		BaseWeaponComponent comp;
		array<BaseMuzzleComponent> muzzles;
		SCR_MuzzleInMagComponent inMagMuzzle;
		SCR_ArsenalInventoryStorageManagerComponent arsenalStorage;
		foreach (IEntity weapon : weaponList)
		{
			comp = BaseWeaponComponent.Cast(weapon.FindComponent(BaseWeaponComponent));
			string weaponSlotType = comp.GetWeaponSlotType();

			// Only refill primary and secondary weapons
			if (!(weaponSlotType == "primary" || weaponSlotType == "secondary"))
				continue;

			muzzles = {};

			//~ Get base muzzle to only supply magazines
			comp.GetMuzzlesList(muzzles);
			foreach (BaseMuzzleComponent muzzle : muzzles)
			{
				if (muzzleType != -1 && muzzle.GetMuzzleType() != muzzleType)
					continue;

				inMagMuzzle = SCR_MuzzleInMagComponent.Cast(muzzle);
				if (inMagMuzzle && !inMagMuzzle.CanBeReloaded())
					continue;

				ResourceName magazineOrProjectilePrefab = muzzle.GetDefaultMagazineOrProjectileName();
				if (SCR_StringHelper.IsEmptyOrWhiteSpace(magazineOrProjectilePrefab))
					continue;

				//~ Get current magazine count and see if it needs to be increased
				int resupplyCount = maxMagazineCount - GetMagazineCountByMuzzle(null,muzzle);
				if (resupplyCount <= 0)
					continue;

				//~ If storage is given check if magazine or projectile is in storage and only allow resupply if it is (Does not care for amount and intended use is with Arsenal)
				if (mustBeInStorage)
				{
					arsenalStorage = SCR_ArsenalInventoryStorageManagerComponent.Cast(mustBeInStorage);
					if ((arsenalStorage && !arsenalStorage.IsPrefabInArsenalStorage(magazineOrProjectilePrefab)) || (!arsenalStorage && mustBeInStorage.GetDepositItemCountByResource(GetOwner(), magazineOrProjectilePrefab) < 1))
						continue;
				}

				//~ Add magazine to be resupplied
				if (!validResupplyItems.Contains(magazineOrProjectilePrefab))
					validResupplyItems.Insert(magazineOrProjectilePrefab, resupplyCount);
				//~ Update count till max
				else
					validResupplyItems[magazineOrProjectilePrefab] = Math.Clamp(validResupplyItems[magazineOrProjectilePrefab] + resupplyCount, 0, maxMagazineCount);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Resupply all weapons in map with the given number of magazines.
	//! \param[in] validResupplyItems Map of ResourceNames and amount to add to inventory
	void ResupplyMagazines(notnull map<ResourceName, int> validResupplyItems)
	{
		//~ Nothing to resupply
		if (validResupplyItems.IsEmpty())
			return;
		
		if (!m_ResupplyMagazineCallback)
			m_ResupplyMagazineCallback = new SCR_ResupplyMagazinesCallback(this);

		//~ Resupply each given entry
		foreach (ResourceName itemPrefab, int count : validResupplyItems)
		{
			m_ResupplyMagazineCallback.Insert(itemPrefab, count);
		}

		m_ResupplyMagazineCallback.Start();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Resupply all weapons so they have given number of magazines.
	//! \param[in] maxMagazineCount Desired number of magazines
	//! \param[in] muzzleType (optional) If you want to Resupply only check specific muzzle types. -1 to Ignore
	//! \param[in] mustBeInStorage (Optional) If storage is given than it will check if the item you try to check is in the storage. Used in tadum with Arsenal to give player magazines that are in that arsenal.
	void ResupplyMagazines(int maxMagazineCount = 4, EMuzzleType muzzleType = -1, InventoryStorageManagerComponent mustBeInStorage = null)
	{
		//~ Get resupply prefabs
		map<ResourceName, int> validResupplyItems = new map<ResourceName, int>();
		GetValidResupplyItemsAndCount(validResupplyItems, maxMagazineCount, muzzleType, mustBeInStorage);
		
		//~ Resupply
		ResupplyMagazines(validResupplyItems);
	}

	//------------------------------------------------------------------------------------------------
	//!
	void EndResupplyMagazines()
	{
		delete m_ResupplyMagazineCallback;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check if item is in given storage
	//! \param[in] storage Storage to check
	//! \param[in] item Item to check
	//! \param[in] player verifier
	//! \return True if item is in inventory
	static bool IsItemInStorage(notnull InventoryStorageManagerComponent storage, ResourceName item, IEntity player)
	{
		SCR_ArsenalInventoryStorageManagerComponent arsenalStorage = SCR_ArsenalInventoryStorageManagerComponent.Cast(storage);
		return (arsenalStorage && arsenalStorage.IsPrefabInArsenalStorage(item)) || (!arsenalStorage && storage.GetDepositItemCountByResource(player, item) > 0);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] muzzle
	//! \param[in] maxMagazineCount
	//! \param[in] mustBeInStorage
	//! \param[out] currentMagazineAmount
	//! \return
	EResupplyUnavailableReason CanResupplyMuzzle(IEntity user, notnull BaseMuzzleComponent muzzle, int maxMagazineCount = -1, InventoryStorageManagerComponent mustBeInStorage = null, out int currentMagazineAmount = -1)
	{
		//~ Cannot resupply weapons that cannot be ressupplied like the US rocket Launcer
		SCR_MuzzleInMagComponent inMagMuzzle = SCR_MuzzleInMagComponent.Cast(muzzle);
		if (inMagMuzzle && !inMagMuzzle.CanBeReloaded())
			return EResupplyUnavailableReason.NO_VALID_WEAPON;

		//~ Get default magazine to resupply
		ResourceName magazineToResupply = muzzle.GetDefaultMagazineOrProjectileName();
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(magazineToResupply))
			return EResupplyUnavailableReason.NO_VALID_WEAPON;
		
		//~ Check if it is in arsenal or in the storage
		if (mustBeInStorage && !IsItemInStorage(mustBeInStorage, magazineToResupply, user))
			return EResupplyUnavailableReason.NOT_IN_GIVEN_STORAGE;
		
		//~ Already has enough magazines
		currentMagazineAmount = GetMagazineCountByMuzzle(user,muzzle); 
		if (maxMagazineCount > 0 && currentMagazineAmount >= maxMagazineCount)
			return EResupplyUnavailableReason.ENOUGH_ITEMS;
		
		//~ Check if there is space in the inventory
		if (!FindStorageForResourceInsert(magazineToResupply, m_Storage))
			return EResupplyUnavailableReason.INVENTORY_FULL;
				
		return EResupplyUnavailableReason.RESUPPLY_VALID;	
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] itemToResupply
	//! \param[in] maxItemCount
	//! \param[in] mustBeInStorage
	//! \param[out] currentItemAmount
	//! \return
	EResupplyUnavailableReason CanResupplyItem(IEntity user, ResourceName itemToResupply, int maxItemCount = -1, InventoryStorageManagerComponent mustBeInStorage = null, out int currentItemAmount = -1)
	{
		//~ Check if it is in arsenal or in the storage
		if (mustBeInStorage && !IsItemInStorage(mustBeInStorage, itemToResupply, user))
			return EResupplyUnavailableReason.NOT_IN_GIVEN_STORAGE;
		
		//~ Already has enough of the item in storage
		currentItemAmount = GetDepositItemCountByResource(user, itemToResupply); 
		if (maxItemCount > 0 && currentItemAmount >= maxItemCount)
			return EResupplyUnavailableReason.ENOUGH_ITEMS;
			
		//~ Check if there is space in the inventory
		if (!FindStorageForResourceInsert(itemToResupply, m_Storage))
			return EResupplyUnavailableReason.INVENTORY_FULL;
	
		return EResupplyUnavailableReason.RESUPPLY_VALID;
	}
	
	//------------------------------------------------------------------------------------------------
	//! try to move item from owner's storage to vicinity. It might be moved to ground or to a storage in the vicinity
	//! \param[in] pItem
	//! \param[in] pStorageTo
	void MoveItemToVicinity(IEntity pItem, BaseInventoryStorageComponent pStorageTo = null);
	
	//------------------------------------------------------------------------------------------------
	//!
	void OpenInventory()
	{
		if (m_CharacterController && m_CharacterController.GetLifeState() != ECharacterLifeState.ALIVE)
			return;

		MenuManager menuManager = GetGame().GetMenuManager();
		ChimeraMenuPreset menu = ChimeraMenuPreset.Inventory20Menu;
		
		MenuBase inventoryMenu = menuManager.FindMenuByPreset(menu);
		if (inventoryMenu)
			return;
		
		menuManager.OpenMenu( menu );
		m_OnInventoryOpenInvoker.Invoke(true);
		
		if (!m_CharacterController)
			return;
		
		// Quit ADS
		m_CharacterController.SetWeaponADS(false);
		m_CharacterController.SetGadgetRaisedModeWanted(false);
		
		// Pin grenade
		if (m_CharacterController.GetInputContext() && m_CharacterController.GetInputContext().GetThrow())
			m_CharacterController.SetThrow(false, true);
				
		// Inspection or lowered weapon stance
		m_bWasRaised = m_CharacterController.IsWeaponRaised();
		m_CharacterController.SetWeaponRaised(false);
				
		SCR_AnalyticsApplication.GetInstance().OpenInventory(GetTotalWeightOfAllStorages());
	}

	//------------------------------------------------------------------------------------------------
	//!
	void CloseInventory()
	{
		MenuManager menuManager = GetGame().GetMenuManager();
		if (!menuManager)
			return;
		
		menuManager.CloseMenuByPreset(ChimeraMenuPreset.Inventory20Menu);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	IEntity GetStorageToOpen()
	{
		IEntity result = m_StorageToOpen;
		m_StorageToOpen = null;
		
		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] storage
	void SetStorageToOpen(IEntity storage)
	{
		m_StorageToOpen = storage;
	}

	//------------------------------------------------------------------------------------------------
	//!
	void Action_OpenInventory()
	{
		CompartmentAccessComponent cac = m_CharacterController.GetCharacter().GetCompartmentAccessComponent();
		if (cac && cac.IsInCompartment())
		{
			IEntity owner = cac.GetCompartment().GetOwner();
			while (owner)
			{
				UniversalInventoryStorageComponent comp = UniversalInventoryStorageComponent.Cast(owner.FindComponent(UniversalInventoryStorageComponent));
				if (comp)
				{
					SetStorageToOpen(owner);
					break;
				}

				owner = owner.GetParent();
			}
		}

		if (cac && (cac.IsGettingIn() || cac.IsGettingOut()))
			return;

		OpenInventory();
	}
	
	//------------------------------------------------------------------------------------------------
	void OnInventoryMenuClosed()
	{
		m_OnInventoryOpenInvoker.Invoke(false);
		
		// Revert inspection or lowered weapon stance
		if (m_CharacterController)
			m_CharacterController.SetWeaponRaised(m_bWasRaised);		
		
		SCR_AnalyticsApplication.GetInstance().CloseInventory(GetTotalWeightOfAllStorages());
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnStorageAdded(BaseInventoryStorageComponent storage) 
	{ 
		// do nothing
 	}
	
//	//------------------------------------------------------------------------------------------------
//	//! Even after physics update
//	//! \param[in] owner The owner entity
//	//! \param[in] frameNumber Time passed since last frame
//	override void EOnPostFrame(IEntity owner, float timeSlice)
//	{
//		m_OnPostFrameInvoker.Invoke(timeSlice);
//	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void DebugListAllItemsInInventory()
	{
		array<IEntity> items = {};
		GetItems(items);
		Print("INV: no item", LogLevel.NORMAL);
		InventoryItemComponent pInvComp;
		ItemAttributeCollection attribs;
		foreach (IEntity item : items)
		{
			pInvComp = InventoryItemComponent.Cast(  item .FindComponent( InventoryItemComponent ) );
			if( pInvComp )
			{
				attribs = pInvComp.GetAttributes();
				if( !attribs )
					break;

				string sName = attribs.GetUIInfo().GetName();
				Print("INV: " + sName, LogLevel.NORMAL);

			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Enable/disable post frame event
	void EnablePostFrame(bool enable)
	{
		if (enable)
			SetEventMask(GetOwner(), EntityEvent.POSTFRAME);
		else
			ClearEventMask(GetOwner(), EntityEvent.POSTFRAME);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	int GetHealthComponentCount()
	{
		return m_iHealthEquipment;
	}
	
	//------------------------------------------------------------------------------------------------	
	//!
	IEntity GetBandageItem()
	{	
		return FindItem(m_BandagePredicate, EStoragePurpose.PURPOSE_DEPOSIT);
	}
	
	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_InventoryStorageManagerComponent()
	{
		m_ERetCode = EInventoryRetCode.RETCODE_DEFAULT_STATE;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool IsAnimationReady()
	{
		if (m_CharacterController)
			return m_CharacterController.CanPlayItemGesture() || m_CharacterController.IsPlayingItemGesture();
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool IsInventoryLocked()
	{
		return m_bIsInventoryLocked;	
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] isLocked
	void SetInventoryLocked(bool isLocked)
	{
		m_bIsInventoryLocked = isLocked;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in,out] items
	//! \param[in] storage
	//! \return
	int GetAllItems(inout array<IEntity> items, BaseInventoryStorageComponent storage)
	{
		if (!storage || !items)
			return 0;
			
		int count = 0;
		
		if (!ClothNodeStorageComponent.Cast(storage))
			count = storage.GetAll(items);

		array<BaseInventoryStorageComponent> itemToReplaceAttachedStorages = {};
		storage.GetOwnedStorages(itemToReplaceAttachedStorages, 1, false);
		foreach (BaseInventoryStorageComponent attachedStorage : itemToReplaceAttachedStorages)
		{			
			if (ClothNodeStorageComponent.Cast(attachedStorage))
				attachedStorage.GetOwnedStorages(itemToReplaceAttachedStorages, 1, false);
			else
				count += attachedStorage.GetAll(items);
		}
			
		return count;
	}
	
	//------------------------------------------------------------------------------------------------	
	//! Get available prefabs of weapons of specified types
	//! \param[in] weapons
	//! \param[in] weaponType
	//! \param[out] prefabs
	void GetWeaponPrefabsOfType(notnull array<IEntity> weapons, EWeaponType weaponType, notnull out array<EntityPrefabData> prefabs)
	{
		BaseWeaponComponent weapon;
		EntityPrefabData prefabData;
		foreach (IEntity item : weapons)
		{
			weapon = BaseWeaponComponent.Cast(item.FindComponent(BaseWeaponComponent));
			if (weapon.GetWeaponType() != weaponType)
				continue;
			
			// Ignore currently selected items
			prefabData = item.GetPrefabData();
			if (prefabData && !prefabs.Contains(prefabData))
				prefabs.Insert(prefabData);
		}
	}
	
	//------------------------------------------------------------------------------------------------	
	//! Find next weapon of specified types, excluding currentItem, but including it in sorting
	//! \param[in] weaponType
	//! \param[in] currentItem
	//! \param[in] allowCurrentPrefab
	//! \return
	IEntity FindNextWeaponOfType(EWeaponType weaponType, IEntity currentItem = null, bool allowCurrentPrefab = false)
	{
		array<EntityPrefabData> prefabs = {};
		
		// Currently selected weapon may be outside of inventory and it has to be considered for sorting
		EntityPrefabData currentPrefab;
		BaseWeaponComponent currentWeapon;
		if (currentItem)
		{
			currentPrefab = currentItem.GetPrefabData();
			currentWeapon = BaseWeaponComponent.Cast(currentItem.FindComponent(BaseWeaponComponent));
			
			if (currentPrefab && currentWeapon.GetWeaponType() == weaponType)
				prefabs.Insert(currentPrefab);
		}
		
		// Collect all the matching prefabs
		array<IEntity> items = {};
		FindItemsWithComponents(items, {BaseWeaponComponent});
		GetWeaponPrefabsOfType(items, weaponType, prefabs);
		
		// No valid prefabs
		if (prefabs.IsEmpty())
			return null;
		
		// TODO: better sorting, perhaps by name
		prefabs.Sort();
		
//		foreach (EntityPrefabData prefab : prefabs)
//		{
//			Print(prefab.GetPrefabName(), LogLevel.WARNING);
//		}
		
		// Select next prefab
		int nextPrefabID = (prefabs.Find(currentPrefab) + 1) % prefabs.Count();
		EntityPrefabData nextPrefab = prefabs[nextPrefabID];
		
		// Return nothing if prefab is unchanged
		if (!allowCurrentPrefab && nextPrefab == currentPrefab)
			return null;
		
		// Select the weapon that matches the type to be selected
		foreach (IEntity item : items)
		{
			if (item && item.GetPrefabData() == nextPrefab)
				return item;
		}
		
		return null;
	}

#else

	//------------------------------------------------------------------------------------------------
	//! \param[in] ERetCode
	void SetReturnCode( EInventoryRetCode ERetCode ) ;

	//------------------------------------------------------------------------------------------------
	void SetReturnCodeDefault() ;

	//------------------------------------------------------------------------------------------------
	//! \return
	EInventoryRetCode GetReturnCode()
	{
		return m_ERetCode;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetTotalWeightOfAllStorages();

	//------------------------------------------------------------------------------------------------
	//! \param[in] pOwner
	void SetLootStorage( IEntity pOwner );

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] pItem
	void InsertItem( IEntity pItem );

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] pOwnerEntity
	//! \param[in] cb
	//! \param[in] bFromVicinity
	void EquipWeapon( IEntity pOwnerEntity, SCR_InvCallBack cb = null, bool bFromVicinity = true );

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] pOwnerEntity
	void EquipGadget( IEntity pOwnerEntity );

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] pOwnerEntity
	void EquipCloth( IEntity pOwnerEntity );

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] storage
	//! \param[in] item
	//! \param[in] preferred
	void EquipAny(BaseInventoryStorageComponent storage, IEntity item, int preferred = 0);

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] weaponStorage
	//! \param[in] weapon
	void EquipItem( EquipedWeaponStorageComponent weaponStorage, IEntity weapon );

	//------------------------------------------------------------------------------------------------
	//!
	void OpenInventory();

	//------------------------------------------------------------------------------------------------
	//!
	void Action_OpenInventory();

	//------------------------------------------------------------------------------------------------
	override void OnStorageAdded(BaseInventoryStorageComponent storage);

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] enable
	void EnablePostFrame(bool enable);

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetHealthComponentCount();

	//------------------------------------------------------------------------------------------------
	//! \return
	IEntity GetBandageItem();
	
#endif	
	
	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_InventoryStorageManagerComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		#ifndef DISABLE_INVENTORY
		
		//ChimeraCharacter pChimeraChar = ChimeraCharacter.Cast( ent );
		//pChimeraChar.s_OnCharacterCreated.Insert( DebugListAllItemsInInventory );
		
		m_Storage = SCR_CharacterInventoryStorageComponent.Cast(ent.FindComponent(CharacterInventoryStorageComponent));
		m_CharacterController = SCR_CharacterControllerComponent.Cast(ent.FindComponent(SCR_CharacterControllerComponent));
		#endif
	}
}
