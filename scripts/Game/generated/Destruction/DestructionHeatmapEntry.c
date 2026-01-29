/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Destruction
\{
*/

class DestructionHeatmapEntry: ScriptAndConfig
{
	proto external int GetWeight();
	proto external void SetWeight(int weight);
	proto external EDestructionHeatmapCategory GetCategory();
	proto external void SetCategory(EDestructionHeatmapCategory category);
	proto external IEntity GetEntity();
	proto external void SetEntity(IEntity ent);
	proto external vector GetMins();
	proto external vector GetMaxs();
	proto external void SetBoundingBox(vector mins, vector maxs);
	//Returns the weight this entry had during its last update
	proto external int GetWeightOfLastUpdate();

	// callbacks

	//Called when the entry is being removed from the heatmap.
	event void OnEntryRemoved();
	//Called when the entry has changed its values and might require an update
	event void OnUpdate();
	event void OnDiag();
}

/*!
\}
*/
