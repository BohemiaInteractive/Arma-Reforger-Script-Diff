/** @ingroup Editor_Entities
*/

//! Layers allowing editor to be opened. All of them have to be activated (certain exceptions may apply when the editor is in unlimited mode)
enum EEditorCanOpen
{
	SCRIPT = 1, ///< Set by script
	ALIVE = 2, ///< When alive
	MODES = 4 ///< At least one editor mode available
};