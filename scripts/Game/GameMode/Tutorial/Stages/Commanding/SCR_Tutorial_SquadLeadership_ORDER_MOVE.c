[EntityEditorProps(insertable: false)]
class SCR_Tutorial_SquadLeadership_ORDER_MOVEClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_SquadLeadership_ORDER_MOVE : SCR_BaseTutorialStage
{
	SCR_AIGroup m_Group;
	AIWaypoint m_MoveWP;
	SCR_MapMarkerBase m_Marker;
	bool m_bRepeated;
	protected bool m_bCommented;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		m_Marker = CreateMarkerCustom("M133_Wreck", SCR_EScenarioFrameworkMarkerCustom.OBJECTIVE_MARKER, SCR_EScenarioFrameworkMarkerCustomColor.REFORGER_ORANGE);
		RegisterWaypoint("M133_Wreck", "", "MISC");
		
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
		
		PlayNarrativeCharacterStage("SQUADLEADERSHIP_Instructor", 5);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnWaypointAdded(AIWaypoint wp)
	{
		if (!m_bCommented)
		{
			m_bCommented = true;
			GetGame().GetCallqueue().CallLater(MoveComment, 6000);
		}

		IEntity wreck = GetGame().GetWorld().FindEntityByName("M133_Wreck");
		if (!wreck)
			return;
		
		if ((vector.Distance(wreck.GetOrigin(), wp.GetOrigin()) > 25) || (wp.Type() != SCR_AIWaypoint))
		{
			if (m_bRepeated)
				ShowHint(2);
			
			m_Group.GetSlave().RemoveWaypoint(wp);
			
			return;
		}

		m_bRepeated = true;
		ShowHint(1);
		m_MoveWP = wp;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void MoveComment()
	{
		PlayNarrativeCharacterStage("SQUADLEADERSHIP_Instructor", 7);
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
		return m_bFinished;
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_Tutorial_SquadLeadership_ORDER_MOVE()
	{
		DeleteMarker(m_Marker);
		GetGame().GetCallqueue().Remove(MoveComment);
	}
}