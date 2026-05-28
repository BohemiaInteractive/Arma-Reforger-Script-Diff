/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components_Sound
\{
*/

/*! Static Sound Components are a light weight version of the SndComponent for sound which will not update its position past creation.
Inherit from this component & module to create game specific changes (signals etc.) to the sound.

Static Sounds are update by Static Sound Module functions:
- StaticSoundsModule::InitSound() - Called after creation of the component.
- StaticSoundsModule::UpdateSound() - Called on update of the module.
- StaticSound::Clear() - Called on clean up or when component is removed from the module.

OBS! Static Sounds can only be used with triggers.
*/
class BaseStaticSoundComponent: GenericComponent
{
}

/*!
\}
*/
