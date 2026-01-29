/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup WorldSystems
\{
*/

sealed class WorldSystems: pointer
{
	proto external WorldSystem FindSystem(typename interfaceType);
	proto external BaseContainer FindSystemSource(typename interfaceType);
	proto external WorldController FindMyController(typename interfaceType);
	proto external WorldController FindController(typename interfaceType, PlayerId ownerPlayerId);
	proto external void GetOwnerPlayerIds(notnull array<PlayerId> outOwnerPlayerIds);
}

/*!
\}
*/
