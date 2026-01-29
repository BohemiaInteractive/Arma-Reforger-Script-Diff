/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Entities
\{
*/

class DestructionManagerClass: GenericEntityClass
{
}

class DestructionManager: GenericEntity
{
	proto external RegionalDestructionManager GetOrCreateRegionalDestructionManager(const vector position);
	proto external void QueueDestructionHeatmapEntry(notnull DestructionHeatmapEntry entryData);
}

/*!
\}
*/
