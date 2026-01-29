[ComponentEditorProps(category: "GameScripted/Editor", description: "Manages Highlights on entities. Such as notification icons on Faction UI", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_HighlightedEditorComponentClass: SCR_BaseEditorComponentClass
{
};

/** @ingroup Editor_Components
*/
/*!
Manager of current layer.
*/
class SCR_HighlightedEditorComponent : SCR_BaseEditorComponent
{
	//References
	protected SCR_DelegateFactionManagerComponent m_DelegateFactionManager;
	
	//Main Script invokers
	protected ref ScriptInvoker Event_OnAddHighlight = new ScriptInvoker;
	protected ref ScriptInvoker Event_OnUpdateHighlight = new ScriptInvoker;
	protected ref ScriptInvoker Event_OnRemoveHighlight = new ScriptInvoker;
	
	//State
	protected bool m_bReplicationIsRunning;
	
	//Highlighted entities
	protected ref map<SCR_EditableEntityComponent, EEditableEntityHighlight> m_HighlightedEntities = new map<SCR_EditableEntityComponent, EEditableEntityHighlight>;
	
	//-------------------------- Add Highlight types --------------------------\\
	protected void AddHighlight(SCR_EditableEntityComponent entity, EEditableEntityHighlight highlight) //Highlights instead of highlight singilar
	{
		if (!entity) 
			return;
		RplId entityID = Replication.FindId(entity);
		
		EEditableEntityHighlight allHighlights;
		
		//Entity in map
		if (m_HighlightedEntities.Find(entity, allHighlights))
		{
			//Already has highlight
			if ((allHighlights & highlight)) 
				return;
			
			//Add highlight
			allHighlights |= highlight;
			
			//Update Map
			m_HighlightedEntities.Set(entity, allHighlights);		
			Event_OnUpdateHighlight.Invoke(entity, allHighlights, GetHighlightCount(allHighlights));
			
			//Send to owner
			Rpc(AddHighlightRpc, entityID, allHighlights);	
			
		}
		//Entity not in map
		else 
		{
			//Add highlight
			allHighlights |= highlight;
			
			//Add to map
			m_HighlightedEntities.Insert(entity, allHighlights);
			
			//Send to owner
			Rpc(AddHighlightToListRpc, entityID, allHighlights);	
		}
		
		//Cleanup
		CleanupHighlightedMap();
		
		//Server Event
		Event_OnAddHighlight.Invoke(entity);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void AddHighlightRpc(RplId entityID, EEditableEntityHighlight allHighlights)
	{
		SCR_EditableEntityComponent entity = SCR_EditableEntityComponent.Cast(Replication.FindItem(entityID));
		if (!entity)
			return;
		
		//Update map
		m_HighlightedEntities.Set(entity, allHighlights);
		
		//Cleanup
		CleanupHighlightedMap();
		
		//Script invokers
		Event_OnAddHighlight.Invoke(entity);
		Event_OnUpdateHighlight.Invoke(entity, allHighlights, GetHighlightCount(allHighlights));
		
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void AddHighlightToListRpc(RplId entityID, EEditableEntityHighlight allHighlights)
	{
		SCR_EditableEntityComponent entity = SCR_EditableEntityComponent.Cast(Replication.FindItem(entityID));
		if (!entity)
			return;
		
		//Add to map
		m_HighlightedEntities.Insert(entity, allHighlights);
		
		//Cleanup
		CleanupHighlightedMap();
		
		//Script invoker
		Event_OnAddHighlight.Invoke(entity);
	}
	
	
	//-------------------------- Remove Highlight types --------------------------\\
	protected void RemoveHighlight(SCR_EditableEntityComponent entity, EEditableEntityHighlight highlight)
	{
		if (!entity) 
			return;
		RplId entityID = Replication.FindId(entity);
		
		EEditableEntityHighlight allHighlights;
		
		//Not found in map
		if (!m_HighlightedEntities.Find(entity, allHighlights)) 
			return;

		//Does not have highlight
		if (!(allHighlights & highlight)) 
			return;
		
		//Remove highlight
		allHighlights &= ~highlight;
		
		//Not highlighted so remove from map
		if (allHighlights == 0)
		{
			//Remove from map
			m_HighlightedEntities.Remove(entity);
			
			//Send to owner
			Rpc(RemoveHighlightFromListRpc, entityID);	
		}
		//Update map
		else 
		{
			//Update map
			m_HighlightedEntities.Set(entity, allHighlights);
			Event_OnUpdateHighlight.Invoke(entity, allHighlights, GetHighlightCount(allHighlights));
			
			//Send to owner
			Rpc(RemoveHighlightRpc, entityID, allHighlights);	
		}
			
		//Clean up
		CleanupHighlightedMap();
		
		//Server Event
		Event_OnRemoveHighlight.Invoke(entity);
	}
	
	
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RemoveHighlightRpc(RplId entityID, EEditableEntityHighlight allHighlights)
	{
		SCR_EditableEntityComponent entity = SCR_EditableEntityComponent.Cast(Replication.FindItem(entityID));
		if (!entity)
			return;
		
		//Update map
		m_HighlightedEntities.Set(entity, allHighlights);
		
		//Cleanup
		CleanupHighlightedMap();
	
		//Script invokers
		Event_OnRemoveHighlight.Invoke(entity);
		Event_OnUpdateHighlight.Invoke(entity, allHighlights, GetHighlightCount(allHighlights));
	}
	
	
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RemoveHighlightFromListRpc(RplId entityID)
	{
		SCR_EditableEntityComponent entity = SCR_EditableEntityComponent.Cast(Replication.FindItem(entityID));
		if (!entity)
			return;
		
		//Remove from map
		m_HighlightedEntities.Remove(entity);
		
		//Cleanup
		CleanupHighlightedMap();
		
		//Script invoker
		Event_OnRemoveHighlight.Invoke(entity);
	}
	
	
	//-------------------------- Cleanup --------------------------\\
	protected void CleanupHighlightedMap()
	{
  	  	foreach (SCR_EditableEntityComponent entity, EEditableEntityHighlight highlight: m_HighlightedEntities)
    	{
       	 	if (entity == null)
			{
				m_HighlightedEntities.Remove(entity);
			}
   	 	}
	}
	
	//-------------------------- Get is Highlight --------------------------\\
	/*!
	Returns if entity is highlighted
	\param entity to get is highlight
	\return true if highlighted
	*/
	bool IsEntityHighlighted(SCR_EditableEntityComponent entity)
	{
		if (entity == null) 
			return false; 
		
		//Not found in map
		EEditableEntityHighlight allHighlights;
		if (!m_HighlightedEntities.Find(entity, allHighlights)) 
			return false;
		
		return m_HighlightedEntities.Contains(entity);
	}
	
	/*!
	Returns the highlight state and how many flags active or will return false if not highlighted
	\param entity to get highlight
	\param [out] highlight
	\param [out] highlightAmount amount of highlights active on entity
	\return true if highlighted
	*/
	bool GetEntityHighlightedState(SCR_EditableEntityComponent entity, out EEditableEntityHighlight highlight, out int highlightAmount)
	{
		if (entity == null) 
			return false; 
		
		//Not found in map
		EEditableEntityHighlight allHighlights;
		if (!m_HighlightedEntities.Find(entity, allHighlights)) 
			return false;
		
		//Return highlight if one or default if many
		if ((allHighlights & (allHighlights - 1)) == 0)
		{
			highlight = allHighlights;
			highlightAmount = 1;
		}
		else
		{
			highlight = EEditableEntityHighlight.DEFAULT;
			highlightAmount = GetHighlightCount(allHighlights);
		}
			
		return true;
	}
	
	/*!
	Returns if entity has a specific highlight
	\param entity to get is highlight
	\param highlight to check for
	\return true if highlighted by that specific highlight
	*/
	bool HasSpecificHighlight(SCR_EditableEntityComponent entity, EEditableEntityHighlight highlight)
	{
		if (!entity) 
			return false; 
		
		//Not found in map
		EEditableEntityHighlight allHighlights;
		if (!m_HighlightedEntities.Find(entity, allHighlights)) 
			return false;
		
		//Return if has specific highlight
		return (allHighlights & highlight);
	}
	
	
	protected int GetHighlightCount(EEditableEntityHighlight highlight)
	{
		array<int> highlightArray = new array<int>;
		SCR_Enum.BitToIntArray(highlight, highlightArray);
		return highlightArray.Count();
	}
	
	//-------------------------- Faction Spawnpoint --------------------------\\
	protected void FactionSpawnPointsInit()
	{
		if (!m_bReplicationIsRunning) 
			return;
		
		map<Faction, SCR_EditableFactionComponent> factionMap = new map<Faction, SCR_EditableFactionComponent>;
		m_DelegateFactionManager.GetFactionDelegates(factionMap);
		
		foreach (Faction faction, SCR_EditableFactionComponent editableFaction: factionMap)
    	{	
			if (editableFaction.GetFactionSpawnPointCount() == 0)
				AddHighlight(editableFaction, EEditableEntityHighlight.NO_FACTION_SPAWNPOINTS);				
   	 	}
	}
	
	protected void OnFactionSpawnPointsUpdated(Faction faction, int factionSpawnPointCount)
	{	
		if (!m_bReplicationIsRunning) 
			return;
		
		//Get editable entity
		SCR_EditableFactionComponent factionEntity = m_DelegateFactionManager.GetFactionDelegate(faction);
		if (!factionEntity) 
			return;
		
		//Send to owner
		if (factionSpawnPointCount == 0)
		{
			if (!HasSpecificHighlight(factionEntity, EEditableEntityHighlight.NO_FACTION_SPAWNPOINTS))
				AddHighlight(factionEntity, EEditableEntityHighlight.NO_FACTION_SPAWNPOINTS);	
		}
		else 
		{
			if (HasSpecificHighlight(factionEntity, EEditableEntityHighlight.NO_FACTION_SPAWNPOINTS))
				RemoveHighlight(factionEntity, EEditableEntityHighlight.NO_FACTION_SPAWNPOINTS);			
		}		
	}
	
	
	//-------------------------- Faction Tasks --------------------------\\
	protected void FactionTasksInit()
	{	
		if (!m_bReplicationIsRunning) 
			return;
		
		map<Faction, SCR_EditableFactionComponent> factionMap = new map<Faction, SCR_EditableFactionComponent>;
		m_DelegateFactionManager.GetFactionDelegates(factionMap);
		
		foreach (Faction faction, SCR_EditableFactionComponent editableFaction: factionMap)
    	{
			if (editableFaction.GetFactionTasksCount() == 0)
				AddHighlight(editableFaction, EEditableEntityHighlight.NO_FACTION_TASKS);				
   	 	}
	}
	
	protected void OnFactionTasksUpdated(Faction faction, int factionTaskCount)
	{	
		if (!m_bReplicationIsRunning) 
			return;
		
		//Get editable entity
		SCR_EditableFactionComponent factionEntity = m_DelegateFactionManager.GetFactionDelegate(faction);
		if (!factionEntity) 
			return;
		
		//Send to owner
		if (factionTaskCount == 0)
		{
			if (!HasSpecificHighlight(factionEntity, EEditableEntityHighlight.NO_FACTION_TASKS))
				AddHighlight(factionEntity, EEditableEntityHighlight.NO_FACTION_TASKS);
		}
		else 
		{
			if (HasSpecificHighlight(factionEntity, EEditableEntityHighlight.NO_FACTION_TASKS))
				RemoveHighlight(factionEntity, EEditableEntityHighlight.NO_FACTION_TASKS);
		}
	}
	
	
	//-------------------------- Get Invokers --------------------------\\
	ScriptInvoker GetOnAddHighlight()
	{
		return Event_OnAddHighlight;
	}
	
	ScriptInvoker GetOnRemoveHighlight()
	{
		return Event_OnRemoveHighlight;
	}
	
	ScriptInvoker GetOnUpdateHighlight()
	{
		return Event_OnUpdateHighlight;
	}
	
	//-------------------------- On Editor Open/Close --------------------------\\
	override void EOnEditorOpenServer()
	{	
		m_bReplicationIsRunning = Replication.IsRunning();
		
		m_DelegateFactionManager = SCR_DelegateFactionManagerComponent.GetInstance();		
		
		if (m_DelegateFactionManager)
		{
			FactionSpawnPointsInit();
			FactionTasksInit();
			
			map<Faction, SCR_EditableFactionComponent> factionMap = new map<Faction, SCR_EditableFactionComponent>;
			m_DelegateFactionManager.GetFactionDelegates(factionMap);
			
			foreach (Faction faction, SCR_EditableFactionComponent editableFaction: factionMap)
    		{	
				editableFaction.GetOnSpawnPointCountChanged().Insert(OnFactionSpawnPointsUpdated);	
				editableFaction.GetOnTaskCountChanged().Insert(OnFactionTasksUpdated);	
   	 		}
		}
	}
	
	override void EOnEditorCloseServer()
	{
		if (m_DelegateFactionManager)
		{
			map<Faction, SCR_EditableFactionComponent> factionMap = new map<Faction, SCR_EditableFactionComponent>;
			m_DelegateFactionManager.GetFactionDelegates(factionMap);
			
			foreach (Faction faction, SCR_EditableFactionComponent editableFaction: factionMap)
    		{	
				editableFaction.GetOnSpawnPointCountChanged().Remove(OnFactionSpawnPointsUpdated);	
				editableFaction.GetOnTaskCountChanged().Remove(OnFactionTasksUpdated);	
   	 		}
		}
		
		//Clear highlighted list
		m_HighlightedEntities.Clear();
	}
};