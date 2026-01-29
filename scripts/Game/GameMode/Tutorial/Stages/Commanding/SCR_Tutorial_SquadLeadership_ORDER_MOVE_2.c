[EntityEditorProps(insertable: false)]
class SCR_Tutorial_SquadLeadership_ORDER_MOVE_2Class: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_SquadLeadership_ORDER_MOVE_2 : SCR_BaseTutorialStage
{
	SCR_AIGroup m_Group;
	AIWaypoint m_MoveWP;
	protected bool m_bCommented;
	protected bool m_bDefendOrderIssued;
	
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!playerController)
			return;
		
		SCR_PlayerControllerGroupComponent playerGroupComp = SCR_PlayerControllerGroupComponent.Cast(playerController.FindComponent(SCR_PlayerControllerGroupComponent));
		if (!playerGroupComp)
			return;
		
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;
		
		m_Group = groupsManager.FindGroup(playerGroupComp.GetGroupID());
		if (!m_Group)
			return;
		
		m_Group.GetSlave().GetOnWaypointAdded().Insert(OnWaypointAdded);
		m_Group.GetSlave().GetOnWaypointCompleted().Insert(OnWaypointCompleted);
		
		PlayNarrativeCharacterStage("SQUADLEADERSHIP_Instructor", 14);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnWaypointAdded(AIWaypoint wp)
	{
		if (!m_bCommented)
		{
			m_bCommented = true;
			GetGame().GetCallqueue().CallLater(MoveComment, 3000);
		}
		
		IEntity crossroad = GetGame().GetWorld().FindEntityByName("WP_SquadLeadership_CROSSROAD");
		if (!crossroad)
			return;
		
		if ((vector.Distance(crossroad.GetOrigin(), wp.GetOrigin()) > 25) || (wp.Type() != SCR_AIWaypoint))
		{
			ShowHint(1);
			m_Group.GetSlave().RemoveWaypoint(wp);
			SCR_EntityHelper.DeleteEntityAndChildren(wp);
			return;
		}
		
		m_MoveWP = wp;
		
		//Allign WP to most suitable position. Player shouldn't notice this.
		wp.SetOrigin(crossroad.GetOrigin());
		ShowHint(2);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void MoveComment()
	{
		PlayNarrativeCharacterStage("SQUADLEADERSHIP_Instructor", 15);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnWaypointCompleted(AIWaypoint wp)
	{
		if (wp != m_MoveWP)
			return;
		
		m_Group.GetSlave().GetOnWaypointAdded().Remove(OnWaypointAdded);
		m_Group.GetSlave().GetOnWaypointCompleted().Remove(OnWaypointCompleted);
		
		m_bFinished = true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		if (!m_bDefendOrderIssued && vector.Distance(m_Player.GetOrigin(), GetGame().GetWorld().FindEntityByName("WP_SquadLeadership_CROSSROAD").GetOrigin()) < 150)
		{
			m_bDefendOrderIssued = true;
			SCR_VoiceoverSystem.GetInstance().PlaySequence("AtCheckpoint");
		}
		
		return m_bFinished;
	}
}