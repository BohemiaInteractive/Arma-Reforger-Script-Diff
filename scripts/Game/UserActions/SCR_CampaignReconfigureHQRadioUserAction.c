//! Action to reconfigure relays in Campaign
class SCR_CampaignReconfigureHQRadioUserAction : ScriptedUserAction
{
	protected SCR_CampaignMilitaryBaseComponent m_Base;
	protected IEntity m_HQRadio;
	
	static const float MAX_BASE_DISTANCE = 50;
	
	//------------------------------------------------------------------------------------------------
	protected bool IsParentBase(IEntity ent)
	{
		m_Base = SCR_CampaignMilitaryBaseComponent.Cast(ent.FindComponent(SCR_CampaignMilitaryBaseComponent));

		// Base was found, stop query
		if (m_Base)
			return false;
		
		// Keep looking
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CheckParentBase(vector origin)
	{
		if (!m_Base)
			Print("HQ radio at " + string.ToString(origin) + " is not close enough (" + MAX_BASE_DISTANCE + "m) to any Conflict base!", LogLevel.ERROR);
	}
	
	//------------------------------------------------------------------------------------------------
	static void ToggleBaseCaptured(SCR_CampaignMilitaryBaseComponent base, bool isBeingCaptured)
	{
		// Find local player controller
		PlayerController playerController = GetGame().GetPlayerController();
		
		if (!playerController)
			return;
		
		// Find conflict network component to send RPC to server
		SCR_CampaignNetworkComponent campaignNetworkComponent = SCR_CampaignNetworkComponent.Cast(playerController.FindComponent(SCR_CampaignNetworkComponent));

		if (campaignNetworkComponent)
			campaignNetworkComponent.ToggleBaseCapture(base, isBeingCaptured);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		if (!pOwnerEntity)
			return;
		
		m_HQRadio = pOwnerEntity;
		
		if (!GetGame().InPlayMode())
			return;
		
		BaseWorld world = GetGame().GetWorld();
		
		if (!world)
			return;
		
		vector origin = pOwnerEntity.GetOrigin();
		
		// Register parent base - find one in the nearest vicinity
		world.QueryEntitiesBySphere(origin, MAX_BASE_DISTANCE, IsParentBase, null, EQueryEntitiesFlags.ALL);
		
		// Throw an error if no base has been found in the vicinity
		GetGame().GetCallqueue().CallLater(CheckParentBase, 1000, false, origin);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionStart(IEntity pUserEntity)
	{
		bool isAI = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(pUserEntity) == 0;
		
		if (!isAI && m_Base)
			ToggleBaseCaptured(m_Base, true);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionCanceled(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		ToggleBaseCaptured(m_Base, false);
	}
	
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{	
		if (!m_Base)
			return;	
		
		bool isAI = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(pUserEntity) == 0;
		
		if (isAI)
		{
			FactionAffiliationComponent comp = FactionAffiliationComponent.Cast(pUserEntity.FindComponent(FactionAffiliationComponent));
			
			if (!comp)
				return;
			
			m_Base.SetFaction(SCR_CampaignFaction.Cast(comp.GetAffiliatedFaction()));
			
			return;
		}
		else
		{
			PlayerController playerController = GetGame().GetPlayerController();
			
			if (!playerController)
				return;
			
			SCR_CampaignNetworkComponent campaignNetworkComponent = SCR_CampaignNetworkComponent.Cast(playerController.FindComponent(SCR_CampaignNetworkComponent));
	
			if (campaignNetworkComponent)
				campaignNetworkComponent.CaptureBase(m_Base);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		bool isAI = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(user) == 0;
		
		if (isAI)
		{
			FactionAffiliationComponent comp = FactionAffiliationComponent.Cast(user.FindComponent(FactionAffiliationComponent));
			
			if (!comp)
				return false;
			
			return (comp.GetAffiliatedFaction() != m_Base.GetFaction());
		}
		else
		{
			SCR_CampaignFaction playerFaction = SCR_CampaignFaction.Cast(SCR_FactionManager.SGetLocalPlayerFaction());
			
			if (!playerFaction)
				return false;
			
			// Already ours
			if (m_Base.GetFaction() == playerFaction)
			{
				SetCannotPerformReason("#AR-Campaign_Action_Done");
				return false;
			}
			
			// No radio signal
			if (m_Base != playerFaction.GetMainBase() && !m_Base.IsHQRadioTrafficPossible(playerFaction))
			{
				SetCannotPerformReason("#AR-Campaign_Action_NoSignal");
				return false;
			}
			
			return true;
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		CharacterControllerComponent comp = CharacterControllerComponent.Cast(user.FindComponent(CharacterControllerComponent));
		
		if (!comp)
			return false;
		
		if (comp.IsDead())
			return false;
		
		return (m_Base != null);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return true;
	}
};
