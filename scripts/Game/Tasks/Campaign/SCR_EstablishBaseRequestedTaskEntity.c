class SCR_EstablishBaseRequestedTaskEntityClass : SCR_BaseRequestedTaskEntityClass
{
}

class SCR_EstablishBaseRequestedTaskEntity : SCR_BaseRequestedTaskEntity
{
	protected SCR_GroupTaskManagerComponent m_GroupTaskManager;

	//------------------------------------------------------------------------------------------------
	protected void OnGroupTaskSet(SCR_Task task, int byPlayerID)
	{
		VerifyTaskExistence(task);
	}

	//------------------------------------------------------------------------------------------------
	protected void VerifyTaskExistence(SCR_Task task)
	{
		SCR_CampaignFactionCommanderHandlerComponent handlerComponent	= SCR_CampaignFactionCommanderHandlerComponent.Cast(SCR_FactionCommanderHandlerComponent.GetInstance());
		if (!handlerComponent)
			return;

		int threshold = handlerComponent.GetBaseMinDistance(SCR_ECampaignBaseType.BASE);

		SCR_EstablishBaseTaskEntity establishTask = SCR_EstablishBaseTaskEntity.Cast(task);
		if (!establishTask)
			return;

		bool isAnyFactionMatch = false;
		foreach (string factionKey : m_TaskData.m_aOwnerFactionKeys)
		{
			if (factionKey.IsEmpty())
				continue;

			if (establishTask.GetTaskData().m_aOwnerFactionKeys.Contains(factionKey))
			{
				isAnyFactionMatch = true;
				break;
			}
		}

		if (!isAnyFactionMatch)
			return;

		if (vector.DistanceSqXZ(establishTask.GetOrigin(), GetOrigin()) > threshold * threshold)
			return;

		// clear establish requests in range
		SetTaskState(SCR_ETaskState.COMPLETED);
		DeleteTask();
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		RplComponent rpl = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (!rpl || rpl.IsProxy())
			return;

		m_GroupTaskManager = SCR_GroupTaskManagerComponent.GetInstance();

		if (m_GroupTaskManager)
			m_GroupTaskManager.GetOnGroupTaskSet().Insert(OnGroupTaskSet);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_EstablishBaseRequestedTaskEntity()
	{
		if (!m_TaskSystem)
			return;

		if (m_GroupTaskManager)
			m_GroupTaskManager.GetOnGroupTaskSet().Remove(OnGroupTaskSet);
	}
}
