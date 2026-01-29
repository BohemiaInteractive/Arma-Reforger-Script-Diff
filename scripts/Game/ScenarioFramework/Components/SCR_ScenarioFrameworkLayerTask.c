[EntityEditorProps(category: "GameScripted/ScenarioFramework/Layer", description: "")]
class SCR_ScenarioFrameworkLayerTaskClass : SCR_ScenarioFrameworkLayerBaseClass
{
}

class SCR_ScenarioFrameworkLayerTask : SCR_ScenarioFrameworkLayerBase
{
	[Attribute(desc: "Name of the task in list of tasks", category: "Task")]
	string m_sTaskTitle;

	[Attribute(desc: "Description of the task", category: "Task", )]
	string m_sTaskDescription;
	
	[Attribute(desc: "Type of task used for ON_TASKS_INIT generation", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_ESFTaskType), category: "Task Enums")];
	SCR_ESFTaskType m_eTypeOfTask;
	
	[Attribute(defvalue: SCR_ETaskOwnership.FACTION.ToString(), desc: "Who will be the owner of the task for whom it will be assignable. In Default, it will be owned by the given Faction.", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_ETaskOwnership), category: "Task Enums")];
	SCR_ETaskOwnership m_eTaskOwnership;
	
	[Attribute(defvalue: SCR_ETaskVisibility.FACTION.ToString(), desc: "To whom will the task be visible for. In Default, it will be visible for given Faction.", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_ETaskVisibility), category: "Task Enums")];
	SCR_ETaskVisibility m_eTaskVisibility;
	
	[Attribute(defvalue: SCR_ETaskUIVisibility.ALL.ToString(), desc: "Where will the task be visible in UI. In default, it will be visible in the Task List and on the Map.", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_ETaskUIVisibility), category: "Task Enums")];
	SCR_ETaskUIVisibility m_eTaskUIVisibility;

	[Attribute(params: "et", desc: "Task prefab", category: "Task")]
	ResourceName m_sTaskPrefab;
	
	[Attribute(desc: "Overrides display name of the spawned object for task purposes", category: "Task")]
	string m_sOverrideObjectDisplayName;
	
	[Attribute(params: "edds imageset", uiwidget: UIWidgets.ResourcePickerThumbnail, desc: "Task icon set", category: "Task UI")]
	ResourceName m_sTaskIconSet;
	
	[Attribute(desc: "Name of the specific icon from the icon set", category: "Task UI")]
	string m_sTaskIconName;
	
	[Attribute(defvalue: "1", desc: "Whether task has functionality on its own or it is just holder for parent task without actuall functionality", category: "Task")]
	bool m_bTaskFunctionsOnItsOwn;
	
	[Attribute(desc: "Conditions that will be checked upon trying to finish a task", category: "Task")]
	ref array<ref SCR_ScenarioFrameworkActivationConditionBase> m_aFinishConditions;
	
	[Attribute(defvalue: SCR_EScenarioFrameworkLogicOperators.AND.ToString(), UIWidgets.ComboBox, "Which Boolean Logic will be used for Finish Conditions.", "", enums: SCR_EScenarioFrameworkLogicOperatorHelper.GetParamInfo(), category: "Task")]
	SCR_EScenarioFrameworkLogicOperators m_eFinishConditionLogic;
	
	[Attribute(defvalue: "0", desc: "Whether or not to show progress bar", category: "Task UI")]
	bool m_bProgressBar;
	
	[Attribute(defvalue: "1", desc: "It will set percentage of progress based on the number of completed tasks that are not optional", category: "Task UI")]
	bool m_bCalculateProgressBarOnCompletedTasks;
	
	[Attribute(desc: "Marker on map is placed directly on the task subject Slot or on layer Slot", category: "Task UI")]
	bool m_bPlaceMarkerOnSubjectSlot;
	
	[Attribute(desc: "Attach marker to entity", category: "Task UI")]
	ref SCR_ScenarioFrameworkGet m_EntityToAttachGetter;
	
	[Attribute(defvalue: "0", desc: "Marker Update Interval. If set to 0, marker position won't be automatically updated", params: "0 inf 0.01", category: "Task UI")]
	protected float m_fMarkerUpdateInterval;
	
	[Attribute(defvalue: SCR_Enum.GetFlagValues(SCR_ETaskNotificationSettings).ToString(), desc: "Task Notification Settings", uiwidget: UIWidgets.Flags, enumType: SCR_ETaskNotificationSettings, category: "Task Notifications")]
	SCR_ETaskNotificationSettings m_eTaskNotificationSettings;

	[Attribute(defvalue: "-1", desc: "Number Of Requiered Tasks To Finish this task if it has subtasks. Optional subtasks are excluded. -1 for currently attached subtasks (It can change in runtime)", params: "-1 inf", category: "Subtask")]
	int m_iNumberOfRequieredTasksToFinish;
	
	[Attribute(desc: "Description that is displayed above subtasks in the parent task", category: "Subtask", )]
	string m_sSubtasksDescription;
	
	[Attribute(defvalue: "0", desc: "If it is a subtask it will be grouped under the top most LayerTask or if attribute target parent task is filled, it will group it under that", category: "Subtask")]
	bool m_bIsSubtask;
	
	[Attribute(defvalue: "0", desc: "If this is subtask, whether or not the subtask is optional or not.", category: "Subtask")]
	bool m_bIsOptional;
	
	[Attribute(desc: "Name of the parent LayerTask if this LayerTask is set to be a Subtask. It will override default behaviour, where parent LayerTask is the top most LayerTask in hierarchy.", category: "Subtask", )]
	string m_sParentLayerTask;

	[Attribute(desc: "What to do once task is finished", category: "Task State Changed Actions")]
	ref array<ref SCR_ScenarioFrameworkActionBase> m_aTriggerActionsOnFinish;
	
	[Attribute(desc: "What to do once task is created", category: "Task State Changed Actions")]
	ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActionsOnCreated;

	[Attribute(desc: "What to do once task is created", category: "Task State Changed Actions")]
	ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActionsOnFailed;
	
	[Attribute(desc: "What to do once task is cancelled", category: "Task State Changed Actions")]
	ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActionsOnCancelled;

	[Attribute(desc: "What to do once task progressed", category: "Task State Changed Actions")]
	ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActionsOnProgress;

	[Attribute(desc: "What to do once task is updated", category: "Task State Changed Actions")]
	ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActionsOnAssigned;

	SCR_ScenarioFrameworkLayerTask m_TopParentLayerTask;
	SCR_ScenarioFrameworkSlotTask m_SlotTask;
	SCR_ScenarioFrameworkTask m_Task;
	SCR_ETaskState m_eLayerTaskState = SCR_ETaskState.CREATED;
	bool m_bTaskResolvedBeforeLoad;
	bool m_bTaskCompletedWithoutSubtasks;
	SCR_TaskSystem m_TaskSystem;

	static const ref ScriptInvoker s_OnTaskSetup = new ScriptInvoker();

	//------------------------------------------------------------------------------------------------
	//! \return Layer task state in current task layer.
	SCR_ETaskState GetLayerTaskState()
	{
		return m_eLayerTaskState;
	}

	//------------------------------------------------------------------------------------------------
	//! \return Indicates if task was resolved before loading.
	bool GetLayerTaskResolvedBeforeLoad()
	{
		return m_bTaskResolvedBeforeLoad;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] state Sets layer task state
	void SetLayerTaskState(SCR_ETaskState state)
	{
		m_eLayerTaskState = state;
	}

	//------------------------------------------------------------------------------------------------
	//! \return the current task.
	SCR_ScenarioFrameworkTask GetTask()
	{
		return m_Task;
	}

	//------------------------------------------------------------------------------------------------
	//! \return The return value represents the prefab for the task in the method.
	ResourceName GetTaskPrefab()
	{
		return m_sTaskPrefab;
	}

	//------------------------------------------------------------------------------------------------
	//! \return the display name for an overridden object.
	string GetOverridenObjectDisplayName()
	{
		return m_sOverrideObjectDisplayName;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] name Sets object display name override.
	void SetOverridenObjectDisplayName(string name)
	{
		m_sOverrideObjectDisplayName = name;
	}

	//------------------------------------------------------------------------------------------------
	//! \return The return value represents the title of the task.
	string GetTaskTitle()
	{
		return m_sTaskTitle;
	}

	//------------------------------------------------------------------------------------------------
	//! \return Original task description string returned by the method.
	string GetOriginalTaskDescription()
	{
		return m_sTaskDescription;
	}

	//------------------------------------------------------------------------------------------------
	//! \return the task description, or sets it if an override object display name is provided.
	string GetTaskDescription()
	{
		if (!m_sOverrideObjectDisplayName.IsEmpty() && m_SlotTask && m_SlotTask.GetOverriddenObjectDisplayName().IsEmpty())
			m_SlotTask.SetOverriddenObjectDisplayName(m_sOverrideObjectDisplayName);

		return m_sTaskDescription;
	}

	//------------------------------------------------------------------------------------------------
	//! \return Task type enum representing the type of task in the mission.
	SCR_ESFTaskType GetTaskType()
	{
		return m_eTypeOfTask;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return topmost parent ScenarioFrameworkLayerTask.
	SCR_ScenarioFrameworkLayerTask GetTopMostParentTaskLayer()
	{
	    SCR_ScenarioFrameworkLayerTask topMostLayer;
	    IEntity entity = GetOwner().GetParent();
	    while (entity)
	    {
	        SCR_ScenarioFrameworkLayerTask layer = SCR_ScenarioFrameworkLayerTask.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerTask));
	        if (layer)
	            topMostLayer = layer; // Keep updating with the last found layer
	
	        entity = entity.GetParent();
	    }
		
	    return topMostLayer;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Restores default settings, clears children, removes spawned entities, optionally reinitializes after restoration.
	//! \param[in] includeChildren Restores default settings for this entity and its children if includeChildren is true.
	//! \param[in] reinitAfterRestoration Restores entity to default state, optionally reinitializes after restoration.
	//! \param[in] affectRandomization determines whether to clear all randomly spawned children entities after restoring default settings.
	override void RestoreToDefault(bool includeChildren = false, bool reinitAfterRestoration = false, bool affectRandomization = true)
	{
		foreach (SCR_ScenarioFrameworkActionBase activationAction : m_aTriggerActionsOnFinish)
		{
			activationAction.RestoreToDefault();
		}
		
		foreach (SCR_ScenarioFrameworkActionBase activationAction : m_aActionsOnCreated)
		{
			activationAction.RestoreToDefault();
		}
		
		foreach (SCR_ScenarioFrameworkActionBase activationAction : m_aActionsOnFailed)
		{
			activationAction.RestoreToDefault();
		}
		
		foreach (SCR_ScenarioFrameworkActionBase activationAction : m_aActionsOnCancelled)
		{
			activationAction.RestoreToDefault();
		}
		
		foreach (SCR_ScenarioFrameworkActionBase activationAction : m_aActionsOnProgress)
		{
			activationAction.RestoreToDefault();
		}
		
		foreach (SCR_ScenarioFrameworkActionBase activationAction : m_aActionsOnAssigned)
		{
			activationAction.RestoreToDefault();
		}
		
		m_SlotTask = null;
		
		if (m_TaskSystem)
			ProcessLayerTaskState(SCR_ETaskState.CANCELLED);
		
		m_Task = null;
		SetLayerTaskState(SCR_ETaskState.CREATED);
		m_bTaskResolvedBeforeLoad = false;
		
		super.RestoreToDefault(includeChildren, reinitAfterRestoration, affectRandomization);
	}

	//------------------------------------------------------------------------------------------------
	//! Initializes layer with same activation type as parent.
	override void DynamicReinit()
	{
		Init(null, SCR_ScenarioFrameworkEActivationType.SAME_AS_PARENT);
	}

	//------------------------------------------------------------------------------------------------
	//! Dynamically despawns this layer.
	//! \param[in] layer for which this is called.
	override void DynamicDespawn(SCR_ScenarioFrameworkLayerBase layer)
	{
		GetOnAllChildrenSpawned().Remove(DynamicDespawn);
		if (!m_bInitiated)
		{
			GetOnAllChildrenSpawned().Insert(DynamicDespawn);
			return;
		}

		if (!m_bInitiated || m_bExcludeFromDynamicDespawn)
			return;

		foreach (SCR_ScenarioFrameworkActivationConditionBase activationCondition : m_aActivationConditions)
		{
			//If just one condition is false, we don't continue and interrupt the init
			if (!activationCondition.Init(GetOwner()))
			{
				if (m_ParentLayer)
					m_ParentLayer.CheckAllChildrenSpawned(this);

				return;
			}
		}

		m_bInitiated = false;
		m_bDynamicallyDespawned = true;
		m_aChildren.RemoveItem(null);
		foreach (SCR_ScenarioFrameworkLayerBase child : m_aChildren)
		{
			child.DynamicDespawn(this);
		}

		foreach (IEntity entity : m_aSpawnedEntities)
		{
			SCR_EntityHelper.DeleteEntityAndChildren(entity);
		}

		m_SlotTask = null;
		m_aSpawnedEntities.Clear();

		GetOnAllChildrenSpawned().Remove(InitTask);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Initializes tasks after all children spawned.
	override void FinishInit()
	{
		GetOnAllChildrenSpawned().Insert(InitTask);
		
		super.FinishInit();
	}

	//------------------------------------------------------------------------------------------------
	//! Triggers actions on task finish if not resolved before load.
	//! \param[in] previousState represents the current state of the task before it transitions to new state.
	//! \param[in] newState Triggers actions on task finish state change.
	void OnTaskStateChanged(SCR_ETaskState previousState, SCR_ETaskState newState)
	{
		if (newState == SCR_ETaskState.CREATED)
		{
			foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnCreated)
			{
				action.OnActivate(GetOwner());
			}
		}
		else if (newState == SCR_ETaskState.COMPLETED && !m_bTaskResolvedBeforeLoad)
		{
			foreach (SCR_ScenarioFrameworkActionBase action : m_aTriggerActionsOnFinish)
			{
				action.OnActivate(GetOwner());
			}
		}
		else if (newState == SCR_ETaskState.FAILED && !m_bTaskResolvedBeforeLoad)
		{
			foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnFailed)
			{
				action.OnActivate(GetOwner());
			}
		}
		else if (newState == SCR_ETaskState.CANCELLED && !m_bTaskResolvedBeforeLoad)
		{
			foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnCancelled)
			{
				action.OnActivate(GetOwner());
			}
		}
		else if (newState == SCR_ETaskState.PROGRESSED && !m_bTaskResolvedBeforeLoad)
		{
			foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnProgress)
			{
				action.OnActivate(GetOwner());
			}
		}
		else
		{
			foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnAssigned)
			{
				action.OnActivate(GetOwner());
			};
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] slotTask related to this layer task.
	void SetSlotTask(SCR_ScenarioFrameworkSlotTask slotTask)
	{
		m_SlotTask = slotTask;
		if (m_Task)
			m_Task.SetSlotTask(slotTask);
	}

	//------------------------------------------------------------------------------------------------
	//! \return related to this layer task.
	SCR_ScenarioFrameworkSlotTask GetSlotTask()
	{
		return m_SlotTask;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Processes the state of a task and its subtasks, updating the task's state and progress bar if necessary
	//! \param[in] state Processes the state of a task and its subtasks, updating the task's state and progress bar if necessary
	//! \param[in] forced parameter represents a boolean value that determines whether to process the task state even if the conditions for task completion are not met
	//! \param[in] calledFromSubtask parameter represents whether the method is being called from a subtask or not. If true, it
	void ProcessLayerTaskState(SCR_ETaskState state, bool forced = false, bool calledFromSubtask = false)
	{
		if (!m_SlotTask && m_Task)
			m_SlotTask = m_Task.GetSlotTask();
		
		if (!m_TaskSystem || !m_Task || !m_SlotTask || m_eLayerTaskState == SCR_ETaskState.CANCELLED || m_eLayerTaskState == SCR_ETaskState.FAILED)
			return;
		
		if (state == SCR_ETaskState.COMPLETED || !m_bTaskCompletedWithoutSubtasks)
		{
			if (m_SlotTask.m_eFinishConditionLogic == SCR_EScenarioFrameworkLogicOperators.AND)
			{
				if (!m_SlotTask.m_aFinishConditions || m_SlotTask.m_aFinishConditions.IsEmpty())
				{
					if (!forced && !SCR_ScenarioFrameworkActivationConditionBase.EvaluateEmptyOrConditions(m_eFinishConditionLogic, m_aFinishConditions, m_SlotTask.GetOwner()))
						return;
				}
				else
				{
					if (!forced && !SCR_ScenarioFrameworkActivationConditionBase.EvaluateEmptyOrConditions(m_eFinishConditionLogic, m_SlotTask.m_aFinishConditions, m_SlotTask.GetOwner()))
						return;
				}
			}
			else
			{
				if (!m_SlotTask.m_aFinishConditions || m_SlotTask.m_aFinishConditions.IsEmpty())
				{
					if (!forced && !SCR_ScenarioFrameworkActivationConditionBase.EvaluateEmptyOrConditions(m_SlotTask.m_eFinishConditionLogic, m_aFinishConditions, m_SlotTask.GetOwner()))
						return;
				}
				else
				{
					if (!forced && !SCR_ScenarioFrameworkActivationConditionBase.EvaluateEmptyOrConditions(m_SlotTask.m_eFinishConditionLogic, m_SlotTask.m_aFinishConditions, m_SlotTask.GetOwner()))
						return;
				}
			}
		}
		
		int allRequiredTasks;
		int finishedTasks;
		
		if (!m_bIsSubtask)
		{
			if (state == SCR_ETaskState.COMPLETED)
			{
				array<SCR_Task> subtasks = m_Task.GetChildTasks();
				array<SCR_ScenarioFrameworkTask> optional_subtasks = {};
				if (subtasks && subtasks.Count() > 0)
				{
					SCR_ScenarioFrameworkTask frameworkTask;
					foreach (SCR_Task subtask : subtasks)
					{
						frameworkTask = SCR_ScenarioFrameworkTask.Cast(subtask);
						if (!frameworkTask)
							continue;
						
						if (!frameworkTask.GetLayerTask())
							continue;
						
						if (frameworkTask.GetLayerTask().m_bIsOptional)
						{
							if (subtask.GetTaskState() != SCR_ETaskState.COMPLETED)
								optional_subtasks.Insert(SCR_ScenarioFrameworkTask.Cast(subtask));
							
							continue;
						}
						
						allRequiredTasks++;
						if (subtask.GetTaskState() == SCR_ETaskState.COMPLETED)
							finishedTasks++;	
					}
					
					if (m_iNumberOfRequieredTasksToFinish == -1)
					{
						if (finishedTasks != allRequiredTasks)
						{
							m_bTaskCompletedWithoutSubtasks = true;
							return;
						}
					}
					else
					{
						if (finishedTasks != m_iNumberOfRequieredTasksToFinish)
						{
							m_bTaskCompletedWithoutSubtasks = true;
							return;
						}
					}
					
					if (calledFromSubtask)
					{
						if (m_bTaskFunctionsOnItsOwn && !m_bTaskCompletedWithoutSubtasks)
							return;
					}
					else
					{
						if (!m_bTaskFunctionsOnItsOwn)
							return;
					}
					
					foreach (SCR_ScenarioFrameworkTask frameworkSubtask : optional_subtasks)
					{
						if (!frameworkSubtask.GetLayerTask())
							continue;
						
						frameworkSubtask.GetLayerTask().ProcessLayerTaskState(SCR_ETaskState.CANCELLED, false, true);
					}
				}
			}
		}
		else
		{
			SetLayerTaskState(state);
			m_TaskSystem.SetTaskState(m_Task, state);
			
			if (state == SCR_ETaskState.COMPLETED)
			{
				SCR_ScenarioFrameworkTask frameworkTask = SCR_ScenarioFrameworkTask.Cast(m_Task.GetParentTask());
				if (!frameworkTask)
					return;
				
				frameworkTask.GetLayerTask().ProcessSubTasks();
			}
			return;
		}
		
		if (m_bProgressBar && m_bCalculateProgressBarOnCompletedTasks && allRequiredTasks != 0)
			m_Task.SetTaskProgress(100 * (finishedTasks/allRequiredTasks), false);
		
		SetLayerTaskState(state);
		m_TaskSystem.SetTaskState(m_Task, state);
	}
	
	//------------------------------------------------------------------------------------------------
	//! This method processes subtasks of a main task, calculates progress bar based on completed subtasks, and updates the layer
	void ProcessSubTasks()
	{
		array<SCR_Task> subtasks = m_Task.GetChildTasks();
		if (subtasks && subtasks.Count() > 0)
		{
			SCR_ScenarioFrameworkTask frameworkTask;
			int allRequiredTasks;
			int finishedTasks;
			foreach (SCR_Task subtask : subtasks)
			{
				frameworkTask = SCR_ScenarioFrameworkTask.Cast(subtask);
				if (!frameworkTask)
					continue;
				
				if (!frameworkTask.GetLayerTask() || frameworkTask.GetLayerTask().m_bIsOptional)
					continue;
				
				allRequiredTasks++;
				if (subtask.GetTaskState() == SCR_ETaskState.COMPLETED)
					finishedTasks++;	
			}
			
			if (m_bProgressBar && m_bCalculateProgressBarOnCompletedTasks && allRequiredTasks != 0)
				m_Task.SetTaskProgress(100 * (finishedTasks/allRequiredTasks), false);
			
			if (m_iNumberOfRequieredTasksToFinish == -1)
			{
				if (finishedTasks == allRequiredTasks)
				{
					ProcessLayerTaskState(SCR_ETaskState.COMPLETED, false, true);
				}
			}
			else
			{
				if (finishedTasks == m_iNumberOfRequieredTasksToFinish)
				{
					ProcessLayerTaskState(SCR_ETaskState.COMPLETED, false, true);
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Initializes task, sets up support entity, and handles task state changes based on scenario framework layer.
	//! \param[in] layer for which this task is to be initialized
	void InitTask(SCR_ScenarioFrameworkLayerBase layer)
	{
		GetOnAllChildrenSpawned().Remove(InitTask);
		
		m_TaskSystem = SCR_TaskSystem.GetInstance();
	    if (!m_TaskSystem)
	        return;
		
		//If m_Task already exists, we are reiniting it, not creating new one. Will already be there for a save-game load.
		if (m_Task)
		{
			// We need to make sure that we have m_SlotTask to work with
			if (!m_SlotTask)
				m_SlotTask = m_Task.GetSlotTask();

			if (!m_SlotTask)
			{
				if (!m_aRandomlySpawnedChildren.IsEmpty())
					m_SlotTask = GetSlotTask(m_aRandomlySpawnedChildren);
				else
					m_SlotTask = GetSlotTask(m_aChildren);
			}

			if (!m_SlotTask)
			{
				Print(string.Format("ScenarioFramework: %1 could not reinit task due to missing m_SlotTask!", GetOwner().GetName()), LogLevel.ERROR);
				return;
			}

			SetLayerTaskState(m_Task.GetTaskState());

			if (m_SlotTask.GetIsTerminated() || m_bIsTerminated)
			{
				if (m_eLayerTaskState >= SCR_ETaskState.COMPLETED)
				{
					m_bTaskResolvedBeforeLoad = true;
					m_SlotTask.SetTaskResolvedBeforeLoad(true);
					ProcessLayerTaskState(m_eLayerTaskState);
				}
			}

			m_Task.HookTaskAsset(m_Entity);
			return;
		}

		if (!m_SlotTask)
		{
			if (!m_aRandomlySpawnedChildren.IsEmpty())
				m_SlotTask = GetSlotTask(m_aRandomlySpawnedChildren);
			else
				m_SlotTask = GetSlotTask(m_aChildren);
			
			if (!m_SlotTask)
			{
				Print(string.Format("ScenarioFramework: %1 could not init task due to missing m_SlotTask!", GetOwner().GetName()), LogLevel.ERROR);
				return;
			}
		}

		if (CreateTask(GetMarkerPosition()))
		{
			SetupTask();
			m_Task.SetLayerTask(this);
			m_SlotTask.OnTaskStateChanged(m_eLayerTaskState);

			if (m_eLayerTaskState >= SCR_ETaskState.COMPLETED)
			{
				m_bTaskResolvedBeforeLoad = true;
				m_SlotTask.SetTaskResolvedBeforeLoad(true);
			}

			if (m_eLayerTaskState != SCR_ETaskState.CREATED)
				ProcessLayerTaskState(m_eLayerTaskState);

			if (m_SlotTask.GetIsTerminated() || m_bIsTerminated)
				SCR_EntityHelper.DeleteEntityAndChildren(m_SlotTask.GetSpawnedEntity());

			m_Task.HookTaskAsset(m_Entity);

			if (m_fMarkerUpdateInterval > 0)
				SCR_ScenarioFrameworkCallQueueSystem.GetCallQueueNonPausable().CallLater(UpdateMarkerPosition, m_fMarkerUpdateInterval, true);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Set interval for updating marker position
	//! \param[in] newInterval for new interval in ms. If set to 0, updates will be disabled
	void SetMarkerUpdateInterval(float newInterval)
	{
		SCR_ScenarioFrameworkCallQueueSystem.GetCallQueueNonPausable().Remove(UpdateMarkerPosition);
		
		if (m_fMarkerUpdateInterval > 0)
			SCR_ScenarioFrameworkCallQueueSystem.GetCallQueueNonPausable().CallLater(UpdateMarkerPosition, m_fMarkerUpdateInterval, true);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Updates marker position on map, if it changed from previous one
	void UpdateMarkerPosition()
	{
		if (!m_TaskSystem || !m_Task)
	        return;
		
		vector newPosition = GetMarkerPosition();
		if (newPosition == m_Task.GetTaskPosition())
			return;
		
		m_TaskSystem.MoveTask(m_Task, GetMarkerPosition());
	}
	
	//------------------------------------------------------------------------------------------------
	// Returns marker position vector, according to set parameters of LayerTask
	vector GetMarkerPosition()
	{
	    if (!m_TaskSystem)
	        return vector.Zero;
		
		if (m_EntityToAttachGetter)
		{
			SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_EntityToAttachGetter.Get());
			if (entityWrapper)
			{
				IEntity entity = entityWrapper.GetValue();
				if (entity)
					return entity.GetOrigin();
			}
		}	
		
		if (m_bPlaceMarkerOnSubjectSlot && m_SlotTask)
			return m_SlotTask.GetOwner().GetOrigin();

		return GetOwner().GetOrigin();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Creates a task for the support entity, sets its target faction, and moves it to the specified location.
	//! \return true if the task creation is successful, false otherwise.
	protected bool CreateTask(vector position)
	{
		SCR_Task task = m_TaskSystem.CreateTask(m_sTaskPrefab, GetName(), PrepareName(), PrepareDescription(), position);
		m_Task = SCR_ScenarioFrameworkTask.Cast(task);
		if (!m_Task)
		{
			Print(string.Format("ScenarioFramework: Creating of task %1 failed! Task manager refused to create it.", m_sTaskTitle), LogLevel.ERROR);
			return false;
		}
		//TODO: make selection of the faction dynamic
		array<Faction> aPlayableFactions = {};
		SCR_Faction factionSelected;
		FactionManager manager = GetGame().GetFactionManager();
		if (!manager)
		{
			Print(string.Format("ScenarioFramework: Creating of task %1 failed! Faction manager doesn't exist", m_sTaskTitle), LogLevel.ERROR);
			return false;
		}

		manager.GetFactionsList(aPlayableFactions);
		if (m_sFactionKey.IsEmpty())
			m_sFactionKey = "US";	//set a default US one if none is set by user

		SCR_Faction testFaction = SCR_Faction.Cast(manager.GetFactionByKey(m_sFactionKey));

		foreach (Faction faction : aPlayableFactions)
		{
			if (!SCR_Faction.Cast(faction))
				continue;

			if (SCR_Faction.Cast(faction).IsPlayable())
			{
				if (m_sFactionKey == "")
				{
					factionSelected = SCR_Faction.Cast(faction);
					break;
				}

				if (faction == testFaction)
				{
					factionSelected = SCR_Faction.Cast(faction);
					break;
				}
			}
		}

		if (!factionSelected)
		{
			Print(string.Format("ScenarioFramework: Creating of task %1 failed for %2! No playable faction available", m_sTaskTitle, GetOwner().GetName()), LogLevel.ERROR);
			return false;
		}
		
		m_TaskSystem.SetTaskOwnership(m_Task, m_eTaskOwnership);
		m_TaskSystem.SetTaskVisibility(m_Task, m_eTaskVisibility);
		m_TaskSystem.SetTaskUIVisibility(m_Task, m_eTaskUIVisibility);

		if (!SCR_StringHelper.IsEmptyOrWhiteSpace(m_sSubtasksDescription))
			m_Task.SetCustomSubtaskDescription(m_sSubtasksDescription);
		
		m_Task.SetSlotTask(m_SlotTask);
		m_TaskSystem.AddTaskFaction(task, factionSelected.GetFactionKey());

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Prepares task name based on slot task or default task title.
	//! \return the task title for the method.
	protected string PrepareName()
	{
		if (m_SlotTask)
		{
			if (!m_SlotTask.GetTaskTitle().IsEmpty())
				return m_SlotTask.GetTaskTitle();
			else
				return m_sTaskTitle;
		}
		else
		{
			Print(string.Format("ScenarioFramework - LayerTask: Task Subject not found doesn't exist for %1. Task won't be spawned!", GetOwner().GetName()), LogLevel.ERROR);
			return m_sTaskTitle;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Prepares task description based on slot task or default task description.
	//! \return the task description or briefing for the current task.
	protected string PrepareDescription()
	{
		if (m_SlotTask)
		{
			if (!m_SlotTask.GetTaskDescription().IsEmpty())
				return m_SlotTask.GetTaskDescription();
			else
				return m_sTaskDescription;
		}
		else
		{
			Print(string.Format("ScenarioFramework - LayerTask: Task Subject not found doesn't exist for %1. Task won't be spawned!", GetOwner().GetName()), LogLevel.ERROR);
			return m_sTaskDescription;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Initializes task icon based on provided task or slot task. If task icon set name or path is empty, error
	protected void InitIcon()
	{
		if (m_SlotTask)
		{
			if (SCR_StringHelper.IsEmptyOrWhiteSpace(m_SlotTask.m_sTaskIconSet))
			{
				if (SCR_StringHelper.IsEmptyOrWhiteSpace(m_sTaskIconSet))
					return;
				
				if (SCR_StringHelper.IsEmptyOrWhiteSpace(m_sTaskIconName))
				{
					Print(string.Format("ScenarioFramework - LayerTask: Init Icon failed for %1 due to empty Task Icon Name", GetOwner().GetName()), LogLevel.ERROR);
					return;
				}
				
				m_Task.SetTaskIconPath(m_sTaskIconSet);
				m_Task.SetTaskIconSetName(m_sTaskIconName);
			}
			else
			{
				if (SCR_StringHelper.IsEmptyOrWhiteSpace(m_SlotTask.m_sTaskIconName))
				{
					Print(string.Format("ScenarioFramework - LayerTask: Init Icon failed for %1 due to empty Task Icon Name on the Slot Task", GetOwner().GetName()), LogLevel.ERROR);
					return;
				}
				
				m_Task.SetTaskIconPath(m_SlotTask.m_sTaskIconSet);
				m_Task.SetTaskIconSetName(m_SlotTask.m_sTaskIconName);
			}
		}
		else
		{
			Print(string.Format("ScenarioFramework - LayerTask: Slot Task not found doesn't exist for %1. Icon won't be initialized", GetOwner().GetName()), LogLevel.ERROR);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Retrieves layer task by name.
	//! \param[in] name of target layer task
	SCR_ScenarioFrameworkLayerTask GetLayerTaskByName(string name)
	{
		IEntity entity = GetGame().GetWorld().FindEntityByName(name);
		if (!entity)
			return null;
		
		return SCR_ScenarioFrameworkLayerTask.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerTask));
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetupTask()
	{
		InitIcon();
		
		if (m_SlotTask)
		{
			array<LocalizedString> descriptionParams = {};
			descriptionParams.Insert(m_SlotTask.GetSpawnedEntityDisplayName());
			m_Task.SetTaskName(PrepareName(), descriptionParams);
			m_Task.SetTaskDescription(PrepareDescription(), descriptionParams);
		}
		else
		{
			Print(string.Format("ScenarioFramework - LayerTask: Task Subject not found doesn't exist for %1. Task won't be spawned!", GetOwner().GetName()), LogLevel.ERROR);
		}
		
		m_TaskSystem.ShowProgressForTask(m_Task, m_bProgressBar);
		
		s_OnTaskSetup.Invoke(m_Task);
		
		//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
		
		string cachedLanguage;
		WidgetManager.GetLanguage(cachedLanguage);
		if (cachedLanguage != "en_us")
			WidgetManager.SetLanguage("en_us");
		
		Print(string.Format("ScenarioFramework: -> LayerTask: SlotTask %1 - generating task %2. Description: %3", m_SlotTask.GetOwner().GetName(), WidgetManager.Translate(m_Task.GetTaskName()), string.Format(WidgetManager.Translate(m_Task.GetTaskDescription(), WidgetManager.Translate(m_Task.GetSlotTask().GetSpawnedEntityDisplayName())))), LogLevel.NORMAL);
		
		if (cachedLanguage != "en_us")
			WidgetManager.SetLanguage(cachedLanguage);
		
		//---- REFACTOR NOTE END ----
		
		if (m_bIsSubtask)
			SetupSubtask();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets up a subtask, disables linking data to related tasks, and registers the task as a subtask of a parent task
	void SetupSubtask()
	{
		SCR_ExtendedTaskData taskData = SCR_ExtendedTaskData.Cast(m_Task.GetTaskData());
		if (taskData)
			taskData.m_bLinkDataToRelatedTasks = false;
		
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(m_sParentLayerTask))
			m_TopParentLayerTask = GetTopMostParentTaskLayer();
		else
			m_TopParentLayerTask = GetLayerTaskByName(m_sParentLayerTask);
		
		if (!m_TopParentLayerTask)
		{
			Print(string.Format("ScenarioFramework - LayerTask: Registering this task as a Subtask failed for %1", GetOwner().GetName()), LogLevel.ERROR);
			return;
		}
		
		if (m_bIsOptional)
		{
			array<LocalizedString> nameParams = {};
			nameParams.Insert("#AR-Tasks_Optional");
			nameParams.Insert(PrepareName());
			m_Task.SetTaskName("%1 %2", nameParams);
		}
		
		if (m_TopParentLayerTask.GetIsInitiated())
			OnTopParentLayerTaskInitialized(m_TopParentLayerTask);
		else
			m_TopParentLayerTask.GetOnAllChildrenSpawned().Insert(OnTopParentLayerTaskInitialized);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] layer Initializes the task for the top parent layer, adding it as a child task to the parent task.
	protected void OnTopParentLayerTaskInitialized(SCR_ScenarioFrameworkLayerBase layer)
	{
		if (!m_TopParentLayerTask || !m_Task)
			return;
		
		m_TopParentLayerTask.GetOnAllChildrenSpawned().Remove(OnTopParentLayerTaskInitialized);
		
		SCR_ScenarioFrameworkTask parentTask = m_TopParentLayerTask.GetTask();
		if (!parentTask)
			return;
		
		m_TaskSystem.AddChildTaskTo(parentTask, m_Task);
	}

	//------------------------------------------------------------------------------------------------
	//! Removes task from support entity and despawns if in edit mode or task is cancelled.
	void ~SCR_ScenarioFrameworkLayerTask()
	{
		if (SCR_Global.IsEditMode())
			return;
		
		SCR_ScenarioFrameworkCallQueueSystem.GetCallQueueNonPausable().Remove(UpdateMarkerPosition);
		
		DynamicDespawn(this);
		if (m_TaskSystem && m_Task)
			ProcessLayerTaskState(SCR_ETaskState.CANCELLED);
	}
}
