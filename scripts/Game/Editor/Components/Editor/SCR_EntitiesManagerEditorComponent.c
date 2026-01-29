[ComponentEditorProps(category: "GameScripted/Editor", description: "Attribute for managing editable entities. Works only with SCR_EditorBaseEntity!", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_EntitiesManagerEditorComponentClass: SCR_BaseEditorComponentClass
{
};

/** @ingroup Editor_Components
*/
/*!
Manager of editable entities in the editor.

Editable entities are locally managed by SCR_EditableEntityCore, this component manages containers (SCR_BaseEditableEntityFilter) which are responsible for tracking entities of specific state.
*/
class SCR_EntitiesManagerEditorComponent : SCR_BaseEditorComponent
{
	protected const int ASYNC_ENTITY_BATCH_SIZE = 1000; //--- How many entities can be updated in one async cycle
	
	[Attribute()]
	private ref array<ref SCR_BaseEditableEntityFilter> m_aFilters;
	
	private ref array<ref SCR_BaseEditableEntityFilter> m_aFiltersSorted = new array<ref SCR_BaseEditableEntityFilter>();
	private ref array<SCR_BaseEditableEntityFilter> m_aFiltersRoot = new array<SCR_BaseEditableEntityFilter>();
	private ref map<EEditableEntityState, SCR_BaseEditableEntityFilter> m_FiltersMap = new map<EEditableEntityState, SCR_BaseEditableEntityFilter>();
	private ref map<EEditableEntityState, ref array<SCR_BaseEditableEntityFilter>> m_Successors = new map<EEditableEntityState, ref array<SCR_BaseEditableEntityFilter>>();
	private SCR_EditableEntityCore m_Core;
	private bool m_bSkipSuccessors;
	protected int m_iAsyncIndex;
	protected ref set<SCR_EditableEntityComponent> m_AsyncEntities;
	
	/*!
	Get filter managing given state.
	\param state Entity state
	\param showError True to log a warning message when the filter was not found
	\return Entity filter
	*/
	SCR_BaseEditableEntityFilter GetFilter(EEditableEntityState state, bool showError = false)
	{
		SCR_BaseEditableEntityFilter filter = null;
		if (m_FiltersMap.Find(state, filter))
		{
			return filter;
		}
		else
		{
			if (showError)
				Print(string.Format("Cannot find editable entity filter '%1'!", typename.EnumToString(EEditableEntityState, state)), LogLevel.ERROR);
			return null;
		}
	}
	/*!
	Get entities of given state.
	\param state Entity state
	\param[out] entities Array to be filled with entities
	*/
	void GetEntities(EEditableEntityState state, out set<SCR_EditableEntityComponent> entities)
	{
		if (state == 0)
		{
			//--- No predecessor, get all entities
			if (m_Core) m_Core.GetAllEntities(entities);
		}
		else
		{
			//--- Get predecessor's entities
			SCR_BaseEditableEntityFilter filter = null;
			if (m_FiltersMap.Find(state, filter)) filter.GetEntities(entities);
		}
	}
	void SetInSuccessors(EEditableEntityState state, set<SCR_EditableEntityComponent> entitiesInsert, set<SCR_EditableEntityComponent> entitiesRemove)
	{
		if (m_bSkipSuccessors) return;
		
		array<SCR_BaseEditableEntityFilter> successors;
		if (!m_Successors.Find(state, successors)) return;
		
		foreach (SCR_BaseEditableEntityFilter successor: successors)
		{
			if (successor.GetAutoAdd() == EEditableEntityFilterAutoAdd.ALWAYS)
			{
				successor.Set(entitiesInsert, entitiesRemove, true);
			}
			else
			{
				//--- When auto-adding is disabled, don't send 'insert' entities further
				successor.Set(null, entitiesRemove, true);
			}
		}
	}
	protected void OnEntityRegistered(SCR_EditableEntityComponent entity)
	{
		foreach (SCR_BaseEditableEntityFilter filter: m_aFiltersRoot)
		{
			if (filter.GetAutoAdd() == EEditableEntityFilterAutoAdd.ALWAYS)
				filter.Add(entity, false);
		}
	}
	protected void OnEntityUnregistered(SCR_EditableEntityComponent entity)
	{
		foreach (SCR_BaseEditableEntityFilter filter: m_aFiltersRoot)
		{
			filter.Remove(entity, true);
		}
	}
	protected void OnParentEntityChanged(SCR_EditableEntityComponent entity, SCR_EditableEntityComponent parentEntity, SCR_EditableEntityComponent parentEntityPrev)
	{
		m_bSkipSuccessors = true; //--- ToDo: Cleaner?
		foreach (SCR_BaseEditableEntityFilter filter: m_aFiltersSorted)
		{
			if (filter.GetAutoAdd() == EEditableEntityFilterAutoAdd.ALWAYS)
				filter.Validate(entity); //--- Check if the entity is still compatible with auto-add filter
			else
				filter.Remove(entity, true); //--- Simply remove from filters without auto-add (e.g., SELECTED)
		}
		m_bSkipSuccessors = false;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Hierarchy management
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void SetParentEntityServer(int entityID, int parentEntityID)
	{
		SCR_EditableEntityComponent entity = SCR_EditableEntityComponent.Cast(Replication.FindItem(entityID));
		SCR_EditableEntityComponent parentEntity = SCR_EditableEntityComponent.Cast(Replication.FindItem(parentEntityID));
		if (entity) entity.SetParentEntity(parentEntity);
	}
	/*!
	Set parent of the entity.
	\param entity Affected entity
	\param parentEntity New parent. When null, the entity will be moved to the root.
	*/
	void SetParentEntity(SCR_EditableEntityComponent entity, SCR_EditableEntityComponent parentEntity)
	{
		int entityID, parentID;
		if (!entity.IsReplicated(entityID)) return;
		if (!parentEntity.IsReplicated(parentID)) return;

		Rpc(SetParentEntityServer, entityID, parentID);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Default functions
	override void EOnEditorDebug(array<string> debugTexts)
	{
		//if (!GetEditorManager().IsOpened()) return;
		if (!IsActive()) return;
		
		debugTexts.Insert("\n--- Entity Arrays");
		if (SCR_EditorBaseEntity.Cast(GetOwner()).IsOpened())
		{
			foreach (auto filter: m_aFiltersSorted)
			{
				if (!filter) continue;
				debugTexts.Insert(string.Format("%1: %2x", filter.GetStateName(), filter.GetEntitiesCount()));
			}
		}
		else
		{
			foreach (auto filter: m_aFiltersSorted)
			{
				if (!filter) continue;
				set<SCR_EditableEntityComponent> entities = new set<SCR_EditableEntityComponent>;
				filter.GetEntities(entities);
				debugTexts.Insert(string.Format("%1: %2x", filter.GetStateName(), entities.Count()));
			}
		}
		debugTexts.Insert("\n");
	}
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		foreach (auto filter: m_aFiltersSorted)
		{
			if (filter) filter.OnFrameBase(timeSlice);
		}
	}
	override void EOnEditorActivate()
	{
		SetEventMask(GetOwner(), EntityEvent.FRAME);

		m_Core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (!m_Core) return;
		
		m_Core.Event_OnEntityRegistered.Insert(OnEntityRegistered);
		m_Core.Event_OnEntityUnregistered.Insert(OnEntityUnregistered);
		m_Core.Event_OnParentEntityChanged.Insert(OnParentEntityChanged);
	}
	override bool EOnEditorActivateAsync(int attempt)
	{
		//--- Init
		if (attempt == 0)
			m_iAsyncIndex = 0;
		
		//--- Exit
		if (m_iAsyncIndex >= m_aFiltersSorted.Count())
			return true;
		
		SCR_BaseEditableEntityFilter filter = m_aFiltersSorted[m_iAsyncIndex];
		filter.OnActivateBase();
		m_iAsyncIndex++;
		return false;
	}
	override void EOnEditorDeactivate()
	{
		if (!m_Core) return;
		
		m_Core.Event_OnEntityRegistered.Remove(OnEntityRegistered);
		m_Core.Event_OnEntityUnregistered.Remove(OnEntityUnregistered);
		m_Core.Event_OnParentEntityChanged.Remove(OnParentEntityChanged);
	}
	override bool EOnEditorDeactivateAsync(int attempt)
	{
		//--- Init
		if (attempt == 0)
		{
			m_iAsyncIndex = 0;
			m_bSkipSuccessors = true;
		}
		
		if (m_bSkipSuccessors)
		{
			//--- Reset filters
			if (m_iAsyncIndex >= m_aFiltersSorted.Count())
			{
				m_bSkipSuccessors = false;
				m_iAsyncIndex = 0;
				m_AsyncEntities = new set<SCR_EditableEntityComponent>;
				if (m_Core) //--- Can be null when async loading was terminated before this component was activated
					m_Core.GetAllEntities(m_AsyncEntities);
				return false;
			}
			
			//--- Deactivate filter one by one
			SCR_BaseEditableEntityFilter filter = m_aFiltersSorted[m_iAsyncIndex];
			filter.OnDeactivateBase();
		}
		else
		{
			//--- Reset entities
			int indexStart = m_iAsyncIndex * ASYNC_ENTITY_BATCH_SIZE;
			if (indexStart >= m_AsyncEntities.Count())
				return true;
			
			int indexEnd = Math.Min((m_iAsyncIndex + 1) * ASYNC_ENTITY_BATCH_SIZE, m_AsyncEntities.Count());
			for (int i = indexStart; i < indexEnd; i++)
			{
				if (m_AsyncEntities[i])
					m_AsyncEntities[i].ResetEntityStates();
			}
		}
		m_iAsyncIndex++;
		return false;
	}
	override void EOnEditorInit()
	{
		if (!m_aFilters) return;
		
		//--- Get the list of valid values
		array<int> enumValues = {};
		SCR_Enum.GetEnumValues(EEditableEntityState, enumValues);
		enumValues.InsertAt(0, 0);
		
		//--- Get managers
		int i = 0;
		while (!m_aFilters.IsEmpty())
		{			
			SCR_BaseEditableEntityFilter filter = m_aFilters[i];
			SCR_BaseEditableEntityFilter predecessor = null;
			
			//--- Filter not found, ignore it
			if (!filter)
			{
				Print(string.Format("Error when loading entity filter on index %1!", i), LogLevel.ERROR);
				m_aFilters.Remove(i);
				continue;
			}
			
			EEditableEntityState state = filter.GetState();
			EEditableEntityState predecessorState = filter.GetPredecessorState();
			
			//--- Incorrectly configured filter, ignore it
			if (state == 0 || state == predecessorState || !enumValues.Contains(predecessorState))
			{
				Print(string.Format("Error when loading entity filter %1:%2 with predecessor %3:%4!", state, typename.EnumToString(EEditableEntityState, state), predecessorState, typename.EnumToString(EEditableEntityState, predecessorState)), LogLevel.ERROR);
				m_aFilters.Remove(i);
				continue;
			}
			
			if (predecessorState == 0)
			{
				//--- No predecessor, insert at the beginning
				m_aFiltersSorted.InsertAt(filter, 0);
				m_aFiltersRoot.Insert(filter);
			}
			else
			{
				m_FiltersMap.Find(predecessorState, predecessor);
				int predecessorID = m_aFiltersSorted.Find(predecessor);
				if (predecessorID < 0)
				{
					//--- Predecessor not yet registered, skip to the next filter
					i = Math.Repeat(i + 1, m_aFilters.Count()); //--- Prevent index overflow
					continue;
				}
				else
				{
					//--- Insert after the predecessor
					m_aFiltersSorted.InsertAt(filter, predecessorID + 1);
					
					//--- Add to successors
					array<SCR_BaseEditableEntityFilter> predecessorSuccessors;
					m_Successors.Find(predecessor.GetState(), predecessorSuccessors);					
					predecessorSuccessors.Insert(filter);
				}
			}
				
			//--- Start again
			m_FiltersMap.Insert(state, filter);
			m_Successors.Insert(state, new array<SCR_BaseEditableEntityFilter>);
			filter.InitVariables(this, predecessor);
			m_aFilters.Remove(i);
			i = 0;
		}
		m_aFilters = null;
		
		/*
		SCR_EditorModeEntity mode = SCR_EditorModeEntity.Cast(GetOwner());
		Print(mode.GetInfo().GetName(), LogLevel.DEBUG);
		foreach (EEditableEntityState state, ref array<SCR_BaseEditableEntityFilter> list: m_Successors)
		{
			Print(typename.EnumToString(EEditableEntityState, state));
			foreach (SCR_BaseEditableEntityFilter filter: list)
			{
				Print("  " + typename.EnumToString(EEditableEntityState, filter.GetState()), LogLevel.VERBOSE);
			}
		}
		*/
	}
};