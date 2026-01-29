//class SCR_UIRequestEvacTaskComponent : ScriptedWidgetComponent
//{
//	protected static const string NO_SIGNAL = "#AR-CampaignTasks_RequestImpossibleSignal-UC";
//	
//	protected SCR_EUIRequestType m_eRequestType;
//	
//	//*********************//
//	//PUBLIC MEMBER METHODS//
//	//*********************//
//	
//	//------------------------------------------------------------------------------------------------
//	//! \param[in] requesterFaction
//	//! \param[in] requesterEntity
//	//! \return
//	static bool IsInRange(Faction requesterFaction, IEntity requesterEntity)
//	{
//		return SCR_GameModeCampaign.GetInstance().GetBaseManager().IsEntityInFactionRadioSignal(requesterEntity, requesterFaction);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//!
//	//! \param[in] playerController
//	//! \return
//	static bool HasSignal(notnull PlayerController playerController)
//	{
//		// Replace all this with better solution
//		//--------------------------------------
//		//--------------------------------------
//		SCR_PlayerController scriptedPlayerController = SCR_PlayerController.Cast(playerController);
//		
//		IEntity controlledEntity;
//		if (scriptedPlayerController)
//			controlledEntity = scriptedPlayerController.GetMainEntity();
//		else
//			controlledEntity = playerController.GetControlledEntity();
//		
//		if (!controlledEntity)
//			return false;
//		
//		FactionAffiliationComponent factionAffiliationComponent = FactionAffiliationComponent.Cast(controlledEntity.FindComponent(FactionAffiliationComponent));
//		if (!factionAffiliationComponent)
//			return false;
//		
//		Faction requesterFaction = factionAffiliationComponent.GetAffiliatedFaction();
//		if (!requesterFaction)
//			return false;
//		
//		if (!IsInRange(requesterFaction, controlledEntity))
//		{
//			SCR_PopUpNotification.GetInstance().PopupMsg(NO_SIGNAL);
//			return false;
//		}
//		
//		return true;
//		//--------------------------------------
//		//--------------------------------------
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//!
//	//! \param[in] playerController
//	static void RequestReinforcements(notnull PlayerController playerController)
//	{
//		SCR_CampaignTaskNetworkComponent taskNetworkComponent = SCR_CampaignTaskNetworkComponent.Cast(playerController.FindComponent(SCR_CampaignTaskNetworkComponent));
//		if (!taskNetworkComponent)
//			return;
//		
//		if (!HasSignal(playerController))
//			return;
//		
//		taskNetworkComponent.RequestReinforcements();
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! \param[in] type
//	void SetRequestType(SCR_EUIRequestType type)
//	{
//		m_eRequestType = type;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	override bool OnClick(Widget w, int x, int y, int button)
//	{
//		// Find local player controller
//		PlayerController playerController = GetGame().GetPlayerController();
//		if (!playerController)
//			return false;
//		
//		switch (m_eRequestType)
//		{
//			case SCR_EUIRequestType.EVAC:
//				break;
//			
//			case SCR_EUIRequestType.REFUEL:
//				break;
//			
//			case SCR_EUIRequestType.REINFORCEMENTS:
//				RequestReinforcements(playerController);
//				break;
//			
//			case SCR_EUIRequestType.TRANSPORT:
//				break;
//		}
//		
//		return false;
//	}
//}
//
//enum SCR_EUIRequestType
//{
//	EVAC,
//	REFUEL,
//	REINFORCEMENTS,
//	TRANSPORT
//}
