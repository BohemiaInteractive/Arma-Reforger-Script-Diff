class SCR_FactionBaseResourceComponentClass : SCR_MilitaryBaseLogicComponentClass
{
}

class SCR_FactionBaseResourceComponent : SCR_MilitaryBaseLogicComponent
{
	protected FactionAffiliationComponent m_FactionAffiliationComp;

	//------------------------------------------------------------------------------------------------
	Faction GetFaction()
	{
		if (!m_FactionAffiliationComp)
			return null;

		Faction faction = m_FactionAffiliationComp.GetAffiliatedFaction();
		if (faction)
			return faction;

		return m_FactionAffiliationComp.GetDefaultAffiliatedFaction();
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnBaseFactionChanged(Faction faction)
	{
		if (!m_FactionAffiliationComp || !faction)
			return;

		if (m_FactionAffiliationComp.GetAffiliatedFaction() == faction)
			return;

		m_FactionAffiliationComp.SetAffiliatedFaction(faction);
	}

	//------------------------------------------------------------------------------------------------
	protected void RegisterBaseLogic()
	{
		if (!m_FactionAffiliationComp)
			return;

		SCR_MilitaryBaseSystem baseManager = SCR_MilitaryBaseSystem.GetInstance();
		if (!baseManager)
			return;

		baseManager.RegisterLogicComponent(this);
	}

	//------------------------------------------------------------------------------------------------
	protected void FetchFactionAffiliation()
	{
		IEntity owner = GetOwner();
		IEntity parent = owner.GetParent();
		while (parent && !m_FactionAffiliationComp)
		{
			m_FactionAffiliationComp = FactionAffiliationComponent.Cast(parent.FindComponent(FactionAffiliationComponent));
			if (m_FactionAffiliationComp)
				owner = parent;

			parent = parent.GetParent();
		}

		if (!m_FactionAffiliationComp)
		{
			m_FactionAffiliationComp = FactionAffiliationComponent.Cast(owner.FindComponent(FactionAffiliationComponent));
			if (!m_FactionAffiliationComp)
				return;
		}

		// In conflict we have to discard bases which come from the command post composition, as they are not used
		bool isCampaign = SCR_GameModeCampaign.Cast(GetGame().GetGameMode()) != null;

		vector ownerPos = owner.GetOrigin();
		float closestDistance = float.MAX;
		SCR_MilitaryBaseComponent closestBase;
		SCR_CampaignMilitaryBaseComponent campaignBase;
		foreach (SCR_MilitaryBaseComponent base : m_aBases)
		{
			if (base.GetOwner() == owner)
				return;

			campaignBase = SCR_CampaignMilitaryBaseComponent.Cast(base);
			if (isCampaign && !campaignBase)
				continue;

			if (campaignBase && campaignBase.GetType() != SCR_ECampaignBaseType.BASE)
				continue;

			float distance = vector.DistanceSqXZ(base.GetOwner().GetOrigin(), ownerPos);
			if (distance < closestDistance)
			{
				closestDistance = distance;
				closestBase = base;
			}
		}

		if (closestBase)
			OnBaseFactionChanged(closestBase.GetFaction());
	}

	//------------------------------------------------------------------------------------------------
	override protected void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		SCR_EditorLinkComponent linkComponent = SCR_EditorLinkComponent.Cast(SCR_EntityHelper.GetMainParent(GetOwner(), true).FindComponent(SCR_EditorLinkComponent));
		if (linkComponent)
		{
			linkComponent.GetOnLinkedEntitiesSpawned().Insert(FetchFactionAffiliation);
			linkComponent.GetOnLinkedEntitiesSpawned().Insert(RegisterBaseLogic);
			return;
		}

		FetchFactionAffiliation();
	}
}