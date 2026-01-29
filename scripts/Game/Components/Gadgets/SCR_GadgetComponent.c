[EntityEditorProps(category: "GameScripted/Gadgets", description: "Gadget base", color: "0 0 255 255")]
class SCR_GadgetComponentClass : ScriptGameComponentClass
{
}

//------------------------------------------------------------------------------------------------
//! Gadget type 
enum EGadgetType
{
	NONE 			= 1,
	MAP 			= 1 << 1,
	COMPASS 		= 1 << 2,
	BINOCULARS 		= 1 << 3,
	FLASHLIGHT 		= 1 << 4,
	RADIO 			= 1 << 5,
	RADIO_BACKPACK 	= 1 << 6,
	WRISTWATCH 		= 1 << 7,
	CONSUMABLE 		= 1 << 8,
	BUILDING_TOOL 	= 1 << 9,
	SPECIALIST_ITEM = 1 << 10,
	NIGHT_VISION 	= 1 << 11,
	DETONATOR		= 1 << 12,
	GPS				= 1 << 13,
}

//! Gadget mode
enum EGadgetMode
{
	ON_GROUND = 0,	// ground
	IN_STORAGE,		// within storage but not slotted
	IN_SLOT,		// in equipment slot
	IN_HAND,		// held in left hand
	LAST
}

// @NOTE(Leo) : short term animation solution
// from conversation with @Théo Escamez: //for now we have 4 items> compass adrianov, compass SY183, Radio ANPRC68 and Radio R148
//! Gadget anim variable
enum EGadgetAnimVariable
{
	NONE = 0,
	ADRIANOV = 1, 
	SY183 = 2,
	ANPRC68 = 3,
	R148 = 4,
	MX991 = 5,
	FLASHLIGHT_SOVIET_01 = 6,
	M22 = 7,
	B12 = 8,
	MAP = 9
}

//! Gadget base class
class SCR_GadgetComponent : ScriptGameComponent
{
	[Attribute("", UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EGadgetAnimVariable), desc: "Gadget anim variable", category: "Gadget")]
	protected EGadgetAnimVariable m_eAnimVariable;

	[Attribute(SCR_EUseContext.NONE.ToString(), uiwidget: UIWidgets.Flags, desc: "From what context this gadget can be toggled.\nValue NONE is always selected but it is ignored when there is any other value selected", enums: ParamEnumArray.FromEnum(SCR_EUseContext), category: "Gadget")]
	protected SCR_EUseContext m_eUseMask;
	
	[Attribute("0 0 0", UIWidgets.Coords, desc: "Adjusted position of prefab within equipment slot, for when items placed intto a same slot have different sizes like flashlights", category: "Gadget")]
	protected vector m_vEquipmentSlotOffset;
	
	[Attribute("0.5", UIWidgets.Auto, desc: "Weapon no fire time set while equipped\n[s]", category: "Gadget")]
	protected float m_fWeaponNoFireTime;

	[Attribute("1", desc: "Can this item be held in hand", category: "Gadget")]
	protected bool m_bCanBeHeld;

	bool m_bFocused;
	protected bool m_bActivated = false;					// current state if the gadget can be toggled on	
	protected EGadgetMode m_iMode = EGadgetMode.ON_GROUND;	// curent gadget mode
	protected ChimeraCharacter m_CharacterOwner;			// current entity in posession of this gadget
				
	//------------------------------------------------------------------------------------------------
	//! \return
	EGadgetAnimVariable GetAnimVariable()
	{
		return m_eAnimVariable;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetWeaponNoFireTime()
	{
		return m_fWeaponNoFireTime;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Is gadget toggled on/off
	//! \return current state on/off
	bool IsToggledOn()
	{
		return m_bActivated;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Event called from SCR_GadgetManagerComponent through RPC request
	//! \param[in] state is gadget state: true - active / false - inactive
	void OnToggleActive(bool state);

	//------------------------------------------------------------------------------------------------
	//! InventoryItemComponent event
	void OnParentSlotChanged(InventoryStorageSlot oldSlot, InventoryStorageSlot newSlot)
	{
		IEntity parentOwner = GetOwner().GetRootParent();
		if (!SCR_ChimeraCharacter.Cast(parentOwner))
		{
			if (!newSlot)
				SCR_GadgetManagerComponent.SetGadgetModeStashed(this, EGadgetMode.ON_GROUND);
			else if (!oldSlot)
				SCR_GadgetManagerComponent.SetGadgetModeStashed(this, EGadgetMode.IN_STORAGE);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Gadget mode change event
	//! \param[in] mode is the target mode being switched to
	//! \param[in] charOwner
	void OnModeChanged(EGadgetMode mode, IEntity charOwner)
	{
		// Clear last mode
		ModeClear(m_iMode);
		
		// Update current mode
		m_iMode = mode;
		
		// Set new mode
		ModeSwitch(mode, charOwner);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set gadget mode 
	//! \param[in] mode is the target mode being switched to
	//! \param[in] charOwner
	protected void ModeSwitch(EGadgetMode mode, IEntity charOwner)
	{
		UpdateVisibility(mode);
		
		// if removing from inventory
		if (mode == EGadgetMode.ON_GROUND)
			m_CharacterOwner = null;
		else
			m_CharacterOwner = ChimeraCharacter.Cast(charOwner);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Clear gadget mode
	//! \param[in] mode is the mode being cleared
	protected void ModeClear(EGadgetMode mode);

	//------------------------------------------------------------------------------------------------
	//! Method called when slot to which item is attached to changed its occlusion state
	//! \param[in] occluded if parent slot is now occluded or not
	void OnSlotOccludedStateChanged(bool occluded);
						
	//------------------------------------------------------------------------------------------------
	//! Activate gadget frame update
	void ActivateGadgetUpdate()
	{
		ConnectToGadgetsSystem();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Deactivate gadget frame flag
	void DeactivateGadgetUpdate()
	{
		DisconnectFromGadgetsSystem();
	}
	

	//------------------------------------------------------------------------------------------------
	//! Set visibility when show/hide hand animation starts/finishes
	// \param[in] inHand states whether the gadget is currently held in hand
	void UpdateVisibility(EGadgetMode mode)
	{		
		// mode1
		InventoryItemComponent itemComponent = InventoryItemComponent.Cast(GetOwner().FindComponent(InventoryItemComponent));
		if (!itemComponent)
			return;
			
		// if in hand or visible while slotted
		if ( mode == EGadgetMode.IN_HAND || (mode == EGadgetMode.IN_SLOT && IsVisibleEquipped()) )
		{	
			// slotted,  Set positioning of visible slotted gadgets on character -> configured in item prefab to allow gadgets of different sizes
			if (mode != EGadgetMode.IN_HAND)
			{			
				InventoryStorageSlot slot = itemComponent.GetParentSlot();
				ItemAnimationAttributes animAttr = ItemAnimationAttributes.Cast(itemComponent.FindAttribute(ItemAnimationAttributes));
				if (slot && animAttr)
				{
					vector matLS[4];
					animAttr.GetAdditiveTransformLS(matLS);
					matLS[3] = matLS[3] + m_vEquipmentSlotOffset;
					slot.SetAdditiveTransformLS(matLS); 
				}
				
				EquipmentStorageSlot equipSlot = EquipmentStorageSlot.Cast(slot);
				if (equipSlot && equipSlot.IsOccluded())
				{
					itemComponent.HideOwner();	// slotted but occluded
					return;
				}
			}
			
			itemComponent.ShowOwner();
		}
		// if slotted and not visible or not slotted
		else
		{
			if (mode == EGadgetMode.ON_GROUND)
				itemComponent.ShowOwner();
			else 
				itemComponent.HideOwner();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Synchronise gadget state
	//! \param[in] state is target state true = active, false = inactive
	void ToggleActive(bool state, SCR_EUseContext context)
	{
		if (!m_CharacterOwner)
			return;

		if (m_eUseMask == SCR_EUseContext.NONE || (m_eUseMask & context) == 0)
			return;

		RplComponent rplComponent = RplComponent.Cast(m_CharacterOwner.FindComponent(RplComponent));
		if (!rplComponent || !rplComponent.IsOwner())
			return;					// NOT owner of the character in possession of this gadget

		// Client side
		rplComponent = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		if (rplComponent && rplComponent.IsProxy())
			OnToggleActive(state);	// activate client side to avoid server delay

		// Sync
		SCR_GadgetManagerComponent.GetGadgetManager(m_CharacterOwner).AskToggleGadget(this, state);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Action listener callback
	void ActivateAction();
	
	//------------------------------------------------------------------------------------------------
	//! Toggle gadget focus state
	//! \param[in] enable is target state
	void ToggleFocused(bool enable)
	{
		m_bFocused = enable;

		if (m_CharacterOwner != SCR_PlayerController.GetLocalControlledEntity())
			return;

		SCR_PlayerController controller = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (controller)
			controller.SetGadgetFocus(enable);
	}
		
	//------------------------------------------------------------------------------------------------
	//! Get IEntity in possession of this Gadget
	//! \return returns entity m_CharacterOwner
	ChimeraCharacter GetCharacterOwner()
	{
		return m_CharacterOwner;
	}
				
	//------------------------------------------------------------------------------------------------
	//! Get gadget type
	//! \return Returns gadget type
	EGadgetType GetType()
	{
		return EGadgetType.NONE;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Get gadget mode
	//! \return Returns gadget mode
	EGadgetMode GetMode()
	{
		return m_iMode;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Can be held in hand
	//! \return Returns true if heldable
	bool CanBeHeld()
	{
		return m_bCanBeHeld;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Provides value that contains mask of contexts from which this gadget can be toggled
	//! \return Returns int mask of SCR_EUseContext 
	SCR_EUseContext GetUseMask()
	{
		return m_eUseMask;
	}

	//------------------------------------------------------------------------------------------------
	//! Gadget has a raised animation version
	//! \return Returns true if raisable
	bool CanBeRaised()
	{
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gadget uses ADS controls from gadget raisable context
	//! \return Returns true if using the controls
	bool IsUsingADSControls()
	{
		return false;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Visible when attached to character gear, further condition may determine visibility, such as if the equipment slot is obstructed by something else
	//! \return Returns true if visible in equipment slot
	bool IsVisibleEquipped()
	{
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] timeSlice
	//Called by Gadgets system
	void Update(float timeSlice);
	
	//------------------------------------------------------------------------------------------------
	//! Starts on frame update event
	protected void ConnectToGadgetsSystem()
	{
		World world = GetOwner().GetWorld();
		GadgetsSystem gadgetSystem = GadgetsSystem.Cast(world.FindSystem(GadgetsSystem));
		if (!gadgetSystem)
			return;
		
		gadgetSystem.Register(this);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DisconnectFromGadgetsSystem()
	{
		World world = GetOwner().GetWorld();
		GadgetsSystem gadgetSystem = GadgetsSystem.Cast(world.FindSystem(GadgetsSystem));
		if (!gadgetSystem)
			return;
		
		gadgetSystem.Unregister(this);
	}
	
	//------------------------------------------------------------------------------------------------	
	override bool RplSave(ScriptBitWriter writer)
	{
		writer.WriteIntRange(m_iMode, 0, EGadgetMode.LAST-1);

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		reader.ReadIntRange(m_iMode, 0, EGadgetMode.LAST-1);
		
		UpdateVisibility(m_iMode);
			
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		DisconnectFromGadgetsSystem();
		
		super.OnDelete(owner);
	}
						
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);

		InventoryItemComponent invComp = InventoryItemComponent.Cast(GetOwner().FindComponent(InventoryItemComponent));
		if (invComp)
			invComp.m_OnParentSlotChangedInvoker.Insert(OnParentSlotChanged);
	}
}
