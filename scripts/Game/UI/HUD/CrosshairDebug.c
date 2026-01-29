class SCR_CrosshairDebug
{
	private BaseWeaponManagerComponent m_WeaponManager;
	private ImageWidget m_CrosshairMuzzleDebugWidget;
	
	//------------------------------------------------------------------------------------------------
	void Update(ChimeraCharacter owner, float timeSlice)
	{
		if (!m_CrosshairMuzzleDebugWidget)
			return;

		bool debugEnabled = DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_CHARACTER_MUZZLE_CROSSHAIR);
		m_CrosshairMuzzleDebugWidget.SetVisible(debugEnabled);
		
		if (!debugEnabled)
			return;
		
		vector muzzleMat[4];
		m_WeaponManager.GetCurrentMuzzleTransform(muzzleMat);

		const float traceDistance = 100;
		vector traceStart = muzzleMat[3];
		vector traceEnd = muzzleMat[3] + muzzleMat[2] * traceDistance;

		autoptr TraceParam p = new TraceParam();
		p.Start = traceStart;
		p.End = traceEnd;
		p.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
		p.Exclude = owner;
		p.LayerMask = TRACE_LAYER_CAMERA;
		BaseWorld world = owner.GetWorld();
		float hit = world.TraceMove(p, null);

		vector endPos = hit * (traceEnd - traceStart) + traceStart;
		vector screenVect = GetGame().GetWorkspace().ProjWorldToScreen(endPos, world);
		FrameSlot.SetPos(m_CrosshairMuzzleDebugWidget, screenVect[0],screenVect[1]);
	}

	//------------------------------------------------------------------------------------------------
	void SCR_CrosshairDebug(ChimeraCharacter owner, Widget root)
	{
		// Detect and store weapon manager
		m_WeaponManager = BaseWeaponManagerComponent.Cast(owner.FindComponent(BaseWeaponManagerComponent)); 
		if (!m_WeaponManager)
			return;
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_CrosshairDebug()
	{
	}
};
