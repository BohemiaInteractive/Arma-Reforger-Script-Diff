[EntityEditorProps(category: "GameScripted/ScenarioFramework/Slot", description: "")]
class SCR_ScenarioFrameworkSlotTaskClass : SCR_ScenarioFrameworkSlotBaseClass
{
}

class SCR_ScenarioFrameworkSlotTask : SCR_ScenarioFrameworkSlotBase
{
	[Attribute(desc: "Name of the task in list of tasks", category: "Task")]
	LocalizedString m_sTaskTitle;

	[Attribute(desc: "Description of the task", category: "Task")]
	LocalizedString m_sTaskDescription;
	
	[Attribute(params: "edds imageset", uiwidget: UIWidgets.ResourcePickerThumbnail, desc: "Task icon set", category: "Task UI")]
	ResourceName m_sTaskIconSet;
	
	[Attribute(desc: "Name of the specific icon from the icon set", category: "Task UI")]
	string m_sTaskIconName;

	[Attribute(desc: "Text for the Execution category in Briefing", category: "Task")]
	LocalizedString m_sTaskExecutionBriefing;

	[Attribute(desc: "StringID for the Intro Voiceline action to be processed. Processing must be setup after tasks are initialized.", category: "Task")]
	string m_sTaskIntroVoiceline;
	
	[Attribute(desc: "Conditions that will be checked upon trying to finish a task", category: "Task")]
	ref array<ref SCR_ScenarioFrameworkActivationConditionBase> m_aFinishConditions;
	
	[Attribute(defvalue: SCR_EScenarioFrameworkLogicOperators.AND.ToString(), UIWidgets.ComboBox, "Which Boolean Logic will be used for Finish Conditions.", "", enums: SCR_EScenarioFrameworkLogicOperatorHelper.GetParamInfo(), category: "Task")]
	SCR_EScenarioFrameworkLogicOperators m_eFinishConditionLogic;

	[Attribute(desc: "What to do once task is finished", UIWidgets.Auto, category: "Task State Changed Actions")]
	ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActionsOnFinished;

	[Attribute(desc: "What to do once task is created", UIWidgets.Auto, category: "Task State Changed Actions")]
	ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActionsOnCreated;

	[Attribute(desc: "What to do once task is created", UIWidgets.Auto, category: "Task State Changed Actions")]
	ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActionsOnFailed;
	
	[Attribute(desc: "What to do once task is cancelled", UIWidgets.Auto, category: "Task State Changed Actions")]
	ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActionsOnCancelled;

	[Attribute(desc: "What to do once task progressed", UIWidgets.Auto, category: "Task State Changed Actions")]
	ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActionsOnProgress;

	[Attribute(desc: "What to do once task is updated", UIWidgets.Auto, category: "Task State Changed Actions")]
	ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActionsOnAssigned;

	SCR_ScenarioFrameworkLayerTask	m_TaskLayer;
	bool m_bTaskResolvedBeforeLoad;
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] newState Task state change event, triggers actions based on state change.
	void OnTaskStateChanged(SCR_ETaskState newState)
	{
		if (m_TaskLayer)
		{
			SCR_ScenarioFrameworkTask task = m_TaskLayer.GetTask();
			if (task)
				m_TaskLayer.OnTaskStateChanged(task.GetTaskState(), newState);
		}
		
		if (newState == SCR_ETaskState.CREATED)
		{
			foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnCreated)
			{
				action.OnActivate(GetOwner());
			}
		}
		else if (newState == SCR_ETaskState.COMPLETED && !m_bTaskResolvedBeforeLoad)
		{
			foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnFinished)
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
	//! \param[in] state Indicates whether the task is resolved before loading.
	void SetTaskResolvedBeforeLoad(bool state)
	{
		m_bTaskResolvedBeforeLoad = state;
	}

	//------------------------------------------------------------------------------------------------
	//! Binds current task to parent task layer if available, else logs error.
	protected void StoreTaskSubjectToParentTaskLayer()
	{
		m_TaskLayer = GetParentTaskLayer();
		if (m_TaskLayer)
		{
			m_TaskLayer.SetSlotTask(this);
			if (m_Entity)
				m_TaskLayer.SetEntity(m_Entity);
		}
		else
		{
			Print(string.Format("ScenarioFramework: %1 could not be spawned and cannot bind it to task %2", m_sObjectToSpawn, m_TaskLayer.GetOwner().GetName()), LogLevel.ERROR);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Returns localized string for task title based on current state.
	//! \param[in] iState iState represents the current state of the task title.
	//! \return the localized string representing the title of the task with index iState.
	LocalizedString GetTaskTitle(int iState = 0)
	{
		return m_sTaskTitle;
	}

	//------------------------------------------------------------------------------------------------
	//! \return Task execution briefing string.
	LocalizedString GetTaskExecutionBriefing()
	{
		return m_sTaskExecutionBriefing;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns the description of the current task based on its state.
	//! \param[in] iState iState represents the current state of the task.
	//! \return the description string for the current task state.
	string GetTaskDescription(int iState = 0)
	{
		return m_sTaskDescription;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Represents the current state of the task or its parent task layer, if no parent layer exists, it returns CREATED
	SCR_ETaskState GetTaskState()
	{
		if (!m_TaskLayer)
		{
			m_TaskLayer = GetParentTaskLayer();
			if (!m_TaskLayer)
				return SCR_ETaskState.CREATED;
		}
		
		if (m_TaskLayer.m_Task)
			return m_TaskLayer.m_Task.GetTaskState();
		else
			return SCR_ETaskState.CREATED;
	}

	//------------------------------------------------------------------------------------------------
	//! \return parent ScenarioFrameworkLayerTask of the entity owning this task.
	SCR_ScenarioFrameworkLayerTask GetParentTaskLayer()
	{
		SCR_ScenarioFrameworkLayerTask layer;
		IEntity entity = GetOwner().GetParent();
		while (entity)
		{
			layer = SCR_ScenarioFrameworkLayerTask.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerTask));
			if (layer)
				return layer;

			entity = entity.GetParent();
		}

		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Resets all action activations, cancels task, and restores default settings.
	//! \param[in] includeChildren Resets actions' activation count, cancels task if TaskLayer exists, resets TaskLayer, and restores default
	//! \param[in] reinitAfterRestoration Resets all action counts after restoring default state, also cancels current task if TaskLayer is present.
	//! \param[in] affectRandomization Affects randomization state during restoration process.
	override void RestoreToDefault(bool includeChildren = false, bool reinitAfterRestoration = false, bool affectRandomization = true)
	{
		foreach (SCR_ScenarioFrameworkActionBase activationAction : m_aActionsOnFinished)
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
		
		if (m_TaskLayer)
			m_TaskLayer.ProcessLayerTaskState(SCR_ETaskState.CANCELLED);
		
		m_TaskLayer = null;
		m_bTaskResolvedBeforeLoad = false;
		
		super.RestoreToDefault(includeChildren, reinitAfterRestoration, affectRandomization);
	}

	//------------------------------------------------------------------------------------------------
	//! Initializes scenario with same activation type as parent.
	override void DynamicReinit()
	{
		Init(null, SCR_ScenarioFrameworkEActivationType.SAME_AS_PARENT);
	}

	//------------------------------------------------------------------------------------------------
	//! Removes self from dynamic despawn list, if not already spawned, initiates despawn, removes inventory change event
	//! \param[in] layer Layer represents the scenario framework layer where dynamic despawning occurs.
	override void DynamicDespawn(SCR_ScenarioFrameworkLayerBase layer)
	{
		GetOnAllChildrenSpawned().Remove(DynamicDespawn);
		if (!m_Entity && !SCR_StringHelper.IsEmptyOrWhiteSpace(m_sObjectToSpawn))
		{
			GetOnAllChildrenSpawned().Insert(DynamicDespawn);
			return;
		}
		
		if (!m_bInitiated || m_bExcludeFromDynamicDespawn)
			return;
		
		if (m_Entity)
		{
			m_vPosition = m_Entity.GetOrigin();
			InventoryItemComponent invComp = InventoryItemComponent.Cast(m_Entity.FindComponent(InventoryItemComponent));
			if (invComp)
				invComp.m_OnParentSlotChangedInvoker.Remove(OnInventoryParentChanged);
		}
		
		m_bInitiated = false;
		m_bDynamicallyDespawned = true;
		SCR_EntityHelper.DeleteEntityAndChildren(m_Entity);
	}	
	
	//------------------------------------------------------------------------------------------------
	//! Checks if layer is not terminated, spawns children if parent exists, returns false if terminated or children not spawnned
	//! \return true if not terminated, otherwise false.
	override bool InitNotTerminated()
	{
		if (m_bIsTerminated)
			StoreTaskSubjectToParentTaskLayer();
		
		return super.InitNotTerminated();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Spawn check for entity initiation, handles termination if no entity, returns true if entity exists, false otherwise.
	//! \return whether entity is successfully spawned or not.
	override bool InitEntitySpawnCheck()
	{
		if (!m_Entity)
			StoreTaskSubjectToParentTaskLayer();
		
		super.InitEntitySpawnCheck();
		
		if (m_Entity)
			return true;
		else
			return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Finishes initialization, sets termination flag, and calls base class FinishInit.
	override void FinishInit()
	{
		StoreTaskSubjectToParentTaskLayer();
		super.FinishInit();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Removes current task from support entity if in scenario mode, otherwise despawns self.
	void ~SCR_ScenarioFrameworkSlotTask()
	{
		if (SCR_Global.IsEditMode())
			return;
		
		DynamicDespawn(this);
		
		if (!m_TaskLayer)
		{
			m_TaskLayer = GetParentTaskLayer();
			if (!m_TaskLayer)
				return;

			m_TaskLayer.ProcessLayerTaskState(SCR_ETaskState.CANCELLED);
		}
	}
}
