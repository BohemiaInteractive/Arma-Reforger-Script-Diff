[EntityEditorProps(category: "GameScripted/Commanding", description: "Commanding manager, attach to game mode entity!.")]
class SCR_CommandingManagerComponentClass : SCR_BaseGameModeComponentClass
{
}

//------------------------------------------------------------------------------------------------
class SCR_CommandingManagerComponent : SCR_BaseGameModeComponent
{
	[Attribute("{04D3B38E23F51754}Prefabs/AI/Groups/Slave_Group.et")]
	protected ResourceName m_sAIGroupPrefab;
	
	[Attribute("{54764D4E706F348C}Configs/Commanding/Commands.conf")]
	protected ResourceName m_sCommandsConfigPath;
	
	[Attribute(defvalue: "8", UIWidgets.EditBox, desc: "How many AI soldiers can be recruited into single player group")]
	protected int m_iMaxAIPerGroup;
	
	protected SCR_PlayerCommandsConfig m_CommandsConfig;
	
	protected static SCR_CommandingManagerComponent s_Instance;
	
	protected ref array<ref SCR_BaseRadialCommand> m_aCommands;
	protected ref map<string, ref SCR_BaseRadialCommand> m_mNameCommand = new map<string, ref SCR_BaseRadialCommand>();

	protected static const int NORMAL_PRIORITY = 50;
	
	//------------------------------------------------------------------------------------------------
	//! \return
	static SCR_CommandingManagerComponent GetInstance()
	{
		return s_Instance;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	ResourceName GetGroupPrefab()
	{
		return m_sAIGroupPrefab;
	}
	
	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_CommandingManagerComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		if (!s_Instance)
			s_Instance = this;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		InitiateCommandMaps();
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode)
			return;
		
		gameMode.GetOnPlayerSpawned().Insert(ResetSlaveGroupWaypoints);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void InitiateCommandMaps()
	{
		SCR_PlayerCommandsConfig commandsConfig = GetCommandsConfig();
		if (!commandsConfig)
			return;
		
		m_aCommands = commandsConfig.GetCommands();
		
		foreach (SCR_BaseRadialCommand command : m_aCommands)
		{
			m_mNameCommand.Insert(command.GetCommandName(), command);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets called when player respawns so the newly recruited AIs dont follow the old waypoints from orders
	//! \param[in] playerId
	//! \param[in] player
	void ResetSlaveGroupWaypoints(int playerId, IEntity player)
	{	
		PlayerController pc = GetGame().GetPlayerManager().GetPlayerController(playerId);
		if (!pc)
			return;
		
		//do nothing if player is not a leader of group
		SCR_PlayerControllerGroupComponent groupComponent = SCR_PlayerControllerGroupComponent.Cast(pc.FindComponent(SCR_PlayerControllerGroupComponent));
		if (!groupComponent || !groupComponent.IsPlayerLeaderOwnGroup())
			return;
		
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;
		
		SCR_AIGroup masterGroup = groupsManager.GetPlayerGroup(playerId);
		if (!masterGroup)
			return;
		
		//no need to do this for empty group because there are no AIs to mess things up
		SCR_AIGroup slaveGroup = masterGroup.GetSlave();
		if (!slaveGroup)
			return;
		
		//reset the waypoints for slave group, so they go to their default behavior
		array<AIWaypoint> currentWaypoints = {};
		slaveGroup.GetWaypoints(currentWaypoints);
		foreach (AIWaypoint currentwp : currentWaypoints)
		{
			slaveGroup.RemoveWaypoint(currentwp);
		}
		
		AIGroupMovementComponent slaveGroupMovementComp = AIGroupMovementComponent.Cast(slaveGroup.FindComponent(AIGroupMovementComponent));
		if (slaveGroupMovementComp)
			slaveGroupMovementComp.SetFormationDisplacement(0);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called on server
	//! \param[in] commandIndex
	//! \param[in] cursorTargetID
	//! \param[in] groupRplID
	//! \param[in] targetPosition
	//! \param[in] playerID
	void RequestCommandExecution(int commandIndex, RplId cursorTargetID, RplId groupRplID, vector targetPosition, int playerID, float seed)
	{
		//find entity of AI that will be responding to commands
		SCR_BaseGroupCommand command = SCR_BaseGroupCommand.Cast(FindCommand(FindCommandNameFromIndex(commandIndex)));
		RplId responderRplId;
		if (command && command.ShouldPlayAIResponse())
		{
			//find speaking AI, find its rplid and pass it, then in playing response, get the entity from rpl it
			SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
			if (!groupsManager)
				return;
			
			SCR_AIGroup playerGroup = groupsManager.GetPlayerGroup(playerID);
			if (!playerGroup)
				return;
			
			SCR_AIGroup slaveGroup = playerGroup.GetSlave();
			if (!slaveGroup)
				return;
	
			AIAgent agent = slaveGroup.GetLeaderAgent();
			if (agent)
			{
				IEntity owner = agent.GetControlledEntity();
				if (owner)
					responderRplId = Replication.FindId(owner);	
			}
		}
		
		//check if the passed arguments are valid, if yes, send a callback RPC to commanders playercontroller so he can make a gesture.			
		RPC_DoExecuteCommand(commandIndex, cursorTargetID, groupRplID, responderRplId, targetPosition, playerID, seed);
		Rpc(RPC_DoExecuteCommand, commandIndex, cursorTargetID, groupRplID, responderRplId, targetPosition, playerID, seed);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] commandIndex
	//! \param[in] cursorTargetID
	//! \param[in] groupRplID
	//! \param[in] targetPosition
	//! \param[in] playerID
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_DoExecuteCommand(int commandIndex, RplId cursorTargetID, RplId groupRplID, RplId responderRplID, vector targetPosition, int playerID, float seed)
	{
		RplComponent rplComp;
		IEntity cursorTarget, group;
		rplComp = RplComponent.Cast(Replication.FindItem(cursorTargetID));
		if (rplComp)
			cursorTarget = rplComp.GetEntity();
		
		rplComp = RplComponent.Cast(Replication.FindItem(groupRplID));
		if (rplComp)
			group = rplComp.GetEntity();
		
		rplComp = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		if (rplComp)
		{
			SCR_BaseRadialCommand command = FindCommand(FindCommandNameFromIndex(commandIndex));
			if (command.Execute(cursorTarget, group, targetPosition, playerID, rplComp.IsProxy()))
			{
				PlayCommanderSound(playerID, commandIndex, seed);
				
				//call this later when the commander is done speaking
				//todo: replace with callback from audio
				SCR_BaseGroupCommand groupCommand = SCR_BaseGroupCommand.Cast(command);
				if (groupCommand && groupCommand.ShouldPlayAIResponse())
					GetGame().GetCallqueue().CallLater(PlayAICommandResponse, 2000, false, playerID, true, seed, responderRplID);
				
				if (!rplComp.IsMaster())
					return;
				
				PlayerController controller = GetGame().GetPlayerManager().GetPlayerController(playerID);
				if (!controller)
					return;
				
				SCR_PlayerControllerCommandingComponent commandingComp = SCR_PlayerControllerCommandingComponent.Cast(controller.FindComponent(SCR_PlayerControllerCommandingComponent));
				if (!commandingComp)
					return;
				
				commandingComp.CommandExecutedCallback(commandIndex);			
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] playerID
	//! \param[in] commandIndex
	void PlayCommanderSound(int playerID, int commandIndex, float seed)
	{
		SCR_ChimeraCharacter playerCharacter = SCR_ChimeraCharacter.Cast(GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID));
		if (!playerCharacter)
			return;
		
		SignalsManagerComponent signalManager = SignalsManagerComponent.Cast(playerCharacter.FindComponent(SignalsManagerComponent));
		if (!signalManager)
			return;

		SCR_CommunicationSoundComponent soundComponent = SCR_CommunicationSoundComponent.Cast(playerCharacter.FindComponent(SCR_CommunicationSoundComponent));
		if (!soundComponent)
			return;

		string soundEventName = GetCommandSoundEventName(commandIndex);
		if (soundEventName.IsEmpty())
			return;
		
		float soundEventSeed = GetCommandSoundEventSeed(commandIndex);
		if (soundEventSeed == -1)
			soundEventSeed = seed;

		int signalSoldierCalled = signalManager.FindSignal("SoldierCalled");
		int signalSeed = signalManager.FindSignal("Seed");
		int signalSoldierCaller = signalManager.FindSignal("SoldierCaller");
		
		//for now some values are hard set since we dont need different cases.
		signalManager.SetSignalValue(signalSoldierCalled, 1000);
		signalManager.SetSignalValue(signalSoldierCaller, 0);
		signalManager.SetSignalValue(signalSeed, seed);
		soundComponent.SoundEventPriority(soundEventName, NORMAL_PRIORITY);
	}
	
	//------------------------------------------------------------------------------------------------
	void PlayAICommandResponse(int playerID, bool positive, float seed, RplId responderRplId)
	{
		IEntity character = IEntity.Cast(Replication.FindItem(responderRplId));
		if (!character)
			return;
		
		SCR_CommunicationSoundComponent soundComponent = SCR_CommunicationSoundComponent.Cast(character.FindComponent(SCR_CommunicationSoundComponent));
		if (!soundComponent)
			return;
		
		SignalsManagerComponent signalManager = SignalsManagerComponent.Cast(character.FindComponent(SignalsManagerComponent));
		if (!signalManager)
			return;

		int signalSeed = signalManager.FindSignal("Seed");
		signalManager.SetSignalValue(signalSeed, seed);
		soundComponent.SoundEventPriority(SCR_SoundEvent.SOUND_CP_POSITIVEFEEDBACK, 50);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] commandName
	//! \return
	SCR_BaseRadialCommand FindCommand(string commandName)
	{
		return m_mNameCommand.Get(commandName);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] commandName
	//! \return
	int FindCommandIndex(string commandName)
	{
		SCR_BaseRadialCommand command = FindCommand(commandName);
		if (!command)
			return -1;
		
		return m_aCommands.Find(command);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] commandIndex
	//! \return
	string GetCommandSoundEventName(int commandIndex)
	{
		SCR_BaseRadialCommand command = m_aCommands.Get(commandIndex);
		SCR_BaseGroupCommand groupCommand = SCR_BaseGroupCommand.Cast(command);
		if (!groupCommand)
			return string.Empty;
		
		
		return groupCommand.GetSoundEventName();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] commandIndex
	//! \return
	float GetCommandSoundEventSeed(int commandIndex)
	{
		SCR_BaseRadialCommand command = m_aCommands.Get(commandIndex);
		SCR_BaseGroupCommand groupCommand = SCR_BaseGroupCommand.Cast(command);
		if (!groupCommand)
			return -2;
		
		
		return groupCommand.GetSoundEventSeed();
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] commandIndex
	//! \return
	string FindCommandNameFromIndex(int commandIndex)
	{
		SCR_BaseRadialCommand command = m_aCommands.Get(commandIndex);
		if (!command)
			return string.Empty;
		
		return command.GetCommandName();
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] commandName
	//! \return
	bool CanShowCommand(string commandName)
	{
		SCR_BaseRadialCommand command = FindCommand(commandName);
		if (!command)
			return false;
		
		return command.CanBeShown();
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] commandName
	//! \return
	bool CanShowOnMap(string commandName)
	{
		SCR_BaseRadialCommand command = FindCommand(commandName);
		if (!command)
			return false;
		
		return command.CanShowOnMap();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_PlayerCommandsConfig GetCommandsConfig()
	{
		if (m_CommandsConfig)
			return m_CommandsConfig;
		
		Resource holder = BaseContainerTools.LoadContainer(m_sCommandsConfigPath);
		if (!holder)
			return null;
		
		BaseContainer container = holder.GetResource().ToBaseContainer();
		if (!container)
			return null;
		
		SCR_PlayerCommandsConfig commandsConfig = SCR_PlayerCommandsConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(container));
		if (!commandsConfig)
			return null;
		
		m_CommandsConfig = commandsConfig;
		return m_CommandsConfig;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	int GetMaxAIPerGroup()
	{
		return m_iMaxAIPerGroup;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] max
	void SetMaxAIPerGroup(int max)
	{
		m_iMaxAIPerGroup = max;
	}
	
	//------------------------------------------------------------------------------------------------		
	IEntity GetRespondingAI(int playerID)
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return null;
		
		SCR_AIGroup playerGroup = groupsManager.GetPlayerGroup(playerID);
		if (!playerGroup)
			return null;
		
		SCR_AIGroup slaveGroup = playerGroup.GetSlave();
		if (!slaveGroup)
			return null;

		AIAgent agent = slaveGroup.GetLeaderAgent();
		if (!agent)
			return null;

		return agent.GetControlledEntity();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	string GetCommandDisplayTextByName(string commandName)
	{
		SCR_BaseRadialCommand command = FindCommand(commandName);
		if (!command)
			return string.Empty;
		
		return command.GetCommandDisplayName();
	}
}
