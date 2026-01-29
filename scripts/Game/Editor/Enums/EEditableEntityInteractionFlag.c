/** @ingroup Editor_Entities
*/

//! Details of entity interaction
enum EEditableEntityInteractionFlag
{
	ALIVE			= 1 << 0, ///< Entity and target are not destroyed, or cannot be destroyed
	DELEGATE		= 1 << 1, ///< Hovering over delegate, i.e., icon, not mesh
	LAYER_EDITING	= 1 << 2, ///< Layer editing is enabled
	NON_PLAYABLE	= 1 << 3, ///< Entity is not a player
	PLACING			= 1 << 4, ///< Current operation is placing
};
