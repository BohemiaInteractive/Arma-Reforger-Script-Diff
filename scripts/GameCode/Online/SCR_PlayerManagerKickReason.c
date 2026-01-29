//~ Make sure to update the PlayerKickReasonConfig if there are any changes to this enum
enum SCR_PlayerManagerKickReason: PlayerManagerKickReason
{
	FRIENDLY_FIRE,
	DISRUPTIVE_BEHAVIOUR,
	
	KICKED_BY_GM = 10,
	BANNED_BY_GM = 11,
};