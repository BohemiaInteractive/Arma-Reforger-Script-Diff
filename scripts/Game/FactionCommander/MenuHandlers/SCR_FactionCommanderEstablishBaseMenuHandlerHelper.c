[BaseContainerProps()]
class SCR_FactionCommanderEstablishBaseMenuHandlerHelper
{
	protected const static string BASE_TOO_CLOSE_TEXT = "#AR-FactionCommander_BaseTooClose";
	protected const static string TASK_TOO_CLOSE_TEXT = "#AR-FactionCommander_TaskTooClose";

	//------------------------------------------------------------------------------------------------
	static bool IsNearAnyBase(notnull Faction commanderFaction, vector position)
	{
		array<SCR_MilitaryBaseComponent> bases = {};
		SCR_MilitaryBaseSystem.GetInstance().GetBases(bases);

		SCR_CampaignMilitaryBaseComponent campaignBase;
		int threshold;

		foreach (SCR_MilitaryBaseComponent base : bases)
		{
			campaignBase = SCR_CampaignMilitaryBaseComponent.Cast(base);

			if (!campaignBase || !campaignBase.IsInitialized())
				continue;

			threshold = SCR_CampaignFactionCommanderHandlerComponent.Cast(SCR_FactionCommanderHandlerComponent.GetInstance()).GetBaseMinDistance(campaignBase);

			if (vector.DistanceSqXZ(base.GetOwner().GetOrigin(), position) < (threshold * threshold))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	static bool IsNearAnyEstablishTask(notnull Faction commanderFaction, vector position)
	{
		array<SCR_Task> tasks = {};
		SCR_TaskSystem.GetInstance().GetTasksByState(tasks, SCR_ETaskState.CREATED | SCR_ETaskState.ASSIGNED, commanderFaction.GetFactionKey());

		int threshold = SCR_CampaignFactionCommanderHandlerComponent.Cast(SCR_FactionCommanderHandlerComponent.GetInstance()).GetBaseMinDistance(SCR_ECampaignBaseType.BASE);

		foreach (SCR_Task task : tasks)
		{
			SCR_EstablishBaseTaskEntity establishTask = SCR_EstablishBaseTaskEntity.Cast(task);

			if (!establishTask)
				continue;

			if (vector.DistanceSqXZ(establishTask.GetOrigin(), position) < (threshold * threshold))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	static bool CanEstablishBase(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity, out string disabledText = "")
	{
		SCR_GameModeCampaign gameModeCampaign = SCR_GameModeCampaign.Cast(GetGame().GetGameMode());
		if (!gameModeCampaign || !gameModeCampaign.GetEstablishingBasesEnabled())
			return false;

		SCR_EstablishBaseTaskEntity hoveredTask = SCR_EstablishBaseTaskEntity.Cast(hoveredEntity);
		if (hoveredTask)
			return true;

		if (IsNearAnyBase(commanderFaction, position))
		{
			disabledText = BASE_TOO_CLOSE_TEXT;
			return false;
		}

		if (IsNearAnyEstablishTask(commanderFaction, position))
		{
			disabledText = TASK_TOO_CLOSE_TEXT;
			return false;
		}

		return true;
	}
}
