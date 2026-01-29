[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditorActionGroup, "m_ActionGroup")]
class SCR_EditorActionGroup
{
	[Attribute(SCR_Enum.GetDefault(EEditorActionGroup.NONE), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EEditorActionGroup))]
	EEditorActionGroup m_ActionGroup;
	
	[Attribute()]
	int m_iOrder;
	
	ref SCR_SortedArray<SCR_BaseEditorAction> m_aActions;
}

[BaseContainerProps(configRoot: true)]
class SCR_EditorActionList
{
	[Attribute(desc: "Editor actions")]
	ref array<ref SCR_BaseEditorAction> m_Actions;
}

class SCR_BaseActionsEditorComponentClass : SCR_BaseEditorComponentClass
{
	[Attribute(desc: "Editor actions", category: "Editor Actions")]
	protected ref array<ref SCR_EditorActionList> m_ActionsLists;
	
	[Attribute(desc: "Editor action groups", category: "Editor Actions")]
	protected ref array<ref SCR_EditorActionGroup> m_ActionGroups;
	
	protected ref array<SCR_BaseEditorAction> m_ActionsSorted = {};
	
	//------------------------------------------------------------------------------------------------
	//! Get actions in this list.
	//! \param[out] outActions Array to be filled with actions
	//! \return Number of actions
	int GetActions(out notnull array<SCR_BaseEditorAction> outActions)
	{
		return outActions.Copy(m_ActionsSorted);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get action on given index.
	//! \param[in] index Desired index
	//! \return Action
	SCR_BaseEditorAction GetAction(int index)
	{
		if (!m_ActionsSorted.IsIndexValid(index))
			return null;

		return m_ActionsSorted[index];
	}

	//------------------------------------------------------------------------------------------------
	//! Get index of an action from the list.
	//! \param[in] action Queried action
	//! \return Index
	int FindAction(SCR_BaseEditorAction action)
	{
		return m_ActionsSorted.Find(action);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] manager
	//! \param[in] toAdd
	void SetShortcuts(SCR_BaseActionsEditorComponent manager, bool toAdd)
	{
		if (toAdd)
		{
			foreach (SCR_BaseEditorAction action : m_ActionsSorted)
			{
				action.AddShortcut(manager);
			}
		}
		else
		{
			foreach (SCR_BaseEditorAction action : m_ActionsSorted)
			{
				action.RemoveShortcut();
			}
		}
	}
	
	//---- REFACTOR NOTE START: Sorting might be done simpler and easir to read?
	
	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] prefab
	void SCR_BaseActionsEditorComponentClass(IEntityComponentSource componentSource, IEntitySource parentSource, IEntitySource prefabSource)
	{
		if (!m_ActionGroups || m_ActionGroups.IsEmpty())
		{
			//--- No action groups defined
			SCR_BaseEditorAction action;
			SCR_SortedArray<SCR_BaseEditorAction> actionsSorted = new SCR_SortedArray<SCR_BaseEditorAction>();
			foreach (SCR_EditorActionList list: m_ActionsLists)
			{
				for (int i = 0, count = list.m_Actions.Count(); i < count; i++)
				{
					action = list.m_Actions[i];
					if (action.IsEnabled())
						actionsSorted.Insert(action.GetOrder(), action);
				}
			}
			actionsSorted.ToArray(m_ActionsSorted);
		}
		else
		{
			//--- Action groups defined, use them for sorting
			SCR_EditorActionGroup actionGroup;
			SCR_SortedArray<SCR_EditorActionGroup> actionGroupsSorted = new SCR_SortedArray<SCR_EditorActionGroup>();
			map<EEditorActionGroup, SCR_EditorActionGroup> actionGroupsMap = new map<EEditorActionGroup, SCR_EditorActionGroup>();
			int actionGroupCount = m_ActionGroups.Count();
			for (int g = 0; g < actionGroupCount; g++)
			{
				actionGroup = m_ActionGroups[g];
				actionGroup.m_aActions = new SCR_SortedArray<SCR_BaseEditorAction>();
				actionGroupsSorted.Insert(actionGroup.m_iOrder, actionGroup);
				actionGroupsMap.Insert(actionGroup.m_ActionGroup, actionGroup);
			}
			
			//--- Put actions to their groups
			SCR_BaseEditorAction action;
			foreach (SCR_EditorActionList list: m_ActionsLists)
			{
				for (int a = 0, count = list.m_Actions.Count(); a < count; a++)
				{
					action = list.m_Actions[a];
					if (action.IsEnabled())
					{
						if (actionGroupsMap.Find(action.GetActionGroup(), actionGroup))
							actionGroup.m_aActions.Insert(action.GetOrder(), action);
						else
							Debug.Error2("SCR_BaseActionsEditorComponentClass.SCR_BaseActionsEditorComponentClass()", string.Format("Cannot register action %1, its group %2 is not configured!", Type(), typename.EnumToString(EEditorActionGroup, action.GetActionGroup())));
					}
				}
			}
			
			//--- Create array sorted by groups and order indexes
			for (int g; g < actionGroupCount; g++)
			{
				actionGroup = actionGroupsSorted[g];
				for (int a, actionCount = actionGroup.m_aActions.Count(); a < actionCount; a++)
				{
					m_ActionsSorted.Insert(actionGroup.m_aActions[a]);
				}
			}
		}
		m_ActionGroups = null;
	}
	
	//---- REFACTOR NOTE END ----
}

class SCR_BaseActionsEditorComponent : SCR_BaseEditorComponent
{
	protected SCR_BaseEditableEntityFilter m_HoverManager;
	protected SCR_BaseEditableEntityFilter m_SelectedManager;
	protected SCR_MenuLayoutEditorComponent m_MenuLayoutManager;
	
	protected SCR_EditableEntityComponent m_HoveredEntity;
	protected ref set<SCR_EditableEntityComponent> m_SelectedEntities = new set<SCR_EditableEntityComponent>();
	
	//------------------------------------------------------------------------------------------------
	//! \return Entity under cursor when context menu was opened
	SCR_EditableEntityComponent GetHoveredEntity()
	{
		return m_HoveredEntity;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets all actions on the component
	//! \param[out] actions output array containing all actions on this component
	//! \return Amount of total actions
	int GetActions(out notnull array<SCR_BaseEditorAction> actions)
	{
		SCR_BaseActionsEditorComponentClass prefabData = SCR_BaseActionsEditorComponentClass.Cast(GetEditorComponentData());
		if (prefabData)
			return prefabData.GetActions(actions);
		else
			return 0;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Evaluates passed list of actions for the current context
	//! \param[in] actions input array with all actions
	//! \param[in] cursorWorldPositioin world positon used to evaluate actions
	//! \param[out] filteredActions output array containing all available actions for the current context
	//! \param[out] flags Conitions to be cached for faster evaluation in functions
	//! \return Amount of available actions
	void EvaluateActions(notnull array<SCR_BaseEditorAction> actions, vector cursorWorldPosition, out notnull array<ref SCR_EditorActionData> filteredActions, out int flags = 0)
	{
		filteredActions.Clear();
		
		flags |= ValidateSelection(false);
		
		foreach (SCR_BaseEditorAction action : actions)
		{
			if (!ActionCanBeShown(action, cursorWorldPosition, flags))
				continue;
			
			bool canBePerformed = ActionCanBePerformed(action, cursorWorldPosition, flags);
			
			filteredActions.Insert(new SCR_EditorActionData(action, canBePerformed));
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets and evaluates actions for current context, combination of GetActions and EvaluateActions functions
	//! \param[in] cursorWorldPosition world positon used to evaluate actions
	//! \param[in] filteredActions output array containing all available actions for the current context
	//! \param[in] flags Cached results of common conditions
	//! \return Amount of available actions
	int GetAndEvaluateActions(vector cursorWorldPosition, out notnull array<ref SCR_EditorActionData> filteredActions, out int flags = 0)
	{
		array<SCR_BaseEditorAction> actions = {};
		GetActions(actions);
		EvaluateActions(actions, cursorWorldPosition, filteredActions, flags);
		return filteredActions.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Evaluate which entities are under cursor and which are selected.
	//! To be overridden by inherited classes.
	//! \param[in] isInstant True if an action is performed right after this evaluation
	//! \return Condition flags passed to actions
	protected int ValidateSelection(bool isInstant)
	{
		return 0;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] action
	//! \param[in] cursorWorldPosition
	//! \param[in] flags
	//! \return
	bool ActionCanBeShown(SCR_BaseEditorAction action, vector cursorWorldPosition, int flags)
	{
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] action
	//! \param[in] cursorWorldPosition
	//! \param[in] flags
	//! \return
	bool ActionCanBePerformed(SCR_BaseEditorAction action, vector cursorWorldPosition, int flags)
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! When action shortcut is activated, check if the action can be performed and if so, perform it.
	//! \param[in] action Action to be performed
	//! \param[in] flags
	void ActionPerformInstantly(SCR_BaseEditorAction action, int flags = 0)
	{
		flags |= ValidateSelection(true);
		
		vector cursorWorldPosition;
		if (m_MenuLayoutManager)
			m_MenuLayoutManager.GetCursorWorldPos(cursorWorldPosition, GetInstantActionTraceFlags()); //--- ToDo: Handle in UI scripts?

		if (ActionCanBePerformed(action, cursorWorldPosition, flags))
			ActionPerformInstantlyNoDialog(action, cursorWorldPosition, flags);
	}

	//------------------------------------------------------------------------------------------------
	protected void ActionPerformInstantlyNoDialog(SCR_BaseEditorAction action, vector cursorWorldPosition, int flags = 0)
	{
		//--- Editor closed while waiting, terminate the loop
		if (!SCR_EditorManagerEntity.IsOpenedInstance())
			return;
		
		//--- Keep waiting until no dialog is open
		if (GetGame().GetMenuManager().IsAnyDialogOpen())
			GetGame().GetCallqueue().CallLater(ActionPerformInstantlyNoDialog, 1, false, action, cursorWorldPosition, flags);
		else
			ActionPerform(action, cursorWorldPosition, flags);
	}

	//------------------------------------------------------------------------------------------------
	protected TraceFlags GetInstantActionTraceFlags()
	{
		return -1;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Perform given action
	//! \param[in] action
	//! \param[in] cursorWorldPosition
	//! \param[in] flags
	void ActionPerform(SCR_BaseEditorAction action, vector cursorWorldPosition, int flags)
	{
		int param = action.GetParam();
		
		//~ Check if on cooldown and set cooldown if any. Always checked locally
		if (action.CheckAndSetCooldown())
			return;
		
		// If action requires execution on server and role is proxy/client, send action to server
		if (action.IsServer() && m_RplComponent.Role() == RplRole.Proxy)
		{
			ActionPerformRpc(action, cursorWorldPosition, flags, param);
		}
		else
		{
			ActionPerform(action, m_HoveredEntity, m_SelectedEntities, cursorWorldPosition, flags, param);
			ActionPerformLocal(action, m_HoveredEntity, m_SelectedEntities, cursorWorldPosition, flags, param);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ActionPerformRpc(SCR_BaseEditorAction action, vector cursorWorldPosition, int flags, int param = -1)
	{
		SCR_BaseActionsEditorComponentClass prefabData = SCR_BaseActionsEditorComponentClass.Cast(GetEditorComponentData());
		if (!prefabData)
			return;
		
		int actionIndex = prefabData.FindAction(action);
		if (actionIndex == -1)
			return;
			
		RplId hoveredEntityID = -1;
		array<RplId> selectedEntityIds = {};
		SerializeEntities(hoveredEntityID, selectedEntityIds);
		
		Rpc(ActionPerformServer, actionIndex, hoveredEntityID, selectedEntityIds, cursorWorldPosition, flags, param);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ActionPerform(SCR_BaseEditorAction action, SCR_EditableEntityComponent hoveredEntityComponent, set<SCR_EditableEntityComponent> selectedEntityComponents, vector cursorWorldPosition, int flags, int param)
	{
		action.Perform(hoveredEntityComponent, selectedEntityComponents, cursorWorldPosition, flags, param);
	}

	//------------------------------------------------------------------------------------------------
	protected void ActionPerformLocal(SCR_BaseEditorAction action, SCR_EditableEntityComponent hoveredEntityComponent, set<SCR_EditableEntityComponent> selectedEntityComponents, vector cursorWorldPosition, int flags, int param)
	{
		action.PerformOwner(m_HoveredEntity, m_SelectedEntities, cursorWorldPosition, flags, param);
		
		SCR_BaseEditorEffect.Activate(action.GetEffects(), this, cursorWorldPosition, selectedEntityComponents);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Perform given action on server, exclusively called by ActionPerform
	//! \param[in] actionIndex Index of the action in the m_Actions list, same on server and client
	//! \param[in] hoveredEntityID RplID of the hovered entity, -1 if none hovered
	//! \param[in] selectedEntityIds RplIDs of the selected entities, empty array if none selected
	//! \param[in] cursorWorldPosition world position where action was intitiated
	//! \param[in] flags EEditorContextActionFlags / EEditorCommandActionFlags for placing
	//! \param[in] param parameter obtained through SCR_BaseEditorAction.GetParam() on client, use to pass client data/ids to server
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void ActionPerformServer(int actionIndex, RplId hoveredEntityID, array<RplId> selectedEntityIds, vector cursorWorldPosition, int flags, int param)
	{
		SCR_BaseActionsEditorComponentClass prefabData = SCR_BaseActionsEditorComponentClass.Cast(GetEditorComponentData());
		if (!prefabData)
			return;
		
		SCR_BaseEditorAction action = prefabData.GetAction(actionIndex);
		if (!action)
		{
			Print("Action with index: " + actionIndex + " not found on server", LogLevel.ERROR);
			return;
		}
		
		SCR_EditableEntityComponent hoveredEntityComponent;
		set<SCR_EditableEntityComponent> selectedEntityComponents = new set<SCR_EditableEntityComponent>;
		DeSerializeEntities(hoveredEntityID, selectedEntityIds, hoveredEntityComponent, selectedEntityComponents);
		
		ActionPerform(action, hoveredEntityComponent, selectedEntityComponents, cursorWorldPosition, flags, param);
		
		Rpc(ActionPerformOwner, actionIndex, cursorWorldPosition, flags, param);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Server callback for when action is executed succesfully, calls PerformOwner function on action for e.g. local effects
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void ActionPerformOwner(int actionIndex, vector cursorWorldPosition, int flags, int param)
	{
		SCR_BaseActionsEditorComponentClass prefabData = SCR_BaseActionsEditorComponentClass.Cast(GetEditorComponentData());
		if (!prefabData)
			return;
		
		SCR_BaseEditorAction action = prefabData.GetAction(actionIndex);
		if (!action)
		{
			Print("Action with index: " + actionIndex + " returned from server, not found on client", LogLevel.ERROR);
			return;
		}
		ActionPerformLocal(action, m_HoveredEntity, m_SelectedEntities, cursorWorldPosition, flags, param);
	}
	
	//------------------------------------------------------------------------------------------------
	private void SerializeEntities(out RplId hoveredEntityId, out array<RplId> selectedEntityIds)
	{
		// Serialize hovered entity to RplId int
		if (m_HoveredEntity)
		{
			if (!GetRplIdFromEditableEntity(m_HoveredEntity, hoveredEntityId))
			{
				Print("Entity " + m_HoveredEntity.GetDisplayName() + " does not have replication component or is not registered for replication", LogLevel.WARNING);
			}
		}
		
		// Serialize selected entities to RplId int array
		foreach (SCR_EditableEntityComponent selectedEntity : m_SelectedEntities)
		{
			if (!selectedEntity)
			{
				continue;
			}
			RplId selectedEntityId;
			if (!GetRplIdFromEditableEntity(selectedEntity, selectedEntityId))
			{
				Print("Entity " + selectedEntity.GetDisplayName() + " does not have replication component or is not registered for replication", LogLevel.WARNING);
				continue;
			}
			selectedEntityIds.Insert(selectedEntityId);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	private bool GetRplIdFromEditableEntity(SCR_EditableEntityComponent entity, out RplId entityRplId)
	{
		entityRplId = Replication.FindId(entity);
		return entityRplId != -1;
	}
	
	//------------------------------------------------------------------------------------------------
	private void DeSerializeEntities(RplId hoveredEntityId, array<RplId> selectedEntityIds, out SCR_EditableEntityComponent hoveredEntityComponent, out set<SCR_EditableEntityComponent> selectedEntityComponents)
	{
		// Deserialize hovered entity RplId
		if (hoveredEntityId != -1)
		{
			if (!GetEditableEntityFromRplId(hoveredEntityId, hoveredEntityComponent))
				Print(string.Format("Hovered entity with RplID: %1 not found on server", hoveredEntityId), LogLevel.NORMAL);
		}
		
		// Deserialize selected entity RplIds
		foreach (RplId selectedEntityRplId : selectedEntityIds)
		{
			SCR_EditableEntityComponent selectedEntityComponent;
			if (!GetEditableEntityFromRplId(selectedEntityRplId, selectedEntityComponent))
			{
				Print(string.Format("Selected entity with RplID: %1 not found on server", selectedEntityRplId), LogLevel.NORMAL	);
				continue;
			}
			selectedEntityComponents.Insert(selectedEntityComponent);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	private bool GetEditableEntityFromRplId(RplId entityRplId, out SCR_EditableEntityComponent editableEntityComponent)
	{
		Managed entityComponent = Replication.FindItem(entityRplId);
		if (!entityComponent)
		{
			Print(string.Format("Entity with RplID: %1 not found on server", entityRplId), LogLevel.NORMAL);
			return false;
		}
		
		editableEntityComponent = SCR_EditableEntityComponent.Cast(entityComponent);
		return editableEntityComponent != null;
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnEditorActivate()
	{
		SCR_BaseActionsEditorComponentClass prefabData = SCR_BaseActionsEditorComponentClass.Cast(GetEditorComponentData());
		if (!prefabData)
			return;
		
		SCR_EntitiesManagerEditorComponent entitiesManager = SCR_EntitiesManagerEditorComponent.Cast(SCR_EntitiesManagerEditorComponent.GetInstance(SCR_EntitiesManagerEditorComponent));
		if (entitiesManager)
		{
			m_HoverManager = entitiesManager.GetFilter(EEditableEntityState.HOVER);
			m_SelectedManager = entitiesManager.GetFilter(EEditableEntityState.SELECTED);
		}
		
		m_MenuLayoutManager = SCR_MenuLayoutEditorComponent.Cast(SCR_MenuLayoutEditorComponent.GetInstance(SCR_MenuLayoutEditorComponent));
		
		prefabData.SetShortcuts(this, true);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnEditorDeactivate()
	{
		SCR_BaseActionsEditorComponentClass prefabData = SCR_BaseActionsEditorComponentClass.Cast(GetEditorComponentData());
		if (!prefabData)
			return;
		
		prefabData.SetShortcuts(this, false);
	}
}
