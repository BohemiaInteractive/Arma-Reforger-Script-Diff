class SCR_PickupRequestedTaskEntityClass : SCR_BaseRequestedTaskEntityClass
{
}

class SCR_PickupRequestedTaskEntity : SCR_BaseRequestedTaskEntity
{
	[Attribute("150", UIWidgets.EditBox, "Area radius [m]", "0 inf")]
	protected float m_fAreaRadius;

	protected float m_fAreaRadiusSq;
	protected RplComponent m_RplComponent;
	protected SCR_GroupsManagerComponent m_GroupsManager;
	protected ref array<int> m_aFilteredRequesters = {};
	protected int m_iRequestersInVehicle;

	static int PERIODICAL_CHECK_INTERVAL = 2000; // ms

	//------------------------------------------------------------------------------------------------
	protected void PeriodicalCheck()
	{
		if (!m_GroupsManager)
			return;

		SCR_AIGroup group = m_GroupsManager.FindGroup(GetRequesterId());
		if (!group)
			return;

		array<int> assigneePlayerIDs = GetTaskAssigneePlayerIDs();
		if (assigneePlayerIDs && !assigneePlayerIDs.IsEmpty() && !IsAnyPlayerAlive(assigneePlayerIDs))
		{
			// fail - all assigned are dead
			m_TaskSystem.SetTaskState(this, SCR_ETaskState.FAILED);
			DeleteTask();
			return;
		}

		PlayerManager playerManager = GetGame().GetPlayerManager();
		array<int> requesterIDs = group.GetPlayerIDs();
		SCR_ChimeraCharacter requesterCharacter;
		SCR_ChimeraCharacter assigneeCharacter;
		vector taskPosition = GetTaskPosition();
		bool isAnyRequesterAlive;

		m_iRequestersInVehicle = 0;

		foreach (int requesterID : requesterIDs)
		{
			requesterCharacter = GetCharacter(requesterID);
			if (!requesterCharacter)
				continue;

			isAnyRequesterAlive = true;

			if (!requesterCharacter.IsInVehicle())
				continue;

			IEntity requesterVehicle = GetVehicle(requesterCharacter);
			if (!requesterVehicle)
				continue;

			// check if a requester is inside of the area
			if (vector.DistanceSqXZ(requesterCharacter.GetOrigin(), taskPosition) <= m_fAreaRadiusSq)
			{
				if (!m_aFilteredRequesters.Contains(requesterID))
					m_aFilteredRequesters.Insert(requesterID);

				continue;
			}

			// we need the requester to be in the area at least once.
			if (!m_aFilteredRequesters.Contains(requesterID))
				continue;

			foreach (int assigneePlayerID : assigneePlayerIDs)
			{
				assigneeCharacter = GetCharacter(assigneePlayerID);
				if (!assigneeCharacter || !assigneeCharacter.IsInVehicle())
					continue;

				IEntity assigneeVehicle = GetVehicle(assigneeCharacter);

				// check if requester is in vehicle with assignee
				if (assigneeVehicle == requesterVehicle)
				{
					m_iRequestersInVehicle++;
					break;
				}
			}
		}

		if (!isAnyRequesterAlive)
		{
			// fail - all requesters are dead
			m_TaskSystem.SetTaskState(this, SCR_ETaskState.FAILED);
			DeleteTask();
		}
		else if (m_iRequestersInVehicle > 0)
		{
			// completed - at least one requester and one assignee are in the vehicle and outside the zone
			m_TaskSystem.SetTaskState(this, SCR_ETaskState.COMPLETED);
			DeleteTask();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsAnyPlayerAlive(notnull array<int> playerIDs)
	{
		foreach (int playerID : playerIDs)
		{
			if (!GetCharacter(playerID))
				continue;

			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_ChimeraCharacter GetCharacter(int playerID)
	{
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID));
		if (!character)
			return null;

		CharacterControllerComponent charControl = character.GetCharacterController();
		if (!charControl || charControl.IsDead())
			return null;

		return character;
	}

	//------------------------------------------------------------------------------------------------
	protected IEntity GetVehicle(notnull IEntity character)
	{
		SCR_CompartmentAccessComponent compartmentAccess = SCR_CompartmentAccessComponent.Cast(character.FindComponent(SCR_CompartmentAccessComponent));
		if (!compartmentAccess)
			return null;

		return compartmentAccess.GetVehicle();
	}

	//------------------------------------------------------------------------------------------------
	protected void AddXPReward()
	{
		if (!m_RplComponent || m_RplComponent.IsProxy())
			return;

		SCR_XPHandlerComponent comp = SCR_XPHandlerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_XPHandlerComponent));
		if (!comp)
			return;

		// add XP to all players in the assigned group
		array<int> assigneePlayerIDs = GetTaskAssigneePlayerIDs();
		if (!assigneePlayerIDs)
			return;

		foreach (int playerID : assigneePlayerIDs)
		{
			comp.AwardXP(playerID, SCR_EXPRewards.PICKUP_TASK_COMPLETED, m_iRequestersInVehicle, false);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void SetTaskState(SCR_ETaskState state)
	{
		if (state == SCR_ETaskState.COMPLETED)
			AddXPReward();

		super.SetTaskState(state);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		if (SCR_Global.IsEditMode(this))
			return;

		m_fAreaRadiusSq = m_fAreaRadius * m_fAreaRadius;
		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		m_GroupsManager = SCR_GroupsManagerComponent.GetInstance();

		if (!m_RplComponent || m_RplComponent.IsProxy())
			return;

		GetGame().GetCallqueue().CallLater(PeriodicalCheck, PERIODICAL_CHECK_INTERVAL, true);
	}

	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_PickupRequestedTaskEntity()
	{
		GetGame().GetCallqueue().Remove(PeriodicalCheck);
	}
}
