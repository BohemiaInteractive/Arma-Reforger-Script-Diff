class SCR_AIThrowGrenadeToBehavior : SCR_AIBehaviorBase
{
	//Target Position of the behavior
	ref SCR_BTParam<vector> m_vTargetPosition = new SCR_BTParam<vector>(SCR_AIActionTask.TARGETPOSITION_PORT);
	ref SCR_BTParam<EWeaponType> e_WeaponType = new SCR_BTParam<EWeaponType>(SCR_AIActionTask.WEAPON_TYPE_PORT);
	ref SCR_BTParam<float> m_fDelay = new SCR_BTParam<float>(SCR_AIActionTask.DELAY_PORT);
	
	BaseWorld m_World;
	float m_fStartTime;
	
#ifdef WORKBENCH
	//Diagnostic visualization
	ref array<ref Shape> m_aDbgShapes = {};
#endif
	private static vector CHARACTER_HEIGHT_OFFSET = {0, 1.6, 0};
	
	//---------------------------------------------------------------------------------------------------------------------------------
	void SCR_AIThrowGrenadeToBehavior(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity, vector position, EWeaponType weaponType, float delay, float priority = PRIORITY_BEHAVIOR_THROW_GRENADE, float priorityLevel = PRIORITY_LEVEL_NORMAL)
	{
		m_vTargetPosition.Init(this, position);
		e_WeaponType.Init(this, weaponType);
		m_fDelay.Init(this, delay);
		
		if (utility)
		{
			m_World = utility.m_OwnerEntity.GetWorld();
			m_fStartTime = m_World.GetWorldTime() + delay;
		}

		m_sBehaviorTree = "{3187CAE77AAF1A35}AI/BehaviorTrees/Chimera/Soldier/ThrowGrenadeTo.bt";
		SetPriority(priority);
		m_fPriorityLevel.m_Value = priorityLevel;
		m_bAllowLook = false;
		
#ifdef WORKBENCH 
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_SHOW_GRENADE_POSITIONS))
			DrawPositionDebug(false);		
#endif
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------
	override int GetCause()
	{
		return SCR_EAIBehaviorCause.COMBAT;
	}
		
	//------------------------------------------------------------------------------------------------------------------------------------
	override float CustomEvaluate()
	{
		if (m_World && m_World.GetWorldTime() < m_fStartTime)
			return 0;
		
		return GetPriority();
	}
	
	//----------------------------------------------------------------------------------
	override void OnActionSelected()
	{
		m_Utility.m_AIInfo.SetAIState(EUnitAIState.BUSY);
	
#ifdef WORKBENCH
		m_aDbgShapes.Clear();
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_SHOW_GRENADE_POSITIONS))
			DrawPositionDebug(true);		
#endif	
	}

	//----------------------------------------------------------------------------------
	override void OnActionCompleted()
	{
		super.OnActionCompleted();
		m_Utility.m_AIInfo.SetAIState(EUnitAIState.AVAILABLE);
#ifdef WORKBENCH
		SCR_AIDebugVisualization.VisualizeMessage(m_Utility.m_OwnerEntity, "Grenade thrown", EAIDebugCategory.INFO, 5); 
		m_aDbgShapes.Clear();
#endif
	}

	//----------------------------------------------------------------------------------
	override void OnActionFailed()
	{
		super.OnActionFailed();
		m_Utility.m_AIInfo.SetAIState(EUnitAIState.AVAILABLE);
#ifdef WORKBENCH
		SCR_AIDebugVisualization.VisualizeMessage(m_Utility.m_OwnerEntity, "Grenade throw failed", EAIDebugCategory.INFO, 5);
		m_aDbgShapes.Clear();
#endif
	}

	//----------------------------------------------------------------------------------
	override string GetActionDebugInfo()
	{
		return this.ToString() + " throwing grenade to " + m_vTargetPosition.m_Value.ToString();
	}
	
	//------------------------------------------------------------------------------------------------------------------------------------
	void DrawPositionDebug(bool selected)
	{
		int shapeColor = Color.RED;
		if (selected)
			shapeColor = Color.GREEN;
#ifdef WORKBENCH
		m_aDbgShapes.Insert(Shape.CreateCylinder(Color.WHITE, ShapeFlags.DEFAULT, m_vTargetPosition.m_Value, 0.3, 100));
		m_aDbgShapes.Insert(Shape.CreateSphere(shapeColor, ShapeFlags.DEFAULT, m_vTargetPosition.m_Value, 0.6));
#endif
	}
};