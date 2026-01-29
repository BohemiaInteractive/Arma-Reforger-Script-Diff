//! When a character dies what is the relationship between the victim and killer
//! Values are flags for quicker checking in the SCR_InstigatorContextData
enum SCR_ECharacterDeathStatusRelations
{
	UNKNOWN = 1 << 0, //!< Not known what killed the character
	
	//~ Suicide
	SUICIDE = 1 << 1, //!< Character killed themselves (Can be an AI, limited player, GM, Admin or possessed AI)
	
	//~ Killed by Enemy
	KILLED_BY_ENEMY_AI = 1 << 2, //!< Character was killed by enemy AI
	KILLED_BY_ENEMY_PLAYER = 1 << 3, //!< Character was killed by enemy player (Can be a limited player, GM, Admin or possessed AI)
	
	//~ Killed by Friendly
	KILLED_BY_FRIENDLY_AI = 1 << 4, //!< Character was killed by friendly AI
	KILLED_BY_FRIENDLY_PLAYER = 1 << 5, //!< Character was killed by friendly player (Can be a limited player, GM, Admin or possessed AI)
	
	//~ GM
	KILLED_BY_UNLIMITED_EDITOR = 1 << 6, //!< Character is killed by a GM action (Eg: Context Neutralize, Lightning strike, etc)
	
	//~ Character was deleted not killed
	DELETED = 1 << 7, //!< Character was deleted
	DELETED_BY_EDITOR = 1 << 8, //!< Character was deleted by Editor
	
	//~ Fallbacks. Should never happend
	VICTIM_IS_NEUTRAL_OR_FACTIONLESS = 1 << 9, //!< Victim had no faction (Fallback)
	KILLED_BY_NEUTRAL_OR_FACTIONLESS = 1 << 10, //!< Character was killed by factionless character (Fallback)
	
	OTHER_DEATH = 1 << 11, //!< Killed by a form of suicide without being punished for it  (Fallback)
	
	//~ Other
	NOT_A_CHARACTER = 1 << 12, //!< The entity destroyed is not a character (Generally means it is a vehicle)
}