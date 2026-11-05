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
	/*!
	Initializes world systems using config specified by configName. It should only
	be done by whoever is managing lifetime of the world these systems belong to.
	Once world systems have been initialized from a config, they must be cleared
	(using Clear()) first before Init() can be called again with different config.
	It is generally recommended to initialize world systems right when world is
	created and then only clear them when world itself is cleared.
	*/
	proto external void Init(ResourceName configName);
	/*!
	Clears world systems previously initialized using Init(). It should only be
	done by whoever is managing lifetime of the world these systems belong to.
	All world system instances will be destroyed and one has to use Init() to
	create them again.
	*/
	proto external void Clear();
	proto external WorldSystem FindSystem(typename interfaceType);
	proto external BaseContainer FindSystemSource(typename interfaceType);
	proto external WorldController FindMyController(typename interfaceType);
	proto external WorldController FindController(typename interfaceType, PlayerId ownerPlayerId);
	proto external void GetOwnerPlayerIds(notnull array<PlayerId> outOwnerPlayerIds);
}

/*!
\}
*/
