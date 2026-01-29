class SCR_HitZoneStateSignal
{
	ref array<ref SCR_HitZone> m_aScriptedHitZones = {};	
	int m_iSignalIdx;
	
	//------------------------------------------------------------------------------------------------
	bool RegisterSignal(SCR_DamageManagerComponent hitZoneContainerComponent, SCR_HitZoneStateSignalData hitZoneStateSignalData, SignalsManagerComponent signalsManagerComponent)
	{	
		array<string> hitZoneNames = hitZoneStateSignalData.m_aHitZoneNames;
		if (!hitZoneNames || hitZoneNames.Count() == 0)
		{
			Print("AUDIO: SCR_HitZoneStateSignal: Missing HitZoneNames", LogLevel.WARNING);
			return false;
		}
		
		foreach (string hitZoneName : hitZoneNames) 
		{
			HitZone hitZone = hitZoneContainerComponent.GetHitZoneByName(hitZoneName);
			SCR_HitZone hitzone = SCR_HitZone.Cast(hitZone);
			if (!hitzone)
				continue;
			
			hitzone.GetOnDamageStateChanged().Insert(OnStateChanged);
			m_aScriptedHitZones.Insert(hitzone);
		}
		
		// No hitzones are spawned in the edit mode so only check the playmode
		if (!SCR_Global.IsEditMode() && m_aScriptedHitZones.Count() == 0)
		{
			Print("AUDIO: SCR_HitZoneStateSignal: No HitZone found", LogLevel.WARNING);
			return false;
		}
		
		string signalName = hitZoneStateSignalData.m_sSignalName;
		if (signalName.IsEmpty())
		{
			Print("AUDIO: SCR_HitZoneStateSignal: Missing signal name", LogLevel.WARNING);
			return false;
		}
		
		m_iSignalIdx = signalsManagerComponent.AddOrFindSignal(signalName);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void UnregisterSignal()
	{		
		foreach(SCR_HitZone hitzone : m_aScriptedHitZones)
		{
			hitzone.GetOnDamageStateChanged().Remove(OnStateChanged);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnStateChanged()
	{
		int max;
		
		foreach(SCR_HitZone hitzone : m_aScriptedHitZones)
		{	
			int state = DamageStateToSignalValue(hitzone.GetDamageState());
			if (state > max)
				max = state;
		}
		
		IEntity owner = m_aScriptedHitZones[0].GetOwner();
		if (!owner)
			return;
		
		SignalsManagerComponent signalsManager = SignalsManagerComponent.Cast(owner.GetRootParent().FindComponent(SignalsManagerComponent));
		if (!signalsManager)
			return;
							
		signalsManager.SetSignalValue(m_iSignalIdx, max);		
	}
	
	//------------------------------------------------------------------------------------------------
	static int DamageStateToSignalValue(EDamageState damageState)
	{
		switch(damageState)
		{
			case EDamageState.UNDAMAGED: {return 0;};
			case EDamageState.INTERMEDIARY: {return 1;};
			case EDamageState.STATE1: {return 2;};
			case EDamageState.STATE2: {return 3;};
			case EDamageState.STATE3: {return 4;};
			case EDamageState.DESTROYED: {return 5;};
		}
		
		return 0;
	}
}