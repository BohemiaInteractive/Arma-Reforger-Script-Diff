class SCR_SuppliesTransferWaypointClass : SCR_AIWaypointClass
{
}

// Base class for supply transfer waypoints (shared logic)
class SCR_SuppliesTransferWaypoint : SCR_AIWaypoint
{
	[Attribute("2000", desc: "How much supplies can transfer per operation", params: "0 inf")]
	protected float m_fTransferAmount;

	[Attribute("2", desc: "Delay between operations [s]", params: "0 inf")]
	protected float m_fTransferDelay;

	protected SCR_CampaignMilitaryBaseComponent m_Base;

	//------------------------------------------------------------------------------------------------
	//! Set the base for supply operation, if not set will auto-discover
	void SetBase(SCR_CampaignMilitaryBaseComponent base)
	{
		m_Base = base;
	}

	//------------------------------------------------------------------------------------------------
	//! \return target base
	SCR_CampaignMilitaryBaseComponent GetBase()
	{
		return m_Base;
	}

	//------------------------------------------------------------------------------------------------
	//! \return Max supplies per operation
	float GetTransferAmount()
	{
		return m_fTransferAmount;
	}

	//------------------------------------------------------------------------------------------------
	//! \return Delay between operations [s]
	float GetTransferDelay()
	{
		return m_fTransferDelay;
	}
}

// Base class for state functionality (shared logic)
class SCR_SuppliesTransferWaypointState : SCR_AIWaypointState
{
	protected const EResourceType RESOURCE_TYPE = EResourceType.SUPPLIES;

	protected SCR_ResourceComponent m_ResourceComponent;
	protected SCR_ResourceGenerator m_ResourceGenerator;
	protected SCR_ResourceConsumer m_ResourceConsumer;

	protected float m_fMaxStoredResource;
	protected float m_fCurrentResource;
	protected float m_fCurrentTransferValue;
	protected float m_fTransferAmount;

	protected SCR_CampaignMilitaryBaseComponent m_Base;

	//------------------------------------------------------------------------------------------------
	protected void InitSupplyTransfer(notnull SCR_AIGroupUtilityComponent utility, SCR_SuppliesTransferWaypoint wp, EResourceGeneratorID generatorId, EResourceGeneratorID consumerId, bool isLoad)
	{
		m_Base = wp.GetBase();
		if (!m_Base)
			m_Base = FindBase(wp.GetOrigin());

		if (!m_Base)
			return;

		SCR_ResourceComponent baseResourceComponent = m_Base.GetResourceComponent();
		if (!baseResourceComponent)
			return;

		// For load operation: DEFAULT is consumer, VEHICLE_LOAD is generator
		// For unload operation: VEHICLE_UNLOAD is consumer, DEFAULT is generator
		if (isLoad)
			m_ResourceConsumer = baseResourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, RESOURCE_TYPE);
		else
			m_ResourceGenerator = baseResourceComponent.GetGenerator(EResourceGeneratorID.DEFAULT, RESOURCE_TYPE);

		m_fTransferAmount = wp.GetTransferAmount();

		IEntity vehicleEntity = GetVehicleFromGroupLeader(utility);
		if (!vehicleEntity)
		{
			Print("The group leader is not in a vehicle.", LogLevel.WARNING);
			if (m_Utility && m_Utility.m_Owner)
				m_Utility.m_Owner.CompleteWaypoint(m_Waypoint);

			return;
		}

		m_ResourceComponent = SCR_ResourceComponent.FindResourceComponent(vehicleEntity);
		if (!m_ResourceComponent)
			return;

		if (isLoad)
			m_ResourceGenerator = m_ResourceComponent.GetGenerator(EResourceGeneratorID.VEHICLE_LOAD, RESOURCE_TYPE);
		else
			m_ResourceConsumer = m_ResourceComponent.GetConsumer(EResourceGeneratorID.VEHICLE_UNLOAD, RESOURCE_TYPE);
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_CampaignMilitaryBaseComponent FindBase(vector position)
	{
		SCR_MilitaryBaseSystem militaryBaseSystem = SCR_MilitaryBaseSystem.GetInstance();
		if (!militaryBaseSystem)
			return null;

		array<SCR_MilitaryBaseComponent> bases = {};
		militaryBaseSystem.GetBases(bases);

		SCR_CampaignMilitaryBaseComponent campaignBase;
		foreach (SCR_MilitaryBaseComponent base : bases)
		{
			campaignBase = SCR_CampaignMilitaryBaseComponent.Cast(base);
			if (!campaignBase)
				continue;

			int radius = campaignBase.GetRadius();
			if (vector.DistanceSqXZ(campaignBase.GetOwner().GetOrigin(), position) <= (radius * radius))
			{
				return campaignBase;
			}
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected IEntity GetVehicleFromGroupLeader(notnull SCR_AIGroupUtilityComponent utility)
	{
		AIAgent aiAgent = utility.GetAIAgent();
		AIGroup group = AIGroup.Cast(aiAgent);
		if (!group)
			return null;

		AIAgent leaderAgent = group.GetLeaderAgent();
		if (!leaderAgent)
			return null;

		ChimeraCharacter character = ChimeraCharacter.Cast(leaderAgent.GetControlledEntity());
		if (!character)
			return null;

		CompartmentAccessComponent compartment = character.GetCompartmentAccessComponent();
		if (!compartment)
			return null;

		return compartment.GetVehicleIn(character);
	}

	//------------------------------------------------------------------------------------------------
	protected void GetResourceValues(out float currentResource, out float maxResource, out float transferAmount)
	{
		if (!m_ResourceConsumer || !m_ResourceGenerator)
			return;

		currentResource = m_ResourceConsumer.GetAggregatedResourceValue();
		maxResource = Math.Min(m_ResourceGenerator.GetAggregatedMaxResourceValue() - m_ResourceGenerator.GetAggregatedResourceValue(), currentResource);

		if (m_fTransferAmount <= 0 || m_fTransferAmount > maxResource)
			transferAmount = maxResource;
		else
			transferAmount = m_fTransferAmount;
	}
}
