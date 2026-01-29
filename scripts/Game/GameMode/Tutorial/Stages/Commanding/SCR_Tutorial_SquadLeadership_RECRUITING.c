[EntityEditorProps(insertable: false)]
class SCR_Tutorial_SquadLeadership_RECRUITINGClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_SquadLeadership_RECRUITING : SCR_BaseTutorialStage
{
	protected SCR_AIGroup m_SlaveGroup;
	protected int m_iRecruitedCount;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		SCR_AIGroup group = SCR_AIGroup.Cast(GetGame().GetWorld().FindEntityByName("REQUESTING_GROUP"));
		if (!group)
			return;
		
		array <AIAgent> agents = {};
		group.GetAgents(agents);
		if (agents.IsEmpty())
			return;
		
		SCR_DamageManagerComponent damManager;
		foreach (AIAgent agent : agents)
		{
			if (!agent)
				continue;
			
			RegisterWaypoint(agent.GetControlledEntity(), "", "ADDSQUAD").SetOffsetVector("0 2 0");
			
			IEntity agentEnt = agent.GetControlledEntity();
			if (!agentEnt)
				continue;
			
			damManager = SCR_DamageManagerComponent.Cast(agentEnt.FindComponent(SCR_DamageManagerComponent));
			if (!damManager)
				continue;
			
			damManager.GetOnDamage().Insert(m_TutorialComponent.OnCharacterDamaged);
		}
		
		//TODO> Separate into function
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!playerController)
			return;
		
		SCR_PlayerControllerGroupComponent playerGroupComp = SCR_PlayerControllerGroupComponent.Cast(playerController.FindComponent(SCR_PlayerControllerGroupComponent));
		if (!playerGroupComp)
			return;
		
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;
		
		group = groupsManager.FindGroup(playerGroupComp.GetGroupID());
		if (!group)
			return;

		m_SlaveGroup = group.GetSlave();
		if (!m_SlaveGroup)
			return;
		
		m_SlaveGroup.GetOnAgentAdded().Insert(OnRecruited);
		
		PlayNarrativeCharacterStage("SQUADLEADERSHIP_Instructor", 4);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnRecruited(AIAgent child)
	{
		m_iRecruitedCount++;
		UnregisterWaypoint(child.GetControlledEntity());
		
		if (m_iRecruitedCount < 2)
			return;
		
		if (m_SlaveGroup)
			m_SlaveGroup.GetOnAgentAdded().Remove(OnRecruited);
		
		m_bFinished = true;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return false;
	}
}