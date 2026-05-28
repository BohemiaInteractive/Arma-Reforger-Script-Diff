/*!
\defgroup Constants Constants
static script constants
\{
*/

/*!
\defgroup StringConstants String constants
\{
*/
const string STRING_EMPTY = "";
/*!
\}
*/


/*!
\defgroup Colors Colors
\{
*/
const int COLOR_RED = 0xFFF22613;
const int COLOR_GREEN = 0xFF2ECC71;
const int COLOR_BLUE = 0xFF4B77BE;
const int COLOR_YELLOW = 0xFFF7CA18;

const int COLOR_RED_A = 0x1fF22613;
const int COLOR_GREEN_A = 0x1f2ECC71;
const int COLOR_BLUE_A = 0x1f4B77BE;
const int COLOR_YELLOW_A = 0x1fF7CA18;
/*!
\}
*/

//------------------------------------------
/*!
\defgroup Debug Debug utilities
\{
*/

/*!
\defgroup DiagMenu Diag menu API definition
\{
*/

//! Enum of DiagMenu id values, binded automatically with engine diag values.
enum EDiagMenu
{
	DM_REND_SHOWVOLUMETRICRAYS,
	DM_REND_SHOWGBUFFER,
	DM_REND_GBUFFER,
	DM_REND_DT_FULL,
	DM_REND_IBL_NUM,
	DM_REND_IBL_MIP,

	/*!
	0 - Do not override settings.
	1 - Override, force VSync on.
	2 - Override, force VSync off.
	*/
	DM_REND_VSYNC_OVERRIDE,
}
/*!
\}
*/

/*!
\}
*/

/*!
\}
*/
