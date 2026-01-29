class SCR_CharacterHelper
{
	//------------------------------------------------------------------------------------------------
	//! Return the player-controlled entity
	//! See EntityUtils.GetPlayer()
	static IEntity GetPlayer()
	{
		return EntityUtils.GetPlayer();
	}

	//------------------------------------------------------------------------------------------------
	//! Check if the provided entity is the local player
	//! See EntityUtils.GetPlayer()
	//! \param[in] entity
	// unused
	static bool IsPlayer(IEntity entity)
	{
		return entity && entity == EntityUtils.GetPlayer();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Checks player control type based on admin list, GM status, admin status. It does not check if player is possessing an AI, use GetCharacterControlType() instead
	//! \param[in] playerID Player ID represents the unique identifier for a player in the game.
	//! \return the character control type for the specified player ID, indicating if they are a player, admin and/or game master.
	static SCR_ECharacterControlType GetPlayerControlType(int playerID)
	{
		if (playerID <= 0)
		{
			Print("'SCR_EntityHelper.GetPlayerControlType()' Given player id is not valid", LogLevel.WARNING);
			return SCR_ECharacterControlType.UNKNOWN;
		}
		
		//~ If character is on the admin list
		SCR_PlayerListedAdminManagerComponent adminListManager = SCR_PlayerListedAdminManagerComponent.GetInstance();
		if (adminListManager && adminListManager.IsPlayerOnAdminList(playerID))
			return SCR_ECharacterControlType.UNLIMITED_EDITOR;
		
		//~ If character is an Admin or Game Master
		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (playerManager && (playerManager.HasPlayerRole(playerID, EPlayerRole.GAME_MASTER) || playerManager.HasPlayerRole(playerID, EPlayerRole.ADMINISTRATOR || playerManager.HasPlayerRole(playerID, EPlayerRole.SESSION_ADMINISTRATOR))))
			return SCR_ECharacterControlType.UNLIMITED_EDITOR;
		
		return SCR_ECharacterControlType.PLAYER;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Determines entity control type based on player ownership, admin status, or AI control
	//! \param[in] entity Represents an in-game character entity, used to determine its control type (player, AI, admin, or game master)
	//! \return character control type: PLAYER, POSSESSED_AI, AI or GM/Admin
	static SCR_ECharacterControlType GetCharacterControlType(IEntity entity)
	{
		//~ No entity given
		if (!entity)
		{
			Print("'SCR_EntityHelper.GetCharacterControlType()' Given entity is null", LogLevel.WARNING);
			return SCR_ECharacterControlType.UNKNOWN;
		}
		
		//~ Entity is not a character
		ChimeraCharacter character = ChimeraCharacter.Cast(entity);
		if (!character)
		{
			Print("'SCR_EntityHelper.GetCharacterControlType()' Given entity is not a character nor a vehicle so unknown type is returned", LogLevel.WARNING);
			return SCR_ECharacterControlType.UNKNOWN;
		}

		//~ Character is a player
		int playerID = SCR_PossessingManagerComponent.GetPlayerIdFromControlledEntity(character);
		if (playerID > 0)
		{
			//~ If character is possesed
			SCR_PossessingManagerComponent possessingManager = SCR_PossessingManagerComponent.GetInstance();
			if (possessingManager.IsPossessing(playerID))
			{
				if (possessingManager.GetPossessedEntity(playerID) == entity)
					return SCR_ECharacterControlType.POSSESSED_AI;
			}
				
			//~ If character is on the admin list
			SCR_PlayerListedAdminManagerComponent adminListManager = SCR_PlayerListedAdminManagerComponent.GetInstance();
			if (adminListManager && adminListManager.IsPlayerOnAdminList(playerID))
				return SCR_ECharacterControlType.UNLIMITED_EDITOR;
			
			//~ If character is an Admin or Game Master
			PlayerManager playerManager = GetGame().GetPlayerManager();
			if (playerManager && (playerManager.HasPlayerRole(playerID, EPlayerRole.GAME_MASTER) || playerManager.HasPlayerRole(playerID, EPlayerRole.ADMINISTRATOR || playerManager.HasPlayerRole(playerID, EPlayerRole.SESSION_ADMINISTRATOR))))
				return SCR_ECharacterControlType.UNLIMITED_EDITOR;
			
			return SCR_ECharacterControlType.PLAYER;
		}
		
		//~ Character is an AI
		return SCR_ECharacterControlType.AI;
	}

	//------------------------------------------------------------------------------------------------
	//! Check if the provided entity is -a- player - a human-controlled entity
	//! See EntityUtils.IsPlayer()
	//! \param[in] entity
	// unused
	static bool IsAPlayer(IEntity entity)
	{
		return entity && EntityUtils.IsPlayer(entity);
	}

	//------------------------------------------------------------------------------------------------
	//! Checks if provided client is in control of provided character or if it is an AI then is this client the owner of it
	//! \param[in] character to check
	//! \param[out] isAi
	//! \return true when this client has the authority over provided character
	static bool IsPlayerOrAIOwner(notnull ChimeraCharacter character, out bool isAi = false)
	{
		isAi = false;
		if (SCR_PlayerController.GetLocalControlledEntity() == character)
			return true;//do this first as IsPlayer fetches all players

		if (EntityUtils.IsPlayer(character))
			return false;

		isAi = true;
		//if this is ai then make sure that we have the authority over this entity
		RplComponent charRplComp = character.GetRplComponent();
		if (charRplComp && !charRplComp.IsOwner())
			return false;

		return true;
	}
}
