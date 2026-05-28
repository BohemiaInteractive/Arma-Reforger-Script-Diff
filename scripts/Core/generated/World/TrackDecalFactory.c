/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup World
\{
*/

class TrackDecalFactory: Managed
{
	/*!
	Create a new instance of the factory. Each instance is intended for a single wheel (i.e. 4 wheel car should has 4 factory instances).
	\param material material of the track decal
	\param size width of the decal
	\param lifetime the time between the track decal is finalized and it is removed
	*/
	proto void TrackDecalFactory(
	  ResourceName material,
	  float size,
	  float lifetime,
	);

	//! Returns true if this instance has a unfinalized decal
	proto external bool HasActiveDecal();
	/*!
	Add a new point to the track decal. The new track decal will be created if needed.

	This function is suitable to be called each frame, point deduplication and track segment merging are done internally.
	\param entity contact point entity
	\param pointWS world space position of the point
	\param normalWS world space normal vector of the point
	\param alpha alpha value of the decal
	\returns false if there was some error
	*/
	proto external bool AddPoint(IEntity entity, vector pointWS, vector normalWS, float alpha);
	/*!
	Mark the decal as final.

	The track decal instance starts to be immutable. New points added after calling this function will create a new track decal that will not be connected with the previous one.
	\param fadeoutLength length of the newly added track segment, fading to the zero alpha value.
	*/
	proto external bool FinalizeDecal(float fadeoutLength);
}

/*!
\}
*/
