/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup WorldSystems
\{
*/

/*!
To create a system, you just need to create a class which inherits from WorldSystem,
and then specify GetSystemPoint to your need.
*/
class WorldSystem: EventProvider
{
	static void InitInfo(WorldSystemInfo outInfo);
	private void WorldSystem();
	private void ~WorldSystem();

	//! Get the world associated with this system.
	proto external World GetWorld();
	//! Get world systems associated with this system.
	proto external WorldSystems GetSystems();
	proto external RplNode GetNode();
	//! Enable/disable system.
	proto external void Enable(bool enable);
	//! Returns true if system is enabled.
	proto external bool IsEnabled();
	/*!
	The 4 functions below are designed so scripters can handle a batch update of multiple entities in a performant way.

	Example:
	override void OnUpdate()
	{
		BeginUpdate();
		foreach (IEntity ent: myents)
		{
			ent.SetWorldTransform(myNewTransform);
			AddEntity(ent);
		}
		Update(); //< This is the call where entities are updated in batch
		EndUpdate();
	}
	*/
	proto external protected void BeginUpdate();
	proto external protected void Update();
	proto external protected void EndUpdate();
	proto external protected void AddEntity(notnull IEntity entity);
	/*!
	Attempts to run a remote procedure call (RPC) of this instance with parameters
	specified in method RplRpc attribute.
	\param      method  Member function to be invoked as an RPC.
	*/
	proto protected void Rpc(func method, void p0 = NULL, void p1 = NULL, void p2 = NULL, void p3 = NULL, void p4 = NULL, void p5 = NULL, void p6 = NULL, void p7 = NULL);

	// callbacks

	/*!
	Should return true for systems this system depends on. False otherwise
	\deprecated Use WorldSystemInfo.ExecuteAfter instead.
	*/
	[Obsolete("Use WorldSystemInfo.AddExecuteAfter() instead.")]
	event protected bool DependsOn(WorldSystemPoint point, WorldSystem system) { return false; };
	/*!
	Called when system are to be intialized.
	This always happens at any point when simulation decides to reinitialize
	the system without actually destroying it.
	*/
	event protected void OnInit();
	/*!
	Called when system are to be cleaned up.
	This always happens at any point when simulation decides to bring the system back
	to the initial state without actually destroying it.
	*/
	event protected void OnCleanup();
	//! Called every time system is started (before the first run and after Enable(true) is called
	event protected void OnStarted();
	//! Called every time system is stopped (after Enable(false) is called and before the system is being destroyed
	event protected void OnStopped();
	//! \deprecated Override OnUpdatePoint() instead.
	event protected void OnUpdate(WorldSystemPoint point);
	//! Called at specific points during World update.
	event protected void OnUpdatePoint(WorldUpdatePointArgs args);
	//! Called at specific one-time events during World lifetime.
	event protected void OnOneTimePoint(WorldOneTimePointArgs args);
	event protected void OnDiag(float timeSlice);
}

/*!
\}
*/
