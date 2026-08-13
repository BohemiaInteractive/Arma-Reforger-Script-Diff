[EnumLinear()]
enum SCR_EOverrideTeamkillKickPenalty // override that prevents teamkills from kicking people
{
	// No override will be applied, "normal behaviour" people TK'ing or getting TK'd from this faction will be kicked eventually
	DEFAULT,
	// people from this faction can TK without getting kicked
	NO_KICKS_TEAMKILLING,
	// people can TK this faction without getting kicked
	NO_KICKS_TEAMKILLED,
	// people from this faction can TK or get TK'd without anybody getting kicked
	NO_KICKS
}