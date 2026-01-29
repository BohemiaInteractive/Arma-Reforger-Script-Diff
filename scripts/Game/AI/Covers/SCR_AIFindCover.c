class SCR_AIFindCover : AITaskScripted
{
	// Inputs
	protected static const string PORT_TARGET_POS = "TargetPos";
	protected static const string PORT_COVER_QUERY_PROPERTIES = "CoverQueryProps";
	
	// Outputs
	protected static const string PORT_COVER_LOCK = "CoverLock";
	
	
	// Cached components of this agent
	protected SCR_AICombatMoveState m_State;
	protected AIPathfindingComponent m_PathfindingComp;
	
	protected ChimeraCoverManagerComponent m_CoverMgr;

	// Constants for amount of covers to check for high priority cover queries and low priority cover queries
	const vector NEAREST_POLY_HALF_EXTEND = "1.0 2.0 1.0";
	const int MAX_COVERS_HIGH_PRIORITY = 25;
	const int MAX_COVERS_LOW_PRIORITY = 15;
	const float NAVMESH_AREA_COST_SCALE = 1/3.0; // It's scaled according to cost of Offroad area type, which is 3.0.
	
	
#ifdef WORKBENCH
	protected ref array<ref Shape> m_aDebugShapes = {};
	protected ref Shape m_CoverShape;

	protected static const ShapeFlags m_SphereFlags = ShapeFlags.NOOUTLINE|ShapeFlags.NOZBUFFER|ShapeFlags.TRANSP;
	protected static const ShapeFlags m_LineFlags = ShapeFlags.NOOUTLINE|ShapeFlags.TRANSP;
	protected static const float DEBUGSPHERE_RADIUS = 0.1;
#endif

	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		SCR_AIUtilityComponent utilityComp = SCR_AIUtilityComponent.Cast(owner.FindComponent(SCR_AIUtilityComponent));
		if (utilityComp)
			m_State = utilityComp.m_CombatMoveState;
		
		AIWorld aiWorld = GetGame().GetAIWorld();
		if (aiWorld)
			m_CoverMgr = ChimeraCoverManagerComponent.Cast(aiWorld.FindComponent(ChimeraCoverManagerComponent));
		
		IEntity myEntity = owner.GetControlledEntity();
		if (myEntity)
			m_PathfindingComp = AIPathfindingComponent.Cast(myEntity.FindComponent(AIPathfindingComponent));
	}

	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		IEntity ownerEntity = owner.GetControlledEntity();
		if (!ownerEntity || !m_State || !m_CoverMgr || !m_PathfindingComp)
			return ENodeResult.FAIL;

		//------------------------------------------------------------------------------------------------
		// Read inputs
		
		CoverQueryProperties queryProps;
		GetVariableIn(PORT_COVER_QUERY_PROPERTIES, queryProps);
		if (!queryProps)
			return ENodeResult.FAIL;

		//------------------------------------------------------------------------------------------------
		// Find cover

#ifdef WORKBENCH
		ClearDebug();
#endif

		vector coverPos, coverTallestPos;
		int tilex, tiley, coverId;
		
		bool coverFound = m_CoverMgr.GetBestCover("Soldiers", m_PathfindingComp, queryProps, coverPos, coverTallestPos, tilex, tiley, coverId);
		
#ifdef WORKBENCH
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_DEBUG_COVERS))
		{
			m_aDebugShapes.Insert(Shape.CreateSphere(Color.GREEN, m_SphereFlags, ownerEntity.GetOrigin(), 0.2));
		}
#endif

		if (!coverFound)
		{
			// Release previous cover lock
			//m_State.ReleaseCover(); // Do not release previous cover if new one was not found. We still want to occupy it.
			ClearVariable(PORT_COVER_LOCK);
			return ENodeResult.FAIL;
		}

		// Create new cover lock, and release the old one
		m_State.AssignCover(new SCR_AICoverLock(tilex, tiley, coverId, coverPos, coverTallestPos));
		
		SetVariableOut(PORT_COVER_LOCK, m_State.GetAssignedCover());
		
#ifdef WORKBENCH
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_DEBUG_COVERS))
		{
			m_CoverShape = Shape.CreateSphere(Color.PINK, m_SphereFlags, coverPos, 0.5);
		}
#endif

		return ENodeResult.SUCCESS;
	}

	protected static ref TStringArray s_aVarsIn = {
		PORT_COVER_QUERY_PROPERTIES
	};
	override TStringArray GetVariablesIn()
	{
		return s_aVarsIn;
	}
	
	protected static ref TStringArray s_aVarsOut = {
		PORT_COVER_LOCK
	};
	override TStringArray GetVariablesOut()
	{
		return s_aVarsOut;
	}

	override static protected bool VisibleInPalette() {	return true; }

	static override string GetOnHoverDescription()
	{
		return "Finds and locks cover through cover manager. Keep in mind that cover is also assigned to CombatMoveState!";
	}
	
	static override bool CanReturnRunning() { return true; }

	#ifdef WORKBENCH
	private void ClearDebug()
	{
		m_aDebugShapes.Clear();
	}
	#endif
}
