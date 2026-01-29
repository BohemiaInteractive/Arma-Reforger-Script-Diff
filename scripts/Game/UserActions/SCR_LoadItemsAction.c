class SCR_LoadItemsAction : SCR_ScriptedUserAction
{
	protected SCR_FilteredInventoryStorageComponentClass m_Data;
	protected SCR_InventoryStorageManagerComponent m_PlayerStorageManager;
	protected BaseInventoryStorageComponent m_TargetStorage;
	protected IEntity m_ItemForTransfer;
	protected IEntity m_TransferredItem;

	protected const string REASON_CANNOT_FIT = "#AR-UserAction_LoadItems_NoSpace";
	protected const string REASON_NO_ITEMS = "#AR-UserAction_LoadItems_NoItems";

	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		SCR_FilteredInventoryStorageComponent storageComp = SCR_FilteredInventoryStorageComponent.Cast(pOwnerEntity.FindComponent(SCR_FilteredInventoryStorageComponent));
		if (!storageComp)
			return;

		m_Data = SCR_FilteredInventoryStorageComponentClass.Cast(storageComp.GetComponentData(pOwnerEntity));
		m_TargetStorage = BaseInventoryStorageComponent.Cast(pOwnerEntity.FindComponent(BaseInventoryStorageComponent));
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (!m_Data || m_Data.GetNumberOfAllowedTypes() < 1)
			return false;

		if (!m_TargetStorage)
			return false;

		return super.CanBeShownScript(user);
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		if (m_ItemForTransfer && m_ItemForTransfer.GetRootParent() == user)
		{
			if (m_PlayerStorageManager && !m_PlayerStorageManager.CanMoveItemToStorage(m_ItemForTransfer, m_TargetStorage))
			{
				SetCannotPerformReason(REASON_CANNOT_FIT);
				return false;
			}

			return true;
		}

		ChimeraCharacter character = ChimeraCharacter.Cast(user);
		if (!user)
			return false;

		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return false;

		m_PlayerStorageManager = SCR_InventoryStorageManagerComponent.Cast(controller.GetInventoryStorageManager());
		if (!m_PlayerStorageManager)
			return false;

		array<IEntity> ownedItems = {};
		SCR_CommonItemTypeSearchPredicate itemSearch = new SCR_CommonItemTypeSearchPredicate(0, m_TransferredItem);
		foreach (ECommonItemType allowedType : m_Data.GetAllowedItemTypes())
		{
			ownedItems.Clear();
			itemSearch.m_eItemType = allowedType;
			m_PlayerStorageManager.FindItems(ownedItems, itemSearch);
			if (ownedItems.IsEmpty())
				continue;

			m_ItemForTransfer = ownedItems[0];
			break;
		}

		SetCannotPerformReason(REASON_NO_ITEMS);
		return false;
	}


	//------------------------------------------------------------------------------------------------
	// If continues action it will only execute everytime the duration is done
	override void PerformContinuousAction(IEntity pOwnerEntity, IEntity pUserEntity, float timeSlice)
	{
		if (!LoopActionUpdate(timeSlice))
			return; 

		PerformAction(pOwnerEntity, pUserEntity);
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!m_ItemForTransfer || !m_PlayerStorageManager || !m_TargetStorage)
			return;

		m_TransferredItem = m_ItemForTransfer;
		m_PlayerStorageManager.TryMoveItemToStorage(m_ItemForTransfer, m_TargetStorage);
		m_ItemForTransfer = null;

		array<IEntity> ownedItems = {};
		array<ECommonItemType> allowedTypes = m_Data.GetAllowedItemTypes();
		SCR_CommonItemTypeSearchPredicate itemSearch = new SCR_CommonItemTypeSearchPredicate(0, m_TransferredItem);
		foreach (ECommonItemType allowedType : allowedTypes)
		{
			ownedItems.Clear();
			itemSearch.m_eItemType = allowedType;
			m_PlayerStorageManager.FindItems(ownedItems, itemSearch);
			if (ownedItems.IsEmpty())
				continue;

			m_ItemForTransfer = ownedItems[0];
			break;
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnActionCanceled(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		m_ItemForTransfer = null;
		m_TransferredItem = null;
	}

	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return true;
	}
}
