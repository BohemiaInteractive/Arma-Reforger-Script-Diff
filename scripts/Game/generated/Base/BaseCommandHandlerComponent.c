/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Base
\{
*/

class BaseCommandHandlerComponentClass: GameComponentClass
{
}

class BaseCommandHandlerComponent: GameComponent
{
	/*!
	Sets the flag for disabling animation update. This function is ignored in MP.
	\param simulationDisabled If true simulation is disabled
	*/
	proto external void SetSimulationDisabled(bool pSimulationDisabled);

	// callbacks

	/*!
	Called during EOnInit.
	\param owner Entity this component is attached to.
	*/
	event protected void OnInit(IEntity owner);
	event protected void OnCommandActivate(int pCmdId);
	event protected void OnCommandDeactivate(int pCmdId);
	/*!
	Overrides the whole native command handling
	\param pDt Delta time since last call.
	\param pCurrentCommandID Current command ID.
	\param pCurrentCommandFinished True if current command is finished.
	*/
	event protected void Update(float pDt, int pCurrentCommandID, bool pCurrentCommandFinished);
}

/*!
\}
*/
