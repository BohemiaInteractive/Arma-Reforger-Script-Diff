class SCR_CampaignTaskHelper
{
	//------------------------------------------------------------------------------------------------
	//! \param[in] campaignMilitaryBase
	//! \param[in] faction
	//! \param[in] taskClass
	//! \return task on base
	static SCR_Task GetTaskOnBase(notnull SCR_CampaignMilitaryBaseComponent campaignMilitaryBase, notnull Faction faction, typename taskClass)
	{
		array<SCR_Task> tasks = {};
		SCR_TaskSystem.GetInstance().GetTasksByStateFiltered(
			tasks,
			SCR_ETaskState.CREATED | SCR_ETaskState.ASSIGNED,
			faction.GetFactionKey(),
			-1,
			taskClass
		);

		SCR_CampaignMilitaryBaseTaskData data;
		SCR_CampaignMilitaryBaseComponent base;
		SCR_CampaignMilitaryBaseTaskEntity campaignMilitaryBaseTask;

		foreach (SCR_Task task : tasks)
		{
			campaignMilitaryBaseTask = SCR_CampaignMilitaryBaseTaskEntity.Cast(task);
			if (!campaignMilitaryBaseTask)
				continue;

			data = SCR_CampaignMilitaryBaseTaskData.Cast(task.GetTaskData());
			if (!data)
				continue;

			base = campaignMilitaryBaseTask.GetMilitaryBase();
			if (!base)
				continue;

			if (base == campaignMilitaryBase)
				return task;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] position
	//! \param[in] faction
	//! \param[in] taskClass
	//! \param[out] nearestTaskDistance
	//! \return nearest task
	static SCR_Task GetNearestTask(vector position, notnull Faction faction, typename taskClass, out float nearestTaskDistance)
	{
		array<SCR_Task> tasks = {};
		SCR_TaskSystem.GetInstance().GetTasksByStateFiltered(
			tasks,
			SCR_ETaskState.CREATED | SCR_ETaskState.ASSIGNED,
			faction.GetFactionKey(),
			-1,
			taskClass
		);

		nearestTaskDistance = float.MAX;
		float taskDistance;
		SCR_Task nearestTask;

		foreach (SCR_Task task : tasks)
		{
			if (!task)
				continue;

			taskDistance = vector.DistanceSqXZ(position, task.GetTaskPosition());
			if (taskDistance < nearestTaskDistance)
			{
				nearestTaskDistance = taskDistance;
				nearestTask = task;
			}
		}

		nearestTaskDistance = Math.Sqrt(nearestTaskDistance);
		return nearestTask;
	}
}
