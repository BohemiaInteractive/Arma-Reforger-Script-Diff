/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Destruction
\{
*/

class DestructionHeatmapRegion: ScriptAndConfig
{
	/*!
	Attempts to merge entries A and B. If it returns true, B is discarded and A will be kept as the result of A+B
	We always merge the new entry into the main entry, as the main entry is more likely to already exist in the world.
	When false is returned, no changes are done.
	\param DestructionHeatmapEntry mainEntry: First heatmap entry
	\param DestructionHeatmapEntry newEntry: Second heatmap entry
	\return Returns true if a merge was done, false otherwise
	*/
	event bool MergeHeatmapEntries(notnull DestructionHeatmapEntry mainEntry, notnull DestructionHeatmapEntry newEntry) {return false;};
	event bool ShouldUpdateHeatmap() { return true;};
	//Called when the entries of the map should be drawn
	event void OnDiag();
}

/*!
\}
*/
