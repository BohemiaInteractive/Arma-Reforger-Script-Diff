//PlayerSupplyAllocation Widget Handler

class SCR_PlayerSupplyAllocationUI : ScriptedWidgetComponent
{
	//Supplies text widget for showing up maximum amount of supplies Allocation and current supplies amount
	[Attribute("MSARSuppliesText", desc: "Text for displaying supplies related info to player")]	
	protected ResourceName m_sSuppliesText;
	
	//Number of currently available allocated supplies
	[Attribute("0", desc: "Available supplies Count")]	
	protected int m_iAvailableSuppliesAmount;
	
	//Current allocated supplies limit
	[Attribute("0", desc: "Allocated supplies limit")]	
	protected int m_iMaxAvailableSuppliesAmount;
		
	//Switch for allowing visible top limit of supplies, if set to true - there will be also top limit visible in inventory at all times no matter this setting
	[Attribute("0", desc: "Bool for switching between supplies amount display modes")]	
	protected bool m_bVisibleTopLimit;

	protected int m_iPlayerSupplyAllocationOffset;

	protected RichTextWidget m_wSuppliesText;

	protected string m_sSuppliesAmount;
	
	[Attribute("#AR-supplies_MSAR_Availability")]
	protected string m_sAvailableText;
	
	[Attribute("#AR-supplies_MSAR_Availability_Long")]
	protected string m_sAvailableTextLong;
	
	protected PlayerController m_PlayerController = GetGame().GetPlayerController();
	
	protected SCR_PlayerSupplyAllocationComponent m_PlayerSupplyAllocationComponent;

	//------------------------------------------------------------------------------------------------
	//! Calls superclass method, subscribes to events and sets player current and limit allocation.
	//! \param[in] w Widget attached to the method, representing the root of the layout.
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		if (SCR_Global.IsEditMode() || !m_PlayerController)
		{
			w.SetVisible(false);
			return;
		}

		m_PlayerSupplyAllocationComponent = SCR_PlayerSupplyAllocationComponent.Cast(m_PlayerController.FindComponent(SCR_PlayerSupplyAllocationComponent));
		if (!m_PlayerSupplyAllocationComponent || !SCR_ArsenalManagerComponent.IsMilitarySupplyAllocationEnabled())
		{
			w.SetVisible(false);
			return;
		}

		w.SetVisible(true);
		m_wSuppliesText = RichTextWidget.Cast(w.GetParent().FindAnyWidget(m_sSuppliesText));

		m_PlayerSupplyAllocationComponent.GetOnAvailableAllocatedSuppliesChanged().Insert(OnAvailableAllocatedSuppliesChanged);
		m_PlayerSupplyAllocationComponent.GetOnMilitarySupplyAllocationChanged().Insert(OnMilitarySupplyAllocationChanged);

		SCR_InventoryMenuUI.GetOnItemHover().Insert(OnItemHover);
		SCR_InventoryMenuUI.GetOnItemHoverEnd().Insert(OnItemHoverEnd);

		SetPlayerCurrentAndLimitAllocation(w);
	}

	//------------------------------------------------------------------------------------------------
	//! Calls superclass method and unsubscribes from events
	//! \param[in] w Widget attached to the method, representing the root of the layout.
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);

		if (m_PlayerSupplyAllocationComponent)
		{
			w.SetVisible(false);
			m_PlayerSupplyAllocationComponent.GetOnAvailableAllocatedSuppliesChanged().Remove(OnAvailableAllocatedSuppliesChanged);
			m_PlayerSupplyAllocationComponent.GetOnMilitarySupplyAllocationChanged().Remove(OnMilitarySupplyAllocationChanged);
		}

		SCR_InventoryMenuUI.GetOnItemHover().Remove(OnItemHover);
		SCR_InventoryMenuUI.GetOnItemHoverEnd().Remove(OnItemHoverEnd);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetPlayerCurrentAndLimitAllocation(Widget w)
	{
		if (!m_PlayerSupplyAllocationComponent)
			return;

		m_iAvailableSuppliesAmount = m_PlayerSupplyAllocationComponent.GetPlayerAvailableAllocatedSupplies();
		m_iMaxAvailableSuppliesAmount = m_PlayerSupplyAllocationComponent.GetPlayerMilitarySupplyAllocation();
		
		UpdateSupplyAmountText();
	}

	//------------------------------------------------------------------------------------------------
	//! Sets the Supplies text to current amount of available and allocated supplies
	protected void UpdateSupplyAmountText()
	{
		if (!m_wSuppliesText)
			return;

		string maxSuppliesString = m_iMaxAvailableSuppliesAmount.ToString();
		if (m_iPlayerSupplyAllocationOffset < 0)
			maxSuppliesString = maxSuppliesString.Format("%1 (<color name='red'>%2</color>)", maxSuppliesString, m_iPlayerSupplyAllocationOffset.ToString());
		else if (m_iPlayerSupplyAllocationOffset > 0)
			maxSuppliesString = maxSuppliesString.Format("%1 (<color name='green'>+%2</color>)", maxSuppliesString, m_iPlayerSupplyAllocationOffset.ToString());

		if (m_bVisibleTopLimit)
			//Formating String to display to player Available supplies/Max Available Supplies, 100/500 Available
			m_wSuppliesText.SetTextFormat(m_sAvailableTextLong, m_iAvailableSuppliesAmount, maxSuppliesString);	
		else
			//Formating String to display to player Available supplies only 100 Available
			m_wSuppliesText.SetTextFormat(m_sAvailableText, m_iAvailableSuppliesAmount);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnItemHover(SCR_InventorySlotUI slot)
	{
		if (!slot || !SCR_ArsenalManagerComponent.IsMilitarySupplyAllocationEnabled())
			return;

		SCR_ArsenalInventorySlotUI arsenalSlot = SCR_ArsenalInventorySlotUI.Cast(slot);
		if (arsenalSlot)
		{
			// Item to be purchased, show the effect of the purchase on MSAR
			m_iPlayerSupplyAllocationOffset = -arsenalSlot.GetPersonalResourceCost();
			UpdateSupplyAmountText();
		}
		else
		{
			// Item to be refunded, show the effect of the refund on MSAR
			m_iPlayerSupplyAllocationOffset = GetItemPlayerSupplyAllocationRefundValue(slot);
			UpdateSupplyAmountText();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnItemHoverEnd()
	{
		m_iPlayerSupplyAllocationOffset = 0;
		UpdateSupplyAmountText();
	}

	//------------------------------------------------------------------------------------------------
	//! Loadout was selected, update MSAR text to include selected loadout price
	//! \param[in] loadout
	void OnLoadoutSelected(SCR_BasePlayerLoadout loadout)
	{
		if (!m_PlayerController)
			return;

		SCR_ArsenalManagerComponent arsenalManager;
		SCR_ArsenalManagerComponent.GetArsenalManager(arsenalManager);
		if (!arsenalManager)
			return;

		m_iPlayerSupplyAllocationOffset = -arsenalManager.GetLoadoutMilitarySupplyAllocationCost(loadout, m_PlayerController.GetPlayerId());
		UpdateSupplyAmountText();
	}

	//------------------------------------------------------------------------------------------------
	protected int GetItemPlayerSupplyAllocationRefundValue(SCR_InventorySlotUI slot)
	{
		SCR_InventoryMenuUI inventoryMenu = SCR_InventoryMenuUI.GetInventoryMenu();
		if (!inventoryMenu)
			return 0;

		SCR_InventoryStorageBaseUI storageUI = inventoryMenu.GetLootStorage();
		if (!storageUI)
			return 0;

		BaseInventoryStorageComponent storage = storageUI.GetCurrentNavigationStorage();
		if (!storage)
			return 0;

		IEntity storageOwner = storage.GetOwner();
		if (!storageOwner)
			return 0;

		SCR_ArsenalComponent arsenalComp = SCR_ArsenalComponent.Cast(storageOwner.FindComponent(SCR_ArsenalComponent));
		if (!arsenalComp)
			return 0;

		InventoryItemComponent item = slot.GetInventoryItemComponent();
		if (!item)
			return 0;

		IEntity itemEntity = item.GetOwner();
		if (!itemEntity)
			return 0;

		SCR_ArsenalManagerComponent arsenalManager;
		if (!SCR_ArsenalManagerComponent.GetArsenalManager(arsenalManager))
			return 0;

		return arsenalManager.GetItemMilitarySupplyAllocationRefundAmount(itemEntity, arsenalComp);
	}

	//------------------------------------------------------------------------------------------------
	//! Available Allocated Supplies amount changed, set the new value and update Supplies text
	//! \param[in] amount
	protected void OnAvailableAllocatedSuppliesChanged(int amount)
	{
		if (m_iAvailableSuppliesAmount == amount)
			return;

		m_iAvailableSuppliesAmount = amount;
		UpdateSupplyAmountText();
	}

	//------------------------------------------------------------------------------------------------
	//! Military Supply Allocation amount changed, set the new value and update Supplies text
	//! \param[in] amount
	protected void OnMilitarySupplyAllocationChanged(int amount)
	{
		if (m_iMaxAvailableSuppliesAmount == amount)
			return;

		m_iMaxAvailableSuppliesAmount = amount;
		UpdateSupplyAmountText();
	}
}