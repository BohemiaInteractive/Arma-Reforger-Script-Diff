class SCR_AISwitchMuzzle : SCR_AIWeaponHandlingBase
{
	protected static const string PORT_MUZZLE_ID = "MuzzleId";
	
	//--------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_WeaponMgrComp || !m_ControlComp || !m_InventoryMgr)
			return ENodeResult.FAIL;
		
		int newMuzzleId = -1;
		GetVariableIn(PORT_MUZZLE_ID, newMuzzleId);
		if (newMuzzleId == -1)
		{
			#ifdef AI_DEBUG
			AddDebugMessage("Skipped muzzle switch, no muzzle was provided");
			#endif
			return ENodeResult.SUCCESS;
		}
		
		BaseCompartmentSlot compartmentSlot = m_CompartmentAccessComp.GetCompartment();
			
		// Return success if done
		// Ignore if we are in compartment. There are no turret weapons where we can choose a muzzle.
		// And there is no API to switch muzzle in a turret anyway.
		int currentMuzzleId = SCR_AIWeaponHandling.GetCurrentMuzzleId(m_WeaponMgrComp);
		if (currentMuzzleId == newMuzzleId || compartmentSlot)
		{
			#ifdef AI_DEBUG
			AddDebugMessage("Muzzle switch completed");
			#endif
			return ENodeResult.SUCCESS;
		}
			
		#ifdef AI_DEBUG
		AddDebugMessage(string.Format("Start Muzzle Switch: %1", newMuzzleId));
		#endif
		SCR_AIWeaponHandling.StartMuzzleSwitch(m_ControlComp, newMuzzleId);
		
		return ENodeResult.RUNNING;
	}
	
	//--------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {PORT_MUZZLE_ID};
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	
	static override bool VisibleInPalette() { return true; }
}