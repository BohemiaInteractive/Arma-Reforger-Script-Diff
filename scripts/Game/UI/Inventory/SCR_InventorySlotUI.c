const string										SLOT_LAYOUT_1x1 		= "{F437ACE2BD5F11E2}UI/layouts/Menus/Inventory/InventoryItemSlot.layout";

const int											CURRENT_AMMO_MAGAZINE	= ARGB( 255, 255, 255, 255 );
const int											CURRENT_AMMO_TEXT		= ARGB( 255, 142, 142, 142 );
const int											CURRENT_AMMO_EMPTY		= ARGB( 255, 255, 75, 75 );

const float											CURRENT_AMMO_OPACITY	= 0.432;

enum ESlotFunction
{
	TYPE_GENERIC,
	TYPE_MAGAZINE,
	TYPE_WEAPON,
	TYPE_GADGET,
	TYPE_HEALTH,
	TYPE_CONSUMABLE,
	TYPE_STORAGE,
	TYPE_CLOTHES
};


//------------------------------------------------------------------------------------------------
//! UI Script
//! Inventory Slot UI Layout
class SCR_InventorySlotUI : ScriptedWidgetComponent
{
	[Attribute("1", UIWidgets.ComboBox, "Slot size", "", ParamEnumArray.FromEnum(ESlotSize) )]
	private ESlotSize m_ESlotSize;
	protected int										m_iSizeX;
	protected int										m_iSizeY;
	protected int										m_iPage;				//helper variable telling us what page the slot should be stored on
	
	private WorkspaceWidget 							m_workspaceWidget 		= null;
	Widget												m_widget 				= null;	
	private ButtonWidget								m_wButton;
	private ImageWidget									m_wIcon 				= null;
	private RenderTargetWidget							m_wPreviewImage 		= null;
	
	protected SCR_InventoryStorageBaseUI				m_pStorageUI;
		
	protected InventoryItemComponent 					m_pItem;
	protected ref SCR_ItemAttributeCollection			m_Attributes;
	protected bool										m_bEnabled 	= true;
	protected bool										m_bSelected = false;
	protected bool										m_bBlocked = false;
	protected Widget									m_wSelectedEffect, m_wSelectedIcon , m_wMoveEffect, m_wDimmerEffect, m_wBlockedEffect;
	protected TextWidget								m_wTextQuickSlot = null;
	protected TextWidget								m_wTextQuickSlotLarge = null;
	protected int										m_iQuickSlotIndex;
	
	protected ProgressBarWidget							m_wVolumeBar, m_wAmmoCount;
	protected int										m_iStackNumber = 1;
	protected TextWidget								m_wStackNumber, m_wMagazineNumber;
	protected OverlayWidget								m_wItemLockThrobber;
	protected string								   	m_sItemName = "";
	protected int										m_aAmmoCountActual = -1;
	protected int										m_aAmmoCountMax = -1;
	
	
	protected bool										m_bVisible;
	protected ESlotFunction								m_eSlotFunction = ESlotFunction.TYPE_GENERIC;
	protected SCR_InventoryProgressBar					m_ProgressBar;
	protected Widget 									m_wCurrentMagazine;
	protected ImageWidget								m_wCurrentMagazineAmmoCount;
	protected Widget 									m_wProgressBar;
	
	//~ FUEL
	protected SCR_FuelManagerComponent m_FuelManager;
	
	#ifdef DEBUG_INVENTORY20
		protected TextWidget							m_wDbgClassText1;
		protected TextWidget							m_wDbgClassText2;
		protected TextWidget							m_wDbgClassText3;
	#endif

	//------------------------------------------------------------------------ USER METHODS ------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	SCR_EAnalyticalItemSlotType GetAnalyticalItemSlotType()
	{
		return m_pStorageUI.GetAnalyticalItemSlotType();
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateReferencedComponent( InventoryItemComponent pComponent, SCR_ItemAttributeCollection attributes = null )
	{
		if ( m_widget )
			Destroy();
		m_pItem = pComponent;
		
		if (attributes)
			m_Attributes = attributes;
		
		if (m_pItem && m_pItem.GetAttributes())
			m_Attributes = SCR_ItemAttributeCollection.Cast( m_pItem.GetAttributes() );			//set the slot attributes (size) based on the information stored in the item 
		if (!m_Attributes)
			return;
		if (m_pItem && !m_Attributes.IsVisible(m_pItem))
			return;

		auto vehicleAttributes = SCR_InventoryVehicleVisibilityAttribute.Cast(m_Attributes.FindAttribute(SCR_InventoryVehicleVisibilityAttribute));
		if (vehicleAttributes && !ShouldVehicleSlotBeVisible(vehicleAttributes))
			return;

		m_workspaceWidget = GetGame().GetWorkspace();
		Widget wGrid = m_pStorageUI.GetStorageGrid();
		
		if (!wGrid)
			return;
		
		m_widget = m_workspaceWidget.CreateWidgets( SetSlotSize(), wGrid );
		m_widget.AddHandler( this );	//calls the HandlerAttached()
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateInventorySlot(InventoryItemComponent comp, int stackNumber)
	{
		m_pItem = comp;
		m_iStackNumber = stackNumber;
		Refresh();
	}

	//------------------------------------------------------------------------------------------------
	bool IsDraggable()
	{
		if (!m_Attributes)
			return true;

		return m_Attributes.IsDraggable();
	}

	//------------------------------------------------------------------------------------------------
	protected string SetSlotSize()
	{
		string slotLayout = SLOT_LAYOUT_1x1;
		switch ( m_Attributes.GetItemSize() ) 
		{
			case ESlotSize.SLOT_1x1:	{ m_iSizeX = 1; m_iSizeY = 1; } break;
			case ESlotSize.SLOT_2x1:	{ m_iSizeX = 2; m_iSizeY = 1; } break;
			case ESlotSize.SLOT_2x2:	{ m_iSizeX = 2; m_iSizeY = 2; } break;
			case ESlotSize.SLOT_3x3:	{ m_iSizeX = 3; m_iSizeY = 3; } break;
		}
		return slotLayout;
	}
	//------------------------------------------------------------------------------------------------
	protected void Init()
	{
		m_iPage = -1;		//no page placement by default
		//and create the visual slot
		
		m_wTextQuickSlot = TextWidget.Cast( m_widget.FindAnyWidget( "TextQuickSlot" ) );
		m_sItemName = GetItemName();	//debug purposes
		
		if (m_pItem)
			m_pItem.m_OnLockedStateChangedInvoker.Insert(OnChangeLockState);
		
		SetItemFunctionality();
		if (m_eSlotFunction == ESlotFunction.TYPE_WEAPON)
		{
			m_wMagazineNumber = TextWidget.Cast( m_widget.FindAnyWidget( "magazineCount" ) );
			m_wCurrentMagazine = m_widget.FindAnyWidget("currentMagazine");
			if (m_wCurrentMagazine)
			{
				m_wCurrentMagazineAmmoCount = ImageWidget.Cast(m_widget.FindAnyWidget("currentMagazineAmmoCount"));		
				ImageWidget outline = ImageWidget.Cast(m_wCurrentMagazine.FindAnyWidget("Outline"));
				if (outline && m_pItem)
				{
					BaseMuzzleComponent muzzleComp = BaseMuzzleComponent.Cast(m_pItem.GetOwner().FindComponent(BaseMuzzleComponent));
					if (muzzleComp)
					{
						MuzzleUIInfo info = MuzzleUIInfo.Cast(muzzleComp.GetUIInfo());
						if (muzzleComp.IsInherited(MuzzleInMagComponent))
						{
							m_wCurrentMagazineAmmoCount.LoadImageFromSet(0, info.m_MagIndicator.m_sImagesetIcons, info.m_MagIndicator.m_sOutline);
							outline.LoadImageFromSet(0, info.m_MagIndicator.m_sImagesetIcons, info.m_MagIndicator.m_sOutline);
						}
						else
						{
							m_wCurrentMagazineAmmoCount.LoadImageFromSet(0, info.m_MagIndicator.m_sImagesetIcons, info.m_MagIndicator.m_sProgress);
							outline.LoadImageFromSet(0, info.m_MagIndicator.m_sImagesetIcons, info.m_MagIndicator.m_sProgress);
						}
					}
				}
			}			
		}

		SetSlotVisible( m_bVisible );

		// If is it storage or attachment, register it to the array for the future use
		if (!BaseInventoryStorageComponent.Cast(m_pItem) && !WeaponAttachmentsStorageComponent.Cast(m_pItem))
			return;
		
		if (m_pStorageUI && m_pStorageUI.GetInventoryMenuHandler())
			m_pStorageUI.GetInventoryMenuHandler().RegisterUIStorage(this);
	}
	
	//------------------------------------------------------------------------------------------------	
	//! returns the storage component associated with this UI component
	BaseInventoryStorageComponent GetStorageComponent() { return BaseInventoryStorageComponent.Cast(m_pItem); }
	
	//------------------------------------------------------------------------------------------------
	LoadoutAreaType GetLoadoutArea()
	{
		if ( !m_pItem )
			return null;
		auto pClothComponent = BaseLoadoutClothComponent.Cast( m_pItem.GetOwner().FindComponent( BaseLoadoutClothComponent ) );
		if ( !pClothComponent )
			return null;
		
		return pClothComponent.GetAreaType();
	}
	
	
	//------------------------------------------------------------------------------------------------
	void SetStackNumber( int i ) { m_iStackNumber = i; }

	//------------------------------------------------------------------------------------------------
	void IncreaseStackNumber() 
	{ 
		m_iStackNumber++;
	}

	//------------------------------------------------------------------------------------------------
	void IncreaseStackNumberWithRplId(RplId id)
	{
		m_iStackNumber++;
	}

	//------------------------------------------------------------------------------------------------
	void OnOwnedSlotsUpdated();

	//------------------------------------------------------------------------------------------------	
	//!
	protected void SetAmmoCount()
	{
		if (!m_pItem)
			return;
		
		MagazineComponent magComp = MagazineComponent.Cast(m_pItem.GetOwner().FindComponent(MagazineComponent));
		if (!magComp)
			return;

		m_aAmmoCountActual = magComp.GetAmmoCount();
		m_aAmmoCountMax = magComp.GetMaxAmmoCount();			
		
		if (m_ProgressBar)
		{
			if (m_aAmmoCountMax == 1)
			{
				m_wProgressBar.SetVisible(false);
				return;
			}
			
			m_ProgressBar.FlipColors();
			m_ProgressBar.SetProgressRange(0, magComp.GetMaxAmmoCount());
			m_ProgressBar.SetCurrentProgress(magComp.GetAmmoCount());		
		}
	}
	
	protected void SetAmmoType()
	{
		Widget ammoType = m_widget.FindAnyWidget("AmmoTypeSize");
		if (!ammoType)
			return;
		SCR_InventoryAmmoTypeIndicator indicator = SCR_InventoryAmmoTypeIndicator.Cast(ammoType.FindHandler(SCR_InventoryAmmoTypeIndicator));
		if (indicator)
			indicator.SetAmmoType(m_pItem);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnFuelAmountChanged(float newFuelValue)
	{
		m_ProgressBar.SetProgressRange(0, m_FuelManager.GetTotalMaxFuel());
		m_ProgressBar.SetCurrentProgress(newFuelValue);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void InitFuelAmount()
	{
		if (!m_wProgressBar)
			m_wProgressBar = m_widget.FindAnyWidget("ProgressBar");

		if (m_wProgressBar)
		{
			m_ProgressBar = SCR_InventoryProgressBar.Cast(m_wProgressBar.FindHandler(SCR_InventoryProgressBar));
			m_ProgressBar.FlipColors();
			m_wProgressBar.SetVisible(true);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	Widget GetButtonWidget() { return m_wButton; }	
	//------------------------------------------------------------------------------------------------
	bool IsSlotEnabled() { return m_bEnabled; }
	//------------------------------------------------------------------------------------------------
	bool IsSlotBlocked() { return m_bBlocked; }
	//------------------------------------------------------------------------------------------------
	bool IsSelected() { return m_bSelected; }
	//------------------------------------------------------------------------------------------------
	//! should be the slot visible?
	void SetSlotVisible( bool bVisible )
	{
		m_bVisible = bVisible;
		m_widget.SetEnabled(bVisible);
		m_widget.SetVisible(bVisible);
		
		if(bVisible)
		{
			m_wPreviewImage = RenderTargetWidget.Cast( m_widget.FindAnyWidget( "item" ) );
			ImageWidget iconImage = ImageWidget.Cast(m_widget.FindAnyWidget("icon"));
			RichTextWidget iconText = RichTextWidget.Cast(m_widget.FindAnyWidget("itemName"));
			TextWidget quickslotNumber = TextWidget.Cast(m_widget.FindAnyWidget("TextQuickSlotLarge"));
			
			//filter out 1-4 quickslots as they are in fact storages :harold:
			if (SCR_InventoryMenuUI.WEAPON_SLOTS_COUNT <= m_iQuickSlotIndex && m_iQuickSlotIndex < 10)
			{
				IEntity controlled = SCR_PlayerController.GetLocalControlledEntity();
				if (controlled)
				{
					SCR_CharacterInventoryStorageComponent storage = GetCharacterStorage(controlled);
					if (storage)
					{
						SCR_QuickslotBaseContainer container = storage.GetContainerFromQuickslot(m_iQuickSlotIndex);
						if (container)
							container.HandleVisualization(iconImage, m_wPreviewImage, iconText, quickslotNumber);
					}
				}
			}
			
			ChimeraWorld world = ChimeraWorld.CastFrom(GetGame().GetWorld());
			if (world)
			{
				ItemPreviewManagerEntity manager = world.GetItemPreviewManager();
				if (manager)
				{
					ItemPreviewWidget renderPreview = ItemPreviewWidget.Cast( m_wPreviewImage );
					IEntity previewEntity = null;
					if (m_pItem)
					{
						previewEntity = m_pItem.GetOwner();
						m_wPreviewImage.SetVisible(true);
					}

					if (renderPreview)
						manager.SetPreviewItem(renderPreview, previewEntity, null, true);
				}
			}
			
			//if the slot has storage, then show its volume bar
			if (m_pStorageUI.Type() != SCR_InventoryStoragesListUI && GetStorageComponent() || m_eSlotFunction == ESlotFunction.TYPE_MAGAZINE)
			{
				m_wProgressBar = m_widget.FindAnyWidget("ProgressBar");
				if (m_wProgressBar)
				{
					m_ProgressBar = SCR_InventoryProgressBar.Cast(m_wProgressBar.FindHandler(SCR_InventoryProgressBar));
					m_wProgressBar.SetVisible(m_eSlotFunction != ESlotFunction.TYPE_WEAPON);
				}
			}
			m_wSelectedEffect = m_widget.FindAnyWidget("SelectedOverlay");
			m_wSelectedIcon = m_widget.FindAnyWidget("SelectedIcon");
			m_wMoveEffect = m_widget.FindAnyWidget( "IconMove" );
			m_wDimmerEffect = m_widget.FindAnyWidget( "Dimmer" );
			m_wBlockedEffect = m_widget.FindAnyWidget("Blocker");
			m_wButton = ButtonWidget.Cast( m_widget.FindAnyWidget( "ItemButton" ) );
			m_wStackNumber = TextWidget.Cast( m_widget.FindAnyWidget( "stackNumber" ) );
			m_wItemLockThrobber = OverlayWidget.Cast(m_widget.FindAnyWidget("itemLockThrobber"));
		
			if ( m_iStackNumber > 1 )
			{
				m_wStackNumber.SetText( m_iStackNumber.ToString() );
				m_wStackNumber.SetVisible( true );
			}
			else
			{
				m_wStackNumber.SetVisible( false );
			}
			
			if (m_eSlotFunction == ESlotFunction.TYPE_MAGAZINE )
			{
				SetAmmoCount();
				UpdateAmmoCount();
			}
			
			if (m_FuelManager)
				OnFuelAmountChanged(m_FuelManager.GetTotalFuel());
			

			if ( m_wMagazineNumber && m_wCurrentMagazineAmmoCount )
			{
				UpdateWeaponAmmoCount();
			}
			
			#ifdef DEBUG_INVENTORY20
				if ( !m_pItem )
				{
					array<string> dbgText = new array<string>();
					this.ToString().Split( "<", dbgText, false );
 					m_wDbgClassText1 = TextWidget.Cast( m_widget.FindAnyWidget( "dbgTextClass1" ) );
					m_wDbgClassText2 = TextWidget.Cast( m_widget.FindAnyWidget( "dbgTextClass2" ) );
					m_wDbgClassText3 = TextWidget.Cast( m_widget.FindAnyWidget( "dbgTextClass3" ) );
					m_wDbgClassText1.SetText( dbgText[0] );
					m_wDbgClassText1.SetEnabled( true );
					m_wDbgClassText1.SetVisible( true );
					m_wDbgClassText2.SetText( dbgText[1] );
					m_wDbgClassText2.SetEnabled( true );
					m_wDbgClassText2.SetVisible( true );
					m_pItem.ToString().Split( "<", dbgText, false );
					if ( dbgText.Count() > 1 )
					{
						m_wDbgClassText3.SetText( dbgText[1] );
						m_wDbgClassText3.SetEnabled( true );
						m_wDbgClassText3.SetVisible( true );
					}
				}
			#endif
			UpdateVolumeBarValue();
			
		}
		else
		{
			m_wPreviewImage = null;
			m_wSelectedEffect = null;
			m_wSelectedIcon = null;
			m_wMoveEffect = null;
			m_wDimmerEffect = null;
			m_wButton = null;
			m_wVolumeBar = null;
			m_wTextQuickSlot = null;
			m_wTextQuickSlotLarge = null;
			m_wStackNumber = null;
			#ifdef DEBUG_INVENTORY20
				m_wDbgClassText1 = null;
				m_wDbgClassText2 = null;
				m_wDbgClassText3 = null;
			#endif
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! should be the slot be blocked?
	void SetSlotBlocked(bool bBlocked)
	{
		m_bBlocked = bBlocked;
		
		if (!m_wBlockedEffect || !m_wDimmerEffect)
			return;
		
		if (m_wBlockedEffect.IsVisible() == bBlocked)
			return;
		
		m_wDimmerEffect.SetVisible(bBlocked);
		m_wBlockedEffect.SetVisible(bBlocked);
	}

	//------------------------------------------------------------------------------------------------
	// ! 
	int GetSlotIndex() { return m_iQuickSlotIndex; }
	
	//------------------------------------------------------------------------------------------------
	// ! 
	void SetQuickSlotIndexVisible( TextWidget textQuickSlot, bool bVisible )
	{
		if ( textQuickSlot && GetGame().GetInputManager().IsUsingMouseAndKeyboard() )
		{
			textQuickSlot.SetText( ( (m_iQuickSlotIndex + 1) % 10 ).ToString() );
			textQuickSlot.SetVisible( bVisible );
		}
	}
	
	
	//------------------------------------------------------------------------------------------------	
	void OnSlotFocused()
	{
		if ( !m_pStorageUI )
			return;
		m_pStorageUI.SetSlotFocused( this, true );
		SCR_InventoryMenuUI pMenuManager = m_pStorageUI.GetInventoryMenuHandler();
		if ( pMenuManager )
		{
			#ifdef DEBUG_INVENTORY20
			
			BaseInventoryStorageComponent dbgStorage;
			InventoryStorageSlot dbgStorageParentSlot;
			
			if ( m_pStorageUI.GetStorage() )
			{
				dbgStorage = m_pStorageUI.GetStorage();
				if ( m_pStorageUI.GetStorage().GetParentSlot() )
					dbgStorageParentSlot = m_pStorageUI.GetStorage().GetParentSlot();
			}
						
			PrintFormat( "INV:OnSlotFocused | m_pStorageUI: %1 | Focused slotUI: %2 | Parent storage: %3 | from parent slot: %4", m_pStorageUI, this, dbgStorage, dbgStorageParentSlot );
			#endif
			pMenuManager.SetSlotFocused( this, GetStorageUI(), true );
		}
	}
	
	//------------------------------------------------------------------------------------------------	
	override bool OnMouseEnter( Widget w, int x, int y )
	{
		OnSlotFocused();
		if ( m_wButton )
			if ( m_wButton.FindHandler( SCR_ButtonComponent ) )
				m_wButton.FindHandler( SCR_ButtonComponent ).OnFocus( w, x, y );
		return false;
	}
		
	//------------------------------------------------------------------------------------------------	
	override bool OnMouseLeave( Widget w, Widget enterW, int x, int y )
	{
		OnSlotFocusLost();
		if ( m_wButton )
			if ( m_wButton.FindHandler( SCR_ButtonComponent ) )
				m_wButton.FindHandler( SCR_ButtonComponent ).OnFocusLost( w, x, y );
		return false;
	}
	
	//------------------------------------------------------------------------------------------------	
	void OnSlotFocusLost()
	{
		if (!m_pStorageUI)
			return;
		m_pStorageUI.SetSlotFocused( this, false );
		SCR_InventoryMenuUI pMenuManager = m_pStorageUI.GetInventoryMenuHandler();
		if ( pMenuManager )
			pMenuManager.SetSlotFocused( this, m_pStorageUI, false );	
	}
	
	//------------------------------------------------------------------------------------------------	
	void ToggleSelected()
	{
		m_bSelected = !m_bSelected;
		if (!m_wSelectedEffect || !m_wSelectedIcon)
			return;

		m_wSelectedEffect.SetVisible( m_bSelected );
		m_wSelectedIcon.SetVisible(false);
		m_pStorageUI.SetSlotSelected( this, m_bSelected );
	}
	//------------------------------------------------------------------------------------------------
	void SetSelected( bool select )
	{
		//TODO: show the selected effect should be done as a component
		if (!m_pItem)
		{
			m_bSelected = false;
			return;
		}

		m_bSelected = select;
		if (!m_wSelectedEffect)
			return;

		m_wSelectedEffect.SetVisible(select);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetEnabled( bool enable )
	{
		if( enable )
			m_widget.SetOpacity( BUTTON_OPACITY_ENABLED );
		else
			m_widget.SetOpacity( BUTTON_OPACITY_DISABLED );
			
		m_widget.SetEnabled( enable );
	}
	
	bool OnDrop(SCR_InventorySlotUI slot)
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	// ! 0 - Disable for move
	// ! 1 - Enable for move
	// ! 2 - Reset
	void SetEnabledForMove( int iSelect )
	{
		switch ( iSelect )
		{
			case 0:
			{
				m_widget.SetEnabled( false );
				if ( m_wMoveEffect )
					m_wMoveEffect.SetVisible( false );	
			} break;
			
			case 1:
			{
				m_widget.SetEnabled( true );
				if ( m_wMoveEffect )
					m_wMoveEffect.SetVisible( true );	
			
			} break;
			
			case 2:
			{
				m_widget.SetEnabled( true );
				if ( m_wMoveEffect )
					m_wMoveEffect.SetVisible( false );	
			} break;
			
		}

	}
		
	//------------------------------------------------------------------------------------------------
	InventoryItemComponent GetInventoryItemComponent() { return m_pItem; }
	//------------------------------------------------------------------------------------------------
	BaseInventoryStorageComponent GetAsStorage() { return BaseInventoryStorageComponent.Cast( m_pItem ); }
	
	//------------------------------------------------------------------------------------------------
	// ! Returns the UI storage the slot is associated with
	SCR_InventoryStorageBaseUI GetStorageUI() { return m_pStorageUI; }	
	
	
	//------------------------------------------------------------------------------------------------
	protected UIInfo GetItemDetails()
	{
		if( !m_Attributes )
			return null;
		return m_Attributes.GetUIInfo();
	}
	
	//------------------------------------------------------------------------------------------------
	string GetItemName()
	{
		UIInfo itemDetails =  GetItemDetails();
		if( !itemDetails )
			return string.Empty;
		return itemDetails.GetName();	
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetItemDetails()
	{
		UIInfo itemDetails =  GetItemDetails();
		if(!itemDetails)
			return;
		
		if(m_pStorageUI)
		{
			SCR_InventoryUIInfo inventoryItemDetails = SCR_InventoryUIInfo.Cast(itemDetails);
			
			if (inventoryItemDetails)
				m_pStorageUI.ShowItemDetails(inventoryItemDetails.GetInventoryItemName(m_pItem), inventoryItemDetails.GetInventoryItemDescription(m_pItem), m_Attributes.GetWeight().ToString());
			else 
				m_pStorageUI.ShowItemDetails(itemDetails.GetName(), itemDetails.GetDescription(), m_Attributes.GetWeight().ToString());
		}
	}			
	
	//------------------------------------------------------------------------------------------------	
	void ClearItemDetails()
	{
		if( m_pStorageUI )
			m_pStorageUI.ShowItemDetails( "", "", "" );
	}
	
	
	//------------------------------------------------------------------------------------------------
	protected void SetImage( string resource, string imageName )
	{
		if ( resource == string.Empty || imageName == string.Empty || m_wIcon == null )
			return;
		
		m_wIcon.LoadImageFromSet( 0, resource, imageName );
		int width, height;
		m_wIcon.GetImageSize( 0, width, height ); 
		m_wIcon.SetSize( 32, 32 );	//TODO force this		
	}			

	//------------------------------------------------------------------------------------------------	
	protected SCR_InventoryStorageManagerComponent GetInventoryManager()	{ return m_pStorageUI.GetInventoryManager(); }
	
	//------------------------------------------------------------------------------------------------	
	bool RemoveItem()
	{
		SCR_InventoryStorageManagerComponent invMan = GetInventoryManager();
		if (!invMan || !invMan.CanMoveItem(m_pItem.GetOwner()))
			return false;
		InventoryStorageSlot pSlot = m_pItem.GetParentSlot();
		if( !pSlot )
			return false;
		auto pStorage = pSlot.GetStorage();
		if( !pStorage )
			return false;
		if(!invMan.CanRemoveItemFromStorage(m_pItem.GetOwner(), pStorage))
			return false;
		
		//TODO: return the check back once the true/false issue is solved
		//if(!invMan().TryRemoveItemFromInventory(m_pItem.GetOwner(), pStorage))
		//return false;		//can't be removed, exit				

		invMan.TryRemoveItemFromInventory(m_pItem.GetOwner(), pStorage);
		
		if( !m_pStorageUI )
			return false;
		if ( m_widget )
		{		
			m_widget.RemoveHandler( this );
			m_widget.RemoveFromHierarchy();
		}
		m_pStorageUI.RemoveSlotUI( this );
		//delete this;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------	
	//! stores the type of the functionality of the item in the slot
	void SetItemFunctionality()
	{
		if(!m_pItem)
			return;
		IEntity item = m_pItem.GetOwner();
		if (!item)
			return;
		
		m_FuelManager = SCR_FuelManagerComponent.Cast(item.FindComponent(FuelManagerComponent));
		if (m_FuelManager && m_FuelManager.GetTotalMaxFuel() > 0)
		{			
			InitFuelAmount();
			return;
		} 

		if (SCR_GadgetComponent.Cast(item.FindComponent(SCR_GadgetComponent)))
		{
			m_eSlotFunction = ESlotFunction.TYPE_GADGET;
			if (MagazineComponent.Cast(item.FindComponent(MagazineComponent)))
				SetAmmoType();

			return;
		}
		
		if (MagazineComponent.Cast(item.FindComponent(MagazineComponent)))
		{
			m_eSlotFunction = ESlotFunction.TYPE_MAGAZINE;
			SetAmmoCount();
			SetAmmoType();

			return;
		} 
		
		if (WeaponComponent.Cast(item.FindComponent(WeaponComponent)))
		{
			m_eSlotFunction = ESlotFunction.TYPE_WEAPON;
			SetAmmoType();
			return;
		}
	}
	
	//------------------------------------------------------------------------------------------------	
	SCR_CharacterInventoryStorageComponent GetCharacterStorage(IEntity entity)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(entity);
		if (!character)
			return null;
		
		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return null;
		
		SCR_InventoryStorageManagerComponent storageManager = GetInventoryManager();
		if (!storageManager)
			return null;
		
		return storageManager.GetCharacterStorage();
	}
	
	//------------------------------------------------------------------------------------------------	
	void UseItem(IEntity player, SCR_EUseContext context)
	{
		if (!m_pItem)
			return;
		
		IEntity item = m_pItem.GetOwner();
		if (!item)
			return;
		
		SCR_CharacterInventoryStorageComponent storage = GetCharacterStorage(player);
		if (!storage)
			return;
		
		if (storage.UseItem(item, m_eSlotFunction, context))
			Refresh();
		
		// if we are using a bandage from inventory, inform analytics.
		SCR_ConsumableItemComponent consumableComp = SCR_ConsumableItemComponent.Cast(item.FindComponent(SCR_ConsumableItemComponent));
		if (consumableComp && consumableComp.GetConsumableType() == SCR_EConsumableType.BANDAGE && context == SCR_EUseContext.FROM_INVENTORY)
			SCR_AnalyticsApplication.GetInstance().UseHealingFromInventory();
	}
	
	//------------------------------------------------------------------------------------------------	
	bool CanUseItem(IEntity player)
	{
		if (!m_pItem)
			return false;
		
		IEntity item = m_pItem.GetOwner();
		if (!item)
			return false;
		
		SCR_CharacterInventoryStorageComponent storage = GetCharacterStorage(player);
		if (!storage)
			return false;
		
		return storage.CanUseItem(item, m_eSlotFunction);
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool ShouldVehicleSlotBeVisible(SCR_InventoryVehicleVisibilityAttribute attr)
	{
		IEntity vehicle = m_pItem.GetOwner();
		SCR_VehicleDamageManagerComponent dm = SCR_VehicleDamageManagerComponent.Cast(vehicle.FindComponent(SCR_VehicleDamageManagerComponent));
		if (dm && dm.IsDestroyed())
			return false;

		ChimeraCharacter player = ChimeraCharacter.Cast(GetGame().GetPlayerController().GetControlledEntity());

		if (attr.GetVisibleInVehicleOnly() && !player.IsInVehicle())
			return false;

		if (!attr.GetVisibleForVehicleFactionOnly())
			return false;

		if (!Vehicle.Cast(vehicle))
			vehicle = vehicle.GetParent();

		FactionAffiliationComponent vehicleFaction = FactionAffiliationComponent.Cast(vehicle.FindComponent(FactionAffiliationComponent));
		if (!vehicleFaction || !vehicleFaction.GetAffiliatedFaction())
			return true;

		FactionAffiliationComponent playerFaction = FactionAffiliationComponent.Cast(player.FindComponent(FactionAffiliationComponent));
		if (!playerFaction || !playerFaction.GetAffiliatedFaction())
			return false;

		return (playerFaction.GetAffiliatedFaction() == vehicleFaction.GetAffiliatedFaction());
	}

	//------------------------------------------------------------------------------------------------	
	// !
	protected void UpdateVolumeBarValue()
	{
		if ( m_ProgressBar && m_pItem )
		{
			BaseInventoryStorageComponent pStorage = GetAsStorage();
			if ( !pStorage )
				return;
			
			string name = ""; 
			if ( pStorage.GetAttributes() && pStorage.GetAttributes().GetUIInfo() )
			{
				name = pStorage.GetAttributes().GetUIInfo().GetName();
			}
			
			float fOccupied = 0.0;
			//TODO: The folowing part needs refactor! This enumeration of attached storages is used on more places.
			if ( pStorage.Type() == ClothNodeStorageComponent )
			{
				array<BaseInventoryStorageComponent> pOwnedStorages = new array<BaseInventoryStorageComponent>();
				pStorage.GetOwnedStorages( pOwnedStorages, 1, false );
				foreach ( BaseInventoryStorageComponent pSubStorage : pOwnedStorages )
				{
					if (SCR_UniversalInventoryStorageComponent.Cast(pSubStorage))				
						fOccupied += pSubStorage.GetOccupiedSpace();
				}
			}
			else
			{
				fOccupied = pStorage.GetOccupiedSpace();
			}
			float fCapacity = pStorage.GetMaxVolumeCapacity();
			m_ProgressBar.SetProgressRange(0, fCapacity);
			m_ProgressBar.SetCurrentProgress(fOccupied);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateAmmoCount()
	{
		if (!m_ProgressBar)
			return;
		if ( m_eSlotFunction != ESlotFunction.TYPE_MAGAZINE )
			return;
		if( !m_pItem )
			return;

		if ( m_aAmmoCountActual > -1 && m_aAmmoCountMax != 1 )
		{
			m_ProgressBar.SetProgressRange(0, m_aAmmoCountMax);
			m_ProgressBar.SetCurrentProgress(m_aAmmoCountActual);
		}
	}

	//------------------------------------------------------------------------------------------------	
	//!
	int GetAmmoCount() { return m_aAmmoCountActual; }
		
	//------------------------------------------------------------------------------------------------
	protected void UpdateStackNumber()
	{
		if ( !m_wStackNumber )
			return;

		m_wStackNumber.SetVisible(m_iStackNumber > 1);
		m_wStackNumber.SetText( m_iStackNumber.ToString() );
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateWeaponAmmoCount()
	{
		if (!m_pItem)
			return;
		if (!m_wMagazineNumber)
			return;
		// Check if the storage is vicinity. We don't want to display ammo counts for weapons that are on the ground.
		if (SCR_InventoryStorageLootUI.Cast(GetStorageUI()))
			return;
		BaseWeaponComponent weaponComp = BaseWeaponComponent.Cast( m_pItem.GetOwner().FindComponent( BaseWeaponComponent ) );
		if (!weaponComp)
			return;
		BaseMuzzleComponent weaponMuzzleComp = weaponComp.GetCurrentMuzzle();
		if (!weaponMuzzleComp)
			return;

		
		int currentAmmoCount = weaponMuzzleComp.GetAmmoCount();
		int maxAmmoCount = weaponMuzzleComp.GetMaxAmmoCount();
		int magazineCount = GetInventoryManager().GetMagazineCountByWeapon(weaponComp);
		
		m_wMagazineNumber.SetText("+" + magazineCount);
		m_wMagazineNumber.SetVisible(true);

		if (magazineCount > 0 && maxAmmoCount > 0)
			m_wCurrentMagazineAmmoCount.SetMaskProgress(Math.InverseLerp(0, maxAmmoCount, currentAmmoCount));
		else if (maxAmmoCount == 0)
			m_wCurrentMagazineAmmoCount.SetMaskProgress(0);

		m_wMagazineNumber.SetVisible(magazineCount > 0);

		if (weaponMuzzleComp.IsInherited(MuzzleInMagComponent))
			m_wCurrentMagazine.SetVisible(weaponMuzzleComp.IsCurrentBarrelChambered());
		else		
			m_wCurrentMagazine.SetVisible(true);
	}
	
	//------------------------------------------------------------------------------------------------
	void Refresh()
	{
		UpdateVolumeBarValue();
		UpdateStackNumber();
		UpdateAmmoCount();
		UpdateWeaponAmmoCount();
	}
		
	
	//------------------------------------------------------------------------------------------------	
	//!Removes just the UI slot
	void Destroy()
	{
		if (m_pStorageUI.GetInventoryMenuHandler())
			m_pStorageUI.GetInventoryMenuHandler().UnregisterUIStorage( this );

		if (m_widget)
		{
			m_widget.RemoveHandler( this );
			m_widget.RemoveFromHierarchy();
			m_widget = null;
		}

		if (m_wPreviewImage)
		{
			ItemPreviewManagerEntity manager = m_pStorageUI.GetInventoryMenuHandler().GetItemPreviewManager();
			if (manager)
			{
				ItemPreviewWidget renderPreview = ItemPreviewWidget.Cast( m_wPreviewImage );
				if (renderPreview)
					manager.SetPreviewItem(renderPreview, null);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void OnChangeLockState()
	{
		if (!m_pItem)
			return;

		if (m_wItemLockThrobber)
			m_wItemLockThrobber.SetVisible(m_pItem.IsLocked());
	}
	
	//------------------------------------------------------------------------------------------------
	void SetSelectedQuickSlot( bool select )
	{
		m_bSelected = select;
		if (!m_wSelectedEffect || !m_wSelectedIcon)
			return;
		m_wSelectedEffect.SetVisible( select );
		m_wSelectedIcon.SetVisible(false);
	}
	
	//------------------------------------------------------------------------------------------------
	ECommonItemType GetCommonItemType()
	{
		if ( !m_Attributes )
			return ECommonItemType.NONE;
		return m_Attributes.GetCommonType();
	}
	
	//------------------------------------------------------------------------------------------------
	
	
	//------------------------------------------------------------------------ COMMON METHODS ----------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------	
	override bool OnFocus(Widget w, int x, int y)
	{
		OnSlotFocused();
		return false;
	}
	
	//------------------------------------------------------------------------------------------------	
	override bool OnFocusLost(Widget w, int x, int y)
	{
		OnSlotFocusLost();
		return false;
	}
	
	//------------------------------------------------------------------------------------------------	
	override bool OnChange(Widget w, bool finished)
	{
		ClearItemDetails();
		return false;
	}
	/*
	//------------------------------------------------------------------------------------------------	
	override bool OnMouseButtonDown( Widget w, int x, int y, int button )
	{
		m_mouseButtonDown = true;
		PrintFormat( "INV: widget: %1, ( %2, %3 ), button: %4 DOWN", w, x, y, button );
		return false;
	}
	
	//------------------------------------------------------------------------------------------------	
	override bool OnMouseButtonUp( Widget w, int x, int y, int button )
	{
		m_mouseButtonDown = false;
		PrintFormat( "INV: widget: %1, ( %2, %3 ), button: %4 UP", w, x, y, button );
		return false;
	}
	*/

	//------------------------------------------------------------------------------------------------	
	Widget GetWidget()	{ return m_widget; }
	
	//------------------------------------------------------------------------------------------------	
	int GetColumnSize() { return m_iSizeX; }
	
	//------------------------------------------------------------------------------------------------	
	int GetRowSize() { return m_iSizeY; }
		
	//------------------------------------------------------------------------------------------------
	void SetSlotSize( ESlotSize slotSize) { m_ESlotSize = slotSize; }
	
	//------------------------------------------------------------------------------------------------
	ESlotSize GetSlotSize()	{ return m_ESlotSize; }
	
	//------------------------------------------------------------------------------------------------	
	void SetPage( int iPage )	{ m_iPage = iPage; }
	
	//------------------------------------------------------------------------------------------------	
	int GetPage()	{ return m_iPage; }
	
	//------------------------------------------------------------------------------------------------	
	//! What functionality the item in the slot has? ( weapon, magazine, health, consumable... )
	ESlotFunction GetSlotedItemFunction() { return m_eSlotFunction; }
	
	//------------------------------------------------------------------------------------------------
	void SetIcon( ResourceName texture )
	{
		m_wIcon = ImageWidget.Cast( m_widget.FindAnyWidget( "icon" ) );
		if ( !m_wIcon )
			return;
		m_wIcon.LoadImageTexture( 0, texture );
		m_wIcon.SetVisible( true );
	}
		
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached( Widget w )
	{
		Init();	
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetItemResource()
	{
		if (!m_pItem)
			return ResourceName.Empty;
		return m_pItem.GetOwner().GetPrefabData().GetPrefabName();
	}

	//------------------------------------------------------------------------------------------------
	bool IsStacked()
	{
		return (m_iStackNumber > 1);
	}

	//------------------------------------------------------------------------------------------------		
	void CheckCompatibility(SCR_InventorySlotUI slot)
	{
		if (!m_widget)
			return;

		SCR_InventoryMenuUI menu = SCR_InventoryMenuUI.Cast(GetGame().GetMenuManager().FindMenuByPreset(ChimeraMenuPreset.Inventory20Menu));
		if (!menu)
			return;

		Widget incompatible = m_widget.FindAnyWidget("Incompatible");
		if (!incompatible)
			return;

		if (!slot)
		{
			incompatible.SetVisible(false);
			return;
		}

		if (slot == this)
			return;

		if (slot.GetSlotedItemFunction() != ESlotFunction.TYPE_MAGAZINE)
			return;

		if (!m_pItem)
			return;

		IEntity owner = m_pItem.GetOwner();
		BaseWeaponComponent weapon = BaseWeaponComponent.Cast(owner.FindComponent(BaseWeaponComponent));
		if (!weapon)
			return;

		InventoryItemComponent itemComp = slot.GetInventoryItemComponent();
		if (!itemComp)
			return;

		MagazineComponent magComp = MagazineComponent.Cast(itemComp.GetOwner().FindComponent(MagazineComponent));
		if (!magComp)
			return;

		BaseMagazineWell well = magComp.GetMagazineWell();
		if (!well)
			return;

		BaseMuzzleComponent muzzle = weapon.GetCurrentMuzzle();
		if (!muzzle)
			return;

		BaseMagazineWell muzzleWell = muzzle.GetMagazineWell();
		if (!muzzleWell)
			return;

		if (!Turret.Cast(owner) && !menu.IsWeaponEquipped(owner) || slot.IsInherited(SCR_ArsenalInventorySlotUI))
		{
			incompatible.SetVisible(true);
			return;
		}

		incompatible.SetVisible(!well.Type().IsInherited(muzzleWell.Type()));
	}

	//------------------------------------------------------------------------------------------------
	bool IsCompatible()
	{
		Widget incompatible = m_widget.FindAnyWidget("Incompatible");
		if (!incompatible)
			return false;

		return !incompatible.IsVisible();
	}

	//------------------------------------------------------------------------------------------------
	void SCR_InventorySlotUI( InventoryItemComponent pComponent = null, SCR_InventoryStorageBaseUI pStorageUI = null, bool bVisible = true, int iSlotIndex = -1, SCR_ItemAttributeCollection pAttributes = null )
	{
		m_pStorageUI = pStorageUI;	
		m_bVisible = bVisible;
		m_iQuickSlotIndex = iSlotIndex;
		m_Attributes = pAttributes;
		UpdateReferencedComponent( pComponent );
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_InventorySlotUI()
	{
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		
		if (m_FuelManager && m_wProgressBar)
			m_FuelManager.GetOnFuelChanged().Remove(OnFuelAmountChanged);
	}
};