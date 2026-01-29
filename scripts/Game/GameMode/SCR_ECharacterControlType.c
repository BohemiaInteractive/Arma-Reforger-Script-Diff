//! What kind of controller the character or (in some cases vehicle) has, eg: AI, Player, possessed AI, GameMaster/Admin
enum SCR_ECharacterControlType
{
	UNKNOWN = 1 << 0, //!< Unknown what the character is
	PLAYER = 1 << 1, //!< Character is a player
	AI = 1 << 2, //!< Character is an AI
	POSSESSED_AI = 1 << 3, //!< Character is an AI possessed by a GM
	UNLIMITED_EDITOR = 1 << 4, //!< Character is a GM, admin or on the admin list
}