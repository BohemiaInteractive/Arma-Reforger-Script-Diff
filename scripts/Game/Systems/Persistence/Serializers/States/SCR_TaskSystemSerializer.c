//! Dummy state proxy to access task system instance
class SCR_TaskSystemData : PersistentState
{
}

class SCR_TaskSystemSerializer : ScriptedStateSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return SCR_TaskSystemData;
	}

	//------------------------------------------------------------------------------------------------
	override ESerializeResult Serialize(notnull Managed instance, notnull BaseSerializationSaveContext context)
	{
		const SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
		if (!taskSystem)
			return ESerializeResult.DEFAULT;

		array<SCR_Task> outTasks();
		taskSystem.GetTasks(outTasks);

		array<ref SCR_TaskSave> tasks();
		foreach (auto task : outTasks)
		{
			if (!task)
				continue;
			
			auto save = SCR_TaskSave.GetTaskTypeSave(task);
			save.Save(task, GetSystem());
			if (!save.IsDefault())
				tasks.Insert(save);
		}

		if (tasks.IsEmpty())
			return ESerializeResult.DEFAULT;

		context.WriteValue("version", 1);
		context.Write(tasks);
		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override bool Deserialize(notnull Managed instance, notnull BaseSerializationLoadContext context)
	{
		const SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
		if (!taskSystem)
			return true; // SF system no longer present, so we can ignore it.

		int version;
		context.Read(version);

		array<ref SCR_TaskSave> tasks();
		context.Read(tasks);

		foreach (auto save : tasks)
		{
			save.Load(taskSystem, GetSystem());
		}

		return true;
	}
}

//------------------------------------------------------------------------------------------------
class SCR_TaskSave
{
	string 						m_sId;
	ResourceName 				m_rPrefab;
	vector 						m_vPosition;
	string 						m_sIconSet;
	ResourceName				m_rIconPath;
	ref array<LocalizedString> 	m_aTaskName = {};
	ref array<LocalizedString>	m_aTaskDescription = {};
	ref array<UUID> 			m_aAssignees = {};
	ref array<string> 			m_aOwnerFactionKeys = {};
	ref array<UUID> 			m_aOwnerGroupIDs = {};
	ref array<UUID> 			m_aOwnerExecutors = {};
	SCR_ETaskState 				m_eState;
	SCR_ETaskOwnership 			m_eOwnership;
	SCR_ETaskVisibility 		m_eVisibility;
	SCR_ETaskUIVisibility 		m_eUIVisibility;

	//------------------------------------------------------------------------------------------------
	void Save(notnull SCR_Task task, notnull PersistenceSystem persistence)
	{
		auto taskData = task.GetTaskData();
		m_sId = taskData.m_sID;
		m_rPrefab = SCR_ResourceNameUtils.GetPrefabName(task);
		m_vPosition = taskData.m_vPosition;

		auto uiInfo = task.GetTaskUIInfo();
		if (uiInfo)
		{
			if (uiInfo.HasCustomIcon())
			{
				m_sIconSet = uiInfo.GetIconSetName();
				if (m_sIconSet)
					m_rIconPath = uiInfo.GetImageSetPath();
			}

			if (uiInfo.HasCustomName())
			{
				array<LocalizedString> params();
				const LocalizedString name = uiInfo.GetUnformattedName(params);
				if (name)
				{
					m_aTaskName.Reserve(params.Count() + 1);
					m_aTaskName.Insert(name);
					m_aTaskName.InsertAll(params);
				}
			}

			if (uiInfo.HasCustomDescription())
			{
				array<LocalizedString> params();
				const LocalizedString description = uiInfo.GetUnformattedDescription(params);
				if (description)
				{
					m_aTaskDescription.Reserve(params.Count() + 1);
					m_aTaskDescription.Insert(description);
					m_aTaskDescription.InsertAll(params);
				}
			}
		}

		foreach (auto asignee : taskData.m_aAssignees)
		{
			const UUID id = GetTaskExecutorId(asignee, persistence);
			if (!id.IsNull())
				m_aAssignees.Insert(id);
		}

		m_aOwnerFactionKeys = taskData.m_aOwnerFactionKeys;

		auto groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (groupManager)
		{
			foreach (auto ownerGroup : taskData.m_aOwnerGroupIDs)
			{
				const UUID id = persistence.GetId(groupManager.FindGroup(ownerGroup));
				if (!id.IsNull())
					m_aOwnerGroupIDs.Insert(id);
			}
		}

		foreach (auto owner : taskData.m_aOwnerExecutors)
		{
			const UUID id = GetTaskExecutorId(owner, persistence);
			if (!id.IsNull())
				m_aOwnerExecutors.Insert(id);
		}

		m_eState = taskData.m_eState;
		m_eOwnership = taskData.m_eOwnership;
		m_eVisibility = taskData.m_eVisibility;
		m_eUIVisibility = taskData.m_eUIVisibility;
	}

	//------------------------------------------------------------------------------------------------
	bool IsDefault()
	{
		return
			m_sId.IsEmpty() &&
			m_vPosition == vector.Zero &&
			m_sIconSet.IsEmpty() &&
			m_aTaskName.IsEmpty() &&
			m_aTaskDescription.IsEmpty() &&
			m_aAssignees.IsEmpty() &&
			m_aOwnerFactionKeys.IsEmpty() &&
			m_aOwnerGroupIDs.IsEmpty() &&
			m_aOwnerExecutors.IsEmpty() &&
			m_eState == SCR_ETaskState.CREATED &&
			m_eOwnership == SCR_ETaskOwnership.NONE &&
			m_eVisibility == SCR_ETaskVisibility.NONE &&
			m_eUIVisibility == SCR_ETaskUIVisibility.NONE;
	}

	//------------------------------------------------------------------------------------------------
	SCR_Task Load(notnull SCR_TaskSystem taskSystem, notnull PersistenceSystem persistence)
	{
		auto task = taskSystem.CreateTask(m_rPrefab, m_sId, "", "", m_vPosition);
		if (!task)
			return null;

		auto uiInfo = task.GetTaskUIInfo();
		if (uiInfo)
		{
			if (m_sIconSet)
			{
				task.SetTaskIconSetName(m_sIconSet);
				task.SetTaskIconPath(m_rIconPath);
			}

			if (!m_aTaskName.IsEmpty())
			{
				const LocalizedString name = m_aTaskName[0];
				m_aTaskName.RemoveOrdered(0);
				task.SetTaskName(name, m_aTaskName);
			}

			if (!m_aTaskDescription.IsEmpty())
			{
				const LocalizedString description = m_aTaskDescription[0];
				m_aTaskDescription.RemoveOrdered(0);
				task.SetTaskDescription(description, m_aTaskDescription);
			}
		}

		taskSystem.SetTaskState(task, m_eState);
		task.SetTaskOwnership(m_eOwnership);
		task.SetTaskVisibility(m_eVisibility);
		task.SetTaskUIVisibility(m_eUIVisibility);

		foreach (auto factionKey : m_aOwnerFactionKeys)
		{
			task.AddOwnerFactionKey(factionKey);
		}

		foreach (auto assignee : m_aAssignees)
		{
			Tuple2<SCR_Task, int> taskContext(task, 0);
			PersistenceWhenAvailableTask loadTask(OnExecutorAvailable, taskContext);
			persistence.WhenAvailable(assignee, loadTask);
		}

		foreach (auto group : m_aOwnerGroupIDs)
		{
			Tuple1<SCR_Task> taskContext(task);
			PersistenceWhenAvailableTask loadTask(OnGroupAvailable, taskContext);
			persistence.WhenAvailable(group, loadTask);
		}

		foreach (auto owner : m_aOwnerExecutors)
		{
			Tuple2<SCR_Task, int> taskContext(task, 1);
			PersistenceWhenAvailableTask loadTask(OnExecutorAvailable, taskContext);
			persistence.WhenAvailable(owner, loadTask);
		}

		return task;
	}

	//------------------------------------------------------------------------------------------------
	UUID GetTaskExecutorId(notnull SCR_TaskExecutor executor, notnull PersistenceSystem persistence)
	{
		auto executorEntity = SCR_TaskExecutorEntity.Cast(executor);
		if (executorEntity)
			return persistence.GetId(executorEntity.GetEntity());

		auto executorGroup = SCR_TaskExecutorGroup.Cast(executor);
		if (executorGroup)
		{
			auto groupManager = SCR_GroupsManagerComponent.GetInstance();
			if (!groupManager)
				return UUID.NULL_UUID;

			return persistence.GetId(groupManager.FindGroup(executorGroup.GetGroupID()));
		}

		auto executorPlayer = SCR_TaskExecutorPlayer.Cast(executor);
		if (executorPlayer)
		{
			auto playerManager = GetGame().GetPlayerManager();
			if (!playerManager)
				return UUID.NULL_UUID;

			return persistence.GetId(playerManager.GetPlayerController(executorPlayer.GetPlayerID()));
		}

		return UUID.NULL_UUID;
	}

	//------------------------------------------------------------------------------------------------
	static SCR_TaskSave GetTaskTypeSave(const SCR_Task task)
	{
		if (task.IsInherited(SCR_CampaignMilitaryBaseTaskEntity))
			return new SCR_CampaignMilitaryTaskSave();

		if (task.IsInherited(SCR_ScenarioFrameworkTask))
			return new SCR_ScenarioFrameworkTaskSave();

		if (task.IsInherited(SCR_ExtendedTask))
			return new SCR_ExtendedTaskSave();

		return new SCR_TaskSave();
	}

	//------------------------------------------------------------------------------------------------
	protected static void OnExecutorAvailable(Managed instance, PersistenceDeferredDeserializeTask task, bool expired, Managed context)
	{
		auto taskContext = Tuple2<SCR_Task, int>.Cast(context);
		if (!taskContext.param1)
			return;

		SCR_TaskExecutor executor;
		auto playerController = PlayerController.Cast(instance);
		if (playerController)
		{
			executor = SCR_TaskExecutor.FromPlayerID(playerController.GetPlayerId());
		}
		else
		{
			auto group = SCR_AIGroup.Cast(instance);
			if (group)
			{
				executor = SCR_TaskExecutor.FromGroup(group.GetGroupID());
			}
			else
			{
				auto entity = IEntity.Cast(instance);
				executor = SCR_TaskExecutor.FromEntity(entity);
			}
		}

		if (!executor)
			return;

		if (taskContext.param2 == 0)
		{
			taskContext.param1.AddTaskAssignee(executor);
			return;
		}

		taskContext.param1.AddOwnerExecutor(executor)
	}

	//------------------------------------------------------------------------------------------------
	protected static void OnGroupAvailable(Managed instance, PersistenceDeferredDeserializeTask task, bool expired, Managed context)
	{
		auto taskContext = Tuple1<SCR_Task>.Cast(context);
		if (!taskContext.param1)
			return;

		auto group = SCR_AIGroup.Cast(instance);
		if (!group)
			return;

		taskContext.param1.AddOwnerGroupID(group.GetGroupID());
	}

	//------------------------------------------------------------------------------------------------
	bool SerializationSave(BaseSerializationSaveContext context)
	{
		context.WriteDefault(m_sId, string.Empty);
		context.WriteDefault(m_rPrefab, ResourceName.Empty);
		context.WriteDefault(m_vPosition, vector.Zero);
		context.WriteDefault(m_sIconSet, string.Empty);
		context.WriteDefault(m_rIconPath, ResourceName.Empty);

		if (!m_aTaskName.IsEmpty() || !context.CanSeekMembers())
			context.Write(m_aTaskName);

		if (!m_aTaskDescription.IsEmpty() || !context.CanSeekMembers())
			context.Write(m_aTaskDescription);

		if (!m_aAssignees.IsEmpty() || !context.CanSeekMembers())
			context.Write(m_aAssignees);

		if (!m_aOwnerFactionKeys.IsEmpty() || !context.CanSeekMembers())
			context.Write(m_aOwnerFactionKeys);

		if (!m_aOwnerGroupIDs.IsEmpty() || !context.CanSeekMembers())
			context.Write(m_aOwnerGroupIDs);

		if (!m_aOwnerExecutors.IsEmpty() || !context.CanSeekMembers())
			context.Write(m_aOwnerExecutors);

		context.WriteDefault(m_eState, SCR_ETaskState.CREATED);
		context.WriteDefault(m_eOwnership, SCR_ETaskOwnership.NONE);
		context.WriteDefault(m_eVisibility, SCR_ETaskVisibility.NONE);
		context.WriteDefault(m_eUIVisibility, SCR_ETaskUIVisibility.NONE);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	bool SerializationLoad(BaseSerializationLoadContext context)
	{
		context.ReadDefault(m_sId, string.Empty);
		context.ReadDefault(m_rPrefab, ResourceName.Empty);
		context.ReadDefault(m_vPosition, vector.Zero);
		context.ReadDefault(m_sIconSet, string.Empty);
		context.ReadDefault(m_rIconPath, ResourceName.Empty);

		if (!context.Read(m_aTaskName) && !context.CanSeekMembers())
			return false;

		if (!context.Read(m_aTaskDescription) && !context.CanSeekMembers())
			return false;

		if (!context.Read(m_aAssignees) && !context.CanSeekMembers())
			return false;

		if (!context.Read(m_aOwnerFactionKeys) && !context.CanSeekMembers())
			return false;

		if (!context.Read(m_aOwnerGroupIDs) && !context.CanSeekMembers())
			return false;

		if (!context.Read(m_aOwnerExecutors) && !context.CanSeekMembers())
			return false;

		context.ReadDefault(m_eState, SCR_ETaskState.CREATED);
		context.ReadDefault(m_eOwnership, SCR_ETaskOwnership.NONE);
		context.ReadDefault(m_eVisibility, SCR_ETaskVisibility.NONE);
		context.ReadDefault(m_eUIVisibility, SCR_ETaskUIVisibility.NONE);
		return true;
	}
}

//------------------------------------------------------------------------------------------------
class SCR_CampaignMilitaryTaskSave : SCR_TaskSave
{
	UUID m_sBaseId;

	//------------------------------------------------------------------------------------------------
	override void Save(notnull SCR_Task task, notnull PersistenceSystem persistence)
	{
		super.Save(task, persistence);

		const SCR_CampaignMilitaryBaseTaskEntity militaryBaseTask = SCR_CampaignMilitaryBaseTaskEntity.Cast(task);
		const SCR_CampaignMilitaryBaseComponent base = militaryBaseTask.GetMilitaryBase();
		if (base)
			m_sBaseId = persistence.GetId(base.GetOwner());
	}

	//------------------------------------------------------------------------------------------------
	override bool IsDefault()
	{
		return super.IsDefault() && m_sBaseId.IsNull();
	}

	//------------------------------------------------------------------------------------------------
	override SCR_Task Load(notnull SCR_TaskSystem taskSystem, notnull PersistenceSystem persistence)
	{
		auto task = SCR_CampaignMilitaryBaseTaskEntity.Cast(super.Load(taskSystem, persistence));
		if (task)
		{
			Tuple1<SCR_CampaignMilitaryBaseTaskEntity> ctx(task);
			PersistenceWhenAvailableTask baseTask(OnBaseAvailable, ctx);
			persistence.WhenAvailable(m_sBaseId, baseTask);
		}

		return task;
	}

	//------------------------------------------------------------------------------------------------
	protected static void OnBaseAvailable(Managed instance, PersistenceDeferredDeserializeTask task, bool expired, Managed context)
	{
		auto base = IEntity.Cast(instance);
		if (!base)
			return;

		const SCR_CampaignMilitaryBaseComponent militaryBase = SCR_CampaignMilitaryBaseComponent.Cast(base.FindComponent(SCR_CampaignMilitaryBaseComponent));
		if (!militaryBase)
			return;

		auto ctx = Tuple1<SCR_CampaignMilitaryBaseTaskEntity>.Cast(context);
		if (ctx.param1)
			ctx.param1.SetMilitaryBaseCallSign(militaryBase.GetCallsign());
	}

	//------------------------------------------------------------------------------------------------
	override bool SerializationSave(BaseSerializationSaveContext context)
	{
		if (!super.SerializationSave(context))
			return false;

		context.WriteDefault(m_sBaseId, UUID.NULL_UUID);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool SerializationLoad(BaseSerializationLoadContext context)
	{
		if (!super.SerializationLoad(context))
			return false;

		context.ReadDefault(m_sBaseId, UUID.NULL_UUID);
		return true;
	}
}

//------------------------------------------------------------------------------------------------
class SCR_ExtendedTaskSave : SCR_TaskSave
{
	float 							m_fProgress;
	ref array<ref SCR_TaskSave>		m_aChildTasks = {};

	//------------------------------------------------------------------------------------------------
	override void Save(notnull SCR_Task task, notnull PersistenceSystem persistence)
	{
		super.Save(task, persistence);

		auto extendedTaskData = SCR_ExtendedTaskData.Cast(task.GetTaskData());
		m_fProgress = extendedTaskData.m_fProgress;
		if (!extendedTaskData.m_aChildTasks)
			return;

		foreach (auto childRpl : extendedTaskData.m_aChildTasks)
		{
			auto rpl = RplComponent.Cast(Replication.FindItem(childRpl));
			if (!rpl)
				continue;

			auto childTask = SCR_Task.Cast(rpl.GetEntity());
			if (childTask)
			{
				auto save = GetTaskTypeSave(childTask);
				save.Save(childTask, persistence);
				if (!save.IsDefault())
					m_aChildTasks.Insert(save);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool IsDefault()
	{
		return
			super.IsDefault() &&
			float.AlmostEqual(m_fProgress, 0) &&
			m_aChildTasks.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	override SCR_Task Load(notnull SCR_TaskSystem taskSystem, notnull PersistenceSystem persistence)
	{
		auto task = SCR_ExtendedTask.Cast(super.Load(taskSystem, persistence));
		if (task)
		{
			task.SetTaskProgress(m_fProgress);
			foreach (auto childSave : m_aChildTasks)
			{
				auto childTask = childSave.Load(taskSystem, persistence);
				if (childTask)
					task.AddChildTask(childTask);
			}
		}

		return task;
	}

	//------------------------------------------------------------------------------------------------
	override bool SerializationSave(BaseSerializationSaveContext context)
	{
		if (!super.SerializationSave(context))
			return false;

		context.WriteDefault(m_fProgress, 0.0);

		if (!m_aChildTasks.IsEmpty() || !context.CanSeekMembers())
			context.Write(m_aChildTasks);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool SerializationLoad(BaseSerializationLoadContext context)
	{
		if (!super.SerializationLoad(context))
			return false;

		context.ReadDefault(m_fProgress, 0);

		if (!context.Read(m_aChildTasks) && !context.CanSeekMembers())
			return false;

		return true;
	}
}

//------------------------------------------------------------------------------------------------
class SCR_ScenarioFrameworkTaskSave : SCR_ExtendedTaskSave
{
	string m_sTaskExecutionBriefing;

	//------------------------------------------------------------------------------------------------
	override void Save(notnull SCR_Task task, notnull PersistenceSystem persistence)
	{
		super.Save(task, persistence);

		auto scenarioFrameworkTaskData = SCR_ScenarioFrameworkTaskData.Cast(task.GetTaskData());
		if (scenarioFrameworkTaskData.m_bCustomBriefing)
			m_sTaskExecutionBriefing = scenarioFrameworkTaskData.m_sTaskExecutionBriefing;
	}

	//------------------------------------------------------------------------------------------------
	override bool IsDefault()
	{
		return
			super.IsDefault() &&
			m_sTaskExecutionBriefing.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	override SCR_Task Load(notnull SCR_TaskSystem taskSystem, notnull PersistenceSystem persistence)
	{
		auto task = SCR_ScenarioFrameworkTask.Cast(super.Load(taskSystem, persistence));
		if (task && !m_sTaskExecutionBriefing.IsEmpty())
			task.SetTaskExecutionBriefing(m_sTaskExecutionBriefing);

		return task;
	}

	//------------------------------------------------------------------------------------------------
	override bool SerializationSave(BaseSerializationSaveContext context)
	{
		if (!super.SerializationSave(context))
			return false;

		context.WriteDefault(m_sTaskExecutionBriefing, string.Empty);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool SerializationLoad(BaseSerializationLoadContext context)
	{
		if (!super.SerializationLoad(context))
			return false;

		context.ReadDefault(m_sTaskExecutionBriefing, string.Empty);
		return true;
	}
}
