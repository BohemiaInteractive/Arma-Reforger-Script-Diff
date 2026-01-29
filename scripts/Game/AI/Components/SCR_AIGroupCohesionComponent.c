[ComponentEditorProps(category: "GameScripted/AI", description: "Component for checking group cohesion and giving awards accordingly")]
class SCR_AIGroupCohesionComponentClass : ScriptComponentClass
{
}

class SCR_AIGroupCohesionComponent : ScriptComponent
{
	[Attribute("90", desc: "Time at which Group Cohesion will be evaluated in seconds")]
	protected float m_fGroupCohesionCheck;

	[Attribute("100", desc: "Distance between players at which they are considered part of the same group")]
	protected float m_fGroupCohesionRadius;

	[Attribute("2", desc: "XP increment per player on the group")]
	protected float m_fGroupCohesionXpIncreasePerPlayer;
	
	[Attribute("3", desc: "Amount of times the XP awarded will be increased in every check where the group is together")]
	protected int m_iMaxGroupCohesionLevel;

	[Attribute("false", desc: "Activate/Deactivate debug draws with information about cohesion level and XP awarded")]
	protected bool m_bDebugActive;

	protected int m_iGroupCohesionLevel = 0;

	protected SCR_AIGroup m_Group;

	// Debug properties
	private int m_iLastAwardedXP = 0;
	private int m_iLastGroupCount = 0;

	//------------------------------------------------------------------------------------------------
	protected float GetGroupCohesionCheckTimeInMs()
	{
		return m_fGroupCohesionCheck * 1000;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnPostInit(IEntity owner)
	{
		RplComponent rplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));

		m_Group = SCR_AIGroup.Cast(owner);
		
		if (!m_Group)
			return;
		
		if (!rplComponent || !rplComponent.IsMaster())
			return;
		
		m_Group.GetOnPlayerAdded().Insert(OnPlayerAddedToGroup);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlayerAddedToGroup(SCR_AIGroup aiGroup, int playerID)
	{
		if (m_Group != aiGroup || aiGroup.GetPlayerCount() < 2)
			return;

		GetGame().GetCallqueue().CallLater(CheckGroupCohesion, GetGroupCohesionCheckTimeInMs(), true);

		if (m_bDebugActive)
			GetGame().GetCallqueue().CallLater(DrawDebug, 0, true);

		aiGroup.GetOnPlayerAdded().Remove(OnPlayerAddedToGroup);
		aiGroup.GetOnPlayerRemoved().Insert(OnPlayerRemovedFromGroup);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlayerRemovedFromGroup(SCR_AIGroup aiGroup, int playerID)
	{
		if (m_Group != aiGroup || aiGroup.GetPlayerCount() >= 2)
			return;

		GetGame().GetCallqueue().Remove(CheckGroupCohesion);

		if (m_bDebugActive)
			GetGame().GetCallqueue().Remove(DrawDebug);

		aiGroup.GetOnPlayerAdded().Insert(OnPlayerAddedToGroup);
		aiGroup.GetOnPlayerRemoved().Remove(OnPlayerRemovedFromGroup);
	}

	//------------------------------------------------------------------------------------------------
	protected void DrawDebug()
	{
		DbgUI.Begin("Cohesion Debug");
		DbgUI.Text("Cohesion Time: " + m_iGroupCohesionLevel * m_fGroupCohesionCheck + "s");
		DbgUI.Text("Last Player Count on Group: " + m_iLastGroupCount + "/" + m_Group.GetPlayerCount());
		DbgUI.Text("Last awarded XP: " + m_iLastAwardedXP + "xp");
		DbgUI.End();
	}

	//------------------------------------------------------------------------------------------------
	protected void CreateAgentDistanceGraph(out array< ref array<int>> playerGraph, array<int> players, int playerCount)
	{
		playerGraph.Reserve(playerCount);

		for (int i = 0; i < playerCount; ++i)
		{
			playerGraph.Insert({});
		}
		
		float cohesionRadiusSq = m_fGroupCohesionRadius * m_fGroupCohesionRadius;
		
		for (int player1Index = 0; player1Index < playerCount - 1; ++player1Index)
		{
			IEntity player1 = GetGame().GetPlayerManager().GetPlayerControlledEntity(players[player1Index]);
			if (!player1)
				continue;

			for (int player2Index = 0; player2Index < playerCount; ++player2Index)
			{
				if (player1Index == player2Index)
					continue;

				IEntity player2 = GetGame().GetPlayerManager().GetPlayerControlledEntity(players[player2Index]);
				if (!player2)
					continue;

				bool areInRange = (vector.DistanceSqXZ(player1.GetOrigin(), player2.GetOrigin()) <= cohesionRadiusSq);

				if (!areInRange)
					continue;

				playerGraph[player1Index].Insert(player2Index);
				playerGraph[player2Index].Insert(player1Index);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	private void FindUnusedAdjacentAgentGraph(int agent, array< ref array<int>> playerGraph, out array<int> group, out array<int> usedAgents)
	{
		usedAgents.Insert(agent);
		group.Insert(agent);

		array<int> currentAgentGraph = playerGraph[agent];

		foreach (int adjacentAgent : currentAgentGraph)
		{
			if (usedAgents.Find(adjacentAgent) < 0)
				FindUnusedAdjacentAgentGraph(adjacentAgent, playerGraph, group, usedAgents);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void FindGroupFromAgentDistanceGraph(out array<int> biggestGroup, const array< ref array<int>> playerGraph, int playerCount, int groupMinSize)
	{
		array<int> usedAgents = {};
		usedAgents.Reserve(playerCount);

		for (int agentToCheck = 0; agentToCheck < playerCount - 1; ++agentToCheck)
		{
			array<int> group = {};

			FindUnusedAdjacentAgentGraph(agentToCheck, playerGraph, group, usedAgents);

			if (group.Count() >= groupMinSize)
			{
				biggestGroup = group;
				return;
			}

			if (group.Count() > biggestGroup.Count())
				biggestGroup = group;

			if (biggestGroup.Count() > playerCount - usedAgents.Count())
				return;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool GetGroupOfPlayersInCohesion(out notnull array<int> playerGroup, out notnull array<int> players, out int groupMinSize)
	{
		players = m_Group.GetPlayerIDs();
		int playerCount = players.Count();

		if (playerCount <= 1)
			return false;

		groupMinSize = (playerCount / 2) + 1;

		array<ref array<int>> playerGraph = {};
		CreateAgentDistanceGraph(playerGraph, players, playerCount);

		FindGroupFromAgentDistanceGraph(playerGroup, playerGraph, playerCount, groupMinSize);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void AwardXPToGroup(array<int> playerGroup, array<int> players, int groupMinSize)
	{
		if (playerGroup.Count() >= groupMinSize)
		{
			SCR_XPHandlerComponent compXP = SCR_XPHandlerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_XPHandlerComponent));

			m_iGroupCohesionLevel = Math.ClampInt(m_iGroupCohesionLevel + 1, 0, m_iMaxGroupCohesionLevel);

			int XPToAward = m_fGroupCohesionXpIncreasePerPlayer * (playerGroup.Count()) * m_iGroupCohesionLevel;

			m_iLastAwardedXP = XPToAward;

			foreach (int playerIndex : playerGroup)
			{
				compXP.AwardXP(players[playerIndex], SCR_EXPRewards.GROUP_COHESION, 1.0, false, XPToAward);
			}
		}
		else
		{
			m_iGroupCohesionLevel = 0;
			m_iLastAwardedXP = 0;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void CheckGroupCohesion()
	{
		array<int> playerGroup = {};
		array<int> players = {};
		int groupMinSize;
		
		if (GetGroupOfPlayersInCohesion(playerGroup, players, groupMinSize))
			AwardXPToGroup(playerGroup, players, groupMinSize);

		m_iLastGroupCount = playerGroup.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	int GetPlayersInCohesion(out notnull array<int> playersInCohesion)
	{
		array<int> playerGroup = {};
		array<int> players = {};
		int groupMinSize;
		
		if (!GetGroupOfPlayersInCohesion(playerGroup, players, groupMinSize))
			return 0;
		
		if (playerGroup.Count() < groupMinSize)
			return 0;
		
		foreach (int playerIndex : playerGroup)
		{
			playersInCohesion.Insert(players[playerIndex]);
		}
		
		return playersInCohesion.Count();
	}

}
