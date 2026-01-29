/** @ingroup Editor_Entities
*/

//! Editor mode that defines overall functionality.
enum EEditorMode
{
	EDIT = 1, ///< Editing and placing entities, configuring attributes
	STRATEGY = 1 << 1, ///< Commanding AI and players
	SPECTATE = 1 << 2, ///< Observing players
	PHOTO = 1 << 3, ///< Photo mode
	ADMIN = 1 << 4, ///< Server administration
	BUILDING = 1 << 5, ///< Building mode
	PHOTO_SAVE = 1 << 6, //< Photo mode for capturing screenshots for GM saves
};