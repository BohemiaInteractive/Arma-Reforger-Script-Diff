[BaseContainerProps()]
class SCR_ItemAttributeCollection: ItemAttributeCollection
{
	[Attribute("2", UIWidgets.ComboBox, "Slot size", "", ParamEnumArray.FromEnum(ESlotSize))]
	protected ESlotSize m_Size;
	
	[Attribute(SCR_EQuickSlotSize.DEFAULT.ToString(), desc: "Slot size when item is equipped in quickbar. Set default to keep original slot size", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_EQuickSlotSize))]
	protected SCR_EQuickSlotSize m_eQuickSlotSize;
	
	[Attribute("2", UIWidgets.ComboBox, "Slot size", "", ParamEnumArray.FromEnum(ESlotID))]
	protected ESlotID m_SlotType;

	[Attribute("1", UIWidgets.CheckBox, "Sets item movable by drag'n'drop")]
	protected bool m_bDraggable;

	[Attribute("1", UIWidgets.CheckBox, "Sets item visible in inventory")]
	protected bool m_bVisible;
	
	[Attribute("1", UIWidgets.CheckBox, "Sets item stackable in inventory")]
	protected bool m_bStackable;
	
	[Attribute("1", UIWidgets.CheckBox, "Allows for the item to be refunded in arsenals (set to false for mission critical items)")]
	protected bool m_bRefundable;

	private ItemPhysicalAttributes m_PhysAttributes;
	
	//------------------------------------------------------------------------------------------------
	float GetVolume() 
	{
		if (m_PhysAttributes != null)
		{
			return m_PhysAttributes.GetVolume();
		}
		return 0.0;
	}

	
	//------------------------------------------------------------------------------------------------
	float GetWeight()
	{
		if (m_PhysAttributes != null)
		{
			return m_PhysAttributes.GetWeight();
		}
		return 0.0;
	};

	//------------------------------------------------------------------------------------------------
	bool IsDraggable()
	{
		return m_bDraggable;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsVisible(InventoryItemComponent inventoryItem)
	{
		return m_bVisible;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsStackable()
	{
		return m_bStackable;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsRefundable()
	{
		return m_bRefundable;
	}

	//------------------------------------------------------------------------------------------------
	void SetSlotSize( ESlotSize slotSize ) 	
	{
		m_Size = slotSize; 
	}

	//------------------------------------------------------------------------------------------------
	void SetDraggable(bool isDraggable)
	{
		m_bDraggable = isDraggable;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetRefundable(bool isRefundable)
	{
		m_bRefundable = isRefundable;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetSlotType( ESlotID slotID ) 	
	{
		m_SlotType = slotID; 
	}
	
	//------------------------------------------------------------------------------------------------
	ESlotID GetSlotType() 		{ return m_SlotType; }

	//------------------------------------------------------------------------------------------------
	int GetSlotSum()
	{
		int iRetVal = 0;
		switch( m_Size )
		{
			case ESlotSize.SLOT_1x1: { iRetVal = 1; } break;
			case ESlotSize.SLOT_2x1: { iRetVal = 2; } break;
			case ESlotSize.SLOT_2x2: { iRetVal = 4; } break;
			case ESlotSize.SLOT_3x3: { iRetVal = 9; } break;
		}
		return iRetVal;
	}
			
	//------------------------------------------------------------------------------------------------
	//! size of the slot the item fits in
	ESlotSize GetItemSize()	
	{ 
		return m_Size; 
	}
	
	//! Size of item when added to the quickbar
	ESlotSize GetQuickSlotItemSize()	
	{ 
		if (m_eQuickSlotSize == SCR_EQuickSlotSize.DEFAULT)
			return GetItemSize();
		
		return m_eQuickSlotSize; 
	}

	override protected void OnInitCollection(IEntityComponentSource src)
	{
		m_PhysAttributes = ItemPhysicalAttributes.Cast(FindAttribute(ItemPhysicalAttributes));
	}
};
