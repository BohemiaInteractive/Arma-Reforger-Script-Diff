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
	proto external void QueryAmbientSoundsBySphere(float radius, EQueryEntitiesFlags queryFlags = EQueryEntitiesFlags.ALL);
	proto external void GetAmbientSoundsCountPerType(out notnull array<int> count);
	proto external vector GetCameraOrigin();
	proto external bool GetRiver(const vector pos, out notnull array<float> count);
	proto external IEntity GetRandomTree(int index, float minHeight);
	proto external void GetClosestEntities(int soundTypeIdx, int nEntities, out notnull array<IEntity> output);
	proto void TracePointToTerrain(const vector point, inout int iSoundGroup);
}

/*!
\}
*/
