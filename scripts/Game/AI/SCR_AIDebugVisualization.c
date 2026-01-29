class SCR_AIDebugVisualizationClass: GenericEntityClass
{
};

/*!
Entity which performs various AI debugging visualizations.
*/

class SCR_AIDebugVisualization : GenericEntity
{
	static SCR_AIDebugVisualization s_Instance;
	protected ref array<ref SCR_AIMessageVisualization> m_aElements = {};
	protected ref array<ref SCR_AIAgentDebugPanel> m_aPanels = {};
	
	//------------------------------------------------------------------------------------------------
	static void Init()
	{
		if (s_Instance)
			SCR_EntityHelper.DeleteEntityAndChildren(s_Instance);
		
		s_Instance = SCR_AIDebugVisualization.Cast(GetGame().SpawnEntity(SCR_AIDebugVisualization, GetGame().GetWorld()));
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_AIDebugVisualization GetInstance()
	{
		if (!s_Instance)
			s_Instance = SCR_AIDebugVisualization.Cast(GetGame().SpawnEntity(SCR_AIDebugVisualization, GetGame().GetWorld()));
		
		return s_Instance;
	}
	
	//------------------------------------------------------------------------------------------------
	static void VisualizeMessage(IEntity entity, string message, EAIDebugCategory category, float showTime, Color color = Color.White, float fontSize = 16, bool ignoreCategory = false)
	{
		int i =  DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_AI_DEBUG_CATEGORY);
		if (category != DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_AI_DEBUG_CATEGORY) && !ignoreCategory)
			return;
		
		SCR_AIDebugVisualization inst = GetInstance();
		
		if (!inst || !entity || message == string.Empty)
			return;

		if (!color)
			color = Color.FromInt(Color.WHITE);

		inst.RemoveVisualization(entity);
		
		// Create element
		SCR_AIMessageVisualization visualization = new SCR_AIMessageVisualization(entity, message, showTime, Color.FromInt(color.PackToInt()), fontSize);
		inst.m_aElements.Insert(visualization);
	}
	
	//------------------------------------------------------------------------------------------------
	void ShowAiAgentDebugPanel(AIAgent agent, IEntity entity)
	{
		SCR_AIAgentDebugPanel existingPanel;
		foreach (SCR_AIAgentDebugPanel p : m_aPanels)
		{
			if ((p.m_Agent && p.m_Agent == agent) || (p.m_Entity && p.m_Entity == entity))
				existingPanel = p;
		}
		
		// Do nothing if this agent already has a panel
		if (existingPanel)
			return;
		
		SCR_AIAgentDebugPanel newPanel = new SCR_AIAgentDebugPanel(agent, entity);
		m_aPanels.Insert(newPanel);
	}

	//------------------------------------------------------------------------------------------------	
	protected void RemoveVisualization(IEntity entity)
	{
		foreach (SCR_AIMessageVisualization vis : m_aElements)
		{
			if (!vis)
				continue;
			
			if (vis.m_TargetEntity == entity)
			{
				m_aElements.RemoveItem(vis);
				vis = null;
				return;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void EOnDiag(IEntity owner, float timeSlice)
	{
		// Update messages above AIs
		bool enabled = DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_AI_DEBUG_CATEGORY);
		if (enabled)
		{
			int count = m_aElements.Count();
			for (int i = count - 1; i >=0; i--)
			{
				bool finished = m_aElements[i].Draw(timeSlice);
				if (finished)
					m_aElements.Remove(i);
			}
		}
		
		// Update AI debug panels
		bool openDebugPanel = DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_OPEN_DEBUG_PANEL);
		if (openDebugPanel)
		{
			AIAgent selectedAgent;
			IEntity selectedEntity;
			
			if (GetSelectedAiAgentOrEntity(selectedAgent, selectedEntity))
				ShowAiAgentDebugPanel(selectedAgent, selectedEntity);
			else
				Debug.Error("Nothing is selected! You must select something with Game Master first!");
			
			DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_AI_OPEN_DEBUG_PANEL, false);
		}
		
		int count = m_aPanels.Count();
		for (int i = count - 1; i >= 0; i--)
		{
			bool requestClose = m_aPanels[i].Update(timeSlice);
			if (requestClose)
				m_aPanels.Remove(i);
		}
		
		// Set BT breakpoint for selected AI agent
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_SET_BT_BREAKPOINT))
		{
			AIAgent selectedAgent;
			IEntity selectedEntity;
			GetSelectedAiAgentOrEntity(selectedAgent, selectedEntity);
			if (selectedAgent)
			{
				AIBehaviorTreeComponent btComp = AIBehaviorTreeComponent.Cast(selectedAgent.FindComponent(AIBehaviorTreeComponent));
				if (btComp)
				{
					btComp.SetBtBreakpoint(true);
				}
			}
			
			DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_AI_SET_BT_BREAKPOINT, false);
		}
		
		// Perception manager
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_SHOW_PERCEPTION_PANEL))
		{
			ShowPerceptionPanel();
		}
	}
	
	//-------------------------------------------------------------------------------------------
	void ShowPerceptionPanel()
	{
		DbgUI.Begin("Perception Panel");
		
		DbgUI.Text("Values for tuning perception properties");
		
		ChimeraWorld world = GetWorld();
		BaseTimeAndWeatherManagerEntity twm = world.GetTimeAndWeatherManager();
		if (!twm)
		{
			DbgUI.Text("BaseTimeAndWeatherManagerEntity not found!");
		}
		else
		{
			vector sunDir;
			vector moonDir;
			float moonPhase;
			twm.GetCurrentSunMoonDirAndPhase(sunDir, moonDir, moonPhase);
			float sunElevDeg = Math.RAD2DEG * Math.Asin(-sunDir[1]);
			float moonElevDeg = Math.RAD2DEG * Math.Asin(-moonDir[1]);
			DbgUI.Text(string.Format("Sun Elevation:  %1 deg", sunElevDeg.ToString(5, 1)));
			DbgUI.Text(string.Format("Moon ElevatioN: %1 deg", moonElevDeg.ToString(5, 1)));
			DbgUI.Text(string.Format("Moon Phase: %1", moonPhase.ToString(4, 2)));
		}
		
		PerceptionManager pm = GetGame().GetPerceptionManager();
		if (!pm)
		{
			DbgUI.Text("PerceptionManager not found!");
		}
		else
		{
			float directLV;
			float ambientLV;
			float totalLV;
			pm.GetAmbientLV(directLV, ambientLV, totalLV);
			DbgUI.Text(string.Format("Direct LV: %1, Ambient LV: %2, Total Ambient LV: %3", directLV, ambientLV, totalLV));
		}
		
		DbgUI.End();
	}
		
	//-------------------------------------------------------------------------------------------
	protected bool GetSelectedAiAgentOrEntity(out AIAgent outAgent, out IEntity outEntity)
	{
		// Get first AI entity selected in Game Master
		SCR_BaseEditableEntityFilter filter = SCR_BaseEditableEntityFilter.GetInstance(EEditableEntityState.SELECTED);
		set<SCR_EditableEntityComponent> selectedEntities = new set<SCR_EditableEntityComponent>();
		filter.GetEntities(selectedEntities);
		
		if (selectedEntities.Count() == 0)
		{
			outAgent = null;
			outEntity = null;
			return false;
		}
			
		// Try to find a group
		foreach (SCR_EditableEntityComponent comp : selectedEntities)
		{
			SCR_EditableGroupComponent editGroupComp = SCR_EditableGroupComponent.Cast(comp);	 
			if (editGroupComp)
			{
				outAgent = AIAgent.Cast(editGroupComp.GetOwner());
				outEntity = null;
				return true;
			}
		}
		
		// Group not found, select first unit
		SCR_EditableCharacterComponent editCharacterComp = SCR_EditableCharacterComponent.Cast(selectedEntities[0]);
		if (editCharacterComp)
		{
			outAgent = editCharacterComp.GetAgent();
			outEntity = outAgent.GetControlledEntity();
			return true;
		}
		
		// It's not an AI or group but could be a basic entity
		outAgent = null;
		outEntity = selectedEntities[0].GetOwner();
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_AIDebugVisualization(IEntitySource src, IEntity parent)
	{
		s_Instance = this;
		
		SetEventMask(EntityEvent.FRAME);
		ConnectToDiagSystem();
		SetFlags(EntityFlags.ACTIVE, true);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_AIDebugVisualization()
	{
		DisconnectFromDiagSystem();
		if (m_aElements)
			m_aElements.Clear();
		m_aElements = null;
	}
};
