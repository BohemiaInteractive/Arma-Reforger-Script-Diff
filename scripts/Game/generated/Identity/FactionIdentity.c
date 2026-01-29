/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Identity
\{
*/

class FactionIdentity: ScriptAndConfig
{
	proto external SocialIdentity GetSocialIdentities();
	proto external void GetSoundIdentities(out notnull array<ref SoundIdentity> outItems);
	proto external void GetVisualIdentities(out notnull array<ref VisualIdentity> outItems);
	proto external ref SoundIdentity CreateSoundIdentity(int id);
	proto external ref VisualIdentity CreateVisualIdentity(int id);
}

/*!
\}
*/
