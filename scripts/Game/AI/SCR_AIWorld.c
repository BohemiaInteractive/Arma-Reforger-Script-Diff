void ScriptInvoker_AIWorldControlModeChangedMethod(AIGroup group, EGroupControlMode mode);
typedef func ScriptInvoker_AIWorldControlModeChangedMethod;
typedef ScriptInvokerBase<ScriptInvoker_AIWorldControlModeChangedMethod> ScriptInvoker_AIWorldControlModeChanged;

//#define DEBUG_NAVMESH_REBUILD_AREAS //--- Show bounds of recalculated navmesh areas

class SCR_AIWorldClass: ChimeraAIWorldClass
{
};

enum EAIDebugCategory
{
	NONE,
	BEHAVIOR,
	DANGER,
	THREAT,
	INFO,
	ORDER,
	COMBAT,
	LAST
};

class SCR_AIWorld : ChimeraAIWorld
{
	static const float MAX_NAVMESH_REBUILD_SIZE = 100 * 100; //--- Squared value
	
	static ref ScriptInvoker s_OnAgentSpawned = new ScriptInvoker();
	static ref ScriptInvoker s_OnAgentRemoved = new ScriptInvoker();
	static bool s_bDiagRegistered;
	
	[Attribute("", UIWidgets.Object)]
	ref array<ref SCR_AIMessageGoal> m_aGoalMessages;
	ref array<ref SCR_AIMessageGoal> m_aGoalMessagesPacked = new array<ref SCR_AIMessageGoal>();
	
	[Attribute("", UIWidgets.Object)]
	ref array<ref SCR_AIMessageInfo> m_aInfoMessages;
	ref array<ref SCR_AIMessageInfo> m_aInfoMessagesPacked = new array<ref SCR_AIMessageInfo>();
	
	[Attribute("", UIWidgets.Object)]
	ref array<ref SCR_AIOrderBase> m_aOrders;
	ref array<ref SCR_AIOrderBase> m_aOrdersPacked = new array<ref SCR_AIOrderBase>();
	
	protected ref ScriptInvoker_AIWorldControlModeChanged m_OnControlModeChanged = new ScriptInvoker_AIWorldControlModeChanged();
	
#ifdef DEBUG_NAVMESH_REBUILD_AREAS
	protected ref array<ref Shape> m_DebugNavmeshRebuildAreas = new array<ref Shape>;
#endif
	//------------------------------------------------------------------------------------------------
	/*!
	Get event called every time any group changes its control mode.
	Called from SCR_AIGroupInfoComponent.
	\return Script invoker
	*/
	ScriptInvoker_AIWorldControlModeChanged GetOnControlModeChanged()
	{
		return m_OnControlModeChanged;
	}
	
	protected void RegisterDbgCategory()
	{
		string categories;
		for (int i = 0; i < EAIDebugCategory.LAST-1; i++)
		{
			categories += typename.EnumToString(EAIDebugCategory, i)+",";
		}
		categories += typename.EnumToString(EAIDebugCategory, EAIDebugCategory.LAST-1);
		
		DiagMenu.RegisterItem(SCR_DebugMenuID.DEBUGUI_AI_DEBUG_CATEGORY, "", "AI Debug Category", "AIScript", categories);
#ifdef AI_DEBUG        
		DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_AI_DEBUG_CATEGORY,EAIDebugCategory.BEHAVIOR);
#endif
	}	

	//------------------------------------------------------------------------------------------------
	override void EOnInit (IEntity owner)
	{
		if (!s_bDiagRegistered)
		{
			DiagMenu.RegisterMenu(SCR_DebugMenuID.DEBUGUI_AI_SCRIPT, "AIScript", "AI");
			//this add AI categories inside to DiagMenu
			RegisterDbgCategory();
			
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_AI_SET_BT_BREAKPOINT, "", "Set BT Breakpoint", "AIScript");
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_AI_PRINT_DEBUG,"","Print debug from BTs","AIScript");
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_AI_PRINT_GROUP_INFO,"","Print init of groups","AIScript");
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_AI_FIRETEAMS, "", "Show fireteams", "AIScript");
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_AI_SUBGROUPS, "", "Show subgroups", "AIScript");
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_AI_TARGET_CLUSTERS, "", "Show target clusters", "AIScript");
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_AI_PRINT_SHOT_STATISTICS,"","Print stats for aiming","AIScript");
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_AI_PRINT_ACTIVITY,"","Print new activity","AIScript");
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_AI_SHOW_DEBUG_SHAPES,"","Show debug shapes from BTs","AIScript");
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_AI_SHOW_FRIENDLY_IN_AIM,"","Show fiendly in aim","AIScript");
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_AI_SHOW_TARGET_LASTSEEN,"","Show target last seen","AIScript");
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_AI_SELECT_FIXED_AGENT,"","Select fixed AIAgent","AIScript");
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_AI_DEBUG_COVERS,"","Debug cover search","AIScript");
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_AI_COMMS_HANDLERS,"", "Show Comms Handlers","AIScript");
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_AI_SHOW_PERCEPTION_PANEL,"","Show perception panel","AIScript");
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_AI_SHOW_GRENADE_POSITIONS,"","Show grenade positions","AIScript");
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_AI_SHOW_SUPPRESS_DEBUG,"","Show suppression debug","AIScript");
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_AI_SHOW_ILLUM_FLARES_POSITIONS,"","Show illum flares positions","AIScript");
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_AI_SHOW_AIM_LEAD_DEBUG,"","Show aim leading debug","AIScript");
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_AI_SHOW_SECTOR_THREAT_FILTER, "", "Show sector threat filter", "AIScript");
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_AI_SHOW_SEARCH_AND_DESTROY,"","Show search and destroy points of interest","AIScript");
			
			
#ifdef AI_DEBUG
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_AI_SEND_MESSAGE, "", "Show Send Message Menu","AIScript");
			DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_AI_PRINT_DEBUG,true);
			DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_AI_PRINT_GROUP_INFO,true);
			
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_AI_SHOW_TARGET_AIMPOINT,"", "Show target aimpoint", "Aiming");
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_AI_SHOW_TARGET_PROJECTED_SIZE,"", "Show target projected size", "Aiming");
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_AI_OPEN_DEBUG_PANEL, "", "Open Debug Panel", "AIScript");
			
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_AI_SHOW_GRENADE_TRACE, "", "Show Grenade Trace", "AIScript");
#endif

			s_bDiagRegistered = true;
		}
		
		typename type_EMessageType_Goal = EMessageType_Goal;
		typename type_EMessageType_Info = EMessageType_Info;
		typename type_EOrderType_Character = EOrderType_Character;
		
		m_aGoalMessagesPacked.Resize(type_EMessageType_Goal.GetVariableCount());
		foreach (SCR_AIMessageGoal message : m_aGoalMessages)
		{
			if(message.m_eUIType != EMessageType_Goal.NONE)
				m_aGoalMessagesPacked[message.m_eUIType] = message;
		}
		
		m_aInfoMessagesPacked.Resize(type_EMessageType_Info.GetVariableCount());
		foreach (SCR_AIMessageInfo message : m_aInfoMessages)
		{
			if(message.m_eUIType != EMessageType_Info.NONE)
				m_aInfoMessagesPacked[message.m_eUIType] = message;
		}
		
		m_aOrdersPacked.Resize(type_EOrderType_Character.GetVariableCount());
		foreach (SCR_AIOrderBase order : m_aOrders)
		{
			if(order.m_eUIType != EOrderType_Character.NONE)
				m_aOrdersPacked[order.m_eUIType] = order;
		}
		
#ifdef AI_DEBUG
		SCR_AISendMessageDebugUI.Init();
		SCR_AIDebugVisualization.Init();
#endif
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDebugAgentTeleport(AIAgent agent)
	{
		PrintFormat("Focused AI agent: %1", agent);
		
		IEntity agentEntity = agent.GetControlledEntity();
		if (!agentEntity) return;
		
		SCR_CameraEditorComponent editorCameraManager = SCR_CameraEditorComponent.Cast(SCR_CameraEditorComponent.GetInstance(SCR_CameraEditorComponent));
		if (editorCameraManager)
		{
			SCR_ManualCamera editorCamera = editorCameraManager.GetCamera();
			if (editorCamera)
			{
				SCR_TeleportToCursorManualCameraComponent teleportComponent = SCR_TeleportToCursorManualCameraComponent.Cast(editorCamera.FindCameraComponent(SCR_TeleportToCursorManualCameraComponent));
				if (teleportComponent)
				{
					teleportComponent.TeleportCamera(agentEntity.GetWorldTransformAxis(3));
				}
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDebugCameraTeleport(vector pos)
	{
		PrintFormat("Moving camera to pos %1", pos);
		
		SCR_CameraEditorComponent editorCameraManager = SCR_CameraEditorComponent.Cast(SCR_CameraEditorComponent.GetInstance(SCR_CameraEditorComponent));
		if (editorCameraManager)
		{
			SCR_ManualCamera editorCamera = editorCameraManager.GetCamera();
			if (editorCamera)
			{
				SCR_TeleportToCursorManualCameraComponent teleportComponent = SCR_TeleportToCursorManualCameraComponent.Cast(editorCamera.FindCameraComponent(SCR_TeleportToCursorManualCameraComponent));
				if (teleportComponent)
				{
					teleportComponent.TeleportCamera(pos);
				}
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void AddedAIAgent(AIAgent agent)
	{
		s_OnAgentSpawned.Invoke(agent);
	}
	
	//------------------------------------------------------------------------------------------------
	override void RemovingAIAgent(AIAgent agent)
	{
		s_OnAgentRemoved.Invoke(agent);
	}
	
	//----------Used for translating enum to message type, confirugable in editor---------------------
	typename GetGoalMessageOfType(EMessageType_Goal eventType)
	{
		return m_aGoalMessagesPacked[eventType].Type();	
	}
	
	typename GetInfoMessageOfType(EMessageType_Info eventType)
	{
		return m_aInfoMessagesPacked[eventType].Type();		
	}
	
	typename GetOrderMessageOfType(EOrderType_Character eventType)
	{
		return m_aOrdersPacked[eventType].Type();
	}
	
	/*!
	Regenerate navmesh in given areas.
	\param areas Array of areas in format (min bounds, max bounds)
	*/
	void RequestNavmeshRebuildAreas(notnull array<ref Tuple2<vector, vector>> areas, notnull array<bool> redoRoads)
	{
#ifdef DEBUG_NAVMESH_REBUILD_AREAS
		vector points[19];
		m_DebugNavmeshRebuildAreas.Clear();
#endif
		int i = 0;
		foreach (Tuple2<vector, vector> area: areas)
		{
			RequestNavmeshRebuild(area.param1, area.param2);
			if(redoRoads[i])
				RequestRoadNetworkRebuild(area.param1, area.param2);
#ifdef DEBUG_NAVMESH_REBUILD_AREAS
			SCR_Shape.GetBoundsPoints(area.param1, area.param2, points);
			m_DebugNavmeshRebuildAreas.Insert(Shape.CreateLines(Color.BLUE, ShapeFlags.NOZBUFFER, points, 19));
#endif
			++i;
		}
	}
	/*!
	Regenerate navmesh around entity and its child entities.
	\param entity Affected entity
	*/
	void RequestNavmeshRebuildEntity(IEntity entity)
	{
		array<ref Tuple2<vector, vector>> areas = new array<ref Tuple2<vector, vector>>; //--- Min, max
		array<bool> redoRoads = new array<bool>;
		GetNavmeshRebuildAreas(entity, areas, redoRoads);
		RequestNavmeshRebuildAreas(areas, redoRoads);
	}
	/*!
	Get navmesh regeneration areas based on entity and its children.
	\param[out] areas Array of areas in format (min bounds, max bounds). Not cleared, will expand on existing areas.
	*/
	void GetNavmeshRebuildAreas(IEntity entity, out notnull array<ref Tuple2<vector, vector>> outAreas, out notnull array<bool> redoRoads)
	{
		//--- No entity, no rebuild needed
		if (!entity)
			return;
		
		//--- Skip entities without physics and mesh (but process their children; e.g., compositions)
		if (entity.GetVObject() && entity.GetPhysics())
		{
			//--- Skip objects not marked for update (ignore their children as well; e.g., soldiers or vehicles)
			if (!(entity.GetPhysics().GetInteractionLayer() & (EPhysicsLayerDefs.Navmesh | EPhysicsLayerDefs.NavmeshVehicle)))
				return;
		
			//--- Get single entity mesh bounds
			vector boundMin, boundMax;
			entity.GetWorldBounds(boundMin, boundMax);
			
			//--- Find the closest bounds
			bool found = false;
			int i = 0;
			foreach (Tuple2<vector, vector> area: outAreas)
			{
				if (vector.DistanceSqXZ(GetAreaCenter(boundMin, boundMax), GetAreaCenter(area.param1, area.param2)) < MAX_NAVMESH_REBUILD_SIZE)
				{
					area.param1 = SCR_Math3D.Min(area.param1, boundMin);
					area.param2 = SCR_Math3D.Max(area.param2, boundMax);
					found = true;
				}
				//Check if the entity has a navlink and it has a link that applies to vehicles
				if (redoRoads[i])
					redoRoads[i] = !HasVehicleNavlink(entity);
				i++;
			}
			
			//--- No suitable bounds found, create new ones
			if (!found)
			{
				outAreas.Insert(new Tuple2<vector, vector>(boundMin, boundMax));	
				//Check if the entity has a navlink and it has a link that applies to vehicles
				redoRoads.Insert(!HasVehicleNavlink(entity));
			}
		}
		//--- Process children
		IEntity child = entity.GetChildren();
		while (child)
		{
			GetNavmeshRebuildAreas(child, outAreas, redoRoads);
			child = child.GetSibling();
		}
	}
	protected vector GetAreaCenter(vector min, vector max)
	{
		return min + (max - min) / 2;
	}
	protected bool HasVehicleNavlink(IEntity entity)
	{
		NavmeshCustomLinkComponent link = NavmeshCustomLinkComponent.Cast(entity.FindComponent(NavmeshCustomLinkComponent));
		if (link)
			return link.HasLinkOfNavmeshType("BTRlike");
		return false;
	}
};