/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

class ScriptedRadioComponentClass: BaseRadioComponentClass
{
}

class ScriptedRadioComponent: BaseRadioComponent
{
	/*!
	Called after all components are initialized.
	\param owner Entity this component is attached to.
	*/
	event void OnPostInit(IEntity owner);
	/*!
	Called during EOnInit.
	\param owner Entity this component is attached to.
	*/
	event void OnInit(IEntity owner);
	/*!
	Called during EOnFrame.
	\param owner Entity this component is attached to.
	\param timeSlice Delta time since last update.
	*/
	event void OnFrame(IEntity owner, float timeSlice);
}

/*!
\}
*/
