/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup UI_HUD
\{
*/

class BaseHUDComponentClass: GameComponentClass
{
}

class BaseHUDComponent: GameComponent
{
	// Gets the array of BaseInfoDisplays
	proto external int GetInfoDisplays(out notnull array<BaseInfoDisplay> outInfoDisplays);
	proto external void RegisterInfoDisplay(BaseInfoDisplay infoDisplay);
	proto external void UnregisterInfoDisplay(BaseInfoDisplay infoDisplay);
}

/*!
\}
*/
