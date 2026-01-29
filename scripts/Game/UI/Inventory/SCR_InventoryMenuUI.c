enum EMenuAction
{
	ACTION_SELECT,
	ACTION_DESELECT,
	ACTION_UNFOLD,
	ACTION_BACK,
	ACTION_MOVEBETWEEN,
	ACTION_DRAGGED,
	ACTION_DROPPED,
	ACTION_MOVEINSIDE,
	ACTION_OPENCONTAINER
};

enum EStateMenuStorage
{
	STATE_INIT = 0,
	STATE_IDLE,
	STATE_OPENED,
};

enum EStateMenuItem
{
	STATE_INIT,
	STATE_IDLE,
	STATE_MOVING_ITEM_STARTED,
};

enum EDropContainer
{
	ISINSIDE,
	ISNOTINSIDE,
	NOCONTAINER,
};

enum EAttachAction
{
	NONE,
	ATTACH,
	DETACH
};

enum EInvInitStage
{
	BEGIN = 0,
	NAVIGATION_BAR,
	QUICK_SLOTS,
	WEAPON_STORAGE,
	STORAGE_LIST,
	STORAGES,
	HIT_ZONES,
	DONE

};

enum EInvInsertFailReason
{
	OK = 		1 << 0,
	SIZE = 		1 << 1,
	WEIGHT = 	1 << 2,
	CAPACITY =	1 << 3
};

//------------------------------------------------------------------------------------------------
class SCR_InvCallBack : ScriptedInventoryOperationCallback
{
	SCR_InventoryStorageBaseUI m_pStorageFrom;
	SCR_InventoryStorageBaseUI m_pStorageTo;
	SCR_InventoryStorageBaseUI m_pStorageToFocus;
	SCR_InventoryStorageManagerComponent m_pStorageMan;

	BaseInventoryStorageComponent m_pStorageToDrop;
	BaseInventoryStorageComponent m_pStorageToPickUp;

	EAttachAction m_eAttachAction = EAttachAction.NONE;
	IEntity m_pItem;
	SCR_InventoryMenuUI m_pMenu;
	ResourceName m_sItemToFocus;
	int m_iSlotToFocus;
	bool m_bShouldEquip;
	bool m_bUpdateSlotOnly;
	bool m_bShouldUpdateQuickSlots;

	void InternalComplete()
	{
		OnComplete();
	}
	//------------------------------------------------------------------------------------------------
	protected override void OnFailed()
	{
		if (m_pMenu && m_pItem && m_bShouldEquip)
		{
			m_pMenu.GetCharacterController().TryEquipRightHandItem(m_pItem, EEquipItemType.EEquipTypeWeapon, false);
			m_bShouldEquip = false;
		}
		
		if (m_pMenu)
			m_pMenu.Action_DeselectItem();
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnComplete()
	{
		if (m_pMenu && m_pItem && m_bShouldEquip)
		{
			m_pMenu.GetCharacterController().TryEquipRightHandItem(m_pItem, EEquipItemType.EEquipTypeWeapon, false);
			m_bShouldEquip = false;
		}

		if (m_eAttachAction == EAttachAction.ATTACH)
		{
			if (m_pStorageMan)
				m_pStorageMan.PlayItemSound(m_pItem, SCR_SoundEvent.SOUND_EQUIP);
		}
		else if (m_eAttachAction == EAttachAction.DETACH)
		{
			if (m_pStorageMan)
				m_pStorageMan.PlayItemSound(m_pItem, SCR_SoundEvent.SOUND_UNEQUIP);
		}

		m_eAttachAction = EAttachAction.NONE;

		bool shouldUpdateStorageList;
		if ( m_pStorageFrom )
		{
			if ( m_pMenu )
			{
				if ( m_pStorageFrom == m_pMenu.GetStorageList() )
				{
					shouldUpdateStorageList = true;
				}
				else
				{
					if (m_bUpdateSlotOnly)
					{
						m_pStorageFrom.UpdateSlotUI(m_sItemToFocus);
						m_bUpdateSlotOnly = false;
					}
					else
					{
						BaseInventoryStorageComponent storage = m_pStorageFrom.GetCurrentNavigationStorage();
						auto open = m_pMenu.GetOpenedStorage(storage);
						if (open)
							open.Refresh();
						
						if (m_pItem)
						{
							auto itemStorage = BaseInventoryStorageComponent.Cast(m_pItem.FindComponent(BaseInventoryStorageComponent));
							open = m_pMenu.GetOpenedStorage(itemStorage);
							if (open)
								open.CloseStorage();
						}

						if (m_pStorageFrom)
							m_pStorageFrom.Refresh();
					}
				}
			}
		}
		else
		{
			if ( m_pMenu )
			{
				shouldUpdateStorageList = true;
			}
		}

		if ( m_pStorageTo )
		{
			if ( m_pMenu )
			{
				if ( m_pStorageTo == m_pMenu.GetStorageList() )
				{
					shouldUpdateStorageList = true;

					auto itemStorage = BaseInventoryStorageComponent.Cast(m_pItem.FindComponent(BaseInventoryStorageComponent));
					SCR_InventoryOpenedStorageUI open = m_pMenu.GetOpenedStorage(itemStorage);
					if (open)
						open.CloseStorage();
				}
				else
				{
					m_pStorageTo.Refresh();
					BaseInventoryStorageComponent storage = m_pStorageTo.GetCurrentNavigationStorage();
					auto open = m_pMenu.GetOpenedStorage(storage);
					if (open)
						open.Refresh();
				}
			}
		}

		if (m_pItem)
		{
			InventoryItemComponent inventoryItemComp = InventoryItemComponent.Cast(m_pItem.FindComponent(InventoryItemComponent));
			InventoryStorageSlot parentSlot;
			if (inventoryItemComp)
				parentSlot = inventoryItemComp.GetParentSlot();

			SCR_HandSlotStorageComponent handSlotStorage;
			if (parentSlot)
				handSlotStorage = SCR_HandSlotStorageComponent.Cast(parentSlot.GetStorage());

			if (handSlotStorage)
				handSlotStorage.SkipAnimation_S();
		}

		if (shouldUpdateStorageList)
		{
			m_pMenu.ShowStoragesList();
			m_pMenu.ShowAllStoragesInList();
		}

		if ( m_pMenu )
		{ 
			if ( m_pStorageFrom != m_pMenu.GetLootStorage() && m_pStorageTo != m_pMenu.GetLootStorage() )
				m_pMenu.RefreshLootUIListener();

			SCR_InventoryStorageBaseUI pStorage = m_pMenu.GetActualStorageInCharacterStorageUI();
			if ( pStorage )
			{
				if ( pStorage.Type() == SCR_InventoryStorageWeaponsUI ) 
					pStorage = m_pMenu.GetStorageList();
				//pStorage.Refresh();
			}

			m_pMenu.NavigationBarUpdate();
			m_pMenu.RefreshPlayerWidget();
		}

		if (m_pStorageToFocus)
		{
			m_pMenu.FocusOnSlotInStorage(m_pStorageToFocus, m_iSlotToFocus);
			m_iSlotToFocus = -1;
		}
		
		if (!m_bShouldEquip && m_pStorageTo && m_pStorageTo.IsInherited(SCR_InventoryStorageLootUI))
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INV_CONTAINER_DIFR_DROP);

		if (m_pStorageToDrop)
		{
			if (m_pMenu && m_bShouldUpdateQuickSlots)
				m_pMenu.ShowQuickSlotStorage();
			m_pStorageToDrop = null;
		}

		if (m_pStorageToPickUp)
		{
			if (m_pMenu && m_bShouldUpdateQuickSlots)
				m_pMenu.ShowQuickSlotStorage();
			m_pStorageToPickUp = null;
		}

		m_bShouldUpdateQuickSlots = false;
		
		ResetVariables();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ResetVariables()
	{
		m_pStorageToFocus = null;
		m_pStorageTo = null;
		m_pStorageFrom = null;
	}
	
};

//------------------------------------------------------------------------------------------------
//! UI Script
//! Inventory Menu UI Layout

void InventorySlotHoverMethod(SCR_InventorySlotUI slot);
typedef func InventorySlotHoverMethod;
typedef ScriptInvokerBase<InventorySlotHoverMethod> ScriptInvokerInventorySlotHover;

class SCR_InventoryMenuUI : ChimeraMenuBase
{	
	
	#ifndef DISABLE_INVENTORY
	static protected ResourceName							m_ItemPreviewManagerPrefab = "{9F18C476AB860F3B}Prefabs/World/Game/ItemPreviewManager.et";
	protected SCR_InventoryStorageManagerComponent			m_InventoryManager 		= null;
	protected SCR_CharacterInventoryStorageComponent		m_StorageManager 		= null;
	protected ChimeraCharacter								m_Player;
	protected Widget										m_wContainer;
	protected Widget										m_wStorageList, m_wLootStorage, m_wOpenedStorage, m_wWeaponSlots;
	protected Widget  										m_wAttachmentStorage;
	protected Widget										m_widget;
	
	protected SCR_InventoryInspectionUI						m_InspectionScreen				= null;
	
	protected SCR_InventorySlotUI							m_pFocusedSlotUI				= null;
	protected SCR_InventorySlotUI							m_pSelectedSlotUI				= null;
	protected SCR_InventorySlotUI							m_DraggedSlot = null;

	protected SCR_InventorySlotStorageUI					m_pFocusedSlotStorageUI			= null;
	protected SCR_InventorySlotStorageUI					m_pSelectedSlotStorageUI		= null;
	protected SCR_InventorySlotStorageUI					m_pSelectedSlotStoragePrevUI	= null;

	protected SCR_InventoryStorageBaseUI					m_pStorageBaseUI				= null;
	protected SCR_InventoryStoragesListUI					m_pStorageListUI				= null;
	protected SCR_InventoryStorageGadgetsUI					m_pGadgetsBaseUI				= null;
	protected SCR_InventoryStorageBaseUI					m_pStorageLootUI				= null;
	protected SCR_InventoryWeaponSlotsUI					m_pWeaponStorage				= null;
	protected EquipedWeaponStorageComponent					m_pWeaponStorageComp			= null;
	protected ref array<SCR_InventoryOpenedStorageUI> 		m_aOpenedStoragesUI				= {};
	protected SCR_InventoryAttachmentStorageUI				m_pAttachmentStorageUI			= null;

	protected SCR_InventoryItemInfoUI						m_pItemInfo						= null;
	protected SCR_InventoryDamageInfoUI						m_pDamageInfo					= null;
	protected SCR_InventoryStorageBaseUI					m_pActiveStorageUI				= null;
	protected SCR_InventoryStorageBaseUI					m_pActiveHoveredStorageUI		= null;
	protected SCR_InventoryStorageBaseUI					m_pPrevActiveStorageUI			= null;
	protected ref array<SCR_InventorySlotUI>				m_aShownStorages				= new array<SCR_InventorySlotUI>();	//used for storing pointers on storages which are displayed on screen ( for the refreshing purposes )
	protected GridLayoutWidget								m_wStoragesContainer;
	protected const int										STORAGE_AREA_COLUMNS			= 2;
	protected int											m_iStorageListCounter			= 0;
	protected int											m_iVicinityDiscoveryRadius		= 0;

	protected ref SCR_InventorySlotUI						m_pInspectedSlot				= null;
	protected ref SCR_InventoryGearInspectionPointUI		m_pGearInspectionPointUI		= null;
	protected Widget										m_wAttachmentContainer			= null;

	const string											ARSENAL_STORAGE_LAYOUT	= "{5C4812D758C0EDB8}UI/layouts/Menus/Inventory/SupplyInventoryContainerGrid.layout";
	const string 											BACKPACK_STORAGE_LAYOUT	= "{E61475CBB560757F}UI/layouts/Menus/Inventory/InventoryContainerGrid.layout";
	const string											WEAPON_STORAGE_LAYOUT	= "{7B28D87B8A1ADD42}UI/layouts/Menus/Inventory/InventoryWeaponSlots.layout";
	const string 											STORAGES_LIST_LAYOUT 	= "{FC579324F5E4B3A4}UI/layouts/Menus/Inventory/InventoryCharacterGrid.layout";
	const string 											ITEM_INFO			 	= "{7BA9AFEC71DAC3CF}UI/layouts/Menus/Inventory/InventoryInfo.layout";
	const string 											DAMAGE_INFO			 	= "{55AFA256E1C20FB2}UI/layouts/Menus/Inventory/InventoryDamageInfo.layout";
	
	const string											STORAGE_LAYOUT_CLOSE_WIDGET_NAME = "CloseStorageBtn";

	const ResourceName										HITZONE_CONTAINER_LAYOUT= "{36DB099B4CDF8FC2}UI/layouts/Menus/Inventory/Medical/HitZonePointContainer.layout";

	[Attribute("{01E150D909447632}Configs/Damage/DamageStateConfig.conf", desc: "Config to get visual data from", params: "conf class=SCR_DamageStateConfig")]
	protected ref SCR_DamageStateConfig m_DamageStateConfig;
	
	
	protected EStateMenuStorage								m_EStateMenuStorage = EStateMenuStorage.STATE_IDLE; // is this useful for anything?
	protected EStateMenuItem								m_EStateMenuItem = EStateMenuItem.STATE_IDLE;
	protected string 										m_sFSMStatesNames[10]={"init", "idle", "item selected", "storage selected", "STATE_STORAGE_OPENED", "storage unfolded", "move started", "move finished", "closing", "STATE_UNKNOWN" };
	protected SCR_NavigationBarUI							m_pNavigationBar			= null;
	protected SCR_InputButtonComponent						m_CloseButton;
	
	const protected ResourceName 							m_sSupplyCostUIInfoPrefab = "{4D8296CB3CB3B8BF}Configs/Inventory/SupplyCost_ItemUIInfo.conf";
	protected ref SCR_SupplyCostItemHintUIInfo 				m_SupplyCostUIInfo;
	const protected ResourceName 							m_sSupplyRefundUIInfoPrefab = "{6FD3DF5A3B2C6D30}Configs/Inventory/ItemHints/SupplySell_ItemHint.conf";
	protected ref SCR_SupplyRefundItemHintUIInfo 			m_SupplyRefundUIInfo;
	const protected ResourceName 							m_sNonrefundableUIInfoPrefab = "{B3E94EB9D0F0EAC6}Configs/Inventory/ItemHints/Nonrefundable_ItemHint.conf";
	protected ref SCR_NonrefundableItemHintUIInfo			m_NonrefundableUIInfo;
	protected const ResourceName 							ITEM_RANK_PREFAB = "{B329E7CABD481EF6}Configs/Inventory/ItemHints/ArsenalItemRank_ItemHint.conf";
	protected ref SCR_ArsenalItemRankHintUIInfo 			m_ArsenalItemRankUIInfo;
	protected const ResourceName 							VALUABLE_INTEL_UIINFO_PREFAB = "{5EE3196892CA5142}Configs/Inventory/ItemHints/ValuableIntel_ItemHint.conf";
	protected ref SCR_ValuableIntelItemHintUIInfo 			m_ValuableIntelUIInfo;
	protected const ResourceName							PLAYER_SUPPLY_ALLOCATION_UIINFO_PREFAB = "{4A370E79EF2FCC54}Configs/Inventory/ItemHints/MSARCost_ItemHint.conf";
	protected ref SCR_PlayerSupplyAllocationItemHintUIInfo	m_PlayerSupplyAllocationUIInfo;
	
	protected const ResourceName 										ITEM_FACTION_INFO_CONFIG = "{80DCFB729B7255F7}Configs/Inventory/ItemHints/ItemFaction_ItemHint.conf";
	protected ref map<FactionKey, ref SCR_FactionOutfitItemHintUIInfo> 	m_mItemFactionUIInfos = new map<FactionKey, ref SCR_FactionOutfitItemHintUIInfo>();
	
	//variables dedicated to move an item from storage to storage
	protected IEntity 											m_pItem;
	protected BaseInventoryStorageComponent						m_pDisplayedStorage, m_pLastCurrentNavStorage;
	protected BaseInventoryStorageComponent						m_pStorageFrom, m_pStorageTo;	//last known storages from the last move operation
//	protected SCR_InventorySlotUI	 							m_pStorageUIFrom;
	protected SCR_InventorySlotStorageUI						m_pStorageUITo;	//last known storagesUI from the last move operation
	protected SCR_CharacterVicinityComponent 					m_pVicinity;
	ItemPreviewWidget 											m_wPlayerRender, m_wPlayerRenderSmall;
	PreviewRenderAttributes										m_PlayerRenderAttributes;
	protected ButtonWidget										m_wButtonShowAll;
	protected SCR_InventoryCharacterWidgetHelper 				m_pCharacterWidgetHelper;
	protected ItemPreviewManagerEntity 							m_pPreviewManager;
	protected bool												m_bDraggingEnabled;
	protected FrameWidget										m_wDragDropContainer;
	protected SCR_SlotUIComponent 								m_pDragDropFrameSlotUI;
	protected ItemPreviewWidget									m_pDragDropPreviewImage;
	protected ref array<SCR_InventoryStorageBaseUI>  			m_aStorages = {};
	protected TextWidget										m_wTotalWeightText;
	protected bool												m_bLocked = false;	//helper variable

	//Item/Weapon Switching
	protected SCR_InventoryStorageQuickSlotsUI			m_pQuickSlotStorage;
	protected Widget									m_wQuickSlotStorage;
	protected ref SCR_InvCallBack						m_pCallBack = new SCR_InvCallBack();
	const int											WEAPON_SLOTS_COUNT = 4;

	protected bool												m_bIsUsingGamepad;
	protected float 											m_fX, m_fY;	//debug;
	protected bool												m_bShowIt = true;
	protected int												m_iMouseX, m_iMouseY
	const int													DRAG_THRESHOLD 			= 1;

	//other character's information
	protected SCR_CharacterControllerComponent					m_CharController;
	//protected ProgressBarWidget								m_wInfoStamina;	// Preparation for showing the stamina level in inventory
	protected ref array<HitZone> 								m_aBleedingHitZones = {};

	protected bool												m_bEnablePagingContext;
	protected bool 												m_bWasJustTraversing;
	protected bool 												m_bStorageSwitchMode;
	protected SCR_InventorySlotUI 								m_pItemToAssign;

	protected Widget											m_wAttachmentPointsContainer;
	protected ref array<SCR_InventoryHitZonePointContainerUI> 	m_aHitZonePoints = {};

	protected SCR_InventoryHitZonePointUI 						m_bleedingHandlerGlobal;
	protected SCR_InventoryHitZonePointUI 						m_fractureHandlerGlobal;
	protected SCR_CountingTimerUI		 						m_morphineTimerHandlerGlobal;
	protected SCR_CountingTimerUI		 						m_salineTimerHandlerGlobal;

	protected SCR_CharacterDamageManagerComponent				m_CharDamageManager;
	protected const int											CHARACTER_HITZONES_COUNT = 7;

	protected SCR_InventorySpinBoxComponent 					m_AttachmentSpinBox;

	protected bool 												m_bProcessInitQueue = false;
	protected EInvInitStage 									m_eInitStage = EInvInitStage.BEGIN;
	protected SCR_LoadingOverlay 								m_LoadingOverlay;

	protected const string BUTTON_USE = "ButtonUse";
	protected const string BUTTON_BUY = "ButtonBuy";
	protected const string BUTTON_SELL = "ButtonSell";
	protected const string BUTTON_BACK = "ButtonBack";
	protected const string BUTTON_DROP = "ButtonDrop";
	protected const string BUTTON_MOVE = "ButtonMove";
	protected const string BUTTON_SWAP = "ButtonSwap";
	protected const string BUTTON_EQUIP = "ButtonEquip";
	protected const string BUTTON_SELECT = "ButtonSelect";
	protected const string BUTTON_PICK_UP = "ButtonPickup";
	protected const string BUTTON_INSPECT = "ButtonInspect";
	protected const string BUTTON_STEP_BACK = "ButtonStepBack";
	protected const string BUTTON_OPEN_STORAGE = "ButtonOpenStorage";
	protected const string BUTTON_QUICK_ASSIGN = "ButtonQuickSlotAssign";
	protected const string BUTTON_ENTER_STORAGE = "ButtonEnterStorage";
	protected const string BUTTON_QUICK_UNASSIGN = "ButtonQuickSlotUnassign";
	protected const string BUTTON_OPEN_NEW_CONTAINER = "ButtonOpenAsContainer";
	protected const string BUTTON_REMOVE_TOURNIQUET = "ButtonRemoveTourniquet";

	protected const LocalizedString GAMEPAD_HINT_BACK = "#AR-Menu_Back";
	protected const LocalizedString GAMEPAD_HINT_MOVE = "#AR-Inventory_Move";
	protected const LocalizedString GAMEPAD_HINT_CLOSE = "#AR-Inventory_Close";
	protected const LocalizedString GAMEPAD_HINT_SELECT = "#AR-Inventory_Select";
	protected const LocalizedString GAMEPAD_HINT_DESELECT = "#AR-Inventory_Deselect";
	
	protected static ref ScriptInvokerInventorySlotHover m_OnItemHover;
	protected static ref ScriptInvoker m_OnItemHoverEnd;
	
	//------------------------------------------------------------------------------------------------
	static ScriptInvokerInventorySlotHover GetOnItemHover()
	{
		if(!m_OnItemHover)
			m_OnItemHover = new ScriptInvokerInventorySlotHover();
		
		return m_OnItemHover;
	}

	//------------------------------------------------------------------------------------------------
	static ScriptInvoker GetOnItemHoverEnd()
	{
		if(!m_OnItemHoverEnd)
			m_OnItemHoverEnd = new ScriptInvoker();

		return m_OnItemHoverEnd;
	}

	//------------------------------------------------------------------------------------------------
	ItemPreviewManagerEntity GetItemPreviewManager()
	{
		return m_pPreviewManager;
	}

	//------------------------------------------------------------------------------------------------
	protected void InitializeCharacterHitZones()
	{
		if (m_AttachmentSpinBox)
			m_AttachmentSpinBox.ClearAll();

		const Widget bleeding = m_widget.FindAnyWidget("BleedingGlobal");
		const Widget fracture = m_widget.FindAnyWidget("FractureGlobal");
		const Widget morphineTimer = m_widget.FindAnyWidget("MorphineTimer");
		const Widget salineTimer = m_widget.FindAnyWidget("SalineTimer");

		m_CharController = SCR_CharacterControllerComponent.Cast(m_Player.GetCharacterController());
		m_CharDamageManager = SCR_CharacterDamageManagerComponent.Cast(m_Player.GetDamageManager());
		if (bleeding)
			m_bleedingHandlerGlobal = SCR_InventoryHitZonePointUI.Cast(bleeding.FindHandler(SCR_InventoryHitZonePointUI));
		
		if (fracture)
			m_fractureHandlerGlobal = SCR_InventoryHitZonePointUI.Cast(fracture.FindHandler(SCR_InventoryHitZonePointUI));
		
		if (morphineTimer)
			m_morphineTimerHandlerGlobal = SCR_CountingTimerUI.Cast(morphineTimer.FindHandler(SCR_CountingTimerUI));
		
		if (salineTimer)
			m_salineTimerHandlerGlobal = SCR_CountingTimerUI.Cast(salineTimer.FindHandler(SCR_CountingTimerUI));
	}

	protected bool InitHitZones(bool batchProcessing = false)
	{
		const int hzToInit = m_aHitZonePoints.Count();
		if (hzToInit >= CHARACTER_HITZONES_COUNT)
			return true;

		const bool updateAttachmentStorage = !batchProcessing || hzToInit + 1 == CHARACTER_HITZONES_COUNT;
		const Widget w = GetGame().GetWorkspace().CreateWidgets(HITZONE_CONTAINER_LAYOUT, m_wAttachmentPointsContainer);
		const SCR_InventoryHitZonePointContainerUI point = SCR_InventoryHitZonePointContainerUI.Cast(w.FindHandler(SCR_InventoryHitZonePointContainerUI));
		point.InitializeHitZoneUI(m_pStorageListUI.GetStorage(), this, hzToInit + 1, m_Player, updateAttachmentStorage);
		m_aHitZonePoints.Insert(point);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Creates visualization of damage currently sustained by the character, and spinbox for remeding that damage
	//! NOTE: Use with care as this method is expesinve, as it evaluates all hit zones on the character
	protected void InitializeAllHitZonesUI(bool startingTheProcess = true)
	{
		if (startingTheProcess)
		{
			if (!m_aHitZonePoints.IsEmpty())
				return;
	
			InitializeCharacterHitZones(); // clear existing data and prepare sources for fetching new data
		}

		InitHitZones(true);
		if (m_aHitZonePoints.Count() < CHARACTER_HITZONES_COUNT)
		{
			GetGame().GetCallqueue().CallLater(InitializeAllHitZonesUI, param1: false); //spread the initialization onto different frames as initialization of HZ is expensive
			return;
		}

		OnDamageStateChanged(); // update the visualization of current effects
		UpdateCharacterPreview(); // update the preview and the position of the hit zone indicators
	}

	//------------------------------------------------------------------------------------------------
	protected void HideCharacterHitZones()
	{
		if (m_AttachmentSpinBox)
			m_AttachmentSpinBox.ClearAll();
		if (m_bleedingHandlerGlobal)
			m_bleedingHandlerGlobal.GetRootWidget().SetVisible(false);		
		
		if (m_fractureHandlerGlobal)
			m_fractureHandlerGlobal.GetRootWidget().SetVisible(false);	
		
		if (m_morphineTimerHandlerGlobal)
			m_morphineTimerHandlerGlobal.GetRootWidget().SetVisible(false);

		if (m_salineTimerHandlerGlobal)
			m_salineTimerHandlerGlobal.GetRootWidget().SetVisible(false);

		foreach (SCR_InventoryHitZonePointContainerUI point : m_aHitZonePoints)
		{
			point.GetRootWidget().RemoveFromHierarchy();
		}
		
		m_aHitZonePoints.Clear();
	}

	//------------------------------------------------------------------------------------------------
	// !
	protected void InitializeCharacterInformation()
	{
		SCR_CharacterBloodHitZone charBloodHZ = m_CharDamageManager.GetBloodHitZone();
		charBloodHZ.GetOnDamageStateChanged().Insert(OnDamageStateChanged);

		m_CharDamageManager.GetOnDamageEffectAdded().Insert(OnDamageStateChanged);
		m_CharDamageManager.GetOnDamageEffectRemoved().Insert(OnDamageStateChanged);
		m_CharDamageManager.GetOnDamageStateChanged().Insert(OnDamageStateChanged);
		OnDamageStateChanged();
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_InventoryStorageBaseUI GetWeaponStorage() { return m_pWeaponStorage; }
	
	//------------------------------------------------------------------------------------------------
	SCR_InventorySlotStorageUI GetSelectedSlotStorageUI() { return m_pSelectedSlotStorageUI; }
	
	//------------------------------------------------------------------------------------------------
	SCR_InventoryStorageBaseUI GetActiveStorageUI() { return m_pActiveStorageUI; }
	
	//------------------------------------------------------------------------------------------------
	SCR_InventorySlotUI GetSelectedSlotUI() { return m_pSelectedSlotUI; }
	
	//------------------------------------------------------------------------------------------------
	void AddItemToAttachmentSelection(string item, Managed data = null)
	{
		if (item.IsEmpty())
			return;
		if (m_AttachmentSpinBox.FindItem(item) < 0)
			m_AttachmentSpinBox.AddItem(item, false, data);

		SetAttachmentSpinBoxActive(m_bIsUsingGamepad);
	}

	//------------------------------------------------------------------------------------------------
	void RemoveItemFromAttachmentSelection(string item)
	{
		int index = m_AttachmentSpinBox.FindItem(item);
		if (index > -1)
			m_AttachmentSpinBox.RemoveItem(index, false);
		
		if (m_pDamageInfo && m_pDamageInfo.GetName() == item)
			m_pDamageInfo.Destroy();

		SetAttachmentSpinBoxActive(m_bIsUsingGamepad);
	}

	//------------------------------------------------------------------------------------------------
	SCR_InventorySpinBoxComponent GetAttachmentSpinBoxComponent()
	{
		return m_AttachmentSpinBox;
	}

	//------------------------------------------------------------------------------------------------
	// !
	void OnDamageStateChanged()
	{
		SCR_CharacterBloodHitZone charBloodHZ = m_CharDamageManager.GetBloodHitZone();

		//TODO@FAC change this strong treshold and make better condition when effects work
		const bool bleedingVisible = (charBloodHZ.GetHealthScaled() <= charBloodHZ.GetDamageStateThreshold(ECharacterBloodState.STRONG));
		float bleedingAmount = Math.InverseLerp(
			charBloodHZ.GetDamageStateThreshold(ECharacterBloodState.UNCONSCIOUS),
			charBloodHZ.GetDamageStateThreshold(ECharacterBloodState.UNDAMAGED),
			charBloodHZ.GetHealthScaled());

		if (m_bleedingHandlerGlobal)
		{
			m_bleedingHandlerGlobal.GetRootWidget().SetVisible(bleedingVisible);
			m_bleedingHandlerGlobal.SetBloodLevelProgress(bleedingAmount);
		}		
		
		if (m_fractureHandlerGlobal)
			m_fractureHandlerGlobal.GetRootWidget().SetVisible(m_CharDamageManager.GetMovementDamage() > 0);
		
		bool visible = false;
		if (m_morphineTimerHandlerGlobal)
		{
			array<ref SCR_PersistentDamageEffect> effects = {};
			m_CharDamageManager.FindAllDamageEffectsOfTypeOnHitZone(SCR_MorphineDamageEffect, m_CharDamageManager.GetDefaultHitZone(), effects);
			
			foreach (SCR_PersistentDamageEffect effect : effects)
			{
				visible = true;
				break;
			}
			
			m_morphineTimerHandlerGlobal.GetRootWidget().SetVisible(visible);
		}
		
		if (m_salineTimerHandlerGlobal)
		{
			visible = false;
			array<ref SCR_PersistentDamageEffect> effects = {};
			m_CharDamageManager.FindAllDamageEffectsOfTypeOnHitZone(SCR_SalineDamageEffect, charBloodHZ, effects);
			
			foreach (SCR_PersistentDamageEffect effect : effects)
			{
				visible = true;
				break;
			}
			
			m_salineTimerHandlerGlobal.GetRootWidget().SetVisible(visible);
		}
	}

	//------------------------------------------------------------------------------------------------
	// !
	void RegisterUIStorage( SCR_InventorySlotUI pStorageUI )
	{
		if ( m_aShownStorages.Find( pStorageUI ) > -1 )
			return;

		if ( SCR_InventorySlotStorageUI.Cast( pStorageUI ) || SCR_InventorySlotWeaponUI.Cast( pStorageUI ) )
			m_aShownStorages.Insert( pStorageUI );
	}

	//------------------------------------------------------------------------------------------------
	// !
	void UnregisterUIStorage( SCR_InventorySlotUI pStorageUI )
	{
		if ( pStorageUI )
			m_aShownStorages.RemoveItem( pStorageUI );
	}

	//------------------------------------------------------------------------------------------------
	void SetActiveStorage( SCR_InventoryStorageBaseUI storageUI )
	{
		m_pPrevActiveStorageUI = m_pActiveStorageUI;
		m_pActiveStorageUI = storageUI;
		if( m_pActiveStorageUI )
			SetSlotFocused(m_pActiveStorageUI.GetFocusedSlot(), m_pActiveStorageUI, true);
	}

	//------------------------------------------------------------------------------------------------
	//---Get total weight from all storages (weapon, gadget, deposit)
	protected float GetTotalWeight()
	{
		if (!m_InventoryManager)
			return 0;
		
		return m_InventoryManager.GetTotalWeightOfAllStorages();
	}

	//------------------------------------------------------------------------------------------------
	SCR_InventoryStorageManagerComponent GetInventoryStorageManager()
	{
		return m_InventoryManager;
	}

	//------------------------------------------------------------------------------------------------
	//! Actualy opened UI storage in character's storage
	SCR_InventoryStorageBaseUI GetActualStorageInCharacterStorageUI() { return m_pStorageBaseUI; }
	SCR_InventoryStorageBaseUI GetStorageList() { return m_pStorageListUI; }
	SCR_InventoryStorageBaseUI GetLootStorage() { return m_pStorageLootUI; }

	//------------------------------------------------------------------------------------------------
	//! Returns whether interaction with inventory items is possible
	bool GetCanInteract()
	{
		if (GetInspectionScreen())
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	void RefreshPlayerWidget()
	{
		if (!m_pPreviewManager)
			return;

		if (m_pInspectedSlot)
		{
			UpdateGearInspectionPreview();
			return;
		}

		if (m_wPlayerRender)
			m_pPreviewManager.SetPreviewItem(m_wPlayerRender, m_Player, m_PlayerRenderAttributes, true);
		if (m_wPlayerRenderSmall)
			m_pPreviewManager.SetPreviewItem(m_wPlayerRenderSmall, m_Player, m_PlayerRenderAttributes);
	}

	protected bool ProcessInitQueue()
	{
		switch (m_eInitStage)
		{
			case EInvInitStage.BEGIN:
			{
				m_eInitStage = EInvInitStage.NAVIGATION_BAR;
			} break;

			case EInvInitStage.NAVIGATION_BAR:
			{
				m_pNavigationBar.SetAllButtonEnabled(false);
				m_pNavigationBar.FillFromConfig();
				NavigationBarUpdate();
				m_eInitStage = EInvInitStage.QUICK_SLOTS;
			} break;

			case EInvInitStage.QUICK_SLOTS:
			{
				InitQuickSlots();
				m_eInitStage = EInvInitStage.WEAPON_STORAGE;
			} break;

			case EInvInitStage.WEAPON_STORAGE:
			{
				ShowEquipedWeaponStorage();
				m_eInitStage = EInvInitStage.STORAGE_LIST;
			} break;

			case EInvInitStage.STORAGE_LIST:
			{
				ShowStoragesList();
				m_eInitStage = EInvInitStage.STORAGES;
			} break;

			case EInvInitStage.STORAGES:
			{
				ShowAllStoragesInList();
				m_eInitStage = EInvInitStage.HIT_ZONES;
			} break;

			case EInvInitStage.HIT_ZONES:
			{
				if (InitHitZones())
				{
					m_wPlayerRender.SetVisible(true);

					m_eInitStage = EInvInitStage.DONE;
				}
			} break;

			case EInvInitStage.DONE:
			{
				OnQueueProcessed();
				return true;
			}
		}

		return false;
	}

	protected void OnQueueProcessed()
	{
		m_bProcessInitQueue = false;
		m_wAttachmentPointsContainer.SetVisible(true);
		UpdateCharacterPreview();
		SetStorageSwitchMode(m_bIsUsingGamepad);
		if (m_LoadingOverlay)
			m_LoadingOverlay.GetRootWidget().RemoveFromHierarchy();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);

		if (m_bProcessInitQueue)
		{
			if (!ProcessInitQueue())
				return;
		}

		if (m_bEnablePagingContext)
			GetGame().GetInputManager().ActivateContext("InventoryPagingContext");

		GetGame().GetInputManager().ActivateContext("InventoryMenuContext");

		if (m_InspectionScreen)
		{
			m_InspectionScreen.UpdateView(tDelta);
			return;
		}

		if (m_pCharacterWidgetHelper && m_pPreviewManager && m_PlayerRenderAttributes && m_pCharacterWidgetHelper.Update(tDelta, m_PlayerRenderAttributes))
		{
			UpdatePreview();
		}
	}

	//------------------------------------------------------------------------------------------------
	void UpdatePreview()
	{
		if (m_pInspectedSlot)
			UpdateGearInspectionPreview();
		else
			UpdateCharacterPreview();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateCharacterPreview()
	{
		if (!m_wPlayerRender)
			return;

		m_pPreviewManager.SetPreviewItem(m_wPlayerRender, m_Player, m_PlayerRenderAttributes);

		for (int i, count = m_aHitZonePoints.Count(); i < count; i++)
		{
			SCR_InventoryHitZonePointContainerUI hp = m_aHitZonePoints[i];
			if (!hp)
				continue;
			TNodeId id = hp.GetBoneIndex();

			vector transform[4];
			Math3D.MatrixIdentity4(transform);
			float xOffset = 0;
			if (hp.GetHitZoneGroup() == ECharacterHitZoneGroup.HEAD)
				xOffset = -40;

			vector screenPos;
			if (m_wPlayerRender.TryGetItemNodePositionInWidgetSpace(id, transform, screenPos))
			{
 				FrameSlot.SetPos(hp.GetRootWidget(), screenPos[0] + xOffset, screenPos[1]);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateGearInspectionPreview()
	{	
		if (!m_pInspectedSlot)
		{
			InspectItem(null);
			return;
		}
		
		InventoryItemComponent itemComp = m_pInspectedSlot.GetInventoryItemComponent();
		
		if (!itemComp)
		{
			InspectItem(null);
			return;
		}
		
		m_pPreviewManager.SetPreviewItem(m_wPlayerRender, itemComp.GetOwner(), m_PlayerRenderAttributes, true);
		
		if (m_wPlayerRender && m_pGearInspectionPointUI)
			m_pGearInspectionPointUI.UpdatePreviewSlotWidgets(m_wPlayerRender);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		if( !Init() )
		{
			Action_CloseInventory();
			return;
		}

		GetGame().SetViewDistance(GetGame().GetMinimumViewDistance());

		ShowVicinity();

		m_bProcessInitQueue = true;

		if (m_pPreviewManager)
		{
			m_wPlayerRender = ItemPreviewWidget.Cast( m_widget.FindAnyWidget( "playerRender" ) );
			auto collection = m_StorageManager.GetAttributes();
			if (collection)
				m_PlayerRenderAttributes = PreviewRenderAttributes.Cast(collection.FindAttribute(SCR_CharacterInventoryPreviewAttributes));

			m_pCharacterWidgetHelper = SCR_InventoryCharacterWidgetHelper(m_wPlayerRender, GetGame().GetWorkspace() );
		}

		if( m_pNavigationBar )
			m_pNavigationBar.m_OnAction.Insert(OnAction);

		GetGame().GetInputManager().AddActionListener("Inventory_Drag", EActionTrigger.DOWN, Action_DragDown);
		GetGame().GetInputManager().AddActionListener("Inventory", EActionTrigger.DOWN, Action_CloseInventory);
		InitAttachmentSpinBox();
		OnInputDeviceIsGamepad(!GetGame().GetInputManager().IsUsingMouseAndKeyboard());
		GetGame().OnInputDeviceIsGamepadInvoker().Insert(OnInputDeviceIsGamepad);		
		
		SetAttachmentSpinBoxActive(m_bIsUsingGamepad);
		
		ResetHighlightsOnAvailableStorages();
		if (!SetOpenStorage())
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INV_OPEN);
		UpdateTotalWeightText();
		
		InitializeCharacterHitZones();
		InitializeCharacterInformation();
		UpdateCharacterPreview();
	}

	void InitAttachmentSpinBox()
	{
		if (m_AttachmentSpinBox)
		{
			m_AttachmentSpinBox.ClearAll();
			return;
		}

		Widget w = m_widget.FindAnyWidget("AttachmentSpinBox");
		if (!w)
		{
			Print("Cannot find AttachmentSpinBox widget in hierarchy!", LogLevel.ERROR);
			return;
		}

		m_AttachmentSpinBox = SCR_InventorySpinBoxComponent.Cast(w.FindHandler(SCR_InventorySpinBoxComponent));
		m_AttachmentSpinBox.m_OnChanged.Insert(NavigationBarUpdate);
	}


	//------------------------------------------------------------------------------------------------
	protected void SetStorageSwitchMode(bool enabled, bool updateNavBar = true)
	{
		m_bStorageSwitchMode = enabled;
		foreach (SCR_InventoryStorageBaseUI storage : m_aStorages)
		{
			if (storage)
				storage.ActivateStorageButton(m_bStorageSwitchMode);
		}

		m_pStorageLootUI.ActivateStorageButton(m_bStorageSwitchMode);
		if (m_pStorageListUI)
			m_pStorageListUI.ActivateStorageButton(m_bStorageSwitchMode);

		foreach (SCR_InventoryOpenedStorageUI storage : m_aOpenedStoragesUI)
		{
			storage.ActivateStorageButton(m_bStorageSwitchMode);
		}

		if (m_bStorageSwitchMode)
		{
			HideItemInfo();
			SCR_InventoryStorageBaseUI focusedStorage = m_pStorageLootUI;
			if (m_pActiveStorageUI)
				focusedStorage = m_pActiveStorageUI;

			// delay by one frame or else when you are selecting an item for transfer (specifically observed with supplies), then game might think that you depressed the transfer button with focus being on this storage, thus instantly dropping that selected item into this storage
			GetGame().GetCallqueue().Call(FocusWidget, param1: focusedStorage.GetButtonWidget());
		}
		else if (m_pActiveStorageUI)
		{
			const SCR_InventoryStorageBaseUI previouslyFocusedStorage = m_pActiveStorageUI; // store locally a reference to it as when focus is transfered, then m_pActiveStorageUI may change or even become null
			if (FocusOnSlotInStorage(m_pActiveStorageUI, m_pActiveStorageUI.GetSlotId(m_pActiveStorageUI.GetLastFocusedSlot())))
				previouslyFocusedStorage.ShowContainerBorder(false);
		}

		if (updateNavBar)
			NavigationBarUpdate();
	}

	//------------------------------------------------------------------------------------------------
	protected void ToggleStorageSwitchMode()
	{
		SetStorageSwitchMode(!m_bStorageSwitchMode);
	}

	protected void SetAttachmentSpinBoxActive(bool enable)
	{
		if (!m_AttachmentSpinBox)
			return;

		const int itemCount = m_AttachmentSpinBox.GetNumItems();
		const bool shouldBeActive = (enable && (itemCount > 0));

		m_AttachmentSpinBox.SetEnabled(shouldBeActive);

		const Widget rootWidget = m_AttachmentSpinBox.GetRootWidget();
		rootWidget.SetVisible(shouldBeActive);

		const Widget leftArea = m_widget.FindAnyWidget("LeftArea");
		const Widget rightArea = m_widget.FindAnyWidget("StoragesListSlot");

		// Switches navigation targets based on the presence of the spinbox in the center
		if (shouldBeActive)
		{
			leftArea.SetNavigation(WidgetNavigationDirection.RIGHT, WidgetNavigationRuleType.EXPLICIT, "AttachmentSpinBox");
			rightArea.SetNavigation(WidgetNavigationDirection.LEFT, WidgetNavigationRuleType.EXPLICIT, "AttachmentSpinBox");
			rootWidget.SetNavigation(WidgetNavigationDirection.RIGHT, WidgetNavigationRuleType.EXPLICIT, "StoragesListSlot");
			rootWidget.SetNavigation(WidgetNavigationDirection.LEFT, WidgetNavigationRuleType.EXPLICIT, "StorageLootSlot");
		}
		else
		{
			leftArea.SetNavigation(WidgetNavigationDirection.RIGHT, WidgetNavigationRuleType.EXPLICIT, "StoragesListSlot");
			rightArea.SetNavigation(WidgetNavigationDirection.LEFT, WidgetNavigationRuleType.EXPLICIT, "StorageLootSlot");
		}
	}

	//------------------------------------------------------------------------------------------------
	bool CanFocusOnSlotInStorage(SCR_InventoryStorageBaseUI storage, int slotIndex)
	{
		if (!storage)
			return false;

		array<SCR_InventorySlotUI> slots = {};
		storage.GetSlots(slots);
		return slots.IsIndexValid(slotIndex);
	}

	//------------------------------------------------------------------------------------------------
	//! Attempts to focus on a slot from provided storage
	//! \param[in] storageUi
	//! \param[in] slotIndex of the slot which should be focused
	//! \return true if focus transfer should be considered as successful, otherwise returns false
	bool FocusOnSlotInStorage(SCR_InventoryStorageBaseUI storageUi, int slotIndex = 0)
	{
		if (!storageUi)
			return false;

		array<SCR_InventorySlotUI> slots = {};
		storageUi.GetSlots(slots);

		if (slots.IsEmpty())
		{
			if (!m_bIsUsingGamepad) // in case of M&K dont go back to the storage mode, as M&K doesnt support that mode, and user will not be able to turn it off
				return true;

			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.CLICK);
			SetStorageSwitchMode(true, false); // if this storage doesnt have any slots on which we could focus, then lets go back to the storage switch mode
			return false;
		}

		if (!slots.IsIndexValid(slotIndex))
			slotIndex = 0; // if provided index is invalid then use first slot

		SCR_InventorySlotUI uiSlot = slots[slotIndex];
		Widget button;
		if (uiSlot)
			button = uiSlot.GetButtonWidget();

		if (!button || !button.IsEnabled())
			uiSlot = FindFirstFocusableUiSlot(slots, button, slotIndex);

		if (!uiSlot)
		{
			if (!m_bIsUsingGamepad)
				return true;

			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.CLICK);
			SetStorageSwitchMode(true, false); // since we dont have a button to focus on we switch back to the storage mode
			return false;
		}

		GetGame().GetWorkspace().SetFocusedWidget(button);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Finds first ui slot which can be focused
	//! \param[in] slots an array of ui slots to check
	//! \param[out] button of the found valid ui slot. If such slot was not found then this will be null
	//! \param[in] ignoredId of the slot that shouldnt be checked
	//! \return slot which can be focused, and if such slot is not found then returns null
	protected SCR_InventorySlotUI FindFirstFocusableUiSlot(notnull array<SCR_InventorySlotUI> slots, out Widget button = null, int ignoredId = -1)
	{
		foreach (int i, SCR_InventorySlotUI uiSlot : slots)
		{
			if (ignoredId == i)
				continue;

			if (!uiSlot || !uiSlot.IsSlotEnabled())
				continue;

			button = uiSlot.GetButtonWidget();
			if (button && button.IsEnabled())
				return uiSlot;
		}

		button = null;
		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! returns true if the init procedure was success
	protected bool Init()
	{
		m_widget = GetRootWidget();

		if ( m_widget == null )
			return false;

		Widget loading = m_widget.FindAnyWidget("LoadingOverlay");
		if (loading)
		{
			loading.SetVisible(true);
			m_LoadingOverlay = SCR_LoadingOverlay.Cast(loading.FindHandler(SCR_LoadingOverlay));
		}

		//Get player
		PlayerController playerController = GetGame().GetPlayerController();
		if ( playerController != null )
		{
			m_Player = ChimeraCharacter.Cast(playerController.GetControlledEntity());
			if ( m_Player != null )
			{
				ChimeraWorld world = ChimeraWorld.CastFrom(m_Player.GetWorld());
				if (!world)
					return false;
				
				m_pPreviewManager = world.GetItemPreviewManager();
				if (!m_pPreviewManager)
				{
					Resource rsc = Resource.Load(m_ItemPreviewManagerPrefab);
					if (rsc.IsValid())
						GetGame().SpawnEntityPrefabLocal(rsc, world);
					
					m_pPreviewManager = world.GetItemPreviewManager();
				}

				m_CharController = SCR_CharacterControllerComponent.Cast(m_Player.GetCharacterController());
				if (m_CharController)
				{
					m_CharController.m_OnLifeStateChanged.Insert(LifeStateChanged);
					if (m_CharController.GetLifeState() != ECharacterLifeState.ALIVE)
						return false;
				}
				
				SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
				if (gameMode)
					gameMode.GetOnControllableDeleted().Insert(OnControllableDeleted);
				
				//Inventory Manager
				m_InventoryManager = SCR_InventoryStorageManagerComponent.Cast( m_Player.FindComponent( SCR_InventoryStorageManagerComponent ) );
				m_StorageManager = SCR_CharacterInventoryStorageComponent.Cast( m_Player.FindComponent( SCR_CharacterInventoryStorageComponent ) );
				if( !m_StorageManager )
					return false;
				m_pWeaponStorageComp = EquipedWeaponStorageComponent.Cast(m_StorageManager.GetWeaponStorage());
				if( !m_pWeaponStorageComp )
					return false;
				if ( m_InventoryManager )
				{
					m_InventoryManager.m_OnItemAddedInvoker.Insert(OnItemAddedListener);
					m_InventoryManager.m_OnItemRemovedInvoker.Insert(OnItemRemovedListener);
				}

				Widget wNaviBar = m_widget.FindAnyWidget( "Footer" );
				if( wNaviBar )
					m_pNavigationBar = SCR_NavigationBarUI.Cast( wNaviBar.FindHandler( SCR_NavigationBarUI ) );
				m_pVicinity = SCR_CharacterVicinityComponent.Cast(m_Player.FindComponent(SCR_CharacterVicinityComponent));
				if (m_pVicinity)
				{
					m_pVicinity.OnVicinityUpdateInvoker.Insert(RefreshLootUIListener);
					m_iVicinityDiscoveryRadius = m_pVicinity.GetDiscoveryRadius();
					IEntity itemOfInterest = m_pVicinity.GetItemOfInterest();
					if (itemOfInterest && itemOfInterest.IsInherited(SCR_ChimeraCharacter))
					{
						SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(itemOfInterest);
						SCR_CharacterControllerComponent charCtrl = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
						if (charCtrl)
							charCtrl.m_OnLifeStateChanged.Insert(OnLootedCharacterLifeStateChanged);
				}
				}

				m_wStoragesContainer = GridLayoutWidget.Cast( m_widget.FindAnyWidget( "StorageGrid" ) );
				m_wTotalWeightText = TextWidget.Cast( m_widget.FindAnyWidget("TotalWeightText") );
			}
			else
			{
				return false;
			}

		}
		else
		{
			return false;
		}

		m_wDragDropContainer = FrameWidget.Cast( m_widget.FindAnyWidget( "DragDropContainer" ) );
		if ( m_wDragDropContainer )
		{
			m_pDragDropFrameSlotUI = SCR_SlotUIComponent.Cast( m_wDragDropContainer.FindHandler( SCR_SlotUIComponent ) );
			m_pDragDropPreviewImage = ItemPreviewWidget.Cast( m_wDragDropContainer.FindAnyWidget( "item" ) );
		}

		m_wAttachmentPointsContainer = m_widget.FindAnyWidget("AttachmentPoints");

		return true;
	}

	protected void OnLootedCharacterLifeStateChanged(ECharacterLifeState previousLifeState, ECharacterLifeState newLifeState)
	{
		if (newLifeState == ECharacterLifeState.ALIVE)
		{
			IEntity itemOfInterest = m_pVicinity.GetItemOfInterest();
			if (itemOfInterest && itemOfInterest.IsInherited(SCR_ChimeraCharacter))
			{
				SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(itemOfInterest);
				SCR_CharacterControllerComponent charCtrl = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
				if (charCtrl)
					charCtrl.m_OnLifeStateChanged.Remove(OnLootedCharacterLifeStateChanged);
			}

			m_pVicinity.SetItemOfInterest(null);
			RefreshLootUIListener();
		}
	}

	//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
	//------------------------------------------------------------------------------------------------
	void OnAction( SCR_InputButtonComponent comp, string action, SCR_InventoryStorageBaseUI pParentStorage = null, int traverseStorageIndex = -1 )
	{
		switch (action)
		{
			case "Inventory":
			{
				Action_CloseInventory();
			}
			break;

			case "Inventory_UnassignFromQuickSlot":
			{
				Action_QuickSlotUnassign();
			} break;

			case "Inventory_EnterStorage":
			{
				SetStorageSwitchMode(false);
			} break;

			case "Inventory_AssignToQuickSlot":
			{
				Action_QuickSlotAssign();
			} break;

			case "Inventory_Drop":
			{
				Action_MoveBetween();
			} break;

			case "Inventory_Equip":
			{
				Action_EquipItem();
			} break;

			case "Inventory_Select":
			{
				if (m_pAttachmentStorageUI && m_AttachmentSpinBox.IsFocused())
				{
					FocusOnSlotInStorage(m_pAttachmentStorageUI);
					return;
				}

				if (m_bStorageSwitchMode && m_bIsUsingGamepad)
				{
					SetStorageSwitchMode(false);
					return;
				}
				if (m_pFocusedSlotUI && !m_pFocusedSlotUI.IsSelected())
					Action_SelectItem();
				else
					Action_DeselectItem();
			} break;

			case "Inventory_Deselect":
			{
				Action_DeselectItem();
			} break;

			case "Inventory_OpenStorage":
			{
				Action_UnfoldItem();
			} break;

			case "Inventory_OpenNewStorage":
			{
				Action_OpenContainer();
			} break;

			case "Inventory_StepBack":
			{
				Action_StepBack( pParentStorage, traverseStorageIndex );
			} break;

			case "Inventory_Inspect":
			{
				Action_Inspect();
			} break;

			case "Inventory_Use":
			{
				Action_UseItem();
			} break;

			case "Inventory_Move":
			{
				if (m_bStorageSwitchMode)
				{
					Action_Drop();
					return;
				}
				else if (m_pSelectedSlotUI)
				{
					Action_Drop();
					return;
				}
				
				Action_MoveItemToStorage();
			} break;

			case "Inventory_Swap":
			{
				if (m_bStorageSwitchMode)
				{
					ToggleStorageSwitchMode();
					FocusOnSlotInStorage(m_pActiveStorageUI);
					return;
				}
				Action_SwapItems(m_pSelectedSlotUI, m_pFocusedSlotUI);
			} break;
			
			case "Inventory_DetachItem":
			{
				SCR_InventoryHitZonePointContainerUI hzContainer = SCR_InventoryHitZonePointContainerUI.Cast(m_AttachmentSpinBox.GetCurrentItemData());
				if (hzContainer)
					hzContainer.RemoveTourniquetFromSlot();
			} break;

			case "InventoryEscape":
			{
				Action_TryCloseInventory();
			} break;
		}
	}
	//---- REFACTOR NOTE END ----

	//------------------------------------------------------------------------------------------------
	protected int CloseOpenedStorage()
	{
		if ( !m_wContainer ) // what?
			return -1;	//any storage opened
		return CloseStorage( m_wContainer );
	}

	//------------------------------------------------------------------------------------------------
	protected int CloseStorage( SCR_InventoryStorageBaseUI pStorageUI )
	{		
		//if (!pStorageUI)
			//return -1;
		Widget w = pStorageUI.GetRootWidget();
		if ( !w )
			return -1;
		
		//~ Close linked storages one frame later to make sure the the current storage is closed
		CloseLinkedStorages(pStorageUI.GetStorage());
		
		return CloseStorage( w );
	}

	//------------------------------------------------------------------------------------------------
	protected int CloseStorage( notnull Widget w )
	{
		auto storageUIHandler = SCR_InventoryStorageBaseUI.Cast( w.FindHandler( SCR_InventoryStorageBaseUI ) );
		if ( !storageUIHandler )
			return -1;	//some storage opened, but it does not have any handler ( wrong )

		int iLastShownPage = storageUIHandler.GetLastShownPage();
		m_pLastCurrentNavStorage = storageUIHandler.GetCurrentNavigationStorage();
		w.RemoveHandler( storageUIHandler );	//remove the handler from the widget
		w.RemoveFromHierarchy();
		
		return iLastShownPage;
	}

	//------------------------------------------------------------------------------------------------
	// ! Hides/Unhides ( not closes ) the storage
	protected void ToggleStorageContainerVisibility( notnull SCR_InventorySlotUI pSlot )
	{
		BaseInventoryStorageComponent pStorage = pSlot.GetAsStorage();
		if ( !pStorage )
			return;
		SCR_InventoryStorageBaseUI pStorageUI = GetStorageUIByBaseStorageComponent( pStorage );
		if ( !pStorageUI )
			return;
		pStorageUI.ToggleShow();
	}

	//------------------------------------------------------------------------------------------------
	// ! Shows the content of the 1st available storage
	void ShowStorage()
	{
		if( !m_StorageManager )
			return;

		array<SCR_UniversalInventoryStorageComponent> pStorages = new array<SCR_UniversalInventoryStorageComponent>();
		m_StorageManager.GetStorages( pStorages );

		if( pStorages.Count() == 0 )
			return;
		SCR_UniversalInventoryStorageComponent pStorage = pStorages.Get( 0 );
		ShowStorage( pStorage );
	}

	//------------------------------------------------------------------------------------------------
	//!
	void ShowStorage( BaseInventoryStorageComponent storage, LoadoutAreaType area = null )
	{
		if ( !m_wStoragesContainer )
			return;
		m_wContainer =  GetGame().GetWorkspace().CreateWidgets( BACKPACK_STORAGE_LAYOUT, m_wStoragesContainer );
		int iRow = Math.Floor( m_iStorageListCounter / STORAGE_AREA_COLUMNS );
		int iCol = m_iStorageListCounter % STORAGE_AREA_COLUMNS;
		m_iStorageListCounter++;

		m_wStoragesContainer.SetRowFillWeight( iRow, 0 );
		m_wStoragesContainer.SetColumnFillWeight( iCol, 0 );

		GridSlot.SetColumn( m_wContainer, iCol );
		GridSlot.SetRow( m_wContainer, iRow );
		GridSlot.SetColumnSpan( m_wContainer, 1 );
		GridSlot.SetRowSpan( m_wContainer, 1 );

		if ( !m_wContainer )
			return;
		if ( storage.Type() == ClothNodeStorageComponent )
		{
			m_wContainer.AddHandler( new SCR_InventoryStorageLBSUI( storage, area, this, 0, null) );
			m_pStorageBaseUI = SCR_InventoryStorageBaseUI.Cast( m_wContainer.FindHandler( SCR_InventoryStorageLBSUI ) );
		}
		else if ( storage.Type() == EquipedWeaponStorageComponent )
		{
			m_wContainer.AddHandler( new SCR_InventoryStorageWeaponsUI( m_StorageManager.GetWeaponStorage(), area, this ) );
			m_pStorageBaseUI = SCR_InventoryStorageWeaponsUI.Cast( m_wContainer.FindHandler( SCR_InventoryStorageWeaponsUI ) );
		}
		else
		{
			m_wContainer.AddHandler( new SCR_InventoryStorageBackpackUI( storage, area, this, 0, null ) );
			m_pStorageBaseUI = SCR_InventoryStorageBaseUI.Cast( m_wContainer.FindHandler( SCR_InventoryStorageBaseUI ) );
		}

		m_aStorages.Insert( m_pStorageBaseUI );
	}

	//------------------------------------------------------------------------------------------------
	void ShowVicinity(bool compact = false) // if true, vicinity will have only 4 rows instead of 6
	{
		if (!m_pVicinity)
		{
			Print("No vicnity component on character!", LogLevel.DEBUG);
			return;
		}

		if ( m_wLootStorage )
		{
			m_wLootStorage.RemoveHandler( m_wLootStorage.FindHandler( SCR_InventoryStorageLootUI ) );	//remove the handler from the widget
			m_wLootStorage.RemoveFromHierarchy();
		}

		Widget parent = m_widget.FindAnyWidget( "StorageLootSlot" );
		m_wLootStorage =  GetGame().GetWorkspace().CreateWidgets( BACKPACK_STORAGE_LAYOUT, parent );
		if ( !m_wLootStorage )
			return;

		if (compact)
			m_wLootStorage.AddHandler( new SCR_InventoryStorageLootUI( null, null, this, 0, null, m_Player, 4, 6 ) );
		else
			m_wLootStorage.AddHandler( new SCR_InventoryStorageLootUI( null, null, this, 0, null, m_Player ) );
		m_pStorageLootUI = SCR_InventoryStorageBaseUI.Cast( m_wLootStorage.FindHandler( SCR_InventoryStorageLootUI ) );
	}

	//------------------------------------------------------------------------------------------------
	void ShowAttachmentStorage(InventorySearchPredicate searchPredicate, bool closeOnly = false)
	{
		if (m_wAttachmentStorage)
		{
			m_wAttachmentStorage.RemoveHandler(m_wAttachmentStorage.FindHandler(SCR_InventoryAttachmentStorageUI));
			if (closeOnly)
				return;
		}

		if (!m_wAttachmentStorage)
		{
			const Widget parent = m_widget.FindAnyWidget("AttachmentStorage");
			if (!parent)
				return;

			m_wAttachmentStorage = GetGame().GetWorkspace().CreateWidgets(BACKPACK_STORAGE_LAYOUT, parent);
			if (!m_wAttachmentStorage)
				return;
		}

		SCR_InventoryAttachmentStorageUI handler = new SCR_InventoryAttachmentStorageUI(m_pStorageListUI.GetStorage(), null, this, 0, null, searchPredicate);
		m_wAttachmentStorage.AddHandler(handler);
		m_pAttachmentStorageUI = handler;
	}

	//------------------------------------------------------------------------------------------------
	void CloseAttachmentStorage()
	{
		if (!m_wAttachmentStorage)
			return;

		m_wAttachmentStorage.RemoveHandler(m_wAttachmentStorage.FindHandler(SCR_InventoryAttachmentStorageUI));
	}

	//------------------------------------------------------------------------------------------------
	SCR_InventoryAttachmentStorageUI GetAttachmentStorageUI()
	{
		return m_pAttachmentStorageUI;
	}

	//------------------------------------------------------------------------------------------------
	void RemoveAttachmentStorage(SCR_InventoryAttachmentStorageUI attStorage)
	{
		CloseStorage(attStorage);
	}

	//------------------------------------------------------------------------------------------------
	// shows opened storage in a new container
	void OpenStorageAsContainer(BaseInventoryStorageComponent storage, bool showVicinity = true, bool hideCloseButton = false)
	{
		foreach (SCR_InventoryOpenedStorageUI openedStorage : m_aOpenedStoragesUI)
		{
			if (openedStorage.GetStorage() == storage)
			{
				// if storage is already open, close it instead
				RemoveOpenStorage(openedStorage);
				return;
			}
		}

		SCR_InventoryOpenedStorageUI handler = CreateOpenedStorageUI(storage);

		Widget parent = m_widget.FindAnyWidget("OpenedStorages");
		Widget newStorage;

		if (SCR_InventoryOpenedStorageArsenalUI.Cast(handler))
			newStorage = GetGame().GetWorkspace().CreateWidgets(ARSENAL_STORAGE_LAYOUT, parent);
		else
			newStorage = GetGame().GetWorkspace().CreateWidgets(BACKPACK_STORAGE_LAYOUT, parent);

		newStorage.AddHandler(handler);

		ButtonWidget closeBtn = handler.ActivateCloseStorageButton();

		ScrollLayoutWidget scroll = ScrollLayoutWidget.Cast(m_widget.FindAnyWidget("LeftAreaScroll"));

		if (scroll)
			scroll.SetSliderPos(0, 0);

		m_aOpenedStoragesUI.Insert(handler);
		
		if (showVicinity)
			ShowVicinity(true);
		
		if (hideCloseButton)
		{
			Widget closeButton = newStorage.FindAnyWidget(STORAGE_LAYOUT_CLOSE_WIDGET_NAME);
			if (closeButton)
				closeButton.SetVisible(false);
		}
		
		OpenLinkedStorages(storage, showVicinity, true);
		FocusOnSlotInStorage(handler);
	}

	//------------------------------------------------------------------------------------------------
	void RemoveOpenStorage(SCR_InventoryOpenedStorageUI openedStorage)
	{
		CloseStorage(openedStorage);
		m_aOpenedStoragesUI.RemoveItem(openedStorage);
		if (m_aOpenedStoragesUI.IsEmpty())
		{
			ShowVicinity();
		}
	}

	//------------------------------------------------------------------------------------------------
	void RefreshLootUIListener()
	{
		if (!m_pVicinity || !m_pStorageLootUI)
			return;
		
		m_pStorageLootUI.Refresh();
		if (m_aOpenedStoragesUI.IsEmpty())
			return;
		
		vector playerOrigin = m_Player.GetOrigin();
		vector entityBoundsMaxs, entityBoundsMins;
		SCR_InventoryOpenedStorageUI storageUI
		for (int index = m_aOpenedStoragesUI.Count() - 1; index >= 0; index--)
		{
			storageUI = m_aOpenedStoragesUI.Get(index);
			if (!storageUI)
				continue;

			//~ If root entity is a vehicle than always take bounds of the vehicle this is because the cargo could be further away than the vehicle but the vehicle is the only one showing in the inventory
			Vehicle vehicleRoot = Vehicle.Cast(storageUI.GetStorage().GetOwner().GetRootParent());
			if (vehicleRoot)
				vehicleRoot.GetWorldBounds(entityBoundsMins, entityBoundsMaxs);
			//~ Root parent is not a vehicle so take bounds of self
			else 
				storageUI.GetStorage().GetOwner().GetWorldBounds(entityBoundsMins, entityBoundsMaxs);
			
			//~ No bounds on storage and rootparent is not avehicle than try to get bounds of parent instead
			if (entityBoundsMins - entityBoundsMaxs == vector.Zero && !vehicleRoot)
			{
				//~ Get the bounds from parents instead if root is not a vehicle
				if (storageUI.GetStorage().GetOwner().GetParent())
					storageUI.GetStorage().GetOwner().GetParent().GetWorldBounds(entityBoundsMins, entityBoundsMaxs);
				
				//~ Check if bounds were found
				if (entityBoundsMins - entityBoundsMaxs == vector.Zero)
				{
					Print("SCR_InventoryMenuUI: RefreshLootUIListener storage owner has no bounds and it could not find a valid parent with bounds!", LogLevel.ERROR);
					RemoveOpenStorage(storageUI);
					return;
				}				
			}
			
			if (!Math3D.IntersectionSphereAABB(playerOrigin, m_iVicinityDiscoveryRadius, entityBoundsMins, entityBoundsMaxs))
				RemoveOpenStorage(storageUI);
		}
	}

	//------------------------------------------------------------------------------------------------
	void ShowStoragesList()
	{
		if( m_wStorageList )
		{
			m_wStorageList.RemoveHandler( m_wStorageList.FindHandler( SCR_InventoryStoragesListUI ) );	//remove the handler from the widget
			m_wStorageList.RemoveFromHierarchy();
		}

		Widget parent = m_widget.FindAnyWidget( "StoragesListSlot" );
		m_wStorageList =  GetGame().GetWorkspace().CreateWidgets( STORAGES_LIST_LAYOUT, parent );

		if( !m_wStorageList )
			return;

		m_wStorageList.AddHandler( new SCR_InventoryStoragesListUI( m_StorageManager, null, this ) );
		m_pStorageListUI = SCR_InventoryStoragesListUI.Cast( m_wStorageList.FindHandler( SCR_InventoryStoragesListUI ) );
	}

	//------------------------------------------------------------------------------------------------
	void ShowAllStoragesInList()
	{
		BaseInventoryStorageComponent lastFocusedStorage;
		int lastFocusedSlotId = -1;
		if (m_pFocusedSlotUI && !m_bStorageSwitchMode)
		{
			lastFocusedStorage = m_pFocusedSlotUI.GetStorageUI().GetStorage();
			lastFocusedSlotId = m_pFocusedSlotUI.GetStorageUI().GetFocusedSlotId();
		}

		if (m_bIsUsingGamepad && lastFocusedSlotId < 0 && !lastFocusedStorage)
			m_bStorageSwitchMode = true;

		if (m_bStorageSwitchMode && m_pActiveHoveredStorageUI)
			lastFocusedStorage = m_pActiveHoveredStorageUI.GetStorage(); // if we weren't targeting specific slot, but we were 'focused' on a specific storage, then lets try to stay focused on it

		m_iStorageListCounter = 0;
		array<SCR_InventorySlotUI> aSlotsUI = {};
		m_pStorageListUI.GetSlots( aSlotsUI );
		if ( !m_aStorages.IsEmpty() )
		{
			foreach ( SCR_InventoryStorageBaseUI pStorage : m_aStorages )
			{
				if ( !pStorage )
					continue;
				CloseStorage( pStorage );
			}
		}

		SortSlotsByLoadoutArea( aSlotsUI );
		m_aStorages.Resize( aSlotsUI.Count() );

		foreach (SCR_InventorySlotUI pSlot : aSlotsUI)
		{
			if (!pSlot)
				continue;

			BaseInventoryStorageComponent pStorage = pSlot.GetAsStorage();
			if (!pStorage)
				continue;

			ShowStorage(pStorage, pSlot.GetLoadoutArea());
			if (pStorage != lastFocusedStorage)
				continue;

			if (lastFocusedSlotId > -1)
				FocusOnSlotInStorage(m_pStorageBaseUI, lastFocusedSlotId);

			m_pActiveStorageUI = m_pStorageBaseUI;
		}

		if (m_bStorageSwitchMode) // since we just recreated all storages and their slots, we need to inform them that we are in storage switch mode
			SetStorageSwitchMode(m_bStorageSwitchMode, false);

		NavigationBarUpdate();
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_InventoryStorageBaseUI GetStorageUIFromVicinity(BaseInventoryStorageComponent storage)
	{
		if (!storage)
			return null;
		array<SCR_InventorySlotUI> slots = {};
		m_pStorageLootUI.GetSlots(slots);
		foreach (SCR_InventorySlotUI slot : slots)
		{
			if (slot && slot.GetStorageComponent() == storage)
				return slot.GetStorageUI();
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_InventoryOpenedStorageUI CreateOpenedStorageUI(BaseInventoryStorageComponent storage)
	{
		IEntity storageOwner = storage.GetOwner();
		
		SCR_ArsenalInventoryStorageManagerComponent arsenalManagerComponent //! v
			= SCR_ArsenalInventoryStorageManagerComponent.Cast(storageOwner.FindComponent(SCR_ArsenalInventoryStorageManagerComponent));
		
		if (arsenalManagerComponent)
			return new SCR_InventoryOpenedStorageArsenalUI(storage, null, this, 0, {storage});
		
		SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.FindResourceComponent(storageOwner);
		
		if (resourceComponent)
		{
			//% Remove the hard coded type for the resources type.
			SCR_ResourceContainer container = resourceComponent.GetContainer(EResourceType.SUPPLIES);
			
			if (container)
				return new SCR_InventoryStorageContainerUI(storage, null, this, 0, {storage});
		}
		
		return new SCR_InventoryOpenedStorageUI(storage, null, this, 0, {storage});
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_InventoryStorageBaseUI GetStorageUIByBaseStorageComponent( BaseInventoryStorageComponent pStorage )
	{
		if (!pStorage)
			return m_pStorageLootUI;
		
		if (pStorage == m_pWeaponStorageComp)
			return m_pWeaponStorage;
		
		if (pStorage == m_StorageManager)
			return m_pStorageListUI;

		if (pStorage && m_StorageManager && pStorage.GetOwner() == m_StorageManager.GetOwner())
			return m_pStorageListUI;

		BaseInventoryStorageComponent parentStorage;

		if (pStorage && pStorage.GetParentSlot())
			parentStorage = pStorage.GetParentSlot().GetStorage();

		if (ClothNodeStorageComponent.Cast(parentStorage))
		{
			foreach (SCR_InventoryStorageBaseUI pStorageUI : m_aStorages)
			{
				if (pStorageUI && pStorageUI.GetStorage() == parentStorage)
					return pStorageUI;
			}
		}

		foreach (SCR_InventoryStorageBaseUI pStorageUI : m_aStorages)
		{
			if (pStorageUI && pStorageUI.GetStorage() == pStorage)
				return pStorageUI;
		}

		foreach (SCR_InventoryStorageBaseUI pStorageUI : m_aOpenedStoragesUI)
		{
			if (pStorageUI && pStorageUI.GetStorage() == pStorage)
				return pStorageUI;
		}
		
		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected void SortSlotsByLoadoutArea( out array<SCR_InventorySlotUI> aSlots )
	{
		array<SCR_InventorySlotUI> tmpSlots = {};
		tmpSlots.Copy( aSlots );
		aSlots.Clear();

		foreach ( SCR_InventorySlotUI pSlotUI : tmpSlots )
		{
			if ( !SCR_InventorySlotStorageUI.Cast( pSlotUI ) || pSlotUI.GetLoadoutArea() == null )
				continue;
			aSlots.Insert(pSlotUI);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void ShowEquipedWeaponStorage()
 	{
		if ( m_wWeaponSlots )
		{
			m_wWeaponSlots.RemoveHandler( m_wWeaponSlots.FindHandler( SCR_InventoryWeaponSlotsUI ) );	//remove the handler from the widget
			m_wWeaponSlots.RemoveFromHierarchy();
		}

		Widget parent = m_widget.FindAnyWidget( "WeaponSlots" );
		m_wWeaponSlots = GetGame().GetWorkspace().CreateWidgets( WEAPON_STORAGE_LAYOUT, parent );

		if ( !m_wWeaponSlots )
			return;

		m_wWeaponSlots.AddHandler( new SCR_InventoryWeaponSlotsUI( m_StorageManager.GetWeaponStorage(), null, this ) );
		m_pStorageBaseUI = SCR_InventoryWeaponSlotsUI.Cast( m_wWeaponSlots.FindHandler( SCR_InventoryWeaponSlotsUI ) );
		m_pWeaponStorage = SCR_InventoryWeaponSlotsUI.Cast( m_pStorageBaseUI );
		m_pWeaponStorageComp = EquipedWeaponStorageComponent.Cast( m_pWeaponStorage.GetStorage() );
 	}
	
	//------------------------------------------------------------------------------------------------
	//~ Add any general hints not found on the item itself such as arsenal cost when the item is in an arsenal
	protected void GetGeneralItemHintsInfos(out notnull array<SCR_InventoryItemHintUIInfo> hintsInfo)
	{		
		bool arsenalCostSet;
		
		//~ Arsenal supply cost hint if item is in an arsenal storage
		if (m_SupplyCostUIInfo)
		{
			SCR_ArsenalInventorySlotUI arsenalSlot = SCR_ArsenalInventorySlotUI.Cast(m_pFocusedSlotUI);
			if (arsenalSlot)
			{
				m_SupplyCostUIInfo.SetSupplyCost(arsenalSlot.GetItemSupplyCost());
				hintsInfo.InsertAt(m_SupplyCostUIInfo, 0);
				arsenalCostSet = true;
				
				//~ Set arsenal rank icon
				if (m_ArsenalItemRankUIInfo)
				{
					SCR_ECharacterRank requiredItemRank = arsenalSlot.GetRequiredRank();					
					
					//~ Add required rank hint if higher then SCR_ECharacterRank.PRIVATE or player is Renegade
					if (requiredItemRank > SCR_ECharacterRank.PRIVATE || SCR_CharacterRankComponent.GetCharacterRank(SCR_PlayerController.GetLocalControlledEntity()) <= SCR_ECharacterRank.RENEGADE)
					{
						m_ArsenalItemRankUIInfo.SetRequiredRank(requiredItemRank);
						hintsInfo.InsertAt(m_ArsenalItemRankUIInfo, arsenalCostSet);
					}
				}
				
				//~ Set MSAR cost info
				if (m_PlayerSupplyAllocationUIInfo)
				{
					m_PlayerSupplyAllocationUIInfo.SetMSARCost(arsenalSlot.GetPersonalResourceCost());
					hintsInfo.Insert(m_PlayerSupplyAllocationUIInfo);
				}
				
			}
		}
		
		//~ Add refund cost
		if (!arsenalCostSet && m_SupplyRefundUIInfo && IsStorageArsenal(m_pStorageLootUI.GetCurrentNavigationStorage()))
		{
			bool isNonRefundable;
			bool nonrefundableItemInStorage = false;
			if (DoesSlotContainNonRefundableItems(m_pFocusedSlotUI, nonrefundableItemInStorage))
			{
				m_NonrefundableUIInfo.SetContainsNonrefundableItem(nonrefundableItemInStorage);
				hintsInfo.InsertAt(m_NonrefundableUIInfo, 0);
				isNonRefundable = true;
			}
			
			if (!isNonRefundable)
			{
				BaseInventoryStorageComponent storage = m_pStorageLootUI.GetCurrentNavigationStorage();
				if (storage)
				{
					SCR_ArsenalComponent arsenalComp = SCR_ArsenalComponent.Cast(storage.GetOwner().FindComponent(SCR_ArsenalComponent));
					if (arsenalComp)
					{
						bool isSupplyStorageAvailable;
						float supplyRefundAmount = SCR_ArsenalManagerComponent.GetItemRefundAmount(m_pFocusedSlotUI.GetInventoryItemComponent().GetOwner(), arsenalComp, true, isSupplyStorageAvailable: isSupplyStorageAvailable);
						
						if (supplyRefundAmount >= 0)
						{
							m_SupplyRefundUIInfo.SetSupplyRefund(supplyRefundAmount, isSupplyStorageAvailable);
							hintsInfo.InsertAt(m_SupplyRefundUIInfo, 0);
						}

						//~ Set MSAR cost info
						if (m_PlayerSupplyAllocationUIInfo)
						{
							InventoryItemComponent item = m_pFocusedSlotUI.GetInventoryItemComponent();
							if (item)
							{
								IEntity itemEntity = item.GetOwner();
								if (itemEntity)
								{
									SCR_ArsenalManagerComponent arsenalManager;
									if (SCR_ArsenalManagerComponent.GetArsenalManager(arsenalManager))
									{
										m_PlayerSupplyAllocationUIInfo.SetMSARCost(arsenalManager.GetItemMilitarySupplyAllocationRefundAmount(itemEntity, arsenalComp));
										hintsInfo.Insert(m_PlayerSupplyAllocationUIInfo);
									}
								}
							}
						}
					}
				}
			}
		}
		
		//~ Faction hint for items with a faction
		if (!m_mItemFactionUIInfos.IsEmpty())
		{
			InventoryItemComponent itemInventory = m_pFocusedSlotUI.GetInventoryItemComponent();
			if (itemInventory)
			{
				SCR_ItemOutfitFactionComponent outfitFaction = SCR_ItemOutfitFactionComponent.Cast(itemInventory.GetOwner().FindComponent(SCR_ItemOutfitFactionComponent));
				if (outfitFaction && outfitFaction.IsValid())
				{
					array<SCR_OutfitFactionData> outfitValues = {};
					outfitFaction.GetOutfitFactionDataArray(outfitValues);
					
					SCR_FactionOutfitItemHintUIInfo foundInfo;
					
					foreach (SCR_OutfitFactionData data : outfitValues)
					{
						if (!m_mItemFactionUIInfos.Find(data.GetAffiliatedFactionKey(), foundInfo))
							continue;
						
						foundInfo.SetFaction(data.GetAffiliatedFaction(), data);
						hintsInfo.Insert(foundInfo);
					}
				}
			}
		}
		
		//~ Character identity item valuable intel hint
		SCR_IdentityInventoryItemComponent identityInventoryItem = SCR_IdentityInventoryItemComponent.Cast(m_pFocusedSlotUI.GetInventoryItemComponent().GetOwner().FindComponent(SCR_IdentityInventoryItemComponent));
		if (identityInventoryItem && identityInventoryItem.HasValuableIntel(true))
		{
			m_ValuableIntelUIInfo.SetIntel(identityInventoryItem);
			hintsInfo.Insert(m_ValuableIntelUIInfo);
		}
	}

	//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
	//------------------------------------------------------------------------------------------------
	void ShowItemInfo( string sName = "", string sDescr = "", float sWeight = 0.0, SCR_InventoryUIInfo uiInfo = null )
	{
		if ( !m_pItemInfo )
		{
			//Widget parent = m_widget.FindAnyWidget( "SoldierInfo" );
			Widget infoWidget = GetGame().GetWorkspace().CreateWidgets(ITEM_INFO, m_widget);
			if ( !infoWidget )
				return;

			infoWidget.AddHandler( new SCR_InventoryItemInfoUI() );
			m_pItemInfo = SCR_InventoryItemInfoUI.Cast( infoWidget.FindHandler( SCR_InventoryItemInfoUI ) );
		}

		if( !m_pItemInfo )
			return;

		Widget w = m_pFocusedSlotUI.GetButtonWidget();

		m_pItemInfo.Show( 0.6, w, true );
		m_pItemInfo.SetName( sName );
		m_pItemInfo.SetDescription( sDescr );
		
		m_pItemInfo.SetWeight( sWeight );
		if (uiInfo && uiInfo.IsIconVisible())
			m_pItemInfo.SetIcon(uiInfo.GetIconPath(), uiInfo.GetIconColor());
		else
			m_pItemInfo.ShowIcon(false);
		
		array<SCR_InventoryItemHintUIInfo> hintsInfo = {};
		
		//~ Add hints
		if (uiInfo)
			uiInfo.GetItemHintArray(hintsInfo);
		
		//~ Add general hints that are not found on the item but are more generic depending where the item is (eg: arsenal) and the components on the item
		//~ These hints cover a wide arrange of items so that each item does not need to be set up individually
		GetGeneralItemHintsInfos(hintsInfo);

		//~ If has hints show them
		if (!hintsInfo.IsEmpty())
			m_pItemInfo.SetItemHints(m_pFocusedSlotUI.GetInventoryItemComponent(), hintsInfo, m_pFocusedSlotUI);
		else
			m_pItemInfo.SetItemHints(m_pFocusedSlotUI.GetInventoryItemComponent());
			
		//~ Create Identity UI
		m_pItemInfo.CreateIdentityUI(SCR_IdentityInventoryItemComponent.Cast(m_pFocusedSlotUI.GetInventoryItemComponent()));

		UpdateItemInfoPosition();
	}

	//------------------------------------------------------------------------------------------------
	//! Updates the position of the item info
	//! \param[in] postponed true if this method is executed with a delay, otherwise false
	protected void UpdateItemInfoPosition(bool postponed = false)
	{
		if (!m_pFocusedSlotUI)
			return;

		Widget w = m_pFocusedSlotUI.GetButtonWidget();
		if (!w)
			return;

		float x, y;
		w.GetScreenPos(x, y);

		float width, height;
		w.GetScreenSize(width, height);
		if (!postponed && x == 0 && y == 0 && width == 0 && height == 0)
		{
			HideItemInfo();
			// delay, because for some reason it is possible that UI will exist, but it won't be ready, as everything will be at 0 0 with 0 width and height ¯\_(-.-)_/¯
			GetGame().GetCallqueue().CallLater(UpdateItemInfoPosition, 100, param1: true);
			return;
		}

		m_pItemInfo.Show(0.2, w, true);

		// Needed to update the widget to the correct size before requesting its size here.
		m_pItemInfo.GetInfoWidget().Update();
		
		float screenSizeX, screenSizeY;
		GetGame().GetWorkspace().GetScreenSize(screenSizeX, screenSizeY);

		float infoWidth, infoHeight;
		m_pItemInfo.GetInfoWidget().GetScreenSize(infoWidth, infoHeight);

		int targetPosX = x;
		int targetPosY = y + height;

		float offsetX = (screenSizeX - infoWidth - targetPosX);
		if (offsetX < 0)
			targetPosX += offsetX;
		float offsetY = (screenSizeY - infoHeight - targetPosY);
		if (offsetY < 0)
			targetPosY += offsetY;

		m_pItemInfo.Move(
			GetGame().GetWorkspace().DPIUnscale(targetPosX), 
			GetGame().GetWorkspace().DPIUnscale(targetPosY));
	}
	
	//------------------------------------------------------------------------------------------------
	void ShowDamageInfo(string sName, SCR_InventoryDamageUIInfo damageInfo)
	{
		Widget w = WidgetManager.GetWidgetUnderCursor();
		if (!w && m_AttachmentSpinBox)
		{
			SCR_InventoryHitZonePointContainerUI currentHZContainer = SCR_InventoryHitZonePointContainerUI.Cast(m_AttachmentSpinBox.GetCurrentItemData());
			if (currentHZContainer)
				w = currentHZContainer.GetRootWidget().FindAnyWidget("HitZoneButton");
		}
		
		if (!w)
			return;
		
		if (!m_pDamageInfo)
 	 	{
		 	Widget infoWidget = GetGame().GetWorkspace().CreateWidgets(DAMAGE_INFO, m_widget);
			if ( !infoWidget )
				return;
		
			infoWidget.AddHandler( new SCR_InventoryDamageInfoUI() );
			m_pDamageInfo = SCR_InventoryDamageInfoUI.Cast( infoWidget.FindHandler( SCR_InventoryDamageInfoUI ) );
		}
	
		if (!m_pDamageInfo)
 	 		return;
		
		m_pDamageInfo.Show( 0.6, w, m_bIsUsingGamepad );
		m_pDamageInfo.SetName( sName );
		
		if (damageInfo)
		{
			m_pDamageInfo.SetDamageStateVisible(damageInfo.m_bDamageIconVisible, damageInfo.m_bDamageRegenerating, damageInfo.m_sDamageIntensity, damageInfo.m_sDamageText);
			m_pDamageInfo.SetBleedingStateVisible(damageInfo.m_bBleedingIconVisible, damageInfo.m_sBleedingText);
			m_pDamageInfo.SetTourniquetStateVisible(damageInfo.m_bTourniquetIconVisible);
			m_pDamageInfo.SetSalineBagStateVisible(damageInfo.m_bSalineBagIconVisible);
			m_pDamageInfo.SetFractureStateVisible(damageInfo.m_bFractureIconVisible, damageInfo.m_bFractureIcon2Visible);
		}

		int iMouseX, iMouseY;

		float x, y;
		w.GetScreenPos(x, y);

		float width, height;
		w.GetScreenSize(width, height);

		float screenSizeX, screenSizeY;
		GetGame().GetWorkspace().GetScreenSize(screenSizeX, screenSizeY);

		float infoWidth, infoHeight;
		m_pDamageInfo.GetInfoWidget().GetScreenSize(infoWidth, infoHeight);

		iMouseX = x;
		iMouseY = y + height;
		if (x + infoWidth > screenSizeX)
			iMouseX = screenSizeX - infoWidth - width * 0.5; // offset info if it would go outside of the screen

		m_pDamageInfo.Move( GetGame().GetWorkspace().DPIUnscale( iMouseX ), GetGame().GetWorkspace().DPIUnscale( iMouseY ) );
	}
	//---- REFACTOR NOTE END ----

	//------------------------------------------------------------------------------------------------
	void DestroyDamageInfo()
	{
		if (m_pDamageInfo)
			m_pDamageInfo.Destroy();
	}

	//------------------------------------------------------------------------------------------------
	void HideItemInfo()
	{
		if ( !m_pItemInfo )
			return;
		m_pItemInfo.Hide();
	}
	
	//------------------------------------------------------------------------------------------------
	void HideDamageInfo()
	{
		if ( !m_pDamageInfo )
			return;
		m_pDamageInfo.Hide();
	}

	//------------------------------------------------------------------------------------------------
	SCR_InventorySlotUI GetCurrentlyFocusedSlot()
	{
		return m_pFocusedSlotUI;
	}

	//------------------------------------------------------------------------------------------------
	void SetSlotFocused( SCR_InventorySlotUI pFocusedSlot, SCR_InventoryStorageBaseUI pFromStorageUI, bool bFocused )
	{
		if (m_pFocusedSlotUI && (m_pFocusedSlotUI != pFocusedSlot || !bFocused))
		{
			SCR_InventorySlotUI oldFocusedSlot = m_pFocusedSlotUI;
			m_pFocusedSlotUI = null;
			oldFocusedSlot.OnSlotFocusLost();
		}

		if( bFocused )
		{
			InputManager pInputManager = GetGame().GetInputManager();
			if ( !( pInputManager && pInputManager.IsUsingMouseAndKeyboard() ) )
			{
				if ( m_pActiveStorageUI != pFromStorageUI )
				{
					if ( m_pActiveStorageUI )
					{
						SCR_InventorySlotUI pLastFocusedSlot = pFromStorageUI.GetLastFocusedSlot();
						pFromStorageUI.SetSlotFocused(pLastFocusedSlot);
					}

					m_pActiveStorageUI = pFromStorageUI;
				}
			}

			if (m_DraggedSlot)
				pFocusedSlot.CheckCompatibility(m_DraggedSlot);

			m_pFocusedSlotUI = pFocusedSlot;
			if (m_OnItemHover)
				m_OnItemHover.Invoke(pFocusedSlot);
			
			SetFocusedSlotEffects();
		}
		else
		{
			if ( !m_bLocked )
			{
				if ( m_pActiveStorageUI )
					m_pActiveStorageUI.SetLastFocusedSlot( pFocusedSlot	 );

				HideItemInfo();
				m_pFocusedSlotUI = null;
				m_pNavigationBar.SetAllButtonEnabled(false);
			}

			if (m_OnItemHoverEnd)
				m_OnItemHoverEnd.Invoke();

			pFocusedSlot.CheckCompatibility(null);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SetFocusedSlotEffects()
	{
		if( !m_pFocusedSlotUI )
			return;

		//show info about the item
		InventoryItemComponent invItemComp = m_pFocusedSlotUI.GetInventoryItemComponent();
		if ( !invItemComp )
			return;
		auto attribs = SCR_ItemAttributeCollection.Cast( invItemComp.GetAttributes() );

		if ( !attribs )
			return;
		UIInfo itemInfo = attribs.GetUIInfo();
		if ( !itemInfo )
			HideItemInfo();
		else
		{
			SCR_InventoryUIInfo inventoryInfo = SCR_InventoryUIInfo.Cast(itemInfo);
			
			if (inventoryInfo)
				ShowItemInfo( inventoryInfo.GetInventoryItemName(invItemComp), inventoryInfo.GetInventoryItemDescription(invItemComp), invItemComp.GetTotalWeight(), inventoryInfo);
			else 
				ShowItemInfo( itemInfo.GetName(), itemInfo.GetDescription(), invItemComp.GetTotalWeight(), null);
		}

		NavigationBarUpdate();
	}

	//------------------------------------------------------------------------------------------------
	//!
	protected EDropContainer IsFocusedItemInsideDropContainer()
	{
		if ( !m_pActiveHoveredStorageUI )
			return EDropContainer.NOCONTAINER;
		if ( m_pActiveHoveredStorageUI.GetFocusedSlot() == m_pFocusedSlotUI )
			return EDropContainer.ISINSIDE;

		return EDropContainer.ISNOTINSIDE;
	}

	//------------------------------------------------------------------------------------------------
	protected bool CanSwapItems(SCR_InventorySlotUI slot1, SCR_InventorySlotUI slot2)
	{
		if (!slot1 || !slot2)
			return false;

		return m_InventoryManager.CanSwapItemStorages(
			slot1.GetInventoryItemComponent().GetOwner(),
			slot2.GetInventoryItemComponent().GetOwner()
		);
	}

	//------------------------------------------------------------------------------------------------
	protected bool CanMoveItem(SCR_InventorySlotUI slot1, SCR_InventorySlotUI slot2)
	{
		if (!slot1 || !slot2)
			return false;

		return m_InventoryManager.CanMoveItemToStorage(
			slot1.GetInventoryItemComponent().GetOwner(),
			slot2.GetStorageComponent()
		);
	}

	//------------------------------------------------------------------------------------------------
	protected bool CanMoveItem(SCR_InventorySlotUI slot1, SCR_InventoryStorageBaseUI slot2)
	{
		if (!slot1 || !slot2)
			return false;

		return m_InventoryManager.CanMoveItemToStorage(
			slot1.GetInventoryItemComponent().GetOwner(),
			slot2.GetStorage()
		);
	}

	//------------------------------------------------------------------------------------------------
	//! Checks if provided item is part of the player hierarchy
	//! \param[in] item
	//! \return true if item belongs to the player, otherwise false
	protected bool ItemBelongsToThePlayer(notnull IEntity item)
	{
		if (!m_Player)
			return false;

		IEntity parent = item.GetParent();
		while (parent)
		{
			if (parent == m_Player)
				return true;

			parent = parent.GetParent();
		}

		return false;
	}

	//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
	//------------------------------------------------------------------------------------------------
	//! Update the states of the navigation buttons used in the inventory
	void NavigationBarUpdate()
	{
		if (!m_pNavigationBar)
			return;

		if (m_bIsUsingGamepad)
		{
			NavigationBarUpdateGamepad();
			if (m_pActiveStorageUI == m_pAttachmentStorageUI)
				return;
		}
		else
		{
			m_pNavigationBar.SetAllButtonEnabled(false);
			m_pNavigationBar.SetButtonEnabled(BUTTON_BACK, true);
			m_pNavigationBar.SetButtonEnabled(BUTTON_SELECT, !m_DraggedSlot);
	
			SCR_InventoryHitZoneUI hzSlot = SCR_InventoryHitZoneUI.Cast(m_pActiveHoveredStorageUI);
			m_pNavigationBar.SetButtonEnabled(BUTTON_REMOVE_TOURNIQUET, (hzSlot && hzSlot.IsTourniquetted()));			
		}

		//if player is dragging a slotted item then we dont want to show hints for items that will happen to be under the cursor
		if (m_DraggedSlot)
			return;

		//if there is no slot under the cursor then there are no hints to show
		if (!m_pFocusedSlotUI)
			return;

		InventoryItemComponent itemComp = m_pFocusedSlotUI.GetInventoryItemComponent();
		if (!itemComp)
			return;

		IEntity item = itemComp.GetOwner();
		if (!item)
			return;

		bool isItemInCharacter;
		SCR_ArsenalInventorySlotUI arsenalSlot = SCR_ArsenalInventorySlotUI.Cast(m_pFocusedSlotUI);
		if (!arsenalSlot)
		{
			isItemInCharacter = ItemBelongsToThePlayer(item);
		}
		else if (!arsenalSlot.IsAvailable())
		{
			m_pNavigationBar.SetButtonEnabled(BUTTON_SELECT, false);
			return;
		}

		if (isItemInCharacter && !m_pSelectedSlotUI)
		{
			m_pNavigationBar.SetButtonEnabled(BUTTON_USE, m_StorageManager.CanUseItem_Inventory(item));

			m_pNavigationBar.SetButtonEnabled(BUTTON_EQUIP, !m_pItemToAssign && m_StorageManager.CanEquipItem_Inventory(item));
			m_pNavigationBar.SetButtonEnabled(BUTTON_INSPECT, CanInspectItemInInventory(item, itemComp));
		}

		BaseInventoryStorageComponent desiredStorage = m_pStorageLootUI.GetCurrentNavigationStorage();
		//is targeted item going to be transfered into the arsenal
		bool isTargetingArsenal = isItemInCharacter && IsStorageArsenal(desiredStorage);

		//if there is no specific storage open as main target, then check if any of the open containers is not an arsenal, as in such case it has the priority when item is transfered
 		if (isItemInCharacter && !isTargetingArsenal && !desiredStorage && m_aOpenedStoragesUI)
			isTargetingArsenal = GetOpenArsenalStorage();

		//check if currently focused item is not in a arsenal
		if (isTargetingArsenal && m_aOpenedStoragesUI.Contains(SCR_InventoryOpenedStorageUI.Cast(m_pActiveHoveredStorageUI)))
			isTargetingArsenal = false;

		if (isTargetingArsenal && DoesSlotContainNonRefundableItems(m_pFocusedSlotUI))
			isTargetingArsenal = false;

		bool allowItemTransfer = !m_bIsUsingGamepad || m_pActiveStorageUI != m_pQuickSlotStorage;

		if (!m_bIsUsingGamepad || !m_pSelectedSlotUI)
		{
			m_pNavigationBar.SetButtonEnabled(BUTTON_BUY, arsenalSlot != null);
			m_pNavigationBar.SetButtonEnabled(BUTTON_SELL, allowItemTransfer && isItemInCharacter && isTargetingArsenal);
			m_pNavigationBar.SetButtonEnabled(BUTTON_PICK_UP, !isItemInCharacter && !arsenalSlot);
			m_pNavigationBar.SetButtonEnabled(BUTTON_DROP, allowItemTransfer && m_pFocusedSlotUI.IsDraggable() && isItemInCharacter && !isTargetingArsenal);
		}
		
		m_pNavigationBar.SetButtonEnabled(BUTTON_STEP_BACK, true);
	
		HandleSlottedItemFunction();
	}

	//------------------------------------------------------------------------------------------------
	//! Update the states of the navigation buttons used in the inventory while gamepad is used
	void NavigationBarUpdateGamepad()
	{
		if (!m_pNavigationBar)
			return;

		m_pNavigationBar.SetAllButtonEnabled(false);
		m_pNavigationBar.SetButtonEnabled(BUTTON_BACK, true);
		m_pNavigationBar.SetButtonEnabled(BUTTON_SELECT, m_pFocusedSlotUI || m_pActiveStorageUI);

		SCR_InventoryHitZoneUI hzSlot = m_AttachmentSpinBox.GetFocusedHZPoint();
		m_pNavigationBar.SetButtonEnabled(BUTTON_REMOVE_TOURNIQUET, hzSlot && hzSlot.IsTourniquetted() && m_AttachmentSpinBox.IsFocused());		

		if (m_pActiveStorageUI && m_pActiveStorageUI == m_pAttachmentStorageUI)
		{
			m_pNavigationBar.SetButtonEnabled(BUTTON_USE, !m_pSelectedSlotUI);
			return;
		}
 
		InventoryItemComponent iic;
		if (m_pFocusedSlotUI)
			iic = m_pFocusedSlotUI.GetInventoryItemComponent();

		IEntity focusedItem;
		bool isItemInCharacter;
		if (iic)
		{
			focusedItem = iic.GetOwner();
			if(focusedItem)
				isItemInCharacter = ItemBelongsToThePlayer(focusedItem);
		}

		if (m_pFocusedSlotUI)
			m_pNavigationBar.SetButtonEnabled(BUTTON_SELECT, !m_pSelectedSlotUI && focusedItem && m_pFocusedSlotUI.IsDraggable());
		else
			m_pNavigationBar.SetButtonEnabled(BUTTON_SELECT, !m_pSelectedSlotUI && m_pActiveStorageUI);

		SCR_InventoryStorageBaseUI selectedStorage;
		if (m_pSelectedSlotUI)
			selectedStorage = m_pSelectedSlotUI.GetStorageUI();

		const bool hoveringOverQuickSlots = m_pActiveStorageUI == m_pQuickSlotStorage;
		const bool transferringSupplies = SCR_SupplyInventorySlotUI.Cast(m_pSelectedSlotUI) && m_pSelectedSlotUI != m_pFocusedSlotUI;
		const bool isTargetingDifferentSlot = m_pSelectedSlotUI != m_pFocusedSlotUI;
		bool isTargetingDifferentStorage = selectedStorage != m_pActiveStorageUI;
		if (!isTargetingDifferentStorage && selectedStorage && (selectedStorage == m_pWeaponStorage || selectedStorage == m_pQuickSlotStorage))
			isTargetingDifferentStorage = isTargetingDifferentSlot; // weapon slots and quick slots should be treated as separate storages, despite them being one

		if (m_bStorageSwitchMode)
		{
			if (m_pSelectedSlotUI)
				m_pNavigationBar.SetButtonActionName(BUTTON_BACK, GAMEPAD_HINT_DESELECT);
			else
				m_pNavigationBar.SetButtonActionName(BUTTON_BACK, GAMEPAD_HINT_CLOSE);

			bool shouldShowMove = m_pSelectedSlotUI && m_pSelectedSlotUI.IsDraggable() && isTargetingDifferentSlot;
			if (shouldShowMove && m_pActiveStorageUI)
				shouldShowMove &= m_pActiveStorageUI.IsStorageHighlighted() && (transferringSupplies || isTargetingDifferentStorage);

			m_pNavigationBar.SetButtonEnabled(BUTTON_MOVE, shouldShowMove);

			m_pNavigationBar.SetButtonEnabled(BUTTON_ENTER_STORAGE, m_pSelectedSlotUI != null && !hoveringOverQuickSlots && m_pActiveStorageUI && m_pActiveStorageUI.IsStorageHighlighted() && !m_pActiveStorageUI.IsArsenal());
		}
		else
		{
			m_pNavigationBar.SetButtonEnabled(BUTTON_SWAP, m_pSelectedSlotUI && isTargetingDifferentSlot && isTargetingDifferentStorage && SCR_ArsenalInventorySlotUI.Cast(m_pSelectedSlotUI) == null);
			m_pNavigationBar.SetButtonEnabled(BUTTON_MOVE, m_pSelectedSlotUI && m_pFocusedSlotUI && (transferringSupplies || isTargetingDifferentStorage));

			m_pNavigationBar.SetButtonActionName(BUTTON_BACK, GAMEPAD_HINT_BACK);
		}

		bool isWeaponQuickSlot = SCR_InventorySlotWeaponSlotsUI.Cast(m_pFocusedSlotUI);
		m_pNavigationBar.SetButtonEnabled(BUTTON_QUICK_ASSIGN, !isWeaponQuickSlot && (hoveringOverQuickSlots && m_pItemToAssign || isItemInCharacter && !hoveringOverQuickSlots && CanItemFitIntoQuickSlots(iic)));
		m_pNavigationBar.SetButtonEnabled(BUTTON_QUICK_UNASSIGN, hoveringOverQuickSlots && isItemInCharacter && !isWeaponQuickSlot && !m_pItemToAssign);
	}
	//---- REFACTOR NOTE END ----

	//------------------------------------------------------------------------------------------------
	//! Checks if given item can be used for entering inspection view in the inventory
	//! \param[in] item
	//! \param[in] iic
	//! \return true when provided item can be inspected in the inventory view
	protected bool CanInspectItemInInventory(notnull IEntity item, notnull InventoryItemComponent iic)
	{
		return item.FindComponent(SCR_WeaponAttachmentsStorageComponent);
	}

	//------------------------------------------------------------------------------------------------
	//! Searches through the list of open containers in order to find the first storage which is an arsenal
	//! \return storage component of the arsenal if found, otherwise null
	protected BaseInventoryStorageComponent GetOpenArsenalStorage()
	{
		if (!m_aOpenedStoragesUI)
			return null;

		BaseInventoryStorageComponent storage;
		//! The first opened storage that belong to arsenal is to be used.
		foreach (SCR_InventoryOpenedStorageUI storageUI: m_aOpenedStoragesUI)
		{
			if (!SCR_InventoryOpenedStorageArsenalUI.Cast(storageUI))
				continue;

			storage = storageUI.GetStorage();
			if (storage)
				return storage;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	void HandleSlottedItemFunction()
	{
		string sAction = GAMEPAD_HINT_SELECT;
		bool arsenalItem = IsStorageArsenal(m_pFocusedSlotUI.GetStorageUI().GetCurrentNavigationStorage()); // hotfix for disabling opening open action in arsenal storages

		switch ( m_pFocusedSlotUI.GetSlotedItemFunction() )
		{
			case ESlotFunction.TYPE_GADGET:
				// m_pNavigationBar.SetButtonEnabled( BUTTON_EQUIP, true );
				break;
			case ESlotFunction.TYPE_WEAPON:
				//m_pNavigationBar.SetButtonEnabled( BUTTON_EQUIP, true );

				InventoryItemComponent itemComp = m_pFocusedSlotUI.GetInventoryItemComponent();

				if (!itemComp)
				 	return;
				
				IEntity item = itemComp.GetOwner();

				if (item)
				{
					WeaponComponent weaponComp = WeaponComponent.Cast(item.FindComponent(WeaponComponent));

					if (weaponComp &&
						weaponComp.GetWeaponType() != EWeaponType.WT_FRAGGRENADE &&
						weaponComp.GetWeaponType() != EWeaponType.WT_SMOKEGRENADE &&
						weaponComp.Type() != SCR_MineWeaponComponent)
					{
						m_pNavigationBar.SetButtonEnabled(BUTTON_OPEN_STORAGE, !arsenalItem && !ItemBelongsToThePlayer(item));
						m_pNavigationBar.SetButtonEnabled(BUTTON_OPEN_NEW_CONTAINER, !arsenalItem);
					}
				}

				break;
			case ESlotFunction.TYPE_MAGAZINE:
				// TODO: show the Reload action
				//m_pNavigationBar.SetButtonEnabled( BUTTON_USE, true );
				break;
			case ESlotFunction.TYPE_CONSUMABLE:
				// TODO: show the Consume action
				m_pNavigationBar.SetButtonEnabled( BUTTON_USE, true );

				break;
			case ESlotFunction.TYPE_STORAGE:
				if( m_EStateMenuItem == EStateMenuItem.STATE_MOVING_ITEM_STARTED && m_pFocusedSlotUI != m_pSelectedSlotUI )
				{
					sAction = "#AR-Inventory_Move";
					//m_pNavigationBar.SetButtonEnabled( BUTTON_SELECT, false );
					//m_pNavigationBar.SetButtonEnabled( BUTTON_MOVE, true );
				}
				// Enable in case the storage is not "togglable" - can be only shown and only opening another storage will close it
				/*else if ( m_EStateMenuStorage == EStateMenuStorage.STATE_OPENED && m_pFocusedSlotUI == m_pSelectedSlotUI && m_pFocusedSlotUI.Type() != SCR_InventorySlotStorageEmbeddedUI)
				{
					m_pNavigationBar.SetButtonEnabled( BUTTON_SELECT, false );
				}*/
				else if ( m_pFocusedSlotUI.Type() == SCR_InventorySlotStorageEmbeddedUI || m_pFocusedSlotUI.Type() == SCR_SupplyInventorySlotUI)
				{
					m_pNavigationBar.SetButtonEnabled(BUTTON_OPEN_STORAGE, !arsenalItem);
					m_pNavigationBar.SetButtonEnabled(BUTTON_OPEN_NEW_CONTAINER, !arsenalItem);
				}
				
				break;

			case ESlotFunction.TYPE_HEALTH:
				// TODO: show the Heal action
				m_pNavigationBar.SetButtonEnabled( BUTTON_USE, true );
				
				break;
		}

		HandleSelectButtonState( sAction );
	}

	//------------------------------------------------------------------------------------------------
	protected void CreateItemSplitDialog(int maxVal, IEntity entityTo, IEntity entityFrom)
	{
		SCR_ItemSplitDialog dialog = SCR_ItemSplitDialog.Create(maxVal, entityTo, entityFrom);

		dialog.m_OnConfirm.Insert(OnItemSplitDialogConfirm);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnItemSplitDialogConfirm(SCR_ItemSplitDialog dialog)
	{		
		if (OnItemSplitDialogConfirm_Merge(dialog))
			return;
		
		if (OnItemSplitDialogConfirm_Create(dialog))
			return;
	}

	//------------------------------------------------------------------------------------------------
	protected bool OnItemSplitDialogConfirm_Merge(inout notnull SCR_ItemSplitDialog dialog)
	{
		SCR_ResourcePlayerControllerInventoryComponent resourceInventoryComp = SCR_ResourcePlayerControllerInventoryComponent.Cast(GetGame().GetPlayerController().FindComponent(SCR_ResourcePlayerControllerInventoryComponent));
		
		if (!resourceInventoryComp)
			return false;
		
		IEntity entityFrom = dialog.GetEntityFrom();
		IEntity entityTo = dialog.GetEntityTo();
		
		SCR_ResourceComponent resourceComponentFrom = SCR_ResourceComponent.FindResourceComponent(entityFrom);
		
		if (!resourceComponentFrom)
			return false;
		
		if (!entityTo)
			return false;
		
		SCR_ResourceComponent resourceComponentTo = SCR_ResourceComponent.FindResourceComponent(entityTo);
		
		if (!resourceComponentTo)
			return false;
		
		resourceInventoryComp.RpcAsk_MergeContainerWithContainerPartial(Replication.FindId(resourceComponentFrom), Replication.FindId(resourceComponentTo), EResourceType.SUPPLIES, dialog.GetSliderValue());
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool OnItemSplitDialogConfirm_Create(inout notnull SCR_ItemSplitDialog dialog)
	{
		SCR_ResourcePlayerControllerInventoryComponent resourceInventoryComp = SCR_ResourcePlayerControllerInventoryComponent.Cast(GetGame().GetPlayerController().FindComponent(SCR_ResourcePlayerControllerInventoryComponent));
		
		if (!resourceInventoryComp)
			return false;
		
		IEntity entityFrom = dialog.GetEntityFrom();
		IEntity entityTo = dialog.GetEntityTo();
		
		SCR_ResourceComponent resourceComponentFrom = SCR_ResourceComponent.FindResourceComponent(entityFrom);
		
		if (!resourceComponentFrom)
			return false;
		
		if (!entityTo)
		{
			resourceInventoryComp.RpcAsk_CreatePhysicalContainerWithContainer(Replication.FindId(resourceComponentFrom), Replication.FindId(null), Replication.FindId(null), EResourceType.SUPPLIES, dialog.GetSliderValue());
			
			return true;
		}
		
		SCR_ResourceComponent resourceComponentTo = SCR_ResourceComponent.FindResourceComponent(entityTo);
		
		//! Failsafe for not processing the same case needed in for merging containers.
		if (resourceComponentTo)
			return false;
		
		SCR_InventoryStorageManagerComponent invManagerTo;
		IEntity entityParentTo = entityTo.GetParent();
		
		if (entityParentTo)
			invManagerTo = SCR_InventoryStorageManagerComponent.Cast(entityParentTo.FindComponent(SCR_InventoryStorageManagerComponent));
		
		if (!invManagerTo)
			return false;
		
		BaseInventoryStorageComponent storageTo = BaseInventoryStorageComponent.Cast(entityTo.FindComponent(BaseInventoryStorageComponent));
		
		if (!storageTo)
			return false;
		
		resourceInventoryComp.RpcAsk_CreatePhysicalContainerWithContainer(Replication.FindId(resourceComponentFrom), Replication.FindId(invManagerTo), Replication.FindId(storageTo), EResourceType.SUPPLIES, dialog.GetSliderValue());
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void HandleSelectButtonState(string sAction = GAMEPAD_HINT_SELECT)
	{
		//TODO: this can be done better
		if (sAction == GAMEPAD_HINT_MOVE)
		{
			m_pNavigationBar.SetButtonActionName(BUTTON_SELECT, sAction);
		}
		else
		{
			if (!m_pFocusedSlotUI.IsSelected())
				m_pNavigationBar.SetButtonActionName(BUTTON_SELECT, sAction);
			else
				m_pNavigationBar.SetButtonActionName(BUTTON_SELECT, GAMEPAD_HINT_DESELECT);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Returns the inspection screen UI
	SCR_InventoryInspectionUI GetInspectionScreen()
	{
		return m_InspectionScreen;
	}

	//------------------------------------------------------------------------------------------------
	//! Opens the inspection screen for the item slot
	void InspectItem(SCR_InventorySlotUI itemSlot)
	{
		if (m_pGearInspectionPointUI)
		{	
			m_pGearInspectionPointUI.ClearSlots();
			m_wAttachmentContainer.RemoveHandler(m_pGearInspectionPointUI);
			m_pGearInspectionPointUI = null;
			m_pInspectedSlot = null;
		}

		if (!itemSlot)
		{
			SetAttachmentSpinBoxActive(false);
			CloseAttachmentStorage();
			InitializeAllHitZonesUI();
			m_PlayerRenderAttributes.RotateItemCamera(Vector(0, 0, 0), Vector(0, 0, 0), Vector(0, 0, 0)); // reset rotation
			UpdateCharacterPreview();
			return;
		}
		
		InventoryItemComponent itemComp = itemSlot.GetInventoryItemComponent();
		
		if (!itemComp)
		{
			InitializeCharacterHitZones();
			CloseAttachmentStorage();
			SetAttachmentSpinBoxActive(m_bIsUsingGamepad);
			UpdateCharacterPreview(); 
			return;
		}
		
		SCR_AnalyticsApplication.GetInstance().UseExaminationFromInventory();
		
		IEntity item = itemComp.GetOwner();
		
		SCR_WeaponAttachmentsStorageComponent weaponAttachmentStorage = SCR_WeaponAttachmentsStorageComponent.Cast(item.FindComponent(SCR_WeaponAttachmentsStorageComponent));
		
		if (weaponAttachmentStorage)
		{
			m_PlayerRenderAttributes.RotateItemCamera(Vector(0, 90, 0), Vector(0, 90, 0), Vector(0, 90, 0)); // rotate inspected weapon to side view
			m_pInspectedSlot = itemSlot;
			InspectWeapon(weaponAttachmentStorage);
			UpdateGearInspectionPreview();
			return;	
		}
		
		/*	Preparations for gear inspection
		ClothNodeStorageComponent clothNodeStorage = ClothNodeStorageComponent.Cast(item.FindComponent(ClothNodeStorageComponent));
		
		if (clothNodeStorage)
		{
			m_pInspectedSlot = itemSlot;
			InspectGear(clothNodeStorage);
			UpdateGearInspectionPreview();
			return;
		}*/
	}
	
	//------------------------------------------------------------------------------------------------
	void InspectWeapon(SCR_WeaponAttachmentsStorageComponent weaponAttachmentStorage)
	{
		DestroyDamageInfo();
		CloseAttachmentStorage();
		HideCharacterHitZones();
		SetAttachmentSpinBoxActive(m_bIsUsingGamepad);
		Widget inspectionFrame = m_widget.FindAnyWidget("AttachmentPoints");
		m_wAttachmentContainer = GetGame().GetWorkspace().CreateWidgets(BACKPACK_STORAGE_LAYOUT, inspectionFrame);
		m_pGearInspectionPointUI = new SCR_InventoryGearInspectionPointUI(weaponAttachmentStorage, null, this, frameSlot: inspectionFrame);
		m_wAttachmentContainer.AddHandler(m_pGearInspectionPointUI);
	}
	
	/*	Preparations for gear inspection
	void InspectGear(ClothNodeStorageComponent clothNodeStorage)
	{
		Widget inspectionFrame = m_widget.FindAnyWidget("AttachmentPoints");
		m_wAttachmentContainer = GetGame().GetWorkspace().CreateWidgets(BACKPACK_STORAGE_LAYOUT, inspectionFrame);
		m_pGearInspectionPointUI = new SCR_InventoryGearInspectionPointUI(clothNodeStorage, null, this, frameSlot: inspectionFrame);
		m_wAttachmentContainer.AddHandler(m_pGearInspectionPointUI);
	}*/
	
	SCR_InventoryGearInspectionPointUI GetGearInspectionUI()
	{
		return m_pGearInspectionPointUI;
	}

	//------------------------------------------------------------------------------------------------
	//! shows only the storages the item can be stored into
	protected void FilterOutStorages( bool bShow = true )
	{
		//Get all slots from the storage list UI
		array<SCR_InventorySlotUI> pSlotsInListUI = {};
		m_pStorageListUI.GetSlots( pSlotsInListUI );
		//foreach ( SCR_InventorySlotUI pSlotFromUI : pSlotsInListUI )
		//	RegisterUIStorage( pSlotFromUI );

		if ( m_pStorageLootUI )
			m_pStorageLootUI.GetSlots( pSlotsInListUI );
		if ( GetActualStorageInCharacterStorageUI() )
			GetActualStorageInCharacterStorageUI().GetSlots( pSlotsInListUI );
		if ( m_pQuickSlotStorage )
			m_pQuickSlotStorage.GetSlots( pSlotsInListUI );
		if ( m_pWeaponStorage )
			m_pWeaponStorage.GetSlots( pSlotsInListUI );

		BaseInventoryStorageComponent pStorageTo;
		foreach ( SCR_InventorySlotUI pStorageSlotUI : pSlotsInListUI )
		{
			if ( !pStorageSlotUI )
				continue;

			pStorageTo = pStorageSlotUI.GetAsStorage();

			if ( bShow )
			{
				if ( !m_pSelectedSlotUI )
					continue;
				if ( pStorageSlotUI.GetAsStorage() == m_pSelectedSlotUI.GetStorageUI().GetCurrentNavigationStorage() )	//it's the same storage as the selected item comes from
				{
					pStorageSlotUI.SetEnabledForMove( 0 );
					continue;
				}

				InventoryItemComponent pInventoryItem = m_pSelectedSlotUI.GetInventoryItemComponent();
				if ( !pInventoryItem )
					continue;

				IEntity pItem = pInventoryItem.GetOwner();

				if ( pStorageTo )
				{
					bool bCanStore = true;

					if ( m_InventoryManager.CanInsertItemInActualStorage( pItem, pStorageTo ) )
					{
						if ( pStorageTo.Type() == SCR_InventoryStorageWeaponsUI )
							if ( !m_pWeaponStorageComp.Contains( pStorageTo.GetOwner() ) )
								bCanStore = false;

						if ( bCanStore )
							pStorageSlotUI.SetEnabledForMove( 1 );
						else
							pStorageSlotUI.SetEnabledForMove( 0 );
					}
					else
					{
						pStorageSlotUI.SetEnabledForMove( 0 );
					}
				}
				else
				{
					pStorageSlotUI.SetEnabledForMove( 0 );
				}

			}
			else
			{
				pStorageSlotUI.SetEnabledForMove( 2 );
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void FilterOutItems( bool bFilterOut )
	{
		array<SCR_InventorySlotUI> aItemsUIInLoot = {};

		//if ( m_pStorageLootUI )
			//aItemsUIInLoot.InsertAll( m_pStorageLootUI.GetUISlots() );
		if ( m_pStorageListUI )
			aItemsUIInLoot.InsertAll( m_pStorageListUI.GetUISlots() );
		//if ( m_pStorageBaseUI )
			//aItemsUIInLoot.InsertAll( m_pStorageBaseUI.GetUISlots() );

		PrintFormat( "INV: Filtering out items %1", 1.5 );

		foreach ( SCR_InventorySlotUI pSlot : aItemsUIInLoot )
		{
			if ( !pSlot.GetStorageComponent() )
			{
				if ( bFilterOut )
				{
					PrintFormat( "INV: Disabling slot %1", pSlot );
					pSlot.SetEnabledForMove( 0 );
				}
				else
				{
					PrintFormat( "INV: Reseting slot %1", pSlot );
					pSlot.SetEnabledForMove( 2 );
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void Action_TryCloseInventory()
	{
		if (m_bWasJustTraversing)
		{
			m_bWasJustTraversing = false;
			return;
		}

		if (m_bIsUsingGamepad)
		{
			if (!m_bStorageSwitchMode)
			{
				SetStorageSwitchMode(true);
				return;
			}

			if (!m_pSelectedSlotUI)
			{
				Action_CloseInventory();
				return;
			}

			DeselectSlot();
			SetStorageSwitchMode(false);
			ResetHighlightsOnAvailableStorages();
			FocusOnSlotInStorage(m_pActiveStorageUI);
			return;
		}
		else
		{
			DeselectSlot();
		}

		Action_CloseInventory();
	}

	//------------------------------------------------------------------------------------------------
	protected void Action_CloseInventory()
	{
		array<BaseInventoryStorageComponent> traverseStorage = {};
		if (m_wLootStorage)
		{
			SCR_InventoryStorageBaseUI storageUIHandler = SCR_InventoryStorageBaseUI.Cast( m_wLootStorage.FindHandler( SCR_InventoryStorageBaseUI ) );
			storageUIHandler.GetTraverseStorage(traverseStorage);
		}

		if (!traverseStorage.IsEmpty())
		{
			BaseInventoryStorageComponent storage = traverseStorage[0];
			if (storage)
			{
				IEntity entity = storage.GetOwner();
				m_InventoryManager.PlayItemSound(entity, SCR_SoundEvent.SOUND_CONTAINER_CLOSE);
			}
		}
		else
		{
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INV_CLOSE);
		}
		
		GetGame().GetInputManager().RemoveActionListener("Inventory_Drag", EActionTrigger.DOWN, Action_DragDown);
		GetGame().GetInputManager().RemoveActionListener("Inventory", EActionTrigger.DOWN, Action_CloseInventory);

		if (m_pVicinity)
		{
			m_pVicinity.ManipulationComplete();
			m_iVicinityDiscoveryRadius = 0;
		}

		MenuManager menuManager = GetGame().GetMenuManager();
		ChimeraMenuPreset menu = ChimeraMenuPreset.Inventory20Menu;

		MenuBase inventoryMenu = menuManager.FindMenuByPreset(menu); // prototype inventory
		if (inventoryMenu)
			menuManager.CloseMenuByPreset(menu);
		
		if  (m_PlayerRenderAttributes)
			m_PlayerRenderAttributes.ResetDeltaRotation();

		if (m_Player)
		{
			m_CharController = SCR_CharacterControllerComponent.Cast(m_Player.GetCharacterController());
			if (m_CharController)
				m_CharController.m_OnLifeStateChanged.Remove(LifeStateChanged);
		}
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode)
			gameMode.GetOnControllableDeleted().Remove(OnControllableDeleted);
		
		if (m_pCharacterWidgetHelper)
			m_pCharacterWidgetHelper.Destroy();
		
		m_pCharacterWidgetHelper = null;

		HideItemInfo();
		HideDamageInfo();
		
		if ( m_pItemInfo )
			m_pItemInfo.Destroy();
		
		if ( m_pDamageInfo )
			m_pDamageInfo.Destroy();
		
		m_pItemInfo = null;
		m_pDamageInfo = null;

		if (m_InventoryManager)
			m_InventoryManager.OnInventoryMenuClosed();
	}
				
	//------------------------------------------------------------------------------------------------
	//! Method inserted on LifeStateChanged invoker on the character controller
	//! When LifeState changes, we may want to close the inventory
	void LifeStateChanged(ECharacterLifeState previousLifeState, ECharacterLifeState newLifeState)
	{
		if (newLifeState != ECharacterLifeState.ALIVE)
			Action_CloseInventory();
	}

	//------------------------------------------------------------------------------------------------
	//Only for cases when character is removed before dying
	protected void OnControllableDeleted(IEntity controllable)
	{
		if (m_Player && m_Player == controllable)
			Action_CloseInventory();
	}

	//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
	//------------------------------------------------------------------------------------------------
	//!
	protected void SimpleFSM(EMenuAction EAction = EMenuAction.ACTION_SELECT)
	{
		SCR_AnalyticsApplication.GetInstance().InteractWithItem(m_pFocusedSlotUI, m_pSelectedSlotUI, EAction, m_bIsUsingGamepad);
		
		switch (EAction)
		{
			case EMenuAction.ACTION_MOVEINSIDE:
			{
				Action_MoveItemToStorage(m_pActiveStorageUI);
				if (m_bIsUsingGamepad)
					SetStorageSwitchMode(true);
				NavigationBarUpdate();
				DeselectSlot();
				FocusOnSlotInStorage(m_pActiveStorageUI);
				ResetHighlightsOnAvailableStorages();
			} break;

			case EMenuAction.ACTION_SELECT:
			{
				if (!m_pFocusedSlotUI)
					return;

				if (m_pSelectedSlotUI)
					m_pSelectedSlotUI.SetSelected(false);

				if (!m_pFocusedSlotUI.IsDraggable())
					return;

				SCR_ArsenalInventorySlotUI arsenalSlot = SCR_ArsenalInventorySlotUI.Cast(m_pFocusedSlotUI);
				if (arsenalSlot && !arsenalSlot.IsAvailable())
					return;

				if (m_bIsUsingGamepad)
				{
					m_pSelectedSlotUI = m_pFocusedSlotUI;
					m_pSelectedSlotUI.SetSelected(true);
					HighlightAvailableStorages(m_pSelectedSlotUI);
				}

				NavigationBarUpdate();
				if (m_bIsUsingGamepad)
					SetStorageSwitchMode(true);
			} break;

			case EMenuAction.ACTION_DESELECT:
			{
				if (m_pSelectedSlotUI)
				{
					ResetHighlightsOnAvailableStorages();
					DeselectSlot()
				}

				NavigationBarUpdate();
			} break;

			case EMenuAction.ACTION_DRAGGED:
			{
				m_EStateMenuItem = EStateMenuItem.STATE_MOVING_ITEM_STARTED;
			} break;

			case EMenuAction.ACTION_DROPPED:
			{
				Action_Drop();
				if (m_pActiveHoveredStorageUI)
					m_pActiveHoveredStorageUI.ShowContainerBorder(false);
				m_EStateMenuItem = EStateMenuItem.STATE_IDLE;
				return;
			} break;

			case EMenuAction.ACTION_MOVEBETWEEN:
			{
				if (m_pFocusedSlotUI == m_pInspectedSlot)
				{
					InspectItem(null);
				}			
				
				if (m_pFocusedSlotUI)
				{
					m_pSelectedSlotUI = m_pFocusedSlotUI;
					SCR_InventoryStorageBaseUI pStorage = m_pFocusedSlotUI.GetStorageUI();

					if (SCR_InventoryOpenedStorageUI.Cast(pStorage) && m_aOpenedStoragesUI.Contains(SCR_InventoryOpenedStorageUI.Cast(pStorage)))
					{
						MoveBetweenFromVicinity();
						SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INV_CONTAINER_DIFR_DROP);
					}
					else if (pStorage != m_pStorageLootUI)
					{
						MoveBetweenToVicinity();
						SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INV_VICINITY_DROP_CLICK);
					}
					else
					{
						MoveBetweenFromVicinity();
						SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INV_PICKUP_CLICK);
					}
					m_pSelectedSlotUI = null;
				}
				ResetHighlightsOnAvailableStorages();
			} break;

			case EMenuAction.ACTION_UNFOLD:
			{
				if ( m_pFocusedSlotUI.GetStorageUI() == m_pStorageListUI ) //if it is slot in the "storage list ui"
				{
					if (m_pFocusedSlotUI.GetAsStorage())	// and if it is a storage
					{
						//ShowStorage( m_pFocusedSlotUI.GetAsStorage() ); 		//show the content of the actualy selected
						ToggleStorageContainerVisibility( m_pFocusedSlotUI );
						m_EStateMenuStorage = EStateMenuStorage.STATE_OPENED;
					}
					else
					{
						//CloseOpenedStorage();	// if it is not storage, show nothing
						//ToggleStorageContainerVisibility( m_pFocusedSlotUI.GetAsStorage() );
						m_EStateMenuStorage = EStateMenuStorage.STATE_IDLE;
					}
					
				}
				else
				{
					TraverseActualSlot();
					NavigationBarUpdate();
					m_EStateMenuStorage = EStateMenuStorage.STATE_OPENED;
				}
			} break;

			case EMenuAction.ACTION_OPENCONTAINER:
			{
				OpenAsNewContainer();
			} break;
		}

		if (!m_bIsUsingGamepad)
			m_pSelectedSlotUI = m_pFocusedSlotUI;
	}
	//---- REFACTOR NOTE END ----

	//------------------------------------------------------------------------------------------------
	protected void Action_OpenContainer()
	{
		SimpleFSM(EMenuAction.ACTION_OPENCONTAINER);
	}

	//------------------------------------------------------------------------------------------------
	//!
	protected void Action_Drop()
	{
		bool onDrop = false;
		if (m_pFocusedSlotUI)
			onDrop = m_pFocusedSlotUI.OnDrop(m_pSelectedSlotUI);

		Widget underCursor = WidgetManager.GetWidgetUnderCursor();
		if (underCursor && underCursor.GetName() == "HitZoneButton")
		{
			SCR_InventoryHitZonePointContainerUI hzContainer = SCR_InventoryHitZonePointContainerUI.Cast(m_AttachmentSpinBox.GetCurrentItemData());
			if (m_pSelectedSlotUI && hzContainer)
			{
				hzContainer.GetStorage().OnDrop(m_pSelectedSlotUI);
			}
		}
		
		if ( !onDrop && IsFocusedItemInsideDropContainer() != EDropContainer.NOCONTAINER )		//dropped to a container
		{
			if (m_pSelectedSlotUI == m_pInspectedSlot)
				InspectItem(null);
			
			if ( m_pFocusedSlotUI )
			{
				if ( IsFocusedItemInsideDropContainer() == EDropContainer.ISINSIDE )
				{
					if ( SCR_InventorySlotStorageUI.Cast( m_pFocusedSlotUI ) )	// storage
					{
						MoveItemToStorageSlot();
					}
					else if ( SCR_InventorySlotWeaponSlotsUI.Cast( m_pFocusedSlotUI ) )	// weapon slot
					{
						EquipWeaponIntoWeaponSlot();
					}
					else if (m_pFocusedSlotUI) // simple slot
					{
						if ( SCR_InventoryStorageQuickSlotsUI.Cast( m_pFocusedSlotUI.GetStorageUI() ) )	//quick slot
						{
							SetItemToQuickSlotDrop();
						}
						else
						{
							MoveItemToStorageSlot();
						}
					}
				}
				else
				{
					DeselectSlot();
					NavigationBarUpdate();
					SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INV_DROP_ERROR);
				}
			}
			else
			{
				// just container
				if (m_pActiveHoveredStorageUI)
					m_pActiveHoveredStorageUI.OnDrop(m_pSelectedSlotUI);
				if (!onDrop)
					MoveItem();
			}
		}
		else
		{
			if (WidgetManager.GetWidgetUnderCursor() == m_wPlayerRender)
			{
				EquipDraggedItem(true);
			}
			else
			{
				//dropped outside of a container
				RemoveItemFromQuickSlotDrop();
			}
		}
		
		ResetHighlightsOnAvailableStorages();
	}

	//------------------------------------------------------------------------------------------------
	void Action_QuickSlotAssign()
	{
		if (m_pItemToAssign && m_pActiveStorageUI == m_pQuickSlotStorage)
		{
			int slotId = m_pQuickSlotStorage.GetSlotId(m_pFocusedSlotUI) + 1;
			SetItemToQuickSlot(slotId, m_pItemToAssign);
			FocusOnSlotInStorage(m_pQuickSlotStorage, slotId - 1);
			m_pItemToAssign = null;
		}
		else
		{
			if (m_pActiveStorageUI != m_pStorageLootUI && m_pActiveStorageUI != m_pQuickSlotStorage)
			{
				m_pItemToAssign = m_pFocusedSlotUI;
				FocusOnSlotInStorage(m_pQuickSlotStorage, 4);
			}
		}
		
		NavigationBarUpdate();
	}

	//------------------------------------------------------------------------------------------------
	void Action_QuickSlotUnassign()
	{
		if (!m_pFocusedSlotUI)
			return;

		InventoryItemComponent itemComp = m_pFocusedSlotUI.GetInventoryItemComponent();
		if (!itemComp)
			return;

		IEntity item = itemComp.GetOwner();
		if (!item)
			return;

		int slotId = m_pQuickSlotStorage.GetFocusedSlotId();
		slotId = Math.Clamp(slotId, 0, m_StorageManager.GetQuickSlotItems().Count());
		m_pItemToAssign = null;
		m_StorageManager.RemoveItemFromQuickSlot(item);
		ShowQuickSlotStorage();
		FocusOnSlotInStorage(m_pQuickSlotStorage, slotId);
		NavigationBarUpdate();
		HideItemInfo();
	}

	//------------------------------------------------------------------------------------------------
	//!
	void MoveItemToStorageSlot()
	{
		bool operationFailed;
		if (MoveItemToStorageSlot_VirtualArsenal(operationFailed))
		{
			if (operationFailed)
			{
				DeselectSlot();
				NavigationBarUpdate();
				SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INV_DROP_ERROR);
			}

			return;
		}
		
		if (MoveItemToStorageSlot_ResourceContainer())
		{
			DeselectSlot();
			return;
		}

		if (!m_pSelectedSlotUI)
			return;
		
		InventoryItemComponent pComp = m_pSelectedSlotUI.GetInventoryItemComponent();
		if (!pComp)
			return;

		IEntity pItem = pComp.GetOwner();
		if (!m_InventoryManager.CanMoveItem(pItem) || !m_pFocusedSlotUI.IsCompatible())
			return;
		
		m_pCallBack.m_pItem = pItem;
		m_pCallBack.m_pMenu = this;
		m_pCallBack.m_pStorageFrom = m_pSelectedSlotUI.GetStorageUI();

		DeselectSlot();
		SCR_InventoryOpenedStorageUI open = GetOpenedStorage(m_pFocusedSlotUI.GetAsStorage());
		if (open)
			m_pCallBack.m_pStorageTo = open;
		else
			m_pCallBack.m_pStorageTo = m_pFocusedSlotUI.GetStorageUI();

		m_pCallBack.m_pStorageToFocus = m_pCallBack.m_pStorageTo;

		BaseInventoryStorageComponent pStorageFromComponent = m_pCallBack.m_pStorageFrom.GetCurrentNavigationStorage();
		BaseInventoryStorageComponent pStorageToComponent = m_pFocusedSlotUI.GetAsStorage();
		
		if (!pStorageToComponent)
			pStorageToComponent = m_pCallBack.m_pStorageTo.GetStorage();

		if (!pStorageToComponent || pStorageToComponent == pStorageFromComponent)
			return;

		if (IsStorageArsenal(pStorageToComponent))
		{
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INV_DROP_ERROR);
			return;
		}
		
		bool shouldEquip = m_pCallBack.m_pStorageTo == m_pStorageListUI;
		bool equip = shouldEquip && m_InventoryManager.EquipAny(m_StorageManager , pItem, 0, m_pCallBack);
		
		if (!equip)
			m_InventoryManager.InsertItem( pItem, pStorageToComponent, pStorageFromComponent, m_pCallBack );
		else
			m_InventoryManager.PlayItemSound(pItem, SCR_SoundEvent.SOUND_EQUIP);
		/*
		if ( pItem.FindComponent( SCR_GadgetComponent ) )
			m_InventoryManager.EquipGadget( pItem, m_pCallBack );
		else
		*/
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void EquipWeaponIntoWeaponSlot()
	{
		if (EquipWeaponIntoWeaponSlot_VirtualArsenal())
			return;

		if (!m_pSelectedSlotUI)
			return;
		
		InventoryItemComponent pComp = m_pSelectedSlotUI.GetInventoryItemComponent();
		if (!pComp)
			return;

		SCR_InventorySlotWeaponSlotsUI weaponSlot = SCR_InventorySlotWeaponSlotsUI.Cast(m_pFocusedSlotUI);
		if (!weaponSlot)
			return;
		
		IEntity pItem = pComp.GetOwner();
		if (!m_InventoryManager.CanMoveItem(pItem) || !m_pFocusedSlotUI.IsCompatible())
			return;
		
		m_pCallBack.m_pItem = pItem;
		m_pCallBack.m_pMenu = this;
		m_pCallBack.m_pStorageFrom = m_pSelectedSlotUI.GetStorageUI();
		m_pCallBack.m_pStorageTo = m_pFocusedSlotUI.GetStorageUI();
		m_pCallBack.m_pStorageToFocus = m_pCallBack.m_pStorageTo;
		m_pCallBack.m_iSlotToFocus = m_pCallBack.m_pStorageTo.GetSlotId(m_pFocusedSlotUI);
		
		InventoryItemComponent pItemToReplaceComp = m_pFocusedSlotUI.GetInventoryItemComponent();
		IEntity pItemToReplace;
		if (pItemToReplaceComp)
			pItemToReplace = pItemToReplaceComp.GetOwner();
		
		if (pItemToReplace)
		{
			ItemAttributeCollection itemAttributes = pComp.GetAttributes();
			if (!itemAttributes)
				return;
			
			AttachmentAttributes itemAttribute = AttachmentAttributes.Cast(itemAttributes.FindAttribute(AttachmentAttributes));
			
			// Check if inserted item is an attachment, if yes, try to add it into the weapon
			if (itemAttribute && itemAttribute.GetAttachmentType())
			{
				BaseInventoryStorageComponent pItemToReplaceStorage = BaseInventoryStorageComponent.Cast(pItemToReplace.FindComponent(BaseInventoryStorageComponent));
				
				if (!pItemToReplaceStorage)
					return;
				
				// Need to check for both since you can't insert from inside your own inventory
				if (m_InventoryManager.TryInsertItemInStorage(pItem, pItemToReplaceStorage, -1, m_pCallBack) 
					|| m_InventoryManager.TryMoveItemToStorage(pItem, pItemToReplaceStorage, -1, m_pCallBack))
				{
					m_InventoryManager.PlayItemSound(pItem, SCR_SoundEvent.SOUND_EQUIP);
				}
				else
				{
					SCR_UISoundEntity.SoundEvent("SOUND_INV_DROP_ERROR");
				}
				
				return;
			}
			
			BaseInventoryStorageComponent itemToReplaceStorage = BaseInventoryStorageComponent.Cast(pItemToReplace.FindComponent(BaseInventoryStorageComponent));
		
			if (itemToReplaceStorage && m_InventoryManager.CanInsertItemInStorage(pItem, itemToReplaceStorage, -1))
			{
				if (m_InventoryManager.TryInsertItemInStorage(pItem, itemToReplaceStorage, -1, m_pCallBack))
					m_InventoryManager.PlayItemSound(pItem, SCR_SoundEvent.SOUND_EQUIP);
				else
					SCR_UISoundEntity.SoundEvent("SOUND_INV_DROP_ERROR");
				
				return;
			}
		}
		
		if (pItemToReplace && pItem && pItemToReplace != pItem)
		{
			if (m_pSelectedSlotUI.GetSlotedItemFunction() == ESlotFunction.TYPE_MAGAZINE)
			{
				m_CharController.ReloadWeaponWith(pItem);
				return;
			}

			if (m_InventoryManager.TrySwapItemStorages(pItem, pItemToReplace, m_pCallBack))
				m_InventoryManager.PlayItemSound(pItem, SCR_SoundEvent.SOUND_EQUIP);
			else
				SCR_UISoundEntity.SoundEvent("SOUND_INV_DROP_ERROR");
			
			return;
		}
		
		DeselectSlot();
		BaseInventoryStorageComponent pStorageToComponent = m_pWeaponStorageComp;
		
		if (!m_InventoryManager.TryMoveItemToStorage( pItem, m_pWeaponStorageComp, weaponSlot.GetWeaponSlotIndex(), m_pCallBack ))
		{
			if (!m_InventoryManager.TryInsertItemInStorage(pItem, m_pWeaponStorageComp, weaponSlot.GetWeaponSlotIndex(), m_pCallBack))
				SCR_UISoundEntity.SoundEvent("SOUND_INV_DROP_ERROR");
				return;
		}
		
		m_InventoryManager.PlayItemSound(pItem, SCR_SoundEvent.SOUND_EQUIP);
	}

	//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
	//------------------------------------------------------------------------------------------------
	//!
	void MoveItem( SCR_InventoryStorageBaseUI pStorageBaseUI = null )
	{
		bool operationFailed;
		if (MoveItem_VirtualArsenal(pStorageBaseUI, operationFailed))
		{
			DeselectSlot();
			NavigationBarUpdate();
			if (operationFailed)
				SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INV_DROP_ERROR);

			return;
		}
		
		if (MoveItem_ResourceContainer(pStorageBaseUI))
		{
			DeselectSlot();
			return;
		}
		
		if ( !m_pSelectedSlotUI )
			return;
		InventoryItemComponent pComp = m_pSelectedSlotUI.GetInventoryItemComponent();
		if ( !pComp )
			return;
		IEntity pItem = pComp.GetOwner();
		if (!m_InventoryManager.CanMoveItem(pItem))
			return;
		
		m_pCallBack.m_pItem = pItem;
		m_pCallBack.m_pMenu = this;
		m_pCallBack.m_pStorageFrom = m_pSelectedSlotUI.GetStorageUI();
		m_pCallBack.m_sItemToFocus = m_pSelectedSlotUI.GetItemResource();

		if ( pStorageBaseUI )
			m_pCallBack.m_pStorageTo = pStorageBaseUI;
		else
			m_pCallBack.m_pStorageTo = m_pActiveHoveredStorageUI;

		m_pCallBack.m_pStorageToFocus = m_pCallBack.m_pStorageTo;
		if (m_pCallBack.m_pStorageTo)
			m_pCallBack.m_iSlotToFocus = m_pCallBack.m_pStorageTo.GetFocusedSlotId();

		BaseInventoryStorageComponent pStorageTo = m_pActiveHoveredStorageUI.GetCurrentNavigationStorage();
		if (IsStorageArsenal(pStorageTo))
		{
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INV_DROP_ERROR);
			return;
		}
		
		if (pStorageTo && pStorageTo.IsInherited(SCR_WeaponAttachmentsStorageComponent))
		{
			m_pCallBack.m_pStorageMan = GetInventoryStorageManager();
			m_pCallBack.m_eAttachAction = EAttachAction.ATTACH;
		}

		if (m_pCallBack.m_pStorageFrom)
		{
			BaseInventoryStorageComponent storageFrom = m_pCallBack.m_pStorageFrom.GetStorage();
			if (storageFrom && storageFrom.IsInherited(SCR_WeaponAttachmentsStorageComponent))
			{
				m_pCallBack.m_pStorageMan = GetInventoryStorageManager();
				m_pCallBack.m_eAttachAction = EAttachAction.DETACH;
			}
		}

		if ( pStorageTo && EquipedWeaponStorageComponent.Cast( pStorageTo ) )
		{
			m_InventoryManager.EquipWeapon( pItem, m_pCallBack, m_pCallBack.m_pStorageFrom == m_pStorageLootUI );
			DeselectSlot();
			return;
		}
		else if ( pStorageTo && CharacterInventoryStorageComponent.Cast( pStorageTo ) )
		{
			m_InventoryManager.EquipAny( pStorageTo, pItem, 0, m_pCallBack );
		}
		else if ( m_pCallBack.m_pStorageTo == m_pStorageLootUI )
		{
			MoveToVicinity( pItem );
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INV_VICINITY_DROP_CLICK);
		}
		else if ( m_pCallBack.m_pStorageFrom == m_pCallBack.m_pStorageTo)
		{
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INV_CONTAINER_SAME_DROP);
		}
		else
		{
			if ( m_pCallBack.m_pStorageFrom == m_pStorageLootUI )
				MoveFromVicinity();
			else
			{
				// disallow reload for unequipped weapons
				if (pItem.FindComponent(BaseMagazineComponent))
				{
					if (m_pActiveHoveredStorageUI.GetCurrentNavigationStorage().IsInherited(SCR_WeaponAttachmentsStorageComponent))
					{
						if (!IsWeaponEquipped(m_pActiveHoveredStorageUI.GetStorage().GetOwner()))
							return;
					}
					else if (m_pCallBack.m_pStorageFrom.GetStorage().IsInherited(SCR_WeaponAttachmentsStorageComponent))
					{
						if (!IsWeaponEquipped(m_pCallBack.m_pStorageFrom.GetStorage().GetOwner()))
							return;
					}
				}

				m_InventoryManager.InsertItem( pItem, m_pActiveHoveredStorageUI.GetCurrentNavigationStorage(), m_pCallBack.m_pStorageFrom.GetStorage(), m_pCallBack );
				DeselectSlot();
			}
		}
	}
	//---- REFACTOR NOTE END ----
	
	//------------------------------------------------------------------------------------------------
	//!
	protected void EquipWeapon()
	{
		if ( !m_pSelectedSlotUI )
			return;
		IEntity pItem = m_pSelectedSlotUI.GetInventoryItemComponent().GetOwner();
		if ( !pItem )
			return;
		
		if ( m_InventoryManager )
			m_InventoryManager.EquipWeapon( pItem, m_pCallBack );
	}
	
	//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
	//------------------------------------------------------------------------------------------------
	//!
	protected void MoveToVicinity( IEntity pItem )
	{
		BaseInventoryStorageComponent pStorageFrom = m_pSelectedSlotUI.GetStorageUI().GetStorage();
			
		if (!m_InventoryManager.CanMoveItem(pItem))
			return;
		
		if (pStorageFrom && pItem.FindComponent(BaseMagazineComponent) && pStorageFrom.IsInherited(SCR_WeaponAttachmentsStorageComponent))
		{
			if (!IsWeaponEquipped(pStorageFrom.GetOwner()))
				return;
		}

		m_pCallBack.m_pStorageFrom = GetStorageUIByBaseStorageComponent(pStorageFrom);
		if (!m_pCallBack.m_pStorageFrom)
			m_pCallBack.m_pStorageFrom = m_pSelectedSlotUI.GetStorageUI();

		BaseInventoryStorageComponent isStorage = BaseInventoryStorageComponent.Cast(pItem.FindComponent(BaseInventoryStorageComponent));
		if (isStorage)
			m_pCallBack.m_pStorageToDrop = isStorage;

		m_pCallBack.m_pStorageTo = m_pStorageLootUI;
		auto storage = m_pStorageLootUI.GetCurrentNavigationStorage();

		if ( storage && !IsStorageArsenal(storage) )
			m_InventoryManager.InsertItem( pItem, m_pStorageLootUI.GetCurrentNavigationStorage(), pStorageFrom, m_pCallBack );	// moving into the opened storage in the  vicinity
		else
		{
			//droping it on the ground
			auto pSlot = m_pSelectedSlotUI.GetInventoryItemComponent().GetParentSlot();
			if ( pSlot )
			{
				if (pStorageFrom && pStorageFrom.IsInherited(SCR_WeaponAttachmentsStorageComponent))
				{
					m_pCallBack.m_pStorageMan = GetInventoryStorageManager();
					m_pCallBack.m_eAttachAction = EAttachAction.DETACH;
				}
				m_InventoryManager.TryRemoveItemFromInventory(pItem, pSlot.GetStorage(), m_pCallBack);
				m_InventoryManager.PlayItemSound(pItem, SCR_SoundEvent.SOUND_DROP);
			}
			else
			{
				SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INV_DROP_ERROR);
			}
		}	
	}
		
	//------------------------------------------------------------------------------------------------
	//!
	protected void MoveFromVicinity()
	{
		IEntity pItem = m_pSelectedSlotUI.GetInventoryItemComponent().GetOwner();
		if (!m_InventoryManager.CanMoveItem(pItem))
			return;

		m_pCallBack.m_pItem = pItem;
		m_pCallBack.m_pMenu = this;

		m_pCallBack.m_pStorageFrom = m_pStorageLootUI;
		m_pCallBack.m_pStorageTo = m_pActiveHoveredStorageUI;

		if (ShouldSpawnItem())
		{
			m_pCallBack.m_bUpdateSlotOnly = true;
			BaseInventoryStorageComponent storageTo = m_InventoryManager.FindStorageForInsert(pItem, m_pActiveHoveredStorageUI.GetCurrentNavigationStorage(), EStoragePurpose.PURPOSE_ANY);
			m_InventoryManager.TrySpawnPrefabToStorage(pItem.GetPrefabData().GetPrefabName(), storageTo, -1, EStoragePurpose.PURPOSE_ANY, m_pCallBack);
			m_InventoryManager.PlayItemSound(pItem, SCR_SoundEvent.SOUND_PICK_UP);
			return;
		}

		if (m_pActiveHoveredStorageUI.GetCurrentNavigationStorage() &&
			m_pActiveHoveredStorageUI.GetCurrentNavigationStorage().IsInherited(SCR_WeaponAttachmentsStorageComponent))
		{
			if (pItem.FindComponent(BaseMagazineComponent))
			{
				if (!IsWeaponEquipped(m_pActiveHoveredStorageUI.GetCurrentNavigationStorage().GetOwner()))
					return;
			}
			m_pCallBack.m_pStorageMan = GetInventoryStorageManager();
			m_pCallBack.m_eAttachAction = EAttachAction.ATTACH;
		}

		BaseInventoryStorageComponent storagefrom = m_pStorageLootUI.GetCurrentNavigationStorage();
		if (!storagefrom)
		{
			IEntity ent = m_pVicinity.GetItemOfInterest();
			if (ent)
				storagefrom = BaseInventoryStorageComponent.Cast(ent.FindComponent(BaseInventoryStorageComponent));
		}
		
		m_InventoryManager.InsertItem(pItem, m_pActiveHoveredStorageUI.GetCurrentNavigationStorage(), storagefrom, m_pCallBack);	//a storage is already opened, try to move it there
		m_InventoryManager.PlayItemSound(pItem, SCR_SoundEvent.SOUND_PICK_UP);
	}

	//------------------------------------------------------------------------------------------------
	//!
	protected void MoveBetweenFromVicinity()
	{
		if (MoveBetweenFromVicinity_VirtualArsenal())
			return;
			
		IEntity pItem = m_pSelectedSlotUI.GetInventoryItemComponent().GetOwner();
		if (!m_InventoryManager.CanMoveItem(pItem))
			return;
		
		//~ Never send items to arsenal
		BaseInventoryStorageComponent storageTo = m_InventoryManager.FindStorageForItem(pItem, EStoragePurpose.PURPOSE_ANY);
		if (storageTo && storageTo.GetOwner().FindComponent(SCR_ArsenalComponent))
			return;

		m_pCallBack.m_pItem = pItem;
		m_pCallBack.m_pMenu = this;

		InventoryStorageSlot parentSlotFrom = m_pSelectedSlotUI.GetInventoryItemComponent().GetParentSlot();
		BaseInventoryStorageComponent storageFrom;
		if (parentSlotFrom)
			storageFrom = parentSlotFrom.GetStorage();
		
		if (!storageFrom)
			storageFrom = m_pStorageLootUI.GetCurrentNavigationStorage();
		
		m_pCallBack.m_pStorageFrom = GetStorageUIByBaseStorageComponent(storageFrom);
		m_pCallBack.m_pStorageTo = m_pStorageListUI;
		m_pCallBack.m_pStorageToFocus = m_pSelectedSlotUI.GetStorageUI();
		m_pCallBack.m_sItemToFocus = m_pSelectedSlotUI.GetItemResource();
		m_pCallBack.m_iSlotToFocus = m_pFocusedSlotUI.GetStorageUI().GetFocusedSlotId();
		m_pCallBack.m_bUpdateSlotOnly = m_pSelectedSlotUI.IsStacked();

		BaseInventoryStorageComponent pStorageTo = m_StorageManager;
		if ( pItem.FindComponent( SCR_GadgetComponent ) )
		{
			BaseInventoryStorageComponent pStorageComp = m_InventoryManager.FindStorageForItem( pItem, EStoragePurpose.PURPOSE_EQUIPMENT_ATTACHMENT );
			if ( pStorageComp )
				pStorageTo = pStorageComp;
			else
			{
				pStorageComp = m_InventoryManager.FindStorageForItem( pItem, EStoragePurpose.PURPOSE_DEPOSIT );
				if ( pStorageComp )
					pStorageTo = pStorageComp;
			}
		}

		if (ShouldSpawnItem())
		{
			m_pCallBack.m_bUpdateSlotOnly = true;
			m_InventoryManager.TrySpawnPrefabToStorage(pItem.GetPrefabData().GetPrefabName(), null, -1, EStoragePurpose.PURPOSE_ANY, m_pCallBack);
			m_InventoryManager.PlayItemSound(pItem, SCR_SoundEvent.SOUND_PICK_UP);
			return;
		}

		BaseInventoryStorageComponent isStorage = BaseInventoryStorageComponent.Cast(pItem.FindComponent(BaseInventoryStorageComponent));
		if (isStorage)
			m_pCallBack.m_pStorageToPickUp = isStorage;

		m_InventoryManager.InsertItem( pItem, null, m_pStorageLootUI.GetCurrentNavigationStorage(), m_pCallBack );
	}
	//---- REFACTOR NOTE END ----

	//------------------------------------------------------------------------------------------------
	protected bool ShouldSpawnItem()
	{
		if (!m_pSelectedSlotUI)
			return false;
		if (!m_pSelectedSlotUI.GetStorageUI())
			return false;

		BaseInventoryStorageComponent currStorage = m_pSelectedSlotUI.GetStorageUI().GetCurrentNavigationStorage();
		return IsStorageArsenal(currStorage);
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsStorageArsenal(BaseInventoryStorageComponent storage)
	{
		if (!storage)
			return false;
		return (storage.GetOwner().FindComponent(SCR_ArsenalComponent) != null);
	}

	//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
	//------------------------------------------------------------------------------------------------
	protected void EquipDraggedItem(bool forceEquip = false)
	{
		if (EquipDraggedItem_VirtualArsenal(forceEquip))
			return;

		if (!m_pSelectedSlotUI || !m_pSelectedSlotUI.GetInventoryItemComponent())
			return;

		IEntity pItem = m_pSelectedSlotUI.GetInventoryItemComponent().GetOwner();
		if (!pItem)
			return;

		m_pCallBack.m_pItem = pItem;
		m_pCallBack.m_pMenu = this;
		m_pCallBack.m_pStorageFrom = m_pSelectedSlotUI.GetStorageUI();

		m_pCallBack.m_pStorageToFocus = m_pSelectedSlotUI.GetStorageUI();
		m_pCallBack.m_sItemToFocus = m_pSelectedSlotUI.GetItemResource();
		m_pCallBack.m_iSlotToFocus = m_pSelectedSlotUI.GetStorageUI().GetSlotId(m_pSelectedSlotUI);
		m_pCallBack.m_bShouldEquip = forceEquip;

		BaseInventoryStorageComponent pStorageTo = m_StorageManager;
		if (m_pSelectedSlotUI.GetStorageUI().Type() == SCR_InventoryStorageWeaponsUI)
		{
			m_pCallBack.m_pStorageTo = m_pWeaponStorage;
		}
		else
		{
			m_pCallBack.m_pStorageTo = m_pStorageListUI;
		}

		if (pItem.FindComponent(SCR_GadgetComponent))
		{
			BaseInventoryStorageComponent pStorageComp = m_InventoryManager.FindStorageForItem( pItem, EStoragePurpose.PURPOSE_EQUIPMENT_ATTACHMENT );
			if (pStorageComp)
				pStorageTo = pStorageComp;
			else
			{
				pStorageComp = m_InventoryManager.FindStorageForItem(pItem, EStoragePurpose.PURPOSE_DEPOSIT);
				if (pStorageComp)
					pStorageTo = pStorageComp;
			}
		}

		if (pItem.FindComponent(WeaponComponent))
		{
			BaseInventoryStorageComponent pStorageComp = m_InventoryManager.FindStorageForItem(pItem, EStoragePurpose.PURPOSE_WEAPON_PROXY);
			if (pStorageComp)
				pStorageTo = pStorageComp;
			else
			{
				pStorageComp = m_InventoryManager.FindStorageForItem(pItem, EStoragePurpose.PURPOSE_DEPOSIT);
				if (pStorageComp)
					pStorageTo = pStorageComp;
			}
		}

		bool equip = m_InventoryManager.EquipAny(pStorageTo , pItem, 0, m_pCallBack);
		if (!equip && m_pSelectedSlotUI)
		{
			m_InventoryManager.InsertItem(pItem, null, m_pSelectedSlotUI.GetStorageUI().GetCurrentNavigationStorage(), m_pCallBack);
		}
		else
		{
			m_InventoryManager.PlayItemSound(pItem, SCR_SoundEvent.SOUND_EQUIP);
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	protected void MoveBetweenToVicinity()
	{
		InventoryItemComponent pInvComponent = m_pFocusedSlotUI.GetInventoryItemComponent();
		if(!pInvComponent)
			return;
		
		IEntity pItem = pInvComponent.GetOwner();
		if (!m_InventoryManager.CanMoveItem(pItem))
			return;

		m_pCallBack.m_pItem = pItem;
		m_pCallBack.m_pMenu = this;

		m_pCallBack.m_pStorageToFocus = GetStorageUIByBaseStorageComponent(m_pSelectedSlotUI.GetStorageUI().GetCurrentNavigationStorage());
		m_pCallBack.m_sItemToFocus = m_pSelectedSlotUI.GetItemResource();
		m_pCallBack.m_iSlotToFocus = m_pFocusedSlotUI.GetStorageUI().GetFocusedSlotId();
		m_pCallBack.m_bUpdateSlotOnly = m_pSelectedSlotUI.IsStacked();

		HideItemInfo();
		if (MoveBetweenToVicinity_VirtualArsenal())
			return;
		
		MoveToVicinity( pItem );
	}

	//------------------------------------------------------------------------------------------------
	//! Checks slotToCheck for any items that cannot be refunded. This is done to avoid deleting mission critical items.
	//! \param[in] slotToCheck is the slot to check - both the item itself aswell as it's contents will be checked.
	//! \param[out] nonrefundableItemInStorage when set to true the slot contains a non refundable item.
	//! \return true if the item is nonrefundable or contains one.
	protected bool DoesSlotContainNonRefundableItems(notnull SCR_InventorySlotUI slotToCheck, out bool nonrefundableItemInStorage = false)
	{
		// Check if the itemitself is nonrefundable
		InventoryItemComponent refundItemComponent = slotToCheck.GetInventoryItemComponent();
		if (!refundItemComponent)
			return false;
		
		SCR_ItemAttributeCollection refundItemAttributes = SCR_ItemAttributeCollection.Cast(refundItemComponent.GetAttributes());
		if (refundItemAttributes && !refundItemAttributes.IsRefundable())
			return true;
		
		BaseInventoryStorageComponent selectedSlotStorage = slotToCheck.GetAsStorage();
		SCR_UniversalInventoryStorageComponent selectedSlotUniversalStorage = SCR_UniversalInventoryStorageComponent.Cast(selectedSlotStorage);
		
		// Check if stroage contains any nonrefundable items
		if (selectedSlotUniversalStorage && selectedSlotUniversalStorage.GetNonRefundableItemCount() > 0)
		{
			nonrefundableItemInStorage = true;
			return true;
		}
		else
		{
			// Check all substorages in a ClothNodeStorage
			ClothNodeStorageComponent selectedSlotClothStorage = ClothNodeStorageComponent.Cast(selectedSlotStorage);
			if (selectedSlotClothStorage)
			{
				array<BaseInventoryStorageComponent> storages = {};
				selectedSlotClothStorage.GetOwnedStorages(storages, 1, false);
				SCR_UniversalInventoryStorageComponent univStor;
				
				foreach (BaseInventoryStorageComponent stor : storages)
				{
					if (!stor)
						continue;
					univStor = SCR_UniversalInventoryStorageComponent.Cast(stor);
					if (!univStor)
						continue;
					if (univStor.GetNonRefundableItemCount() > 0)
					{
						nonrefundableItemInStorage = true;
						return true;
					}
				}
			}
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool MoveBetweenToVicinity_VirtualArsenal()
	{
		BaseInventoryStorageComponent storageComponent = m_pStorageLootUI.GetCurrentNavigationStorage();
		
 		if (!storageComponent && m_aOpenedStoragesUI) //! Relevant for OpenStorage classes
			storageComponent = GetOpenArsenalStorage();
		
		if (!storageComponent || !IsStorageArsenal(storageComponent))	
			return false;
		
		// Check if slot contains any nonrefundable items
		if (DoesSlotContainNonRefundableItems(m_pSelectedSlotUI))
			return false;
		
		//! Perform refund logic.
		IEntity arsenalEntity = storageComponent.GetOwner();
		if (arsenalEntity)
		{
			SCR_ArsenalComponent arsenalComp = SCR_ArsenalComponent.Cast(arsenalEntity.FindComponent(SCR_ArsenalComponent));
			
			//~ Arsenal is disabled so cannot refund
			if (arsenalComp && !arsenalComp.IsArsenalEnabled())
				return true;
		}
		
		InventoryItemComponent inventoryItemComponent	= m_pSelectedSlotUI.GetInventoryItemComponent();
		SCR_ResourceComponent resourceComponent			= SCR_ResourceComponent.FindResourceComponent(arsenalEntity);
		SCR_ResourcePlayerControllerInventoryComponent resourceInventoryComponent = SCR_ResourcePlayerControllerInventoryComponent.Cast(GetGame().GetPlayerController().FindComponent(SCR_ResourcePlayerControllerInventoryComponent));
		
		if (!resourceInventoryComponent)
			return false;
		
		resourceInventoryComponent.RpcAsk_ArsenalRefundItem(Replication.FindId(resourceComponent), Replication.FindId(inventoryItemComponent), EResourceType.SUPPLIES);
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool MoveItem_VirtualArsenal(inout SCR_InventoryStorageBaseUI pStorageBaseUI = null, out bool operationFailed = false)
	{
		SCR_ArsenalInventorySlotUI arsenalInventorySlotUI;
		
		if (m_pSelectedSlotUI)
			 arsenalInventorySlotUI = SCR_ArsenalInventorySlotUI.Cast(m_pSelectedSlotUI);
		else
			return false;
		
		if (!arsenalInventorySlotUI)
		{
			if (m_pActiveHoveredStorageUI && !IsStorageArsenal(m_pActiveHoveredStorageUI.GetCurrentNavigationStorage()))
				return false;
			
			// Check if slot contains any nonrefundable items
			if (DoesSlotContainNonRefundableItems(m_pSelectedSlotUI))
				return false;
			
			//! Perform refund logic.
			BaseInventoryStorageComponent storageComponent = m_pStorageLootUI.GetCurrentNavigationStorage();

			if (!storageComponent) //! Relevant for OpenStorage classes
				storageComponent = m_pActiveHoveredStorageUI.GetStorage();
			
			if (!storageComponent || !IsStorageArsenal(storageComponent) )	
				return false;
			
			IEntity arsenalEntity = storageComponent.GetOwner();
			if (arsenalEntity)
			{
				SCR_ArsenalComponent arsenalComp = SCR_ArsenalComponent.Cast(arsenalEntity.FindComponent(SCR_ArsenalComponent));
				
				//~ Arsenal is disabled so cannot refund
				if (arsenalComp && !arsenalComp.IsArsenalEnabled())
				{
					operationFailed = true;
					return true;
				}
			}
				
			InventoryItemComponent inventoryItemComponent	= m_pSelectedSlotUI.GetInventoryItemComponent();
			SCR_ResourceComponent resourceComponent			= SCR_ResourceComponent.FindResourceComponent(arsenalEntity);
			SCR_ResourcePlayerControllerInventoryComponent resourceInventoryComponent = SCR_ResourcePlayerControllerInventoryComponent.Cast(GetGame().GetPlayerController().FindComponent(SCR_ResourcePlayerControllerInventoryComponent));
			
			resourceInventoryComponent.RpcAsk_ArsenalRefundItem(Replication.FindId(resourceComponent), Replication.FindId(inventoryItemComponent), EResourceType.SUPPLIES);

			return true;
		}
		
		BaseInventoryStorageComponent storageComponent = m_pActiveHoveredStorageUI.GetCurrentNavigationStorage();
		if (!storageComponent)
			return false;
		if (arsenalInventorySlotUI && storageComponent.GetOwner().FindComponent(SCR_ArsenalInventoryStorageManagerComponent))
			return true;
		
		SCR_InventoryStorageManagerComponent invManagerTo	= m_pActiveHoveredStorageUI.GetInventoryManager();
		BaseInventoryStorageComponent storageTo				= m_pActiveHoveredStorageUI.GetCurrentNavigationStorage();
		IEntity arsenalEntity								= SCR_InventoryStorageBaseUI.ARSENAL_SLOT_STORAGES.Get(arsenalInventorySlotUI);
		SCR_ResourceComponent resourceComponent				= SCR_ResourceComponent.FindResourceComponent(arsenalEntity);
		ResourceName resourceName							= arsenalInventorySlotUI.GetItemResource();
		SCR_ResourcePlayerControllerInventoryComponent resourceInventoryComponent = SCR_ResourcePlayerControllerInventoryComponent.Cast(GetGame().GetPlayerController().FindComponent(SCR_ResourcePlayerControllerInventoryComponent));

		if (storageTo.IsInherited(ClothNodeStorageComponent))
		{
			storageTo = invManagerTo.FindActualStorageForItemResource(resourceName, storageTo);
			if (!storageTo)
			{
				operationFailed = true;
				return true;
			}
		}

		if (invManagerTo.CanInsertItemInStorage(arsenalInventorySlotUI.GetInventoryItemComponent().GetOwner(), storageTo))
			resourceInventoryComponent.RpcAsk_ArsenalRequestItem(Replication.FindId(resourceComponent), Replication.FindId(invManagerTo),  Replication.FindId(storageTo), resourceName, EResourceType.SUPPLIES);
		else
			operationFailed = true;

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool EquipWeaponIntoWeaponSlot_VirtualArsenal()
	{
		SCR_ArsenalInventorySlotUI arsenalInventorySlotUI;
		
		if (m_pSelectedSlotUI)
			 arsenalInventorySlotUI = SCR_ArsenalInventorySlotUI.Cast(m_pSelectedSlotUI);
		else if (!m_pSelectedSlotUI || !arsenalInventorySlotUI)
			return false;
		
		SCR_InventorySlotWeaponSlotsUI weaponSlot = SCR_InventorySlotWeaponSlotsUI.Cast(m_pFocusedSlotUI);
		
		if (!weaponSlot)
			return false;
		
		SCR_InventoryStorageManagerComponent invManagerTo	= weaponSlot.GetStorageUI().GetInventoryManager();
		BaseInventoryStorageComponent storageTo				= weaponSlot.GetStorageUI().GetStorage();
		IEntity arsenalEntity								= SCR_InventoryStorageBaseUI.ARSENAL_SLOT_STORAGES.Get(arsenalInventorySlotUI);
		if (!arsenalEntity)
			return false;
		SCR_ResourceComponent resourceComponent				= SCR_ResourceComponent.FindResourceComponent(arsenalEntity);
		ResourceName resourceName							= arsenalInventorySlotUI.GetItemResource();
		auto resourceInventoryComponent						= SCR_ResourcePlayerControllerInventoryComponent.Cast(GetGame().GetPlayerController().FindComponent(SCR_ResourcePlayerControllerInventoryComponent));
		
		if (CanAddAttachment(arsenalInventorySlotUI, weaponSlot))
		{
			storageTo = weaponSlot.GetStorageComponent();
		}

		if (invManagerTo.CanInsertItemInStorage(arsenalInventorySlotUI.GetInventoryItemComponent().GetOwner(), storageTo))
			resourceInventoryComponent.RpcAsk_ArsenalRequestItem(Replication.FindId(resourceComponent), Replication.FindId(invManagerTo),  Replication.FindId(storageTo), resourceName, EResourceType.SUPPLIES);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//!	Attempts to process MoveItemToStorageSlot logic specific for virtual arsenal and returns 
	//!		whenever it was successful or not.
	//!	
	//!	\return true if the operation was indeed for virtual arsenal, false otherwise.
	protected bool MoveItemToStorageSlot_VirtualArsenal(out bool operationFailed = false)
	{
		SCR_ArsenalInventorySlotUI arsenalInventorySlotUI;
		
		if (m_pSelectedSlotUI)
			 arsenalInventorySlotUI = SCR_ArsenalInventorySlotUI.Cast(m_pSelectedSlotUI);
		else
			return false;
		
		if (!arsenalInventorySlotUI)
		{
			// If the slot was dragged onto a non arsenal slot then do not process virtual arsenal.
			if (!SCR_ArsenalInventorySlotUI.Cast(m_pFocusedSlotUI))
				return false;
			
			// Check if slot contains any nonrefundable items
			if (DoesSlotContainNonRefundableItems(m_pSelectedSlotUI))
				return false;
			
			//! Perform refund logic.
			BaseInventoryStorageComponent storageComponent = m_pStorageLootUI.GetCurrentNavigationStorage();
			
			if (!storageComponent) //! Relevant for OpenStorage classes
				storageComponent = m_pActiveHoveredStorageUI.GetStorage();
			
			if (!storageComponent || !IsStorageArsenal(storageComponent) )	
				return false;
			
			IEntity arsenalEntity = storageComponent.GetOwner();
			if (arsenalEntity)
			{
				SCR_ArsenalComponent arsenalComp = SCR_ArsenalComponent.Cast(arsenalEntity.FindComponent(SCR_ArsenalComponent));
				
				//~ Arsenal is disabled so cannot refund
				if (arsenalComp && !arsenalComp.IsArsenalEnabled())
				{
					operationFailed = true;
					return true;
				}
			}
				
			InventoryItemComponent inventoryItemComponent	= m_pSelectedSlotUI.GetInventoryItemComponent();
			SCR_ResourceComponent resourceComponent			= SCR_ResourceComponent.FindResourceComponent(arsenalEntity);
			SCR_ResourcePlayerControllerInventoryComponent resourceInventoryComponent = SCR_ResourcePlayerControllerInventoryComponent.Cast(GetGame().GetPlayerController().FindComponent(SCR_ResourcePlayerControllerInventoryComponent));
		
			resourceInventoryComponent.RpcAsk_ArsenalRefundItem(Replication.FindId(resourceComponent), Replication.FindId(inventoryItemComponent), EResourceType.SUPPLIES);
			
			return true;
		}
		
		/*
		Do nothing but exit as successful because of dragging arsenal virtual item into another
			arsenal virtual item.
		*/
		if (SCR_ArsenalInventorySlotUI.Cast(m_pFocusedSlotUI))
		{
			operationFailed = true;
			return true;
		}
		
		SCR_InventoryStorageManagerComponent invManagerTo	= m_pFocusedSlotUI.GetStorageUI().GetInventoryManager();
		BaseInventoryStorageComponent storageTo				= m_pFocusedSlotUI.GetAsStorage();
		if (!storageTo)
			storageTo = m_pFocusedSlotUI.GetStorageUI().GetStorage();

		IEntity arsenalEntity								= SCR_InventoryStorageBaseUI.ARSENAL_SLOT_STORAGES.Get(arsenalInventorySlotUI);
		SCR_ResourceComponent resourceComponent				= SCR_ResourceComponent.FindResourceComponent(arsenalEntity);
		ResourceName resourceName							= arsenalInventorySlotUI.GetItemResource();
		SCR_ResourcePlayerControllerInventoryComponent resourceInventoryComponent = SCR_ResourcePlayerControllerInventoryComponent.Cast(GetGame().GetPlayerController().FindComponent(SCR_ResourcePlayerControllerInventoryComponent));
		
		if (storageTo.IsInherited(ClothNodeStorageComponent))
		{
			storageTo = invManagerTo.FindActualStorageForItemResource(resourceName, storageTo);
			if (!storageTo)
			{
				operationFailed = true;
				return true;
			}
		}

		if (invManagerTo.CanInsertItemInStorage(arsenalInventorySlotUI.GetInventoryItemComponent().GetOwner(), storageTo))
			resourceInventoryComponent.RpcAsk_ArsenalRequestItem(Replication.FindId(resourceComponent), Replication.FindId(invManagerTo),  Replication.FindId(storageTo), resourceName, EResourceType.SUPPLIES);
		else
			operationFailed = true;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool EquipDraggedItem_VirtualArsenal(bool forceEquip = false)
	{
		SCR_ArsenalInventorySlotUI arsenalInventorySlotUI;
		
		if (m_pSelectedSlotUI)
			 arsenalInventorySlotUI = SCR_ArsenalInventorySlotUI.Cast(m_pSelectedSlotUI);
		
		if (!m_pSelectedSlotUI || !arsenalInventorySlotUI)
			return false;

		if (!arsenalInventorySlotUI.IsAvailable())
			return true;

		SCR_InventoryStorageManagerComponent invManagerTo	= m_InventoryManager;
		IEntity slotEntity									= arsenalInventorySlotUI.GetInventoryItemComponent().GetOwner();
		BaseInventoryStorageComponent storageTo				= m_InventoryManager.FindStorageForItem(slotEntity, EStoragePurpose.PURPOSE_ANY);
		IEntity arsenalEntity								= SCR_InventoryStorageBaseUI.ARSENAL_SLOT_STORAGES.Get(arsenalInventorySlotUI);
		SCR_ResourceComponent resourceComponent				= SCR_ResourceComponent.FindResourceComponent(arsenalEntity);
		ResourceName resourceName							= arsenalInventorySlotUI.GetItemResource();
		auto resourceInventoryComponent						= SCR_ResourcePlayerControllerInventoryComponent.Cast(GetGame().GetPlayerController().FindComponent(SCR_ResourcePlayerControllerInventoryComponent));
		
		if (invManagerTo.CanInsertItemInStorage(slotEntity, storageTo))
			resourceInventoryComponent.RpcAsk_ArsenalRequestItem(Replication.FindId(resourceComponent), Replication.FindId(invManagerTo),  Replication.FindId(storageTo), resourceName, EResourceType.SUPPLIES);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool MoveBetweenFromVicinity_VirtualArsenal()
	{
		SCR_ArsenalInventorySlotUI arsenalInventorySlotUI;
		
		if (m_pSelectedSlotUI)
			 arsenalInventorySlotUI = SCR_ArsenalInventorySlotUI.Cast(m_pSelectedSlotUI);
		
		if (!m_pSelectedSlotUI || !arsenalInventorySlotUI)
			return false;
		
		if (!arsenalInventorySlotUI.IsAvailable())
			return true;
		
		SCR_InventoryStorageManagerComponent invManagerTo	= m_InventoryManager;
		IEntity slotEntity									= arsenalInventorySlotUI.GetInventoryItemComponent().GetOwner();
		BaseInventoryStorageComponent storageTo				= m_InventoryManager.FindStorageForItem(slotEntity, EStoragePurpose.PURPOSE_ANY);
		IEntity arsenalEntity								= SCR_InventoryStorageBaseUI.ARSENAL_SLOT_STORAGES.Get(arsenalInventorySlotUI);
		SCR_ResourceComponent resourceComponent				= SCR_ResourceComponent.FindResourceComponent(arsenalEntity);
		ResourceName resourceName							= arsenalInventorySlotUI.GetItemResource();
		auto resourceInventoryComponent						= SCR_ResourcePlayerControllerInventoryComponent.Cast(GetGame().GetPlayerController().FindComponent(SCR_ResourcePlayerControllerInventoryComponent));
		
		if (invManagerTo.CanInsertItemInStorage(slotEntity, storageTo))
			resourceInventoryComponent.RpcAsk_ArsenalRequestItem(Replication.FindId(resourceComponent), Replication.FindId(invManagerTo),  Replication.FindId(storageTo), resourceName, EResourceType.SUPPLIES);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool MoveItem_ResourceContainer(inout SCR_InventoryStorageBaseUI pStorageBaseUI = null)
	{
		if (!m_pSelectedSlotUI)
			return false;
		
		IEntity draggedEntity = m_pSelectedSlotUI.GetInventoryItemComponent().GetOwner();
		
		if (!draggedEntity)
			return false;
		
		SCR_ResourceComponent resourceComponentFrom = SCR_ResourceComponent.FindResourceComponent(draggedEntity);
		
		if (!resourceComponentFrom)
			return false;
		
		//~ TODO: Hotfix until proper solution, block moving supplies if occupied by character
		SCR_BaseCompartmentManagerComponent compartmentManager = SCR_BaseCompartmentManagerComponent.Cast(resourceComponentFrom.GetOwner().FindComponent(SCR_BaseCompartmentManagerComponent));
		if (compartmentManager && compartmentManager.BlockSuppliesIfOccupied() && compartmentManager.GetOccupantCount() > 0)
			return false;
		
		if (!m_pActiveHoveredStorageUI)
			return false;
		
		SCR_InventoryStorageManagerComponent invManagerTo = m_pActiveHoveredStorageUI.GetInventoryManager();
		
		if (!invManagerTo)
			return false;
		
		SCR_ResourceInteractor interactorFrom = resourceComponentFrom.GetConsumer(EResourceGeneratorID.DEFAULT_STORAGE, EResourceType.SUPPLIES);
		
		if (!interactorFrom)
			interactorFrom = resourceComponentFrom.GetConsumer(EResourceGeneratorID.VEHICLE_UNLOAD, EResourceType.SUPPLIES);
		
		if (!interactorFrom)
			return false;
		
		BaseInventoryStorageComponent storageTo = m_pActiveHoveredStorageUI.GetStorage();
		
		//! Create to vicinity
		if (!storageTo)
		{
			SCR_EntityCatalog resourceContainerCatalog = SCR_EntityCatalogManagerComponent.GetInstance().GetEntityCatalogOfType(EEntityCatalogType.SUPPLY_CONTAINER_ITEM); 
			array<SCR_EntityCatalogEntry> entries = {};
			array<SCR_BaseEntityCatalogData> data = {};
			
			resourceContainerCatalog.GetEntityListWithData(SCR_ResourceContainerItemData, entries, data);
		
			SCR_ResourceContainerItemData datum = SCR_ResourceContainerItemData.Cast(data[0]);
			float maxStoredResources = Math.Min(interactorFrom.GetAggregatedResourceValue(), datum.GetMaxResourceValue());
			
			if (maxStoredResources <= 0)
				return true;
			
			CreateItemSplitDialog(maxStoredResources, null, draggedEntity);
			
			return true;
		}
		
		IEntity targetEntity = storageTo.GetOwner();
		if (!targetEntity)
			return false;
		
		SCR_ResourceComponent targetResourceComponent = SCR_ResourceComponent.FindResourceComponent(targetEntity);
		
		//! If targetEntity has a resource component on them then we cancel but we interrupt 
		//! the whole procesing chain of MoveItemToStorageSlot.
		if (targetResourceComponent)
			return MoveItemToStorageSlot_ResourceContainer();
		
		//! If they are the same slot (or same resourceComponent) and have resource component on them then we cancel but we interrupt 
		//! the whole procesing chain of MoveItemToStorageSlot.
		if (draggedEntity == targetEntity || resourceComponentFrom == targetResourceComponent)
			return true;
		
		SCR_EntityCatalog resourceContainerCatalog = SCR_EntityCatalogManagerComponent.GetInstance().GetEntityCatalogOfType(EEntityCatalogType.SUPPLY_CONTAINER_ITEM); 
		array<SCR_EntityCatalogEntry> entries = {};
		array<SCR_BaseEntityCatalogData> data = {};
		float maxResourceValueTo = 0.0;
		
		resourceContainerCatalog.GetEntityListWithData(SCR_ResourceContainerItemData, entries, data);
		
		foreach (int idx, SCR_EntityCatalogEntry entry: entries)
		{
			SCR_ResourceContainerItemData datum = SCR_ResourceContainerItemData.Cast(data[idx]);
			
			if (invManagerTo.CanInsertResourceInStorage(entry.GetPrefab(), storageTo))
			{
				maxResourceValueTo = datum.GetMaxResourceValue();
				break;
			}
		}
		
		float maxStoredResources = Math.Min(interactorFrom.GetAggregatedResourceValue(), maxResourceValueTo);
		
		if (maxStoredResources <= 0)
			return true;
		
		CreateItemSplitDialog(maxStoredResources, targetEntity, draggedEntity);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool MoveItemToStorageSlot_ResourceContainer()
	{
		if (!m_pSelectedSlotUI)
			return false;
		
		SCR_InventorySlotUI draggedSlot = m_pSelectedSlotUI;
		if (m_bDraggingEnabled)
			draggedSlot = m_DraggedSlot;

		InventoryItemComponent iic = draggedSlot.GetInventoryItemComponent();
		if (!iic)
			return false;

		IEntity draggedEntity = iic.GetOwner();
		if (!draggedEntity)
			return false;
		
		SCR_ResourceComponent resourceComponentFrom = SCR_ResourceComponent.FindResourceComponent(draggedEntity);
		
		if (!resourceComponentFrom)
			return false;
		
		IEntity targetEntity;
		
		if (m_pFocusedSlotUI && m_pFocusedSlotUI.GetInventoryItemComponent())
			targetEntity = m_pFocusedSlotUI.GetInventoryItemComponent().GetOwner();
		else if (m_pActiveHoveredStorageUI && m_pActiveHoveredStorageUI.GetStorage())
			targetEntity = m_pActiveHoveredStorageUI.GetStorage().GetOwner();
		
		if (!targetEntity)
			return false;
		
		SCR_ResourceComponent resourceComponentTo = SCR_ResourceComponent.FindResourceComponent(targetEntity);
		
		if (!resourceComponentTo)
			return false;
		
		//~ TODO: Hotfix until proper solution, block moving supplies if occupied by character
		SCR_BaseCompartmentManagerComponent compartmentManager = SCR_BaseCompartmentManagerComponent.Cast(resourceComponentTo.GetOwner().FindComponent(SCR_BaseCompartmentManagerComponent));
		if (compartmentManager && compartmentManager.BlockSuppliesIfOccupied() && compartmentManager.GetOccupantCount() > 0)
				return false;
		
		SCR_ResourceInteractor interactorFrom = resourceComponentFrom.GetConsumer(EResourceGeneratorID.DEFAULT_STORAGE, EResourceType.SUPPLIES);
		
		if (!interactorFrom)
			interactorFrom = resourceComponentFrom.GetConsumer(EResourceGeneratorID.VEHICLE_UNLOAD, EResourceType.SUPPLIES);
		
		if (!interactorFrom)
			return false;
		
		SCR_ResourceInteractor interactorTo = resourceComponentTo.GetConsumer(EResourceGeneratorID.DEFAULT_STORAGE, EResourceType.SUPPLIES);
		
		if (!interactorTo)
			interactorTo = resourceComponentTo.GetGenerator(EResourceGeneratorID.VEHICLE_LOAD, EResourceType.SUPPLIES);
		
		if (!interactorTo)
			return false;
		
		//! If they are the same slot and have resource component on them then we cancel but we interrupt 
		//! the whole procesing chain of MoveItemToStorageSlot.
		if (draggedSlot == m_pFocusedSlotUI)
			return true;
		
		float maxStoredResources = Math.Min(interactorFrom.GetAggregatedResourceValue(), interactorTo.GetAggregatedMaxResourceValue() - interactorTo.GetAggregatedResourceValue());
		
		if (maxStoredResources <= 0)
			return true;
		
		CreateItemSplitDialog(maxStoredResources, targetEntity, draggedEntity);
		
		return true;
	}

	//---- REFACTOR NOTE END ----
	//------------------------------------------------------------------------------------------------
	void DeselectSlot()
	{
		if (!m_pSelectedSlotUI)
			return;

		m_pSelectedSlotUI.SetSelected(false);
		m_pSelectedSlotUI = null;

		if (m_pNavigationBar)
			m_pNavigationBar.SetButtonActionName(BUTTON_SELECT, GAMEPAD_HINT_SELECT);
	}
	
	//------------------------------------------------------------------------------------------------
	bool SetOpenStorage()
	{
		IEntity m_pStorageToOpen = GetInventoryStorageManager().GetStorageToOpen();

		if (!m_pStorageToOpen)
			return false;

		BaseInventoryStorageComponent comp = BaseInventoryStorageComponent.Cast(m_pStorageToOpen.FindComponent(BaseInventoryStorageComponent));

		if (!comp)
			return false;

		SCR_InventoryStorageBaseUI storageUI = GetStorageUIFromVicinity(comp);

		if (!storageUI)
			return false;

		if (storageUI.IsTraversalAllowed())
			storageUI.Traverse(comp);

		SetStorageSwitchMode(false);

		ButtonWidget lastCloseTraverseButton = storageUI.GetLastCloseTraverseButton();

		if (m_bIsUsingGamepad && lastCloseTraverseButton && !CanFocusOnSlotInStorage(storageUI, 0))
			GetGame().GetCallqueue().Call(FocusWidget, lastCloseTraverseButton);
		else
			GetGame().GetCallqueue().Call(FocusOnSlotInStorage, storageUI, 0);
		
		return true;
	}
	
	protected void FocusWidget(Widget focusWidget)
	{
		GetGame().GetWorkspace().SetFocusedWidget(focusWidget);
	}

	//------------------------------------------------------------------------------------------------
	//!
	protected void TraverseActualSlot()
	{
		m_pSelectedSlotUI = m_pFocusedSlotUI;
		auto storage = m_pFocusedSlotUI.GetAsStorage();
		if (storage)
		{
			SCR_InventoryStorageBaseUI parentUIContainer = m_pFocusedSlotUI.GetStorageUI();
			
			if (parentUIContainer.IsTraversalAllowed() && ShouldAllowTraverseActualSlot())
				parentUIContainer.Traverse(storage);
			
			ButtonWidget lastCloseTraverseButton = parentUIContainer.GetLastCloseTraverseButton();
			
			if (m_bIsUsingGamepad && lastCloseTraverseButton && !CanFocusOnSlotInStorage(parentUIContainer, 0))
				GetGame().GetWorkspace().SetFocusedWidget(lastCloseTraverseButton);
			else
				FocusOnSlotInStorage(parentUIContainer);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Method that returns if traverse on selected slot should be performed.
	//! \return true if traverse on the selected slot should be performed, false otherwise.
	protected bool ShouldAllowTraverseActualSlot()
	{
		InventoryItemComponent inventoryItemComponent = m_pSelectedSlotUI.GetInventoryItemComponent();
		
		// Check if the selected slot is an arsenal, and if it's in some inventory item then disallow traverse.
		if (inventoryItemComponent && inventoryItemComponent.GetParentSlot() && inventoryItemComponent.GetOwner().FindComponent(SCR_ArsenalInventoryStorageManagerComponent))
			return false;
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void OpenAsNewContainer()
	{
		m_pSelectedSlotUI = m_pFocusedSlotUI;
		auto storage = m_pFocusedSlotUI.GetAsStorage();
		if (storage)
		{
			OpenStorageAsContainer(storage);
			OpenLinkedStorages(storage);
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INV_HOTKEY_OPEN);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Opens any linked storages when the parent storage is opened via traversal or as a container
	\param parentStorage Parent Storage of which linked storages should be opened
	\param showVicinity If true will force showVicinity, set to false if opening linked storages via traversal
	*/
	void OpenLinkedStorages(BaseInventoryStorageComponent parentStorage, bool showVicinity = true, bool hideCloseButton = true)
	{
		SCR_UniversalInventoryStorageComponent scrStorage = SCR_UniversalInventoryStorageComponent.Cast(parentStorage);
		if (!scrStorage)
			return;
		
		OpenLinkedStoragesDelayed(scrStorage, showVicinity, hideCloseButton);
		
		//~ Call the actual open linked storages 1 frame later so it can check properly if the storages it should open are already open
		//GetGame().GetCallqueue().CallLater(OpenLinkedStoragesDelayed, param1: scrStorage, param2: showVicinity, param3: hideCloseButton);
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Don't call directly. Call OpenLinkedStorages() instead!
	protected void OpenLinkedStoragesDelayed(SCR_UniversalInventoryStorageComponent scrStorage, bool showVicinity = true, bool hideCloseButton = true)
	{
		array<BaseInventoryStorageComponent> linkedStorages = {};
		if (scrStorage.GetLinkedStorages(linkedStorages) <= 0)
			return;		
		
		bool alreadyOpen;
		
		foreach (BaseInventoryStorageComponent linkedStorage : linkedStorages)
		{
			//~ Storage is already open by means of traversal
			if (m_pStorageLootUI && m_pStorageLootUI.GetCurrentNavigationStorage() == linkedStorage)
				continue;
			
			alreadyOpen = false;
			
			foreach (SCR_InventoryOpenedStorageUI openedStorage : m_aOpenedStoragesUI)
			{
				//~ Already open
				if (openedStorage.GetStorage() == linkedStorage)
				{
					alreadyOpen = true;
					break;
				}
			}
			
			if (alreadyOpen)
				continue;
			
			OpenStorageAsContainer(linkedStorage, showVicinity, hideCloseButton);
		}
	}
	
	//---- REFACTOR NOTE START: As an call later function as it needs to check if the parent is still open ----
	//------------------------------------------------------------------------------------------------
	/*!
	Closes any linked storages when the parent storage is closed via traversal or closed as a container
	\param parentStorage Parent Storage of which linked storages should be closed
	*/
	void CloseLinkedStorages(BaseInventoryStorageComponent parentStorage)
	{
		SCR_UniversalInventoryStorageComponent scrStorage = SCR_UniversalInventoryStorageComponent.Cast(parentStorage);
		if (!scrStorage)
			return;
		
		//~ Call the actual close linked storages 1 frame later so it can check if the parent storage is still open via container or traversal
		GetGame().GetCallqueue().CallLater(CloseLinkedStoragesDelayed, param1: scrStorage);
	}
	//---- REFACTOR NOTE END ----
	
	//------------------------------------------------------------------------------------------------
	//~ Don't call directly. Call CloseLinkedStorages() instead!
	protected void CloseLinkedStoragesDelayed(SCR_UniversalInventoryStorageComponent scrStorage)
	{
		if (!scrStorage)
			return;
		
		array<BaseInventoryStorageComponent> linkedStorages = {};
		if (scrStorage.GetLinkedStorages(linkedStorages) <= 0)
			return;
	
		//~ Trying to close linked storages but the parent storage is still open via traversal
		if (m_pStorageLootUI && m_pStorageLootUI.GetCurrentNavigationStorage() == scrStorage)
			return;
		
		//~ Trying to close linked storages but the parent storage is still open as a container
		foreach (SCR_InventoryOpenedStorageUI openedStorage : m_aOpenedStoragesUI)
		{
			if (!openedStorage)
				continue;
			
			if (openedStorage.GetStorage() == scrStorage)
				return;
		}

		//~ Close each linked storage that is open
		foreach (BaseInventoryStorageComponent linkedStorage : linkedStorages)
		{
			foreach (SCR_InventoryOpenedStorageUI openedStorage : m_aOpenedStoragesUI)
			{
				if (!openedStorage)
					continue;
				
				//~ Storage is open
				if (openedStorage.GetStorage() == linkedStorage)
				{
					RemoveOpenStorage(openedStorage);
					break;
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! when clicked on item / storage slot using controller or mouse
	protected void Action_SelectItem()
	{
		// Ignore if we cannot interact
		if (!GetCanInteract())
			return;
		SimpleFSM( EMenuAction.ACTION_SELECT );
	}

	//------------------------------------------------------------------------------------------------
	void Action_DeselectItem()
	{
		SimpleFSM(EMenuAction.ACTION_DESELECT);
		ResetHighlightsOnAvailableStorages();
	}

	//------------------------------------------------------------------------------------------------
	protected void Action_MoveInsideStorage()
	{
		SimpleFSM(EMenuAction.ACTION_MOVEINSIDE);
		ResetHighlightsOnAvailableStorages();
	}

	//------------------------------------------------------------------------------------------------
	protected void Action_EquipItem()
	{
		if (m_pFocusedSlotUI)
			m_pFocusedSlotUI.UseItem(m_Player, SCR_EUseContext.FROM_INVENTORY);

		NavigationBarUpdate();
	}

	//------------------------------------------------------------------------------------------------
	protected void Action_MoveBetween()
	{
		// Ignore if we cannot interact
		if (!GetCanInteract())
			return;
		SimpleFSM( EMenuAction.ACTION_MOVEBETWEEN );
		ResetHighlightsOnAvailableStorages();
	}

	//------------------------------------------------------------------------------------------------
	void Action_Inspect()
	{	
		if (m_pFocusedSlotUI == m_pInspectedSlot)
			InspectItem(null);
		else
			InspectItem(m_pFocusedSlotUI);
		
		SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INV_WEAPON_INSPECT);
	}

	//------------------------------------------------------------------------------------------------
	//!
	protected void Action_UseItem()
	{
		if (m_pFocusedSlotUI)
		{
			// TODO: Currently won't work if there are no HitzonePointContainers. (Won't work for saline bag if the player is not injured in any way)
			SCR_InventoryHitZonePointContainerUI hzContainer = SCR_InventoryHitZonePointContainerUI.Cast(m_AttachmentSpinBox.GetCurrentItemData());
			if (hzContainer)
			{
				SCR_InventoryHitZoneUI hzStorage = hzContainer.GetStorage();
				if (!hzStorage)
					return;
				
				IEntity item = m_pFocusedSlotUI.GetInventoryItemComponent().GetOwner();
				if (!item)
					return;
				
				if (hzStorage.CanApplyItem(item))
				{
					hzStorage.OnDrop(m_pFocusedSlotUI);
				}
				else
				{
					hzContainer = null;
					hzStorage = null;
					
					for (int i = 0, max = m_AttachmentSpinBox.GetNumItems(); i < max; i++)
					{
						hzContainer = SCR_InventoryHitZonePointContainerUI.Cast(m_AttachmentSpinBox.GetItemData(i));
						if (!hzContainer)
							continue;
						
						hzStorage = hzContainer.GetStorage();
						if (hzStorage && hzStorage.CanApplyItem(item))
							hzStorage.OnDrop(m_pFocusedSlotUI)
					}
				}
			}

			SCR_InventorySlotGearInspectionUI point = SCR_InventorySlotGearInspectionUI.Cast(m_AttachmentSpinBox.GetCurrentItemData());
			if (point)
			{
				m_pGearInspectionPointUI.SetSlotFocused(point);
				point.OnDrop(m_pFocusedSlotUI);
			}
			
			if (m_pFocusedSlotUI && (!hzContainer || !point))
			{
				m_pFocusedSlotUI.UseItem(m_Player, SCR_EUseContext.FROM_INVENTORY);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	protected void Action_UnfoldItem()
	{
 		SimpleFSM( EMenuAction.ACTION_UNFOLD );
	}

	//------------------------------------------------------------------------------------------------
	void Action_StepBack(SCR_InventoryStorageBaseUI pParentStorage = null, int traverseStorageIndex = -1)
	{
		//TODO: m_pActiveStorageUI must be set according the active SCR_InventoryStorageBaseUI
		if (pParentStorage && pParentStorage.IsTraversalAllowed())
		{
			pParentStorage.Back(traverseStorageIndex);
			FilterOutStorages(false);
			m_EStateMenuItem = EStateMenuItem.STATE_IDLE;
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INV_CONTAINER_CLOSE);
			FocusOnSlotInStorage(pParentStorage);
		}
		ResetHighlightsOnAvailableStorages();
	}
	//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
	//------------------------------------------------------------------------------------------------
	void Action_DragDown()
	{
		if ( m_bDraggingEnabled )
			return;
		if ( !m_pFocusedSlotUI )
			return;
		if (!m_pFocusedSlotUI.IsDraggable())
			return;

		if ( m_pFocusedSlotUI && WidgetManager.GetWidgetUnderCursor() != m_pFocusedSlotUI.GetButtonWidget() )
			return;

		if 	(m_pFocusedSlotUI.Type() != SCR_InventorySlotUI &&
			m_pFocusedSlotUI.Type() != SCR_InventorySlotLBSUI &&
			m_pFocusedSlotUI.Type() != SCR_InventorySlotStorageUI &&
			m_pFocusedSlotUI.Type() != SCR_InventorySlotStorageEmbeddedUI &&
			m_pFocusedSlotUI.Type() != SCR_InventoryStorageWeaponsUI &&
			m_pFocusedSlotUI.Type() != SCR_InventorySlotWeaponUI &&
			m_pFocusedSlotUI.Type() != SCR_InventorySlotQuickSlotUI &&
			m_pFocusedSlotUI.Type() != SCR_InventorySlotWeaponSlotsUI &&
			m_pFocusedSlotUI.Type() != SCR_InventorySlotGearInspectionUI &&
			m_pFocusedSlotUI.Type() != SCR_ArsenalInventorySlotUI &&
			m_pFocusedSlotUI.Type() != SCR_SupplyInventorySlotUI)
			return;

		m_bDraggingEnabled = false;
		WidgetManager.GetMousePos( m_iMouseX, m_iMouseY );
		GetGame().GetInputManager().AddActionListener( "Inventory_Drag", EActionTrigger.PRESSED, Action_OnDrag );
	}

	//------------------------------------------------------------------------------------------------
	void Action_OnDrag()
	{
		int iMouseX, iMouseY;
		WidgetManager.GetMousePos( iMouseX, iMouseY );
		if ( !m_bDraggingEnabled )
		{
			int dX = Math.AbsInt( iMouseX - m_iMouseX );
			int dY = Math.AbsInt( iMouseY - m_iMouseY );
			if ( ( dX < DRAG_THRESHOLD ) && ( dY < DRAG_THRESHOLD ) )
				return;
			if (!m_pFocusedSlotUI)
			{
				m_bDraggingEnabled = true;
				GetGame().GetInputManager().AddActionListener("Inventory_Drag", EActionTrigger.UP, Action_DragUp);
				return;
			}
				
			if (!m_pFocusedSlotUI.IsDraggable())
				return;
			InventoryItemComponent focusedItemComp = m_pFocusedSlotUI.GetInventoryItemComponent();
			if (!focusedItemComp)
				return;
			
			m_DraggedSlot = m_pFocusedSlotUI;
			InventoryStorageSlot slot = focusedItemComp.GetParentSlot();
			if (slot)
			{
				IEntity owner = slot.GetOwner();
				while (owner)
				{
					if (GetGame().GetPlayerController().GetControlledEntity() == owner)
					{
						SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INV_CONTAINER_DRAG);
						break;
					}
					
					owner = owner.GetParent();
					if (!owner)
						SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INV_VICINITY_DRAG);
				}
			}
			else
			{
				SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INV_VICINITY_DRAG);
			}
			
			m_bDraggingEnabled = true;
			SimpleFSM( EMenuAction.ACTION_DRAGGED );
			m_wDragDropContainer.SetVisible( true );

			if ( m_pPreviewManager && m_pSelectedSlotUI )
			{
				HighlightAvailableStorages(m_pSelectedSlotUI);
				IEntity previewEntity = null;
				InventoryItemComponent pComp = m_pSelectedSlotUI.GetInventoryItemComponent();
				if ( pComp )
				{
					previewEntity = pComp.GetOwner();
					if (m_pDragDropPreviewImage)
						m_pPreviewManager.SetPreviewItem(m_pDragDropPreviewImage, previewEntity);
				}
			}
			ContainerSetPos( iMouseX, iMouseY );
			GetGame().GetInputManager().AddActionListener( "Inventory_Drag", EActionTrigger.UP, Action_DragUp );
		}
		else
		{
			ContainerSetPos( iMouseX, iMouseY );
		}
	}

	//------------------------------------------------------------------------------------------------
	void Action_DragUp()
	{
		GetGame().GetInputManager().RemoveActionListener( "Inventory_Drag", EActionTrigger.PRESSED, Action_OnDrag );
		GetGame().GetInputManager().RemoveActionListener( "Inventory_Drag", EActionTrigger.UP, Action_DragUp );
		if (m_pPreviewManager)
			m_pPreviewManager.SetPreviewItem(m_pDragDropPreviewImage, null);
		GetGame().GetCallqueue().CallLater(m_wDragDropContainer.SetVisible, 0, false, false);
		if (m_DraggedSlot)
			SimpleFSM( EMenuAction.ACTION_DROPPED );
		m_bDraggingEnabled = false;
		m_DraggedSlot = null;
		ResetHighlightsOnAvailableStorages();
	}
	//---- REFACTOR NOTE END ----

	//------------------------------------------------------------------------------------------------
	void ContainerSetPos( int iMouseX, int iMouseY )
	{
		float fWidth, fHeight;
		WorkspaceWidget wWorkSpace = GetGame().GetWorkspace();
		m_wDragDropContainer.GetScreenSize( fWidth, fHeight );
		fWidth = wWorkSpace.DPIUnscale( fWidth ) / 2;
		fHeight = wWorkSpace.DPIUnscale( fHeight ) / 2;

		m_pDragDropFrameSlotUI.SetPosX( wWorkSpace.DPIUnscale( iMouseX ) - fWidth );
		m_pDragDropFrameSlotUI.SetPosY( wWorkSpace.DPIUnscale( iMouseY ) - fHeight );
	}

	//------------------------------------------------------------------------------------------------
	// !
	protected void Action_Move( SCR_InventoryStorageBaseUI pStorageBase )
	{
		IEntity pItem = m_pSelectedSlotUI.GetInventoryItemComponent().GetOwner();
		m_InventoryManager.InsertItem( pItem, pStorageBase.GetCurrentNavigationStorage(), m_pFocusedSlotUI.GetStorageUI().GetCurrentNavigationStorage() );
		ResetHighlightsOnAvailableStorages();
	}

	//------------------------------------------------------------------------------------------------
	SCR_InventoryOpenedStorageUI GetOpenedStorage(BaseInventoryStorageComponent storage)
	{
		if (!storage)
			return null;

		foreach (SCR_InventoryOpenedStorageUI openedStorage : m_aOpenedStoragesUI)
		{
			if (openedStorage.GetStorage() == storage)
				return openedStorage;
		}
		return null;
	}

	//------------------------------------------------------------------------------------------------
	// !
	protected void Action_LootMove()
	{
		if ( !m_pFocusedSlotUI )
			return;
		IEntity pItem = m_pFocusedSlotUI.GetInventoryItemComponent().GetOwner();
		if (m_InventoryManager.CanMoveItem(pItem))
			return;

		m_pCallBack.m_pItem = pItem;
		m_pCallBack.m_pMenu = this;

		// Loot ===>>> Character
		if ( m_pFocusedSlotUI.GetStorageUI() == m_pStorageLootUI )
		{
			m_pCallBack.m_pStorageFrom = m_pStorageLootUI;
			m_pCallBack.m_pStorageTo = m_pFocusedSlotUI.GetStorageUI();

			if ( m_pDisplayedStorage.Type() == EquipedWeaponStorageComponent )			//default view opened, put the item into the first suitable storage
			{
				m_InventoryManager.InsertItem( pItem, null, null, m_pCallBack );
			}
			else
			{
				m_InventoryManager.InsertItem( pItem, m_pStorageBaseUI.GetCurrentNavigationStorage(), m_pStorageLootUI.GetCurrentNavigationStorage(), m_pCallBack ); 			//a storage is already opened, try to move it there
			}
		}
		else
		// Character ===>>> Loot
		{
			m_pCallBack.m_pStorageFrom = m_pFocusedSlotUI.GetStorageUI();
			m_pCallBack.m_pStorageTo = m_pStorageLootUI;
			auto storage = m_pStorageLootUI.GetCurrentNavigationStorage();
			if ( storage )
				m_InventoryManager.InsertItem( pItem, m_pStorageLootUI.GetCurrentNavigationStorage(), m_pDisplayedStorage, m_pCallBack );	// moving into the opened storage in the  vicinity
			else
			{
				//droping it on the ground
				auto pSlot = m_pFocusedSlotUI.GetInventoryItemComponent().GetParentSlot();
				if ( pSlot )
					m_InventoryManager.TryRemoveItemFromInventory(pItem, pSlot.GetStorage(), m_pCallBack);
			}	
		}
		ResetHighlightsOnAvailableStorages();
	}

	//------------------------------------------------------------------------------------------------
	// !
	protected bool IsStorageInsideLBS( BaseInventoryStorageComponent pStorage, ClothNodeStorageComponent pLBSStorage )
	{
		if ( !pLBSStorage )
			return false;
		array<BaseInventoryStorageComponent> aOutStorages = new array<BaseInventoryStorageComponent>();
		pLBSStorage.GetOwnedStorages( aOutStorages, 1, false );
		return aOutStorages.Find(pStorage) != -1;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnItemAddedListener( IEntity item, notnull BaseInventoryStorageComponent storage )
	{
		if ( storage == m_StorageManager.GetLootStorage() )
			return;

		if (m_pCallBack.m_pStorageToPickUp)
		{
			if (item.FindComponent(BaseInventoryStorageComponent) != m_pCallBack.m_pStorageToPickUp)
			{
				int quickSlotIndex = m_StorageManager.GetDefaultQuickSlot(item);
				if (quickSlotIndex > -1)
					m_pQuickSlotStorage.RefreshSlot(quickSlotIndex);
				return;
			}
		}

		if ( storage.Type() == EquipedWeaponStorageComponent )	//we are updating the weapon storage on character
		{			
			if ( m_pWeaponStorage )
				m_pWeaponStorage.Refresh();
		}
		else if ( storage.Type() == WeaponAttachmentsStorageComponent )	// we are updating a weapon ( attachement / magazin inserted )
		{
			//if ( m_pWeaponStorage && m_pWeaponStorage.GetCurrentNavigationStorage() == storage )
			if ( m_pWeaponStorage )
				m_pWeaponStorage.Refresh();
		}
		else
		{
			SCR_InventoryStorageBaseUI storageUI = GetStorageUIByBaseStorageComponent(storage);
			if (storageUI)
				storageUI.Refresh();
		}

		if (m_StorageManager.GetEntityIndexInQuickslots(item) > -1)
			ShowQuickSlotStorage();

		UpdateTotalWeightText();
		
		if (m_pWeaponStorage && item.FindComponent(MagazineComponent))
		{
			if (m_pWeaponStorageComp.Get(0) || m_pWeaponStorageComp.Get(1))
			{
				m_pWeaponStorage.Refresh();
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnItemRemovedListener( IEntity item, notnull BaseInventoryStorageComponent storage )
	{
		if ( !storage )
			return;

		if (m_pCallBack.m_pStorageToDrop)
		{
			if (item.FindComponent(BaseInventoryStorageComponent) != m_pCallBack.m_pStorageToDrop)
			{
				int quickSlotIndex = m_StorageManager.GetEntityIndexInQuickslots(item);
				if (quickSlotIndex > -1)
					m_pCallBack.m_bShouldUpdateQuickSlots = true;
				return;
			}
		}

		SCR_ConsumableItemComponent consumable = SCR_ConsumableItemComponent.Cast(item.FindComponent(SCR_ConsumableItemComponent));
		if (consumable && consumable.GetConsumableEffect() && consumable.GetConsumableEffect().GetDeleteOnUse()) // consumables require different handling
		{
			if (m_wAttachmentStorage && m_pAttachmentStorageUI)
				m_pAttachmentStorageUI.Refresh();
			SCR_InventoryStorageBaseUI storageUI = GetStorageUIByBaseStorageComponent(storage);
			if (storageUI)
				storageUI.Refresh();
			ShowQuickSlotStorage();
		}
		
		if ( storage == m_StorageManager.GetLootStorage() )
			return;

		if (m_StorageManager.GetLastQuickSlotId(item) > -1)
			ShowQuickSlotStorage();

		SCR_InventoryStorageBaseUI storageUI = GetStorageUIByBaseStorageComponent(storage);
		if (storageUI)
			storageUI.Refresh();

		UpdateTotalWeightText();
		
		if (m_pWeaponStorage && item.FindComponent(MagazineComponent))
		{
			if (m_pWeaponStorageComp.Get(0) || m_pWeaponStorageComp.Get(1))
			{
				m_pWeaponStorage.Refresh();
			}
		}		
	}

	//------------------------------------------------------------------------------------------------
	void OnContainerHovered( SCR_InventoryStorageBaseUI pContainer )
	{
		if ( m_bDraggingEnabled )
			pContainer.ShowContainerBorder( m_DraggedSlot != null );
		m_pActiveHoveredStorageUI = pContainer;
		pContainer.SetPagingActive(true);
		NavigationBarUpdate();

		m_bEnablePagingContext = pContainer.GetPageCount() > 1;
	}

	//------------------------------------------------------------------------------------------------
	void HighlightAvailableStorages(SCR_InventorySlotUI itemSlot)
	{
		if (!itemSlot)
			return;
		
		if (!m_pActiveHoveredStorageUI)
			return;
		
		InventoryItemComponent itemComp = itemSlot.GetInventoryItemComponent();
		if (!itemComp)
			return;
		
		IEntity itemEntity = itemComp.GetOwner();
		if (!itemEntity)
			return;
		
		SCR_EntityCatalogManagerComponent entityCatalogManager = SCR_EntityCatalogManagerComponent.GetInstance();
		string smallestContainerPrefab = "";
		
		if (entityCatalogManager)
		{
			SCR_EntityCatalog resourceContainerCatalog = entityCatalogManager.GetEntityCatalogOfType(EEntityCatalogType.SUPPLY_CONTAINER_ITEM); 
			array<SCR_EntityCatalogEntry> entries = {};
			array<SCR_BaseEntityCatalogData> data = {};
			
			resourceContainerCatalog.GetEntityListWithData(SCR_ResourceContainerItemData, entries, data);
			
			if (!entries.IsEmpty())
			{
				int minResourceValue;
				int currentEntryValue;
				
				SCR_ResourceContainerItemData datum = SCR_ResourceContainerItemData.Cast(data[0]);
				
				minResourceValue = datum.GetMaxResourceValue();
				
				if (!minResourceValue)
					minResourceValue = 0;
	
				foreach (int idx, SCR_EntityCatalogEntry entry: entries)
				{
					datum = SCR_ResourceContainerItemData.Cast(data[idx]);
					currentEntryValue = datum.GetMaxResourceValue();
						
					if (currentEntryValue > minResourceValue)
						break;
					
					minResourceValue = currentEntryValue;
					
					if (!entry)
						continue;
					
					smallestContainerPrefab = entry.GetPrefab();
				}
			}
		}
		
		BaseInventoryStorageComponent contStorage;
		array<BaseInventoryStorageComponent> contStorageOwnedStorages = {};
		SCR_EquipmentStorageComponent equipmentStorage;
		IEntity smallestContainerEntity;
		
		if (smallestContainerPrefab != "")
			smallestContainerEntity = GetGame().SpawnEntityPrefabLocal(Resource.Load(smallestContainerPrefab));
		
		foreach (SCR_InventoryStorageBaseUI storageBaseUI: m_aStorages)
		{	
			if (!storageBaseUI)
				continue;
			if (storageBaseUI.Type() == SCR_InventoryStorageLootUI)
				continue;
			if (storageBaseUI == m_pActiveHoveredStorageUI)
				continue;
			
			contStorage = storageBaseUI.GetStorage();	
			if (!contStorage)
				continue;
			
			float itemWeight = itemComp.GetTotalWeight();
			float totalWeightWithInsertedItem;
			
			totalWeightWithInsertedItem = storageBaseUI.GetTotalRoundedUpWeight(contStorage);
			totalWeightWithInsertedItem += Math.Round(itemWeight * 100) * 0.01;
			
			storageBaseUI.UpdateTotalWeight(totalWeightWithInsertedItem);
			
			float totalOccupiedVolumeWithInsertedItem;
			totalOccupiedVolumeWithInsertedItem = storageBaseUI.GetOccupiedVolume(contStorage);
			totalOccupiedVolumeWithInsertedItem += itemComp.GetTotalVolume();	

			contStorageOwnedStorages.Clear();
			contStorage.GetOwnedStorages(contStorageOwnedStorages, 1, false);
			contStorageOwnedStorages.Insert(contStorage);
			
			bool shouldUpdateVolumePercentage = true;
			
			// Check to see if the itemEntity can fit into any equipment Storages so that volume is not updated in those cases.
			BaseInventoryStorageComponent validStorage = m_InventoryManager.FindStorageForInsert(itemEntity, contStorage, EStoragePurpose.PURPOSE_EQUIPMENT_ATTACHMENT);
			if (validStorage)
			{
				shouldUpdateVolumePercentage = false;
			}

			if (!m_InventoryManager.CanInsertItemInActualStorage(itemEntity, contStorage) && itemEntity.FindComponent(SCR_ResourceComponent))
				shouldUpdateVolumePercentage = false;
			
			if (shouldUpdateVolumePercentage)
				storageBaseUI.UpdateVolumePercentage(storageBaseUI.GetOccupiedVolumePercentage(contStorage, totalOccupiedVolumeWithInsertedItem), true);
			
			bool canInsert = m_InventoryManager.CanInsertItemInActualStorage(itemEntity, contStorage);
			
			if (!canInsert && SCR_ResourceComponent.FindResourceComponent(itemEntity))
				canInsert = m_InventoryManager.CanInsertItemInActualStorage(smallestContainerEntity, contStorage);

			if (!canInsert && validStorage)
				canInsert = m_InventoryManager.CanInsertItemInActualStorage(itemEntity, validStorage);
			
			SCR_UniversalInventoryStorageComponent uniContStorage = SCR_UniversalInventoryStorageComponent.Cast(contStorage);
			bool weightCheck = true;
			if (uniContStorage)
				weightCheck = uniContStorage.IsAdditionalWeightOk(itemComp.GetTotalWeight());

			EInvInsertFailReason reason;
			if (!contStorage.PerformDimensionValidation(itemEntity))
				reason |= EInvInsertFailReason.SIZE;

			if (!weightCheck)
				reason |= EInvInsertFailReason.WEIGHT;

			if (!contStorage.PerformVolumeValidation(itemEntity))
				reason |= EInvInsertFailReason.CAPACITY;

			if (canInsert)
				storageBaseUI.SetStorageAsHighlighted(true);
			else
				storageBaseUI.SetStorageAsHighlighted(false, reason);
		}
		
		delete smallestContainerEntity;	
	}
	
	//------------------------------------------------------------------------------------------------
	void ResetHighlightsOnAvailableStorages()
	{
		foreach (SCR_InventoryStorageBaseUI storageBaseUI: m_aStorages)
		{
			if (!storageBaseUI)
				continue;
			
			storageBaseUI.UpdateVolumePercentage(storageBaseUI.GetOccupiedVolumePercentage(storageBaseUI.GetStorage()));
			storageBaseUI.UpdateTotalWeight(storageBaseUI.GetTotalRoundedUpWeight(storageBaseUI.GetStorage()));
			storageBaseUI.SetStorageAsHighlighted(true);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void OnContainerLeft( SCR_InventoryStorageBaseUI pContainer )
	{		
		pContainer.ShowContainerBorder( false );
		m_pActiveHoveredStorageUI = null;
		pContainer.SetPagingActive(false);
		NavigationBarUpdate();
	}
	
	//------------------------------------------------------------------------------------------------
	void OnContainerFocused(SCR_InventoryStorageBaseUI pContainer)
	{
		pContainer.ShowContainerBorder(true);
		SetActiveStorage(pContainer);
		m_pActiveStorageUI = pContainer;
		NavigationBarUpdate();
	}

	//------------------------------------------------------------------------------------------------
	void OnContainerFocusLeft(SCR_InventoryStorageBaseUI pContainer)
	{
		pContainer.ShowContainerBorder(false);
		SetActiveStorage(pContainer);
		m_pActiveStorageUI = null;
		NavigationBarUpdate();
	}

	//------------------------------------------------------------------------------------------------
	SCR_CharacterControllerComponent GetCharacterController()
	{
		return m_CharController;
	}

	//------------------------------------------------------------------------------------------------
	//! 
	protected void RefreshUISlotStorages()
	{
		foreach ( SCR_InventorySlotUI pStorage : m_aShownStorages )
		{
			if ( pStorage )
				pStorage.Refresh();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateTotalWeightText()
	{
		if (!m_wTotalWeightText)
			return;
	
		float weight = Math.Round(GetTotalWeight() * 100) * 0.01;
					
		if (weight <= 0 )
		 weight = 0;
					
		m_wTotalWeightText.SetTextFormat("#AR-ValueUnit_Short_Kilograms", weight);
	}

	protected void InitQuickSlots()
	{
		ShowQuickSlotStorage();

		int quickSlotCount = m_StorageManager.GetQuickSlotItems().Count();
		for (int slotId = 1; slotId <= quickSlotCount; ++slotId)
		{
			GetGame().GetInputManager().AddActionListener("InventoryQuickSlot" + slotId, EActionTrigger.PRESSED, Action_SelectQuickSlot);
		}

	}

	//------------------------------------------------------------------------------------------------
	void ShowQuickSlotStorage()
	{
		if( m_wQuickSlotStorage )
		{
			m_wQuickSlotStorage.RemoveHandler( m_wQuickSlotStorage.FindHandler( SCR_InventoryStorageQuickSlotsUI ) );	//remove the handler from the widget
			m_wQuickSlotStorage.RemoveFromHierarchy();
		}
		
		Widget parent = m_widget.FindAnyWidget( "QuickSlots" );
		m_wQuickSlotStorage =  GetGame().GetWorkspace().CreateWidgets( "{A1E61EF091EAC47D}UI/layouts/Menus/Inventory/InventoryQuickSlotsGrid.layout", parent );
				
		if( !m_wQuickSlotStorage )
			return;
		
		OverlaySlot.SetVerticalAlign( m_wQuickSlotStorage, LayoutVerticalAlign.Bottom );
		OverlaySlot.SetHorizontalAlign(m_wQuickSlotStorage, LayoutHorizontalAlign.Left);
					
		m_wQuickSlotStorage.AddHandler( new SCR_InventoryStorageQuickSlotsUI( null, null, this ) );
		m_pQuickSlotStorage = SCR_InventoryStorageQuickSlotsUI.Cast( m_wQuickSlotStorage.FindHandler( SCR_InventoryStorageQuickSlotsUI ) );
	}
	
	//------------------------------------------------------------------------------------------------
	// !
	void Action_SelectQuickSlot()
	{
		if ( !GetGame().GetInputManager() )
			return;
		int iSlot = -1;

		int quickSlotCount = m_StorageManager.GetQuickSlotItems().Count();
		for (int id = 1; id <= quickSlotCount; ++id)
		{
			if (GetGame().GetInputManager().GetActionTriggered("InventoryQuickSlot" + id))
				iSlot = id;
		}

		if ( iSlot != -1 )
			SetItemToQuickSlot( iSlot );
	}

	//------------------------------------------------------------------------------------------------
	// !
	protected void RemoveItemFromQuickSlotDrop()
	{
		if ( !m_pSelectedSlotUI )
			return;
		InventoryItemComponent pInventoryComponent = m_pSelectedSlotUI.GetInventoryItemComponent();
		if ( !pInventoryComponent )
			return;
		m_StorageManager.StoreItemToQuickSlot( pInventoryComponent.GetOwner(), m_pSelectedSlotUI.GetSlotIndex() );
		ShowQuickSlotStorage();
	}

	//------------------------------------------------------------------------------------------------
	// !
	protected void SetItemToQuickSlot( int iSlotIndex, SCR_InventorySlotUI slot = null )
	{
		if (!slot)
			slot = m_pFocusedSlotUI;

		if (!slot)
			return;

		if (iSlotIndex < WEAPON_SLOTS_COUNT)
			return;

		SCR_InventoryStorageBaseUI storageUI = slot.GetStorageUI();
		if (storageUI 
		&& (storageUI.IsInherited(SCR_InventoryStorageLootUI) || storageUI.IsInherited(SCR_InventoryOpenedStorageUI))
		|| IsStorageArsenal(storageUI.GetCurrentNavigationStorage()))
			return;	//we don't want to take anything from vicinity to the quickslot

		if (slot.Type() == SCR_InventorySlotQuickSlotUI)
			return;

		InventoryItemComponent pInventoryComponent = slot.GetInventoryItemComponent();
		if (!pInventoryComponent)
			return;

		IEntity pItem = pInventoryComponent.GetOwner();
		if (!pItem)
			return;

		if (m_pWeaponStorageComp.Contains(pItem))
			return;
		
		if (!CanItemFitIntoQuickSlots(pInventoryComponent))
			return; //so far only items with one line are supported ( issue on the UI side )

		m_StorageManager.StoreItemToQuickSlot( pItem, --iSlotIndex );
		ShowQuickSlotStorage();
	}

	//------------------------------------------------------------------------------------------------
	//! Checks if item's slot configuration matches size requirements for a quick slot
	//! \param[in] iic InventoryItemComponent of the validated item
	//! returns true if item slot size is either 1x1 or 2x1, otherwise false
	protected bool CanItemFitIntoQuickSlots(notnull InventoryItemComponent iic)
	{
		SCR_ItemAttributeCollection itemAttributes = SCR_ItemAttributeCollection.Cast(iic.GetAttributes());
		if (!itemAttributes)
			return true;

		ESlotSize size = itemAttributes.GetQuickSlotItemSize();
		return size == ESlotSize.SLOT_1x1 || size == ESlotSize.SLOT_2x1;
	}

	//------------------------------------------------------------------------------------------------
	// !
	protected void SetItemToQuickSlotDrop()
	{
		if (!m_pFocusedSlotUI)
			return;

		if (!m_pSelectedSlotUI)
			return;

		if (!m_pFocusedSlotUI.IsCompatible())
			return;

		SCR_InventoryStorageBaseUI selectedSlotStorage = m_pSelectedSlotUI.GetStorageUI();
		if (selectedSlotStorage && 
			(selectedSlotStorage.IsInherited(SCR_InventoryStorageLootUI) || selectedSlotStorage.IsInherited(SCR_InventoryOpenedStorageUI)) ||
			IsStorageArsenal(selectedSlotStorage.GetCurrentNavigationStorage()))
		{
			return;	//we don't want to take anything from vicinity to the quickslot
		}

		IEntity pOriginalEntity;
		if ( ( SCR_InventorySlotQuickSlotUI.Cast( m_pSelectedSlotUI ) ) && ( SCR_InventorySlotQuickSlotUI.Cast( m_pFocusedSlotUI ) ) ) // swapping
		{
			InventoryItemComponent pComp = m_pFocusedSlotUI.GetInventoryItemComponent();
			if ( pComp )
			{
				pOriginalEntity = pComp.GetOwner();
			}
		}

		InventoryItemComponent inventoryComponent = m_pSelectedSlotUI.GetInventoryItemComponent();
		if (!inventoryComponent)
			return;
		
		IEntity item = inventoryComponent.GetOwner();
		if (!item || item.IsDeleted())
			return;

		if (!CanItemFitIntoQuickSlots(inventoryComponent))
		{
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INV_DROP_ERROR);
			return;
		}

		int iSlotIndex = m_pFocusedSlotUI.GetSlotIndex();
		if (iSlotIndex < WEAPON_SLOTS_COUNT || m_pWeaponStorageComp.Contains(item))
		{
			ShowQuickSlotStorage();
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INV_DROP_ERROR);
			if (m_pSelectedSlotUI.GetButtonWidget())
				FocusWidget(m_pSelectedSlotUI.GetButtonWidget());
			else
				FocusWidget(m_pFocusedSlotUI.GetButtonWidget());

			DeselectSlot();
			NavigationBarUpdate();
			return;
		}
		
		m_StorageManager.StoreItemToQuickSlot(item, iSlotIndex);
		if (pOriginalEntity)
			m_StorageManager.StoreItemToQuickSlot(pOriginalEntity, m_pSelectedSlotUI.GetSlotIndex());

		ShowQuickSlotStorage();
		
		SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INV_HOTKEY_CONFIRM);

		SetStorageSwitchMode(false);
		Action_DeselectItem();
		FocusOnSlotInStorage(m_pQuickSlotStorage, iSlotIndex);
	}

	//------------------------------------------------------------------------------------------------
	protected void Action_SwapItems(SCR_InventorySlotUI item1, SCR_InventorySlotUI item2)
	{
		if (!item1 || !item2)
			return;

		IEntity from = item1.GetInventoryItemComponent().GetOwner();
		IEntity to = item2.GetInventoryItemComponent().GetOwner();

		if (!(m_InventoryManager.CanMoveItem(from) && m_InventoryManager.CanMoveItem(to)))
			return;
		
		m_pCallBack.m_pStorageFrom = m_pSelectedSlotUI.GetStorageUI();
		m_pCallBack.m_pStorageTo = m_pFocusedSlotUI.GetStorageUI();
		m_pCallBack.m_pItem = from;
		m_pCallBack.m_pMenu = this;
		m_pCallBack.m_pStorageToFocus = m_pCallBack.m_pStorageFrom;
		m_pCallBack.m_sItemToFocus = m_pSelectedSlotUI.GetItemResource();
		m_pCallBack.m_iSlotToFocus = m_pSelectedSlotUI.GetStorageUI().GetSlotId(m_pSelectedSlotUI);
		
		//if (m_InventoryManager.CanSwapItemStorages(from, to))
		m_InventoryManager.TrySwapItemStorages(from, to, m_pCallBack); 
	}

	//------------------------------------------------------------------------------------------------
	protected void Action_MoveItemToStorage(SCR_InventoryStorageBaseUI toStorage = null)
	{
		if (!toStorage)
			toStorage = m_pFocusedSlotUI.GetStorageUI();

		if (!toStorage || !m_pSelectedSlotUI)
			return;

		MoveItem(toStorage);
		ResetHighlightsOnAvailableStorages();
	}

	//------------------------------------------------------------------------------------------------
	bool IsWeaponEquipped(IEntity weapon)
	{
		if (!weapon)
			return false;

		BaseWeaponManagerComponent weaponMan = BaseWeaponManagerComponent.Cast(m_Player.FindComponent(BaseWeaponManagerComponent));
		if (!weaponMan)
			return false;

		BaseWeaponComponent weaponComp = BaseWeaponComponent.Cast(weapon.FindComponent(BaseWeaponComponent));
		if (!weaponComp)
			return false;

		return weaponComp == weaponMan.GetCurrentWeapon();
	}

	//------------------------------------------------------------------------------------------------
	bool CanAddAttachment(SCR_InventorySlotUI itemSlot, SCR_InventorySlotUI targetSlot)
	{
		InventoryItemComponent itemComp = itemSlot.GetInventoryItemComponent();
		if (!itemComp || !itemComp.GetOwner())
			return false;
		
		InventoryItemComponent targetItemComp = targetSlot.GetInventoryItemComponent();
		if (!targetItemComp || !targetItemComp.GetOwner())
			return false;

		ItemAttributeCollection itemAttributes = itemComp.GetAttributes();
		if (!itemAttributes)
			return false;

		AttachmentAttributes itemAttribute = AttachmentAttributes.Cast(itemAttributes.FindAttribute(AttachmentAttributes));

		if (itemAttribute && itemAttribute.GetAttachmentType())
		{
			BaseInventoryStorageComponent pItemToReplaceStorage = BaseInventoryStorageComponent.Cast(targetItemComp.GetOwner().FindComponent(BaseInventoryStorageComponent));
			
			if (!pItemToReplaceStorage)
				return false;

			return m_InventoryManager.CanInsertItemInStorage(itemComp.GetOwner(), pItemToReplaceStorage) 
				|| m_InventoryManager.CanMoveItemToStorage(itemComp.GetOwner(), pItemToReplaceStorage);
		}

		return false;
	}

	void OnAttachmentSpinboxFocused()
	{
		NavigationBarUpdate();	
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsUsingGamepad()
	{
		return m_bIsUsingGamepad;
	}

	//------------------------------------------------------------------------------------------------
	void OnInputDeviceIsGamepad(bool isGamepad)
	{
		bool oldState = m_bIsUsingGamepad;
		bool switchedToMnK = m_bIsUsingGamepad && m_bIsUsingGamepad != isGamepad;
		m_bIsUsingGamepad = isGamepad;
		SetStorageSwitchMode(m_bIsUsingGamepad, false);
		SetAttachmentSpinBoxActive(isGamepad);
		if (oldState != isGamepad)
		{
			if (oldState)
				m_pNavigationBar.SetButtonActionName(BUTTON_BACK, GAMEPAD_HINT_CLOSE); // force reset the lable of this hint as for M&K there is only one mode for this button
			else if (m_pSelectedSlotUI)
				DeselectSlot() // as there is nothing that cleares the selected slot which was selected when using M&K
		}

		NavigationBarUpdate();
	}

	//------------------------------------------------------------------------------------------------	
	SCR_InventorySlotUI GetDraggedSlot()
	{
		return m_DraggedSlot;
	}

	//------------------------------------------------------------------------------------------------
	bool IsDraggingEnabled()
	{
		return m_bDraggingEnabled;
	}

	//------------------------------------------------------------------------------------------------	
	static SCR_InventoryMenuUI GetInventoryMenu()
	{
		return SCR_InventoryMenuUI.Cast(GetGame().GetMenuManager().FindMenuByPreset(ChimeraMenuPreset.Inventory20Menu));
	}

	//------------------------------------------------------------------------------------------------
	void SCR_InventoryMenuUI()
	{
		m_SupplyCostUIInfo = SCR_SupplyCostItemHintUIInfo.Cast(SCR_BaseContainerTools.CreateInstanceFromPrefab(m_sSupplyCostUIInfoPrefab));
		m_SupplyRefundUIInfo = SCR_SupplyRefundItemHintUIInfo.Cast(SCR_BaseContainerTools.CreateInstanceFromPrefab(m_sSupplyRefundUIInfoPrefab));
		m_NonrefundableUIInfo = SCR_NonrefundableItemHintUIInfo.Cast(SCR_BaseContainerTools.CreateInstanceFromPrefab(m_sNonrefundableUIInfoPrefab));
		m_ArsenalItemRankUIInfo = SCR_ArsenalItemRankHintUIInfo.Cast(SCR_BaseContainerTools.CreateInstanceFromPrefab(ITEM_RANK_PREFAB));
		m_ValuableIntelUIInfo = SCR_ValuableIntelItemHintUIInfo.Cast(SCR_BaseContainerTools.CreateInstanceFromPrefab(VALUABLE_INTEL_UIINFO_PREFAB));
		m_PlayerSupplyAllocationUIInfo = SCR_PlayerSupplyAllocationItemHintUIInfo.Cast(SCR_BaseContainerTools.CreateInstanceFromPrefab(PLAYER_SUPPLY_ALLOCATION_UIINFO_PREFAB));
		
		//~ Create generic hint info for items with factions
		FactionManager factionManager = GetGame().GetFactionManager();
		if (factionManager)
		{
			array<Faction> factions = {};
			factionManager.GetFactionsList(factions);
			
			SCR_FactionOutfitItemHintUIInfo factionHintInfo;
			foreach (Faction faction : factions)
			{
				factionHintInfo = SCR_FactionOutfitItemHintUIInfo.Cast(SCR_BaseContainerTools.CreateInstanceFromPrefab(ITEM_FACTION_INFO_CONFIG));
				if (!factionHintInfo)
					break;
				
				m_mItemFactionUIInfos.Insert(faction.GetFactionKey(), factionHintInfo);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_InventoryMenuUI()
	{
		// reset view distance back to value defined in video settings
		BaseContainer vs = GetGame().GetGameUserSettings().GetModule("SCR_VideoSettings");
		if (vs)
		{
			int dist;
			vs.Get("m_iViewDistance", dist);
			GetGame().SetViewDistance(dist);
		}
	}
	
	#else
	protected float GetTotalLoadWeight();
	SCR_InventoryStorageManagerComponent GetInventoryStorageManager();
	bool GetCanInteract();
	override void OnMenuOpen();
	protected void Init();
	void ShowStorage();
	void ShowStorage( BaseInventoryStorageComponent storage );
	protected void ShowStoragesList();
	protected void ShowEquipedWeaponStorage();
	void ShowItemInfo( string sName = "", string sDescr = "", float sWeight = 0.0 );
	void HideItemInfo();
	void SetSlotFocused( SCR_InventorySlotUI pFocusedSlot );
	SCR_InventoryInspectionUI GetInspectionScreen();
	void InspectItem(SCR_InventorySlotUI itemSlot);
	void FilterOutStorages();
	protected void Action_CloseInventory();
	protected void Action_SelectItem();
	protected void Action_EquipItem();
	protected void Action_Drop();
	protected void Action_Inspect();
	protected void OnItemAddedListener( IEntity item, BaseInventoryStorageComponent storage );
	protected void OnItemRemovedListener( IEntity item, BaseInventoryStorageComponent storage );
	override bool OnClick( Widget w, int x, int y, int button );
	void FilterOutStorages( false );
	void MoveItem();
	void SCR_InventoryMenuUI();
	#endif
}
