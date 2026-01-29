/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Perception
\{
*/

class PerceivableComponent: GameComponent
{
	proto external EAIUnitType GetUnitType();
	//! Returns overall visual recognition factor
	proto external float GetVisualRecognitionFactor();
	//! Represents ambient light, dynamic lights, own lights.
	proto external float GetIlluminationFactor();
	//! Returns ambient light LV value (logarithmic). Can be used for diagnostics.
	proto external float GetAmbientLV();
	//! Returns our sound emission power
	proto external float GetSoundPower();
	//! Returns estimated size in meters. Currently it's calculated based on physics or visual bounding box.
	proto external float GetEstimatedVisualSize();
	//! Fills the provided array with the list of all aimpoints as locations in model space
	//! Returns the number of output elements.
	proto external int GetAimpoints(out notnull array<vector> outPoints);
	proto external int GetAimpointsOfType(out notnull array<ref AimPoint> outPoints, EAimPointType type);
	//! Returns true when this perceivable is considered disarmed
	proto external bool IsDisarmed();
	proto external void SetDisarmed(bool disarmed);
	//! Compartment access component
	proto external CompartmentAccessComponent GetCompartmentAccessComponent();
	proto external bool IsInCompartment();
	proto external FactionAffiliationComponent GetFactionAffiliationComponent();
	//! Overrides faction value used by PerceptionComponents (see GetPerceivedFaction). Null value removes this override.
	proto external void SetPerceivedFactionOverride(Faction faction);
	//! Returns value set by SetPerceivedFactionOverride()
	proto external Faction GetPerceivedFactionOverride();
	//! Returns value set by SetPerceivedFactionOverride if it is not null. Otherwise returns value from FactionAffilicationComponent.
	//! PerceptionComponent uses this method as well to resolve faction of target.
	proto external Faction GetPerceivedFaction();

	// callbacks

	event void EOnInit(IEntity owner);
}

/*!
\}
*/
