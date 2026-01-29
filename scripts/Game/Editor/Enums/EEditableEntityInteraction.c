/** @ingroup Editor_Entities
*/

//! Type of suggested interaction when hovering edited entity on top of another entity
enum EEditableEntityInteraction
{
	NONE, ///< Simple transform
	LAYER, ///< Open layer
	SLOT, ///< Attach to slot
	PASSENGER, ///< Place as passenger of vehicle. Used by context action SCR_ChooseAndSpawnOccupantsContextAction not placing opperation
};