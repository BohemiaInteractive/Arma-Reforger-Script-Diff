class SCR_FactionCommanderVolunteerUserAction : ScriptedUserAction
{
	protected bool m_bInProgress;
	protected bool m_bIsCommanderRoleEnabled;

	protected SCR_VotingManagerComponent m_VotingManager;
	protected SCR_FactionCommanderSignupComponent m_SignupComponent;

	protected static ref ScriptInvokerInt s_OnCanBePerformed;

	protected static const string NEXT_VOLUNTEERING_COOLDOWN_TEXT = "#AR-FactionCommander_NextVolunteeringCooldown";
	protected static const string REPLACE_COMMANDER_COOLDOWN_TEXT = "#AR-FactionCommander_ReplaceCommanderCooldown";

	//------------------------------------------------------------------------------------------------
	static ScriptInvokerInt GetOnCanBePerformed()
	{
		if (!s_OnCanBePerformed)
			s_OnCanBePerformed = new ScriptInvokerInt();

		return s_OnCanBePerformed;
	}

	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		if (System.IsConsoleApp())
			return;

		BaseGameMode gameMode = GetGame().GetGameMode();

		if (!gameMode)
			return;

		SCR_GameModeCampaign campaignGameMode = SCR_GameModeCampaign.Cast(gameMode);
		if (campaignGameMode)
			m_bIsCommanderRoleEnabled = campaignGameMode.GetCommanderRoleEnabled();
		else
			m_bIsCommanderRoleEnabled = true;

		m_SignupComponent = SCR_FactionCommanderSignupComponent.Cast(pOwnerEntity.FindComponent(SCR_FactionCommanderSignupComponent));

		if (!m_SignupComponent)
			return;

		if (!SCR_FactionCommanderHandlerComponent.GetInstance())
			return;

		m_VotingManager = SCR_VotingManagerComponent.GetInstance();

		if (m_VotingManager)
		{
			m_VotingManager.GetOnVotingStart().Insert(OnVotingStart);
			m_VotingManager.GetOnVotingEnd().Insert(OnVotingEnd);
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (m_bInProgress || !m_VotingManager || ! m_SignupComponent || !m_bIsCommanderRoleEnabled)
			return false;

		SCR_Faction playerFaction = SCR_Faction.Cast(SCR_FactionManager.SGetLocalPlayerFaction());

		if (!playerFaction)
			return false;

		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(user);

		if (playerFaction.GetCommanderId() == playerId)
			return false;

		return m_SignupComponent.IsControlledByFaction(playerFaction);
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(user);
		PlayerController pc = GetGame().GetPlayerManager().GetPlayerController(playerId);

		if (!pc)
			return false;

		BaseGameMode gameMode = GetGame().GetGameMode();

		if (!gameMode)
			return false;

		SCR_FactionCommanderHandlerComponent handlerComponent = SCR_FactionCommanderHandlerComponent.GetInstance();

		if (!handlerComponent)
			return false;

		if (handlerComponent.CheckRank() && SCR_CharacterRankComponent.GetCharacterRank(user) < handlerComponent.GetMinimumRank())
		{
			SCR_Faction playerFaction = SCR_Faction.Cast(SCR_FactionManager.SGetLocalPlayerFaction());

			if (playerFaction)
				SetCannotPerformReason(playerFaction.GetRankName(handlerComponent.GetMinimumRank()));

			return false;
		}

		SCR_FactionCommanderPlayerComponent comp = SCR_FactionCommanderPlayerComponent.Cast(pc.FindComponent(SCR_FactionCommanderPlayerComponent));

		if (!comp)
			return false;

		ChimeraWorld world = GetGame().GetWorld();

		if (!world)
			return false;

		WorldTimestamp nextVolunteeringAvailableAt = comp.GetNextVolunteeringTimestamp();
		WorldTimestamp canVolunteeringAgain = comp.GetReplaceCommanderCooldownTimestamp();

		if (IsCooldownActive(nextVolunteeringAvailableAt, NEXT_VOLUNTEERING_COOLDOWN_TEXT))
			return false;

		if (IsCooldownActive(canVolunteeringAgain, REPLACE_COMMANDER_COOLDOWN_TEXT))
			return false;

		if (s_OnCanBePerformed)
			s_OnCanBePerformed.Invoke(playerId);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsCooldownActive(WorldTimestamp deadline, string reasonKey)
	{
		ChimeraWorld world = GetGame().GetWorld();
		if (!world)
			return false;

		WorldTimestamp currentTimestamp = world.GetServerTimestamp();
		if (deadline.Greater(currentTimestamp))
		{
			float timeLeft = deadline.DiffMilliseconds(currentTimestamp);
			int hours, minutes, seconds;

			SCR_DateTimeHelper.GetHourMinuteSecondFromSeconds(Math.Ceil(timeLeft * 0.001), hours, minutes, seconds);
			ActionNameParams[0] = minutes.ToString();
			ActionNameParams[1] = seconds.ToString();

			if (seconds < 10)
				ActionNameParams[1] = "0" + ActionNameParams[1];

			SetCannotPerformReason(reasonKey);
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!m_VotingManager)
			return;

		PlayerController pc = GetGame().GetPlayerController();

		if (!pc)
			return;

		SCR_VoterComponent comp = SCR_VoterComponent.Cast(pc.FindComponent(SCR_VoterComponent));

		if (!comp)
			return;

		comp.Vote(EVotingType.COMMANDER, pc.GetPlayerId());
	}

	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnVotingStart(EVotingType type, int value)
	{
		if (type == EVotingType.COMMANDER && value == GetGame().GetPlayerController().GetPlayerId())
			m_bInProgress = true;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnVotingEnd(EVotingType type, int value, int winner)
	{
		if (type != EVotingType.COMMANDER)
			return;

		int playerID = GetGame().GetPlayerController().GetPlayerId();
		if (winner == playerID || value == playerID)
			m_bInProgress = false;
	}
}
