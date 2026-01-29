/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
Base class for input mapping key custom data config. Input Mapping config is set in project settings (.gproj file) in InputManagerSettings.UiMappings. This config can be accessible using INputManager.GetKeyUIMapping method
\code
	BaseContainer container = GetGame().GetInputManager().GetKeyUIMapping("keyboard:KC_G");
	if (container)
	{
		BaseContainer data;
		container.Get("Data", data);
		...
	}
	\endcode
*/
class InputMappingKeyData: ScriptAndConfig
{
}
