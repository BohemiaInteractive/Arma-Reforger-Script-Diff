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
		if (!m_Base)
			return;

		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(pUserEntity);
		if (!character)
			return;

		ToggleBaseCapture(character, m_Base, true);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionCanceled(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!m_Base)
			return;

		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(pUserEntity);
		if (!character)
			return;

		ToggleBaseCapture(character, m_Base, false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ToggleBaseCapture(notnull SCR_ChimeraCharacter character, notnull SCR_CampaignMilitaryBaseComponent base, bool isBeingCaptured)
	{
		if (character.GetRplComponent().Role() != RplRole.Authority)
			return;

		SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(character.GetFaction());
		if (!faction)
			return;

		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(character);

		if (isBeingCaptured)
			base.BeginCapture(faction, playerId);
		else
			base.EndCapture();
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{
		if (!m_Base)
			return;	

		if (!CanBePerformedScript(pUserEntity))
			return; // authority needs to make sure that user is able to actually do this

		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(pUserEntity);
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(pUserEntity);
		SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(character.GetFaction());

		m_Base.CaptureRelay(faction, playerId);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(user);
		if (!character)
			return false;

		SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(character.GetFaction());
		if (!faction)
			return false;
		
		// Already ours
		if (m_Base.GetFaction() == faction)
		{
			SetCannotPerformReason("#AR-Campaign_Action_Done");
			return false;
		}
		
		// No radio signal
		if (m_Base != faction.GetMainBase() && !m_Base.IsHQRadioTrafficPossible(faction))
		{
			SetCannotPerformReason("#AR-Campaign_Action_NoSignal");
			return false;
		}
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		return m_Base != null;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBroadcastScript()
	{
		return false;
	}
}
