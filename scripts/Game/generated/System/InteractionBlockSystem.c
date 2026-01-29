/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup System
\{
*/

class InteractionBlockSystem: GameSystem
{
	proto external bool CanCreateInteractionBlocks(IEntity ent, IEntity otherEnt);
	//! Create interaction blocks between ent->GetRootParent() and otherEnt while taking into account childrens entities.
	proto external bool Create(IEntity ent, IEntity otherEnt);
	//! Create interaction blocks between ent and otherEnt without taking into account childrens entities.
	proto external bool CreateSimpleBlock(IEntity ent, IEntity otherEnt);
	//! Check if an interaction block exist between ent and otherEnt
	proto external bool DoesExist(IEntity ent, IEntity otherEnt);
	//! Removes all interaction blocks linked to this entity
	proto external bool Remove(IEntity ent);
	//! Removes interaction block between ent and otherEnt and taking into account childrens entities.
	proto external bool RemoveInteractionBlocks(IEntity ent, IEntity otherEnt);
	//! Removes interaction blocks between ent and otherEnt without taking into account childrens entities.
	proto external bool RemoveSimpleBlock(IEntity ent, IEntity otherEnt);
}

/*!
\}
*/
