[BaseContainerProps(), SCR_ContainerAIActionTitle()]
class SCR_ScenarioFrameworkAIActionShootFlare : SCR_ScenarioFrameworkAIAction
{
	[Attribute(desc: "Target entity should shoot at.")]
	ref SCR_ScenarioFrameworkGet m_Getter;

	[Attribute("", UIWidgets.Object, "Offset from target entity. If target entity doesn't exist, origin is shooter position and target vector is offset from his position. - use PointInfo")]
	ref PointInfo m_TargeOffset;

	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		IEntity entity;
		if (m_Getter)
		{
			SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
			if (!entityWrapper)
			{
				Print(string.Format("ScenarioFramework Action: Issue with Getter detected for Action %1.", this), LogLevel.ERROR);
				return;
			}

			entity = entityWrapper.GetValue();
			if (!entity)
			{
				Print(string.Format("ScenarioFramework Action: Entity not found for Action %1.", this), LogLevel.ERROR);
				return;
			}
		}

		SCR_AIGroupUtilityComponent groupUtility = SCR_AIGroupUtilityComponent.Cast(m_AIGroup.FindComponent(SCR_AIGroupUtilityComponent));
		if (!groupUtility)
			return;

		BaseWeaponComponent weaponComponent;
		BaseMagazineComponent magazineComponent;
		int muzzleId;
		bool agentFound;
		SCR_ChimeraAIAgent agent;

		foreach (SCR_AIInfoComponent infoComp : groupUtility.m_aInfoComponents)
		{
			agent = SCR_ChimeraAIAgent.Cast(infoComp.GetOwner());
			if (!agent)
				continue;

			// Ignore agents that are not "available", considered combat ready
			if (!IsAgentAvailable(agent))
				continue;

			agentFound = SCR_AIActivityIllumFlareFeature.GetAgentIllumWeaponAndMuzzle(infoComp, weaponComponent, magazineComponent, muzzleId);
			if (agentFound)
				break;
		}

		if (!agentFound)
			return;
		
		AICommunicationComponent comms = groupUtility.m_Owner.GetCommunicationComponent();
		if (!comms)
			return;

		IEntity character = agent.GetControlledEntity();
		if (!character)
			return;

		vector targetPosition
		if (entity)
			targetPosition = entity.GetOrigin();
		else
			targetPosition = character.GetOrigin();

		vector targetLocalOffset[4];
		if (m_TargeOffset)
			m_TargeOffset.GetLocalTransform(targetLocalOffset);

		targetPosition = targetPosition + targetLocalOffset[3];

		SCR_AIMessage_FireIllumFlareAt msg = SCR_AIMessage_FireIllumFlareAt.Create(targetPosition);
		msg.m_RelatedGroupActivity = null;
		msg.m_fPriorityLevel = SCR_AIActionBase.PRIORITY_BEHAVIOR_RETREAT_MELEE;
		msg.SetReceiver(agent);
		comms.RequestBroadcast(msg, agent);
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsAgentAvailable(notnull SCR_ChimeraAIAgent agent)
	{
		SCR_AIInfoComponent infoComp = agent.m_InfoComponent;

		return agent && infoComp.GetAIState() == EUnitAIState.AVAILABLE &&
			!infoComp.HasUnitState(EUnitState.IN_TURRET) && !infoComp.HasUnitState(EUnitState.IN_VEHICLE) &&
			!infoComp.HasUnitState(EUnitState.UNCONSCIOUS);
	}
}
