void TaskRequesterIdInvokerDelegate(SCR_BaseRequestedTaskEntity task, int requesterId);
typedef func TaskRequesterIdInvokerDelegate;
typedef ScriptInvokerBase<TaskRequesterIdInvokerDelegate> SCR_TaskRequesterIdInvoker;

class SCR_BaseRequestedTaskEntityClass : SCR_TaskClass
{
}

class SCR_BaseRequestedTaskEntity : SCR_Task
{
	static const int REQUEST_MAX_COUNT = 2;
	static const int INVALID_REQUESTER_ID = -1;

	protected SCR_BaseRequestedTaskData m_BaseRequestedTaskData;
	protected ref SCR_TaskRequesterIdInvoker m_OnRequesterIdChanged;

	//------------------------------------------------------------------------------------------------
	SCR_TaskRequesterIdInvoker GetOnRequesterIdChanged()
	{
		if (!m_OnRequesterIdChanged)
			m_OnRequesterIdChanged = new SCR_TaskRequesterIdInvoker();

		return m_OnRequesterIdChanged;
	}

	//------------------------------------------------------------------------------------------------
	void SetTaskRequesterId(int groupId)
	{
		Rpc(RpcDo_SetTaskRequesterId, groupId);
		RpcDo_SetTaskRequesterId(groupId);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_SetTaskRequesterId(int groupId)
	{
		OnSetRequesterId(groupId);

		if (m_OnDisplayDataChanged)
			m_OnDisplayDataChanged.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSetRequesterId(int requesterId)
	{
		if (!m_BaseRequestedTaskData || requesterId == INVALID_REQUESTER_ID || m_BaseRequestedTaskData.m_iRequesterId == requesterId)
			return;

		m_BaseRequestedTaskData.m_iRequesterId = requesterId;

		SCR_RequestedTaskUIInfo taskUIInfo = SCR_RequestedTaskUIInfo.Cast(m_BaseRequestedTaskData.m_UIInfo);
		if (taskUIInfo)
			taskUIInfo.SetRequesterGroupId(requesterId);

		if (m_OnRequesterIdChanged)
			m_OnRequesterIdChanged.Invoke(this, requesterId);
	}

	//------------------------------------------------------------------------------------------------
	override protected void ReadUIInfo(ScriptBitReader reader, out SCR_TaskUIInfo info)
	{
		super.ReadUIInfo(reader, info);

		array<LocalizedString> nameParams;
		array<LocalizedString> descParams;
		info.GetUnformattedName(nameParams);
		info.GetUnformattedDescription(descParams);

		info = SCR_RequestedTaskUIInfo.CreateRequestedTaskUIInfo(info.GetName(), nameParams, info.GetDescription(), descParams, info.GetImageSetPath(), info.GetIconSetName());
	}

	//------------------------------------------------------------------------------------------------
	//! \return if the local player can cancel the request
	bool CanBeCanceledByLocalPlayer()
	{
		// commander can cancel all requests
		if (SCR_FactionCommanderPlayerComponent.IsLocalPlayerCommander())
			return true;

		SCR_PlayerControllerGroupComponent playerControllerGroupComponent = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!playerControllerGroupComponent)
			return false;

		// group leader can't cancel own request if a group is already assigned
		if (m_BaseRequestedTaskData.m_aAssignees.Count() > 0 ||
			!RequestedByLocalPlayerGroup() ||
			!playerControllerGroupComponent.IsPlayerLeaderOwnGroup())
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] playerId
	//! \return true if the player is from requester group
	bool IsPlayerFromRequesterGroup(int playerId)
	{
		SCR_GroupsManagerComponent comp = SCR_GroupsManagerComponent.GetInstance();
		if (!comp)
			return false;

		SCR_AIGroup playerGroup = comp.GetPlayerGroup(playerId);
		if (!playerGroup)
			return false;

		if (!m_BaseRequestedTaskData)
			return false;

		return m_BaseRequestedTaskData.m_iRequesterId == playerGroup.GetGroupID();
	}

	//------------------------------------------------------------------------------------------------
	//! \return true if the local player is from requester group
	bool RequestedByLocalPlayerGroup()
	{
		return IsPlayerFromRequesterGroup(SCR_PlayerController.GetLocalPlayerId());
	}

	//------------------------------------------------------------------------------------------------
	//! \return requester Id
	int GetRequesterId()
	{
		return m_BaseRequestedTaskData.m_iRequesterId;
	}

	//------------------------------------------------------------------------------------------------
	protected void DeleteTask()
	{
		if (!m_TaskSystem)
			return;

		m_TaskSystem.DeleteTask(this);
	}

	//------------------------------------------------------------------------------------------------
	override bool RplSave(ScriptBitWriter writer)
	{
		if (!super.RplSave(writer))
			return false;

		if (m_BaseRequestedTaskData)
			writer.WriteInt(m_BaseRequestedTaskData.m_iRequesterId);
		else
			writer.WriteInt(INVALID_REQUESTER_ID);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		if (!super.RplLoad(reader))
			return false;

		if (!m_BaseRequestedTaskData)
			m_BaseRequestedTaskData = SCR_BaseRequestedTaskData.Cast(m_TaskData);

		int requesterId;
		reader.ReadInt(requesterId);
		RpcDo_SetTaskRequesterId(requesterId);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void InitializeData()
	{
		if (!m_TaskData)
			m_TaskData = new SCR_BaseRequestedTaskData();

		super.InitializeData();

		m_BaseRequestedTaskData = SCR_BaseRequestedTaskData.Cast(m_TaskData);
	}
}
