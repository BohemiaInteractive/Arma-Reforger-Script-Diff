/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

/*!
Component for handling world subscene attached to entity for light portals,
interior audio, visibility, etc.
*/
class WorldSubsceneComponent: GenericComponent
{
	/*!
	Returns portal count of the subscene or 0.
	*/
	proto external int GetPortalCount();
	/*!
	Returns the name of the portal or an empty string if the index is out of bounds or the portal doesn't have a name.
	*/
	proto external string GetPortalName();
	/*!
	Spatial based lookup for portals using oriented bounding box, there are no guarantees which portal gets returned
	when the area of interest covers multiple portals.

	\param mins Minimum of the local space oriented bounding box
	\param maxs Maximum of the local space oriented bounding box
	\param worldTransform World transformation matrix for the bounding box
	\return Index of a first incident portal or 0xFFFF when no portal is found
	*/
	proto external int FindPortalByOBB(vector mins, vector maxs, vector worldTransform[4]);
	/*!
	Name based portal lookup. Use with caution, involves linear search, i.e., not meant for runtime routine searches for portal indices.
	Returns a first occurrence of a portal with the given name, i.e., when two portals have the same name, the second one won't ever be hit.

	\param name Name of the portal to find
	\return Index of the found portal or 0xFFFF when no portal with supplied name was found
	*/
	proto external int FindPortalByName(string name);
	/*!
	Spatial based aperture registration for portals using oriented bounding box, there are no guarantees which portal gets returned
	when the area of interest covers multiple portals.

	Returned aperture index starts at 0 and for subsequent calls for the same portal the index is increased by 1. The aperture index is
	then used to identify who is trying to affect the portal, e.g., situation where the portal is controlled by two separate door components.
	Up to 4 independent apertures per single portal are supported.

	\param mins Minimum of the local space oriented bounding box
	\param maxs Maximum of the local space oriented bounding box
	\param worldTransform World transformation matrix for the bounding box
	\param[out] portalIdx The index of a portal if one was found or 0xFFFF
	\param[out] apertureIdx Unique aperture index for each portal or 0xFF when no portal was found
	*/
	proto void RegisterApertureByOBB(vector mins, vector maxs, vector worldTransform[4], out int portalIdx, out int apertureIdx);
	/*!
	Registers aperture for a specified portal by its index (see FindPortalByName() on how to obtain such an index). Aperture index starts at 0
	and for subsequent calls for the same portal the index	is increased by 1. The aperture index is then used to identify who is trying to affect
	the portal, e.g., situation where the portal is controlled by two separate door components. Up to 4 independent apertures per single portal are
	supported.

	\param portalIdx The index of a portal to register aperture to
	\return doorIdx Unique aperture index for each portal or 0xFF if wrong portalIdx is passed in
	*/
	proto external int RegisterApertureByPortalId(int portalIdx);
	/*!
	Sets how open/closed the portal is using values from range [0, 1]:
	0 - fully closed, 1 - fully open.

	\param portalIdx Index of the target portal
	\param apertureIdx Which registered aperture triggered the action
	\param value Value from [0, 1] setting the opening of the portal for the selected aperture
	*/
	proto external void SetPortalOpening(int portalIdx, int apertureIdx, float value);
	/*!
	Gets how open/closed the portal (aggregate value for all apertures) is using values from range [0, 1] 0 - fully closed, 1 - fully open

	\param portalIdx Index of the target portal
	\return Portal opening value or 0 when the index is out of bounds
	*/
	proto external float GetPortalOpening(int portalIdx);
	/*!
	Set the portal intensity in range [0, 1] (default = 1) for fine-grained control over the visual output

	\param portalIdx Index of the target portal
	\param value Intensity value from range [0, 1]
	*/
	proto external void SetPortalIntensity(int portalIdx, float value);
	/*!
	Get the portal intensity in range [0, 1] (default = 1) for fine-grained control over the visual output

	\param portalIdx Index of the target portal
	\return Portal intensity value or 0 when the index is out of bounds
	*/
	proto external float GetPortalIntensity(int portalIdx);
	/*!
	Returns whether the specified portal is enabled or disabled. Disabled portals are ignored during subscene traversal.

	\param portalIdx Index of the target portal
	\return Portal enabled/disabled state or false when the index is out of bounds
	*/
	proto external bool IsPortalEnabled(int portalIdx);
	/*!
	Sets/clears the portal enabled/disabled state - disabled portals are ignored during subscene traversal.

	\param portalIdx Index of the target portal
	\param value Intended portal state: true = enabled, false = disabled
	*/
	proto external void SetPortalEnabled(int portalIdx, bool value);
	/*!
	Returns the state of the sound blocking portal flag (B), by default this setting comes from a portal material.
	When true the portal blocks sound (e.g., window unless broken). It is mutually exclusive with the portal sound
	passing flag (S), see further.

	\param portalIdx Index of the target portal
	\return Portal sound blocking flag state or false when the index is out of bounds
	*/
	proto external bool IsPortalBlockingSound(int portalIdx);
	/*!
	Sets/clears the state of the sound blocking portal flag (B) overriding any material settings - when set
	the portal blocks sound (e.g., window unless broken). It is mutually exclusive with the portal sound
	passing flag (S), see further, so when blocking sound is enabled the pass sound flag (S) is cleared.

	\param portalIdx Index of the target portal
	\param value Intended portal sound blocking flag (B) state: true = blocking sound, false = not set (driven by aperture)
	*/
	proto external void SetPortalBlockingSound(int portalIdx, bool value);
	/*!
	Returns the state of the sound passing portal flag (S). When set the portal passes sound even if the opening value is 0
	(e.g., closed door with broken glass inset). It is mutually exclusive with the sound blocking flag (B) see above.

	\param portalIdx Index of the target portal
	\return Portal sound passing flag state or false when the index is out of bounds
	*/
	proto external bool IsPortalPassingSound(int portalIdx);
	/*!
	Sets/clears the state of the sound passing portal flag (S) potentially overriding any material settings w.r.t.
	the sound blocking flag (B) which can be set from the material - because these two flags are mutually exclusive,
	setting the sound passing flag (S) clears the sound blocking flag (B), see SetPortalBlockingSound(). When set the
	portal passes sound even if the opening value is 0 (e.g., closed door with broken glass inset).

	When clearing this flag the sound blocking flag (B) is restored if it was set originally in the material

	\param portalIdx Index of the target portal
	\param value Intended portal sound passing flag (S) state: true = passing sound even if closed, false = not set (driven by aperture)
	*/
	proto external void SetPortalPassingSound(int portalIdx, bool value);
}

/*!
\}
*/
