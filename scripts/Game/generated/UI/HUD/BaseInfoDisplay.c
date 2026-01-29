/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup UI_HUD
\{
*/

class BaseInfoDisplay: ScriptAndConfig
{
	proto external GenericComponent FindComponentInParentContainer(typename typeName);

	// callbacks

	event protected void OnInit(IEntity owner);
	event protected void UpdateValues(IEntity owner, float timeSlice);
	event protected void OnStartDraw(IEntity owner);
	event protected void OnStopDraw(IEntity owner);
}

/*!
\}
*/
