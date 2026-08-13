/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Gamepad
\{
*/

class GamepadEffectsManagerComponent: GameComponent
{
	proto external bool IsEnabled();
	//! Disabled managers clear their runtime effects and are ignored by gamepad output.
	proto external void SetEnabled(bool val, bool bMakeEffectsHandlerDirty = true);
	proto external IEntity GetOwner();
	proto external bool IsContextEnabled(string ctxName);
	//! Enable/Disable contexts by name
	proto external void SetContextEnabled(string ctxName, bool val);
	//! Get owned effect by name.
	proto external GamepadEffect GetOwnedEffectByName(string effectName);
}

/*!
\}
*/
