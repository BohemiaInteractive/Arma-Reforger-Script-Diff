/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup WorldSystems
\{
*/

class WorldController: EventProvider
{
	static void InitInfo(WorldControllerInfo outInfo);
	private void WorldController();
	private void ~WorldController();

	proto external World GetWorld();
	proto external WorldSystems GetSystems();
	proto external RplNode GetNode();
	proto external bool IsMyOwn();
	/*!
	Returns PlayerId of the player who owns this controller. This function will
	return correct player identifier when called both on client and on server.
	*/
	proto external PlayerId GetOwnerPlayerId();
	/*!
	Returns Identity Id of the player who owns this controller. This function will
	return correct player identifier when called both on client and on server.
	*/
	proto external UUID GetOwnerIdentityId();
	/*!
	Returns RplIdentity of the player who owns this controller if it is available.
	As RplIdentity is specific to each Replication instance, this function may
	return different values for different instances of the same controller. In
	other words, controller of particular player will return different identity
	depending on its role (authority or proxy).
	Specifically:
		- authority always returns RplIdentity of player who owns the controller (on
		  listen server, it returns RplIdentity::Local() for local player)
		- proxy returns RplIdentity::Local() when it belongs to local player
		- proxy returns RplIdentity::Invalid() when it does not belong to local player

	\return RplIdentity representing player who owns the controller on this
	        Replication instance, or invalid identity when this player cannot be
	        addressed from this Replication instance.
	*/
	proto external RplIdentity GetOwnerRplIdentity();
	/*!
	Attempts to run a remote procedure call (RPC) of this instance with parameters
	specified in method RplRpc attribute.
	\param      method  Member function to be invoked as an RPC.
	*/
	proto protected void Rpc(func method, void p0 = NULL, void p1 = NULL, void p2 = NULL, void p3 = NULL, void p4 = NULL, void p5 = NULL, void p6 = NULL, void p7 = NULL);
	proto external WorldSystem FindSystem(typename interfaceType);
	proto external BaseContainer FindSystemSource(typename interfaceType);

	// callbacks

	event protected void OnAuthorityReady();
	event protected void OnAuthorityClosing();
}

/*!
\}
*/
