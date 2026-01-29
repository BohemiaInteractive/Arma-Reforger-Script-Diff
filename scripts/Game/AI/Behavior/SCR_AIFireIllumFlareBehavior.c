class SCR_AIFireIllumFlareBehavior : SCR_AIBehaviorBase
{
	ref SCR_BTParam<vector> m_vTargetPosition = new SCR_BTParam<vector>(SCR_AIActionTask.TARGETPOSITION_PORT);	
	ref SCR_BTParam<BaseWeaponComponent> m_WeaponComponent = new SCR_BTParam<BaseWeaponComponent>(SCR_AIActionTask.WEAPON_COMP_PORT);
	ref SCR_BTParam<BaseMagazineComponent> m_MagazineComponent = new SCR_BTParam<BaseMagazineComponent>(SCR_AIActionTask.MAGAZINE_COMP_PORT);
	ref SCR_BTParam<int> m_iMuzzleId = new SCR_BTParam<int>(SCR_AIActionTask.MUZZLE_ID);
	
#ifdef WORKBENCH
	//Diagnostic visualization
	ref array<ref Shape> m_aDbgShapes = {};
#endif

	void SCR_AIFireIllumFlareBehavior(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity, vector targetPosition, BaseWeaponComponent weaponComponent, BaseMagazineComponent magazineComponent, int muzzleId, float priority = PRIORITY_BEHAVIOR_FIRE_ILLUM_FLARE, float priorityLevel = PRIORITY_LEVEL_NORMAL)
	{		
		m_vTargetPosition.Init(this, targetPosition);
		m_WeaponComponent.Init(this, weaponComponent);
		m_MagazineComponent.Init(this, magazineComponent);
		m_iMuzzleId.Init(this, muzzleId);

		m_sBehaviorTree = "{6543CC29914F2933}AI/BehaviorTrees/Chimera/Soldier/FireIllumFlareAt.bt";
		SetPriority(priority);
		m_fPriorityLevel.m_Value = priorityLevel;
		m_bAllowLook = false;
		
#ifdef WORKBENCH 
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_SHOW_ILLUM_FLARES_POSITIONS))
			DrawPositionDebug();		
#endif
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------
	override int GetCause()
	{
		return SCR_EAIBehaviorCause.COMBAT;
	}	
		
	//----------------------------------------------------------------------------------
	override void OnActionSelected()
	{		
		m_Utility.m_AIInfo.SetAIState(EUnitAIState.BUSY);
	
#ifdef WORKBENCH
		m_aDbgShapes.Clear();
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_SHOW_ILLUM_FLARES_POSITIONS))
			DrawPositionDebug();		
#endif	
	}

	//----------------------------------------------------------------------------------
	override void OnActionCompleted()
	{		
		super.OnActionCompleted();
		m_Utility.m_AIInfo.SetAIState(EUnitAIState.AVAILABLE);
#ifdef WORKBENCH
		SCR_AIDebugVisualization.VisualizeMessage(m_Utility.m_OwnerEntity, "Illum flare fired", EAIDebugCategory.INFO, 5); 
		m_aDbgShapes.Clear();
#endif
	}

	//----------------------------------------------------------------------------------
	override void OnActionFailed()
	{		
		super.OnActionFailed();
		m_Utility.m_AIInfo.SetAIState(EUnitAIState.AVAILABLE);
#ifdef WORKBENCH
		SCR_AIDebugVisualization.VisualizeMessage(m_Utility.m_OwnerEntity, "Firing illum flare failed", EAIDebugCategory.INFO, 5);
		m_aDbgShapes.Clear();
#endif
	}

	//----------------------------------------------------------------------------------
	override string GetActionDebugInfo()
	{
		return this.ToString() + " firing illum flare at " + m_vTargetPosition.m_Value.ToString();
	}
	
	//------------------------------------------------------------------------------------------------------------------------------------
	void DrawPositionDebug()
	{
#ifdef WORKBENCH
		m_aDbgShapes.Insert(Shape.CreateSphere(Color.WHITE, ShapeFlags.DEFAULT, m_vTargetPosition.m_Value, 0.7));
#endif
	}
};