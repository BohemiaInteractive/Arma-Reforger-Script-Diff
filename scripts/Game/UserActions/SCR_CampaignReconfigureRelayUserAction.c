//! Action to reconfigure relays in Campaign
class SCR_CampaignReconfigureRelayUserAction : ScriptedUserAction
{
	// Member variables
	protected SCR_CampaignMilitaryBaseComponent m_Base;
	protected bool m_bCanBeShownResult = false;

	//------------------------------------------------------------------------------------------------
	//! Get player's faction
	//! \param player Player's entity
	static Faction GetPlayerFaction(IEntity player)
	{		
		if (!player)
			return null;
		
		if (!ChimeraCharacter.Cast(player))
			return null;
		
		auto foundComponent = ChimeraCharacter.Cast(player).FindComponent(FactionAffiliationComponent);
		Faction faction;

		if (foundComponent)
		{
			auto castedComponent = FactionAffiliationComponent.Cast(foundComponent);
			faction = castedComponent.GetAffiliatedFaction();
		};

		return faction;
	}
	
	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		if (!pOwnerEntity || !GetGame().InPlayMode())
			return;
		
		IEntity parent = pOwnerEntity.GetParent();
		
		// Register parent base
		if (parent && parent.FindComponent(SCR_CampaignMilitaryBaseComponent))
			m_Base = SCR_CampaignMilitaryBaseComponent.Cast(parent.FindComponent(SCR_CampaignMilitaryBaseComponent));
	} 
	
	//------------------------------------------------------------------------------------------------
	override void OnActionStart(IEntity pUserEntity)
	{
		if (m_Base)
			SCR_CampaignReconfigureHQRadioUserAction.ToggleBaseCaptured(m_Base, true);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionCanceled(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (m_Base)
			SCR_CampaignReconfigureHQRadioUserAction.ToggleBaseCaptured(m_Base, false);
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
		SCR_CampaignFaction playerFaction;
		bool isAI = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(user) == 0;
		
		if (isAI)
		{
			FactionAffiliationComponent comp = FactionAffiliationComponent.Cast(user.FindComponent(FactionAffiliationComponent));
			
			if (!comp)
				return false;
			
			playerFaction = SCR_CampaignFaction.Cast(comp.GetAffiliatedFaction());
		}
		else
			playerFaction = SCR_CampaignFaction.Cast(SCR_FactionManager.SGetLocalPlayerFaction());
		
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

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		return (m_Base != null);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return true;
	}
};
