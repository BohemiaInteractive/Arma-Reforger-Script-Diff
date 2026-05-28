/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Entities
\{
*/

class CharacterEntity: PawnEntity
{
	proto external CharacterPhysicsComponent GetCharacterPhysicsComponent();
	proto external CharacterMovementComponent GetMovementComponent();
	proto external CharacterAnimGraphComponent GetAnimGraphComponent();
	proto external CharacterSndComponent GetSoundComponent();
	proto external CharacterSimStrategy GetSimStrategy();
	/*!
	adds an entity that will copy character's pose even if it has added bones.
	entity *must* manually be removed from the list *before* it is deleted.
	*/
	proto external void AddExplicitlyRetargetedEntity(IEntity entity);
	//! removes an entity from the list of entities to copy character's pose
	proto external void RemoveExplicitlyRetargetedEntity(IEntity entity);
}

/*!
\}
*/
