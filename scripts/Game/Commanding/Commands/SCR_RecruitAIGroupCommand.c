//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_BaseGroupCommandTitleField("m_sCommandName")]
class SCR_RecruitAIGroupCommand : SCR_BaseGroupCommand
{
	//------------------------------------------------------------------------------------------------
	override bool Execute(IEntity cursorTarget, IEntity target, vector targetPosition, int playerID, bool isClient)
	{
		if (isClient)
		{
			//place to place a logic that would be executed for other players
			return true;
		}		
		
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerID));
		if (!playerController)
			return false;
		
		SCR_PlayerControllerGroupComponent groupController = SCR_PlayerControllerGroupComponent.Cast(playerController.FindComponent(SCR_PlayerControllerGroupComponent));
		if (!groupController)
			return false;
		
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(cursorTarget);
		if (!character)
			return false;
		
		if (GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(character) == 0)
			groupController.RequestAddAIAgent(character, playerID);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShown()
	{
		if (!CanBeShownInCurrentLifeState())
			return false;
		
		PlayerCamera camera = GetGame().GetPlayerController().GetPlayerCamera();
		if (!camera)
			return false;
		
		IEntity cursorTarget = camera.GetCursorTarget();
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(cursorTarget);
		if (!character)
			return false;
		
		if (character.IsRecruited() || !character.IsRecruitable())
			return false;
		
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!playerController)
			return false;
		
		SCR_RespawnSystemComponent respawnComponent = SCR_RespawnSystemComponent.GetInstance();
		if (!respawnComponent)
			return false;
		
		SCR_PlayerControllerGroupComponent groupController = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!groupController)
			return false;
		
		SCR_AIGroup group = groupController.GetPlayersGroup();
		if (!group)
			return false;
		
		SCR_AIGroup slaveGroup = group.GetSlave();
		if (!slaveGroup)
			return false;
		
		SCR_CommandingManagerComponent commandingManager = SCR_CommandingManagerComponent.GetInstance();
		if (!commandingManager)
			return false;
		
		int maxAI = commandingManager.GetMaxAIPerGroup();
		//in case there is a limit on how many AIs can be in single group.
		if (maxAI != -1 && slaveGroup.GetAgentsCount() >= maxAI)
			return false;
		
		if (!CanRoleShow())
			return false;
		
		if (character.GetFaction() != playerController.GetLocalControlledEntityFaction())
			return false;
		
		int playerID = GetGame().GetPlayerController().GetPlayerId();
		
		SCR_Faction playerFaction;
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (factionManager)
		{
			playerFaction = SCR_Faction.Cast(factionManager.GetPlayerFaction(playerID));
		}
		
		return !groupController.IsAICharacterInAnyGroup(character, playerFaction);
	}
}