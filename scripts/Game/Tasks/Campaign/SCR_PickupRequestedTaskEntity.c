class SCR_PickupRequestedTaskEntityClass : SCR_BaseRequestedTaskEntityClass
{
}

class SCR_PickupRequestedTaskEntity : SCR_BaseRequestedTaskEntity
{
	[Attribute("", desc: "Title text shown for members of the requester group.")]
	protected string m_sOwnerTitle;

	static int CHECK_DISTANCE_SQ = 100 * 100;
	static int PERIODICAL_CHECK_INTERVAL = 2000; // ms

	//------------------------------------------------------------------------------------------------
	override protected void OnSetRequesterId(int requesterId)
	{
		super.OnSetRequesterId(requesterId);

		RplComponent rpl = RplComponent.Cast(FindComponent(RplComponent));
		if (!rpl || rpl.IsProxy())
			return;

		GetGame().GetCallqueue().CallLater(PeriodicalCheck, PERIODICAL_CHECK_INTERVAL, true);
	}

	//------------------------------------------------------------------------------------------------
	protected void PeriodicalCheck()
	{
		SCR_GroupsManagerComponent component = SCR_GroupsManagerComponent.GetInstance();
		if (!component)
			return;

		SCR_AIGroup group = component.FindGroup(GetRequesterId());
		if (!group)
			return;

		PlayerManager pManager = GetGame().GetPlayerManager();
		array<int> playerIds = group.GetPlayerIDs();
		SCR_ChimeraCharacter character;
		bool done;
		vector taskPosition = GetOrigin();

		foreach (int playerId : playerIds)
		{
			character = SCR_ChimeraCharacter.Cast(pManager.GetPlayerControlledEntity(playerId));

			if (!character)
				continue;

			CharacterControllerComponent charControl = character.GetCharacterController();

			if (!charControl || charControl.IsDead())
				continue;

			if (!character.IsInVehicle())
				continue;

			if (vector.DistanceSqXZ(character.GetOrigin(), taskPosition) > CHECK_DISTANCE_SQ)
				continue;

			done = true;
		}

		if (!done)
			return;

		GetGame().GetCallqueue().Remove(PeriodicalCheck);

		m_TaskSystem.SetTaskState(this, SCR_ETaskState.COMPLETED);
		DeleteTask();
	}

	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_PickupRequestedTaskEntity()
	{
		GetGame().GetCallqueue().Remove(PeriodicalCheck);
	}
}
