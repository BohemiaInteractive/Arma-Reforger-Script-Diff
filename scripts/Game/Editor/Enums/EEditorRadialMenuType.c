/** @ingroup Editable_Entities
*/ 

//! Unique radial menus used in the editor, radialmenus can use the same component for data handeling, this allows them to be categorized by enum type instead.
enum EEditorRadialMenuType
{
	ACTIONS,			///< Action menu used for context actions (neutralize, heal, delete)
	COMMANDS			///< Command menu which will instantly set the selected command (move, defend)
};