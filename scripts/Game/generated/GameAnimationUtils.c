/*
===========================================
Do not modify, this script is generated
===========================================
*/

sealed class GameAnimationUtils
{
	private void GameAnimationUtils();
	private void ~GameAnimationUtils();

	//! return -1 if operation was unsuccessful
	static proto AnimationEventID RegisterAnimationEvent(string animationEventString);
	//! Find or add AnimationTagID from provided string
	//! @Note: you should always cache and reuse returned id in order to reduce overhead
	//! return -1 if operation was unsuccessful
	static proto AnimationTagID RegisterAnimationTag(string animationTagString);
	static proto string GetEventString(AnimationEventID eventID);
	static proto string GetTagString(AnimationTagID tagID);
	//! Animate entities colliders from current state of mesh object bones transformation
	static proto void AnimateColliders(IEntity entity);
	//! Searches for index of sub mesh with provided meshName
	//! return -1 in case failed
	static proto int FindMeshIndex(IEntity ent, string meshName);
	//! Set visibility of sub mesh at specified index
	static proto void ShowMesh(IEntity ent, int meshIndex, bool visible);
}
