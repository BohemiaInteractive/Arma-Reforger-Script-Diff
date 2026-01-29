class SCR_WeaponInfo_MultiWeaponTurret : SCR_InfoDisplayExtended
{
	[Attribute("", UIWidgets.EditBox, "Name of the turret slot from the SlotManagerComponent")]
	protected string m_sTurretSlot;
	
	[Attribute("{4789C90C5591CBC7}Configs/VehicleHUD/VehicleWeaponInfo_GroupWeaponFiremodes.conf")]
	protected ref GroupWeaponFiremode m_FiremodeIconsConfig;
	
	//List of Icons mapped to types
	[Attribute("{8D3E102893955B15}Configs/Inventory/ItemHints/MagazineAmmoType.conf")]
	protected ref SCR_AmmoTypeInfoConfig m_AmmoTypeIcons;
	
	[Attribute("{09468E8C42D80502}UI/layouts/HUD/WeaponInfo/WeaponInfo_AmmoTypeIcon.layout")]
	protected ResourceName m_sAmmoTypeIconLayout;
	
	[Attribute("1")]
	protected bool m_bIsPilotOnly;
	
	[Attribute("0")]
	protected bool m_bShouldFade;

	[Attribute(uiwidget: UIWidgets.ComboBox, desc: "List of modes for which quantity indicator should be visible.", enumType: EWeaponGroupFireMode)]
	ref array<EWeaponGroupFireMode> m_aShowQuantityIndicator;
	
	protected EventHandlerManagerComponent m_turretEventHandler;
	protected BaseWeaponManagerComponent m_WeaponManager;
	
	protected ref SCR_WeaponInfo_MultiWeaponTurretWidgets m_Widgets;

	//Animations
	protected ref SCR_FadeInOutAnimator m_WeaponInfoPanelAnimator;
	protected ref SCR_FadeInOutAnimator m_ZeroingAnimator;
	
	protected bool m_bFadeInOutActive;
	
	protected ref SCR_MultiWeaponTurretDataHolder m_DataHolder;
	protected EWeaponFeature m_eWeaponStateEvent = EWeaponFeature.NONE;
	
	//Height in pixels	
	protected const int SIZE_FIREMODE = 50;
	protected const int SIZE_MAGAZINES = 50;
	
	//Resources
	protected const ResourceName DEFAULT_MAGAZINE_LAYOUT = "{8A98061335A620E1}UI/layouts/HUD/WeaponInfo/SingleMagazineWidget_RocketPod.layout";
	protected const ResourceName DEFAULT_EMPTY_PYLON_LAYOUT = "{5C5829628BBDB274}UI/layouts/HUD/WeaponInfo/WeaponInfo_EmptyPylon.layout";
	
	//Standard colors for flashes
	protected const ref Color COLOR_WHITE = Color.FromSRGBA(255, 255, 255, 255);
	protected const ref Color COLOR_ORANGE = Color.FromSRGBA(226, 167, 79, 255);
	
	//The localization strings are not the correct ones, because of localization lock at the time of completion
	protected const string EMPTY_TEXT_LOCALIZED = "#AR-WeaponUI_Empty";
	protected const string MIXED_TEXT_LOCALIZED = "#AR-WeaponUI_Mixed";
	
	//Time until whole panel fades out
	protected const float FADEOUT_PANEL_DELAY = 6; 
	//Time until zeroing fades out
	protected const float FADEOUT_OPTICS_DELAY = 6;

	private static ref array<EAmmoType> s_AmmoTypes = new array<EAmmoType>();
	private static ref array<IEntity> s_LoadedRockets = new array<IEntity>();
	
	void SCR_WeaponInfo_MultiWeaponTurret()
	{
		if (s_AmmoTypes.Count() == 0)
			SCR_Enum.GetEnumValues(EAmmoType, s_AmmoTypes);
	}
	
	//######## CALLBACKS ########
	//------------------------------------------------------------------------------------------------
	protected void OnTurretChanged(TurretComponent newTurret)
	{
		// Set weapon state change flag
		m_eWeaponStateEvent |= EWeaponFeature.WEAPON;
		
		Show(newTurret != null);
		if(!newTurret)
			return;
		
		IEntity turretEntity = newTurret.GetOwner();
		
		//reset widget state
		if (!m_DataHolder)
		{
			m_DataHolder = new SCR_MultiWeaponTurretDataHolder();
			m_DataHolder.Init();
		}
		else
		{
			m_DataHolder.Reset();
			if (m_DataHolder.m_FireModeManager)
				m_DataHolder.m_FireModeManager.GetOnTurretFireModeValuesChanged().Remove(UpdateQuantityIndicator);
		}
		
		//Get turret and turret Controller
		m_DataHolder.m_Turret = newTurret;
		m_DataHolder.m_TurretController = TurretControllerComponent.Cast(turretEntity.FindComponent(TurretControllerComponent));
		if (!m_DataHolder.m_Turret || !m_DataHolder.m_TurretController)
			return;
		
		//Get the MultiFire Manager, which contains weapons list, weapons groups and more info we need. Also store the weapons list.
		m_DataHolder.m_multiWComp = MultiFireWeaponManagerComponent.Cast(turretEntity.FindComponent(MultiFireWeaponManagerComponent));
		if (!m_DataHolder.m_multiWComp)
			return;
		
		m_DataHolder.m_FireModeManager = SCR_FireModeManagerComponent.Cast(turretEntity.FindComponent(SCR_FireModeManagerComponent));
		if (!m_DataHolder.m_FireModeManager)
			return;
		
		m_DataHolder.m_FireModeManager.GetOnTurretFireModeValuesChanged().Insert(UpdateQuantityIndicator);
		//Get all slots and filter out the empty ones
		m_DataHolder.m_multiWComp.GetWeaponsSlots(m_DataHolder.m_aWeaponslots);
		
		AddSlotsEvents();
		
		//fill up everything else
		FillMagazines();
		OnSelectedFiregroupChange(false);
		OnFiremodeChangedInit();
		UpdateWeaponNames();
		
		// Check if weapon has scope sights component
		m_DataHolder.m_Sights = SCR_2DSightsComponent.Cast(m_DataHolder.m_Turret.GetSights());//GetSights();
		m_DataHolder.m_SightsZoomFOVInfo = null;
		
		if (m_DataHolder.m_Sights)
			m_DataHolder.m_SightsZoomFOVInfo = SCR_SightsZoomFOVInfo.Cast(m_DataHolder.m_Sights.GetFOVInfo());
		
		if (m_DataHolder.m_SightsZoomFOVInfo)
			m_DataHolder.m_SightsZoomFOVInfo.GetEventOnZoomChanged().Insert(OnZoomChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnFiremodeChangedInit()
	{
		string sFiremode;
		m_DataHolder.m_FireModeManager.GetFireMode(sFiremode);
		EWeaponGroupFireMode firemode = typename.StringToEnum(EWeaponGroupFireMode, sFiremode);
		
		OnFiremodeChanged(firemode);
		UpdateQuantityIndicator(firemode, m_DataHolder.m_FireModeManager.GetRippleQuantity());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnFiremodeChanged(EWeaponGroupFireMode newFiremode)
	{
		m_eWeaponStateEvent |= EWeaponFeature.FIREMODE;		
		
		UpdateFireModeIndicator(newFiremode);
		
		SetCurrentAmmoType();
		HighlightCurrentAmmoType();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnSelectedFiregroupChange(bool hasAmmo)
	{
		m_eWeaponStateEvent |= EWeaponFeature.FIREMODE;
		
		string req;
		ref array<int> groupIds = m_DataHolder.m_FireModeManager.GetCurrentWeaponGroup(req).m_aWeaponsGroupIds;
		
		foreach (WeaponSlotComponent key, SCR_SingleMagazineWidgetComponent_Base slot : m_DataHolder.m_mWeaponMags)
		{
			if (groupIds.Contains(key.GetWeaponSlotIndex()))
				slot.SetSelected(true);
			else
				slot.SetSelected(false);
		}
		
		//OnFiremodeChangedInit();
		SetCurrentAmmoType();
		HighlightCurrentAmmoType();
		UpdateWeaponNames();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnAmmoCountChanged(BaseWeaponComponent changedWeaponComp, BaseMuzzleComponent changedMuzzleComp, BaseMagazineComponent changedMagComp, int newAmmoCount, bool isBarrelChambered)
	{
		if (!m_DataHolder)
			return;
		
		m_eWeaponStateEvent |= EWeaponFeature.AMMOCOUNT;
		
		BaseWeaponComponent tempWeaponComp;
		
		foreach (WeaponSlotComponent slotComp : m_DataHolder.m_aWeaponslots)
		{
			IEntity slottedWeapon = slotComp.GetWeaponEntity();
			if (!slottedWeapon)
				continue;
			
			tempWeaponComp = BaseWeaponComponent.Cast(slottedWeapon.FindComponent(BaseWeaponComponent));
			
			if (tempWeaponComp == changedWeaponComp)
			{
				m_DataHolder.m_mWeaponMags[slotComp].UpdateAmmoCount(changedWeaponComp);
				break;
			}
		}
		
		UpdateAmmoTypes();
		UpdateWeaponNames();
		SetCurrentAmmoType();
		HighlightCurrentAmmoType();
	}	
	
	//------------------------------------------------------------------------------------------------
	protected void OnNewWeaponAttached(IEntity newWeapon)
	{
		foreach (WeaponSlotComponent curSlot : m_DataHolder.m_aWeaponslots)
		{
			if (curSlot.GetWeaponEntity() != newWeapon)
				continue;
			
			if (m_DataHolder.m_mWeaponMags.Contains(curSlot))
				m_DataHolder.m_mWeaponMags[curSlot].DestroyMagazine();
			
			CreateMagazine(curSlot);
			
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnNewWeaponDetached(IEntity newWeapon)
	{	
		foreach (WeaponSlotComponent curSlot : m_DataHolder.m_aWeaponslots)
		{
			if (curSlot.GetWeaponEntity())
				continue;
			
			if (!m_DataHolder.m_mWeaponMags.Contains(curSlot))
				continue;
			
			m_DataHolder.m_mWeaponMags[curSlot].DestroyMagazine();
			m_DataHolder.m_mWeaponMags.Remove(curSlot);
			
			CreateEmptyPylon(curSlot);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnTurretReload(BaseWeaponComponent weapon, bool isFinished, TurretControllerComponent turretController)
	{
		OnAmmoCountChanged(weapon, null, null, 0, true);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnZoomChanged(float zoom, float fov)
	{
		if (!m_DataHolder)
			return;
	
		// Set weapon state change flag
		m_eWeaponStateEvent |= EWeaponFeature.ZOOM;
		m_DataHolder.m_fZoom = zoom;
			
		UpdateZoomIndicator();
	}	

	//------------------------------------------------------------------------------------------------
	protected void OnCompartmentEntered(IEntity vehicle, BaseCompartmentManagerComponent manager, IEntity occupant, int managerID, int slotID)
	{
		//band-aid to check if player trying to display is actually the pilot
		IEntity playerEnt = m_PlayerController.GetControlledEntity();
		BaseCompartmentSlot playerCompSlot;
		if (occupant != playerEnt)
			return;
		
		m_bFadeInOutActive = true;

		if (!m_bIsPilotOnly || IsPilot(vehicle))
			m_WeaponInfoPanelAnimator.FadeIn(m_bShouldFade);
		else
			m_WeaponInfoPanelAnimator.FadeOutInstantly();
	}
	
	//######## UTILITIES ########
	//------------------------------------------------------------------------------------------------
	protected void FillMagazines()
	{
		foreach (WeaponSlotComponent slotComp : m_DataHolder.m_aWeaponslots)
		{
			if (slotComp.GetWeaponEntity())
				CreateMagazine(slotComp);
			else
				CreateEmptyPylon(slotComp);
		}
		
		FillAmmoTypes();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateEmptyPylon(WeaponSlotComponent slotComp)
	{
		Widget magWidget = GetGame().GetWorkspace().CreateWidgets(DEFAULT_EMPTY_PYLON_LAYOUT, m_Widgets.m_wMagazinesHolder);
		
		SetMagazinePositionalZOrder(slotComp, magWidget);
		
		SCR_SingleMagazineWidgetComponent_Base magWidgetComp = SCR_SingleMagazineWidgetComponent_Base.Cast(magWidget.FindHandler(SCR_SingleMagazineWidgetComponent_Base));
		if (!magWidgetComp)
			return;
		
		bool isSelected;
		string groupName;
		SCR_WeaponGroup currentGroup = m_DataHolder.m_FireModeManager.GetCurrentWeaponGroup(groupName);
		if (currentGroup.m_aWeaponsGroupIds.Contains(slotComp.GetWeaponSlotIndex()))
			isSelected = true;
		
		magWidgetComp.Init(NULL, SIZE_MAGAZINES, null, isSelected);
		m_DataHolder.m_mWeaponMags[slotComp] = magWidgetComp;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateMagazine(WeaponSlotComponent slotComp)
	{
		IEntity weaponEntity = slotComp.GetWeaponEntity();
		if (!weaponEntity)
			return;
		
		BaseWeaponComponent weapon = BaseWeaponComponent.Cast(weaponEntity.FindComponent(BaseWeaponComponent));
		if (!weapon)
			return;
		
		WeaponUIInfo wInfo = WeaponUIInfo.Cast(slotComp.GetUIInfo());
		
		//use those resoures to spawn and initialize magazine widget	
		Widget magWidget;
		if (wInfo)
			magWidget = GetGame().GetWorkspace().CreateWidgets(wInfo.m_MagIndicator.m_sMagazineLayout, m_Widgets.m_wMagazinesHolder);
		else
			magWidget = GetGame().GetWorkspace().CreateWidgets(DEFAULT_MAGAZINE_LAYOUT, m_Widgets.m_wMagazinesHolder);
		
		//display on correct side of the UI
		SetMagazinePositionalZOrder(slotComp, magWidget);
		
		SCR_SingleMagazineWidgetComponent_Base magWidgetComp = SCR_SingleMagazineWidgetComponent_Base.Cast(magWidget.FindHandler(SCR_SingleMagazineWidgetComponent_Base));
		if (!magWidgetComp)
			return;
		
		bool isSelected;
		string groupName;
		SCR_WeaponGroup currentGroup = m_DataHolder.m_FireModeManager.GetCurrentWeaponGroup(groupName);
		if (currentGroup.m_aWeaponsGroupIds.Contains(slotComp.GetWeaponSlotIndex()))
			isSelected = true;
		
		magWidgetComp.Init(weapon, SIZE_MAGAZINES, wInfo.m_MagIndicator, isSelected);
		m_DataHolder.m_mWeaponMags[slotComp] = magWidgetComp;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetMagazinePositionalZOrder(WeaponSlotComponent slotComp, Widget magWidget)
	{
		if (slotComp.GetWeaponSlotIndex() <= m_DataHolder.m_aWeaponslots.Count() / 2)
			magWidget.SetZOrder(slotComp.GetWeaponSlotIndex() - (m_DataHolder.m_aWeaponslots.Count() / 2));
		else
			magWidget.SetZOrder(slotComp.GetWeaponSlotIndex() - (m_DataHolder.m_aWeaponslots.Count() / 2) +1);
		
	}
	
	//------------------------------------------------------------------------------------------------
	protected void FillAmmoTypes()
	{
		EAmmoType equippedTypes = 0;
		
		foreach (WeaponSlotComponent weaponSlotComp : m_DataHolder.m_aWeaponslots)
		{
			if (weaponSlotComp.IsChamberingPossible())
				equippedTypes |= GetAllRocketAmmoTypes(weaponSlotComp);
			else
				equippedTypes |= GetGenericAmmoTypes(weaponSlotComp);
		}
		
		Widget ammoIcon;
		SCR_AmmoTypeIcon ammoIconComp;
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		
		foreach (EAmmoType type : s_AmmoTypes)
		{
			ammoIcon = workspace.CreateWidgets(m_sAmmoTypeIconLayout, m_Widgets.m_wAmmoType);
			ammoIconComp = SCR_AmmoTypeIcon.Cast(ammoIcon.FindHandler(SCR_AmmoTypeIcon));
			
			ammoIconComp.Init(type, m_AmmoTypeIcons);
			m_DataHolder.m_mAmmoTypeWidgets[type] = ammoIconComp;
			
			m_DataHolder.m_mAmmoTypeWidgets[type].SetVisibility(type & equippedTypes);
		}
		
		SetCurrentAmmoType();
		HighlightCurrentAmmoType();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateAmmoTypes()
	{
		EAmmoType equippedTypes = 0;
		
		foreach (WeaponSlotComponent weaponSlotComp : m_DataHolder.m_aWeaponslots)
		{
			if (weaponSlotComp.IsChamberingPossible())
				equippedTypes |= GetAllRocketAmmoTypes(weaponSlotComp);
			else
				equippedTypes |= GetGenericAmmoTypes(weaponSlotComp);
		}
		
		foreach (EAmmoType type : s_AmmoTypes)
		{	
			m_DataHolder.m_mAmmoTypeWidgets[type].SetVisibility(type & equippedTypes);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected EAmmoType GetAllRocketAmmoTypes(WeaponSlotComponent weaponSlotComp)
	{
		RocketEjectorMuzzleComponent muz = RocketEjectorMuzzleComponent.Cast(weaponSlotComp.GetCurrentMuzzle());
		if (!muz)
			return 0;
		
		EAmmoType ammo = 0;
		MagazineComponent magComp;
		MagazineUIInfo magInfo;

		muz.GetLoadedEntities(s_LoadedRockets);
		foreach (IEntity rocket : s_LoadedRockets)
		{
			if (!rocket)
				continue;
			
			magComp = MagazineComponent.Cast(rocket.FindComponent(MagazineComponent));
			if (!magComp)
				continue;
			
			magInfo = MagazineUIInfo.Cast(magComp.GetUIInfo());
			
			ammo |= magInfo.GetAmmoTypeFlags();
		}
		
		return ammo;
	}
	
	//------------------------------------------------------------------------------------------------
	protected EAmmoType GetCurrentRocketAmmoType(WeaponSlotComponent weaponSlotComp)
	{
		RocketEjectorMuzzleComponent muz = RocketEjectorMuzzleComponent.Cast(weaponSlotComp.GetCurrentMuzzle());
		if (!muz)
			return 0;
		
		IEntity rocket = muz.GetBarrelProjectile(muz.GetCurrentBarrelIndex());
		if (!rocket)
			return 0;
		
		MagazineComponent magComp = MagazineComponent.Cast(rocket.FindComponent(MagazineComponent));
		if (!magComp)
			return 0;
		
		MagazineUIInfo magInfo = MagazineUIInfo.Cast(magComp.GetUIInfo());
		return magInfo.GetAmmoTypeFlags();
	}
	
	//------------------------------------------------------------------------------------------------
	protected EAmmoType GetGenericAmmoTypes(WeaponSlotComponent weaponSlotComp)
	{
		RocketEjectorMuzzleComponent muz = RocketEjectorMuzzleComponent.Cast(weaponSlotComp.GetCurrentMuzzle());
		if (!muz)
			return 0;
	
		BaseMagazineComponent magazine = muz.GetMagazine();
		if (!magazine)
			return 0;
		
		return MagazineUIInfo.Cast(magazine.GetUIInfo()).GetAmmoTypeFlags();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void HighlightCurrentAmmoType()
	{
		foreach (EAmmoType key, SCR_AmmoTypeIcon widget : m_DataHolder.m_mAmmoTypeWidgets)
		{
			if (m_DataHolder.m_eAmmoTypeFlags & key)
				widget.SetNewState(SCR_EWeaponInfoIconState.HIGHLIGHTED);
			else
				widget.SetNewState(SCR_EWeaponInfoIconState.DISABLED);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetCurrentAmmoType()
	{
		m_DataHolder.m_eAmmoTypeFlags = 0;
		
		SCR_RocketEjectorMuzzleComponent muz;
		SCR_WeaponGroup currentGroup;
		
		foreach (WeaponSlotComponent slot : m_DataHolder.m_aWeaponslots)
		{
			string groupName;
			currentGroup = m_DataHolder.m_FireModeManager.GetCurrentWeaponGroup(groupName);
			
			if (currentGroup.m_aWeaponsGroupIds.Contains(slot.GetWeaponSlotIndex()))
			{
				muz = SCR_RocketEjectorMuzzleComponent.Cast(slot.GetCurrentMuzzle());
				if (!muz) 
					continue;
				
				if (slot.IsChamberingPossible())
					m_DataHolder.m_eAmmoTypeFlags |= GetCurrentRocketAmmoType(slot);
				else
					m_DataHolder.m_eAmmoTypeFlags |= GetGenericAmmoTypes(slot);
			}
				
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateWeaponNames()
	{
		InventoryItemComponent invComp;
		SCR_RocketEjectorMuzzleComponent muzzle;
		IEntity rocket;
		UIInfo wInfo;
		WeaponComponent wComp;
		
		m_DataHolder.m_sWeaponNames = string.Empty;
		
		foreach (WeaponSlotComponent slotComp, SCR_SingleMagazineWidgetComponent_Base value : m_DataHolder.m_mWeaponMags)
		{	
			if (value.GetCurrentState() != SCR_EWeaponInfoIconState.HIGHLIGHTED)
				continue;
			
			if (slotComp.IsChamberingPossible())
			{
				muzzle = SCR_RocketEjectorMuzzleComponent.Cast(slotComp.GetCurrentMuzzle());
				if (!muzzle)
					continue;
				
				rocket = muzzle.GetBarrelProjectile(muzzle.GetCurrentBarrelIndex());
				if (!rocket)
					continue;
				
				invComp = InventoryItemComponent.Cast(rocket.FindComponent(InventoryItemComponent));
				if (!invComp)
					continue;
				
				wInfo = invComp.GetUIInfo();
			}
			else
			{
				IEntity weaponEntity = slotComp.GetWeaponEntity();
				if (!weaponEntity)
					continue;
				
				wComp = WeaponComponent.Cast(weaponEntity.FindComponent(WeaponComponent));
				if (!wComp)
					continue;
				
				wInfo = wComp.GetUIInfo();
			}
				
			string name = wInfo.GetName();
			
			if (m_DataHolder.m_sWeaponNames.IsEmpty())
				m_DataHolder.m_sWeaponNames = wInfo.GetName();
			
			else if (m_DataHolder.m_sWeaponNames != wInfo.GetName())
				m_DataHolder.m_sWeaponNames = MIXED_TEXT_LOCALIZED;
		}
		
		if (m_DataHolder.m_sWeaponNames.IsEmpty())
			m_DataHolder.m_sWeaponNames = EMPTY_TEXT_LOCALIZED;
		
		m_Widgets.m_wWeaponName.SetTextFormat(m_DataHolder.m_sWeaponNames);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateFireModeIndicator(EWeaponGroupFireMode newFiremode)
	{
		if (!m_DataHolder || !m_Widgets)
			return;

		m_Widgets.m_wFiremodeIcon.SetVisible(false);
		m_Widgets.m_wFiremodeGlow.SetVisible(false);
		
		m_FiremodeIconsConfig.SetIconAndGlowTo(newFiremode, m_Widgets.m_wFiremodeIcon, m_Widgets.m_wFiremodeGlow);
		
		//resize widgets
		int sx, sy;
		m_Widgets.m_wFiremodeIcon.GetImageSize(0, sx, sy);
		
		float ratio;
		if (sy != 0)
			ratio = sx / sy;
		
		m_Widgets.m_wFiremodeIcon.SetSize(SIZE_FIREMODE * ratio, SIZE_FIREMODE);
		m_Widgets.m_wFiremodeGlow.SetSize(SIZE_FIREMODE * ratio, SIZE_FIREMODE);
		
		m_Widgets.m_wFiremodeIcon.SetVisible(true);
		m_Widgets.m_wFiremodeGlow.SetVisible(true);
		
		AnimateWidget_ColorFlash(m_Widgets.m_wFiremodeIcon, EWeaponFeature.FIREMODE);
	}

	//------------------------------------------------------------------------------------------------
	//! Method used to update the visibility as well as the text of the projectile quantity indicator
	//! \param[in] firemode
	//! \param[in] quantity
	//! \param[in] weaponGroupId
	protected void UpdateQuantityIndicator(EWeaponGroupFireMode fireMode, int quantity, int weaponGroupId = 0)
	{
		if (!m_Widgets || !m_Widgets.m_wProjectileQuantityText)
			return;

		bool show;
		foreach (int enabledMode : m_aShowQuantityIndicator)
		{
			if (enabledMode != fireMode)
				continue;

			show = true;
			break;
		}

		m_Widgets.m_wProjectileQuantityText.SetVisible(show);
		if (show)
			m_Widgets.m_wProjectileQuantityText.SetText(quantity.ToString());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateZoomIndicator()
	{
		if (!m_DataHolder || !m_Widgets)
			return;

		float zoom = m_DataHolder.m_fZoom;

		if (zoom <= 0 || !m_DataHolder.m_bInADS)
		{
			m_Widgets.m_wOptics.SetVisible(false);
			m_Widgets.m_wOpticsText.SetText(string.Empty);
		}
		else
		{
			string sZoom = zoom.ToString(-1,1);
			sZoom = string.Format("%1 ×", sZoom);

			m_Widgets.m_wOptics.SetVisible(true);
			m_Widgets.m_wOpticsText.SetText(sZoom);
			
			AnimateWidget_ColorFlash(m_Widgets.m_wOptics, EWeaponFeature.ZOOM);
		}
	}	
	
	//------------------------------------------------------------------------------------------------
	protected bool IsPilot(IEntity ownerVehicle)
	{
		//band-aid to check if player trying to display is actually the pilot
		if (!m_PlayerController)
			return false;
		
		IEntity playerEnt = m_PlayerController.GetControlledEntity();
		
		BaseCompartmentSlot playerCompSlot;
		
		BaseCompartmentManagerComponent compManager = BaseCompartmentManagerComponent.Cast(ownerVehicle.FindComponent(BaseCompartmentManagerComponent));
		if (!compManager)
			return false;
		
		array <BaseCompartmentSlot> compartments = {};
		compManager.GetCompartments(compartments);
		if (compartments.IsEmpty())
			return false;
		
		return compartments[0].GetOccupant() == playerEnt;
	}
	
	//######## INITIALIZATION ########
	//------------------------------------------------------------------------------------------------
	protected void AddEventHandlers(IEntity owner)
	{	
		//Find the slot that contains our Turret
		SlotManagerComponent slotManager = SlotManagerComponent.Cast(owner.FindComponent(SlotManagerComponent));
		EntitySlotInfo turSlot = slotManager.GetSlotByName(m_sTurretSlot);
		if (!turSlot)
			return;
		
		TurretComponent initialTurret = TurretComponent.Cast(turSlot.GetAttachedEntity().FindComponent(TurretComponent));
		//TurretComponent initialTurret = TurretComponent.Cast(owner.FindComponent(TurretComponent));
		
		m_turretEventHandler = EventHandlerManagerComponent.Cast(initialTurret.GetOwner().FindComponent(EventHandlerManagerComponent));
		if (!m_turretEventHandler)
			return;
		
		if (m_EventHandlerManager)
		{
			//helicopter specific events
			m_EventHandlerManager.RegisterScriptHandler("OnCompartmentEntered", this, OnCompartmentEntered);
			
			//turret specific events
			m_turretEventHandler.RegisterScriptHandler("OnTurretChange", this, OnTurretChanged);
			m_turretEventHandler.RegisterScriptHandler("OnFiremodeChange", this, OnFiremodeChanged);
			m_turretEventHandler.RegisterScriptHandler("OnAmmoCountChanged", this, OnAmmoCountChanged);
			m_turretEventHandler.RegisterScriptHandler("OnTurretReload", this, OnTurretReload);
			m_turretEventHandler.RegisterScriptHandler("OnFireGroupChange", this, OnSelectedFiregroupChange);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void AddSlotsEvents()
	{
		InventoryStorageSlot storageInfo;
		
		foreach (WeaponSlotComponent weaponSlot : m_DataHolder.m_aWeaponslots)
		{
			storageInfo = weaponSlot.GetSlotInfo();
			if (!storageInfo)
				continue;
			storageInfo.GetAttachedEntityInvoker().Insert(OnNewWeaponAttached);
			storageInfo.GetDetachedEntityInvoker().Insert(OnNewWeaponDetached);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RemoveSlotEvents()
	{
		InventoryStorageSlot storageInfo;
		
		foreach (WeaponSlotComponent weaponSlot : m_DataHolder.m_aWeaponslots)
		{
			storageInfo = weaponSlot.GetSlotInfo();
			if (!storageInfo)
				continue;
			storageInfo.GetAttachedEntityInvoker().Remove(OnNewWeaponAttached);
			storageInfo.GetDetachedEntityInvoker().Remove(OnNewWeaponDetached);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RemoveEventHandlers(IEntity owner)
	{
		if (m_EventHandlerManager)
		{
			//helicopter specific events
			m_EventHandlerManager.RemoveScriptHandler("OnCompartmentEntered", this, OnCompartmentEntered);
		}
		m_EventHandlerManager = null;	
		
		if (m_turretEventHandler)
		{
			//turret specific events
			m_turretEventHandler.RemoveScriptHandler("OnTurretChange", this, OnTurretChanged);
			m_turretEventHandler.RemoveScriptHandler("OnFiremodeChange", this, OnFiremodeChanged);
			m_turretEventHandler.RemoveScriptHandler("OnAmmoCountChanged", this, OnAmmoCountChanged);
			m_turretEventHandler.RemoveScriptHandler("OnTurretReload", this, OnTurretReload);
			m_turretEventHandler.RemoveScriptHandler("OnFireGroupChange", this, OnSelectedFiregroupChange);
		}
		m_turretEventHandler = null;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override bool DisplayStartDrawInit(IEntity owner)
	{
		if (!owner)
			return false;
		
		//find and store Weapon Manager and Menu Manager
		m_WeaponManager = BaseWeaponManagerComponent.Cast(owner.FindComponent(BaseWeaponManagerComponent));
		if (!m_WeaponManager)
			return false;

		m_MenuManager = GetGame().GetMenuManager();
		if (!m_MenuManager)
			return false;
	
		m_DataHolder = new SCR_MultiWeaponTurretDataHolder();
		m_DataHolder.Init();
		
		AddEventHandlers(owner);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void DisplayStartDraw(IEntity owner)
	{
		if (!m_wRoot)
			return;
		
		m_Widgets = new SCR_WeaponInfo_MultiWeaponTurretWidgets();
		m_Widgets.Init(m_wRoot);
		
		if (!m_WeaponInfoPanelAnimator)
			m_WeaponInfoPanelAnimator = new SCR_FadeInOutAnimator(m_Widgets.m_wWeaponInfoPanel, UIConstants.FADE_RATE_FAST, UIConstants.FADE_RATE_SLOW, FADEOUT_PANEL_DELAY, true);

		if (!m_ZeroingAnimator)
			m_ZeroingAnimator = new SCR_FadeInOutAnimator(m_Widgets.m_wZeroing, UIConstants.FADE_RATE_FAST, UIConstants.FADE_RATE_SLOW, FADEOUT_OPTICS_DELAY, true);	
		
		//Hide the UI if it is only for pilot and the player is not the pilot
		//sadly currently only way I have to manage this, because of how UI is initialized from the helicopter
		if (m_bIsPilotOnly && !IsPilot(owner))
		{
			m_bFadeInOutActive = true;
			m_WeaponInfoPanelAnimator.FadeOutInstantly();
		}
		
		//CURRENT PLACEHOLDER CODE, DISPLAYING ON HELICOPTER INSTEAD OF TURRET
		//Find the slot that contains our Turret
		SlotManagerComponent slotManager = SlotManagerComponent.Cast(owner.FindComponent(SlotManagerComponent));
		EntitySlotInfo turSlot = slotManager.GetSlotByName(m_sTurretSlot);
		if (!turSlot)
			return;
		
		TurretComponent initialTurret = TurretComponent.Cast(turSlot.GetAttachedEntity().FindComponent(TurretComponent));
		if (!initialTurret)
			return;
		
		//Using the info display on the turret instead, will be used instead of the above once gamecode is ready
		//TurretComponent initialTurret = TurretComponent.Cast(owner.FindComponent(TurretComponent));
		
		m_Widgets.m_wFiremode.SetVisible(true);
		OnTurretChanged(initialTurret);
		
	}	
	
	//------------------------------------------------------------------------------------------------
	protected override void DisplayStopDraw(IEntity owner)
	{
		// Un-register weapon related weapon event handlers
		RemoveEventHandlers(owner);
		RemoveSlotEvents();
		
		m_DataHolder = null;
		
		// Destroy the animators
		m_WeaponInfoPanelAnimator = null;
		m_ZeroingAnimator = null;
		
		m_wRoot = null;
	}
	
	//######## DISPLAY UPDATE & FADINGS ########
	//------------------------------------------------------------------------------------------------
	protected override void DisplayUpdate(IEntity owner, float timeSlice)
	{		
		if (!m_wRoot || !m_DataHolder || !m_Widgets)
			return;
		
		if (m_eWeaponStateEvent != EWeaponFeature.NONE && m_bShouldFade) 
		{
			FadeElements();
			m_eWeaponStateEvent = EWeaponFeature.NONE;	
			m_bFadeInOutActive = true;
		}
		
		if (!m_bFadeInOutActive)
			return;
		
		if (m_ZeroingAnimator)
			m_ZeroingAnimator.Update(timeSlice);
		
		if (m_WeaponInfoPanelAnimator)
			m_WeaponInfoPanelAnimator.Update(timeSlice);
		
		m_bFadeInOutActive = m_WeaponInfoPanelAnimator.IsRunning() || m_ZeroingAnimator.IsRunning();	
	}
	
	//------------------------------------------------------------------------------------------------
	protected void FadeElements()
	{
		// Get some shared states / conditions for fading
		bool inADS = m_DataHolder && m_DataHolder.m_bInADS;
		bool inInspection = m_DataHolder && m_DataHolder.m_bInInspectionMode;
		
		// Main weapon UI (magazine, magazine count, firemode) fadein
		bool panelVisibleEvent = m_eWeaponStateEvent != 0;
		bool panelVisibleOverride = inADS || inInspection;
		if (panelVisibleEvent || panelVisibleOverride)
			m_WeaponInfoPanelAnimator.FadeIn(!panelVisibleOverride);
		
		// Zeroing indicator fadein
		bool zeroingWidgetHasText = !m_Widgets.m_wZeroingText.GetText().IsEmpty();
		bool zeroingVisibleEvent = m_eWeaponStateEvent & (EWeaponFeature.ADS | EWeaponFeature.ZEROING | EWeaponFeature.MUZZLE | EWeaponFeature.WEAPON | EWeaponFeature.INSPECTION);
		bool zeroingVisibleOverride = inADS || inInspection;
		if (zeroingWidgetHasText && (zeroingVisibleEvent || zeroingVisibleOverride))
			m_ZeroingAnimator.FadeIn(!zeroingVisibleOverride);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void AnimateWidget_ColorFlash(Widget w, EWeaponFeature requiredFlag = -1, float speed = UIConstants.FADE_RATE_SLOW)
	{
		if (!w)
			return;
		
		if (requiredFlag != -1 && m_eWeaponStateEvent != requiredFlag)
			return;
		
		w.SetColor(COLOR_ORANGE);
		AnimateWidget.Color(w, COLOR_WHITE, speed);
	}
}

[BaseContainerProps(configRoot: true)]
class GroupWeaponFiremode
{
	[Attribute("")]
	ResourceName m_sImageset;
	
	[Attribute("")]
	ResourceName m_sImagesetGlow;
	
	[Attribute()]
	ref array<ref GroupWeaponFiremodeEntry> m_aFiremodes;
	
	void SetIconAndGlowTo(EWeaponGroupFireMode firemode, out ImageWidget icon, out ImageWidget glow)
	{
		foreach (GroupWeaponFiremodeEntry modeEntry : m_aFiremodes)
		{
			if (modeEntry.m_eMode == firemode)
			{
				icon.LoadImageFromSet(0, m_sImageset, modeEntry.m_Icon);
				glow.LoadImageFromSet(0, m_sImagesetGlow, modeEntry.m_IconGlow);
				
				icon.SetImage(0);
				glow.SetImage(0);
			}
		}
	}
}

[BaseContainerProps()]
class GroupWeaponFiremodeEntry
{
	[Attribute("0", UIWidgets.ComboBox, "Firemode", "", enumType: EWeaponGroupFireMode)]
	EWeaponGroupFireMode m_eMode;
	
	[Attribute("")]
	string m_sDisplayName;
	
	[Attribute("")]
	string m_Icon;
	
	[Attribute("")]
	string m_IconGlow;
}

[BaseContainerProps()]
class WeaponInfoColorSet
{
	[Attribute(UIColors.GetColorAttribute(UIColors.IDLE_ACTIVE), UIWidgets.ColorPicker)]
	ref Color m_IconColor;
	
	[Attribute(UIColors.GetColorAttribute(UIColors.BACKGROUND_HOVERED), UIWidgets.ColorPicker)]
	ref Color m_BackgroundColor;
	
	[Attribute(UIColors.GetColorAttribute(UIColors.IDLE_ACTIVE), UIWidgets.ColorPicker)]
	ref Color m_GlowColor;
	
	[Attribute(UIColors.GetColorAttribute(UIColors.CONTRAST_DEFAULT), UIWidgets.ColorPicker)]
	ref Color m_ContrastColor;	
}

enum SCR_EWeaponInfoIconState
{
	IDLE,
	HIGHLIGHTED,
	DISABLED
}

class SCR_MultiWeaponTurretDataHolder
{
	//cache information containers
	TurretComponent m_Turret;
	TurretControllerComponent m_TurretController;
	MultiFireWeaponManagerComponent m_multiWComp;
	SCR_FireModeManagerComponent m_FireModeManager;
	
	EAmmoType m_eAmmoTypeFlags;
	
	SCR_2DSightsComponent m_Sights;
	SCR_SightsZoomFOVInfo m_SightsZoomFOVInfo;
	
	ref array<WeaponSlotComponent> m_aWeaponslots = {};
	ref array<Widget> m_aEmptyPylonWidgets = {};
	ref map<EAmmoType, SCR_AmmoTypeIcon> m_mAmmoTypeWidgets;
	ref map<WeaponSlotComponent, SCR_SingleMagazineWidgetComponent_Base> m_mWeaponMags;
	
	bool m_bInADS;
	bool m_bInInspectionMode;
	
	float m_fZoom;
	
	string m_sWeaponNames;
	
	//------------------------------------------------------------------------------------------------
	void Init()
	{
		m_Turret = null;
		m_TurretController = null;
		m_aWeaponslots = {};
		m_mAmmoTypeWidgets = new map<EAmmoType, SCR_AmmoTypeIcon>();
		m_mWeaponMags = new map<WeaponSlotComponent, SCR_SingleMagazineWidgetComponent_Base>();
		m_multiWComp = null;
		m_FireModeManager = null;
		m_bInADS = false;
		m_bInInspectionMode = false;
		m_fZoom = 0;
		m_bInADS = false;
		m_Sights = null;
		m_SightsZoomFOVInfo = null;
		m_sWeaponNames = "";
		m_eAmmoTypeFlags = 0;
	}
	
	//------------------------------------------------------------------------------------------------
	void Reset()
	{
		m_Turret = null;
		m_TurretController = null;
		m_aWeaponslots = {};
		m_multiWComp = null;
		m_FireModeManager = null;
		m_bInADS = false;
		m_bInInspectionMode = false;
		m_fZoom = 0;
		m_bInADS = false;
		m_Sights = null;
		m_SightsZoomFOVInfo = null;
		m_sWeaponNames = "";
		m_eAmmoTypeFlags = 0;
		
		//remove previous widgets if present (required for when player is switching seats)
		if (m_mWeaponMags.Count() > 0)
		{
			Widget rootw;
			foreach (WeaponSlotComponent wSlt, SCR_SingleMagazineWidgetComponent_Base mag : m_mWeaponMags)
			{
				if (mag)
					rootw = mag.GetRootWidget();
				if (rootw)
					rootw.RemoveFromHierarchy();
			}
			m_mWeaponMags.Clear();
			
			foreach(EAmmoType key, SCR_AmmoTypeIcon value : m_mAmmoTypeWidgets)
			{
				value.DeleteWidgets();
			}
			m_mAmmoTypeWidgets.Clear();
		}
	}
}