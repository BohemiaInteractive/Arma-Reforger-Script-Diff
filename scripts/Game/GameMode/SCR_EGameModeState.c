/*!
	Defines states of a game mode.
*/
enum SCR_EGameModeState
{
	/*!
		This is an optional state of the game.
		It can be used to allow more players connect before game mode itself engages.
	*/
	PREGAME = 0,

	/*!
		This is the main state of the game.
		It is where the main loop of the game happens.
	*/
	GAME,

	/*!
		This is the state to which game transfers when it ends.
		It can be used to show scoreboards, additional messages or allow
		user-based voting, etc.
	*/
	POSTGAME
};