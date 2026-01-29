/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Weapon
\{
*/

class ScriptedSightsComponentClass: SightsComponentClass
{
}

class ScriptedSightsComponent: SightsComponent
{
	/*!
	Called during EOnInit.
	\param owner Entity this component is attached to.
	*/
	event void OnInit(IEntity owner);
	//! Called every time the sight is activated.
	event void OnSightADSActivated();
	/*!
	Called on post-frame as long as sights ADS is active
	\param owner Entity this component is attached to.
	\param timeSlice Delta time since last update.
	*/
	event void OnSightADSPostFrame(IEntity owner, float timeSlice);
	//! Called every time the sight is deactivated.
	event void OnSightADSDeactivated();
	//! Called to get the ADS percentage when the sights go active
	event float GetADSActivationPercentageScript() { return 1.0; };
	//! Called to get the ADS percentage when the sights go inactive
	event float GetADSDeactivationPercentageScript() { return 1.0; };
}

/*!
\}
*/
