class SCR_AISearchAndDestroyActivity : SCR_AIActivityBase
{
	protected ref SCR_BTParamAssignable<vector> m_vPosition = new SCR_BTParamAssignable<vector>(SCR_AIActionTask.POSITION_PORT);
	protected ref SCR_BTParam<IEntity> m_Entity = new SCR_BTParam<IEntity>(SCR_AIActionTask.ENTITY_PORT);
	
	protected SCR_AIGroup m_Group;
	protected const float m_fBehaviorRadius = 3;
	
	protected SCR_SearchAndDestroyWaypoint m_WP;
	protected AIPathfindingComponent m_pathFindindingComp;
	protected vector m_vGridOrigin;
	protected ref array<vector> m_aTiles = {};
	protected bool m_bAllTilesLoaded;
	
	protected ref array<vector> m_aCorrectedPositions = {};
	protected ref array<vector> m_aAvailablePositions = {};
	protected ref array<vector> m_aAssignedPositions = {};

	protected float m_fHoldingTime = 600.0;
	protected float m_fStartTime;
	protected float m_fTimeStamp;
	
#ifdef WORKBENCH
	protected ref array<ref Shape> m_aDbgShape = {};
#endif
	
	//------------------------------------------------------------------------------------------------
	void InitParameters(vector position, IEntity entity)
	{
		m_vPosition.Init(this, position);
		m_vPosition.m_AssignedOut = (position != vector.Zero);
		m_Entity.Init(this, entity);
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_AISearchAndDestroyActivity(SCR_AIGroupUtilityComponent utility, AIWaypoint relatedWaypoint, vector pos, IEntity ent, EMovementType movementType = EMovementType.RUN, bool useVehicles = false, float priority = PRIORITY_ACTIVITY_SEEK_AND_DESTROY, float priorityLevel = PRIORITY_LEVEL_NORMAL)
	{
		InitParameters(pos, ent);
		SetPriority(priority);
		
		m_Group = SCR_AIGroup.Cast(m_Utility.GetAIAgent());
		if (!m_Group)
			return;
		m_pathFindindingComp = AIPathfindingComponent.Cast(m_Group.FindComponent(AIPathfindingComponent));
		m_vGridOrigin = pos;

		AIWaypoint wp = relatedWaypoint;
		if (wp)
			m_WP = SCR_SearchAndDestroyWaypoint.Cast(wp);
		
		m_fStartTime = GetGame().GetWorld().GetWorldTime();
		if (m_WP)
			m_fHoldingTime = m_WP.GetHoldingTime();
		m_fTimeStamp = m_fStartTime;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionSelected()
	{
		super.OnActionSelected();
		GenerateInvestigationGrid();
		
		if (m_WP)
			m_WP.GetOnWaypointPropertiesChanged().Insert(OnWaypointPropertiesChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	override float CustomEvaluate()
	{		
		float currentTime = GetGame().GetWorld().GetWorldTime();
		
		if (GetActionState() == EAIActionState.RUNNING)
		{
			// check every 2s if all tiles are loaded and proceed w/ activity
			if (!m_bAllTilesLoaded && currentTime - m_fTimeStamp > 2)
			{
				if (LoadTiles())
				{
					AdjustInvestigationPositions();
				
					// assign first available position to all fireteams
					ref array<SCR_AIGroupFireteam> fireTeams = {};
					m_Utility.m_FireteamMgr.GetFreeFireteams(fireTeams, SCR_AIGroupFireteam);
					
					foreach (SCR_AIGroupFireteam ft : fireTeams)
						AssignInvestigationPositions(ft);
					
					m_bAllTilesLoaded = true;
				}
				
				m_fTimeStamp = currentTime;
			}
		}
		
		return GetPriority();
	}
	
	//------------------------------------------------------------------------------------------------
	// on investigation finished, assign another investigation position
	override void OnChildBehaviorFinished(SCR_AIBehaviorBase childBehavior)
	{

		SCR_AIMoveAndInvestigateBehavior behavior = SCR_AIMoveAndInvestigateBehavior.Cast(childBehavior);
		if (!childBehavior || !behavior)
			return;
		if (m_aAssignedPositions.Contains(behavior.m_vPosition.m_Value) < 0)
			return;
		
		m_aAssignedPositions.RemoveItem(behavior.m_vPosition.m_Value);
		
		AIAgent agent = childBehavior.m_Utility.GetOwner();
		if (!agent)
			return;
		SCR_AIGroupFireteam ft = m_Utility.m_FireteamMgr.FindFireteam(agent);
		if (!ft)
			return;

		if (behavior.GetActionState() != EAIActionState.FAILED)
			AssignInvestigationPositions(ft);
		
		// Fail behavior when WP holding time is up
		if (m_WP)
		{
			float currentTime = GetGame().GetWorld().GetWorldTime();
			if (currentTime - m_fStartTime > m_fHoldingTime)
			behavior.Fail();
		}
	}
		
	//------------------------------------------------------------------------------------------------
	protected void GenerateInvestigationGrid()
	{	
		float actionRadius = 20.0;
		float tileSize;
		vector tileCenterPos;
		vector XOffset;
		vector XZOffset;
		
		m_aTiles.Clear();

		if (m_WP)
		{
			actionRadius = m_WP.GetCompletionRadius();
			m_vGridOrigin = m_WP.GetOrigin() + Vector(-actionRadius, 0, -actionRadius);
		}
		
		// get fireteam x tile ratio
		int fireteamCount = m_Utility.m_FireteamMgr.GetFireteamCount();
		if (fireteamCount == 0)
			return;
		const int maxFireteam = 8;
		float fireteamTileRatio = (maxFireteam - fireteamCount) / 10;
		
		tileSize = Math.Clamp(fireteamTileRatio * actionRadius, 5, 20);
		vector addedTileSize = Vector(tileSize, tileSize, tileSize);
		
		int tileCount = (actionRadius * 2) / tileSize;
			
		for (int x = 0; x < tileCount; x++)
		{
			for (int z = 0; z < tileCount; z++)
			{
				// calculate tile vector of its X and XZ 
				vector vectorX = Vector(x * tileSize, 0, z * tileSize);
				vector vectorXZ = vectorX + addedTileSize;
				// add offset of WP origin
				XOffset = vectorX + m_vGridOrigin;
				XZOffset = vectorXZ + m_vGridOrigin;
				// get center position
				tileCenterPos = XZOffset / 2 + XOffset / 2;
				m_aTiles.Insert(tileCenterPos);
				
#ifdef WORKBENCH
				if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_SHOW_SEARCH_AND_DESTROY))
				{
					m_aDbgShape.Insert(Shape.Create(ShapeType.BBOX, Color.DARK_CYAN, ShapeFlags.WIREFRAME, XOffset, XZOffset));	
				}
#endif
			}
		}
	}
	
	//-----------------------------------------------------------------------------------------------------
	// are grid tiles loaded?
	protected bool LoadTiles()
	{
		bool allTilesLoaded = true;
		
		if (!m_pathFindindingComp)
			return false;
		NavmeshWorldComponent navmesh = m_pathFindindingComp.GetNavmeshComponent();
		if (!navmesh)
			return false;
		
		foreach (vector tile : m_aTiles)
		{
			if (!navmesh.IsTileLoaded(tile))
			{
				navmesh.LoadTileIn(tile);
				allTilesLoaded = false;
			}
		}
		
		return allTilesLoaded;
	}
	
	//------------------------------------------------------------------------------------------------
	// adjust investigation positions vis-a-vis navmesh
	protected void AdjustInvestigationPositions()
	{
		vector correctedPosition;
		m_aCorrectedPositions.Clear();
		
		foreach (vector tile : m_aTiles)
		{
			m_pathFindindingComp.GetClosestPositionOnNavmesh(tile, "15 15 15", correctedPosition);
			
			if (!m_aCorrectedPositions.Contains(correctedPosition))
			{
#ifdef WORKBENCH
				if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_SHOW_SEARCH_AND_DESTROY))
				{
					m_aDbgShape.Insert(Shape.CreateSphere(Color.RED, ShapeFlags.NOZWRITE, correctedPosition, 0.5));
				}
#endif
				m_aCorrectedPositions.Insert(correctedPosition);
			}
		}
		
		m_aAvailablePositions.Copy(m_aCorrectedPositions);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void AssignInvestigationPositions(SCR_AIGroupFireteam ft)
	{
		array<AIAgent> agents = {};
		ft.GetMembers(agents);
		foreach (AIAgent ag : agents)
		{
			SCR_AIMessage_Investigate msg = SCR_AIMessage_Investigate.Create(this, m_aAvailablePositions[0], m_fBehaviorRadius, true, EAIUnitType.UnitType_Infantry);
			m_Utility.m_Mailbox.RequestBroadcast(msg,ag);
		}
		
#ifdef WORKBENCH
				if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_SHOW_SEARCH_AND_DESTROY))
				{
					m_aDbgShape.Insert(Shape.CreateSphere(Color.GREEN, ShapeFlags.NOZWRITE, m_aAvailablePositions[0], 1.5));
				}
#endif
		
		m_aAssignedPositions.Insert(m_aAvailablePositions[0]);
		m_aAvailablePositions.Remove(0);
		
		if (!m_aAvailablePositions.IsIndexValid(0))
		{
#ifdef WORKBENCH
			ClearDebug();
#endif
			m_aAvailablePositions.Copy(m_aCorrectedPositions);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnWaypointPropertiesChanged()
	{
		GenerateInvestigationGrid();
		m_bAllTilesLoaded = false;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionFailed()
	{
		super.OnActionFailed();
		SendCancelMessagesToAllAgents();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionDeselected()
	{
		super.OnActionDeselected();
		SendCancelMessagesToAllAgents();
		
		if (m_WP)
			m_WP.GetOnWaypointPropertiesChanged().Remove(OnWaypointPropertiesChanged);
	}
		
	//------------------------------------------------------------------------------------------------
	override string GetActionDebugInfo()
	{
		return this.ToString() + " seek and destroy around" + m_Entity.m_Value.ToString();
	}
	
	//-----------------------------------------------------------------------------------------------
#ifdef WORKBENCH
	private void ClearDebug()
	{
		m_aDbgShape.Clear();
	}
#endif
};