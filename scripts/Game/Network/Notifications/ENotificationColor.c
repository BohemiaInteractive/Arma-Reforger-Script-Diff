/*!
Notification Color used when displaying notifications
*/
enum ENotificationColor
{
	NEUTRAL,
	WARNING,
	POSITIVE,
	NEGATIVE,
	GM, //!< GMs notification that is shown to players
	
	//~ Always keep the first faction related color 1000 or higher! This is checked in code if a faction related color should be used. It checks for FACTION_FRIENDLY_IS_NEGATIVE so always add new colors below
	FACTION_FRIENDLY_IS_NEGATIVE = 1000, //!< Will either set POSITIVE COLOR if target entity is hostile to player, NEGATIVE COLOR if target entity is friendly to player
	FACTION_FRIENDLY_IS_POSITIVE = 1001, //!< Will either set NEGATIVE COLOR if target entity is hostile to player, POSITIVE COLOR if target entity is friendly to player
	FACTION_ENEMY_IS_NEGATIVE_ONLY = 1002, //!< Will set color to NEGATIVE if the target entity is an enemy. Will set it to NEUTRAL if the entity is an ally
	FACTION_FRIENDLY_IS_POSITIVE_ONLY = 1003, //!< Will set color to POSITIVE if the target entity is an ally. Will set it to NEUTRAL if the entity is an enemy
	
	//~ Sets the notification color to the faction color
	FACTION_COLOR = 1004, //!< The color will be set to faction color.
};