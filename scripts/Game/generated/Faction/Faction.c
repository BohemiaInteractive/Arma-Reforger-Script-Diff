/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Faction
\{
*/

class Faction: ScriptAndConfig
{
	proto external FactionIdentity GetFactionIdentity();
	//! Returns true if this faction and other faction are friendly.
	proto external bool IsFactionFriendly(Faction otherFaction);
	//! Returns true if this faction and other faction are hostile.
	proto external bool IsFactionEnemy(Faction otherFaction);
	//! Returns the name of this faction for via this Faction's UI Info.
	proto external string GetFactionName();
	//! Returns the identifier of this faction specified in FactionManager.
	proto external FactionKey GetFactionKey();
	//! Returns the color of this faction specified in FactionManager.
	proto ref Color GetFactionColor();
	//! Returns the UI info of this faction.
	proto external UIInfo GetUIInfo();
	//! Returns the encryption key used for radio transmissions.
	proto external string GetFactionRadioEncryptionKey();
}

/*!
\}
*/
