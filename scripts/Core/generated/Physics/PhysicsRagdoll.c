/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Physics
\{
*/

/*!
Wrapper over Ragdoll simulation
*/
sealed class PhysicsRagdoll: pointer
{
	/*!
	Changes activation state of the ragdoll simulation.
	\param act New activation state
	*/
	proto external void SetActive(int act);
	/*!
	Destroys ragdoll simulation. Only if it is the current physics simulation.
	*/
	proto external void Destroy();
	/*!
	Enables ragdoll simulation. This replaces current physics simulation.
	*/
	proto external bool Enable();
	/*!
	Disables ragdoll simulation.
	\param resetSkeleton True to update entity transforms
	*/
	proto external bool Disable(bool resetSkeleton);
	/*!
	Returns rigidbody of a specified bone
	\param nodeIndex Index of a bone in skeleton
	*/
	proto external Physics GetBoneRigidBodyByNodeIndex(int nodeIndex);
	/*!
	Returns rigidbody of a specified bone
	\param index Index of a bone. Must be within bounds (GetNumBones)
	*/
	proto external Physics GetBoneRigidBody(int index);
	//! Returns number of bones in ragdoll
	proto external int GetNumBones();
	/*!
	Creates ragdoll from definition provided. This replaces current physics simulation.
	\code
		IEntity rdEntity = GetWorld().FindEntityByName("rd_test");
		string rdName = "{CE761502CE2E1990}Prefabs/Characters/character.ragdoll";
		int rdLayer = 1 << 10;
		PhysicsRagdoll rd = PhysicsRagdoll.CreateRagdoll(rdEntity, rdName, 1, rdLayer);
	\endcode
	\param owner Entity that will be associated with ragdoll
	\param ragdollDefName Ragdoll resource name
	\param mass Ragdoll mass
	\param layerMask Bit mask of layers
	*/
	static proto PhysicsRagdoll CreateRagdoll(notnull IEntity owner, string ragdollDefName, float mass, int layerMask);
	/*!
	Returns ragdoll on provided entity if available.
	\param owner Entity checked for ragdoll
	*/
	static proto PhysicsRagdoll GetRagdoll(notnull IEntity owner);
}

/*!
\}
*/
