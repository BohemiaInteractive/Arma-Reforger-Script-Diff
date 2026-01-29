/*
===========================================
Do not modify, this script is generated
===========================================
*/

class ScriptedSightsFOVInfo: SightsFOVInfo
{
	/*!
	Called on initialization.
	\param owner Parent entity.
	\param sights Parent sights component.
	*/
	event protected void OnInit(IEntity owner, BaseSightsComponent sights);
	/*!
	Called every frame when active to update current state.
	\param owner Parent entity.
	\param sights Parent sights component.
	\param timeSlice Delta of time since last update.
	*/
	event protected void OnUpdate(IEntity owner, BaseSightsComponent sights, float timeSlice);
	/*!
	Return current field of view of this info.
	If 0 or less is returned, first person perspective FOV will be used instead.
	Value can be accessed via SightsFOVInfo.GetFOV();
	\return Returns field of view in degrees.
	*/
	event protected float GetCurrentFOV() { return 0.0; };
}
