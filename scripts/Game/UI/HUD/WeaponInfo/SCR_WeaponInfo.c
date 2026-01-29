//#define WEAPON_INFO_DEBUG
//#define WEAPON_INFO_DEBUG_STATES

//#define WEAPON_INFO_DEBUG_WATERFALL_EVENTS
//#define WEAPON_INFO_BLOCK_WATERFALL_EVENTS

// Will enable lots of logging on every event
// Can define it through -scrDefine WEAPON_INFO_DEBUG

enum EAmmoType
{
	/*UNDEFINED = 0,*/
	FMJ = 1 << 0,
	TRACER = 1 << 1,
	AP = 1 << 2,
	HE = 1 << 3,
	HEAT = 1 << 4,
	FRAG = 1 << 5,
	SMOKE = 1 << 6,
	INCENDIARY = 1 << 7,
	SNIPER = 1 << 8,
	ILLUMINATION = 1 << 9,
	TRAINING = 1 << 10,
	SUBSONIC = 1 << 11,
	HEDP = 1 << 12,
}

enum EWeaponFeature
{
	NONE = 0,
	ZEROING = 1,
	FIREMODE = 2,
	AMMOCOUNT = 4,
	MAGAZINE = 8,
	MUZZLE = 16,
	WEAPON = 32,
	ADS	= 64,
	MAGAZINE_COUNT = 128,
	INSPECTION = 256,
	ZOOM = 512,
	RELOADING = 1024,
	MISC = 32768
};

class SCR_WeaponInfo : SCR_InfoDisplayExtended
{
	#ifdef WEAPON_INFO_DEBUG
	static void _print(string str)
	{
		int hour, minute, second;
		System.GetHourMinuteSecondUTC(hour, minute, second);
		string timeStr = string.Format("%1.%2.%3", hour, minute, second);
		Print(string.Format("%1 [Weapon Info UI] %2", timeStr, str), LogLevel.DEBUG);
	}
	#endif
	
	const int SIZE_FIREMODE = 50;			// Height in pixels	
	const int SIZE_MAGAZINE = 77;			// Height in pixels	
		
	const float UPDATE_INTERVAL = 0.1;	
	const float FADED_OPACITY = 0.3;
	
	static const ref Color COLOR_WHITE = Color.FromSRGBA(255, 255, 255, 255);
	static const ref Color COLOR_ORANGE = Color.FromSRGBA(226, 167, 79, 255);
	
	protected const float FADEOUT_PANEL_DELAY = 6; // Time until whole panel fades out
	protected const float FADEOUT_OPTICS_DELAY = 6; // Time until zeroing fades out
	protected const float FADEOUT_AMMO_TYPE_DELAY = 6; // Time until ammo type fades out
	
	protected BaseWeaponManagerComponent m_WeaponManager;
	protected SCR_InventoryStorageManagerComponent m_InventoryManager;
	protected SCR_CompartmentAccessComponent m_CompartmentAccess;

	// Members for previous states of weapon
	protected ref SCR_WeaponState m_WeaponState;
	
	protected EWeaponFeature m_eWeaponStateEvent = EWeaponFeature.NONE;

	// Resources
	protected const ResourceName DEFAULT_WEAPON_INFO_LAYOUT = "{8170DE93810F928A}UI/layouts/HUD/WeaponInfo/WeaponInfo.layout";
	protected const ResourceName DEFAULT_MAGAZINE_INDICATOR = "{7C114BA7C59E198D}Configs/WeaponInfo/MagazineIndicators/box.conf";
		
	// Widgets
	ref SCR_WeaponInfoWidgets m_Widgets;
	
	protected ref SCR_FadeInOutAnimator m_WeaponInfoPanelAnimator;
	protected ref SCR_FadeInOutAnimator m_ZeroingAnimator;
	
	protected bool m_bFadeInOutActive = false;
	
	// Other
	ref SCR_MagazinePredicate m_pMagazineSearchPredicate = new SCR_MagazinePredicate(); // Predicate for searching for magazines
	ref SCR_PrefabDataPredicate m_pPrefabDataPredicate = new SCR_PrefabDataPredicate();
	
	//------------------------------------------------------------------------------------------------
	protected void OnWeaponChanged(BaseWeaponComponent weapon, BaseWeaponComponent prevWeapon)
	{
		#ifdef WEAPON_INFO_DEBUG
		_print("OnWeaponChanged");
		_print(string.Format("    weapon:     %1", weapon));
		#endif

		// Set weapon state change flag
		m_eWeaponStateEvent |= EWeaponFeature.WEAPON;

		// In case of grenades, the provided weapon is a weapon slot, with the actual grenade inside this slot
		WeaponSlotComponent weaponSlot = WeaponSlotComponent.Cast(weapon);

		if (weaponSlot)
		{
			IEntity e = weaponSlot.GetWeaponEntity();
			
			if (e)
				weapon = BaseWeaponComponent.Cast(e.FindComponent(BaseWeaponComponent));
			else
				weapon = null;
		}
		
		// Initialize weapon state from our current weapon
		if (!m_WeaponState)
			m_WeaponState = new SCR_WeaponState();

		if (m_WeaponState.m_Weapon != weapon)
		{
			// Remove any possible existing "zoom-changed" invokers
			if (m_WeaponState.m_SightsZoomFOVInfo)
				m_WeaponState.m_SightsZoomFOVInfo.GetEventOnZoomChanged().Remove(OnZoomChanged);
			
			// Wipe the weapon state data
			m_WeaponState.Init();
		}
		
		// Store current weapon		
		m_WeaponState.m_Weapon = weapon;		
		
		// Hide weapon UI if there is no weapon equipped and stop
		Show(weapon != null);
		
		if (!weapon)
			return;
		
		// Get weapon & grenade UI info
		m_WeaponState.m_WeaponUI = WeaponUIInfo.Cast(weapon.GetUIInfo());
		m_WeaponState.m_GrenadeUI = GrenadeUIInfo.Cast(m_WeaponState.m_WeaponUI);

		// If weapon is a grenade, get its type
		if (m_WeaponState.m_GrenadeUI)
		{
			m_WeaponState.m_eAmmoTypeFlags = m_WeaponState.m_GrenadeUI.GetAmmoTypeFlags();
			m_WeaponState.m_bHasSpecialAmmo = !(m_WeaponState.m_eAmmoTypeFlags == 0 || m_WeaponState.m_eAmmoTypeFlags & EAmmoType.FMJ);
			
			m_WeaponState.m_MagazineConfig = m_WeaponState.m_GrenadeUI.m_MagIndicator;
		}		
		
		// Check if weapon is a grenade
		IEntity e = weapon.GetOwner();
		
		if ((e && e.FindComponent(GrenadeMoveComponent)) || SCR_MineWeaponComponent.Cast(weapon))		
			m_WeaponState.m_bIsExplosive = true;
		else
			m_WeaponState.m_bIsExplosive = false;

		// Check if weapon has scope sights component
		m_WeaponState.m_Sights = GetSights();
		m_WeaponState.m_SightsZoomFOVInfo = null;
		
		if (m_WeaponState.m_Sights)
			m_WeaponState.m_SightsZoomFOVInfo = SCR_SightsZoomFOVInfo.Cast(m_WeaponState.m_Sights.GetFOVInfo());
		
		if (m_WeaponState.m_SightsZoomFOVInfo)
			m_WeaponState.m_SightsZoomFOVInfo.GetEventOnZoomChanged().Insert(OnZoomChanged);
		
		// Auto-trigger child events
		#ifndef WEAPON_INFO_BLOCK_WATERFALL_EVENTS
		OnMuzzleChanged_init(weapon);
		OnADSChanged_init(weapon);
		#endif
	}	
	
	//------------------------------------------------------------------------------------------------
	protected void OnMuzzleChanged_init(BaseWeaponComponent weapon)
	{
		BaseMuzzleComponent muzzle;
		
		if (weapon)
			muzzle = weapon.GetCurrentMuzzle();
		
		#ifdef WEAPON_INFO_DEBUG_WATERFALL_EVENTS
		Print("OnMuzzleChanged_init");
		Print(string.Format("    weapon:     %1", weapon));
		Print(string.Format("    muzzle:     %1", muzzle));
		#endif
		
		OnMuzzleChanged(weapon, muzzle, null);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMuzzleChanged(BaseWeaponComponent weapon, BaseMuzzleComponent muzzle, BaseMuzzleComponent prevMuzzle)
	{
		if (!m_WeaponState)
			return;
		
		#ifdef WEAPON_INFO_DEBUG
		_print("OnMuzzleChanged");
		_print(string.Format("    weapon:     %1", weapon));
		_print(string.Format("    muzzle:     %1", muzzle));
		#endif
		
		m_eWeaponStateEvent |= EWeaponFeature.MUZZLE;
		
		m_WeaponState.m_Muzzle = muzzle;
		
		if (!muzzle)
		{
			// GRENADE
			
			m_WeaponState.m_MuzzleUI = null;
			m_WeaponState.m_bBarrelChambered = false;
			m_WeaponState.m_bBarrelCanBeChambered = false;
		}
		else
		{
			// NON-GRENADE
			
			m_WeaponState.m_MuzzleUI = MuzzleUIInfo.Cast(muzzle.GetUIInfo());
			m_WeaponState.m_bBarrelChambered = muzzle.IsBarrelChambered(muzzle.GetCurrentBarrelIndex());
			m_WeaponState.m_bBarrelCanBeChambered = muzzle.IsChamberingPossible();
		}

		// Update weapon name & caliber (mostly for vehicle weapons)
		UpdateWeaponNameAndCaliber(m_WeaponState);		
				
		// Auto-trigger child events
		#ifndef WEAPON_INFO_BLOCK_WATERFALL_EVENTS
		OnFiremodeChanged_init(weapon, muzzle);
		OnZeroingChanged_init(weapon);
		OnMagazineChanged_init(weapon, muzzle);		
		#endif
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnMagazineChanged_init(BaseWeaponComponent weapon, BaseMuzzleComponent muzzle)
	{
		BaseMagazineComponent magazine;
		
		if (muzzle)
			magazine = muzzle.GetMagazine();
		
		#ifdef WEAPON_INFO_DEBUG_WATERFALL_EVENTS
		_print("OnMagazineChanged_init");
		_print(string.Format("    weapon:       %1", weapon));
		_print(string.Format("    muzzle:     %1", muzzle));
		_print(string.Format("    magazine:     %1", magazine));
		#endif
		
		OnMagazineChanged(weapon, magazine, null);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMagazineChanged(BaseWeaponComponent weapon, BaseMagazineComponent magazine, BaseMagazineComponent prevMagazine)
	{
		if (!m_WeaponState)
			return;
		
		#ifdef WEAPON_INFO_DEBUG
		_print("OnMagazineChanged");
		_print(string.Format("    weapon:       %1", weapon));
		_print(string.Format("    magazine:     %1", magazine));
		#endif
		
		// Set weapon state change flag
		m_eWeaponStateEvent |= EWeaponFeature.MAGAZINE;
		
		m_WeaponState.m_Magazine = magazine;
		
		if (magazine)
			m_WeaponState.m_MagazineUI = MagazineUIInfo.Cast(magazine.GetUIInfo());	
		else
			m_WeaponState.m_MagazineUI = null;

		if (m_WeaponState.m_MagazineUI)
		{
			m_WeaponState.m_eAmmoTypeFlags = m_WeaponState.m_MagazineUI.GetAmmoTypeFlags();
			m_WeaponState.m_bHasSpecialAmmo = !(m_WeaponState.m_eAmmoTypeFlags == 0 || m_WeaponState.m_eAmmoTypeFlags & EAmmoType.FMJ);
		}		

		// Initialize the magazine icon based on magazineUi > muzzleUi > weaponUi
		m_WeaponState.m_MagazineConfig = GetMagazineConfig(m_WeaponState);		
		
		#ifndef WEAPON_INFO_BLOCK_WATERFALL_EVENTS		
		OnMagazineCountChanged_init(weapon, magazine);
		OnAmmoCountChanged_init(weapon, magazine);
		#endif
		
		// Update magazine texture setup
		UpdateMagazineIndicator_Textures(m_WeaponState);
		UpdateAmmoTypeIndicator(m_WeaponState);		
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMagazineCountChanged_init(BaseWeaponComponent weapon, BaseMagazineComponent magazine)
	{
		BaseMuzzleComponent muzzle;
		
		if (weapon)
			muzzle = weapon.GetCurrentMuzzle();
		
		#ifdef WEAPON_INFO_DEBUG_WATERFALL_EVENTS
		_print("OnMagazineCountChanged_init");
		_print(string.Format("    weapon:       %1", weapon));
		_print(string.Format("    muzzle:     %1", muzzle));
		_print(string.Format("    magazine:     %1", magazine));
		#endif
		
		// Get the character/vehicle inventory manager
		InventoryStorageManagerComponent invManager = SCR_WeaponInfo.GetInventoryManager();
		
		if (!weapon || !invManager)
		{
			OnMagazineCountChanged(weapon, 0, false);
			return;
		}
		
		int magazineCount = 0;
		
		if (muzzle)
		{
			// WEAPON WITH MUZZLE -> NON-GRENADE
			
			// Find compatible magazines for this magazine well
			BaseMagazineWell magWell = muzzle.GetMagazineWell();
			
			if (magWell)
			{
				m_pMagazineSearchPredicate.magWellType = magWell.Type();
				array<IEntity> magEntities = new array<IEntity>;
				invManager.FindItems(magEntities, m_pMagazineSearchPredicate);
				magazineCount = magEntities.Count();
			}
			else
			{
				// No magazine well, weapon is probably not reloadable
				magazineCount = 0;
			}
		}
		else
		{
			// WEAPON WITHOUT MUZZLE -> GRENADE
			
			// Find compatible weapons (same grenades for instance)
			m_pPrefabDataPredicate.prefabData = weapon.GetOwner().GetPrefabData();
			array<IEntity> sameWeapons = new array<IEntity>;
			invManager.FindItems(sameWeapons, m_pPrefabDataPredicate);
			
			// Remaining 'mags' count text
			magazineCount = sameWeapons.Count();
		}		
		
		bool isGrenade = m_WeaponState.m_bIsExplosive;
		
		OnMagazineCountChanged(weapon, magazineCount, isGrenade);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMagazineCountChanged(BaseWeaponComponent weapon, int magazineCount, bool isGrenade)
	{
		if (!m_WeaponState || !weapon)
			return;
		
		#ifdef WEAPON_INFO_DEBUG
		_print("OnMagazineCountChanged");
		_print(string.Format("    weapon:        %1", weapon));
		_print(string.Format("    magazineCount: %1", magazineCount));
		_print(string.Format("    isGrenade:     %1", isGrenade));
		#endif
		
		BaseMagazineComponent magazine = weapon.GetCurrentMagazine();
		m_WeaponState.m_Magazine = magazine;
		
		// Set weapon state change flag
		m_eWeaponStateEvent |= EWeaponFeature.MAGAZINE_COUNT;
		
		m_WeaponState.m_iMagCount = magazineCount;
		m_WeaponState.m_bIsExplosive = isGrenade;

		// Update magazine textures for rocket and grenade launchers
		// Magazine for rocket and grenade launchers gets deleted after firing
		if (!magazine)
		{
			UpdateMagazineIndicator_Textures(m_WeaponState);		
			UpdateAmmoTypeIndicator(m_WeaponState);
		}
				
		UpdateMagazineIndicator_Count(m_WeaponState);
	}
		
	//------------------------------------------------------------------------------------------------
	protected void OnAmmoCountChanged_init(BaseWeaponComponent weapon, BaseMagazineComponent magazine)
	{
		BaseMuzzleComponent muzzle;
		
		if (weapon)
			muzzle = weapon.GetCurrentMuzzle();
		
		#ifdef WEAPON_INFO_DEBUG_WATERFALL_EVENTS
		_print("OnAmmoCountChanged_init");
		_print(string.Format("    weapon:       %1", weapon));
		_print(string.Format("    muzzle:     %1", muzzle));
		_print(string.Format("    magazine:     %1", magazine));
		#endif
		
		int ammoCount = 0;
		
		if (magazine)
			ammoCount = magazine.GetAmmoCount();
		
		bool isBarrelChambered = false;
		
		if (muzzle)
			isBarrelChambered = muzzle.IsCurrentBarrelChambered();
		
		OnAmmoCountChanged(weapon, muzzle, magazine, ammoCount, isBarrelChambered);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnAmmoCountChanged(BaseWeaponComponent weapon, BaseMuzzleComponent muzzle, BaseMagazineComponent magazine, int ammoCount, bool isBarrelChambered)
	{
		if (!m_WeaponState)
			return;

		#ifdef WEAPON_INFO_DEBUG
		_print("OnAmmoCountChanged");
		_print(string.Format("    weapon:            %1", weapon));
		_print(string.Format("    muzzle:            %1", muzzle));
		_print(string.Format("    magazine:          %1", magazine));
		_print(string.Format("    ammoCount:         %1", ammoCount));
		_print(string.Format("    isBarrelChambered: %1", isBarrelChambered));
		#endif

		// Set weapon state change flag
		m_eWeaponStateEvent |= EWeaponFeature.AMMOCOUNT;	
		
		m_WeaponState.m_bBarrelChambered = isBarrelChambered;
		
		m_WeaponState.m_iMagAmmoCount = ammoCount;
		m_WeaponState.m_iMagMaxAmmoCount = 1;
		
		if (magazine)
			m_WeaponState.m_iMagMaxAmmoCount = magazine.GetMaxAmmoCount();				
		
		if (m_WeaponState.m_iMagMaxAmmoCount == 0)
			m_WeaponState.m_iMagMaxAmmoCount = 1;
		
		m_WeaponState.m_fMagAmmoPerc = m_WeaponState.m_iMagAmmoCount / m_WeaponState.m_iMagMaxAmmoCount;
		
		if (magazine)
			UpdateMagazineIndicator_Progress(m_WeaponState);
		
		UpdateBulletInChamberIndicator(m_WeaponState);
		
		if (ammoCount == 0 && weapon)
			SCR_LogitechLEDManager.ActivateState(ELogitechLEDState.RELOAD);
	}	
	
	//------------------------------------------------------------------------------------------------
	protected void OnFiremodeChanged_init(BaseWeaponComponent weapon, BaseMuzzleComponent muzzle)
	{
		BaseFireMode firemode;
		
		if (muzzle)
			firemode = muzzle.GetCurrentFireMode();
		
		#ifdef WEAPON_INFO_DEBUG_WATERFALL_EVENTS
		_print("OnFiremodeChanged_init");
		_print(string.Format("    weapon:       %1", weapon));
		_print(string.Format("    muzzle:     %1", muzzle));
		_print(string.Format("    firemode:     %1", firemode));
		#endif		
		
		OnFiremodeChanged(weapon, muzzle, firemode);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnFiremodeChanged(BaseWeaponComponent weapon, BaseMuzzleComponent muzzle, BaseFireMode firemode)
	{
		if (!m_WeaponState)
			return;
		
		#ifdef WEAPON_INFO_DEBUG
		_print("OnFiremodeChanged");
		_print(string.Format("    weapon:   %1", weapon));
		_print(string.Format("    muzzle:   %1", muzzle));
		_print(string.Format("    firemode: %1", firemode));
		#endif	
	
		// Set weapon state change flag
		m_eWeaponStateEvent |= EWeaponFeature.FIREMODE;
		
		if (firemode)
			m_WeaponState.m_FireModeType = firemode.GetFiremodeType();
		else
			m_WeaponState.m_FireModeType = EWeaponFiremodeType.Semiauto;
		
		UpdateFireModeIndicator(m_WeaponState);
	}	
	
	//------------------------------------------------------------------------------------------------
	protected void OnZeroingChanged_init(BaseWeaponComponent weapon)
	{
		int zeroing = 0;
		
		if (weapon)
			zeroing = weapon.GetCurrentSightsZeroing();
		
		#ifdef WEAPON_INFO_DEBUG_WATERFALL_EVENTS
		_print("OnZeroingChanged_init");
		_print(string.Format("    weapon:   %1", weapon));
		_print(string.Format("    zeroing:   %1", zeroing));
		#endif			
		
		OnZeroingChanged(weapon, zeroing);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnZeroingChanged(BaseWeaponComponent weapon, int zeroing)
	{
		if (!m_WeaponState)
			return;
		
		#ifdef WEAPON_INFO_DEBUG
		_print("OnZeroingChanged");
		_print(string.Format("    weapon:  %1", weapon));
		_print(string.Format("    zeroing: %1", zeroing));
		#endif
		
		// Set weapon state change flag
		m_eWeaponStateEvent |= EWeaponFeature.ZEROING;
		
		m_WeaponState.m_iZeroing = zeroing;
		
		UpdateZeroingIndicator(m_WeaponState);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnADSChanged_init(BaseWeaponComponent weapon)
	{
		bool inADS = false;
		
		if (weapon)
			inADS = weapon.IsSightADSActive();

		#ifdef WEAPON_INFO_DEBUG_WATERFALL_EVENTS
		_print("OnADSChanged_init");
		_print(string.Format("    weapon:  %1", weapon));
		_print(string.Format("    inADS: %1", inADS));
		#endif		
				
		OnADSChanged(weapon, inADS);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnADSChanged(BaseWeaponComponent weapon, bool inADS)
	{
		if (!m_WeaponState)
			return;
		
		#ifdef WEAPON_INFO_DEBUG
		_print("OnADSChanged");
		_print(string.Format("    weapon: %1", weapon));
		_print(string.Format("    inADS:  %1", inADS));
		#endif
		
		// Set weapon state change flag
		m_eWeaponStateEvent |= EWeaponFeature.ADS;	
		
		m_WeaponState.m_bInADS = inADS;
		
		#ifndef WEAPON_INFO_BLOCK_WATERFALL_EVENTS		
		OnZoomChanged_init(weapon);
		#endif
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnZoomChanged_init(BaseWeaponComponent weapon)
	{
		float zoom = 0;

		if (m_WeaponState.m_SightsZoomFOVInfo)
			zoom = m_WeaponState.m_SightsZoomFOVInfo.GetCurrentZoom();

		#ifdef WEAPON_INFO_DEBUG_WATERFALL_EVENTS
		_print("OnZoomChanged_init");
		_print(string.Format("    weapon:  %1", weapon));
		_print(string.Format("    zoom: %1", zoom));
		#endif		
				
		OnZoomChanged(zoom, -1);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnZoomChanged(float zoom, float fov)
	{
		if (!m_WeaponState)
			return;
		
		#ifdef WEAPON_INFO_DEBUG
		_print("OnZoomChanged");
		_print(string.Format("    zoom: %1", zoom));
		_print(string.Format("    fov: %1", fov));
		#endif	
	
		// Set weapon state change flag
		m_eWeaponStateEvent |= EWeaponFeature.ZOOM;			
		
		m_WeaponState.m_fZoom = zoom;
			
		UpdateZoomIndicator(m_WeaponState);
	}	
	
	//------------------------------------------------------------------------------------------------
	protected void OnInspectionModeChanged(bool state)
	{
		if (!m_WeaponState)
			return;
		
		#ifdef WEAPON_INFO_DEBUG
		_print("OnInspectionModeChanged");
		_print(string.Format("    state: %1", state));
		#endif	
	
		// Set weapon state change flag
		m_eWeaponStateEvent |= EWeaponFeature.INSPECTION;			
		
		m_WeaponState.m_bInInspectionMode = state;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnAttachmentChanged(WeaponComponent weapon, bool isAttached, IEntity attachmentEntity)
	{
		if (!m_WeaponState)
			return;
		
		#ifdef WEAPON_INFO_DEBUG
		_print("OnAttachmentChanged");
		_print(string.Format("    isAttached: %1", isAttached));
		_print(string.Format("    attachmentEntity: %1", attachmentEntity));
		#endif	
		
		SCR_2DSightsComponent sights = GetSights();
		
		// Sights has changed
		if (sights != m_WeaponState.m_Sights)
		{
			// Remove any possible existing "zoom-changed" invokers
			if (m_WeaponState.m_SightsZoomFOVInfo)
				m_WeaponState.m_SightsZoomFOVInfo.GetEventOnZoomChanged().Remove(OnZoomChanged);
			
			m_WeaponState.m_Sights = sights;
			m_WeaponState.m_SightsZoomFOVInfo = null;
			
			if (m_WeaponState.m_Sights)
				m_WeaponState.m_SightsZoomFOVInfo = SCR_SightsZoomFOVInfo.Cast(m_WeaponState.m_Sights.GetFOVInfo());
			
			if (m_WeaponState.m_SightsZoomFOVInfo)
				m_WeaponState.m_SightsZoomFOVInfo.GetEventOnZoomChanged().Insert(OnZoomChanged);			
		}
		
		if (!sights)
		{
			m_WeaponState.m_Sights = null;
			m_WeaponState.m_SightsZoomFOVInfo = null;		
		}		
	}

	//------------------------------------------------------------------------------------------------
	override protected void DisplayUpdate(IEntity owner, float timeSlice)
	{		
		if (!m_wRoot || !m_WeaponState || !m_WeaponState.m_Weapon)
			return;

		if (m_eWeaponStateEvent != EWeaponFeature.NONE)
		{
			FadeElements();
						
			m_eWeaponStateEvent = EWeaponFeature.NONE;
			
			m_bFadeInOutActive = true;
		};
		
		if (!m_bFadeInOutActive)
			return;
		
		if (m_ZeroingAnimator)
			m_ZeroingAnimator.Update(timeSlice);
		if (m_WeaponInfoPanelAnimator)
			m_WeaponInfoPanelAnimator.Update(timeSlice);
		
		// Print(string.Format("[ANIM STATES] m_WeaponInfoPanelAnimator: %1 | m_ZeroingAnimator: %2", m_WeaponInfoPanelAnimator.GetState(), m_ZeroingAnimator.GetState()));
		
		m_bFadeInOutActive = m_WeaponInfoPanelAnimator.IsRunning() || m_ZeroingAnimator.IsRunning();
	}
	
	
	//------------------------------------------------------------------------------------------------
	protected void FadeElements()
	{
		// Weapon state debug
		#ifdef WEAPON_INFO_DEBUG_STATES			
		string weaponStateDebug = "Weapon event flags: " + m_eWeaponStateEvent.ToString();

		if (m_eWeaponStateEvent & EWeaponFeature.WEAPON)
			weaponStateDebug += "|WEAPON";			
		
		if (m_eWeaponStateEvent & EWeaponFeature.MUZZLE)
			weaponStateDebug += "|MUZZLE";			
					
		if (m_eWeaponStateEvent & EWeaponFeature.MAGAZINE)
			weaponStateDebug += "|MAGAZINE";
		
		if (m_eWeaponStateEvent & EWeaponFeature.AMMOCOUNT)
			weaponStateDebug += "|AMMOCOUNT";
		
		if (m_eWeaponStateEvent & EWeaponFeature.FIREMODE)
			weaponStateDebug += "|FIREMODE";
		
		if (m_eWeaponStateEvent & EWeaponFeature.ZEROING)
			weaponStateDebug += "|ZEROING";						

		if (m_eWeaponStateEvent & EWeaponFeature.ADS)
			weaponStateDebug += "|ADS";	

		if (m_eWeaponStateEvent & EWeaponFeature.MAGAZINE_COUNT)
			weaponStateDebug += "|MAGAZINE_COUNT";		

		if (m_eWeaponStateEvent & EWeaponFeature.INSPECTION)
			weaponStateDebug += "|INSPECTION";			
						
		if (m_eWeaponStateEvent & EWeaponFeature.ZOOM)
			weaponStateDebug += "|ZOOM";								

		if (m_eWeaponStateEvent & EWeaponFeature.MISC)
			weaponStateDebug += "|MISC";				

		Print(weaponStateDebug);
		#endif
		
		// Get some shared states / conditions for fading
		//bool inspectingWeapon = m_CharacterController && m_CharacterController.GetIsInspectionMode();
		bool isGrenade = m_WeaponState && m_WeaponState.m_bIsExplosive;
		bool inADS = m_WeaponState && m_WeaponState.m_bInADS;
		bool inInspection = m_WeaponState && m_WeaponState.m_bInInspectionMode;
					
		// Main weapon UI (magazine, magazine count, firemode) fadein
		bool panelVisibleEvent = m_eWeaponStateEvent != 0;
		bool panelVisibleOverride = isGrenade || inADS || inInspection;
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
	SCR_2DSightsComponent GetSights()
	{
		SightsComponent sights = m_WeaponManager.GetCurrentSights();
		
		if (!sights)
			return null;
		
		return SCR_2DSightsComponent.Cast(sights);
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_MagazineIndicatorConfiguration GetMagazineConfig(SCR_WeaponState state)
	{
		if (!state)
			return null;
		
		SCR_MagazineIndicatorConfiguration config;
		
		if (state.m_Muzzle)
		{		
			// WEAPON WITH MUZZLE
			if (!state.m_MuzzleUI)
			{
				// No muzzle UI ?! Just hide it
				config = null;
			}
			else
			{
								
				if (state.m_Magazine)
				{
					// MAGAZINE IS IN THE GUN
					config = state.m_MagazineUI.m_MagIndicator;

					if (!config)
						config = state.m_MuzzleUI.m_MagIndicator;
				}
				else
				{
					// NO MAGAZINE
					// Take mag configuration from muzzle
					
					config = state.m_MuzzleUI.m_MagIndicator;
				}
			}
		}
		else
		{
			// NO MUZZLE, probably weapon must provide icon
			if (state.m_WeaponUI)
			{
				config = state.m_WeaponUI.m_MagIndicator;
			}
			else
			{
				config = null;
			}
		}	
	
		return config;
	}
		
	//------------------------------------------------------------------------------------------------
	protected void UpdateMagazineIndicator_Textures(SCR_WeaponState state)
	{
		if (!state || !m_Widgets)
			return;
		
		if (!state.m_MagazineConfig)
			state.m_MagazineConfig = GetMagazineConfig(state);
		
		SCR_MagazineIndicatorConfiguration config = state.m_MagazineConfig;
		
		if (!config)
		{
			m_Widgets.m_wMagazineIndicator.SetVisible(false);
		}
		else
		{
			m_Widgets.m_wMagazineIndicator.SetVisible(true);
			
			if (config.m_bProgressBar)
			{
				SetWidgetImage(m_Widgets.m_wMagazineOutline, config.m_sImagesetIcons, config.m_sOutline, SIZE_MAGAZINE);
				SetWidgetImage(m_Widgets.m_wMagazineBackground, config.m_sImagesetIcons, config.m_sBackground, SIZE_MAGAZINE);
				SetWidgetImage(m_Widgets.m_wMagazineGlow, config.m_sImagesetGlows, config.m_sBackground, SIZE_MAGAZINE);			
			}
			else
			{
				SetWidgetImage(m_Widgets.m_wMagazineOutline, config.m_sImagesetIcons, config.m_sOutline, SIZE_MAGAZINE);
				//SetWidgetImage(m_Widgets.m_wMagazineBackground, config.m_sImagesetIcons, config.m_sOutline, SIZE_MAGAZINE);
				SetWidgetImage(m_Widgets.m_wMagazineGlow, config.m_sImagesetGlows, config.m_sOutline, SIZE_MAGAZINE);			
			}
			
			// Setup textures for reloading indicator
			SetWidgetImage(m_Widgets.m_wReloadBackground, config.m_sImagesetIcons, config.m_sProgress, SIZE_MAGAZINE);
			m_Widgets.m_wReloadBackground.SetVisible(false);			
			SetWidgetImage(m_Widgets.m_wReloadOutline, config.m_sImagesetIcons, config.m_sOutline, SIZE_MAGAZINE);
			m_Widgets.m_wReloadOutline.SetVisible(false);

			
			// Setup visibility, keep only Outline texture, if there is no progress indication
			m_Widgets.m_wMagazineProgress.SetVisible(config.m_bProgressBar && state.m_Magazine);
			m_Widgets.m_wMagazineBackground.SetVisible(config.m_bProgressBar);			
			m_Widgets.m_wMagazineOutline.SetOpacity(1);
						
			if (state.m_Magazine)
			{
				if (config.m_bProgressBar)
				{
					SetWidgetImage(m_Widgets.m_wMagazineProgress, config.m_sImagesetIcons, config.m_sProgress, SIZE_MAGAZINE);
					m_Widgets.m_wMagazineProgress.LoadMaskFromSet(config.m_sImagesetIcons, config.m_sProgressAlphaMask);
					m_Widgets.m_wMagazineProgress.SetMaskMode(ImageMaskMode.REGULAR);
				}
			}
			else
			{
				if (!state.m_bIsExplosive)
					m_Widgets.m_wMagazineOutline.SetOpacity(FADED_OPACITY);
			}
			
			//AnimateWidget_ColorFlash(m_Widgets.m_wMagazineIndicator);
		}
	}	
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateMagazineIndicator_Progress(SCR_WeaponState state)
	{
		if (!state || !m_Widgets)
			return;
		
		//AnimateWidget_ColorFlash(m_Widgets.m_wMagazineIndicator, EWeaponFeature.AMMOCOUNT);
		
		m_Widgets.m_wMagazineProgress.SetMaskProgress(state.m_fMagAmmoPerc);
	}			

	//------------------------------------------------------------------------------------------------
	protected void UpdateMagazineIndicator_Count(SCR_WeaponState state)
	{
		if (!state || !m_Widgets)
			return;

		int count = state.m_iMagCount;
		
		// Add +1 if weapon is a grenade, so it shows the total # of grenades in possession
		if (state.m_bIsExplosive)
			count++;

		string countText = count.ToString();
		
		/*
		if (!state.m_bIsExplosive)
			countText = "+" + countText;
		*/
		
		if (count > 0)
			m_Widgets.m_wMagCountText.SetOpacity(1);
		else
			m_Widgets.m_wMagCountText.SetOpacity(FADED_OPACITY);
		
		if (m_Widgets.m_wMagCountText.GetText() == countText) 
			return;

		m_Widgets.m_wMagCountText.SetText(countText);		
		
		AnimateWidget_ColorFlash(m_Widgets.m_wMagCountText, EWeaponFeature.MAGAZINE_COUNT);	
		AnimateWidget_TextPopUp(m_Widgets.m_wMagCountText, 36, 54, EWeaponFeature.MAGAZINE_COUNT);
	}		
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateBulletInChamberIndicator(SCR_WeaponState state)
	{
		if (!state || !m_Widgets)
			return;

		if (state.m_bBarrelChambered)
			m_Widgets.m_wFiremodeIcon.SetOpacity(1);
		else
			m_Widgets.m_wFiremodeIcon.SetOpacity(FADED_OPACITY);
	}		
	
	//------------------------------------------------------------------------------------------------
	void UpdateFireModeIndicator(SCR_WeaponState state)
	{
		if (!state || !m_Widgets)
			return;

		m_Widgets.m_wFiremodeIcon.SetVisible(false);
		m_Widgets.m_wFiremodeGlow.SetVisible(false);
		
		if (state.m_Muzzle && state.m_MuzzleUI && state.m_MuzzleUI.ShowFiremodeIcon())
		{
			EWeaponFiremodeType fm = state.m_FireModeType;
			string icon = state.m_MuzzleUI.GetFiremodeIconName(fm);
			
			if (icon != string.Empty)
			{
				SetWidgetImage(m_Widgets.m_wFiremodeIcon, state.m_MuzzleUI.GetFiremodeIconImageset(), icon, SIZE_FIREMODE);
				SetWidgetImage(m_Widgets.m_wFiremodeGlow, state.m_MuzzleUI.GetFiremodeGlowImageset(), icon, SIZE_FIREMODE);
			
				m_Widgets.m_wFiremodeIcon.SetVisible(true);
				m_Widgets.m_wFiremodeGlow.SetVisible(true);
				
				AnimateWidget_ColorFlash(m_Widgets.m_wFiremodeIcon, EWeaponFeature.FIREMODE);
			}
		}		
	}

	//------------------------------------------------------------------------------------------------
	void UpdateZeroingIndicator(SCR_WeaponState state)
	{
		if (!state || !m_Widgets)
			return;

		int zeroing = state.m_iZeroing;

		if (zeroing <= 0)
		{
			m_Widgets.m_wZeroing.SetVisible(false);
			m_Widgets.m_wZeroingText.SetText(string.Empty);
		}
		else
		{
			m_Widgets.m_wZeroing.SetVisible(true);
			string sZeroing = string.Format("%1 m", zeroing);
			m_Widgets.m_wZeroingText.SetText(sZeroing);
			
			AnimateWidget_ColorFlash(m_Widgets.m_wZeroing, EWeaponFeature.ZEROING);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateZoomIndicator(SCR_WeaponState state)
	{
		if (!state || !m_Widgets)
			return;

		float zoom = state.m_fZoom;

		if (zoom <= 0 || !state.m_bInADS)
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
	void UpdateAmmoTypeIndicator(SCR_WeaponState state)
	{
		if (!state || !m_Widgets)
			return;
		
		if (!state.m_bHasSpecialAmmo || (!state.m_MagazineUI && !state.m_GrenadeUI))
		{
			m_Widgets.m_wAmmoType.SetVisible(false);
			return;
		}

		string sAmmoTypeText;
		bool bShowAmmoTypeText;
		
		if (state.m_GrenadeUI)
		{
			sAmmoTypeText = state.m_GrenadeUI.GetAmmoType();
			bShowAmmoTypeText = state.m_GrenadeUI.ShowAmmoTypeText();
		}
		else
		{
			sAmmoTypeText = state.m_MagazineUI.GetAmmoType();
			bShowAmmoTypeText = state.m_MagazineUI.ShowAmmoTypeText();
		}
		
		if (!bShowAmmoTypeText)
			sAmmoTypeText = "";
		
		m_Widgets.m_wAmmoTypeText.SetText(sAmmoTypeText);

		m_Widgets.m_wAmmoType.SetVisible(true);
				
		m_Widgets.m_wAmmoType_FMJ.SetVisible(state.m_eAmmoTypeFlags & EAmmoType.FMJ);
		m_Widgets.m_wAmmoType_AP.SetVisible(state.m_eAmmoTypeFlags & EAmmoType.AP);
		m_Widgets.m_wAmmoType_Frag.SetVisible(state.m_eAmmoTypeFlags & EAmmoType.FRAG);
		m_Widgets.m_wAmmoType_Smoke.SetVisible(state.m_eAmmoTypeFlags & EAmmoType.SMOKE);
		m_Widgets.m_wAmmoType_HE.SetVisible(state.m_eAmmoTypeFlags & EAmmoType.HE);
		m_Widgets.m_wAmmoType_HEAT.SetVisible(state.m_eAmmoTypeFlags & EAmmoType.HEAT);
		m_Widgets.m_wAmmoType_Incendiary.SetVisible(state.m_eAmmoTypeFlags & EAmmoType.INCENDIARY);
		m_Widgets.m_wAmmoType_Tracer.SetVisible(state.m_eAmmoTypeFlags & EAmmoType.TRACER);
		m_Widgets.m_wAmmoType_Sniper.SetVisible(state.m_eAmmoTypeFlags & EAmmoType.SNIPER);
		m_Widgets.m_wAmmoType_Illumination.SetVisible(state.m_eAmmoTypeFlags & EAmmoType.ILLUMINATION);
		
		//AnimateWidget_ColorFlash(m_Widgets.m_wAmmoType);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateWeaponNameAndCaliber(SCR_WeaponState state)
	{
		if (!state || !m_Widgets || !state.m_WeaponUI)
			return;
		
		// Update weapon name indicator
		bool showName = state.m_WeaponUI.ShowWeaponName();
		
		if (showName)
			m_Widgets.m_wWeaponName.SetText(state.m_WeaponUI.GetName());
		
		m_Widgets.m_wWeaponName.SetVisible(showName);
		
		// Update muzzle caliber indicator
		bool showCaliber;
		
		if (state.m_MuzzleUI)
		{
			showCaliber = state.m_MuzzleUI.ShowCaliber();
			
			if (showCaliber)
				m_Widgets.m_wCaliber.SetText(state.m_MuzzleUI.GetCaliber());
		}
		
		m_Widgets.m_wCaliber.SetVisible(showCaliber);
	}		
	
	//------------------------------------------------------------------------------------------------
	void AnimateWidget_ColorFlash(Widget w, EWeaponFeature requiredFlag = -1, float speed = UIConstants.FADE_RATE_SLOW)
	{
		if (!w)
			return;
		
		if (requiredFlag != -1 && m_eWeaponStateEvent != requiredFlag)
			return;
		
		w.SetColor(COLOR_ORANGE);
		AnimateWidget.Color(w, COLOR_WHITE, speed);
	}	

	//------------------------------------------------------------------------------------------------
	void AnimateWidget_TextPopUp(Widget w, float size, float sizeBoosted, EWeaponFeature requiredFlag = -1, float speed = UIConstants.FADE_RATE_SLOW)
	{
		if (!w)
			return;
		
		if (requiredFlag != -1 && m_eWeaponStateEvent != requiredFlag)
			return;
		
		float width = FrameSlot.GetSizeX(w);
		
		FrameSlot.SetSize(w, width, sizeBoosted);
		float widthHeight[2] = {width, size};
		AnimateWidget.Size(w, widthHeight, speed);
	}		
		
	//------------------------------------------------------------------------------------------------
	override protected bool DisplayStartDrawInit(IEntity owner)
	{
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(owner);
		if (!character)
			return false;

		m_WeaponManager = character.GetWeaponManager();
		if (!m_WeaponManager)
			return false;

		m_CompartmentAccess = SCR_CompartmentAccessComponent.Cast(character.GetCompartmentAccessComponent());
		if (!m_CompartmentAccess)
			return false;
		
		m_MenuManager = GetGame().GetMenuManager();
		if (!m_MenuManager)
			return false;
		
		// Register weapon related weapon event handlers
		AddEventHandlers(owner);
		
		// Fallback in case the layout is not provided as an InfoDisplay attribute
		if (m_LayoutPath == string.Empty)
			m_LayoutPath = DEFAULT_WEAPON_INFO_LAYOUT;
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void DisplayStartDraw(IEntity owner)
	{
		if (!m_wRoot)
			return;
		
		m_Widgets = new SCR_WeaponInfoWidgets();
		m_Widgets.Init(m_wRoot);
		
		m_Widgets.m_wMagazineIndicator.SetVisible(true);
		
		if (!m_WeaponInfoPanelAnimator)
			m_WeaponInfoPanelAnimator = new SCR_FadeInOutAnimator(m_Widgets.m_wWeaponInfoPanel, UIConstants.FADE_RATE_FAST, UIConstants.FADE_RATE_SLOW, FADEOUT_PANEL_DELAY, true);

		if (!m_ZeroingAnimator)
			m_ZeroingAnimator = new SCR_FadeInOutAnimator(m_Widgets.m_wZeroing, UIConstants.FADE_RATE_FAST, UIConstants.FADE_RATE_SLOW, FADEOUT_OPTICS_DELAY, true);	
		
		// Initialize the UI based on what weapon character currently has
		OnWeaponChanged(m_WeaponManager.GetCurrentWeapon(), null);		
	}	
		
	//------------------------------------------------------------------------------------------------
	override protected void DisplayStopDraw(IEntity owner)
	{
		if (m_WeaponState && m_WeaponState.m_SightsZoomFOVInfo)
			m_WeaponState.m_SightsZoomFOVInfo.GetEventOnZoomChanged().Remove(OnZoomChanged);
		
		m_WeaponState = null;
		
		// Un-register weapon related weapon event handlers
		RemoveEventHandlers(owner);
		
		// Destroy the animators
		m_WeaponInfoPanelAnimator = null;
		m_ZeroingAnimator = null;
		
		m_wRoot = null;
	}

	//------------------------------------------------------------------------------------------------
	// Called when the visibility flags no longer suspend the GUI; e.g. GM left and GUI marked as not to show in GM -> GUI can show again
	// Doesn't mean the GUI is visible, it is just not hidded due to visibility flags; use m_bShown to check the visibility
	override protected void DisplayOnResumed()
	{
		super.DisplayOnResumed();

		if (m_WeaponManager)
			OnWeaponChanged(m_WeaponManager.GetCurrentWeapon(), null); // treat this as if we have changed the weapon, to properly reevaluate if we should show this HUD element
	}

	//------------------------------------------------------------------------------------------------
	protected void AddEventHandlers(IEntity owner)
	{
		if (m_EventHandlerManager)
		{
			m_EventHandlerManager.RegisterScriptHandler("OnWeaponChanged", this, OnWeaponChanged);
			m_EventHandlerManager.RegisterScriptHandler("OnMuzzleChanged", this, OnMuzzleChanged);
			m_EventHandlerManager.RegisterScriptHandler("OnMagazineChanged", this, OnMagazineChanged);
			m_EventHandlerManager.RegisterScriptHandler("OnMagazineCountChanged", this, OnMagazineCountChanged);
			m_EventHandlerManager.RegisterScriptHandler("OnAmmoCountChanged", this, OnAmmoCountChanged);
			m_EventHandlerManager.RegisterScriptHandler("OnFiremodeChanged", this, OnFiremodeChanged);
			m_EventHandlerManager.RegisterScriptHandler("OnZeroingChanged", this, OnZeroingChanged);
			m_EventHandlerManager.RegisterScriptHandler("OnADSChanged", this, OnADSChanged);
			m_EventHandlerManager.RegisterScriptHandler("OnInspectionModeChanged", this, OnInspectionModeChanged);
			m_EventHandlerManager.RegisterScriptHandler("OnWeaponAttachmentChanged", this, OnAttachmentChanged);
		}		
	}

	//------------------------------------------------------------------------------------------------
	protected void RemoveEventHandlers(IEntity owner)
	{
		if (m_EventHandlerManager)
		{
			m_EventHandlerManager.RemoveScriptHandler("OnWeaponChanged", this, OnWeaponChanged);
			m_EventHandlerManager.RemoveScriptHandler("OnMuzzleChanged", this, OnMuzzleChanged);
			m_EventHandlerManager.RemoveScriptHandler("OnMagazineChanged", this, OnMagazineChanged);
			m_EventHandlerManager.RemoveScriptHandler("OnMagazineCountChanged", this, OnMagazineCountChanged);
			m_EventHandlerManager.RemoveScriptHandler("OnAmmoCountChanged", this, OnAmmoCountChanged);
			m_EventHandlerManager.RemoveScriptHandler("OnFiremodeChanged", this, OnFiremodeChanged);
			m_EventHandlerManager.RemoveScriptHandler("OnZeroingChanged", this, OnZeroingChanged);
			m_EventHandlerManager.RemoveScriptHandler("OnADSChanged", this, OnADSChanged);
			m_EventHandlerManager.RemoveScriptHandler("OnInspectionModeChanged", this, OnInspectionModeChanged);
			m_EventHandlerManager.RemoveScriptHandler("OnWeaponAttachmentChanged", this, OnAttachmentChanged);
		}
		m_EventHandlerManager = null;	
	}	
	
	//------------------------------------------------------------------------------------------------
	//! Returns player's inventory manager, or vehicle's inv manager if player is in vehicle
	protected static InventoryStorageManagerComponent GetInventoryManager()
	{		
		PlayerController pc = GetGame().GetPlayerController();
			
		if (!pc)
			return null;
		
		IEntity playerEntity = pc.GetControlledEntity();
		
		if (!playerEntity)
			return null;
		
		InventoryStorageManagerComponent invManager = InventoryStorageManagerComponent.Cast(playerEntity.FindComponent(InventoryStorageManagerComponent));
		
		CompartmentAccessComponent compAccess = CompartmentAccessComponent.Cast(playerEntity.FindComponent(CompartmentAccessComponent));
		
		if (!compAccess)
			return invManager;
		
		BaseCompartmentSlot compSlot = compAccess.GetCompartment();
		
		if (!compSlot)
			return invManager;

		IEntity vehEntity = compSlot.GetVehicle();
		InventoryStorageManagerComponent vehInvMgr = InventoryStorageManagerComponent.Cast(
			vehEntity.FindComponent(InventoryStorageManagerComponent));
		
		if (!vehInvMgr)
		{
			vehEntity = compSlot.GetOwner();
			vehInvMgr = InventoryStorageManagerComponent.Cast(vehEntity.FindComponent(InventoryStorageManagerComponent));		
		}
		
		if (!vehInvMgr)
			return null;
		
		return vehInvMgr;
	}
	
	//------------------------------------------------------------------------------------------------
	protected BaseWeaponManagerComponent GetWeaponManager(IEntity owner)	
	{
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(owner);
		if (!character)
			return null;

		// Detect and store weapon manager
		BaseWeaponManagerComponent weaponManager = BaseWeaponManagerComponent.Cast(character.FindComponent(BaseWeaponManagerComponent));
		if (!weaponManager)
			return null;
		
		return weaponManager;
	}
		
	//---------------------------------------------------------------------------------------------------------
	//! Sets widget's image to an image or imageset
	protected void SetWidgetImage(ImageWidget w, string imageOrImageset, string imageName = "", int size = -1)
	{
		if (!imageName.IsEmpty())
		{
			// Assume it's an image set
			w.LoadImageFromSet(0, imageOrImageset, imageName);
		}
		else if (!imageOrImageset.IsEmpty())
		{
			// Assume it's an image
			w.LoadImageTexture(0, imageOrImageset);
		}

		if (size == -1)
			return;
				
		// Perform resizing
		int sx, sy;
		w.GetImageSize(0, sx, sy);
		
		float ratio = sx / sy;
		
		w.SetSize(size * ratio, size);
	}
};