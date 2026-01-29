/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup UI_HUD
\{
*/

class HUDManagerComponentClass: SCR_BaseHUDComponentClass
{
}

class HUDManagerComponent: SCR_BaseHUDComponent
{
	proto external bool IsHUDEnabled();
	proto external bool EnableHUD(bool state);
	proto external bool StartDrawing(BaseInfoDisplay display);
	proto external void StopDrawing(BaseInfoDisplay display);
	proto external int GetInfoDisplaysDrawn(out notnull array<BaseInfoDisplay> outInfoDisplays, bool getAllShown = false);

	// callbacks

	/*!
	Called during EOnInit.
	\param owner Entity this component is attached to.
	*/
	event protected void OnInit(IEntity owner);
	event protected void OnUpdate(IEntity owner);
	event protected void OnControlledEntityChanged(IEntity from, IEntity to);
}

/*!
\}
*/
