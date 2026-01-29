/** @ingroup Editor_Entities
*/

//! Unique editor state
enum EEditorState
{
	NONE, ///< No interaction allowed
	SELECTING, ///< Able to select entities
	MULTI_SELECTING, ///< Drawing multi-selection frame
	PLACING, ///< Placing a new entity
	TRANSFORMING ///< Moving or rotating an entity
};