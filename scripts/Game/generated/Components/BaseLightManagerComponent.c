/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

class BaseLightManagerComponentClass: GameComponentClass
{
}

class BaseLightManagerComponent: GameComponent
{
	//! Gets the enable flag (all lights)
	proto external bool GetLightsEnabled();
	//! Sets the state flag for given type
	proto external void SetLightsState(ELightType type, bool state = false, int side = -1);
	//! Gets the state flag for given type
	proto external bool GetLightsState(ELightType type, int side = -1);
	//! Searches for Light by ID
	proto external BaseLightSlot FindLight(int lightID);
	//! Returns a list and count of all lights
	proto external int GetLights(out notnull array<BaseLightSlot> outLights);
	//! Changes functional state of light surface and all linked lights for provided surface id, return true on success
	proto external bool TrySetSurfaceFunctional(int surfaceID, bool functiona);
	proto external int GetEmissiveSurfaces(out notnull array<EmissiveLightSurfaceSlot> outSurfaces);

	// callbacks

	/*!
	Called during EOnInit.
	\param owner Entity this component is attached to.
	*/
	event protected void OnInit(IEntity owner);
	/*!
	Called during EOnFrame.
	\param owner Entity this component is attached to.
	\param timeSlice Delta time since last update.
	*/
	event protected void OnFrame(IEntity owner, float timeSlice);
}

/*!
\}
*/
