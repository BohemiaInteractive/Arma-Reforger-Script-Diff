class SCR_ScenarioPanelEditorUIComponent : ScriptedWidgetComponent
{
	[Attribute("ToolbarBackgroundOverlay")]
	protected string m_sFullWindowName;
	
	[Attribute("ConfigureScenarioButton")]
	protected string m_sButtonOnlyName;
	
	protected Widget m_wRoot;
	
	protected bool m_bListeningToEvents;
	
	//------------------------------------------------------------------------------------------------
	//Show/hide full UI
	protected void ShowButtonOrFullWindow(bool showFullWindow)
	{
		Widget fullWindow = m_wRoot.FindAnyWidget(m_sFullWindowName);
		Widget buttonOnly = m_wRoot.FindAnyWidget(m_sButtonOnlyName);
		
		if (!fullWindow || !buttonOnly)
			return;
		
		fullWindow.SetVisible(showFullWindow);
		buttonOnly.SetVisible(!showFullWindow);
	}
	
	//------------------------------------------------------------------------------------------------
	//When an attribute has been changed
	protected void OnAttributeChanges()
	{
		StopListeningToEvents();
		ShowButtonOrFullWindow(true);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPlayerSpawned(int playerID, SCR_EditableEntityComponent entity, SCR_EditableEntityComponent prevEntity)
	{
		SCR_PlayersManagerEditorComponent editorPlayerManager = SCR_PlayersManagerEditorComponent.Cast(SCR_PlayersManagerEditorComponent.GetInstance(SCR_PlayersManagerEditorComponent));
		if (!editorPlayerManager || !editorPlayerManager.HasLocalPlayerSpawnedOnce())
			return;
		
		StopListeningToEvents();
		ShowButtonOrFullWindow(true);
	}
	
	//------------------------------------------------------------------------------------------------
	//When faction is set playable
	protected void OnFactionSetPlayable(SCR_Faction faction, bool playable)
	{
		if (!playable)
			return;
		
		StopListeningToEvents();
		ShowButtonOrFullWindow(true);
	}
	
	//------------------------------------------------------------------------------------------------
	//Remove all event listeners
	protected void StopListeningToEvents()
	{
		if (!m_bListeningToEvents)
			return;
		
		m_bListeningToEvents = false;
		
		//Remove on faction set playable listener
		SCR_DelegateFactionManagerComponent delegateFactionManager = SCR_DelegateFactionManagerComponent.GetInstance();
		if (delegateFactionManager)
		{
			map<Faction, SCR_EditableFactionComponent> delegates = new map<Faction, SCR_EditableFactionComponent>();
			delegateFactionManager.GetFactionDelegates(delegates);
			SCR_Faction scrFaction;
			
			foreach (Faction f, SCR_EditableFactionComponent d : delegates)
			{
				scrFaction = SCR_Faction.Cast(f);
				
				if (!scrFaction)
					continue;
				
				scrFaction.GetOnFactionPlayableChanged().Remove(OnFactionSetPlayable);
			}
		}
		
		//Remove on attribute changes listener
		SCR_AttributesManagerEditorComponent attributeManager = SCR_AttributesManagerEditorComponent.Cast(SCR_AttributesManagerEditorComponent.GetInstance(SCR_AttributesManagerEditorComponent));
		if (attributeManager)
			attributeManager.GetOnAttributeChangesApplied().Remove(OnAttributeChanges);
		
		//Remove on local player spawned listener
		SCR_PlayersManagerEditorComponent editorPlayerManager = SCR_PlayersManagerEditorComponent.Cast(SCR_PlayersManagerEditorComponent.GetInstance(SCR_PlayersManagerEditorComponent));
		if (editorPlayerManager)
			editorPlayerManager.GetOnSpawn().Remove(OnPlayerSpawned);
	}	
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		//Are there any playable factions
		SCR_DelegateFactionManagerComponent delegateFactionManager = SCR_DelegateFactionManagerComponent.GetInstance();
		if (!delegateFactionManager|| delegateFactionManager.GetPlayableFactionDelegateCount() > 0)
			return;
		
		//Has attribute been changed yet
		SCR_AttributesManagerEditorComponent attributeManager = SCR_AttributesManagerEditorComponent.Cast(SCR_AttributesManagerEditorComponent.GetInstance(SCR_AttributesManagerEditorComponent, true));
		if (!attributeManager || attributeManager.GetChangedAttributesOnce())
			return;
		
		//Local player has spawned yet
		SCR_PlayersManagerEditorComponent editorPlayerManager = SCR_PlayersManagerEditorComponent.Cast(SCR_PlayersManagerEditorComponent.GetInstance(SCR_PlayersManagerEditorComponent));
		if (!editorPlayerManager || editorPlayerManager.HasLocalPlayerSpawnedOnce())
			return;
		
		m_wRoot = w;
		
		//Show button only
		ShowButtonOrFullWindow(false);
		
		//Listen to events
		m_bListeningToEvents = true;
		
		//On faction set playable listener
		map<Faction, SCR_EditableFactionComponent> delegates = new map<Faction, SCR_EditableFactionComponent>();
		delegateFactionManager.GetFactionDelegates(delegates);
		SCR_Faction scrFaction;
		
		foreach (Faction f, SCR_EditableFactionComponent d : delegates)
		{
			scrFaction = SCR_Faction.Cast(f);
			
			if (!scrFaction)
				continue;
			
			scrFaction.GetOnFactionPlayableChanged().Insert(OnFactionSetPlayable);
		}
		
		//On attribute changes listener
		attributeManager.GetOnAttributeChangesApplied().Insert(OnAttributeChanges);
		
		//On local player spawned listener
		if (editorPlayerManager)
			editorPlayerManager.GetOnSpawn().Insert(OnPlayerSpawned);
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		StopListeningToEvents();
	}
}
