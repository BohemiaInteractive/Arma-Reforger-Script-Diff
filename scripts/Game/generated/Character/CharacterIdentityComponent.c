/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Character
\{
*/

class CharacterIdentityComponentClass: GameComponentClass
{
}

/*!
* Component that takes care of identity of characters
*/
class CharacterIdentityComponent: GameComponent
{
	proto external IEntity GetHeadEntity();
	/*
	Sets the Alpha for the head.
	\param a target Aplha between 0-255
	*/
	proto external void SetHeadAlpha(int a);
	/*
	Sets the visibility of all submeshes.
	\param visible Whether all submeshes should be drawn or not.
	*/
	proto external void SetVisibleAll(bool visible);
	/*
	Sets wound state for specific bodyPart.
	\param bodyPart Name of the part to be changed
	\param wound Whether part should be wounded or not.
	*/
	proto external bool SetWoundState(string bodyPart, bool wound);
	/*!
	Check if bodypart is covered
	\param bodyPart name of bodypart
	*/
	proto external bool IsCovered(string bodyPart);
	/*
	Reuse currently stored Identity
	As an example
		CharacterIdentityComponent identityComponent = CharacterIdentityComponent.Cast( soldierEntity.FindComponent(CharacterIdentityComponent));
			if (!identityComponent)
				return;
		VisualIdentity visId = identityComponent.GetIdentity().GetVisualIdentity();
		visId.SetHead("{24D28E910BF9F648}Prefabs/Characters/Heads/Head_Asian_02.et");
		identityComponent.CommitChanges();
	*/
	proto external void CommitChanges();
	//! Get identity of this character
	proto external Identity GetIdentity();
	/*!
	Check if given IDs are within bounds of existing identities in the current faction assigned to the owner.
	\return true if all parameters are correct, false if one or more values is wrong or there is no faction component
	*/
	proto external bool CheckIdentityIDs(int alias, int name, int surname, int soundIdentity, int visualIdentity);
	/*!
	This will change identity to given ids defined in characters faction -> factionIdentity
	\returns true on success false on wrong input
	*/
	proto external bool SetIdentityFromIDs(int alias, int name, int surname, int soundIdentity, int visualIdentity);
	//! Set the identity to the player, and replicates the identity to clients
	proto external void SetIdentity(Identity cfg);

	// callbacks

	event void OnBodyPartStateChanged(string bodyPart, bool visible, bool wounded);
}

/*!
\}
*/
