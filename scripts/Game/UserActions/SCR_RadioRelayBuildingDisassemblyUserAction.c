class SCR_RadioRelayBuildingDisassemblyUserAction : SCR_CampaignBuildingDisassemblyUserAction
{
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		if (!super.CanBePerformedScript(user))
			return false;

		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(user);
		if (!character)
			return false;

		return CanRelayBeDismantled(character);
	}

	//------------------------------------------------------------------------------------------------
	override protected void DeleteComposition(notnull IEntity composition, notnull SCR_ChimeraCharacter character)
	{
		if (!CanRelayBeDismantled(character))
			return;

		super.DeleteComposition(composition, character);
	}

	//------------------------------------------------------------------------------------------------
	protected bool CanRelayBeDismantled(notnull SCR_ChimeraCharacter dismantlingUser)
	{
		if (!m_FactionComponent)
			return true;

		Faction structureFaction = m_FactionComponent.GetAffiliatedFaction();
		if (!structureFaction)
			return false;

		if (!IsPlayerFactionSame(dismantlingUser))
			return true;

		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		if (!campaign)
			return true;

		SCR_CampaignMilitaryBaseComponent campaignBase = GetBase();
		if (!campaignBase)
			return false;

		SCR_DismantleCampaignMilitaryBaseTaskEntity task = SCR_DismantleCampaignMilitaryBaseTaskEntity.Cast(SCR_CampaignTaskHelper.GetTaskOnBase(campaignBase, structureFaction, SCR_DismantleCampaignMilitaryBaseTaskEntity));
		if (task)
			return true; // if base is meant to be dismantled then this will have to be dismantled

		SCR_CampaignBuildingManagerComponent buildingManagerComponent = SCR_CampaignBuildingManagerComponent.Cast(campaign.FindComponent(SCR_CampaignBuildingManagerComponent));
		if (!buildingManagerComponent)
			return false;

		array<SCR_CampaignBuildingCompositionComponent> compositions = {};
		buildingManagerComponent.GetBuildingCompositions(campaignBase, compositions);
		foreach (SCR_CampaignBuildingCompositionComponent composition : compositions)
		{
			if (composition && composition != m_CompositionComponent && HasCompositionLabel(composition, EEditableEntityLabel.SERVICE_ANTENNA))
				return true; // if there is more than one relay then player is allowed to dismantle this one
		}

		return false;
	}
}