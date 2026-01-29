class SCR_WeaponInfoVehicle : SCR_WeaponInfo
{
	[Attribute(desc: "Name of the compartment for which this element should be enabled.\nLeave empty if it is meant for all compartments in this entity")]
	protected string m_sCompartmentName;

	protected TurretComponent m_Turret;
	protected TurretControllerComponent m_TurretController;

	//------------------------------------------------------------------------------------------------
	override protected void DisplayUpdate(IEntity owner, float timeSlice)
	{
		if (!m_wRoot || !m_WeaponState)
			return;

		if (m_WeaponState.m_bReloading)
		{
			// Set weapon state change flag
			m_eWeaponStateEvent |= EWeaponFeature.RELOADING;
		
			UpdateMagazineIndicator_Reloading(false);
		}
		
		super.DisplayUpdate(owner, timeSlice);		
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnMuzzleChanged(BaseWeaponComponent weapon, BaseMuzzleComponent muzzle, BaseMuzzleComponent prevMuzzle)
	{	
		super.OnMuzzleChanged(weapon, muzzle, prevMuzzle);
		
		// Auto-trigger child events
		#ifndef WEAPON_INFO_BLOCK_WATERFALL_EVENTS
		OnTurretReload_init(weapon);
		#endif		
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnTurretReload_init(BaseWeaponComponent weapon)
	{
		float rldTime = m_TurretController.GetReloadTime();
		
		if (rldTime == 0)
			return;
		
		#ifdef WEAPON_INFO_DEBUG_WATERFALL_EVENTS
		Print("OnTurretReload_init");
		Print(string.Format("    weapon:     %1", weapon));
		Print(string.Format("    finished:     %1", false));
		#endif
		
		OnTurretReload(weapon, false, m_TurretController);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTurretReload(BaseWeaponComponent weapon, bool finished, TurretControllerComponent turretController)
	{
		if (!m_WeaponState)
			return;
		
		#ifdef WEAPON_INFO_DEBUG
		_print("OnTurretReload");
		_print(string.Format("    weapon:   %1", weapon));
		_print(string.Format("    finished:   %1", finished));
		_print(string.Format("    turretController: %1", turretController));		
		#endif	
		
		if (m_TurretController != turretController)
			return;
		
		m_WeaponState.m_bReloading = !finished;
		
		UpdateMagazineIndicator_Reloading(finished);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateMagazineIndicator_Reloading(bool finished)
	{
		float rldProgress = 0;
		
		if (!finished)
		{
			float rldTime = m_TurretController.GetReloadTime();
			float rldDuration = m_TurretController.GetReloadDuration();		

			if (rldDuration > 0)	
				rldProgress = Math.Clamp((rldDuration - rldTime) / rldDuration, 0, 1);
		}
		
		m_Widgets.m_wReloadOutline.SetMaskProgress(rldProgress);
		m_Widgets.m_wReloadOutline.SetVisible(rldProgress > 0);
		
		m_Widgets.m_wReloadBackground.SetMaskProgress(rldProgress);
		m_Widgets.m_wReloadBackground.SetVisible(rldProgress > 0);
	}	
	
	//------------------------------------------------------------------------------------------------
	override protected bool DisplayStartDrawInit(IEntity owner)
	{
		if(!owner)
			return false;
		
		m_Turret = TurretComponent.Cast(owner.FindComponent(TurretComponent));

		m_TurretController = TurretControllerComponent.Cast(owner.FindComponent(TurretControllerComponent));
		if(!m_TurretController)
			return false;
		
		// Detect and store weapon manager
		m_WeaponManager = m_TurretController.GetWeaponManager();
		if (!m_WeaponManager)
			return false;	

		m_MenuManager = GetGame().GetMenuManager();
		if (!m_MenuManager)
			return false;
				
		AddEventHandlers(owner);
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void DisplayStartDraw(IEntity owner)
	{
		super.DisplayStartDraw(owner);

		ChimeraCharacter character = ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		if (!character)
			return;
		
		m_CompartmentAccess = SCR_CompartmentAccessComponent.Cast(character.GetCompartmentAccessComponent());
		if (!m_CompartmentAccess)
			return;

		BaseCompartmentSlot compartment = m_CompartmentAccess.GetCompartment();
		if (!compartment)
			return;

		m_CompartmentAccess.GetOnCompartmentEntered().Insert(OnCompartmentEntered); // watch if player has switched to a different compartment in this entity
		EvaluateCurrentCompartment(compartment);
	}

	//------------------------------------------------------------------------------------------------
	override protected void DisplayStopDraw(IEntity owner)
	{
		super.DisplayStopDraw(owner);

		if (!m_CompartmentAccess)
			return;

		m_CompartmentAccess.GetOnCompartmentEntered().Remove(OnCompartmentEntered);
		m_CompartmentAccess = null;
	}

	//------------------------------------------------------------------------------------------------
	//! Evaluates if compartment requirements were met, and if so, then it will try to show the HUD element, otherwise it will try to hide it.
	//! \param[in] currentCompartment
	protected void EvaluateCurrentCompartment(notnull BaseCompartmentSlot currentCompartment)
	{
		if (m_sCompartmentName.IsEmpty())
			return;

		if (m_sCompartmentName == currentCompartment.GetCompartmentName())
		{
			Show(true);
			if (m_eWeaponStateEvent == 0)
				m_eWeaponStateEvent = EWeaponFeature.WEAPON; // we just took control of a weapon thus we need to show HUD for it

			return;
		}

		Show(false);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCompartmentEntered(IEntity targetEntity, BaseCompartmentManagerComponent manager, int mgrID, int slotID, bool move)
	{
		BaseCompartmentSlot compartment = m_CompartmentAccess.GetCompartment();
		if (compartment)
			EvaluateCurrentCompartment(compartment);
	}

	//------------------------------------------------------------------------------------------------
	override protected void DisplayControlledEntityChanged(IEntity from, IEntity to)
	{
		super.DisplayControlledEntityChanged(from, to);

		if (!m_CompartmentAccess)
			return;

		if (m_CompartmentAccess.GetOwner() != from)
			return;

		m_CompartmentAccess.GetOnCompartmentEntered().Remove(OnCompartmentEntered);
		m_CompartmentAccess = null;
	}

	//------------------------------------------------------------------------------------------------
	override protected void AddEventHandlers(IEntity owner)
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
			m_EventHandlerManager.RegisterScriptHandler("OnTurretReload", this, OnTurretReload);
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void RemoveEventHandlers(IEntity owner)
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
			m_EventHandlerManager.RemoveScriptHandler("OnTurretReload", this, OnTurretReload);
		}
		m_EventHandlerManager = null;	
	}	
			
	//------------------------------------------------------------------------------------------------
	override SCR_2DSightsComponent GetSights()
	{
		if (!m_Turret)
			return null;
		
		SCR_2DSightsComponent sights = SCR_2DSightsComponent.Cast(m_Turret.FindComponent(SCR_2DSightsComponent));
		
		return sights;
	}
};
