void OnFactionCommanderChangedDelegate(SCR_Faction faction, int commanderPlayerId);
typedef func OnFactionCommanderChangedDelegate;
typedef ScriptInvokerBase<OnFactionCommanderChangedDelegate> OnFactionCommanderChangedInvoker;

//! Handles voting and AI decisions for the faction Commander role. Should be attached to gamemode entity.
[ComponentEditorProps(category: "GameScripted/Commander", description: "Handles voting and AI decisions for the faction Commander role. Should be attached to gamemode entity.")]
class SCR_FactionCommanderHandlerComponentClass : SCR_BaseGameModeComponentClass
{
}

class SCR_FactionCommanderHandlerComponent : SCR_BaseGameModeComponent
{
	[Attribute("600", desc: "How often can a player volunteer for the Commander role (seconds).", params: "0 inf 1")]
	protected int m_iVolunteerCooldown;

	[Attribute("300", desc: "When the commander can be replaced after being elected (seconds).", params: "0 inf 1")]
	protected int m_iReplaceCommanderCooldown;

	[Attribute("1", desc: "When enabled, players need to gain a specific rank to be able to voluteer for the Commander role.")]
	protected bool m_bCheckRank;

	[Attribute(SCR_ECharacterRank.PRIVATE.ToString(), uiwidget: UIWidgets.ComboBox, desc: "If enabled, only players with this rank or higher can voluteer for the Commander role.", enumType:SCR_ECharacterRank)]
	protected SCR_ECharacterRank m_eMinimumCommanderRank;

	[Attribute(desc: "Array of AI faction commanders")]
	protected ref array<ref SCR_BaseAIFactionCommander> m_aAIFactionCommanders;

	protected ref OnFactionCommanderChangedInvoker m_OnFactionCommanderChanged;

	protected ref map<int, WorldTimestamp> m_mVotingTimestamps = new map<int, WorldTimestamp>();

	[RplProp(onRplName: "OnFactionCommanderChanged")]
	protected ref array<int> m_aFactionCommanders = {};		// {factionIndex, commanderId, factionIndex, commanderId, ...}

	protected RplComponent m_RplComponent;

	protected ref map<int, WorldTimestamp> m_mReplaceCommanderCooldowns = new map<int, WorldTimestamp>();

	//------------------------------------------------------------------------------------------------
	static SCR_FactionCommanderHandlerComponent GetInstance()
	{
		BaseGameMode gameMode = GetGame().GetGameMode();

		if (!gameMode)
			return null;

		return SCR_FactionCommanderHandlerComponent.Cast(gameMode.FindComponent(SCR_FactionCommanderHandlerComponent));
	}

	//------------------------------------------------------------------------------------------------
	OnFactionCommanderChangedInvoker GetOnFactionCommanderChanged()
	{
		if (!m_OnFactionCommanderChanged)
			m_OnFactionCommanderChanged = new OnFactionCommanderChangedInvoker();

		return m_OnFactionCommanderChanged;
	}

	//------------------------------------------------------------------------------------------------
	int GetVolunteerCooldown()
	{
		return m_iVolunteerCooldown;
	}

	//------------------------------------------------------------------------------------------------
	WorldTimestamp GetFactionCooldown(int factionIndex)
	{
		return m_mReplaceCommanderCooldowns.Get(factionIndex);
	}

	//------------------------------------------------------------------------------------------------
	bool CheckRank()
	{
		return m_bCheckRank;
	}

	//------------------------------------------------------------------------------------------------
	SCR_ECharacterRank GetMinimumRank()
	{
		return m_eMinimumCommanderRank;
	}

	//------------------------------------------------------------------------------------------------
	//! Sets the commander to his previous group if exists and if there is a free slot, otherwise a new group will be created for him
	//! \param[in] oldCommanderId
	protected void SetGroupForOldCommander(int oldCommanderId)
	{
		if (oldCommanderId <= 0)
			return;

		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_GroupsManagerComponent));
		if (!groupsManager)
			return;

		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(oldCommanderId);
		if (!playerController)
			return;

		SCR_Faction faction = SCR_Faction.Cast(SCR_FactionManager.SGetPlayerFaction(oldCommanderId));
		if (!faction)
			return;

		SCR_PlayerControllerGroupComponent playerControllerGroupComponent = SCR_PlayerControllerGroupComponent.Cast(playerController.FindComponent(SCR_PlayerControllerGroupComponent));
		if (!playerControllerGroupComponent)
			return;

		// return player back to previous non commander group
		int previousGroupID = playerControllerGroupComponent.GetPreviousGroupID();
		if (previousGroupID >= 0)
		{
			SCR_AIGroup previousGroup = groupsManager.FindGroup(previousGroupID);
			if (previousGroup && previousGroup.GetPlayerCount() < previousGroup.GetMaxMembers())
			{
				playerControllerGroupComponent.RequestJoinGroup(previousGroupID);
				return;
			}
		}

		SCR_AIGroup group = groupsManager.GetFirstNotFullForFaction(faction, null, true);
		if (group)
		{
			playerControllerGroupComponent.RequestJoinGroup(group.GetGroupID());
			return;
		}

		playerControllerGroupComponent.RequestCreateGroupWithData(SCR_EGroupRole.ASSAULT, false, "", "");

		group = groupsManager.GetFirstNotFullForFaction(faction, null, true);
		if (group)
		{
			playerControllerGroupComponent.RequestJoinGroup(group.GetGroupID());
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Sets the commander to his special commander group
	//! \param[in] old commander id
	protected void SetGroupForNewCommander(int newCommanderId)
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_GroupsManagerComponent));
		if (!groupsManager)
			return;

		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(newCommanderId);
		if (!playerController)
			return;

		SCR_Faction faction = SCR_Faction.Cast(SCR_FactionManager.SGetPlayerFaction(newCommanderId));
		if (!faction)
			return;

		array<SCR_AIGroup> playableGroups = groupsManager.GetPlayableGroupsByFaction(faction);
		if (!playableGroups)
			return;

		SCR_AIGroup commanderGroup;
		foreach (SCR_AIGroup aiGroup : playableGroups)
		{
			if (aiGroup && aiGroup.GetGroupRole() == SCR_EGroupRole.COMMANDER)
			{
				commanderGroup = aiGroup;
				break;
			}
		}

		if (!commanderGroup || commanderGroup.GetPlayerCount() > 0)
			return;

		SCR_PlayerControllerGroupComponent playerControllerGroupComponent = SCR_PlayerControllerGroupComponent.Cast(playerController.FindComponent(SCR_PlayerControllerGroupComponent));
		if (!playerControllerGroupComponent)
			return;

		playerControllerGroupComponent.RequestJoinGroup(commanderGroup.GetGroupID());
	}

	//------------------------------------------------------------------------------------------------
	void SetFactionCommander(notnull SCR_Faction faction, int commanderPlayerId)
	{
		if (faction.GetCommanderId() == commanderPlayerId)
			return;

		FactionManager fManager = GetGame().GetFactionManager();

		if (!fManager)
			return;

		int factionIndex = fManager.GetFactionIndex(faction);
		bool found;

		foreach (int index, int element : m_aFactionCommanders)
		{
			if (index % 2 != 0)
				continue;

			if (element != factionIndex)
				continue;

			if (m_aFactionCommanders.IsIndexValid(index + 1))
				m_aFactionCommanders.Set(index + 1, commanderPlayerId);
			else
				m_aFactionCommanders.Insert(commanderPlayerId);

			found = true;
		}

		if (!found)
		{
			m_aFactionCommanders.Insert(factionIndex);
			m_aFactionCommanders.Insert(commanderPlayerId);
		}

		ChimeraWorld world = GetGame().GetWorld();
		if (!world)
			return;

		if (commanderPlayerId > 0)
		{
			WorldTimestamp timeStamp = world.GetServerTimestamp().PlusSeconds(m_iReplaceCommanderCooldown);
			m_mReplaceCommanderCooldowns.Set(factionIndex, timeStamp);
			Rpc(RpcDo_FactionCooldown, factionIndex, timeStamp);
		}
		else
		{
			m_mReplaceCommanderCooldowns.Remove(factionIndex);
			Rpc(RpcDo_FactionCooldown, factionIndex, world.GetServerTimestamp());
		}

		OnFactionCommanderChanged();
		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_FactionCooldown(int factionIdx, WorldTimestamp timeStamp)
	{
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return;

		SCR_Faction myFaction = SCR_Faction.Cast(factionManager.GetLocalPlayerFaction());
		if (!myFaction)
			return;

		int myFactionIdx = factionManager.GetFactionIndex(myFaction);
		if (myFactionIdx != factionIdx)
			return;

		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;

		SCR_FactionCommanderPlayerComponent comp = SCR_FactionCommanderPlayerComponent.Cast(playerController.FindComponent(SCR_FactionCommanderPlayerComponent));
		if (!comp)
			return;

		comp.SetReplaceCommanderCooldownTimestamp(timeStamp);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnFactionCommanderChanged()
	{
		FactionManager fManager = GetGame().GetFactionManager();

		if (!fManager)
			return;

		foreach (int index, int element : m_aFactionCommanders)
		{
			if (index % 2 != 0)
				continue;

			if (!m_aFactionCommanders.IsIndexValid(index + 1))
				return;

			SCR_Faction faction = SCR_Faction.Cast(fManager.GetFactionByIndex(element));

			if (!faction)
				continue;

			int commanderId = m_aFactionCommanders[index + 1];
			int commanderIdOld = faction.GetCommanderId();

			if (commanderIdOld == commanderId)
				continue;

			faction.SetCommanderId(commanderId);

			// called on server
			if (m_RplComponent && m_RplComponent.Role() == RplRole.Authority)
				OnServerCommanderChanged(commanderId, commanderIdOld);

			PlayerController pc = GetGame().GetPlayerController();

			if (pc)
			{
				int playerId = pc.GetPlayerId();

				if (commanderIdOld == playerId || commanderId == playerId)
				{
					SCR_FactionCommanderPlayerComponent component = SCR_FactionCommanderPlayerComponent.Cast(pc.FindComponent(SCR_FactionCommanderPlayerComponent));

					if (component)
					{
						if (commanderIdOld == playerId)
							component.OnCommanderRightsLost();
						else
							component.OnCommanderRightsGained();
					}
				}
			}

			if (m_OnFactionCommanderChanged)
				m_OnFactionCommanderChanged.Invoke(faction, commanderId);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnServerCommanderChanged(int newCommanderId, int oldCommanderId)
	{
		SetGroupForOldCommander(oldCommanderId);
		SetGroupForNewCommander(newCommanderId);
	}

	//------------------------------------------------------------------------------------------------
	//! Unregister player from Commander role when they change factions
	protected void OnPlayerFactionChanged(int playerId, SCR_PlayerFactionAffiliationComponent playerFactionAffiliationComponent, Faction faction)
	{
		FactionManager fManager = GetGame().GetFactionManager();

		if (!fManager)
			return;

		int factionIndex = -1;

		foreach (int index, int element : m_aFactionCommanders)
		{
			if (index % 2 == 0)
				continue;

			if (element != playerId)
				continue;

			factionIndex = m_aFactionCommanders[index - 1];
			break;
		}

		if (factionIndex == -1)
			return;

		SCR_Faction commandedFaction = SCR_Faction.Cast(fManager.GetFactionByIndex(factionIndex));

		if (!commandedFaction || commandedFaction == faction)
			return;

		SetFactionCommander(commandedFaction, 0);
		SCR_NotificationsComponent.SendToFaction(commandedFaction, false, ENotification.VOTING_COMMANDER_WITHDRAW, playerId);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnVotingStart(EVotingType type, int value)
	{
		if (type != EVotingType.COMMANDER)
			return;

		ChimeraWorld world = GetGame().GetWorld();

		if (!world)
			return;

		WorldTimestamp timestamp = world.GetServerTimestamp().PlusSeconds(m_iVolunteerCooldown);
		m_mVotingTimestamps.Set(value, timestamp);
		SetNextVolunteeringTimestamp(value, timestamp);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnVotingEnd(EVotingType type, int value, int winner)
	{
		if (type != EVotingType.COMMANDER && type != EVotingType.COMMANDER_WITHDRAW)
			return;

		PlayerController controller = GetGame().GetPlayerManager().GetPlayerController(winner);

		if (!controller)
			return;

		SCR_PlayerXPHandlerComponent playerXPHandlerComponent = SCR_PlayerXPHandlerComponent.Cast(controller.FindComponent(SCR_PlayerXPHandlerComponent));

		if (!playerXPHandlerComponent)
			return;

		playerXPHandlerComponent.UpdatePlayerRank(true);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetNextVolunteeringTimestamp(int playerId, WorldTimestamp timestamp)
	{
		PlayerController pc = GetGame().GetPlayerManager().GetPlayerController(playerId);

		if (!pc)
			return;

		SCR_FactionCommanderPlayerComponent comp = SCR_FactionCommanderPlayerComponent.Cast(pc.FindComponent(SCR_FactionCommanderPlayerComponent));

		if (!comp)
			return;

		comp.SetNextVolunteeringTimestamp(timestamp);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerRegistered(int playerId)
	{
		super.OnPlayerRegistered(playerId);

		if (m_mVotingTimestamps.Get(playerId))
			SetNextVolunteeringTimestamp(playerId, m_mVotingTimestamps.Get(playerId));
	}

	//------------------------------------------------------------------------------------------------
	//! Unregister player from Commander role when they disconnect
	override void OnPlayerDisconnected(int playerId, KickCauseCode cause, int timeout)
	{
		super.OnPlayerDisconnected(playerId, cause, timeout);

		if (Replication.IsClient())
			return;

		FactionManager fManager = GetGame().GetFactionManager();

		if (!fManager)
			return;

		int factionIndex = -1;

		foreach (int index, int element : m_aFactionCommanders)
		{
			if (index % 2 == 0)
				continue;

			if (element != playerId)
				continue;

			factionIndex = m_aFactionCommanders[index - 1];
			break;
		}

		if (factionIndex == -1)
			return;

		SCR_Faction faction = SCR_Faction.Cast(fManager.GetFactionByIndex(factionIndex));

		if (!faction)
			return;

		if (faction.GetCommanderId() != playerId)
			return;

		SetFactionCommander(faction, 0);
		SCR_NotificationsComponent.SendToFaction(faction, false, ENotification.VOTING_COMMANDER_WITHDRAW, playerId);

		int factionIdx = fManager.GetFactionIndex(faction);
		m_mReplaceCommanderCooldowns.Remove(factionIdx);

	}

	//------------------------------------------------------------------------------------------------
	override void OnGameModeStart()
	{
		if (!m_RplComponent || m_RplComponent.IsProxy())
			return;

		foreach (SCR_BaseAIFactionCommander commander : m_aAIFactionCommanders)
		{
			commander.Init(this);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnGameEnd()
	{
		if (!m_RplComponent || m_RplComponent.IsProxy())
			return;

		foreach (SCR_BaseAIFactionCommander commander : m_aAIFactionCommanders)
		{
			commander.Deinit();
		}
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));

		if (Replication.IsServer())
		{
			SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());

			if (factionManager)
				factionManager.GetOnPlayerFactionChanged_S().Insert(OnPlayerFactionChanged);

			SCR_VotingManagerComponent votingManager = SCR_VotingManagerComponent.GetInstance();

			if (votingManager)
			{
				votingManager.GetOnVotingStart().Insert(OnVotingStart);
				votingManager.GetOnVotingEnd().Insert(OnVotingEnd);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		if (!GetGame().InPlayMode())
			return;

		SetEventMask(owner, EntityEvent.INIT);
	}
}
