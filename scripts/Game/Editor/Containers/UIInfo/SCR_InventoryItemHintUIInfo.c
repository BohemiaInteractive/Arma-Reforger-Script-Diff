enum SCR_EInventoryHintInputDevice
{
	ANY = 0,
	KEYBOARD_MOUSE_ONLY,
	GAMEPAD_ONLY,
}

[BaseContainerProps()]
class SCR_InventoryItemHintUIInfo : SCR_ColorUIInfo
{
	[Attribute(SCR_EInventoryHintInputDevice.ANY.ToString(), desc: "Show hint depending on input device", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(SCR_EInventoryHintInputDevice))]
	protected SCR_EInventoryHintInputDevice m_eShowWithInputDevice;
	
	[Attribute("0", desc: "If true than will never show this hint when item is in arsenal")]
	protected bool m_bHideInArsenal;
	
	protected SCR_InventorySlotUI m_LastFocusedSlot;

	//------------------------------------------------------------------------------------------------
	//!
	//! \param item
	//! \param focusedSlot
	//! \return
	bool CanBeShown(InventoryItemComponent item, SCR_InventorySlotUI focusedSlot)
	{
		m_LastFocusedSlot = focusedSlot;

		//~ Make sure it is not shown if stored in arsenal
		if (m_bHideInArsenal && SCR_ArsenalInventorySlotUI.Cast(focusedSlot))
			return false;
		
		switch (m_eShowWithInputDevice)
		{
			case SCR_EInventoryHintInputDevice.ANY:
				return true;
			case SCR_EInventoryHintInputDevice.KEYBOARD_MOUSE_ONLY:
				return GetGame().GetInputManager().IsUsingMouseAndKeyboard();
			case SCR_EInventoryHintInputDevice.GAMEPAD_ONLY:
				return !GetGame().GetInputManager().IsUsingMouseAndKeyboard();
		}
	
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool SetIconTo(ImageWidget imageWidget)
	{
		if (!imageWidget)
			return false;
		
		if (!super.SetIconTo(imageWidget))
		{
			imageWidget.SetOpacity(0);
			return false;
		}
		
		imageWidget.SetColor(GetColor());
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param item
	//! \param textWidget
	//! \return
	bool SetItemHintNameTo(InventoryItemComponent item, TextWidget textWidget)
	{
		if (!textWidget)
			return false;
		
		textWidget.SetText(GetItemHintName(item));
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Function to override to get custom/dynamic item hint name
	string GetItemHintName(InventoryItemComponent item)
	{
		return GetName();
	}
}
