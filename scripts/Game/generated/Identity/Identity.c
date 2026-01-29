/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Identity
\{
*/

class Identity: ScriptAndConfig
{
	//! Get Character name
	proto external string GetName();
	//! Set charactera name
	proto external void SetName(string name);
	//! Get Charactger alias
	proto external string GetAlias();
	//! Set Character alias
	proto external void SetAlias(string alias);
	//! Get Character surname
	proto external string GetSurname();
	//! Set Character surname
	proto external void SetSurname(string surname);
	//! Get the Character Name "Alias" Surname in a string. The format is not localized based on the country.
	proto external string GetFullName();
	proto external VisualIdentity GetVisualIdentity();
	proto external void SetVisualIdentity(VisualIdentity id);
	proto external SoundIdentity GetSoundIdentity();
	proto external void SetSoundIdentity(SoundIdentity id);
}

/*!
\}
*/
