class SCR_AIActivityIllumFlareFeature: SCR_AIActivityFeatureBase
{
	static bool GetAgentIllumWeaponAndMuzzle(SCR_AIInfoComponent infoComp, out BaseWeaponComponent weaponComponent, out BaseMagazineComponent magazineComponent, out int muzzleId)
	{
		if (!infoComp)
			return false;
		
		SCR_AICombatComponent combatComp = infoComp.GetCombatComponent();
		if (!combatComp)
			return false;
		
		// Find illum mags
		EAICombatPropertiesComponentTraits traits = EAICombatPropertiesComponentTraits.BALLISTIC_ILLUMINATION;
		magazineComponent = combatComp.m_WeaponTargetSelector.FindMagazineWithTraits(traits, false);
		if (!magazineComponent)
			return false;
			
		BaseMagazineWell magWell = magazineComponent.GetMagazineWell();
			
		if (!magWell)
			return false;
			
		typename magWellType = magWell.Type();
		muzzleId = -1;
		weaponComponent = combatComp.m_WeaponTargetSelector.FindWeaponAndMuzzleForMagazineWell(magWellType, false, muzzleId);

		return weaponComponent && muzzleId != -1;
	}
	
	//-------------------------------------------------------------------------------------
	bool Execute(notnull SCR_AIGroupUtilityComponent groupUtility, vector targetPosition, SCR_AIActivityBase activity)
	{
		BaseWeaponComponent weaponComponent;
		BaseMagazineComponent magazineComponent;
		int muzzleId;
		bool agentFound;
		SCR_ChimeraAIAgent agent;
		
		foreach (SCR_AIInfoComponent infoComp: groupUtility.m_aInfoComponents)
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
		
		if (agentFound)
		{			
			AICommunicationComponent comms = groupUtility.m_Owner.GetCommunicationComponent();
			if (!comms)
				return false;
			
			// Everything is set, now we will adjust target position and send order to agent
			IEntity character = agent.GetControlledEntity();
			if (character)
			{
				// Adjust target position for UGL flares
				vector position = character.GetOrigin();
				vector direction = vector.Direction(position, targetPosition).Normalized();
				targetPosition = position + (direction * Math.RandomFloat(150, 170));
				targetPosition[1] = targetPosition[1] + Math.RandomFloat(60, 90);
			}
			
			SCR_AIMessage_FireIllumFlareAt msg = SCR_AIMessage_FireIllumFlareAt.Create(targetPosition);
			msg.m_RelatedGroupActivity = activity;
			msg.m_fPriorityLevel = SCR_AIActionBase.PRIORITY_BEHAVIOR_THROW_GRENADE;
			msg.SetReceiver(agent);
			comms.RequestBroadcast(msg, agent);
			
			return true;
		}

		return false;			
	}
}