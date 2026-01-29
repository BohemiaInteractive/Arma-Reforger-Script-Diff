class SCR_AIUpdateTargetSuppressionData : AITaskScripted
{
	// Inputs
	protected static const string PORT_SUPPRESSION_VOLUME = "SuppressionVolume";
	
	// Outputs
	protected static const string PORT_VISIBLE = "Visible";
	protected static const string PORT_TIME_LAST_SEEN = "TimeLastSeen_ms";
	protected static const string PORT_FIRE_TREE_ID = "FireTreeId";	
	
	// These IDs must match to actual trees in the tree
	protected const int FIRE_TREE_INVALID 		= -1;	// No aiming or firing is allowed at all
	protected const int FIRE_TREE_LOOK			= 0;	// Looking at target without firing
	protected const int FIRE_TREE_SUPPRESSIVE	= 1;
	
	// Related to visibility check
	protected float m_fVisibilityCheckTimer = VISIBILITY_CHECK_INTERVAL_S; // We need the vision check to run right on start. This data is required by movement logic.
	protected bool m_bTargetVisible = false;
	protected float m_fTargetLastSeenTime_ms = 0; // World time
	protected ref TraceParam m_TraceParam;
	protected ref array<IEntity> m_TraceParamExcludeArray;
	protected const float VISIBILITY_CHECK_INTERVAL_S = 0.75;
	protected const float VISIBILITY_CHECK_TRACE_RESULT_THRESHOLD = 0.5;
	
	// Other
	protected SCR_AIUtilityComponent m_UtilityComponent;
	protected PerceptionComponent m_PerceptionComponent;
	
	#ifdef WORKBENCH
	protected ref array<ref Shape> m_aDebugShapes = {};
	#endif
	
	//---------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		m_UtilityComponent = SCR_AIUtilityComponent.Cast(owner.FindComponent(SCR_AIUtilityComponent));
		IEntity myEntity = owner.GetControlledEntity();
		if (myEntity)
			m_PerceptionComponent = PerceptionComponent.Cast(myEntity.FindComponent(PerceptionComponent));
	}
	
	//---------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		SCR_AISuppressionVolumeBase suppressionVolume;
		if (!GetVariableIn(PORT_SUPPRESSION_VOLUME, suppressionVolume) || !suppressionVolume)
			return ENodeResult.FAIL;
		
		IEntity myEntity = owner.GetControlledEntity();
		if (!myEntity || !m_UtilityComponent || !m_PerceptionComponent)
			return ENodeResult.FAIL;
		
		m_fVisibilityCheckTimer += dt;		
		if (m_fVisibilityCheckTimer >= VISIBILITY_CHECK_INTERVAL_S)
		{
			m_bTargetVisible = CheckTargetVisibility(myEntity, suppressionVolume);
			
			if (m_bTargetVisible)
				m_fTargetLastSeenTime_ms = GetGame().GetWorld().GetWorldTime();
			
			m_fVisibilityCheckTimer = 0;
		}
		
		int fireTreeid = ResolveFireTree(m_bTargetVisible);
		
		// Write data to ports
		SetVariableOut(PORT_VISIBLE, m_bTargetVisible);
		SetVariableOut(PORT_TIME_LAST_SEEN, m_fTargetLastSeenTime_ms);
		SetVariableOut(PORT_FIRE_TREE_ID, fireTreeid);
		
		return ENodeResult.SUCCESS;
	}
	
	//---------------------------------------------------------------------------
	bool CheckTargetVisibility(notnull IEntity myEntity, notnull SCR_AISuppressionVolumeBase suppressionVolume)
	{
		// Init trace params
		if (!m_TraceParam)
		{
			m_TraceParam = new TraceParam();
			m_TraceParam.TargetLayers = EPhysicsLayerDefs.FireGeometry;
			m_TraceParam.Flags = TraceFlags.ENTS | TraceFlags.OCEAN | TraceFlags.WORLD | TraceFlags.ANY_CONTACT;
			m_TraceParamExcludeArray = {};
			m_TraceParam.ExcludeArray = m_TraceParamExcludeArray;
		}
		
		// Update entity exclude array
		m_TraceParamExcludeArray.Clear();
		m_TraceParamExcludeArray.Insert(myEntity);
		IEntity myEntityParent = myEntity.GetParent();
		if (myEntityParent)
		{
			m_TraceParamExcludeArray.Insert(myEntityParent);
			IEntity myEntityParentParent = myEntityParent.GetParent();
			if (myEntityParentParent)
				m_TraceParamExcludeArray.Insert(myEntityParentParent);
		}
		
		// Update start and end pos
		// If we were to trace from start to end, and trace fraction in result is above VISIBILITY_CHECK_TRACE_RESULT_THRESHOLD,
		// then we treat it as good visibility to target.
		// But it means that we don't need to trace whole length anyway, but only fraction of it.
		// That's why actual trace end is lerped.
		ChimeraCharacter myCharacter = ChimeraCharacter.Cast(myEntity);
		if (myCharacter)
			m_TraceParam.Start = myCharacter.EyePosition();
		else
			m_TraceParam.Start = myEntity.GetOrigin();
		vector traceEndPosIdeal = suppressionVolume.GetCenterPosition() + Vector(0, 2, 0); // Raise it a few meters up, to help around slopes
		m_TraceParam.End = vector.Lerp(m_TraceParam.Start, traceEndPosIdeal, VISIBILITY_CHECK_TRACE_RESULT_THRESHOLD);
		
		float traceResult = GetGame().GetWorld().TraceMove(m_TraceParam, null);
		
		bool visible = traceResult == 1.0;
		
		#ifdef WORKBENCH
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_SHOW_DEBUG_SHAPES))
		{
			m_aDebugShapes.Clear();
			
			int lineColor;
			if (visible)
				lineColor = Color.GREEN;
			else
				lineColor = Color.RED;
			
			vector lineVerts[2];
			lineVerts[0] = m_TraceParam.Start;
			lineVerts[1] = m_TraceParam.End;
			Shape lineShape = Shape.CreateLines(lineColor, ShapeFlags.DEFAULT, lineVerts, 2);
			m_aDebugShapes.Insert(lineShape);
			
			if (traceResult != 1.0)
			{
				vector hitPos = m_TraceParam.Start + traceResult * (m_TraceParam.End - m_TraceParam.Start);
				Shape sphereShape = Shape.CreateSphere(Color.RED, ShapeFlags.DEFAULT, hitPos, 0.2);
				m_aDebugShapes.Insert(sphereShape);
			}
		}
		#endif
		
		return visible;
	}
	
	//---------------------------------------------------------------------------
	int ResolveFireTree(bool targetVisible)
	{
		// Is aiming forbidden by combat move?
		SCR_AIBehaviorBase executedBehavior = SCR_AIBehaviorBase.Cast(m_UtilityComponent.GetExecutedAction());
		if (executedBehavior && executedBehavior.m_bUseCombatMove && !m_UtilityComponent.m_CombatMoveState.m_bAimAtTarget)
			return FIRE_TREE_INVALID;
		
		// Friendly in aim?
		if (m_PerceptionComponent.GetFriendlyInLineOfFire())
			return FIRE_TREE_LOOK;
		
		if (targetVisible)
			return FIRE_TREE_SUPPRESSIVE;
		
		return FIRE_TREE_LOOK;
	}
	
	//---------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = { PORT_SUPPRESSION_VOLUME };
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	
	protected static ref TStringArray s_aVarsOut = { PORT_VISIBLE, PORT_TIME_LAST_SEEN, PORT_FIRE_TREE_ID };
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	static override bool VisibleInPalette() { return true; }
}