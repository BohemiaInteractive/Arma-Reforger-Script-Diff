//------------------------------------------------------------------------------------------------
//! To prevent fetching data for each condition multiple times within a single frame,
//! we use this container instead to pass around data where needed.
//! TODO: Possibly reuse for other HUD needs to save some perf?
class SCR_AvailableActionsConditionData
{
	protected ChimeraCharacter m_Character;
	protected CharacterControllerComponent m_CharacterController;
	protected SCR_InventoryStorageManagerComponent m_StorageManager;

	//! Whether data is valid at current moment, FetchData has to be called prior to this to have it updated
	protected bool m_bIsValid;

	//! Current character stance
	protected ECharacterStance m_eCharacterStance;
	// Type of current compartment
	protected ECompartmentType m_eCompartmentType;
	//! Is character freelooking?
	protected bool m_bIsCharacterFreelook;
	//! Is character freelooking forced?
	protected bool m_bIsCharacterFreelookForced;
	//! Is character freelooking toggled?
	protected bool m_bIsCharacterFreelookToggled;
	//! Is character aiming down sights?
	protected bool m_bIsCharacterADS;
	//! Is character seated in a vehicle?
	protected bool m_bIsCharacterInVehicle;
	//! Is character getting in?
	protected bool m_bIsCharacterGettingIn;
	//! Is character getting out?
	protected bool m_bIsCharacterGettingOut;
	//! Is character sprinting?
	protected bool m_bIsCharacterSprinting;
	//! Is character swimming?
	protected bool m_bIsCharacterSwimming;
	//! Is character falling? (airborne)
	protected bool m_bIsCharacterFalling;
	//! Is character reloading
	protected bool m_bIsCharacterReloading;
	//! Can character get out of vehicle?
	protected bool m_bCanCharacterGetOutVehicle;
	//! Is character weapon raised
	protected bool m_bIsCharacterWeaponRaised;
	//! Can character fire with weapon
	protected bool m_bCanCharacterFireWeapon;
	//! Is character using an item at the moment
	protected bool m_bIsCharacterUsingItem;
	//! Is character bleeding
	protected bool m_bIsCharacterBleeding;
	//! Is inventory open
	protected bool m_bIsInventoryOpen;
	//! Is  quick slot bar available
	protected bool m_bIsQuickSlotAvailable;
	//! Is weapon quick bar shown
	protected bool m_bIsQuickSlotShown;
	//! Is gadget selection mode active
	protected bool m_bIsGadgetSelection;
	//! Is weapon manipulation mode active
	protected bool m_bIsWeaponManipulation;
	//! Is character in focus mode
	protected float m_fFocusMode;
	//! How long is character bleeding
	protected float m_fBleedingTime;
	//! How long is character sprinting
	protected float m_fSprintingTime;
	//! Number of additional magazines character has for current weapon
	protected int m_iAdditionalMagazines;
	//! Number of medial items
	protected int m_iMedicalItemCount;
	//! Number of medical items in quick slots
	protected int m_iMedicalItemCountInQuickSlots;
	//! Currently equipped item entity (if any)
	protected IEntity m_CurrentItemEntity;
	//! Current character weapon entity (if any)
	protected IEntity m_CurrentWeaponEntity;
	//! Current weapon
	protected BaseWeaponComponent m_CurrentWeapon;
	//! Current weapon muzzle
	protected BaseMuzzleComponent m_CurrentMuzzle;
	//! Current weapon magazine
	protected BaseMagazineComponent m_CurrentMagazine;

	protected string m_sCurrentlyPlacedItemName;
	protected IEntity m_LastPlacedItem;

	//! Current vehicle
	protected IEntity m_CurrentVehicle;
	//! How long is player using turbo as driver of vehicle
	protected float m_fTurboTime;
	//! Current vehicle controller
	protected BaseControllerComponent m_CurrentVehicleController;
	//! Current vehicle signals
	protected SignalsManagerComponent m_CurrentVehicleSignals;
	//! Current vehicle weapon
	protected BaseWeaponComponent m_CurrentVehicleWeapon;
	//! Current turret controller
	protected TurretControllerComponent m_CurrentTurretController;
	
	// Character health state
	protected SCR_CharacterDamageManagerComponent m_CharacterDamageComponent;
	protected bool m_bIsCharacterConscious;
	protected bool m_bHasBleedingLimbs;
	protected bool m_bHasTourniquetLimb;

	// Von variables ----------------------
	protected SCR_VoNComponent m_VON;
	protected SCR_VONController m_VONController;

	//! Is character using radio
	protected bool m_bCharacterIsUsingRadio;
	//! Count of available radios
	protected bool m_bCharacterRadiosCount;
	bool m_bIsRadioLongRange;
	bool m_bOwnsLongRangeRadio;

	//! Inventory fetching limitation
	protected bool m_bCanFetchInventory = true;
	
	//! Map vars
	protected SCR_MapEntity m_MapEntity;
	bool m_bCanRotateMapElement;
	bool m_bCanActivateMapElement;
	bool m_bCanDragMapElement;
	bool m_bCanResetElementRotation;

	//------------------------------------------------------------------------------------------------
	private void OnItemAddedListener(IEntity item, BaseInventoryStorageComponent storage)
	{
		m_bCanFetchInventory = true;
	}

	private void OnItemRemovedListener(IEntity item, BaseInventoryStorageComponent storage)
	{
		m_bCanFetchInventory = true;
	}

	private void OnInventoryOpen(bool open)
	{
		if (open)
		{
			SCR_InventoryAvailableCondition.IncrementCounter();
		}
		m_bIsInventoryOpen = open;
	}

	private void OnQuickSlotOpen(bool open)
	{
		if (open)
		{
			SCR_WeaponQuickSlotAvailableCondition.IncrementCounter();
		}
		m_bIsQuickSlotAvailable = open;
		m_bIsQuickSlotShown = open;
	}

	//------------------------------------------------------------------------------------------------
	//! Clears all variables by setting them to their default state
	//! Also invalidates this data
	private void Clear()
	{
		m_eCharacterStance = ECharacterStance.STAND;
		m_eCompartmentType = ECompartmentType.PILOT;
		m_bIsCharacterFreelook = false;
		m_bIsCharacterFreelookForced = false;
		m_bIsCharacterFreelookToggled = false;
		m_bIsCharacterADS = false;
		m_bIsCharacterInVehicle = false;
		m_bIsCharacterGettingIn = false;
		m_bIsCharacterGettingOut = false;
		m_bIsCharacterSwimming = false;
		m_bIsCharacterSprinting = false;
		m_bIsCharacterFalling = false;
		m_bIsCharacterReloading = false;
		m_bIsCharacterWeaponRaised = false;
		m_bCanCharacterFireWeapon = false;
		m_bIsCharacterUsingItem = false;
		m_bIsGadgetSelection = false;
		m_bIsWeaponManipulation = false;
		m_fFocusMode = 0.0;
		//m_iAdditionalMagazines = 0;
		m_CurrentItemEntity = null;
		m_CurrentWeaponEntity = null;
		m_CurrentWeapon = null;
		m_CurrentMuzzle = null;
		m_CurrentMagazine = null;
		m_CurrentVehicleWeapon = null;
		m_CurrentTurretController = null;
		
		// Health
		m_CharacterDamageComponent = null;
		m_bIsCharacterConscious = false;
		m_bHasBleedingLimbs = false;
		m_bHasTourniquetLimb = false;

		m_bIsValid = false;
	}

	/* Gadgets magic */
	protected SCR_GadgetManagerComponent m_GadgetManager;
	
	//------------------------------------------------------------------------------------------------
	// Getters
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	ChimeraCharacter GetCharacter()
	{
		return m_Character;
	}

	//------------------------------------------------------------------------------------------------
	CharacterControllerComponent GetCharacterController()
	{
		return m_CharacterController;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns held gadget or null if none
	IEntity GetHeldGadget()
	{
		if (m_GadgetManager)
			return m_GadgetManager.GetHeldGadget();
		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns held gadget component or null if none
	SCR_GadgetComponent GetHeldGadgetComponent()
	{
		if (m_GadgetManager)
			return m_GadgetManager.GetHeldGadgetComponent();
		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Return held gadget being aimed with
	bool GetGadgetRaised()
	{
		if (m_GadgetManager)
			return m_GadgetManager.GetIsGadgetADS();
		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns gadget by type or null if none
	IEntity GetGadget(EGadgetType type)
	{
		if (m_GadgetManager)
			return m_GadgetManager.GetGadgetByType(type);
		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns current magazine or null if none
	BaseMagazineComponent GetCurrentMagazine()
	{
		return m_CurrentMagazine;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns current weapon or none if null
	BaseMuzzleComponent GetCurrentMuzzle()
	{
		return m_CurrentMuzzle;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns current weapon or none if null
	BaseWeaponComponent GetCurrentWeapon()
	{
		return m_CurrentWeapon;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns current character stance
	ECharacterStance GetCharacterStance()
	{
		return m_eCharacterStance;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns current compartment type
	ECompartmentType GetCompartmentType()
	{
		return m_eCompartmentType;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns <0,1> of focus mode amount; 0 = none, 1 = full focus
	float GetFocusModeAmount()
	{
		return m_fFocusMode;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns for how long is character bleeding
	float GetCharacterBleedingTime()
	{
		return m_fBleedingTime;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns for how long is character bleeding
	float GetCharacterSprintingTime()
	{
		return m_fSprintingTime;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns count of additional available magazines for current weapon
	//! TODO: MuzzleInMag :)))))
	int GetAdditionalMagazinesCount()
	{
		return m_iAdditionalMagazines;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns count of medial items
	int GetMedicalItemCount()
	{
		return m_iMedicalItemCount;
	}

	//------------------------------------------------------------------------------------------------
	int GetMedicalItemCountInQuickSlots()
	{
		return m_iMedicalItemCountInQuickSlots;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns whether character is in a vehicle or not
	bool GetIsCharacterInVehicle()
	{
		return m_bIsCharacterInVehicle;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns whether character is getting in
	bool GetIsCharacterGettingIn()
	{
		return m_bIsCharacterGettingIn;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns whether character is getting out
	bool GetIsCharacterGettingOut()
	{
		return m_bIsCharacterGettingOut;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns whether character is sprinting
	bool GetIsCharacterSprinting()
	{
		return m_bIsCharacterSprinting;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns whether character is swimming or not
	bool GetIsCharacterSwimming()
	{
		return m_bIsCharacterSwimming;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns whether character is falling (airborne) or not
	bool GetIsCharacterFalling()
	{
		return m_bIsCharacterFalling;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns whether character is reloading or not
	bool GetIsCharacterReloading()
	{
		return m_bIsCharacterReloading;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns whether character is freelooking
	bool GetIsCharacterFreelook()
	{
		return m_bIsCharacterFreelook;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns whether character is freelooking forced
	bool GetIsCharacterFreelookForced()
	{
		return m_bIsCharacterFreelookForced;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns whether character is freelooking toggled
	bool GetIsCharacterFreelookToggled()
	{
		return m_bIsCharacterFreelookToggled;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns whether character is aiming down sights or not
	bool GetIsCharacterADS()
	{
		return m_bIsCharacterADS;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns if character has raised weapon
	bool GetIsCharacterWeaponRaised()
	{
		return m_bIsCharacterWeaponRaised;
	}

	//------------------------------------------------------------------------------------------------
	//! Return if player can fire weapon
	bool GetCanCharacterFireWeapon() { return m_bCanCharacterFireWeapon; }

	//------------------------------------------------------------------------------------------------
	//! Returns if character is currently using an item
	bool GetIsCharacterUsingItem()
	{
		return m_bIsCharacterUsingItem;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns whether character can get out of vehicle (if in any)
	bool GetCanCharacterGetOutVehicle()
	{
		return m_bCanCharacterGetOutVehicle;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns if character is bleeding
	bool GetIsCharacterBleeding()
	{
		return m_bIsCharacterBleeding;
	}

	//------------------------------------------------------------------------------------------------
	bool GetIsCharacterUsingRadio()
	{
		return m_bCharacterIsUsingRadio;
	}

	//------------------------------------------------------------------------------------------------
	int GetCharacterRadiosCount()
	{
		return m_bCharacterRadiosCount;
	}

	//------------------------------------------------------------------------------------------------
	bool IsInventoryOpen()
	{
		return m_bIsInventoryOpen;
	}

	//------------------------------------------------------------------------------------------------
	bool IsQuickSlotAvailable()
	{
		return m_bIsQuickSlotAvailable;
	}

	//------------------------------------------------------------------------------------------------
	bool IsQuickSlotShown()
	{
		return m_bIsQuickSlotShown;
	}

	//------------------------------------------------------------------------------------------------
	bool IsGadgetSelection()
	{
		return m_bIsGadgetSelection;
	}

	//------------------------------------------------------------------------------------------------
	bool IsWeaponManipulation()
	{
		return m_bIsWeaponManipulation;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns current character item or null if none
	IEntity GetCurrentItemEntity()
	{
		return m_CurrentItemEntity;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns current character weapon or null if none
	IEntity GetCurrentWeaponEntity()
	{
		return m_CurrentWeaponEntity;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns current character weapon or null if none
	string GetCurrentlyPlacedItemName()
	{
		return m_sCurrentlyPlacedItemName;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns current controlled vehicle
	IEntity GetCurrentVehicle()
	{
		return m_CurrentVehicle;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns current controlled vehicle's controller
	BaseControllerComponent GetCurrentVehicleController()
	{
		return m_CurrentVehicleController;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns current character weapon or null if none
	SignalsManagerComponent GetCurrentVehicleSignals()
	{
		return m_CurrentVehicleSignals;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns current controlled vehicle weapon or null if none
	BaseWeaponComponent GetCurrentVehicleWeapon()
	{
		return m_CurrentVehicleWeapon;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns controller of currently controlled turret
	TurretControllerComponent GetCurrentTurretController()
	{
		return m_CurrentTurretController;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns for how long is vehicle using turbo
	float GetCurrentVehicleTurboTime()
	{
		return m_fTurboTime;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns the actual animation controller
	CharacterAnimationComponent GetAnimationComponent()
	{
		if (m_Character)
			return m_Character.GetAnimationComponent();

		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_CharacterDamageManagerComponent GetCharacterDamageComponent()
	{
		return m_CharacterDamageComponent;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetIsCharacterConscious()
	{
		return m_bIsCharacterConscious;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetHasBleedingLimbs()
	{
		return m_bHasBleedingLimbs;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetHasTourniquetLimb()
	{
		return m_bHasTourniquetLimb;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsValid()
	{
		return m_bIsValid;
	}
	
	//------------------------------------------------------------------------------------------------
	// Fetch data
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	protected void FetchCurrentlyPlacedItemName()
	{
		SCR_CampaignBuildingGadgetToolComponent gadgetComponent = SCR_CampaignBuildingGadgetToolComponent.Cast(m_GadgetManager.GetHeldGadgetComponent());
		if (!gadgetComponent)
			return;

		SCR_MultiPartDeployableItemComponent deployableItemComp = gadgetComponent.GetCurrentlyHandledComponent();
		if (!deployableItemComp)
			return;

		IEntity item = deployableItemComp.GetOwner();
		if (!item)
			return;

		if (item != m_LastPlacedItem)
		{
			InventoryItemComponent iic = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
			if (!iic)
				return;

			UIInfo info = iic.GetUIInfo();
			if (!info)
				return;

			m_sCurrentlyPlacedItemName = info.GetName();
			m_LastPlacedItem = item;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Fetches data from the provided entity
	//! Sets the validity of the data which can be received via IsValid()
	void FetchData(IEntity controlledEntity, float timeSlice)
	{
		// Detect when character changes
		ChimeraCharacter character = ChimeraCharacter.Cast(controlledEntity);
		if (character != m_Character)
		{
			m_Character = character;

			if (character)
			{
				m_CharacterController = character.GetCharacterController();
				m_GadgetManager = SCR_GadgetManagerComponent.GetGadgetManager(character);
				m_VON = SCR_VoNComponent.Cast(character.FindComponent(SCR_VoNComponent));
			}
		}

		if (m_GadgetManager)
			FetchCurrentlyPlacedItemName();

		if (!m_Character)
		{
			m_CharacterController = null;
			m_GadgetManager = null;
			m_VON = null;

			m_bIsValid = false;
			return;
		}

		if (!m_CharacterController)
		{
			m_bIsValid = false;
			return;
		}

		// Add inventory listeners for efficient magazine/grenade count updates
		SCR_InventoryStorageManagerComponent storageManager = SCR_InventoryStorageManagerComponent.Cast(m_CharacterController.GetInventoryStorageManager());
		if (storageManager != m_StorageManager)
		{
			if (m_StorageManager)
			{
				m_StorageManager.m_OnItemAddedInvoker.Remove(OnItemAddedListener);
				m_StorageManager.m_OnItemRemovedInvoker.Remove(OnItemRemovedListener);
				m_StorageManager.m_OnInventoryOpenInvoker.Remove(OnInventoryOpen);
				m_StorageManager.m_OnQuickBarOpenInvoker.Remove(OnQuickSlotOpen);
			}

			m_StorageManager = storageManager;

			if (m_StorageManager)
			{
				m_StorageManager.m_OnItemAddedInvoker.Insert(OnItemAddedListener);
				m_StorageManager.m_OnItemRemovedInvoker.Insert(OnItemRemovedListener);
				m_StorageManager.m_OnInventoryOpenInvoker.Insert(OnInventoryOpen);
				m_StorageManager.m_OnQuickBarOpenInvoker.Insert(OnQuickSlotOpen);
			}
		}

		// Invalidates and clears any data prior to following collection
		Clear();

		// Current character stance
		m_eCharacterStance = m_CharacterController.GetStance();
		// Is character freelooking?
		m_bIsCharacterFreelook = m_CharacterController.IsFreeLookEnabled();
		// Is character forced to freelook?
		m_bIsCharacterFreelookForced = m_CharacterController.IsFreeLookForced() || m_CharacterController.IsFreeLookEnforced();
		// Is character having freelook toggled?
		m_bIsCharacterFreelookToggled = m_CharacterController.GetFreeLookInput() && !GetGame().GetInputManager().GetActionValue("Freelook") >= 0.5;
		// Is character ADS?
		m_bIsCharacterADS = m_CharacterController.IsWeaponADS();
		// Is character sprinting?
		m_bIsCharacterSprinting = m_CharacterController.IsSprinting();
		// Is character swimming?
		m_bIsCharacterSwimming = m_CharacterController.IsSwimming();
		// Is character falling? (is airborne?)
		m_bIsCharacterFalling = m_CharacterController.IsFalling();
		// Is character in vehicle?
		m_bIsCharacterInVehicle = m_Character.IsInVehicle();
		// Can character get out?
		m_bCanCharacterGetOutVehicle = m_CharacterController.CanGetOutVehicleScript();
		// Is character weapon raised?
		m_bIsCharacterWeaponRaised = m_CharacterController.IsWeaponRaised();
		// Can character fire weapon
		m_bCanCharacterFireWeapon = m_CharacterController.CanFire();
		// Is character currently using an item?
		m_bIsCharacterUsingItem = m_CharacterController.IsUsingItem();
		// Item that character is holding in his right hand
		m_CurrentItemEntity = m_CharacterController.GetAttachedGadgetAtLeftHandSlot();

		// Temporary sprinting time tracking
		if (m_bIsCharacterSprinting && !m_CharacterController.GetIsSprintingToggle())
			m_fSprintingTime += timeSlice;
		else
			m_fSprintingTime = 0;

		// Vehicle compartment and controls
		CompartmentAccessComponent compartmentAccess = m_Character.GetCompartmentAccessComponent();

		if (compartmentAccess)
		{
			m_bCanCharacterGetOutVehicle = m_bCanCharacterGetOutVehicle && compartmentAccess.CanGetOutVehicle();
			m_bIsCharacterGettingIn = compartmentAccess.IsGettingIn();
			m_bIsCharacterGettingOut = compartmentAccess.IsGettingOut();

			// Vehicle compartment
			BaseCompartmentSlot slot = compartmentAccess.GetCompartment();
			if (slot)
			{
				// Is character forced to freelook by compartment?
				m_bIsCharacterFreelookForced = m_bIsCharacterFreelookForced || slot.GetForceFreeLook();

				// Vehicle
				IEntity vehicle = slot.GetOwner();
				
				// In case the slot is actually in the injected compartment, like the backseats of Jeep
				vehicle = GetVehicle(vehicle);	
				
				if (vehicle != m_CurrentVehicle)
				{
					m_CurrentVehicle = vehicle;

					if (vehicle)
					{
						m_CurrentVehicleSignals = SignalsManagerComponent.Cast(vehicle.FindComponent(SignalsManagerComponent));
					}
				}

				// Temporary turbo time tracking
				// TODO: Condition activation time
				m_eCompartmentType = slot.GetType();
				if (m_eCompartmentType == ECompartmentType.PILOT && GetGame().GetInputManager().GetActionTriggered("CarTurbo"))
					m_fTurboTime += timeSlice;
				else
					m_fTurboTime = 0;

				// Turret controls
				m_CurrentVehicleController = slot.GetController();
				m_CurrentTurretController = TurretControllerComponent.Cast(m_CurrentVehicleController);
				if (m_CurrentTurretController)
				{
					BaseWeaponManagerComponent weaponManager = m_CurrentTurretController.GetWeaponManager();
					if (weaponManager)
						m_CurrentVehicleWeapon = weaponManager.GetCurrentWeapon();
				}
			}
			else
			{
				m_CurrentVehicle = null;
			}
		}

		// Clear vehicle variables
		if (!m_CurrentVehicle)
		{
			m_CurrentVehicleSignals = null;
			m_CurrentVehicleController = null;
		}

		// Current character weapon manager
		// Current character weapon
		BaseWeaponManagerComponent weaponManager = m_CharacterController.GetWeaponManagerComponent();
		if (weaponManager)
		{
			// Weapon slot -> weapon entity
			WeaponSlotComponent currentSlot = weaponManager.GetCurrentSlot();
			if (currentSlot)
				m_CurrentWeaponEntity = currentSlot.GetWeaponEntity();

			// BaseWeaponComponent
			BaseWeaponComponent currentWeapon = weaponManager.GetCurrent();
			if (currentWeapon != m_CurrentWeapon)
			{
				m_CurrentWeapon = currentWeapon;
				m_bCanFetchInventory = true;
			}

			// Muzzle and magazine
			if (m_CurrentWeapon)
			{
				m_CurrentMuzzle = m_CurrentWeapon.GetCurrentMuzzle();
				m_CurrentMagazine = m_CurrentWeapon.GetCurrentMagazine();
			}

			// Is character reloading
			m_bIsCharacterReloading = m_CharacterController.IsReloading();
		}

		// Does character have additional mags for current weapon?
		if (m_bCanFetchInventory && m_StorageManager)
		{
			if (m_CurrentWeapon)
				m_iAdditionalMagazines = m_StorageManager.GetMagazineCountByWeapon(m_CurrentWeapon);

			m_iMedicalItemCount = m_StorageManager.GetHealthComponentCount();

			// Check medical items in quick slots
			SCR_CharacterInventoryStorageComponent characterStorage = m_StorageManager.GetCharacterStorage();
			if (characterStorage)
			{
				m_iMedicalItemCountInQuickSlots = 0;

				array<ref SCR_QuickslotBaseContainer> items = characterStorage.GetQuickSlotItems();
				SCR_QuickslotEntityContainer entityContainer;
				foreach (SCR_QuickslotBaseContainer container : items)
				{
					entityContainer = SCR_QuickslotEntityContainer.Cast(container);
					if (entityContainer && entityContainer.GetEntity()  && entityContainer.GetEntity().FindComponent(SCR_ConsumableItemComponent))
						m_iMedicalItemCountInQuickSlots++;
				}
			}

			m_bCanFetchInventory = false;
		}

		// Camera handler to check focus mode
		CameraHandlerComponent cameraHandler = m_CharacterController.GetCameraHandlerComponent();
		if (cameraHandler)
			m_fFocusMode = cameraHandler.GetFocusMode();

		// Fetch available gadgets
		m_bIsGadgetSelection = GetGame().GetInputManager().GetActionTriggered("GadgetSelection");
		m_bIsWeaponManipulation = GetGame().GetInputManager().GetActionTriggered("WeaponManipulation");

		// VON status
		if (m_VONController)
			m_bCharacterRadiosCount = m_VONController.GetVONEntryCount();

		// VON usage
		if (m_VON)
		{
			m_bCharacterIsUsingRadio = m_VON.IsTransmitingRadio();
			if (m_bCharacterIsUsingRadio)
				m_bIsRadioLongRange = m_VONController.IsUsingLRR();
			
			m_bOwnsLongRangeRadio = m_VONController.IsLRRAvailable();
		}
		
		// Addition data
		FetchHealthData(timeSlice);
		
		// Map data
		if (m_MapEntity && m_MapEntity.IsOpen())
			FetchMapData(timeSlice);
	}

	//------------------------------------------------------------------------------------------------
	//! Gets vehicle entity from potentially attached entity
	protected IEntity GetVehicle(IEntity vehicle)	
	{
		IEntity parent = null;
		
		if (vehicle && vehicle.Type() != Vehicle)
			parent = vehicle.GetParent();	
	
		if (parent)
			vehicle = GetVehicle(parent);
		
		return vehicle;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Fetch data related to character health state, bleeding, used medicals, etc.
	protected void FetchHealthData(float timeSlice)
	{
		m_CharacterDamageComponent = SCR_CharacterDamageManagerComponent.Cast(m_Character.GetDamageManager());
		if (!m_CharacterDamageComponent)
		{
			Print("[SCR_AvailableActionsConditionData] - can't fetch health data!", LogLevel.WARNING);
			return;
		}
		
		// Check bleeding
		if (m_CharacterDamageComponent)
			m_bIsCharacterBleeding = m_CharacterDamageComponent.IsBleeding();

		// Bleeding time tracking
		if (m_bIsCharacterBleeding)
			m_fBleedingTime += timeSlice;
		else
			m_fBleedingTime = 0;

		if (m_CharacterController)
			m_bIsCharacterConscious = !m_CharacterController.IsUnconscious();
		
		// Has tourniquet on any limb 
		array<ECharacterHitZoneGroup> limbs = {};
		m_CharacterDamageComponent.GetAllExtremities(limbs);
		
		for (int i = 0, count = limbs.Count(); i < count; i++)
		{
			m_bHasTourniquetLimb = m_CharacterDamageComponent.GetGroupTourniquetted(limbs[i]);
			if (m_bHasTourniquetLimb)
				break;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Fetch 2D map related data
	protected void FetchMapData(float timeSlice)
	{
		m_bCanRotateMapElement = false;
		m_bCanDragMapElement = false;
		m_bCanActivateMapElement = false;
		m_bCanResetElementRotation = false;

		array<Widget> mapWidgetsUnderCursor = SCR_MapCursorModule.GetMapWidgetsUnderCursor();

		SCR_MapElementMoveComponent moveComp;
		foreach (Widget widget : mapWidgetsUnderCursor)
		{
			if (!widget)
				continue;
			
			moveComp = SCR_MapElementMoveComponent.Cast(widget.FindHandler(SCR_MapElementMoveComponent));	
			if (!moveComp)
				continue;

			m_bCanRotateMapElement = moveComp.m_bCanRotate;
			m_bCanDragMapElement = moveComp.m_bCanDrag;
			m_bCanActivateMapElement = moveComp.m_bCanActivate;
			m_bCanResetElementRotation = moveComp.m_bCanResetRotation;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// Contructor 
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Creates condition data container
	void SCR_AvailableActionsConditionData()
	{
		m_VONController = SCR_VONController.Cast(GetGame().GetPlayerController().FindComponent(SCR_VONController));
		m_MapEntity = SCR_MapEntity.GetMapInstance();
	}
};
