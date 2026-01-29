class SCR_AISwitchWeaponMovedCallback : ScriptedInventoryOperationCallback
{
	ref SCR_AISwitchWeapon m_Task;
	
	//------------------------------------------------------------------------------------------------
	void SCR_AISwitchWeaponMovedCallback(SCR_AISwitchWeapon task)
	{
		m_Task = task;
	}
	
	void Invalidate() { m_Task = null; }
	
	//------------------------------------------------------------------------------------------------
	override protected void OnComplete()
	{
		if (m_Task)
		{
			m_Task.OnMoveWeaponFinished(true);
			Invalidate();
		}
			
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnFailed()
	{
		if (m_Task)
		{
			m_Task.OnMoveWeaponFinished(false);
			Invalidate();
		}

	}
};


class SCR_AISwitchWeapon : SCR_AIWeaponHandlingBase
{
	protected static const int WEAPON_STATE_IDLE = 0;
	protected static const int WEAPON_STATE_MOVING = 1;
	protected static const int WEAPON_STATE_MOVED = 2;	
	
	protected static const string PORT_WEAPON_COMPONENT = "WeaponComponent";
	
	protected int m_iWeaponState;
	
	protected EquipedWeaponStorageComponent m_EqWeaponStorageComp;
	protected ref SCR_AISwitchWeaponMovedCallback m_WeaponMoveCallback;
	
	//--------------------------------------------------------------------------------------------
	void OnMoveWeaponFinished(bool result)
	{
		#ifdef AI_DEBUG
		AddDebugMessage(string.Format("OnMoveWeaponFinished: %1", result));
		#endif
		
		m_iWeaponState = WEAPON_STATE_MOVED;
	}
	
	//--------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_WeaponMgrComp || !m_ControlComp || !m_InventoryMgr)
			return ENodeResult.FAIL;
		
		BaseWeaponComponent newWeaponComp = null;
		GetVariableIn(PORT_WEAPON_COMPONENT, newWeaponComp);
		if (!newWeaponComp)
			return ENodeResult.FAIL;
		
		// Resolve which weapon manager to use
		BaseCompartmentSlot compartmentSlot = m_CompartmentAccessComp.GetCompartment();
		BaseWeaponManagerComponent weaponMgr;
		if (compartmentSlot)
			weaponMgr = BaseWeaponManagerComponent.Cast(compartmentSlot.GetOwner().FindComponent(BaseWeaponManagerComponent));
		else
			weaponMgr = m_WeaponMgrComp;
		
		if (!weaponMgr)
			return ENodeResult.FAIL;
		
		BaseWeaponComponent currentWeaponComp = weaponMgr.GetCurrentWeapon();
		
		if (compartmentSlot)
		{
			//-----------------------------------------
			// Turret weapon switching
			
			IEntity compartmentParentEntity = compartmentSlot.GetOwner();
			TurretControllerComponent turretController = TurretControllerComponent.Cast(compartmentParentEntity.FindComponent(TurretControllerComponent));
			
			if (!turretController)
			{
				#ifdef AI_DEBUG
				AddDebugMessage("Weapon switch failed: no turret controller on turret", LogLevel.WARNING);
				#endif
				return ENodeResult.FAIL;
			}
			
			if (currentWeaponComp == newWeaponComp) // Return success if done
			{
				#ifdef AI_DEBUG
				AddDebugMessage("Weapon switch completed");
				#endif
				return ENodeResult.SUCCESS;
			}
			
			#ifdef AI_DEBUG
			AddDebugMessage(string.Format("StartWeaponSwitchTurret: %1 %2 %3", newWeaponComp, newWeaponComp.GetOwner(), newWeaponComp.GetOwner().GetPrefabData().GetPrefabName()));
			#endif
			SCR_AIWeaponHandling.StartWeaponSwitchTurret(turretController, newWeaponComp, owner.GetControlledEntity());
			return ENodeResult.RUNNING;
		}
		else
		{
			//-----------------------------------------
			// Character weapon switching
			
			if (m_ControlComp.IsChangingItem())
				return ENodeResult.RUNNING;
			
			if (currentWeaponComp == newWeaponComp) // Return success if done
			{
				#ifdef AI_DEBUG
				AddDebugMessage("Weapon switch completed");
				#endif
				return ENodeResult.SUCCESS;
			}
			
			if (!m_InventoryMgr.Contains(newWeaponComp.GetOwner()))
			{
				#ifdef AI_DEBUG
				AddDebugMessage("Weapon switch failed: weapon is not in inventory", LogLevel.WARNING);
				#endif
				return ENodeResult.FAIL;
			}
			
			#ifdef AI_DEBUG
			AddDebugMessage(string.Format("StartWeaponSwitchCharacter: %1 %2 %3", newWeaponComp, newWeaponComp.GetOwner(), newWeaponComp.GetOwner().GetPrefabData().GetPrefabName()));
			#endif
				
			if (m_iWeaponState == WEAPON_STATE_MOVING)
				return ENodeResult.RUNNING;
					
			array<IEntity> weapons = {};
			m_WeaponMgrComp.GetWeaponsList(weapons);
				
			IEntity newWeaponEnt = newWeaponComp.GetOwner();
								
			if (!weapons.Contains(newWeaponEnt))
			{
				// Return fail if weapon was moved and it's still unavailable
				if (m_iWeaponState == WEAPON_STATE_MOVED)
					return ENodeResult.FAIL;
						
				IEntity controlledEnt = owner.GetControlledEntity();
					
				if (!m_EqWeaponStorageComp)
					m_EqWeaponStorageComp = EquipedWeaponStorageComponent.Cast(controlledEnt.FindComponent(EquipedWeaponStorageComponent));
					
				if (!m_EqWeaponStorageComp)
					return ENodeResult.FAIL;
					
				m_iWeaponState = WEAPON_STATE_MOVING;
				
				if (m_WeaponMoveCallback)
					m_WeaponMoveCallback.Invalidate();
				
				m_WeaponMoveCallback = new SCR_AISwitchWeaponMovedCallback(this);
				
				InventoryStorageSlot slot = m_EqWeaponStorageComp.FindSuitableSlotForItem(newWeaponEnt);
				IEntity existingEntityAtSlot = null;
				if (slot)
					existingEntityAtSlot = slot.GetAttachedEntity();
				
				if (existingEntityAtSlot)
				{
					#ifdef AI_DEBUG
					AddDebugMessage("Weapon is not in the weapon manager list, slot already occupied, requesting swap of weapon and existing entity in slot");
					#endif
					m_InventoryMgr.TrySwapItemStorages(newWeaponEnt, existingEntityAtSlot, m_WeaponMoveCallback);
				}
				else
				{
					#ifdef AI_DEBUG
					AddDebugMessage("Weapon is not in the weapon manager list, requesting move to EquipedWeaponStorageComponent");
					#endif
					m_InventoryMgr.TryMoveItemToStorage(newWeaponEnt, m_EqWeaponStorageComp, -1, m_WeaponMoveCallback);
				}
				
				return ENodeResult.RUNNING;
			}
				
			m_iWeaponState = WEAPON_STATE_IDLE;
			SCR_AIWeaponHandling.StartWeaponSwitchCharacter(m_ControlComp, newWeaponComp);
			
			return ENodeResult.RUNNING;
		}
		
		return ENodeResult.FAIL;
	}
	
	
	override void OnAbort(AIAgent owner, Node nodeCausingAbort)
	{
		m_iWeaponState = WEAPON_STATE_IDLE;
		
		if (m_WeaponMoveCallback)
			m_WeaponMoveCallback.Invalidate();
		
		m_WeaponMoveCallback = null;
	}
	
	//--------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {PORT_WEAPON_COMPONENT};
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	
	static override bool VisibleInPalette() { return true; }
}