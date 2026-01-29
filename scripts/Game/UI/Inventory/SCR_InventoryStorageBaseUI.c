//------------------------------------------------------------------------------------------------

//! UI Script
//! Inventory of type: non-uniform grid storage

class SCR_InventoryStorageBaseUI : ScriptedWidgetComponent
{
	static ref map<SCR_ArsenalInventorySlotUI, IEntity> ARSENAL_SLOT_STORAGES = new map<SCR_ArsenalInventorySlotUI, IEntity>();
	
	const float													STORAGE_NON_HIGHLIGHTED_OPACITY = 0.35;
	const string												ARSENAL_TITLE_LAYOUT 		= "{E668FD2152D06CF9}UI/layouts/Menus/Inventory/ArsenalInventoryTraverseTitlePanel.layout";
	const string												SUPPLY_TITLE_LAYOUT 		= "{4CAAD8087F93ABE6}UI/layouts/Menus/Inventory/SupplyInventoryTraverseTitlePanel.layout";
	const string												GENERIC_CONTAINER_PREFAB	= "{9F0C54A3740F7FC9}Prefabs/Props/Military/SupplyBox/SupplyPortableContainers/SupplyPortableContainer_01/SupplyContainerGeneric.et";
	const string 												STORAGE_TITLE_LAYOUT 		= "{1E788C30AF7F76E9}UI/layouts/Menus/Inventory/InventoryTraverseTitlePanel.layout";
	const string												SLOT_LAYOUT_1x1_EMPTY 		= "{B11F8BE49DFB62A2}UI/layouts/Menus/Inventory/Inventory20Slot_1x1_empty.layout";
	protected const int											SLOT_LAYOUT_WIDTH			= 66;
	protected ChimeraCharacter									m_Player;
	protected SCR_InventoryStorageManagerComponent				m_InventoryManager;						//manager - handles all the adding / removing operations
	protected SCR_CharacterInventoryStorageComponent			m_InventoryStorage;						//top-most character's inventory component
	protected InputManager										m_pInputManager;
	protected BaseInventoryStorageComponent						m_Storage;								//linked storage
	protected SCR_ResourceComponent								m_ResourceComponent;
	protected SCR_ResourceConsumer								m_ResourceConsumer;
	protected SCR_InventoryMenuUI								m_MenuHandler;
	protected ref array<SCR_InventorySlotUI>					m_aSlots				= {};
	protected ref array<Widget>									m_aEmptySlotWidget;
	protected SCR_InventorySlotUI								m_pSelectedSlot			= null;
	protected SCR_InventorySlotUI								m_pPrevSelectedSlot		= null;
	protected SCR_InventorySlotUI								m_pFocusedSlot			= null;
	protected SCR_InventorySlotUI								m_pLastFocusedSlot		= null;			//memorize the last focused slot before leaving the actual UI storage
	
	protected WorkspaceWidget 									m_workspaceWidget 		= null;
	protected Widget											m_widget 				= null;
	protected GridLayoutWidget									m_wGrid;								//non-uniform grid
	protected string											m_sGridPath				= "GridLayout0";
	protected Widget 											m_wDropContainer;
	protected LoadoutAreaType									m_eSlotAreaType			= null;
	protected bool												m_bEnabled				= false;			//if it is enabled for manipulation ( can be removed or an item can be moved into )
	protected bool												m_bSelected				= false;
	protected bool												m_bShown				= true;
	protected bool												m_bIsArsenal;
	protected ItemPreviewWidget									m_wItemStorage, m_wTraverseItemStorage;
	protected RenderTargetWidget								m_wPreviewImage, m_wTraversePreviewImage;
	
	protected ref SCR_Matrix									m_iMatrix;
	protected int												m_iMaxRows, m_iMaxColumns, m_iPageSize;
	//protected int												m_iPage					= 0;
	protected int												m_iNrOfPages			= 0;
	protected TextWidget 										m_wStorageName, m_wCapacityPercentageText, m_wWeightText, m_wResourceAvailableText, m_wResourceStoredText;
	protected Widget											m_wCapacityDisplay, m_wWeightDisplay, m_wResourceAvailableDisplay, m_wResourceStoredDisplay;
	protected SizeLayoutWidget									m_wWidthOverride;
	protected int												m_iLastShownPage		= 0;
	protected SCR_SpinBoxPagingComponent 						m_wPagingSpinboxComponent;
	protected ref array<BaseInventoryStorageComponent> 			m_aTraverseStorage 			= {};
	protected ref array<Widget>									m_aTraverseTitle 			= {};
	protected ref array<SCR_ResourceComponent>					m_aResourceCompsInStorage	= {};
	protected ButtonWidget 										m_wButton, m_wLastCloseTraverseButton, m_wCloseStorageButton;
	protected ref SCR_ResourceSystemSubscriptionHandleBase		m_ResourceSubscriptionHandleConsumer;
	
	protected LocalizedString m_sSuppliesStoredFormat = "#AR-Campaign_BaseSuppliesAmount";
	protected LocalizedString m_sSuppliesAvailableFormat = "#AR-Supplies_Arsenal_Availability";
	protected SCR_InventoryProgressBar m_ProgressBar;
	protected Widget m_wProgressBar;
	protected Widget m_wWarningOverlay;
	protected static const ref Color m_WeightDefault = new Color(0.73, 0.73, 0.73, 1);
	
	protected static ref ScriptInvokerVoid s_OnArsenalEnter;	
	//------------------------------------------------------------------------ USER METHODS ------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	static ScriptInvokerVoid GetOnArsenalEnter()
	{
		if (!s_OnArsenalEnter)
			s_OnArsenalEnter = new ScriptInvokerVoid();
		
		return s_OnArsenalEnter;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_EAnalyticalItemSlotType GetAnalyticalItemSlotType()
	{
		return SCR_EAnalyticalItemSlotType.OTHER;
	}
	
	//------------------------------------------------------------------------------------------------
	void Init()
	{
		m_pInputManager = GetGame().GetInputManager();
		//get the workspace
		if ( m_workspaceWidget == null )
			m_workspaceWidget = GetGame().GetWorkspace();
						
		//Get player
		// TODO: make it independent on player ( might be used for example in editor to change the loadout of a character )
		PlayerController pPlayerController = GetGame().GetPlayerController();
		if( !pPlayerController )
			return;
		m_Player = ChimeraCharacter.Cast(pPlayerController.GetControlledEntity());
		if( !m_Player )
			return;
		
		//Get inventory manager and the actual character's topmost storage
		m_InventoryManager = SCR_InventoryStorageManagerComponent.Cast( m_Player.FindComponent( SCR_InventoryStorageManagerComponent ) );
		m_InventoryStorage = SCR_CharacterInventoryStorageComponent.Cast( m_Player.FindComponent( SCR_CharacterInventoryStorageComponent ) );
		m_wGrid = GridLayoutWidget.Cast( m_widget.FindAnyWidget( m_sGridPath ) );
		m_wStorageName = TextWidget.Cast( m_widget.FindAnyWidget( "StorageName" ) );
		m_wItemStorage = ItemPreviewWidget.Cast( m_widget.FindAnyWidget( "itemStorage" ) );
		m_wCapacityPercentageText = TextWidget.Cast(m_widget.FindAnyWidget("CapacityPercentageText"));
		m_wWeightText = TextWidget.Cast(m_widget.FindAnyWidget("WeightText"));
		m_wWeightDisplay = m_widget.FindAnyWidget("Weight");
		m_wWidthOverride = SizeLayoutWidget.Cast(m_widget.FindWidget("TitleWidthOverride"));
		m_wWarningOverlay = m_widget.FindAnyWidget("WarningOverlay");;

		if (m_wWeightDisplay)
			m_wWeightDisplay.SetVisible(true);
		
		m_wProgressBar = m_widget.FindAnyWidget("ProgressBar");
		if (m_wProgressBar)
		{
			m_ProgressBar = SCR_InventoryProgressBar.Cast(m_wProgressBar.FindHandler(SCR_InventoryProgressBar));
			m_wProgressBar.SetVisible(true);
		}
		
		InitPaging();
		SetRowsAndColumns();
		
		//Set the proper ratio according to the number of columns and rows
		SizeLayoutWidget wSizeContainer = SizeLayoutWidget.Cast( m_widget.FindAnyWidget( "SizeLayout0" ) );
		if ( wSizeContainer )
			wSizeContainer.SetAspectRatio( m_iMaxColumns / m_iMaxRows );
		
		if (m_wWidthOverride)
			m_wWidthOverride.SetWidthOverride(m_iMaxColumns * SLOT_LAYOUT_WIDTH);
		
		//Get the storage the UI container will be showing the content from
		if (!m_InventoryStorage)
			m_Storage = m_InventoryStorage.GetStorageFromLoadoutSlot( m_eSlotAreaType );
		if (!m_Storage)
			m_Storage = m_InventoryStorage.GetWeaponStorage();
		if (!m_Storage)
			return;

		if (Type() != SCR_InventoryStorageLootUI && m_wWeightDisplay)
		{
			if (Type() != SCR_InventoryStorageWeaponsUI && m_wCapacityDisplay)
			{
				
				m_wCapacityDisplay.SetVisible(true);
				UpdateVolumePercentage( GetOccupiedVolumePercentage( m_Storage ) );
			}

			m_wWeightDisplay.SetVisible(true);
			UpdateTotalWeight( GetTotalRoundedUpWeight( m_Storage ) );
		}

		SetPreviewItem();
		if (!m_wStorageName)
			return;
		ItemAttributeCollection attributes = m_Storage.GetAttributes();
		if (!attributes)
			return;
		UIInfo uiInfo = attributes.GetUIInfo();
		if (!uiInfo)
			return;
		m_wStorageName.SetText(uiInfo.GetName());

		m_wResourceStoredDisplay = m_widget.FindAnyWidget("ResourceDisplayStored");
		if (m_wResourceStoredDisplay)
			m_wResourceStoredText = TextWidget.Cast(m_wResourceStoredDisplay.FindAnyWidget("ResourceText"));
		
		m_wResourceAvailableDisplay = m_widget.FindAnyWidget("ResourceDisplayAvailable");
		if (m_wResourceAvailableDisplay)
			m_wResourceAvailableText = TextWidget.Cast(m_wResourceAvailableDisplay.FindAnyWidget("ResourceText"));
		
		if (!m_wResourceStoredText || !m_wResourceAvailableText)
			return;
		
		//~ Will ignore storage components of vehicle slots if they already have a storage assigned
		if (m_Storage && m_Storage.GetOwner())
			m_ResourceComponent = SCR_ResourceComponent.FindResourceComponent(m_Storage.GetOwner(), true);
		
		if (m_ResourceComponent)
			RefreshConsumer();
		
		if (!m_ResourceConsumer)
			return;
		
		SCR_ResourcePlayerControllerInventoryComponent resourceInventoryPlayerComponent = SCR_ResourcePlayerControllerInventoryComponent.Cast(GetGame().GetPlayerController().FindComponent(SCR_ResourcePlayerControllerInventoryComponent));
		
		if (!resourceInventoryPlayerComponent)
			return;
		
		m_ResourceComponent.TEMP_GetOnInteractorReplicated().Insert(RefreshConsumer);
		
		RplId resourceInventoryPlayerComponentRplId = Replication.FindId(resourceInventoryPlayerComponent);
		
		if (!resourceInventoryPlayerComponentRplId.IsValid())
			return;
		
		m_ResourceSubscriptionHandleConsumer = GetGame().GetResourceSystemSubscriptionManager().RequestSubscriptionListenerHandle(m_ResourceConsumer, resourceInventoryPlayerComponentRplId);
	}
	
	//------------------------------------------------------------------------------------------------
	void InitPaging()
	{
		if( m_InventoryStorage )
		{
			m_wPagingSpinboxComponent = SCR_SpinBoxPagingComponent.GetSpinBoxPagingComponent("SpinBoxContainer", m_widget);
			if ( m_wPagingSpinboxComponent )
			{
				m_wPagingSpinboxComponent.SetPageCount( 0 );
				m_wPagingSpinboxComponent.SetCurrentItem( 0, false );
				m_wPagingSpinboxComponent.SetButtonsActive(false);
				SetPagingActive(false);
			}
			
		}
	}

	//------------------------------------------------------------------------------------------------
	bool IsArsenal()
	{
		return m_bIsArsenal;
	}

	//------------------------------------------------------------------------------------------------
	void SetPagingActive(bool active)
	{
		if (!m_wPagingSpinboxComponent)
			return;

		m_wPagingSpinboxComponent.m_OnChanged.Remove(Action_ChangePage);
		if (active)
			m_wPagingSpinboxComponent.m_OnChanged.Insert(Action_ChangePage);

		m_wPagingSpinboxComponent.SetButtonsActive(active);
		m_wPagingSpinboxComponent.SetButtonsVisible(active);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocus( Widget w, int x, int y )
	{
		if ( m_MenuHandler )
			m_MenuHandler.OnContainerHovered( this );
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost( Widget w, int x, int y )
	{
		if ( m_MenuHandler )
			m_MenuHandler.OnContainerLeft( this );
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		if ( m_MenuHandler )
			m_MenuHandler.OnContainerHovered( this );

		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave( Widget w, Widget enterW, int x, int y)
	{
		if (enterW && w && w.FindAnyWidget(enterW.GetName()))
			return false;
		
		if ( m_MenuHandler )
			m_MenuHandler.OnContainerLeft( this );

		return false;
	}
	
	bool OnDrop(SCR_InventorySlotUI slot)
	{
		return false;
	}
/*	
	override bool OnUpdate(Widget w)
	{
		Widget w2 = w;
		PrintFormat( "INV: widget: %1", WidgetManager.GetWidgetUnderCursor() );
		return false;
	}
*/
	
	//------------------------------------------------------------------------------------------------
	void SetRowsAndColumns()
	{
		for ( int m = 0; m < m_iMaxRows; m++ )
		{
			m_wGrid.SetRowFillWeight( m, 0 );
			for ( int n = 0; n < m_iMaxColumns; n++ )
			{
				m_wGrid.SetColumnFillWeight( n, 0 );
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	private BaseInventoryStorageComponent PushState(BaseInventoryStorageComponent storage)
	{
		m_aTraverseStorage.Insert(storage);
		return storage;
	}

	//------------------------------------------------------------------------------------------------
	private BaseInventoryStorageComponent PopState(int traverseStorageIndex)
	{
		if ( m_aTraverseStorage.IsEmpty() )
			return null;

		if ( traverseStorageIndex == -1 )
			return null;
				
		for ( int i = m_aTraverseStorage.Count() - 1; i >= traverseStorageIndex; --i)
		{
			m_aTraverseStorage.Remove(i);
			if (m_aTraverseStorage.IsEmpty())
				return null;
		}
		return m_aTraverseStorage[ m_aTraverseStorage.Count() - 1 ];
	}

	//------------------------------------------------------------------------------------------------
	BaseInventoryStorageComponent GetCurrentNavigationStorage()
	{
		if (m_aTraverseStorage.Count() > 0)
		{
			return m_aTraverseStorage[m_aTraverseStorage.Count() - 1];
		}
		return m_Storage;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsTraversalAllowed()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void Traverse(BaseInventoryStorageComponent storage)
	{
		DeleteSlots();
 		PushState( storage );
		FillItemsFromStorage( storage );
		SortSlots();
		ShowPage( 0 );
		
		//play open sound
		if (!m_aTraverseStorage.Find(storage))
		{
			IEntity entity = storage.GetOwner();
			m_InventoryManager.PlayItemSound(entity, SCR_SoundEvent.SOUND_CONTAINER_OPEN);
		}
		
		SCR_InventoryMenuUI inventoryMenu = GetInventoryMenuHandler();
		if (inventoryMenu)
			inventoryMenu.OpenLinkedStorages(storage, false);
	}

	//------------------------------------------------------------------------------------------------
	void Back(int traverseStorageIndex)
	{
		DeleteSlots();
		
		BaseInventoryStorageComponent lastStorage = m_aTraverseStorage.Get(traverseStorageIndex);
		if (lastStorage)
		{
			//play CLOSE sound
			IEntity entity = lastStorage.GetOwner();
			m_InventoryManager.PlayItemSound(entity, SCR_SoundEvent.SOUND_CONTAINER_CLOSE);
			
			//~ Close any linked storages
			SCR_InventoryMenuUI inventoryMenu = GetInventoryMenuHandler();
			if (inventoryMenu)
				inventoryMenu.CloseLinkedStorages(lastStorage);
		}
		
		auto storage = PopState( traverseStorageIndex );
		
		if (!storage)
		{
			Home();
			return;
		}
		
		FillItemsFromStorage( storage );
		SortSlots();
		ShowPage( 0 );
	}
	
	//------------------------------------------------------------------------------------------------
	void Home()
	{
		DeleteSlots();
		m_aTraverseStorage.Clear();
		ClearTraverseTitles();
		CreateSlots();
		SortSlots();
		ShowPage(0);
	}

	//------------------------------------------------------------------------------------------------
	float GetTotalResources(notnull SCR_ResourceConsumer resourceContainer)
	{
		return resourceContainer.GetAggregatedResourceValue();
	}
	
	//------------------------------------------------------------------------------------------------
	float GetMaxResources(notnull SCR_ResourceConsumer resourceContainer)
	{
		return resourceContainer.GetAggregatedMaxResourceValue();
	}
	
	//------------------------------------------------------------------------------------------------
	void GetTraverseStorage( out notnull array<BaseInventoryStorageComponent> aTraverseStorage )
	{
		aTraverseStorage.Copy( m_aTraverseStorage );
	}

	//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
	//------------------------------------------------------------------------------------------------
	void Action_ChangePage( SCR_SpinBoxPagingComponent pSpinBox, int page )
	{
		int slotToFocus = -1;
		
		SortSlots();
		ShowPage(m_wPagingSpinboxComponent.GetCurrentIndex(), slotToFocus);
		
		GetGame().GetCallqueue().CallLater(m_MenuHandler.FocusOnSlotInStorage, 0, false, this, slotToFocus); // call later, otherwise ShowPage() would get triggered again right away
	}
		
	//------------------------------------------------------------------------------------------------
	void Action_PrevPage()
	{
		int slotToFocus = -1;
		
		SortSlots();
		ShowPage(m_iLastShownPage - 1, slotToFocus);
		
		GetGame().GetCallqueue().CallLater(m_MenuHandler.FocusOnSlotInStorage, 0, false, this, slotToFocus); // call later, otherwise ShowPage() would get triggered again right away
	}
	//------------------------------------------------------------------------------------------------
	void Action_NextPage()
	{
		int slotToFocus = -1;
		
		SortSlots();
		ShowPage(m_iLastShownPage + 1, slotToFocus);
		
		GetGame().GetCallqueue().CallLater(m_MenuHandler.FocusOnSlotInStorage, 0, false, this, slotToFocus); // call later, otherwise ShowPage() would get triggered again right away
	}
	//---- REFACTOR NOTE END ----

	//------------------------------------------------------------------------------------------------
	bool IsStorageEnabled() { return m_bEnabled; }
	//------------------------------------------------------------------------------------------------
	bool IsStorageSelected() { return m_bSelected; }
	//------------------------------------------------------------------------------------------------
	bool GetLastShownPage() { return m_iLastShownPage; }
	//------------------------------------------------------------------------------------------------
	int GetPageCount() { return m_iNrOfPages; }
	
	//------------------------------------------------------------------------------------------------
	BaseInventoryStorageComponent GetStorage() { return m_Storage; }
	
	//------------------------------------------------------------------------------------------------
	EquipedWeaponStorageComponent GetWeaponStorage() { return EquipedWeaponStorageComponent.Cast( m_Storage ); }

	//------------------------------------------------------------------------------------------------
	string GetStorageName()
	{
		ItemAttributeCollection attributes = m_Storage.GetAttributes();
		if( !attributes )
			return "none";
		UIInfo uiInfo = attributes.GetUIInfo();
		if( !uiInfo )
			return "none";
		return uiInfo.GetName();
	}
	
	//------------------------------------------------------------------------------------------------
	// !
	SCR_InventorySlotUI GetSelectedSlot() { return m_pSelectedSlot; }
	SCR_InventorySlotUI GetPrevSelectedSlot() { return m_pPrevSelectedSlot; }
	
	//------------------------------------------------------------------------------------------------
	// ! Hides / unhides the storage container
	void Show( bool bShow )
	{
		/* Enable to have the expand / collapse feature
		m_bShown = bShow;
		m_wGrid.SetVisible( m_bShown );
		if ( m_bShown )
		{
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INV_CONTAINER_OPEN, true);
			m_InventoryStorage.SetStorageAsShown( m_Storage );
			if ( m_iNrOfPages > 1 && m_wPagingSpinboxComponent )
			{
				ShowPagesCounter( true, 0, m_iNrOfPages );
			}
		}
		else
		{
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INV_CONTAINER_CLOSE, true);
			m_InventoryStorage.SetStorageAsHidden( m_Storage );
			ShowPagesCounter( false );
		}
		*/
	}

		//------------------------------------------------------------------------------------------------
	// ! Hides / unhides the storage container
	void ToggleShow()
	{
		Show( !m_bShown );
	}
	
	
	//------------------------------------------------------------------------------------------------
	// !
	SCR_InventorySlotUI GetFocusedSlot() { return m_pFocusedSlot; }
	
	//------------------------------------------------------------------------------------------------
	// !
	SCR_InventorySlotUI GetLastFocusedSlot() { return m_pLastFocusedSlot; }
	
	//------------------------------------------------------------------------------------------------
	// ! returns the grid widget of the storage
	Widget GetStorageGrid() { return m_wGrid; }
	//------------------------------------------------------------------------------------------------
	// ! returns the root widget
	Widget GetRootWidget() { return m_widget; }
		
	//------------------------------------------------------------------------------------------------
	// ! returns the inventory menu handler
	SCR_InventoryMenuUI GetInventoryMenuHandler() { return m_MenuHandler; }
		
	//------------------------------------------------------------------------------------------------
	SCR_InventoryStorageManagerComponent GetInventoryManager() { return m_InventoryManager; }
	
	//------------------------------------------------------------------------------------------------
	// ! set the slot id of the storage the content will be shown from
	protected void SetSlotAreaType( LoadoutAreaType eSlotType )	{ m_eSlotAreaType = eSlotType; }
	
	//------------------------------------------------------------------------------------------------
	// !
	LoadoutAreaType GetSlotAreaType()	{ return m_eSlotAreaType; }
	
	//------------------------------------------------------------------------------------------------
	// !
	void GetUISlots( out array<SCR_InventorySlotUI> outArray)
	{
		outArray.Copy( m_aSlots );
	}
	
	//------------------------------------------------------------------------------------------------
	// !
	array<SCR_InventorySlotUI> GetUISlots()
	{
		return m_aSlots;
	}
	
	//------------------------------------------------------------------------------------------------
	// !
	void RemoveSlotUI( SCR_InventorySlotUI pSlot )
	{
		m_aSlots.RemoveItem( pSlot );
		SortSlots();
	}

	//------------------------------------------------------------------------------------------------
	void RefreshSlot(int index);

	//------------------------------------------------------------------------------------------------
	// ! creates empty slot - just visual of the matrix layout
	protected Widget CreateEmptySlotUI( int iRow, int iCol )
	{
		Widget pWidget = m_workspaceWidget.CreateWidgets( SLOT_LAYOUT_1x1_EMPTY, m_wGrid );
		GridSlot.SetColumn( pWidget, iCol );
		GridSlot.SetRow( pWidget, iRow );
		//SizeLayoutSlot.SetHorizontalAlign( pWidget, LayoutHorizontalAlign.Center );
						
		return pWidget;
	}
	
	//------------------------------------------------------------------------------------------------
	// ! creates the slot
	protected SCR_InventorySlotUI CreateSlotUI( InventoryItemComponent pComponent, SCR_ItemAttributeCollection pAttributes = null )
	{
		if ( !pComponent )
		{
			return new SCR_InventorySlotUI( null, this, false, -1, pAttributes );
		}
		else if ( WeaponAttachmentsStorageComponent.Cast( pComponent ) )
		{
			return new SCR_InventorySlotWeaponUI( pComponent, this, false );			//creates the slot
		}
		else if (SCR_ResourceComponent.FindResourceComponent(pComponent.GetOwner()))
		{
			return new SCR_SupplyInventorySlotUI(pComponent, this, false);			//creates the slot
		}
		else if ( BaseInventoryStorageComponent.Cast( pComponent ) )
		{
			return new SCR_InventorySlotStorageEmbeddedUI( pComponent, this, false );			//creates the slot
		}
		else if (pComponent.GetOwner().FindComponent(SCR_ArsenalInventoryStorageManagerComponent))
		{
			SCR_ArsenalInventorySlotUI slotUI = new SCR_ArsenalInventorySlotUI(pComponent, this, false, -1, null);
			BaseInventoryStorageComponent inventoryStorageComponent = GetCurrentNavigationStorage();

			if (inventoryStorageComponent)
			{
				slotUI.SetArsenalResourceComponent(SCR_ResourceComponent.FindResourceComponent(inventoryStorageComponent.GetOwner()));
				SCR_InventoryStorageBaseUI.ARSENAL_SLOT_STORAGES.Insert(slotUI, inventoryStorageComponent.GetOwner());
			}
			
			return slotUI;
		}
		else
		{
			return new SCR_InventorySlotUI( pComponent, this, false );			//creates the slot
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// !
	protected void DeleteSlots()
	{
		foreach ( SCR_InventorySlotUI pSlot: m_aSlots )
		{
			if( pSlot )
			{
				pSlot.Destroy();
			}

		}
		m_aSlots.Clear();
	}
	
	//------------------------------------------------------------------------------------------------
	// !
	private bool ValidateNavigationQueue()
	{
		int traverseSize = m_aTraverseStorage.Count();
		
		if (traverseSize > 0)
		{
			BaseInventoryStorageComponent lastStorage = null;
			ref array<IEntity> rootItems = new array<IEntity>();
			GetAllItems(rootItems);

			for (int i = 0; i < traverseSize; i++)
			{
				BaseInventoryStorageComponent current = m_aTraverseStorage[i];

				if (!current)
					break;
				bool found = false;
				for (int j = 0; j < rootItems.Count(); j++)
				{
					if (!rootItems[j])
						continue;
					
					auto storage = BaseInventoryStorageComponent.Cast(
						rootItems[j].FindComponent(BaseInventoryStorageComponent)
					);
					if (!storage)
						continue;
					BaseInventoryStorageComponent parentStorage;
					if ( current.GetParentSlot() )
						parentStorage = current.GetParentSlot().GetStorage();

					found = storage == current || parentStorage == storage;
					if (found)
						break;
				}

				if (!found)
					break;
				

				// Last storage in queue
				if (i == traverseSize - 1)
				{
					lastStorage = current;
					return true;
				}
				
				rootItems.Clear();
				current.GetAll(rootItems);
			}
			return false;
		}
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void RefreshConsumer()
	{
		m_ResourceConsumer = m_ResourceComponent.GetConsumer(EResourceGeneratorID.VEHICLE_UNLOAD, EResourceType.SUPPLIES);
		
		if (!m_ResourceConsumer)
			m_ResourceConsumer = m_ResourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT_STORAGE, EResourceType.SUPPLIES);
		
		Refresh();
	}

	//------------------------------------------------------------------------------------------------
	void RefreshResources()
	{
		if (m_ResourceComponent)
		{
			float resources, maxResources;
			bool hasStored = false;

			//~ Display stored resources
			if (SCR_ResourceSystemHelper.GetStoredAndMaxResources(m_ResourceComponent, resources, maxResources))
			{
				UpdateStoredResources(resources, maxResources);
				hasStored = true;
				m_wResourceStoredDisplay.SetVisible(true);
			}
			else 
			{
				m_wResourceStoredDisplay.SetVisible(false);
			}
					
			//~ Display available resources
			if (m_ResourceComponent.IsResourceTypeEnabled() && SCR_ResourceSystemHelper.GetAvailableResources(m_ResourceComponent, resources))
			{
				UpdateAvailableResources(resources, hasStored);
				m_wResourceAvailableDisplay.SetVisible(true);
			}
			else 
			{
				m_wResourceAvailableDisplay.SetVisible(false);
			}
			
		}
	}

	//------------------------------------------------------------------------------------------------
	// !
	void Refresh()
	{
		ResourceName focusedItemName;
		if (m_pFocusedSlot)
		{
			focusedItemName = m_pFocusedSlot.GetItemResource(); // remember the slot which was in focus
		}
		else
		{
			SCR_InventorySlotUI focusedSlot = m_MenuHandler.GetCurrentlyFocusedSlot();
			if (focusedSlot && m_aSlots.Contains(focusedSlot))
				focusedItemName = focusedSlot.GetItemResource(); //if for some reason menu was focused on slot from this storage, and it didnt know, lets focus that slot
		}

		if ( ValidateNavigationQueue() )
		{
			int traverseSize = m_aTraverseStorage.Count();
			if (traverseSize > 0)
			{
				ClearTraverseTitles();
				FillItemsFromStorage(m_aTraverseStorage[traverseSize - 1]);
			}
			else
			{
				// Create root layout
				CreateSlots();
			}
		}
		else
		{
			m_iLastShownPage = 0;
			m_aTraverseStorage.Clear();
			ClearTraverseTitles();

			// Create root layout
			CreateSlots();
		}
		SortSlots();
		ShowPage(m_iLastShownPage);
		
		if ( m_Storage )
		{
			UpdateVolumePercentage( GetOccupiedVolumePercentage( m_Storage ) );
			UpdateTotalWeight( GetTotalRoundedUpWeight( m_Storage ) );
		}
		
		array<IEntity> tmpAItemsInStorage = new array<IEntity>();
		BaseInventoryStorageComponent tmpStorage = m_Storage;
		GetAllItemsFromDepth( tmpAItemsInStorage );
		m_Storage = tmpStorage;
		
		SetStorageAsHighlighted(true);
		
		if ( m_pSelectedSlot )
			m_pSelectedSlot.Refresh();

		array<IEntity> itemsInStorage = {};
		GetAllItems(itemsInStorage); // Needs a better solution. Very wasteful.
		CheckIfAnyItemInStorageHasResources(itemsInStorage);
		RefreshResources();

		if (focusedItemName.IsEmpty())
			return;

		foreach (int i, SCR_InventorySlotUI uiSlot : m_aSlots)
		{
			if (uiSlot && uiSlot.GetItemResource() == focusedItemName)
			{
				m_MenuHandler.FocusOnSlotInStorage(this, i);
				return;
			}
		}

		if (!m_aSlots.IsEmpty()) // if this container had a slot which was in focus, then despite the fact that specific prefab is not present, lets focus on something in here, in order to prevent focus being shifted onto a random spot in the inventory
			m_MenuHandler.FocusOnSlotInStorage(this, 0);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateStoredResources(float totalResources, float maxResources)
	{
		if (!m_wResourceStoredText)	
			return;
		
		/*if (m_wWeightDisplay)
			m_wWeightDisplay.SetVisible(false);
		if (m_wCapacityDisplay)
			m_wCapacityDisplay.SetVisible(false);*/
			
		m_wResourceStoredText.SetTextFormat(m_sSuppliesStoredFormat, SCR_ResourceSystemHelper.SuppliesToString(totalResources), SCR_ResourceSystemHelper.SuppliesToString(maxResources));
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateAvailableResources(float totalResources, bool hasStored)
	{
		if (!m_wResourceAvailableText)
			return;
		
		/*if (m_wWeightDisplay)
			m_wWeightDisplay.SetVisible(false);
		if (m_wCapacityDisplay)
			m_wCapacityDisplay.SetVisible(false);*/

		if (!hasStored)
			m_wResourceAvailableText.SetTextFormat(m_sSuppliesAvailableFormat, SCR_ResourceSystemHelper.SuppliesToString(totalResources));
		else 
			m_wResourceAvailableText.SetText(SCR_ResourceSystemHelper.SuppliesToString(totalResources));
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateContainerResources(notnull Widget targetTitle, notnull BaseInventoryStorageComponent targetStorage)
	{
		SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.FindResourceComponent(targetStorage.GetOwner());
		if(!resourceComponent)
			return;
		
		float totalResources, maxResources;
		
		Widget resourceDisplay = targetTitle.FindAnyWidget("ResourceDisplayStored");
		TextWidget resourceText;
		
		if (resourceDisplay)
		{
			resourceText = TextWidget.Cast(resourceDisplay.FindAnyWidget("ResourceText"));
			
			//~ Display stored resources
			if (resourceText && SCR_ResourceSystemHelper.GetStoredAndMaxResources(resourceComponent, totalResources, maxResources))
			{
				resourceText.SetTextFormat(m_sSuppliesStoredFormat, SCR_ResourceSystemHelper.SuppliesToString(totalResources), SCR_ResourceSystemHelper.SuppliesToString(maxResources));
				resourceDisplay.SetVisible(true);
			}
			else 
			{
				resourceDisplay.SetVisible(false);
			}
		}
		
		resourceDisplay = targetTitle.FindAnyWidget("ResourceDisplayAvailable");
		
		if (resourceDisplay)
		{
			resourceText = TextWidget.Cast(resourceDisplay.FindAnyWidget("ResourceText"));
			
			//~ Display available resources
			if (resourceText && SCR_ResourceSystemHelper.GetAvailableResources(resourceComponent, totalResources) && resourceComponent.IsResourceTypeEnabled())
			{
				resourceText.SetTextFormat(m_sSuppliesAvailableFormat, SCR_ResourceSystemHelper.SuppliesToString(totalResources));
				resourceDisplay.SetVisible(true);
				return;
			}
			else 
			{
				resourceDisplay.SetVisible(false);
			}
		}
	}
	
	protected void UpdateOwnedSlots(notnull array<IEntity> pItemsInStorage)
	{
		// TODO: optimize? ( the existing slots must be deleted. The previous algorithm doesn't work any more, since the number of slots is unknown due to the aggregation-stacking of the items of the same type )
		DeleteSlots();
		
		// number of items for which slots are not created, as they are part of the stack in a already exising slot
		int iCompensationOffset = 0;
		array<SCR_InventorySlotUI> slotsToUpdate = {};
					
		InventoryItemComponent iic;
		SCR_ItemAttributeCollection attributes;
		RplComponent rplComp;
		SCR_InventorySlotUI uiSlot;
		foreach (int i, IEntity item : pItemsInStorage)
		{
			if (!item)
				continue;

			iic = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
			if (!iic)
				continue; // item in inventory without iic sounds like insanity but im not going to question safety

			const int slotId = FindItem(iic);
			if (slotId < 0)
			{
				uiSlot = CreateSlotUI(iic);
				if (!uiSlot)
				{
					iCompensationOffset++;
					continue;
				}

				if (i - iCompensationOffset > m_aSlots.Count() - 1)			//if the item is not identical, create a new slow
					m_aSlots.Insert(uiSlot);
				else
					m_aSlots[i - iCompensationOffset++] = uiSlot;

				continue;
			}

			attributes = SCR_ItemAttributeCollection.Cast(iic.GetAttributes());
			if (!attributes || !attributes.IsStackable())
				continue;

			uiSlot = m_aSlots[slotId];
			if (!uiSlot)
				continue;

			rplComp = RplComponent.Cast(item.FindComponent(RplComponent));
			if (rplComp && uiSlot.IsInherited(SCR_SupplyInventorySlotUI))
			{
				uiSlot.IncreaseStackNumberWithRplId(rplComp.Id()); 
				slotsToUpdate.Insert(uiSlot);
			}
			else
			{
				uiSlot.IncreaseStackNumber();		//if it exists, just increase the stacked number and don't create new slot
			}

			iCompensationOffset++;
		}

		foreach (SCR_InventorySlotUI slot : slotsToUpdate)
		{
			slot.OnOwnedSlotsUpdated();
		}
	}

	//------------------------------------------------------------------------------------------------
	void UpdateSlotUI(ResourceName item)
	{
		int id = GetSlotIdForItem(item);

		array<IEntity> pItemsInStorage = {};

		BaseInventoryStorageComponent storage = GetCurrentNavigationStorage();
	
		GetAllItems(pItemsInStorage, storage);

		int count = pItemsInStorage.Count();
		int newStackCount = 0;
		InventoryItemComponent comp;

		for (int i = 0; i < count; ++i)
		{
			if (pItemsInStorage[i].GetPrefabData().GetPrefabName() == item)
			{
				comp = GetItemComponentFromEntity(pItemsInStorage[i]);
				newStackCount++;
			}
		}
		
		m_aSlots[id].UpdateInventorySlot(comp, newStackCount);
	}
	
	//------------------------------------------------------------------------------------------------
	// !
	protected void UpdateOwnedSlots(notnull array<BaseInventoryStorageComponent> pItemsInStorage)
	{
		int count = pItemsInStorage.Count();
		if (count < m_aSlots.Count())
		{
			for (int i = m_aSlots.Count() - count; i > 0; i--)
			{
				auto slotUI = m_aSlots[m_aSlots.Count() - 1];
				if (slotUI)
					slotUI.Destroy();
			}
			
		}
		m_aSlots.Resize(count);
		for (int i = 0; i < count; i++)
		{
			if (m_aSlots[i])
				m_aSlots[i].UpdateReferencedComponent(pItemsInStorage[i]);
			else
				m_aSlots[i] = CreateSlotUI(pItemsInStorage[i]);
		}
	}

	//------------------------------------------------------------------------------------------------
	/*protected void FindAndSetResourceCount(Widget targetTitle, BaseInventoryStorageComponent targetStorage)
	{
		TextWidget resourceText = TextWidget.Cast(targetTitle.FindAnyWidget("ResourceText"));
		if(!resourceText)
			return;
		
		SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.FindResourceComponent(targetStorage.GetOwner());
		if(!resourceComponent)
			return;
		
		SCR_ResourceConsumer resourceConsumer = resourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT_STORAGE, EResourceType.SUPPLIES);
	
		if (!resourceConsumer)
			resourceConsumer = resourceComponent.GetConsumer(EResourceGeneratorID.VEHICLE_UNLOAD, EResourceType.SUPPLIES);
		
		if (!resourceConsumer)
			resourceConsumer = resourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);
		
		if (!resourceConsumer)
			return;
		
		resourceText.SetText(resourceConsumer.GetAggregatedResourceValue().ToString());
	}*/
	
	//------------------------------------------------------------------------------------------------
	// ! Looks for the very same item, including attachements and its content. Actually, it uses
	// ! the total weight to compare it. Not 100% reliable, but cheap ( instead of comparing the contents and attachements )
	// ! TODO: generate a key/hash for the items state
	protected int FindItem( notnull InventoryItemComponent pInvItem )
	{	
		IEntity pEntity = pInvItem.GetOwner();
		if ( !pEntity )
			return -1;
		EntityPrefabData pEntPrefabData = pEntity.GetPrefabData();
		if ( !pEntPrefabData )
			return -1;
		float fWeight = pInvItem.GetTotalWeight();
		
		MagazineComponent pMagazine = MagazineComponent.Cast( pEntity.FindComponent( MagazineComponent ) );
		
		for ( int i = m_aSlots.Count() - 1; i >= 0; i-- )
		{
			if ( !m_aSlots[ i ] )
				continue;
			InventoryItemComponent pInvItemActual = m_aSlots[ i ].GetInventoryItemComponent();
			
			if ( fWeight != pInvItemActual.GetTotalWeight() )
				continue;
			
			IEntity pEnt = pInvItemActual.GetOwner();
			if ( pEnt && ( pEntPrefabData == pEnt.GetPrefabData() ) )			//is it the same prefab?
			{
				MagazineComponent pMagazineActual = MagazineComponent.Cast( pEnt.FindComponent( MagazineComponent ) );
				if ( pMagazineActual ) //TODO: we actually don't have the ammo in magazines physically. So comparing weight won't help here. We need to count the bullets. Later to remove and use the weight.
				{
					if (pMagazineActual.GetAmmoCount() != pMagazine.GetAmmoCount())
						continue;
				}
				return i;
			}
		}
		return -1;
	}
		
	//------------------------------------------------------------------------------------------------
	// !
	protected int FindItemByName( string sItemName, int iActualIndex )
	{
		int iCount = m_aSlots.Count();
		for ( int i = 0; i < iCount; i++ )
		{
			if ( !m_aSlots[ i ] )
				continue;
			string sName = m_aSlots[ i ].GetItemName();
			if ( sName.Compare( sItemName, false ) == 0 )
				return i;
		}
		return -1;
	}
	
	//------------------------------------------------------------------------------------------------
	// !
	protected string GetNameFromComponent( notnull InventoryItemComponent pComponent )
	{
		ItemAttributeCollection pAttributes = pComponent.GetAttributes();
		if ( !pAttributes )
			return string.Empty;
		if ( !pAttributes.GetUIInfo() )
			return string.Empty;
		return pAttributes.GetUIInfo().GetName();
	}

	//------------------------------------------------------------------------------------------------
	// !
	protected int CreateSlots( )
	{
		ref array<IEntity> pItemsInStorage = new array<IEntity>();
		GetAllItems( pItemsInStorage );
		UpdateOwnedSlots(pItemsInStorage);
		return 0;
	}
	
	protected int GetIndexFromCoords(int x, int y)
	{
		return m_iMaxColumns * y + x;
	}
	
	//------------------------------------------------------------------------------------------------
	// ! creates the the grid from array of UI items
	protected void SortSlots()
	{
		array<int> aCoordinates;
		int iWidgetColumnSize, iWidgetRowSize;
		int iPageCounter = 0;
				
		//reset all elements to 0 - free it
		m_iMatrix.Reset();
			
		foreach ( SCR_InventorySlotUI pSlot: m_aSlots )
		{
			
			if( !pSlot )
				continue;
			Widget w = pSlot.GetWidget();
			if( !w )
				continue;
			
			iWidgetColumnSize = pSlot.GetColumnSize();
			iWidgetRowSize = pSlot.GetRowSize();
			
			//find the 1st suitable position
			aCoordinates = m_iMatrix.Reserve1stFreePlace( iWidgetColumnSize, iWidgetRowSize );


			if( ( aCoordinates[0] != -1 ) && ( aCoordinates[1] != -1 ) )
			{
				GridSlot.SetColumn( w, aCoordinates[0] );
				GridSlot.SetRow( w, aCoordinates[1] );
				pSlot.SetPage( iPageCounter );
			}
			else
			{
				iPageCounter++;
				m_iMatrix.Reset();
				//find the 1st suitable position
				aCoordinates = m_iMatrix.Reserve1stFreePlace( iWidgetColumnSize, iWidgetRowSize );
				if( ( aCoordinates[0] == -1 ) || ( aCoordinates[1] == -1 ) )
					return;
				GridSlot.SetColumn( w, aCoordinates[0] );
				GridSlot.SetRow( w, aCoordinates[1] );
				pSlot.SetPage( iPageCounter );
			}
			GridSlot.SetColumnSpan( w, iWidgetColumnSize );
			GridSlot.SetRowSpan( w, iWidgetRowSize );
		}
		m_iNrOfPages = iPageCounter + 1;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void FillWithEmptySlots()
	{
		array<int> aCoordinates = {};
		
		foreach ( int iIndex, SCR_InventorySlotUI pSlot: m_aSlots )
		{
			if ( pSlot )
				continue;
			ReserveAndSetSlot( iIndex );
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// !
	void ReserveAndSetSlot( int iIndex,  )
	{
		array<int> aCoordinates = m_iMatrix.Reserve1stFreePlace( 1, 1 );
		SCR_ItemAttributeCollection pAttrib = new SCR_ItemAttributeCollection();
		pAttrib.SetSlotType( ESlotID.SLOT_ANY );
		pAttrib.SetSlotSize( ESlotSize.SLOT_1x1 );
		SCR_InventorySlotUI pSlotNew = CreateSlotUI( null, pAttrib );
		m_aSlots.Set( iIndex, pSlotNew );
		GridSlot.SetColumn( pSlotNew.GetWidget(), aCoordinates[0] );
		GridSlot.SetRow( pSlotNew.GetWidget(), aCoordinates[1] );
	}
	
	
	//------------------------------------------------------------------------------------------------
	// !
	protected void ShowPage( int iPage, out int slotToFocus = -1 )
	{
		if( iPage >= m_iNrOfPages )
			iPage = m_iNrOfPages - 1;
		if( iPage < 0 )
			iPage = 0;

		// get all empty slots for the storage and reset their visibility
		array<Widget> emptySlots = {};
		Widget child;
		if (m_wGrid)
			child = m_wGrid.GetChildren();
		int size = m_iMaxRows * m_iMaxColumns;
		for (int i = 0; i < size; ++i)
		{
			if (!child)
				break;
			emptySlots.Insert(child);
			child.SetOpacity(1);
			child = child.GetSibling();
		}

		SCR_InventorySlotUI firstVisibleSlot;
		foreach ( int i, SCR_InventorySlotUI pSlot: m_aSlots )
		{
			if ( pSlot )
			{
				bool visible = (pSlot.GetPage() == iPage);
				pSlot.SetSlotVisible(visible);

				if (!visible && pSlot == m_pLastFocusedSlot)	// when slot is made invisible then it looses track of its button, as it is disabled
					m_pLastFocusedSlot = null;					// thus we should never try to refocus on such slot as its not possible

				if (visible && !firstVisibleSlot)	// because previously focused slot might be disabled now
					firstVisibleSlot = pSlot;		// we need to find first visible slot, in case that we will want to focus on it

				if (!visible)
					continue;

				if (slotToFocus == -1)
					slotToFocus = i;

				if (IsInherited(SCR_InventoryWeaponSlotsUI))
					continue;

				int startIndex = GridSlot.GetColumn(pSlot.m_widget) + GridSlot.GetRow(pSlot.m_widget) * m_iMaxColumns;
				for (int y = 0, maxY = pSlot.GetRowSize(); y < maxY; ++y)
				{
					for (int x = 0, maxX = pSlot.GetColumnSize(); x < maxX; ++x)
					{
						int index = startIndex + x + y * m_iMaxColumns;
						if (!emptySlots.IsIndexValid(index))
							break;
						// hide the empty slot if it's underneath an item slot
						emptySlots[index].SetOpacity(0);
					}
				}
			}
		}

		if (!m_pLastFocusedSlot) // if we didnt have, or slot that was focused last time, got disabled
			m_pLastFocusedSlot = firstVisibleSlot; // then we need to point to something that has a button, meaning that it can be focused

		if( m_iNrOfPages > 1 )
			ShowPagesCounter( true, iPage+1, m_iNrOfPages );
		else
			ShowPagesCounter( false );
		
		if ( m_wPagingSpinboxComponent )
			m_wPagingSpinboxComponent.SetCurrentItem( iPage, false );
		
		m_iLastShownPage = iPage;
	}

	//------------------------------------------------------------------------------------------------
	// ! Removes all of the existing titles and clears the array.
	protected void ClearTraverseTitles()
	{
		if (m_aTraverseTitle.IsEmpty())
			return;
		
		Widget titleLayout = m_widget.FindAnyWidget("titleLayout");
		if (!titleLayout)
			return;
		
		foreach (Widget title: m_aTraverseTitle)
		{
			titleLayout.RemoveChild(title);
		}
		
		m_aTraverseTitle.Clear();
	}
	
	//------------------------------------------------------------------------------------------------
	// ! Fills out the storage with items from the storage param. Also creates traverse titles if traversing.
	protected void FillItemsFromStorage(BaseInventoryStorageComponent storage)
	{
		ClearTraverseTitles();
		
		int count = m_aTraverseStorage.Count();
		
		array<IEntity> pItemsInStorage = new array<IEntity>();
		GetAllItems(pItemsInStorage, GetCurrentNavigationStorage());
		UpdateOwnedSlots(pItemsInStorage);
		
		BaseInventoryStorageComponent currentStorage;
		Widget titleLayout = m_widget.FindAnyWidget("titleLayout");
		Widget title;
		RenderTargetWidget previewImage;
		ItemPreviewManagerEntity manager;
		ItemPreviewWidget renderPreview;
		IEntity previewEntity;
		SCR_InventoryNavigationButtonBack closeContainerButton;
		ButtonWidget navigationHelperWidget;
		SCR_EventHandlerComponent navigationHandler;
		
		for (int i = 0; i < count; ++i)
		{
			currentStorage = m_aTraverseStorage[i];
			
			if (currentStorage.GetOwner().FindComponent(SCR_ArsenalInventoryStorageManagerComponent))
				title = GetGame().GetWorkspace().CreateWidgets(SUPPLY_TITLE_LAYOUT/*ARSENAL_TITLE_LAYOUT*/, titleLayout);
			else if (SCR_ResourceComponent.FindResourceComponent(currentStorage.GetOwner(), true))
				title = GetGame().GetWorkspace().CreateWidgets(SUPPLY_TITLE_LAYOUT, titleLayout);
			else
				title = GetGame().GetWorkspace().CreateWidgets(STORAGE_TITLE_LAYOUT, titleLayout);
			
			if (!title)
				return;
			
			m_aTraverseTitle.Insert( title );
			
			previewImage = RenderTargetWidget.Cast(title.FindAnyWidget("itemStorage"));
			manager = GetInventoryMenuHandler().GetItemPreviewManager();
			if (manager)
			{
				renderPreview = ItemPreviewWidget.Cast(previewImage);
				previewEntity = null;
				if (renderPreview && currentStorage)
					manager.SetPreviewItem(renderPreview, currentStorage.GetOwner(), null, true);
			}
			
			FindAndSetTitleName(title, currentStorage);
			UpdateContainerResources(title, currentStorage);
			
			closeContainerButton = SCR_InventoryNavigationButtonBack.Cast(SCR_InventoryNavigationButtonBack.GetInputButtonComponent("ButtonTraverseBack", title));
			if (!closeContainerButton)
				return;
				
			closeContainerButton.m_OnActivated.Insert(m_MenuHandler.OnAction);
			closeContainerButton.SetParentStorage(this);
			closeContainerButton.SetStorageIndex(i);
			closeContainerButton.SetVisible(true);
				
			navigationHelperWidget = ButtonWidget.Cast(title.FindAnyWidget("navigationHelper"));
			
			if (!navigationHelperWidget)
				return;
			
			if (i == count - 1)
				m_wLastCloseTraverseButton = navigationHelperWidget;
			
			navigationHelperWidget.AddHandler(new SCR_EventHandlerComponent);
			navigationHandler = SCR_EventHandlerComponent.Cast(navigationHelperWidget.FindHandler(SCR_EventHandlerComponent));
			
			if(!navigationHandler)
				return;
			
			navigationHandler.GetOnClick().Insert(closeContainerButton.OnActivate);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void FindAndSetTitleName(Widget targetTitle, BaseInventoryStorageComponent targetStorage)
	{
		TextWidget titleNameWidget = TextWidget.Cast(targetTitle.FindAnyWidget("TextC"));
		if(!titleNameWidget)
			return;
		ItemAttributeCollection attributes = targetStorage.GetAttributes();
		if(!attributes)
			return;
		UIInfo uiInfo = attributes.GetUIInfo();
		if(!uiInfo)
			return;
		titleNameWidget.SetText(uiInfo.GetName());
	}
	
	//------------------------------------------------------------------------------------------------
	// !
	protected void GetAllItems( out notnull array<IEntity> pItemsInStorage, BaseInventoryStorageComponent pStorage = null )
	{		
		if (pStorage)
		{
			IEntity ownerEntity = pStorage.GetOwner();
			if (!ownerEntity)
				return;
			SCR_ArsenalInventoryStorageManagerComponent arsenalManagerComponent = SCR_ArsenalInventoryStorageManagerComponent.Cast(ownerEntity.FindComponent(SCR_ArsenalInventoryStorageManagerComponent));
			
			if (arsenalManagerComponent)
			{
				m_bIsArsenal = true;
				if (!pStorage)
					return;
				
				ChimeraWorld chimeraWorld = GetGame().GetWorld();
				ItemPreviewManagerEntity itemPreviewManagerEntity = chimeraWorld.GetItemPreviewManager();
			
				SCR_ArsenalComponent arsenalComponent	= SCR_ArsenalComponent.Cast(ownerEntity.FindComponent(SCR_ArsenalComponent));
				array<ResourceName> prefabsToSpawn		= new array<ResourceName>();
				
				if (!arsenalComponent)
					return;
				
				arsenalComponent.GetAvailablePrefabs(prefabsToSpawn);
				
				foreach (ResourceName resourceName: prefabsToSpawn)
				{
					pItemsInStorage.Insert(itemPreviewManagerEntity.ResolvePreviewEntityForPrefab(resourceName));
				}
				
				if (s_OnArsenalEnter)
					s_OnArsenalEnter.Invoke();
				
				return;
			}
			
			if (ClothNodeStorageComponent.Cast(pStorage))
			{
				array<BaseInventoryStorageComponent> pStorages = {};
				array<IEntity> pItems = {};
				pStorage.GetOwnedStorages(pStorages, 1, false);
				
				foreach (BaseInventoryStorageComponent pStor : pStorages)
				{
					if (!pStor)
						continue;
					pStor.GetAll(pItems);
					pItemsInStorage.Copy(pItems);
				}
				return;
			}
		
			pStorage.GetAll(pItemsInStorage);
			return;
		}
			
		if (m_Storage)
			m_Storage.GetAll(pItemsInStorage);
	}
	
	
	//------------------------------------------------------------------------------------------------
	// ! gets all the items only from the given depth. I.e. all items inserted in LBS pouches
	// ! changes the m_Storage, back it up !!!
	protected void GetAllItemsFromDepth( out notnull array<IEntity> pItemsInStorages, int iDepth = 3, int iDepthDefault = 0 )
	{
		
		if ( !m_Storage )
			return;
				
		if ( iDepthDefault == 0 )
			iDepthDefault = iDepth;
						
		#ifdef DEBUG_INVENTORY20
			if ( iDepthDefault == iDepth )
			{
				LocalizedString sStorageName = "";
				ItemAttributeCollection pAttribs = m_Storage.GetAttributes();
				if ( pAttribs )
				{
					UIInfo pInfo = pAttribs.GetUIInfo();
					if ( pInfo )
						sStorageName = pInfo.GetName();
					Print( "-----------------------------------------------" );
					PrintFormat( "%1 ( %2 )", sStorageName, BaseInventoryStorageComponent.Cast( m_Storage ) );
				}
			}
		#endif
		
		array<IEntity> aTraverseStorageTmp = new array<IEntity>();
		m_Storage.GetAll( aTraverseStorageTmp );

		IEntity pEntity;

		for ( int m = 0; m < aTraverseStorageTmp.Count(); m++ )
		{
			pEntity = aTraverseStorageTmp[ m ];
			//BaseInventoryStorageComponent tmpStorage
			m_Storage = BaseInventoryStorageComponent.Cast( pEntity.FindComponent( BaseInventoryStorageComponent ) );
			InventoryItemComponent pInventoryComponent = InventoryItemComponent.Cast( pEntity.FindComponent( InventoryItemComponent ) );
			if ( iDepth == 1 )
				pItemsInStorages.Insert( pEntity );
			if ( m_Storage )
			{
				#ifdef DEBUG_INVENTORY20
					ItemAttributeCollection pAttrs = m_Storage.GetAttributes();
					if ( !pAttrs )
						return;
					LocalizedString sName = pAttrs.GetUIInfo().GetName();
					string sTextIn = "";
					ConstructSlashes( sTextIn, iDepthDefault-iDepth );
					PrintFormat( "|%1 %2 ( %3 )", sTextIn, sName, m_Storage );
				#endif
				GetAllItemsFromDepth( pItemsInStorages, iDepth - 1, iDepthDefault );
			}
			#ifdef DEBUG_INVENTORY20
				else
				{
					if ( pInventoryComponent )
					{
						LocalizedString sName = "";
						if ( pInventoryComponent.GetAttributes() )
							if ( pInventoryComponent.GetAttributes().GetUIInfo() )
								sName = pInventoryComponent.GetAttributes().GetUIInfo().GetName();
						string sTextIn = "";
						ConstructSlashes( sTextIn, iDepthDefault-iDepth );
						PrintFormat( "|%1 %2 ( %3 )", sTextIn, sName, pInventoryComponent );
					}
				}
			#endif
		}
	}

	//------------------------------------------------------------------------------------------------
	void UpdateTotalWeight(float totalWeight, Color fontColor = Color.Gray25)
	{
		if (!m_wWeightText)
			return;

		m_wWeightText.SetTextFormat("#AR-ValueUnit_Short_Kilograms", totalWeight);
	}
 	
 	//------------------------------------------------------------------------------------------------
	float GetTotalRoundedUpWeight(BaseInventoryStorageComponent storage)
	{
		if (!storage)
			return -1;
		
		float totalWeight = storage.GetTotalWeight();
		
		totalWeight = Math.Round(totalWeight * 100); // totalWeight * 100 here to not lose the weight precision
		
		return totalWeight / 100;
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateVolumePercentage(float percentage, bool previewOnly = false)// Color fontColor = Color.Gray25)
	{
		if (m_ProgressBar)
		{
			m_ProgressBar.SetProgressRange(0, 100);
			if (previewOnly)
				m_ProgressBar.SetCurrentProgressPreview(percentage);
			else
			{
				m_ProgressBar.SetCurrentProgress(percentage);
				m_ProgressBar.SetCurrentProgressPreview(percentage);				
			}

			RichTextWidget percentageText = RichTextWidget.Cast(m_widget.FindAnyWidget("VolumeOver"));
			if (percentageText)
			{
				percentageText.SetTextFormat("#AR-ValueUnit_Percentage_Add", percentage - 100);
				percentageText.SetVisible(percentage > 100);
				Widget progressVolumeOver = m_widget.FindAnyWidget("ProgressVolumeOver");
				progressVolumeOver.SetVisible(percentage > 100);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool CheckIfAnyItemInStorageHasResources(array<IEntity> items)
	{
		SCR_ResourceComponent resComp;
		
		foreach(SCR_ResourceComponent oldResComp: m_aResourceCompsInStorage)
		{
			if (!oldResComp)
				continue;
			
			SCR_ResourceConsumer oldResCont = oldResComp.GetConsumer(EResourceGeneratorID.DEFAULT_STORAGE, EResourceType.SUPPLIES);
		
			if (!oldResCont)
				continue;
			
			oldResComp.TEMP_GetOnInteractorReplicated().Remove(RefreshResources);	
		}
		
		m_aResourceCompsInStorage.Clear();
		
		foreach (IEntity item: items)
		{
			if (!item)
				continue;
			
			resComp = SCR_ResourceComponent.FindResourceComponent(item);
			
			if (!resComp)
				continue;
			
			m_aResourceCompsInStorage.Insert(resComp);
			
			SCR_ResourceConsumer resCont = resComp.GetConsumer(EResourceGeneratorID.DEFAULT_STORAGE, EResourceType.SUPPLIES);
		
			if (!resCont)
				continue;
			
			resComp.TEMP_GetOnInteractorReplicated().Insert(RefreshResources);	
		}
		
		if (m_aResourceCompsInStorage.IsEmpty())
			return false;
			
		return true;
	}

	//------------------------------------------------------------------------------------------------
	float GetOccupiedVolumePercentage(BaseInventoryStorageComponent storage, float occupiedSpace = 0)
	{
		if (!storage)
			return -1;

		float occupiedVolumePercantage;
		float capacity = GetMaxVolumeCapacity(storage);
		
		if (capacity == 0)
			return 0;
		
		if (occupiedSpace != 0)
			occupiedVolumePercantage = Math.Round(occupiedSpace / (capacity / 100));
		else
			occupiedVolumePercantage = Math.Round(GetOccupiedVolume(storage) / (capacity / 100));
		
		return occupiedVolumePercantage;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetOccupiedVolume(BaseInventoryStorageComponent storage)
	{
		if (!storage)
			return -1;

		float fOccupied = 0.0;
		if (ClothNodeStorageComponent.Cast(storage))
		{
			array<BaseInventoryStorageComponent> pOwnedStorages = {};
			storage.GetOwnedStorages(pOwnedStorages, 1, false);
			foreach (BaseInventoryStorageComponent pSubStorage : pOwnedStorages)
				if (SCR_UniversalInventoryStorageComponent.Cast(pSubStorage))
					fOccupied += pSubStorage.GetOccupiedSpace();
		}
		else
		{
			fOccupied = storage.GetOccupiedSpace();
		}
		
		return fOccupied;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetMaxVolumeCapacity(BaseInventoryStorageComponent storage)
	{
		if (!storage)
			return -1;

		float fMaxCapacity = 0.0;
		if (ClothNodeStorageComponent.Cast(storage))
		{
			array<BaseInventoryStorageComponent> pOwnedStorages = {};
			storage.GetOwnedStorages(pOwnedStorages, 1, false);
			foreach (BaseInventoryStorageComponent pSubStorage : pOwnedStorages)
			{
				if (SCR_UniversalInventoryStorageComponent.Cast(pSubStorage))
					fMaxCapacity += pSubStorage.GetMaxVolumeCapacity();
			}
		}
		else
		{
			fMaxCapacity = storage.GetMaxVolumeCapacity();
		}
		
		return fMaxCapacity;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetStorageAsHighlighted(bool isVisible, EInvInsertFailReason reason = EInvInsertFailReason.OK)
	{
		if (!m_widget)
			return;

		if (!m_wWarningOverlay)
			return;

		m_wWarningOverlay.SetVisible(!isVisible);
		if (m_ProgressBar)
			m_ProgressBar.SetPreviewColor(isVisible);

		if (m_wWeightDisplay)
			m_wWeightDisplay.SetColor(m_WeightDefault);
		
		if (reason == EInvInsertFailReason.OK)
			return;
		
		Widget size = m_wWarningOverlay.FindAnyWidget("Oversized");
		Widget weight = m_wWarningOverlay.FindAnyWidget("Overweight");
		Widget volume = m_wWarningOverlay.FindAnyWidget("Overvolume");
		
		if (size) 
			size.SetVisible(reason & EInvInsertFailReason.SIZE);

		if (weight) 
		{
			if (m_wWeightDisplay && reason & EInvInsertFailReason.WEIGHT)
				m_wWeightDisplay.SetColor(Color.Red);
			weight.SetVisible(reason & EInvInsertFailReason.WEIGHT);
		}

		if (volume)
			volume.SetVisible(reason & EInvInsertFailReason.CAPACITY);
	}

	//------------------------------------------------------------------------------------------------
	bool IsStorageHighlighted()
	{
		if (!m_widget)
			return false;

		return !IsWarningOverlayVisible();
	}
	
	//------------------------------------------------------------------------------------------------
	// ! formating helper
	private void ConstructSlashes( out string sText, int iSlashes )
	{
		for ( int m = 0; m <= ( iSlashes * 2 ); m++ )
		{
			sText += "-";
		}
	}

	//------------------------------------------------------------------------------------------------
	// ! creates the simple matrix of dimension [ m_iMaxRows, m_iMaxColumns ]
	//! \param[in] recreate set to true will first destroy previously created empty slots. If it is false, then game will only create new slots if current number of empty slots is different than (m_iMaxRows * m_iMaxColumns)
	protected void CreateEmptyLayout(bool recreate = false)
	{
		if (!m_aEmptySlotWidget)
			m_aEmptySlotWidget = {};
		else if (recreate)
			DestroyEmptySlots();
		else if (m_aEmptySlotWidget.Count() == m_iMaxRows * m_iMaxColumns)
			return; // if number of slots has not changed then dont make new ones

		for( int iLoop = 0; iLoop < m_iMaxRows; iLoop++ )
		{
			for( int jLoop = 0; jLoop < m_iMaxColumns; jLoop++ )
			{
				m_aEmptySlotWidget.Insert(CreateEmptySlotUI( iLoop, jLoop ));
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Destroys all empty slots which were created by this handler
	protected void DestroyEmptySlots()
	{
		if (!m_aEmptySlotWidget)
			return;

		foreach (Widget emptySlot : m_aEmptySlotWidget)
		{
			if (!emptySlot)
				continue;

			emptySlot.RemoveFromHierarchy();
		}

		m_aEmptySlotWidget.Clear();
	}
	
	//------------------------------------------------------------------------------------------------
	// ! get the item inventory component
	InventoryItemComponent GetItemComponentFromEntity( IEntity pEntity )
	{
		if ( pEntity == null )
			return null;
		return InventoryItemComponent.Cast(pEntity.FindComponent( InventoryItemComponent ));
	}
	
	//------------------------------------------------------------------------------------------------
	// !
	void GetSlots( out notnull array<SCR_InventorySlotUI> outSlots )
	{
		outSlots.InsertAll( m_aSlots );
	}
	
	//------------------------------------------------------------------------------------------------
	// ! displays the info in details
	void ShowItemDetails( string sName, string sDescription, string sWeight );
	
	//------------------------------------------------------------------------------------------------
	// ! helper for showing the number of pages
	private void ShowPagesCounter( bool bShow = false, int iPage = 0, int iNumberOfPages = 0 )
	{
		if ( !m_wPagingSpinboxComponent )
			return;
		
		m_wPagingSpinboxComponent.SetVisible( bShow, false );
						
		if ( bShow )
		{
			m_wPagingSpinboxComponent.SetCanNavigate( true );
			m_wPagingSpinboxComponent.SetPageCount( iNumberOfPages );
			m_wPagingSpinboxComponent.SetCurrentItem( iPage, false );
		}
		else
		{
			m_wPagingSpinboxComponent.SetCanNavigate( true );
		}
	}
	
	//-------------------------------------- MOVE MANAGEMENT -----------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! memorize the last focused slot ( in case we leaving the actual UI storage and we want to select this slot again when we are back )
	void SetLastFocusedSlot( SCR_InventorySlotUI pSlot )
	{
		m_pLastFocusedSlot = pSlot;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void SetSlotFocused( SCR_InventorySlotUI pSlotUI = null, bool bSelected = true )
	{
		if(bSelected)
		{
			m_pFocusedSlot = pSlotUI;
			if (m_pFocusedSlot)
				SetLastFocusedSlot(m_pFocusedSlot);
		}
		else
		{
			m_pFocusedSlot = null;
		}
	}
		
	//------------------------------------------------------------------------------------------------
	// ! selects the actual slot
	void SetSlotSelected( SCR_InventorySlotUI pSlot = null, bool bSelected = true )
	{
		m_pPrevSelectedSlot = m_pSelectedSlot;
		
		if( bSelected )
		{
			m_pSelectedSlot = pSlot;
		}
		else
			m_pSelectedSlot = null;
	}
		
	//------------------------------------------------------------------------------------------------
	void ShowContainerBorder( bool bShow )
	{
		if (bShow && IsWarningOverlayVisible())
			return;

		if ( m_wDropContainer )
		{
			m_wDropContainer.SetVisible( bShow );
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_FE_BUTTON_HOVER);
		}
	}

	//------------------------------------------------------------------------------------------------
	void ActivateStorageButton(bool active)
	{
		if (m_wButton)
		{
			m_wButton.SetEnabled(active);
			m_wButton.SetVisible(active);
			foreach (SCR_InventorySlotUI slot : m_aSlots)
			{
				if (slot && slot.GetButtonWidget())
					slot.GetButtonWidget().SetEnabled(!active);
			}
		}

		if (m_wLastCloseTraverseButton)
			m_wLastCloseTraverseButton.SetEnabled(!active);
	}

	//------------------------------------------------------------------------------------------------
	int GetFocusedSlotId()
	{
		if (!m_pFocusedSlot)
			return -1;
		return m_aSlots.Find(m_pFocusedSlot);
	}

	//------------------------------------------------------------------------------------------------
	int GetSlotId(SCR_InventorySlotUI slot)
	{
		return m_aSlots.Find(slot);
	}

	//------------------------------------------------------------------------------------------------
	int GetSlotIdForItem(ResourceName itemName)
	{
		int slotCount = m_aSlots.Count();
		SCR_InventorySlotUI slot;

		for (int slotId = 0; slotId < slotCount; ++slotId)
		{
			slot = m_aSlots[slotId];
			if (slot && slot.GetItemResource() == itemName)
			{
				return slotId;
			}
		}

		return 0;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetPreviewItem()
	{
		m_wPreviewImage = RenderTargetWidget.Cast( m_widget.FindAnyWidget( "itemStorage" ) );
		
		ChimeraWorld world = ChimeraWorld.CastFrom(GetGame().GetWorld());
		if (!world)
			return;
		
		ItemPreviewManagerEntity manager = world.GetItemPreviewManager();
		if (!manager)
			return;
		
		ItemPreviewWidget renderPreview = ItemPreviewWidget.Cast( m_wPreviewImage );
		if (!renderPreview)
			return;
		
		IEntity previewEntity = null;
		if (m_Storage)
			previewEntity = m_Storage.GetOwner();
		
		manager.SetPreviewItem(renderPreview, previewEntity, null, true);
	}
	
	//------------------------------------------------------------------------ COMMON METHODS ----------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	void DimSlots( bool bDim )
	{
		foreach ( SCR_InventorySlotUI pSlot : m_aSlots )
		{
			if ( pSlot )
			{
				if ( !pSlot.IsSelected() )
				{
					pSlot.SetEnabledForMove( !bDim );
				}
			}
		}
	}
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached( Widget w )
	{
		if( !w )
			return;
					
		m_wDropContainer = w.FindAnyWidget( "DropContainer" );
		m_wButton = ButtonWidget.Cast(w.FindAnyWidget("Button"));
		if (m_wButton)
		{
			SCR_InventoryStorageButton btn = SCR_InventoryStorageButton.Cast(m_wButton.FindHandler(SCR_InventoryStorageButton));
			if (btn)
			{
				btn.SetParent(this);
			}
			else
			{
				SCR_InventoryStorageButton newBtn = new SCR_InventoryStorageButton();
				newBtn.SetParent(this);
				m_wButton.AddHandler(newBtn);
			}

			m_wButton.SetOpacity(0.1);
		}

		m_widget = w;
		Init();
	}
	
	//------------------------------------------------------------------------------------------------
	override event void HandlerDeattached(Widget w)
	{
		if (m_ResourceComponent)
			m_ResourceComponent.TEMP_GetOnInteractorReplicated().Remove(RefreshConsumer);
		
		m_ResourceSubscriptionHandleConsumer = null;
		DeleteSlots();
	}
			
	//------------------------------------------------------------------------------------------------
	Widget GetButtonWidget()
	{
		return m_wButton;
	}
	
	//------------------------------------------------------------------------------------------------
	ButtonWidget GetLastCloseTraverseButton()
	{
		return m_wLastCloseTraverseButton;
	}
	
	bool IsWarningOverlayVisible()
	{
		return m_wWarningOverlay && m_wWarningOverlay.IsVisible();
	}

	//------------------------------------------------------------------------------------------------
	void SCR_InventoryStorageBaseUI(
		BaseInventoryStorageComponent storage,
		LoadoutAreaType slotID = null,
		SCR_InventoryMenuUI menuManager = null,
		int iPage = 0,
		array<BaseInventoryStorageComponent> aTraverseStorage = null)
	{
		m_Storage = storage;
		m_MenuHandler 	= menuManager;
		m_eSlotAreaType = slotID;
		m_iMaxRows 		= 3;
		m_iMaxColumns 	= 4;
		m_iMatrix = new SCR_Matrix( m_iMaxColumns, m_iMaxRows );
		m_iPageSize 	= m_iMaxRows * m_iMaxColumns;
		m_iLastShownPage = iPage;
		if ( aTraverseStorage )
			m_aTraverseStorage.Copy( aTraverseStorage );
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_InventoryStorageBaseUI()
	{
		if( m_aSlots )
			delete m_aSlots;
		if( m_iMatrix )
			delete m_iMatrix;
	}
};

//------------------------------------------------------------------------------------------------
class SCR_InventoryStorageButton : ScriptedWidgetComponent
{
	protected SCR_InventoryStorageBaseUI m_Parent;

	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		m_Parent.GetInventoryMenuHandler().OnContainerFocused(m_Parent);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		m_Parent.GetInventoryMenuHandler().OnContainerFocusLeft(m_Parent);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	void SetParent(SCR_InventoryStorageBaseUI parent)
	{
		m_Parent = parent;
	}
};