class SCR_AIFindTagetToLookAt: AITaskScripted
{	
	[Attribute("1", UIWidgets.EditBox, "Max time since target was last seen", "")]
	float m_fTimeSinceSeenMax_S;
	
	[Attribute("1", UIWidgets.EditBox, "Max time since target was last detected", "")]
	float m_fTimeSinceDetectedMax_S;
	
	protected static const string PORT_BASE_TARGET = "BaseTarget";

	protected PerceptionComponent m_PerceptionComp;
	protected SCR_AIConfigComponent m_ConfigComp;
	
	#ifdef WORKBENCH
	protected ref Shape m_Shape;
	#endif
	
	
	override void OnInit(AIAgent owner)
	{
		GenericEntity ent = GenericEntity.Cast(owner.GetControlledEntity());
		if (!ent)
			return;
		
		m_PerceptionComp = PerceptionComponent.Cast(ent.FindComponent(PerceptionComponent));
		m_ConfigComp = SCR_AIConfigComponent.Cast(owner.FindComponent(SCR_AIConfigComponent));	
	}
	
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_ConfigComp || !m_ConfigComp.m_EnablePerception || !m_PerceptionComp)
			return ENodeResult.FAIL;
		
		BaseTarget outTarget = null;

		float updateInterval = m_PerceptionComp.GetUpdateInterval();
		float timeSinceSeenMax = Math.Max(m_fTimeSinceSeenMax_S, updateInterval) + 0.02;
		float timeSinceDetectedMax = Math.Max(m_fTimeSinceDetectedMax_S, updateInterval) + 0.02;
		
		// Firse we search for target in detected category, since these targets are new and not identified yet
		outTarget = m_PerceptionComp.GetClosestTarget(ETargetCategory.DETECTED, timeSinceSeenMax, timeSinceDetectedMax);
		
		//if (outTarget)
		//	Print("SCR_AIFindTagetToLookAt: Found detected target");
		
		// If there is no target in detected category, also check targets in enemy category
		// Once target was identified, it stays in enemy category for a long time
		if (!outTarget)
		{
			outTarget = m_PerceptionComp.GetClosestTarget(ETargetCategory.ENEMY, timeSinceSeenMax, timeSinceDetectedMax);
			//if (outTarget)
			//	Print("SCR_AIFindTagetToLookAt: Found enemy target");
		}
		
		//Print(string.Format("SCR_AIFindTagetToLookAt: Returning target: %1", outTarget));
		
		// Temporary fix to not look at disarmed (unconscious) targets
		// Ideally it should be handled through GetClosestTarget call
		if (outTarget && outTarget.IsDisarmed())
			outTarget = null;
		
		SetVariableOut(PORT_BASE_TARGET, outTarget);
		
		#ifdef WORKBENCH
		if (outTarget && DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_SHOW_TARGET_LASTSEEN))
		{
			m_Shape = Shape.CreateSphere(COLOR_BLUE_A, ShapeFlags.NOOUTLINE|ShapeFlags.NOZBUFFER|ShapeFlags.TRANSP, outTarget.GetLastSeenPosition(), 0.1);
		}
		#endif
		
		if (outTarget)
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	
	protected static ref TStringArray s_aVarsOut = {
		PORT_BASE_TARGET
	};
	override TStringArray GetVariablesOut()
    {
        return s_aVarsOut;
    }
	
		static override bool VisibleInPalette()
    {
        return true;
    }
};