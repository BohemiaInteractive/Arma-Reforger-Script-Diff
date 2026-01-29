/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Physics
\{
*/

sealed class PhysicsUtils
{
	private void PhysicsUtils();
	private void ~PhysicsUtils();

	//! Looks for a collider using `colliderName` in `meshObject`, returns collider index or -1.
	static proto int FindColliderIndex(MeshObject meshObject, string colliderName);
	//! Gets node index (parent bone) from `colliderIndex` in `meshObject`, returns node index or -1.
	static proto int GetNodeIndex(MeshObject meshObject, int colliderIndex);
}

/*!
\}
*/
