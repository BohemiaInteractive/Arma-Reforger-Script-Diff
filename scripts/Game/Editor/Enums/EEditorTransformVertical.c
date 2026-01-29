/** @ingroup Editor_Entities
*/

//! Vertical transformation mode
enum EEditorTransformVertical
{
	SEA			= 1 << 0, ///< Above sea level
	TERRAIN		= 1 << 1, ///< Above terrain level
	GEOMETRY	= 1 << 2, ///< Above geometry level
};