class SCR_AISwitchMagazine : SCR_AIWeaponHandlingBase
{
	protected static const string PORT_MAGAZINE_COMPONENT = "MagazineComponent";
	
	// Used for reloading of turret
	// Acrual reload timer is not implemented in gamecode yet for AI, so we fake the reload time
	protected bool m_bReloadingTimer;
	protected float m_fReloadFinishTime_ms;
	
	//--------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_WeaponMgrComp || !m_ControlComp || !m_InventoryMgr)
			return ENodeResult.FAIL;
		
		BaseMagazineComponent newMagazineComp = null;
		GetVariableIn(PORT_MAGAZINE_COMPONENT, newMagazineComp);
		
		if (!newMagazineComp)
		{
			#ifdef AI_DEBUG
			AddDebugMessage("Skipped magazine switch, no magazine was provided");
			#endif
			return ENodeResult.SUCCESS;
		}
		
		// Resolve which weapon manager to use
		BaseCompartmentSlot compartmentSlot = m_CompartmentAccessComp.GetCompartment();
		BaseWeaponManagerComponent weaponMgr;
		if (compartmentSlot)
			weaponMgr = BaseWeaponManagerComponent.Cast(compartmentSlot.GetOwner().FindComponent(BaseWeaponManagerComponent));
		else
			weaponMgr = m_WeaponMgrComp;
		
		// Check if we already have this magazine
		BaseMagazineComponent currentMagazineComp = SCR_AIWeaponHandling.GetCurrentMagazineComponent(weaponMgr);
		if (currentMagazineComp == newMagazineComp)
		{
			if (!compartmentSlot)
			{
				if (m_ControlComp.IsReloading())
					return ENodeResult.RUNNING;
				
				if (!SCR_AIWeaponHandling.IsCurrentMuzzleChambered(weaponMgr, true))
				{
					#ifdef AI_DEBUG
					AddDebugMessage("Muzzle is not chambered, requesting reload");
					#endif
					
					m_ControlComp.ReloadWeapon();
					return ENodeResult.RUNNING;
				}
			}
			
			
			#ifdef AI_DEBUG
			AddDebugMessage("Magazine switch completed");
			#endif
			return ENodeResult.SUCCESS;
		}
		
		// Switch magazine
		if (compartmentSlot)
		{
			//-----------------------------------------
			// Turret magazine switching
			
			IEntity compartmentParentEntity = compartmentSlot.GetOwner();
			TurretControllerComponent turretController = TurretControllerComponent.Cast(compartmentParentEntity.FindComponent(TurretControllerComponent));
			InventoryStorageManagerComponent turretInventoryMgr = turretController.GetInventoryManager();
			
			if (turretInventoryMgr.Contains(newMagazineComp.GetOwner()))
			{
				if (!m_bReloadingTimer)
				{	
					// Start reloading timer
					m_bReloadingTimer = true;
					float reloadDuration_ms = 1000 * turretController.GetReloadDuration();
					m_fReloadFinishTime_ms = GetGame().GetWorld().GetWorldTime() + reloadDuration_ms;
					
					#ifdef AI_DEBUG
					AddDebugMessage(string.Format("Started turret reload timer: %1s %2 %3 %4", reloadDuration_ms/1000.0, newMagazineComp, newMagazineComp.GetOwner(), newMagazineComp.GetOwner().GetPrefabData().GetPrefabName()));
					#endif
				}
				else
				{
					float currentTime_ms = GetGame().GetWorld().GetWorldTime();
					if (currentTime_ms >= m_fReloadFinishTime_ms)
					{
						m_bReloadingTimer = false;
						
						#ifdef AI_DEBUG
						AddDebugMessage(string.Format("StartMagazineSwitchTurret() %1 %2 %3", newMagazineComp, newMagazineComp.GetOwner(), newMagazineComp.GetOwner().GetPrefabData().GetPrefabName()));
						#endif
						SCR_AIWeaponHandling.StartMagazineSwitchTurret(turretController, newMagazineComp);
					}
				}
				
				return ENodeResult.RUNNING;
			}
			else
			{
				#ifdef AI_DEBUG
				AddDebugMessage("Failed magazine switch, the magazine was not found in turret inventory");
				#endif
				return ENodeResult.FAIL;
			}
		}
		else
		{
			//-----------------------------------------
			// Character magazine switching
			
			if (m_ControlComp.IsReloading())
			{
				return ENodeResult.RUNNING;
			}
			else
			{
				// Start magazine change
				// Ensure that the magazine is indeed in our inventory
				// This check prevents AI equipping a rocket he just shot
				if (m_InventoryMgr.Contains(newMagazineComp.GetOwner()))
				{
					#ifdef AI_DEBUG
					AddDebugMessage(string.Format("StartMagazineSwitchCharacter() %1 %2 %3", newMagazineComp, newMagazineComp.GetOwner(), newMagazineComp.GetOwner().GetPrefabData().GetPrefabName()));
					#endif
					SCR_AIWeaponHandling.StartMagazineSwitchCharacter(m_ControlComp, newMagazineComp);
					return ENodeResult.RUNNING;
				}
				else
				{
					#ifdef AI_DEBUG
					AddDebugMessage("Failed magazine switch, the magazine was not found in inventory");
					#endif
					return ENodeResult.FAIL;
				}
			}
		}
		
		return ENodeResult.FAIL;
	}
	
	//--------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {PORT_MAGAZINE_COMPONENT};
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	
	static override bool VisibleInPalette() { return true; }
}