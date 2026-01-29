/** @ingroup ManualCamera
*/

/*!
Flags serving as conditions for manual camera components.
*/
enum EManualCameraFlag
{
	MOVE = 1, ///< Camera is moving
	ROTATE = 2, ///< Camera is rotating
	MOVE_ATL = 4, ///< Camera is maintaining height above terrain
	ORBIT = 8, ///< Camera is orbiting
	ABOVE_TERRAIN = 16, ///< Camera cannot go below terrain
	SPEED_ATL = 32, ///< Camera is faster when higher above terrain
	ROTATE_MODIFIER = 64 ///< Rotation is enabled only with modifier
};