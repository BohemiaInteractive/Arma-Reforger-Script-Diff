//[EntityEditorProps(category: "GameScripted/Tasks", description: "A support task.", color: "0 0 255 255")]
//class SCR_EvacuateTaskClass: SCR_RequestedTaskClass
//{
//};
//
////------------------------------------------------------------------------------------------------
//class SCR_EvacuateTask : SCR_RequestedTask
//{
//	static const string SUPPORT_TASK_DESCRIPTION_TEXT = "#AR-CampaignTasks_TitleEvac";
//	static const SCR_ECampaignBaseType BASES_FILTER = SCR_ECampaignBaseType.BASE;
//	
//	protected vector m_vStartOrigin;
//	
//	//------------------------------------------------------------------------------------------------
//	static float GetMinDistanceFromStart()
//	{
//		if (!GetTaskManager())
//			return 1000;
//		
//		SCR_EvacuateTaskSupportEntity supportEntity = SCR_EvacuateTaskSupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_EvacuateTaskSupportEntity));
//		if (supportEntity)
//			return supportEntity.GetMinDistanceFromStart();
//		
//		return 1000;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	static float GetMaxDistanceFromRequester()
//	{
//		if (!GetTaskManager())
//			return 50;
//		
//		SCR_EvacuateTaskSupportEntity supportEntity = SCR_EvacuateTaskSupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_EvacuateTaskSupportEntity));
//		if (supportEntity)
//			return supportEntity.GetMaxDistanceFromRequester();
//		
//		return 50;
//	}
//
//	//------------------------------------------------------------------------------------------------
//	void PeriodicalCheck()
//	{
//		if (!GetTaskManager())
//			return;
//		
//		SCR_BaseTaskSupportEntity supportEntity = GetTaskManager().FindSupportEntity(SCR_BaseTaskSupportEntity);
//		if (!supportEntity)
//			return;
//		
//		bool assigneeCloseEnough = true;
//		bool requesterInBase = false;
//		GenericEntity requesterEntity = GenericEntity.Cast(m_Requester.GetControlledEntity());
//		
//		if (!requesterEntity)
//		{
//			supportEntity.FailTask(this);
//			return;
//		}
//		
//		FactionAffiliationComponent factionAffiliationComponent = FactionAffiliationComponent.Cast(requesterEntity.FindComponent(FactionAffiliationComponent));
//		if (!factionAffiliationComponent)
//			return;
//		
//		Faction requesterFaction = factionAffiliationComponent.GetAffiliatedFaction();
//		
//		vector requesterOrigin = requesterEntity.GetOrigin();
//		
//		array<SCR_MilitaryBaseComponent> bases = {};
//		SCR_MilitaryBaseSystem.GetInstance().GetBases(bases);
//		
//		for (int i = 0; i < bases.Count(); i++)
//		{
//			SCR_CampaignMilitaryBaseComponent campaignBase = SCR_CampaignMilitaryBaseComponent.Cast(bases[i]);
//			
//			if (!campaignBase)
//				continue;
//			
//			if (requesterFaction != campaignBase.GetCampaignFaction())
//				continue;
//			
//			vector baseOrigin = campaignBase.GetOwner().GetOrigin();
//			float baseToStartDistance = vector.Distance(baseOrigin, m_vStartOrigin);
//			
//			if (baseToStartDistance < SCR_EvacuateTask.GetMinDistanceFromStart())
//				continue;
//			
//			if (!campaignBase.GetIsEntityPresent(requesterEntity))
//				continue;
//			
//			requesterInBase = true;
//			break;
//		}
//		
//		SCR_BaseTaskExecutor assignee = GetAssignee();
//		if (!assignee)
//			assigneeCloseEnough = false;
//		else
//		{
//			IEntity assigneeEntity = assignee.GetControlledEntity();
//			vector assigneePosition = vector.Zero;
//			if (assigneeEntity)
//				assigneePosition = assigneeEntity.GetOrigin();
//			float distance = vector.Distance(requesterOrigin, assigneePosition);
//			
//			if (distance > SCR_EvacuateTask.GetMaxDistanceFromRequester())
//				assigneeCloseEnough = false;
//		}
//		
//		if (requesterInBase)
//		{
//			if (assigneeCloseEnough)
//				supportEntity.FinishTask(this);
//			else
//				supportEntity.FailTask(this);
//		}
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void SetStartOrigin(vector startOrigin)
//	{
//		if (m_vStartOrigin != vector.Zero)
//		{
//			m_vStartOrigin = startOrigin;
//			return;
//		}
//		
//		if (!m_Requester)
//			return;
//		
//		GenericEntity requesterEntity = GenericEntity.Cast(m_Requester.GetControlledEntity());
//		if (!requesterEntity)
//			return;
//		
//		m_vStartOrigin = requesterEntity.GetOrigin();
//		
//		ShowAvailableTask();
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	vector GetStartOrigin()
//	{
//		return m_vStartOrigin;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	override bool RplSave(ScriptBitWriter writer)
//	{		
//		vector startOrigin = GetStartOrigin();
//		writer.WriteVector(startOrigin);
//		
//		return super.RplSave(writer);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	override bool RplLoad(ScriptBitReader reader)
//	{
//		vector startOrigin;
//		reader.ReadVector(startOrigin);
//		SetStartOrigin(startOrigin);
//			
//		return super.RplLoad(reader);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void SCR_EvacuateTask(IEntitySource src, IEntity parent)
//	{
//		if (SCR_Global.IsEditMode(this))
//			return;
//		
//		SetEventMask(EntityEvent.FRAME);
//		SetIndividual(true);
//		
//		if (!GetTaskManager().IsProxy())
//			SCR_BaseTaskManager.s_OnPeriodicalCheck5Second.Insert(PeriodicalCheck);
//	}
//
//	//------------------------------------------------------------------------------------------------
//	void ~SCR_EvacuateTask()
//	{
//		if (SCR_Global.IsEditMode(this) || !GetGame().GetGameMode())
//			return;
//		
//		if (!GetTaskManager().IsProxy())
//			SCR_BaseTaskManager.s_OnPeriodicalCheck5Second.Remove(PeriodicalCheck);
//	}
//
//};
