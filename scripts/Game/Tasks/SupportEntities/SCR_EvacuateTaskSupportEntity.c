//------------------------------------------------------------------------------------------------
//[Obsolete(), EntityEditorProps(category: "GameScripted/Tasks", description: "Evacuate task support entity.", color: "0 0 255 255")]
//class SCR_EvacuateTaskSupportEntityClass: SCR_RequestedTaskSupportEntityClass
//{
//};

//------------------------------------------------------------------------------------------------
//[Obsolete()]
//class SCR_EvacuateTaskSupportEntity : SCR_RequestedTaskSupportEntity
//{
//	[Attribute("#AR-CampaignTasks_RequestImpossiblePosition-UC")]
//	protected string m_sNowhereToRun;
//	
//	[Attribute("#AR-CampaignTasks_RequestImpossibleSignal-UC")]
//	protected string m_sNoSignal;
//	
//	[Attribute("1000", desc: "Minimum distance of a base from the place this task is requested from. (in m)")]
//	protected float m_fMinDistanceFromStart;
//	
//	[Attribute("50", desc: "Maximum distance of the rescuer from the evacuated player, once a rescue base is reached. (in m)")]
//	protected float m_fMaxDistanceFromRequester;
//	
//	//------------------------------------------------------------------------------------------------
//	override bool CanRequest()
//	{
//		return (super.CanRequest() && HasValidBase());
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	override void Request()
//	{
//		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
//		if (!playerController)
//			return;
//		
//		SCR_MapRadialUI ctxMenu = SCR_MapRadialUI.GetInstance();
//		if (!ctxMenu)
//			return;
//		
//		vector position = ctxMenu.GetMenuWorldPosition();
//		
//		IEntity requesterEntity = playerController.GetMainEntity();
//		if (!requesterEntity)
//			return;
//		
//		FactionAffiliationComponent factionAffiliationComponent = FactionAffiliationComponent.Cast(requesterEntity.FindComponent(FactionAffiliationComponent));
//		if (!factionAffiliationComponent)
//			return;
//		
//		Faction requesterFaction = factionAffiliationComponent.GetAffiliatedFaction();
//		if (!requesterFaction)
//			return;
//		
//		vector requesterOrigin = requesterEntity.GetOrigin();
//		array<SCR_MilitaryBaseComponent> bases = {};
//		
//		SCR_MilitaryBaseSystem.GetInstance().GetBases(bases);
//		
//		bool foundValidBase = false;
//		for (int i = bases.Count() - 1; i >= 0; i--)
//		{
//			if (requesterFaction != bases[i].GetFaction())
//				continue;
//			
//			float baseToStartDistance = vector.Distance(bases[i].GetOwner().GetOrigin(), requesterOrigin);
//			
//			if (baseToStartDistance > SCR_EvacuateTask.GetMinDistanceFromStart())
//			{
//				foundValidBase = true;
//				break;
//			}
//		}
//		
//		if (!foundValidBase)
//		{
//			SCR_PopUpNotification.GetInstance().PopupMsg(m_sNowhereToRun);
//			return;
//		}
//		
//		if (!SCR_UIRequestEvacTaskComponent.IsInRange(requesterFaction, requesterEntity))
//		{
//			SCR_PopUpNotification.GetInstance().PopupMsg(m_sNoSignal);
//			return;
//		}
//		
//		// Find task network component to send RPC to server
//		SCR_CampaignTaskNetworkComponent taskNetworkComponent = SCR_CampaignTaskNetworkComponent.Cast(playerController.FindComponent(SCR_CampaignTaskNetworkComponent));
//		if (!taskNetworkComponent)
//			return;
//		
//		taskNetworkComponent.RequestEvacuation(position);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//This should only be called on the server!
//	void RequestEvacuation(int requesterID, vector position)
//	{
//		SCR_EvacuateTask evacuateTask = SCR_EvacuateTask.Cast(CreateTask());
//		if (!evacuateTask)
//			return;
//		
//		evacuateTask.SetStartOrigin(position); // Should be enough to be set just on the server
//		
//		SCR_BaseTaskExecutor requester = SCR_BaseTaskExecutor.GetTaskExecutorByID(requesterID);
//		
//		SetRequester(evacuateTask, requester);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	float GetMinDistanceFromStart()
//	{
//		return m_fMinDistanceFromStart;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	float GetMaxDistanceFromRequester()
//	{
//		return m_fMaxDistanceFromRequester;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	bool HasValidBase()
//	{
//		IEntity requesterEntity = SCR_PlayerController.GetLocalControlledEntity();
//		if (!requesterEntity)
//			return false;
//		
//		FactionAffiliationComponent factionAffiliationComponent = FactionAffiliationComponent.Cast(requesterEntity.FindComponent(FactionAffiliationComponent));
//		if (!factionAffiliationComponent)
//			return false;
//		
//		Faction requesterFaction = factionAffiliationComponent.GetAffiliatedFaction();
//		if (!requesterFaction)
//			return false;
//		
//		bool foundValidBase = false;
//		
//		vector requesterOrigin = requesterEntity.GetOrigin();
//		array<SCR_MilitaryBaseComponent> bases = {};
//		
//		SCR_MilitaryBaseSystem.GetInstance().GetBases(bases);
//		
//		for (int i = bases.Count() - 1; i >= 0; i--)
//		{
//			if (requesterFaction != bases[i].GetFaction())
//				continue;
//			
//			float baseToStartDistance = vector.Distance(bases[i].GetOwner().GetOrigin(), requesterOrigin);
//			
//			if (baseToStartDistance > SCR_EvacuateTask.GetMinDistanceFromStart())
//			{
//				foundValidBase = true;
//				break;
//			}
//		}
//		
//		return foundValidBase;
//	}
//};