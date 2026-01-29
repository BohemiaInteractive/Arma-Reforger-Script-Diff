//~ Use the inherent versions!
[BaseContainerProps(), BaseContainerCustomStringTitleField("USE INHERITED VERSION ONLY!")]
class SCR_BaseResupplySupportStationAction : SCR_BaseItemHolderSupportStationAction
{
	[Attribute(ESupportStationType.RESUPPLY_AMMO.ToString(), desc: "Which Resupply this action is: Medical or ammo", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(ESupportStationType))]
	protected ESupportStationType m_eSupportStationType;
	
	[Attribute("-1", desc: "How much of the given item (or weapon magazines) can be in the inventory of the character. -1 means it is unlimited")]
	protected int m_iMaxResupplyCount;
	
	[Attribute("#AR-SupportStation_Resupply_ActionInvalid_NoInventorySpace", desc: "Text shown on action if player cannot resupply because inventory is full", uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sInvalidNoInventorySpace;
	
	[Attribute("#AR-SupportStation_Resupply_ActionInvalid_RankTooLow", desc: "Text shown on action if player cannot resupply because their rank is Renegade", uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sInvalidRankTooLow;
	
	[Attribute("#AR-SupportStation_Resupply_ActionInvalid_NotEnoughAvailableAllocatedSupplies", desc: "Text shown on action if player cannot resupply because the player does not have enough Available Allocated Supplies", uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sInvalidNotEnoughAvailableAllocatedSupplies;
	
	[Attribute(ENotification.UNKNOWN.ToString(), desc: "Notification when the action is used. Shown to player using the action if resupply self or on the player that is being resupplied if resupply other. Leave unknown to ignore", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(ENotification))]
	protected ENotification m_eNotificationOnUse; 
	
	protected SCR_ArsenalComponent m_ArsenalComponent;
	
	protected SCR_ArsenalManagerComponent m_ArsenalManager;
	
	protected SCR_InventoryStorageManagerComponent m_InventoryManagerTarget;
	
	protected EResupplyUnavailableReason m_eResupplyUnavailableReason;
	
	protected bool m_bCanResupply;
	protected int m_iCurrentItemAmount = -1;
	
	protected const LocalizedString X_OUTOF_Y_FORMATTING = "#AR-SupportStation_ActionFormat_ItemAmount";
	protected const LocalizedString CURRENT_ITEM_AMOUNT_FORMATTING = "#AR-SupportStation_ActionFormat_CurrentItemAmount";
	
	//------------------------------------------------------------------------------------------------
	protected override ESupportStationType GetSupportStationType()
	{
		return m_eSupportStationType;	
	}
	
	//------------------------------------------------------------------------------------------------
	protected override LocalizedString GetInvalidPerformReasonString(ESupportStationReasonInvalid reasonInvalid)
	{
		if (reasonInvalid == ESupportStationReasonInvalid.RESUPPLY_ENOUGH_ITEMS)
			return WidgetManager.Translate(X_OUTOF_Y_FORMATTING, m_iCurrentItemAmount, m_iMaxResupplyCount);
		else if (reasonInvalid == ESupportStationReasonInvalid.RESUPPLY_INVENTORY_FULL)
			return m_sInvalidNoInventorySpace;
		else if (reasonInvalid == ESupportStationReasonInvalid.RANK_TOO_LOW)
			return m_sInvalidRankTooLow;
		else if (reasonInvalid == ESupportStationReasonInvalid.NOT_ENOUGH_AVAILABLE_ALLOCATED_SUPPLIES)
			return m_sInvalidNotEnoughAvailableAllocatedSupplies;
		
		return super.GetInvalidPerformReasonString(reasonInvalid);
	}

	//------------------------------------------------------------------------------------------------
	//~ Returns true if the action is shown but disabled because of no supplies, max items or inventory full. Otherwise hide it to avoid to many options
	protected bool GetShowButDisabled()
	{
		if (m_eResupplyUnavailableReason == EResupplyUnavailableReason.NOT_IN_GIVEN_STORAGE)
			return false;

		if (m_eResupplyUnavailableReason == EResupplyUnavailableReason.ENOUGH_ITEMS || m_eResupplyUnavailableReason == EResupplyUnavailableReason.INVENTORY_FULL)
			return true;
			
		if (m_eCannotPerformReason == ESupportStationReasonInvalid.NO_SUPPLIES || m_eCannotPerformReason == ESupportStationReasonInvalid.RANK_TOO_LOW || m_eCannotPerformReason == ESupportStationReasonInvalid.NOT_ENOUGH_AVAILABLE_ALLOCATED_SUPPLIES)
			return true;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	ENotification GetNotificationOnUse()
	{
		return m_eNotificationOnUse;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override bool CanHaveMultipleUsers()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{	
		if (!m_ResupplyData)
			return false;
		
		if (m_ArsenalComponent && !m_ArsenalComponent.IsArsenalEnabled())
			return false;
		
		return super.CanBeShownScript(user);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
 	{		
		if (GetItemPrefab().IsEmpty())
		{
			UpdateCanResupply(GetOwner(), user);
			if (GetItemPrefab().IsEmpty())
				return false;
		}
		
		bool canBePerformed = super.CanBePerformedScript(user);
		
		//~ Cannot resupply as resupply action unavailible
		if (canBePerformed && !m_bCanResupply)
		{
			//~ Set reason action is unavailible
			if (m_eResupplyUnavailableReason == EResupplyUnavailableReason.ENOUGH_ITEMS)
				SetCanPerform(false, ESupportStationReasonInvalid.RESUPPLY_ENOUGH_ITEMS);
			else if (m_eResupplyUnavailableReason == EResupplyUnavailableReason.INVENTORY_FULL)
				SetCanPerform(false, ESupportStationReasonInvalid.RESUPPLY_INVENTORY_FULL);
			else if (m_eResupplyUnavailableReason == EResupplyUnavailableReason.NOT_IN_GIVEN_STORAGE)
				SetCanPerform(false, ESupportStationReasonInvalid.RESUPPLY_NOT_IN_STORAGE);
			else if (m_eResupplyUnavailableReason == EResupplyUnavailableReason.NO_VALID_WEAPON)
				SetCanPerform(false, ESupportStationReasonInvalid.RESUPPLY_NO_VALID_WEAPON);
			else if (m_eResupplyUnavailableReason == EResupplyUnavailableReason.RANK_TOO_LOW)
				SetCanPerform(false, ESupportStationReasonInvalid.RANK_TOO_LOW);
			else if (m_eResupplyUnavailableReason == EResupplyUnavailableReason.NOT_ENOUGH_AVAILABLE_ALLOCATED_SUPPLIES)
				SetCanPerform(false, ESupportStationReasonInvalid.NOT_ENOUGH_AVAILABLE_ALLOCATED_SUPPLIES);
			
			return false;
		}
		
		return canBePerformed;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void CanBePerformedUpdate(IEntity user)
	{
		//~ Check if can add to inventory. Only called every x seconds to save performance
		UpdateCanResupply(GetOwner(), user);
		super.CanBePerformedUpdate(user);
	}

	//------------------------------------------------------------------------------------------------
	//~ Can Resupply loops through entire inventory so only call this every x seconds
	protected void UpdateCanResupply(IEntity owner, IEntity user)
	{
		m_bCanResupply = false;
		
		//~ Reset reason
		m_eResupplyUnavailableReason = EResupplyUnavailableReason.NO_VALID_WEAPON;
			
		SetTargetInventory(user, owner);
		if (!m_InventoryManagerTarget)
			return;
	
		BaseMuzzleComponent muzzle;
		if (!m_ResupplyData.GetResupplyItemOrMuzzle(m_InventoryManagerTarget.GetOwner(), owner, m_SupportStationGadget, m_sItemPrefab, muzzle))
			return;
		
		//~ No valid item found
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(m_sItemPrefab))
			return;
		
		//~ Check if resupply is availible (And get current item amount)
		if (muzzle)
			m_eResupplyUnavailableReason = m_InventoryManagerTarget.CanResupplyMuzzle(user, muzzle, m_iMaxResupplyCount, GetProviderInventory(), m_iCurrentItemAmount);
		else
			m_eResupplyUnavailableReason = m_InventoryManagerTarget.CanResupplyItem(user, m_sItemPrefab, m_iMaxResupplyCount, GetProviderInventory(), m_iCurrentItemAmount);
		
		m_bCanResupply = m_eResupplyUnavailableReason == EResupplyUnavailableReason.RESUPPLY_VALID;

		//~ Check for rank if items are ranked locked and disable the action if player is Renegade or doesnt meet rank requirement
		if (!m_bCanResupply || !m_ArsenalManager || !m_ArsenalManager.AreItemsRankLocked())
			return;

		SCR_ECharacterRank characterRank = SCR_CharacterRankComponent.GetCharacterRank(m_InventoryManagerTarget.GetOwner());
		SCR_ResupplyCatalogItemSupportStationData catalogSupportData = SCR_ResupplyCatalogItemSupportStationData.Cast(m_ResupplyData);
		SCR_ResupplyHeldWeaponSupportStationData muzzleSupportData = SCR_ResupplyHeldWeaponSupportStationData.Cast(m_ResupplyData);
		
		if (characterRank <= SCR_ECharacterRank.RENEGADE || (catalogSupportData && characterRank < catalogSupportData.GetRequiredRank(owner, m_SupportStationGadget)))
			m_eResupplyUnavailableReason = EResupplyUnavailableReason.RANK_TOO_LOW;

		if (SCR_ArsenalManagerComponent.IsMilitarySupplyAllocationEnabled())
		{
			PlayerManager playerManager = GetGame().GetPlayerManager();
			int playerId = playerManager.GetPlayerIdFromControlledEntity(user);

			SCR_PlayerController playerController = SCR_PlayerController.Cast(playerManager.GetPlayerController(playerId));
			SCR_PlayerSupplyAllocationComponent playerSupplyAllocationComponent;

			if (playerController)
				playerSupplyAllocationComponent = SCR_PlayerSupplyAllocationComponent.Cast(playerController.FindComponent(SCR_PlayerSupplyAllocationComponent));

			if (muzzle)
			{
				if (playerSupplyAllocationComponent && muzzleSupportData && !playerSupplyAllocationComponent.HasPlayerEnoughAvailableAllocatedSupplies(muzzleSupportData.GetRequiredAvailableAllocatedSupplies(owner, m_sItemPrefab)))
					m_eResupplyUnavailableReason = EResupplyUnavailableReason.NOT_ENOUGH_AVAILABLE_ALLOCATED_SUPPLIES;
			}
			else
			{
				if (playerSupplyAllocationComponent && catalogSupportData && !playerSupplyAllocationComponent.HasPlayerEnoughAvailableAllocatedSupplies(catalogSupportData.GetRequiredAvailableAllocatedSupplies(owner, m_SupportStationGadget)))
					m_eResupplyUnavailableReason = EResupplyUnavailableReason.NOT_ENOUGH_AVAILABLE_ALLOCATED_SUPPLIES;
			}
		}

		m_bCanResupply = m_eResupplyUnavailableReason == EResupplyUnavailableReason.RESUPPLY_VALID;
	}
	
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
 	{
		super.PerformAction(pOwnerEntity, pUserEntity);
		UpdateCanResupply(pOwnerEntity, pUserEntity);
	}	
	
	//------------------------------------------------------------------------------------------------
	protected void SetTargetInventory(IEntity user, IEntity owner)
	{
		return;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_InventoryStorageManagerComponent GetTargetInventory()
	{
		return m_InventoryManagerTarget;
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Get inventory of who ever provide the item. This simply checkes if the item is in the inventory. This should be an arsenal as it does not remove anything from the inventory
	protected InventoryStorageManagerComponent GetProviderInventory()
	{
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void ResetReferencesOnServer()
	{
		m_sItemPrefab = string.Empty;
		super.ResetReferencesOnServer();
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void DelayedInit(IEntity owner)
	{
		if (!owner)
			return;
		
		super.DelayedInit(owner);
		
		m_ArsenalComponent = SCR_ArsenalComponent.FindArsenalComponent(owner);
		
		SCR_ArsenalManagerComponent.GetArsenalManager(m_ArsenalManager);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override string GetActionStringParam()
	{		
		if (!m_bCanPerform)
		{
			if (m_eResupplyUnavailableReason != EResupplyUnavailableReason.INVENTORY_FULL)
				return string.Empty;
		}
		
		if (m_iMaxResupplyCount < 0)
			return WidgetManager.Translate(CURRENT_ITEM_AMOUNT_FORMATTING, m_iCurrentItemAmount);
		
		return WidgetManager.Translate(X_OUTOF_Y_FORMATTING, m_iCurrentItemAmount, m_iMaxResupplyCount);
	}
}

