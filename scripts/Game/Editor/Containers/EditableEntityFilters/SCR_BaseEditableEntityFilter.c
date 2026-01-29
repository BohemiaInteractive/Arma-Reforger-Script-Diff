void SCR_BaseEditableEntityFilter_OnChange(EEditableEntityState state, set<SCR_EditableEntityComponent> entitiesInsert, set<SCR_EditableEntityComponent> entitiesRemove);
typedef func SCR_BaseEditableEntityFilter_OnChange;

/** @ingroup Editor_Components_Entities
*/
/*!
Base class for a manager of editable entities of specific state.

Attached on SCR_EntitiesManagerEditorComponent.
*/
//#define ENTITY_FILTER_DEBUG
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityState, "m_State")]
class SCR_BaseEditableEntityFilter
{
	[Attribute(desc: "Entity state this component manages.", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EEditableEntityState))]
	protected EEditableEntityState m_State;

	[Attribute(desc: "Entity state which preceeds this component.\nOnly entities which are registered in the predecessor will be evaluated by this component.", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EEditableEntityState))]
	protected EEditableEntityState m_PredecessorState;

	[Attribute(defvalue: "0", desc: "True if predecessors' entities should be automatically registered in this component as well.", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EEditableEntityFilterAutoAdd))]
	protected EEditableEntityFilterAutoAdd m_bAutoAdd;

	[Attribute(desc: "When true, entities with IGNORE_LAYERS will not be evaluated when their parent changes state of this filter.")]
	protected bool m_bSkipIgnored;

	[Attribute(desc: "When true, only entities without NON_INTERACTIVE flag will be allowed in.")]
	protected bool m_bOnlyInteractive;

	protected SCR_EditableEntityCore m_Core;
	protected SCR_EntitiesManagerEditorComponent m_Manager;
	protected SCR_BaseEditableEntityFilter m_Predecessor;
	protected ref set<SCR_EditableEntityComponent> m_Entities;
	protected int m_iEntitiesCount; //--- Cached value for faster access
	protected string m_sStateName = "N/A";
	protected ref ScriptInvokerBase<SCR_BaseEditableEntityFilter_OnChange> m_OnChange = new ScriptInvokerBase<SCR_BaseEditableEntityFilter_OnChange>();

#ifdef ENTITY_FILTER_DEBUG
	protected EEditableEntityState m_DebugState = EEditableEntityState.FOCUSED;
#endif

	void InitVariables(SCR_EntitiesManagerEditorComponent manager, SCR_BaseEditableEntityFilter predecessor)
	{
		if (m_Manager) return;
		m_Manager = manager;
		m_Predecessor = predecessor;
	}
	/*!
	Get current filter instance.
	\param Desired entity state
	\param showError True to log a warning message when the filter was not found
	\return Entity state
	*/
	static SCR_BaseEditableEntityFilter GetInstance(EEditableEntityState state, bool showError = false)
	{
		SCR_EntitiesManagerEditorComponent entitiesManager = SCR_EntitiesManagerEditorComponent.Cast(SCR_EntitiesManagerEditorComponent.GetInstance(SCR_EntitiesManagerEditorComponent));
		if (entitiesManager)
		{
			return entitiesManager.GetFilter(state, showError);
		}
		else
		{
			if (showError) Print(string.Format("Cannot find editable entity filter '%1', entities manager not found!", typename.EnumToString(EEditableEntityState, state)), LogLevel.ERROR);
			return null;
		}
	}
	/*!
	Get first entity of given filter type.
	\return Editable entity
	*/
	static SCR_EditableEntityComponent GetFirstEntity(EEditableEntityState state)
	{
		SCR_BaseEditableEntityFilter filter = SCR_BaseEditableEntityFilter.GetInstance(state);
		if (filter)
			return filter.GetFirstEntity();
		else
			return null;
	}
	/*!
	Get first entity of given filter type.
	\return Editable entity
	*/
	static int GetEnititiesStatic(out set<SCR_EditableEntityComponent> entities, EEditableEntityState state, bool includeChildren = false, bool evaluate = true)
	{
		SCR_BaseEditableEntityFilter filter = SCR_BaseEditableEntityFilter.GetInstance(state);
		if (filter)
			return filter.GetEntities(entities, includeChildren, evaluate);
		else
			return 0;
	}
	/*!
	Get entity state this component is managing.
	\return Entity state
	*/
	EEditableEntityState GetState()
	{
		return m_State;
	}
	/*!
	Get name of the entity state this component is managing.
	\return Entity state name
	*/
	string GetStateName()
	{
		return m_sStateName;
	}
	/*!
	Get entity state of the component which preceeds this one.
	\return Entity state
	*/
	EEditableEntityState GetPredecessorState()
	{
		return m_PredecessorState;
	}
	/*!
	Get component which preceeds this one.
	\return Entity manager component
	*/
	SCR_BaseEditableEntityFilter GetPredecessor()
	{
		return m_Predecessor;
	}
	/*!
	Check if entities can be registered automatically based on predecessor component.
	\return True if automatic adding is enabled
	*/
	EEditableEntityFilterAutoAdd GetAutoAdd()
	{
		return m_bAutoAdd;
	}
	/*!
	Get entities managed by this filter.
	When the editor is opened, cached array is returned.
	When the editor is closed, the array is re-calculated based on conditions of this component and all its predecessors. Use sparingly!
	\param[out] entities Array to be filled with entities
	\param includeChildren True to also include all children of filtered entities (even when they don't meet filter's condition)
	\return Number of filtered entities
	*/
	int GetEntities(out set<SCR_EditableEntityComponent> entities, bool includeChildren = false, bool evaluate = true)
	{
		if (m_Entities)
		{
			//--- Return cached entities directly
			entities.Copy(m_Entities);
		}
		else
		{
			if (m_bAutoAdd == EEditableEntityFilterAutoAdd.NEVER)
				return 0;

			//--- Get entities filtered by the predecessor
			if (m_Predecessor)
				m_Predecessor.GetEntities(entities);
			else if (GetCore())
				GetCore().GetAllEntities(entities);

			//--- Remove incompatible entities
			if (evaluate)
			{
				for (int i = 0; i < entities.Count(); i++)
				{
					if (!CanAdd(entities[i]))
					{
						entities.Remove(i);
						i--;
					}
				}
			}
		}

		//--- Add children of filtered entities
		if (includeChildren)
			InsertChildren(entities);

		return entities.Count();
	}
	/*!
	Get number of cached entities.
	\return Entity count
	*/
	int GetEntitiesCount()
	{
		return m_iEntitiesCount;
		//if (m_Entities) return m_Entities.Count();
		//return 0;
	}
	
	/*!
	Check if the filter contains any entities.
	\return True when empty
	*/
	bool IsEmpty()
	{
		return m_iEntitiesCount == 0;
	}
	
	/*!
	Get the first cached entity.
	\return Entity. When no entity is registered or no array is cached, null is returned.
	*/
	SCR_EditableEntityComponent GetFirstEntity()
	{
		if (m_Entities && !m_Entities.IsEmpty()) return m_Entities[0];
		return null;
	}
	
	/*!
	Check if given entity is among cached entities.
	\return True if registered
	*/
	bool Contains(SCR_EditableEntityComponent entity)
	{
		return m_Entities && m_Entities.Find(entity) != -1;
	}
	
	/*!
	Get script invoker which is triggered every time some entities are added or removed.
	\return Script invoker
	*/
	ScriptInvokerBase<SCR_BaseEditableEntityFilter_OnChange> GetOnChanged()
	{
		return m_OnChange;
	}

	protected int GetEntitiesPointer(out set<SCR_EditableEntityComponent> entities)
	{
		entities = m_Entities; //--- Faster, but unsafe
		return m_iEntitiesCount;
	}

	protected SCR_EditableEntityComponent GetParentBelowCurrentLayer(SCR_EditableEntityComponent entity)
	{
		SCR_LayersEditorComponent layersManager = SCR_LayersEditorComponent.Cast(SCR_LayersEditorComponent.GetInstance(SCR_LayersEditorComponent));
		if (layersManager)
		{
			SCR_EditableEntityComponent parentBelowCurrentLayer = null;
			parentBelowCurrentLayer = layersManager.GetParentBelowCurrentLayer(entity);
			if (parentBelowCurrentLayer) return parentBelowCurrentLayer;
		}
		return entity;
	}

	protected sealed SCR_EntitiesManagerEditorComponent GetManager()
	{
		return m_Manager;
	}

	protected sealed SCR_EditableEntityCore GetCore()
	{
		return m_Core;
	}

	protected void InitEntities()
	{
		if (m_Entities) return;

		//--- Filter only compatible entities
		if (m_bAutoAdd != EEditableEntityFilterAutoAdd.NEVER)
		{
			set<SCR_EditableEntityComponent> entities = new set<SCR_EditableEntityComponent>;
			GetEntities(entities, evaluate: false); //--- Don't evaluate conditions, it will happen in Add() function
			m_Entities = new set<SCR_EditableEntityComponent>;
			m_iEntitiesCount = 0;
			Add(entities);
		}
		else
		{
			m_Entities = new set<SCR_EditableEntityComponent>;
			m_iEntitiesCount = 0;
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Array manipulation
	protected void OnChanged(set<SCR_EditableEntityComponent> entitiesInsert, set<SCR_EditableEntityComponent> entitiesRemove)
	{
		m_OnChange.Invoke(m_State, entitiesInsert, entitiesRemove);
		m_Manager.SetInSuccessors(m_State, entitiesInsert, entitiesRemove);
	}

	protected void OnChanged()
	{
		m_OnChange.Invoke(m_State, m_Entities, null);
	}

	/*! @name Array Manipulation Functions
	Functions to add or remove entities to the cached array.
	*/
	///@{

	/*!
	Register given entity.
	\param entityInsert Entity to be registered
	\param onlyDirect True to also include children of all affected entities
	\return True if cached array changed
	*/
	bool Add(SCR_EditableEntityComponent entityInsert, bool onlyDirect = false)
	{
		if (!entityInsert)
			return false;
		
		set<SCR_EditableEntityComponent> entitiesInsert = new set<SCR_EditableEntityComponent>;
		entitiesInsert.Insert(entityInsert);
		
		return Add(entitiesInsert, onlyDirect);
	}

	/*!
	Register multiple entities.
	\param entitiesInsert Entities to be registered
	\param onlyDirect True to also include children of all affected entities
	\return True if cached array changed
	*/
	bool Add(notnull set<SCR_EditableEntityComponent> entitiesInsert, bool onlyDirect = false)
	{
		if (!m_Entities)
			return false;

		bool isChange = false;
		set<SCR_EditableEntityComponent> entitiesInsertProcessed = new set<SCR_EditableEntityComponent>;
		if (!onlyDirect)
			InsertChildren(entitiesInsert);
		
		foreach (SCR_EditableEntityComponent entity : entitiesInsert)
		{
			if (AddToArray(entity, isChange))
				entitiesInsertProcessed.Insert(entity);
		}
		
		if (isChange)
			OnChanged(entitiesInsertProcessed, null);
		
		return isChange;
	}

	/*!
	Unregister given entity.
	\param entityRemove Entity to be unregistered
	\param onlyDirect True to also include children of all affected entities
	\return True if cached array changed
	*/
	bool Remove(SCR_EditableEntityComponent entityRemove, bool onlyDirect = false)
	{
		if (!entityRemove)
			return false;
		
		set<SCR_EditableEntityComponent> entitiesRemove = new set<SCR_EditableEntityComponent>;
		entitiesRemove.Insert(entityRemove);
		
		return Remove(entitiesRemove, onlyDirect);
	}

	/*!
	Unregister multiple entities.
	\param entitiesRemove Entities to be unregistered
	\param onlyDirect True to also include children of all affected entities
	\return True if cached array changed
	*/
	bool Remove(notnull set<SCR_EditableEntityComponent> entitiesRemove, bool onlyDirect = false)
	{
		if (!m_Entities)
			return false;

		bool isChange = false;
		set<SCR_EditableEntityComponent> entitiesRemoveProcessed = new set<SCR_EditableEntityComponent>;
		if (!onlyDirect)
			InsertChildren(entitiesRemove);

		foreach (SCR_EditableEntityComponent entity : entitiesRemove)
		{
			if (RemoveFromArray(entity, isChange))
				entitiesRemoveProcessed.Insert(entity);
		}
		if (isChange)
			OnChanged(null, entitiesRemoveProcessed);

		return isChange;
	}

	/*!
	Toggle state of given entity, i.e., register it if it's unregistered and unregister it if it's registered.
	\param entity Affected entity
	\param onlyDirect True to also include children of all affected entities
	\return True if cached array changed
	*/
	bool Toggle(SCR_EditableEntityComponent entity, bool onlyDirect = false)
	{
		if (CanAdd(entity))
			return Add(entity, onlyDirect);
		else
			return Remove(entity, onlyDirect);
	}

	/*!
	Toggle state of given entities, i.e., register them if they're unregistered and unregister them if they're registered.
	\param entities Affected entities
	\param onlyDirect True to also include children of all affected entities
	\return True if cached array changed
	*/
	bool Toggle(notnull set<SCR_EditableEntityComponent> entities, bool onlyDirect = false)
	{
		if (!m_Entities)
			return false;

		set<SCR_EditableEntityComponent> entitiesInsert = new set<SCR_EditableEntityComponent>;
		set<SCR_EditableEntityComponent> entitiesRemove = new set<SCR_EditableEntityComponent>;
		foreach (SCR_EditableEntityComponent entity : entities)
		{
			if (Contains(entity))
				entitiesRemove.Insert(entity);
			else
				entitiesInsert.Insert(entity);
		}

		return Set(entitiesInsert, entitiesRemove, onlyDirect);
	}

	/*!
	Replace cached array with given entity. ALl currently registered entities will be removed.
	\param entityInsert Entity to be registered
	\param onlyDirect True to also include children of all affected entities
	\para, keepExisting True to keep all already listed entities, provided they still pass filter condition
	\return True if cached array changed
	*/
	bool Replace(SCR_EditableEntityComponent entityInsert, bool onlyDirect = false, bool keepExisting = false)
	{
		if (!m_Entities) return false;

		if (entityInsert)
		{
			set<SCR_EditableEntityComponent> entitiesInsert = new set<SCR_EditableEntityComponent>;
			entitiesInsert.Insert(entityInsert);
			return Replace(entitiesInsert, onlyDirect, keepExisting);
		}
		else
		{
			return Clear();
		}
	}
	/*!
	Replace cached array with entities. ALl currently registered entities will be removed.
	\param entitiesInsert Entities to be registered
	\param onlyDirect True to also include children of all affected entities
	\para, keepExisting True to keep all already listed entities, provided they still pass filter condition
	\return True if cached array changed
	*/
	bool Replace(set<SCR_EditableEntityComponent> entitiesInsert, bool onlyDirect = false, bool keepExisting = false)
	{
		if (!m_Entities)
			return false;

		/*
		set<SCR_EditableEntityComponent> entitiesRemove = new set<SCR_EditableEntityComponent>;
		entitiesRemove.Copy(m_Entities);
		return Set(entitiesInsert, entitiesRemove, onlyDirect);
		*/

		if (!onlyDirect) InsertChildren(entitiesInsert);

		bool isChange = false;
		set<SCR_EditableEntityComponent> entitiesInsertProcessed = new set<SCR_EditableEntityComponent>;
		set<SCR_EditableEntityComponent> entitiesRemoveProcessed = new set<SCR_EditableEntityComponent>;

		//--- Process existing entities
		if (keepExisting)
		{
			//--- Append all existing entities
			foreach (SCR_EditableEntityComponent entity : m_Entities)
			{
				entitiesInsert.Insert(entity);
			}
		}
		else
		{
			//--- Remove entities which are not marked for insertion
			SCR_EditableEntityComponent entity;
			for (int i = 0, entitiesCount = m_Entities.Count(); i < entitiesCount; i++)
			{
				entity = m_Entities[i];
				if (entitiesInsert.Find(entity) == -1 && RemoveFromArray(entity, isChange))
				{
					//--- Current element not marked for insertion, remove it
					entitiesRemoveProcessed.Insert(entity);
					entitiesCount--;
					i--;
				}
			}
		}

		//--- Process new entities and make sure they are in the list
		bool isChangeEntity;
		foreach (SCR_EditableEntityComponent entityInsert : entitiesInsert)
		{
			isChangeEntity = false;
			if (AddToArray(entityInsert, isChangeEntity, true))
			{
				if (isChangeEntity)
					entitiesInsertProcessed.Insert(entityInsert);
			}
			else if (RemoveFromArray(entityInsert, isChangeEntity)) //--- Marked for insertion, but not compatible - remove if it already was in the list
			{
				entitiesRemoveProcessed.Insert(entityInsert);
			}
			isChange |= isChangeEntity;
		}
		if (isChange) OnChanged(entitiesInsertProcessed, entitiesRemoveProcessed);
		return isChange;
	}

	/*!
	Register and unregister entities in one go.
	\param entityInsert Entity to be registered
	\param entityRemove Entity to be unregistered
	\param onlyDirect True to also include children of all affected entities
	\return True if cached array changed
	*/
	bool Set(SCR_EditableEntityComponent entityInsert, SCR_EditableEntityComponent entityRemove, bool onlyDirect = false)
	{
		if (entityInsert && entityRemove)
		{
			set<SCR_EditableEntityComponent> entitiesInsert = new set<SCR_EditableEntityComponent>;
			entitiesInsert.Insert(entityInsert);
			set<SCR_EditableEntityComponent> entitiesRemove = new set<SCR_EditableEntityComponent>;
			entitiesRemove.Insert(entityRemove);
			return Set(entitiesInsert, entitiesRemove, onlyDirect);
		}
		if (entityInsert)
		{
			return Add(entityInsert, onlyDirect);
		}
		else
		{
			return Remove(entityRemove, onlyDirect);
		}
	}

	/*!
	Register and unregister entities in one go.
	\param entitiesInsert Entities to be registered
	\param entitiesRemove Entities to be unregistered
	\param onlyDirect True to also include children of all affected entities
	\return True if cached array changed
	*/
	bool Set(set<SCR_EditableEntityComponent> entitiesInsert, set<SCR_EditableEntityComponent> entitiesRemove, bool onlyDirect = false)
	{
		if (!m_Entities) return false;

		//--- When one fhe arrays is empty, use specialized function instead
		bool entitiesInsertEmpty = !entitiesInsert || entitiesInsert.IsEmpty();
		bool entitiesRemoveEmpty = !entitiesRemove || entitiesRemove.IsEmpty();
		if (entitiesRemoveEmpty)
		{
			if (!entitiesInsertEmpty)
				return Add(entitiesInsert, onlyDirect);
			return false; //--- Both Insert and Remove arrays are empty, terminate
		}
		else
		{
			if (entitiesInsertEmpty)
				return Remove(entitiesRemove, onlyDirect);
		}

		//--- Add children entities
		if (!onlyDirect)
		{
			InsertChildren(entitiesInsert);
			InsertChildren(entitiesRemove);
		}

		bool isChange = false;
		set<SCR_EditableEntityComponent> entitiesInsertProcessed = new set<SCR_EditableEntityComponent>;
		set<SCR_EditableEntityComponent> entitiesRemoveProcessed = new set<SCR_EditableEntityComponent>;
		foreach (SCR_EditableEntityComponent entity : entitiesRemove)
		{
			//--- Don't remove what's to be added anyway
			int insertID = entitiesInsert.Find(entity);
			if (insertID >= 0)
			{
				entitiesInsert.Remove(insertID);
				continue;
			}

			if (RemoveFromArray(entity, isChange))
				entitiesRemoveProcessed.Insert(entity);
		}
		foreach (SCR_EditableEntityComponent entity : entitiesInsert)
		{
			if (AddToArray(entity, isChange))
				entitiesInsertProcessed.Insert(entity);
		}
		if (isChange) OnChanged(entitiesInsertProcessed, entitiesRemoveProcessed);
		return isChange;
	}
	/*!
	Fill the list from the predecessor.
	\return True if cached array changed
	*/
	bool SetFromPredecessor()
	{
		set<SCR_EditableEntityComponent> entities = new set<SCR_EditableEntityComponent>;
		if (m_Predecessor)
		{
			m_Predecessor.GetEntities(entities);
		}
		else if (GetCore())
		{
			GetCore().GetAllEntities(entities);
		}
		return Replace(entities, true);
	}
	/*!
	If the entity is added to the filter, check if its condition is still valid. If not, remove the entity.
	\param entity Queried entity
	\return True if the array changed
	*/
	bool Validate(SCR_EditableEntityComponent entity)
	{
		if (!m_Entities)
			return false;

		bool isChange = false;
		set<SCR_EditableEntityComponent> entitiesInsertProcessed = new set<SCR_EditableEntityComponent>;
		set<SCR_EditableEntityComponent> entitiesRemoveProcessed = new set<SCR_EditableEntityComponent>;

		if (AddToArray(entity, isChange, true))
		{
			if (isChange)
				entitiesInsertProcessed.Insert(entity);
		}
		else if (RemoveFromArray(entity, isChange))
		{
			entitiesRemoveProcessed.Insert(entity);
		}

		if (isChange) OnChanged(entitiesInsertProcessed, entitiesRemoveProcessed);
		return isChange;
	}
	/*!
	Clear cached array.
	\return True if the array changed
	*/
	bool Clear()
	{
		if (!m_Entities) return false;

		set<SCR_EditableEntityComponent> entitiesRemove = new set<SCR_EditableEntityComponent>;
		entitiesRemove.Copy(m_Entities);
		return Remove(entitiesRemove, true);
	}
	/*!
	Copy the list of registered entities from another filter.
	\param filter Filter from which the list of entities will be copied
	\param onlyDirect True to also include children of all affected entities
	\return True if cached array changed
	*/
	bool Copy(SCR_BaseEditableEntityFilter filter, bool onlyDirect = false)
	{
		if (!filter) return false;
		set<SCR_EditableEntityComponent> entities = new set<SCR_EditableEntityComponent>;
		filter.GetEntities(entities);
		return Replace(entities, onlyDirect);
	}
	///@}

	protected sealed void InsertChildren(out set<SCR_EditableEntityComponent> entities)
	{
		if (!entities || entities.IsEmpty()) return;

		//--- Copy the array first, so we don't modify it while processing it (set.Insert() doesn't add to the end)
		set<SCR_EditableEntityComponent> entitiesCopy = new set<SCR_EditableEntityComponent>;
		entitiesCopy.Copy(entities);

		for (int i = 0, entitiesCount = entitiesCopy.Count(); i < entitiesCount; i++)
		{
			entitiesCopy[i].GetChildren(entities, false, m_bSkipIgnored);
		}
	}

	protected sealed bool AddToArray(SCR_EditableEntityComponent entity, out bool isChange, bool returnWhenExists = false)
	{
		if (
			entity
			&& (m_PredecessorState == 0 || entity.HasEntityState(m_PredecessorState))
			&& (!m_bOnlyInteractive || !entity.HasEntityFlag(EEditableEntityFlag.NON_INTERACTIVE))
			&& CanAdd(entity)
		)
		{
			if (!entity.IsRegistered())
			{
				entity.Log(string.Format("Cannot set entity as %1, it's not registered!", GetStateName()), true, LogLevel.WARNING);
				return false;
			}

			//--- Add the entity. If it's already added, return straight away (but do not modify isChange)
			if (!m_Entities.Insert(entity))
				return returnWhenExists;

			m_iEntitiesCount++;
			entity.SetEntityState(m_State, true);
			isChange = true;

#ifdef ENTITY_FILTER_DEBUG
			if ((m_DebugState & m_State) == m_State)
				entity.Log(typename.EnumToString(EEditableEntityState, m_State), true);
#endif
			return true;
		}
		return false;
	}
	protected sealed bool RemoveFromArray(SCR_EditableEntityComponent entity, out bool isChange)
	{
		int index = m_Entities.Find(entity);
		if (index != -1)
		{
			m_Entities.Remove(index);
			m_iEntitiesCount--;
			entity.SetEntityState(m_State, false);
			isChange = true;

#ifdef ENTITY_FILTER_DEBUG
			if ((m_DebugState & m_State) == m_State)
				entity.Log(typename.EnumToString(EEditableEntityState, m_State), true, LogLevel.WARNING);
#endif
			return true;
		}
		return false;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Log
	/*!
	Print all cached entities to the log.
	*/
	void Log()
	{
		string ownerName;
		SCR_EditorModeEntity mode = SCR_EditorModeEntity.Cast(GetManager().GetOwner());
		if (mode)
			ownerName = typename.EnumToString(EEditorMode, mode.GetModeType()) + ": ";

		Print("--------------------------------------------------", LogLevel.DEBUG);
		Print(string.Format("--- %1%2 (%3)", ownerName, m_sStateName, GetEntitiesCount()), LogLevel.DEBUG);
		set<SCR_EditableEntityComponent> entities;
		GetEntitiesPointer(entities);
		foreach (SCR_EditableEntityComponent entity : entities)
		{
			if (entity) entity.Log("", true);
		}
		Print("--------------------------------------------------", LogLevel.DEBUG);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Functions called from entities manager
	void OnFrameBase(float timeSlice)
	{
		EOnEditorFrame(timeSlice);
	}
	
	void OnActivateBase()//out set<SCR_EditableEntityComponent> entities)
	{
		m_Core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		m_sStateName = Type().EnumToString(EEditableEntityState, m_State);

		EOnEditorActivate();
		InitEntities(); //--- Must be after EOnEditorActivate(), so the manager can initialize before entities are filled
	}
	
	void OnDeactivateBase()
	{
		//--- Previously used to call state change handlers on entities
		//if (m_Manager && !m_Manager.IsRemoved())
		//	Clear();

		m_Entities = null;
		m_iEntitiesCount = 0;
		EOnEditorDeactivate();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	/*! @name Override Functions
	Functions to be overridden by child classes (all of them local to the editor owner).
	*/
	///@{
	/*!
	Check if the entity meets condition for this entity state.
	\param entity Editable entity
	\return True if the condition is met
	*/
	bool CanAdd(SCR_EditableEntityComponent entity)
	{
		return true;
	}
	//! When the editor is opened (before cached array is intialized)
	protected void EOnEditorActivate();
	//! When the editor is closed (after cached array is destroyed)
	protected void EOnEditorDeactivate();
	//! Every frame while the editor is opened
	protected void EOnEditorFrame(float timeSlice);
	///@}
}
