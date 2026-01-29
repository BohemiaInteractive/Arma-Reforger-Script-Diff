void ScriptInvokerGroupTaskSetMethod(SCR_Task task, int byPlayerID);
typedef func ScriptInvokerGroupTaskSetMethod;
typedef ScriptInvokerBase<ScriptInvokerGroupTaskSetMethod> ScriptInvokerGroupTaskSet;

void ScriptInvokerRequestedGroupTaskSetMethod(SCR_Task task, int byPlayerID, int requesterGroupId);
typedef func ScriptInvokerRequestedGroupTaskSetMethod;
typedef ScriptInvokerBase<ScriptInvokerRequestedGroupTaskSetMethod> ScriptInvokerRequestedGroupTaskSet;

[ComponentEditorProps(category: "GameScripted/Tasks", description: "Group task manager helps set groups to tasks, attach to game mode entity")]
class SCR_GroupTaskManagerComponentClass : ScriptComponentClass
{
}

class SCR_GroupTaskManagerComponent : ScriptComponent
{
	[Attribute("{8FD30741F5E75C87}Configs/Task/GroupTaskRelevanceConfig.conf", desc: "Config with group task relevance", params: "conf class=SCR_GroupTaskRelevanceConfig")]
	protected ref SCR_GroupTaskRelevanceConfig m_GroupTaskRelevanceConfig;

	[Attribute("1", desc:"Enables assigning frequencies", category:"Frequency settings")]
	protected bool m_bEnabledAssigningFrequencies;

	[Attribute("200", desc:"The frequency offset used to assign new frequencies", params:"0 inf", category:"Frequency settings")]
	protected int m_iTaskFrequencyOffset;

	[Attribute("45000", desc:"Min frequency", params:"0 inf", category:"Frequency settings")]
	protected int m_iTaskFrequencyMin;

	[Attribute("54000", desc:"Max frequency", params:"0 inf", category: "Frequency settings")]
	protected int m_iTaskFrequencyMax;

	[Attribute(desc:"List of tasks to which frequency will be assigned in the min, max range", category: "Frequency settings")]
	protected ref array<string> m_aAllowedTasksWithFrequencies;

	[RplProp()]
	protected ref array<bool> m_aFreeFactionFrequency = {};

	protected SCR_TaskSystem m_TaskSystem;
	protected SCR_GroupsManagerComponent m_GroupsManager;
	protected FactionManager m_FactionManager;
	protected RplComponent m_RplComponent;

	protected ref ScriptInvokerGroupTaskSet m_OnGroupTaskSet;
	protected ref ScriptInvokerRequestedGroupTaskSet m_OnRequestedGroupTaskSet;
	protected ref map<int, ref FactionHolder> m_mUsedFrequenciesByFactionMap = new map<int, ref FactionHolder>();
	protected ref map<int, int> m_mUsedFrequenciesByTaskMap = new map<int, int>(); //! <TaskRplId, frequency>

	protected static SCR_GroupTaskManagerComponent s_Instance;

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_AddTaskFrequency(RplId taskRplId, int frequency)
	{
		m_mUsedFrequenciesByTaskMap.Set(taskRplId, frequency);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_RemoveTaskFrequency(RplId taskRplId)
	{
		m_mUsedFrequenciesByTaskMap.Remove(taskRplId);
	}

	//------------------------------------------------------------------------------------------------
	//! Check if the assigning frequencies is enabled.
	//! \return true when enabled
	bool IsEnabledAssigningFrequencies()
	{
		return m_bEnabledAssigningFrequencies;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] faction
	//! \return true if any frequency is free for a faction
	bool IsAnyFrequencyFreeForFaction(notnull Faction faction)
	{
		int factionIndex = m_FactionManager.GetFactionIndex(faction);
		if (!m_aFreeFactionFrequency || !m_aFreeFactionFrequency.IsIndexValid(factionIndex))
			return false;

		return m_aFreeFactionFrequency[factionIndex];
	}

	//------------------------------------------------------------------------------------------------
	protected void SetFreeFrequencyForFaction(notnull Faction faction, bool isFree)
	{
		int factionIndex = m_FactionManager.GetFactionIndex(faction);
		if (!m_aFreeFactionFrequency || !m_aFreeFactionFrequency.IsIndexValid(factionIndex))
			return;

		m_aFreeFactionFrequency[factionIndex] = isFree;

		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	protected string GetTaskPrefabClassName(ResourceName taskPrefabName)
	{
		Resource resource = BaseContainerTools.LoadContainer(taskPrefabName);
		if (!resource || !resource.IsValid())
		{
			Print("Task prefab resource name is not valid", LogLevel.WARNING);
			return string.Empty;
		}

		BaseResourceObject resourceObject = resource.GetResource();
		if (!resourceObject)
			return string.Empty;

		BaseContainer container = resourceObject.ToBaseContainer();
		return container.GetClassName();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] taskPrefabResourceName
	//! \param[in] faction
	//! \return true when a task can be created for a faction
	bool CanCreateNewTaskWithResourceName(ResourceName taskPrefabResourceName, notnull Faction faction)
	{
		return CanCreateNewTaskWithClassName(GetTaskPrefabClassName(taskPrefabResourceName), faction);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] taskTypename
	//! \param[in] faction
	//! \return true when a task can be created for a faction
	bool CanCreateNewTaskWithTypename(typename taskTypename, notnull Faction faction)
	{
		return CanCreateNewTaskWithClassName(taskTypename.ToString(), faction);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] taskClassName
	//! \param[in] faction
	//! \return true when a task can be created for a faction
	bool CanCreateNewTaskWithClassName(string taskClassName, notnull Faction faction)
	{
		if (taskClassName == string.Empty)
		{
			Print("Task prefab class name is not set", LogLevel.WARNING);
			return false;
		}

		// Ignore free frequency check if task is not in list
		if (!m_aAllowedTasksWithFrequencies.Contains(taskClassName))
			return true;

		if (m_RplComponent.IsProxy())
			return IsAnyFrequencyFreeForFaction(faction);

		return GetFreeFrequency(faction) != -1;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] task
	//! \return true when the task can be assigned a frequency
	bool CanAssignFrequencyToTask(notnull SCR_Task task)
	{
		if (!IsEnabledAssigningFrequencies())
			return false;

		string taskClassName = task.Type().ToString();
		if (taskClassName == string.Empty)
		{
			Print("Task prefab class name is not set", LogLevel.WARNING);
			return false;
		}

		if (m_aAllowedTasksWithFrequencies.Contains(taskClassName))
			return true;

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] task
	//! \return task frequency
	int GetTaskFrequency(notnull SCR_Task task)
	{
		RplId taskRplId = GetTaskRplId(task);
		int frequency = 0;
		if (!taskRplId.IsValid())
			Print("Task rplId is not valid", LogLevel.WARNING);
		else if (!m_mUsedFrequenciesByTaskMap.Find(taskRplId, frequency))
			Print("Frequency not found", LogLevel.WARNING);

		return frequency;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] faction
	//! \param[in] frequency
	//! \return task
	SCR_Task GetTaskByFrequency(notnull Faction faction, int frequency)
	{
		if (!m_TaskSystem || frequency <= 0)
			return null;

		array<SCR_Task> tasks = {};
		m_TaskSystem.GetTasksByState(tasks, SCR_ETaskState.CREATED | SCR_ETaskState.ASSIGNED, faction.GetFactionKey());

		foreach (SCR_Task task : tasks)
		{
			if (!task)
				continue;

			int assignedFrequency = GetTaskFrequency(task);
			if (assignedFrequency != frequency)
				continue;

			return task;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] task
	//! \return task translated name
	string GetTaskTranslatedName(notnull SCR_Task task)
	{
		SCR_TaskData data = task.GetTaskData();
		if (data && data.m_UIInfo)
		{
			return data.m_UIInfo.GetTranslatedName();
		}

		return string.Empty;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] task
	//! \return task faction
	Faction GetTaskFaction(notnull SCR_Task task)
	{
		Faction faction;
		foreach (string factionKey : task.GetTaskData().m_aOwnerFactionKeys)
		{
			if (factionKey.IsEmpty())
				continue;

			faction = m_FactionManager.GetFactionByKey(factionKey);
			if (faction)
				return faction;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected RplId GetTaskRplId(SCR_Task task)
	{
		// cannot use Replication.FindItemId(task), because when calling the OnTaskRemoved event rplId is no longer valid

		RplComponent taskRplComponent = RplComponent.Cast(task.FindComponent(RplComponent));
		if (!taskRplComponent)
		{
			Print("Task RplComponent not exist", LogLevel.WARNING);
			return RplId.Invalid();
		}

		return taskRplComponent.Id();
	}

	//------------------------------------------------------------------------------------------------
	protected void AssignFrequency(notnull SCR_Task task)
	{
		if (!IsEnabledAssigningFrequencies())
			return;

		typename taskTypename = task.Type();
		if (taskTypename == typename.Empty)
			return;

		if (!m_aAllowedTasksWithFrequencies.Contains(taskTypename.ToString()))
			return;

		Faction faction = GetTaskFaction(task);
		if (!faction)
			return;

		int frequency = GetFreeFrequency(faction);
		if (frequency == -1)
			return;

		ClaimFrequency(task, frequency, faction);
	}

	//------------------------------------------------------------------------------------------------
	protected void ClaimFrequency(SCR_Task task, int frequency, Faction faction)
	{
		if (!task || !faction)
			return;

		RplId taskRplId = GetTaskRplId(task);
		if (!taskRplId.IsValid())
		{
			Print("RplId is not valid", LogLevel.WARNING);
			return;
		}

		FactionHolder usedForFactions = new FactionHolder();
		FactionHolder factions = new FactionHolder();

		if (!m_mUsedFrequenciesByFactionMap.Find(frequency, usedForFactions))
		{
			factions.Insert(faction);
			m_mUsedFrequenciesByFactionMap.Insert(frequency, factions);

		}
		else if (usedForFactions.Find(faction) == -1)
		{
			usedForFactions.Insert(faction);
		}

		RpcDo_AddTaskFrequency(taskRplId, frequency);
		Rpc(RpcDo_AddTaskFrequency, taskRplId, frequency);

		// check free frequency
		SetFreeFrequencyForFaction(faction, GetFreeFrequency(faction) != -1);
	}

	//------------------------------------------------------------------------------------------------
	protected int GetFreeFrequency(Faction faction)
	{
		int factionHQFrequency; // Don't assign this frequency to any of the tasks

		SCR_Faction scrFaction = SCR_Faction.Cast(faction);
		if (scrFaction)
			factionHQFrequency = scrFaction.GetFactionRadioFrequency();

		FactionHolder usedForFactions = new FactionHolder();
		int minFrequencyAvailable = m_iTaskFrequencyMin;

		while (minFrequencyAvailable <= m_iTaskFrequencyMax)
		{
			if (minFrequencyAvailable == factionHQFrequency)
			{
				minFrequencyAvailable += m_iTaskFrequencyOffset;
				continue; // We cannot assign this frequency to task, it is used by the factions HQ
			}

			if (!m_mUsedFrequenciesByFactionMap.Find(minFrequencyAvailable, usedForFactions))
				break; // No assigned frequencies for this faction yet

			if (usedForFactions.Find(faction) == -1)
				break; // Unused frequency found

			minFrequencyAvailable += m_iTaskFrequencyOffset;
		}

		if (minFrequencyAvailable <= m_iTaskFrequencyMax)
			return minFrequencyAvailable;

		Print("Ran out of frequencies for tasks", LogLevel.WARNING);
		return -1;
	}

	//------------------------------------------------------------------------------------------------
	protected void ReleaseFrequency(notnull SCR_Task task)
	{
		if (!IsEnabledAssigningFrequencies())
			return;

		RplId taskRplId = GetTaskRplId(task);
		if (!taskRplId.IsValid())
		{
			Print("RplId is not valid", LogLevel.WARNING);
			return;
		}

		int frequency;
		if (!m_mUsedFrequenciesByTaskMap.Find(taskRplId, frequency))
		{
			Print("This task has not assigned frequency", LogLevel.WARNING);
			return;
		}

		Faction faction = GetTaskFaction(task);
		if (!faction)
			return;

		FactionHolder factions = new FactionHolder();
		if (m_mUsedFrequenciesByFactionMap.Find(frequency, factions))
		{
			if (factions.Count() <= 1 && factions.Find(faction) != -1)
			{
				m_mUsedFrequenciesByFactionMap.Remove(frequency);
			}
			else
			{
				int factionIndex = factions.Find(faction);
				if (factionIndex >= 0 && factionIndex < factions.Count())
					factions.Remove(factionIndex);
			}

			RpcDo_RemoveTaskFrequency(taskRplId);
			Rpc(RpcDo_RemoveTaskFrequency, taskRplId);

			SetFreeFrequencyForFaction(faction, true);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \return instance of SCR_GroupTaskManagerComponent
	static SCR_GroupTaskManagerComponent GetInstance()
	{
		return s_Instance;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerGroupTaskSet GetOnGroupTaskSet()
	{
		if (!m_OnGroupTaskSet)
			m_OnGroupTaskSet = new ScriptInvokerGroupTaskSet();

		return m_OnGroupTaskSet;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerRequestedGroupTaskSet GetOnRequestedGroupTaskSet()
	{
		if (!m_OnRequestedGroupTaskSet)
			m_OnRequestedGroupTaskSet = new ScriptInvokerRequestedGroupTaskSet();

		return m_OnRequestedGroupTaskSet;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] taskTypename
	//! \return group task relevance
	SCR_GroupTaskRelevance GetGroupTaskRelevance(typename taskTypename)
	{
		if (!m_GroupTaskRelevanceConfig)
			return null;

		return m_GroupTaskRelevanceConfig.GetGroupTaskRelevance(taskTypename);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnGroupRoleChanged(int groupID, SCR_EGroupRole groupRole)
	{
		if (!m_GroupsManager)
			return;

		SCR_AIGroup group = m_GroupsManager.FindGroup(groupID);
		if (!group)
			return;

		// assign new group as owner to all tasks
		Faction faction = group.GetFaction();
		if (!faction)
			return;

		array<SCR_Task> tasks = {};
		m_TaskSystem.GetTasksByState(tasks, SCR_ETaskState.CREATED | SCR_ETaskState.ASSIGNED, faction.GetFactionKey());

		SCR_GroupTaskRelevance groupTaskRelevance;
		SCR_TaskExecutor taskGroupExecutor = SCR_TaskExecutorGroup.FromGroup(group.GetGroupID());
		SCR_BaseRequestedTaskEntity requestedTask;

		foreach (SCR_Task task : tasks)
		{
			if (m_TaskSystem.GetTaskOwnership(task) != SCR_ETaskOwnership.EXECUTOR)
				continue;

			// skip assigning requester group, it can't be assigned
			requestedTask = SCR_BaseRequestedTaskEntity.Cast(task);
			if (requestedTask && group.GetGroupID() == requestedTask.GetRequesterId())
			{
				// requester group see own request
				m_TaskSystem.AddTaskGroup(task, group.GetGroupID());
				continue;
			}

			groupTaskRelevance = GetGroupTaskRelevance(task.Type());
			if (!groupTaskRelevance)
				continue;

			if (groupTaskRelevance.IsTaskExecutableByGroup(group))
				m_TaskSystem.AddTaskExecutor(task, taskGroupExecutor);

			if (groupTaskRelevance.IsTaskVisibleForGroup(group))
				m_TaskSystem.AddTaskGroup(task, group.GetGroupID());
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlayableGroupRemoved(SCR_AIGroup group)
	{
		if (!group)
			return;

		CheckAndDeleteRequestedTasks(group);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlayerRemoved(SCR_AIGroup group, int playerID)
	{
		if (!group || group.GetPlayerCount() > 0)
			return;

		CheckAndDeleteRequestedTasks(group);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTaskRemoved(SCR_Task task)
	{
		ReleaseFrequency(task);
	}

	//------------------------------------------------------------------------------------------------
	protected void CheckAndDeleteRequestedTasks(SCR_AIGroup group)
	{
		if (!group)
			return;

		Faction faction = group.GetFaction();
		if (!faction)
			return;

		array<SCR_Task> tasks = {};
		m_TaskSystem.GetTasksByState(tasks, SCR_ETaskState.CREATED | SCR_ETaskState.ASSIGNED, faction.GetFactionKey());

		SCR_BaseRequestedTaskEntity requestedTask;

		// delete all requested tasks created by this group
		for (int i = tasks.Count() - 1; i >= 0; i--)
		{
			if (m_TaskSystem.GetTaskOwnership(tasks[i]) != SCR_ETaskOwnership.EXECUTOR)
				continue;

			requestedTask = SCR_BaseRequestedTaskEntity.Cast(tasks[i]);
			if (!requestedTask || group.GetGroupID() != requestedTask.GetRequesterId())
				continue;

			m_TaskSystem.DeleteTask(requestedTask);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] task
	void SetGroupTaskOwnership(notnull SCR_Task task)
	{
		if (!m_FactionManager || !m_TaskSystem || !m_GroupsManager)
			return;

		if (m_TaskSystem.GetTaskOwnership(task) != SCR_ETaskOwnership.EXECUTOR)
			return;

		array<string> taskFactions = m_TaskSystem.GetTaskFactions(task);
		if (!taskFactions || taskFactions.IsEmpty())
			return;

		SCR_GroupTaskRelevance groupTaskRelevance = GetGroupTaskRelevance(task.Type());
		if (!groupTaskRelevance)
			return;

		Faction faction;
		array<SCR_AIGroup> groups;
		foreach (string taskFaction : taskFactions)
		{
			faction = m_FactionManager.GetFactionByKey(taskFaction);
			if (!faction)
				continue;

			groups = m_GroupsManager.GetPlayableGroupsByFaction(faction);
			if (!groups || groups.IsEmpty())
				continue;

			foreach (SCR_AIGroup group : groups)
			{
				if (groupTaskRelevance.IsTaskExecutableByGroup(group))
					m_TaskSystem.AddTaskExecutor(task, SCR_TaskExecutorGroup.FromGroup(group.GetGroupID()));

				if (groupTaskRelevance.IsTaskVisibleForGroup(group))
					m_TaskSystem.AddTaskGroup(task, group.GetGroupID());
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] task
	//! \param[in] byPlayerID
	void SetGroupTask(notnull SCR_Task task, int byPlayerID)
	{
		SetGroupTaskOwnership(task);
		AssignFrequency(task);

		if (m_OnGroupTaskSet)
			m_OnGroupTaskSet.Invoke(task, byPlayerID);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] task
	void SetRequestedGroupTaskOwnership(notnull SCR_Task task)
	{
		if (!m_TaskSystem || !m_GroupsManager)
			return;

		if (m_TaskSystem.GetTaskOwnership(task) != SCR_ETaskOwnership.EXECUTOR)
			return;

		SCR_BaseRequestedTaskEntity requestedTask = SCR_BaseRequestedTaskEntity.Cast(task);
		if (!requestedTask)
			return;

		array<string> taskFactions = m_TaskSystem.GetTaskFactions(task);
		if (!taskFactions || taskFactions.IsEmpty())
			return;

		SCR_GroupTaskRelevance 	groupTaskRelevance = GetGroupTaskRelevance(task.Type());
		if (!groupTaskRelevance)
			return;

		Faction faction;
		array<SCR_AIGroup> groups;
		foreach (string taskFaction : taskFactions)
		{
			faction = m_FactionManager.GetFactionByKey(taskFaction);
			if (!faction)
				continue;

			groups = m_GroupsManager.GetPlayableGroupsByFaction(faction);
			if (!groups)
				continue;

			int groupID;
			foreach (SCR_AIGroup group : groups)
			{
				groupID = group.GetGroupID();

				// skip assigning requester group as executor, it can't be assigned
				if (groupID == requestedTask.GetRequesterId())
				{
					// requester group see own request
					m_TaskSystem.AddTaskGroup(task, groupID);
					continue;
				}

				if (groupTaskRelevance.IsTaskExecutableByGroup(group))
					m_TaskSystem.AddTaskExecutor(task, SCR_TaskExecutorGroup.FromGroup(groupID));

				if (groupTaskRelevance.IsTaskVisibleForGroup(group))
					m_TaskSystem.AddTaskGroup(task, groupID);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] task
	//! \param[in] byPlayerID
	//! \param[in] requesterGroupId
	void SetRequestedGroupTask(notnull SCR_Task task, int byPlayerID, int requesterGroupId)
	{
		SetRequestedGroupTaskOwnership(task);
		AssignFrequency(task);

		if (m_OnRequestedGroupTaskSet)
			m_OnRequestedGroupTaskSet.Invoke(task, byPlayerID, requesterGroupId);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] task
	//! \param[in] faction
	//! \return Playable groups sorted by group task relevance
	array<SCR_AIGroup> GetPlayableGroupsSortedByRelevance(notnull SCR_Task task, notnull Faction faction)
	{
		if (!m_GroupsManager)
			return null;

		SCR_GroupTaskRelevance groupTaskRelevance = GetGroupTaskRelevance(task.Type());
		if (!groupTaskRelevance)
			return null;

		array<SCR_AIGroup> playableGroups = m_GroupsManager.GetPlayableGroupsByFaction(faction);
		if (!playableGroups)
			return null;

		array<ref SCR_GroupRoleRelevance> relevanceList;
		groupTaskRelevance.GetOrderedGroupRoleRelevanceList(relevanceList);
		if (!relevanceList)
			return null;

		array<SCR_AIGroup> playableGroupsSorted = {};
		SCR_EGroupRole groupRole;
		foreach (SCR_GroupRoleRelevance groupRoleRelevance : relevanceList)
		{
			groupRole = groupRoleRelevance.GetGroupRole();

			foreach (SCR_AIGroup playableGroup : playableGroups)
			{
				if (playableGroup.GetGroupRole() == groupRole)
					playableGroupsSorted.Insert(playableGroup);
			}
		}

		return playableGroupsSorted;
	}

	//------------------------------------------------------------------------------------------------
	override bool RplSave(ScriptBitWriter writer)
	{
		int usedFrequenciesByTaskMapCount = m_mUsedFrequenciesByTaskMap.Count();
		writer.WriteInt(usedFrequenciesByTaskMapCount);

		foreach (int key, int value : m_mUsedFrequenciesByTaskMap)
		{
			writer.WriteInt(key);
			writer.WriteInt(value);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		int usedFrequenciesByTaskMapCount;
		reader.ReadInt(usedFrequenciesByTaskMapCount);

		int taskRplId;
		int frequency;
		SCR_Task task;
		Faction faction;
		m_mUsedFrequenciesByTaskMap.Clear();

		for (int i = 0; i < usedFrequenciesByTaskMapCount; i++)
		{
			reader.ReadInt(taskRplId);
			reader.ReadInt(frequency);
			m_mUsedFrequenciesByTaskMap.Insert(taskRplId, frequency);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		m_FactionManager = GetGame().GetFactionManager();
		m_GroupsManager = SCR_GroupsManagerComponent.GetInstance();

		m_TaskSystem = SCR_TaskSystem.GetInstance();
		if (!m_TaskSystem)
			return;

		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (!m_RplComponent || m_RplComponent.IsProxy())
			return;

		// check and set free faction frequencies
		for (int i, count = m_FactionManager.GetFactionsCount(); i < count; i++)
		{
			m_aFreeFactionFrequency.Insert(false);
			Faction faction = m_FactionManager.GetFactionByIndex(i);
			SetFreeFrequencyForFaction(faction, GetFreeFrequency(faction) != -1);
		}

		if (m_GroupsManager)
			m_GroupsManager.GetOnPlayableGroupRemoved().Insert(OnPlayableGroupRemoved);

		SCR_AIGroup.GetOnPlayerRemoved().Insert(OnPlayerRemoved);
		SCR_AIGroup.GetOnGroupRoleChanged().Insert(OnGroupRoleChanged);
		m_TaskSystem.GetOnTaskRemoved().Insert(OnTaskRemoved);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		super.OnDelete(owner);

		if (m_GroupsManager)
			m_GroupsManager.GetOnPlayableGroupRemoved().Remove(OnPlayableGroupRemoved);

		SCR_AIGroup.GetOnPlayerRemoved().Remove(OnPlayerRemoved);
		SCR_AIGroup.GetOnGroupRoleChanged().Remove(OnGroupRoleChanged);

		if (m_TaskSystem)
			m_TaskSystem.GetOnTaskRemoved().Remove(OnTaskRemoved);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_GroupTaskManagerComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		if (!s_Instance)
			s_Instance = this;
		else
			Print("[SCR_GroupTaskManagerComponent.SCR_GroupTaskManagerComponent] instance is missing", LogLevel.WARNING);
	}
}
