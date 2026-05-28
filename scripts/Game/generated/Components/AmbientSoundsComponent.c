/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

class AmbientSoundsComponentClass: SoundComponentClass
{
}

class AmbientSoundsComponent: SoundComponent
{
	//! Results true if the query could be performed. Returns false otherwise.
	//! After the query is finished, SCR_OnQueryFinished is triggered.
	proto external bool QueryAmbientSoundsBySphere(float radius, EQueryEntitiesFlags queryFlags = EQueryEntitiesFlags.ALL);
	//! Clears the count array and fills it with the nuber of entities of a given sound type.
	//! Results from the last call to QueryAmbientSoundsBySphere are used.
	proto external void GetAmbientSoundsCountPerType(out notnull array<int> count);
	//! Returns the last camera position
	proto external vector GetCameraOrigin();
	//! Fills the vals array with two items. Item 0 stands for how much river there is, 1 stands for the slope.
	//! Returs false if there is not river at a given position.
	proto external bool GetRiver(const vector pos, out notnull array<float> vals);
	//! Returns a random tree of given sound types.
	//! Results from the last call to QueryAmbientSoundsBySphere are used.
	proto external IEntity GetRandomTree(EAmbientSoundFlags soundFlags, float minHeight);
	//! Fills the ouput array with up to nEntities closest entities of given sound types.
	//! Results from the last call to QueryAmbientSoundsBySphere are used.
	proto external void GetClosestEntities(EAmbientSoundFlags soundFlags, int nEntities, out notnull array<IEntity> output);
	//! Results from the last call to QueryAmbientSoundsBySphere are used.
	proto void TracePointToTerrain(const vector point, inout int iSoundGroup);

	// callbacks

	//! Triggered when a big-enough change in the surrounding area is detected that would require an update.
	event void OnRefreshNeeded();
	//! Triggered when the result of QueryAmbientSoundsBySphere query becomes available.
	event void OnQueryFinished();
}

/*!
\}
*/
