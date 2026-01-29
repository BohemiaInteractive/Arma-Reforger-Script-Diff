/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup World
\{
*/

sealed class TrackDecal: Decal
{
	/*!
	Checks whether adding new point to Track decal is possible.
	\param entity		entity to add new Track point
	\param materialName			material of decal
	\param newPoint 	new point to add
	\return
		LMD_ERROR 	= error when adding new point (invalid decal)
		LMD_VALID 	= can add new point
		LMD_DIFF_ENT 	= new point is on different entity
		LMD_TOO_FAR	= new point is too far from previous point
	*/
	proto external int CanAddToTrackDecal(IEntity entity, ResourceName materialName, vector newPoint);
	/*!
	add new point to decal, internally, new point is added when previous point is in some
	distance or the angle is more than some threshold
	\param point		contact point
	\param normal		normal of contact
	\param alpha		translucency in point
	\return true if everything was OK, false if not. In this case, the application MUST not used later the pointer to decal, it's finalized internally !
	*/
	proto external bool AddPointToTrackDecal(vector point, vector normal, float alpha);
	/*!
	finalize Track adding, e.g. when entity lose contact with ground -> the pointer to decal
	should have only world and entity if it has something to render, otherwise it's destroyed here
	\param addAlpha	if to add last point with transition to zero alpha
	\param alphaDist	distance to add last point
	*/
	proto external void FinalizeTrackDecal(bool addAlpha, float alphaDist);
	//! Returns last Track point or -65535.0 in all components.
	proto external vector GetLastTrackPoint();
	//! Returns whether Track was finalized.
	proto external bool IsTrackFinalized();
	/*!
	set global parameters for Track generation
	\param minSegmentLength		minimum length segment, when new point is added (4 default), when is less, just the end position is on the fly updated
	\param maxSegmentLength		maximum segment length, when length is bigger, the path is finished
	\param degAngle				angle in degrees, when is more, the path is finished
	*/
	static proto void SetGlobalTrackParams(float minSegmentLength, float maxSegmentLength, float degAngle);
}

/*!
\}
*/
