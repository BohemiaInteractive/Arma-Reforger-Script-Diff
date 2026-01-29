[EntityEditorProps(insertable: false)]
class SCR_Tutorial_SquadLeadership_FollowingOrderWaitClass : SCR_BaseTutorialStageClass
{
}

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_SquadLeadership_FollowingOrderWait : SCR_BaseTutorialStage
{
	protected ref array <IEntity> m_aSoldiers;
	
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_aSoldiers = {};
		
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!playerController)
			return;
		
		SCR_PlayerControllerGroupComponent playerGroupComp = SCR_PlayerControllerGroupComponent.Cast(playerController.FindComponent(SCR_PlayerControllerGroupComponent));
		if (!playerGroupComp)
			return;
		
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;
		
		SCR_AIGroup group = groupsManager.FindGroup(playerGroupComp.GetGroupID());
		if (!group)
			return;
		
		array <AIAgent> agents = {};
		group.GetSlave().GetAgents(agents);
		if (agents.IsEmpty())
			return;
		
		IEntity soldier;
		foreach (AIAgent agent : agents)
		{
			if (!agent)
				continue;
			
			soldier = agent.GetControlledEntity();
			if (!soldier)
				continue;
			
			m_aSoldiers.Insert(soldier); 
			RegisterWaypoint(soldier, "", "MISC");
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (!m_aSoldiers)
			return false;
		
		vector unitPos;
		vector playerPos = m_Player.GetOrigin();
		
		for (int i = m_aSoldiers.Count() - 1; i >= 0; i--)
		{
			if (!m_aSoldiers[i])
				continue;
			
			unitPos = m_aSoldiers[i].GetOrigin();
			if (vector.Distance(unitPos, playerPos) <= 10)
				m_aSoldiers.Remove(i);
		}
		
		return m_aSoldiers.IsEmpty();
	}
}