/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup UI
\{
*/

//! UIInfo - declare object, allows to define UI elements
class UIInfo: ScriptAndConfig
{
	/*!
	Returns the text provided by this UI info. See "Name" attribute.
	*/
	proto external string GetName();
	/*!
	Set the name of this UI Info.
	*IMPORTANT* This will not be synchronized in multiplayer. You need to do it yourself!
	*/
	proto external void SetName(string name);
	/*!
	Returns the description provided by this UI info. See "Description" attribute.
	*/
	proto external string GetDescription();
	/*!
	Set the description of this UI Info.
	*IMPORTANT* This will not be synchronized in multiplayer. You need to do it yourself!
	*/
	proto external void SetDescription(string description);
	/*!
	Returns the icon ResourceName provided by this UI info. See "Icon" attribute.
	*/
	proto external ResourceName GetIconPath();
}

/*!
\}
*/
