//------------------------------------------------------------------------------------------------
//! UI Script
//! Inventory Slot UI Layout
class SCR_InventorySlotQuickSlotUI : SCR_InventorySlotUI
{
	ResourceName m_sPrimarySecondaryWeapon = "{F565C4A388964DBE}UI/Textures/InventoryIcons/InventorySlot_gadget.edds";	//TODO: icon for weapon
	
	protected ImageWidget m_wGamepadHintSmall;
	protected ImageWidget m_wGamepadHintLarge;
	protected RichTextWidget m_wKeybindText;

	//------------------------------------------------------------------------ USER METHODS ----------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	override SCR_EAnalyticalItemSlotType GetAnalyticalItemSlotType()
	{
		return SCR_EAnalyticalItemSlotType.HORIZONTAL;
	}
	
	//------------------------------------------------------------------------------------------------
	override void Init()
	{
		super.Init();
		GetGame().OnInputDeviceIsGamepadInvoker().Insert(UpdateHints);
	}
	
	//------------------------------------------------------------------------------------------------
	//! should be the slot visible?
	override void SetSlotVisible( bool bVisible )
	{
		super.SetSlotVisible( bVisible );

		m_wKeybindText = RichTextWidget.Cast(m_widget.FindAnyWidget("KeybindRichText"));
		if (m_wKeybindText)
		{
			EInputDeviceType currentDevice = GetGame().GetInputManager().GetLastUsedInputDevice();
			InputBinding binding = GetGame().GetInputManager().CreateUserBinding();

			string deviceString = "keyboard";
			if (currentDevice == EInputDeviceType.GAMEPAD)
				deviceString = "gamepad";

			string name = string.Format("%1%2", "SwitchWeaponCategory", (m_iQuickSlotIndex + 1));

			array<string> bindings = {};
			if (deviceString == "gamepad" &&
				!binding.GetBindings(name, bindings, currentDevice))
				return;

			if (name == "SwitchWeaponCategory10")
				name = "SwitchWeaponCategory0";

			m_wKeybindText.SetText(string.Format("<action name='%1' preset='%2' device='" + deviceString + "' scale='1.25'/>", name, ""));
		}

		if( bVisible )
		{
			m_wSelectedEffect = m_widget.FindAnyWidget("SelectedFrame");
			
			if ( !m_pItem )	//if item is not available, it's the empty slot, show the large number
			{
				m_wTextQuickSlotLarge = TextWidget.Cast( m_widget.FindAnyWidget( "TextQuickSlotLarge" ) );
				SetQuickSlotIndexVisible( m_wTextQuickSlotLarge, true );
				SetQuickSlotHintVisible(m_wKeybindText);
			}
			else
			{
				SetQuickSlotIndexVisible( m_wTextQuickSlot, m_pStorageUI.GetInventoryMenuHandler() != null );
				SetQuickSlotHintVisible(m_wKeybindText);
			}
		}
		else
		{
			m_wSelectedEffect = null;
			m_wTextQuickSlotLarge = null;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override string SetSlotSize()
	{
		ESlotSize slotSize;
		string slotLayout = SLOT_LAYOUT_1x1;

		if (m_iQuickSlotIndex < 2 || SCR_WeaponSwitchingBaseUI.s_bRadial)
		{
			slotSize = ESlotSize.SLOT_2x1;
			m_iSizeX = 2; 
			m_iSizeY = 1;
		}
		else
		{
			slotSize = ESlotSize.SLOT_1x1;
			m_iSizeX = 1; 
			m_iSizeY = 1; 
		}

		return slotLayout;
	}

	void UpdateHints()
	{
		// SetQuickSlotHintVisible(m_wKeybindText);
		m_wKeybindText.SetVisible(!GetGame().GetInputManager().IsUsingMouseAndKeyboard());
	}

	//------------------------------------------------------------------------------------------------
	void SetQuickSlotHintVisible(RichTextWidget hintWidget)
	{
		if (!GetGame().GetInputManager().IsUsingMouseAndKeyboard() && hintWidget.IsEnabled()) 
			hintWidget.SetVisible(true);		
	}

	//------------------------------------------------------------------------ COMMON METHODS ----------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	void SCR_InventorySlotQuickSlotUI( InventoryItemComponent pComponent = null, SCR_InventoryStorageBaseUI pStorageUI = null, bool bVisible = true, int iSlotIndex = -1, SCR_ItemAttributeCollection pAttributes = null )
	{
		m_iQuickSlotIndex = iSlotIndex;
		m_Attributes = pAttributes;
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_InventorySlotQuickSlotUI()
	{
	}
};