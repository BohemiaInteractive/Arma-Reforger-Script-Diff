/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

class FactionAffiliationComponentClass: GameComponentClass
{
}

class FactionAffiliationComponent: GameComponent
{
	//! Returns the entity owning the component.
	proto external IEntity GetOwner();
	//! Returns the affiliated faction or null if none.
	proto external Faction GetAffiliatedFaction();
	//! Returns the default affiliated faction or null if none.
	proto external Faction GetDefaultAffiliatedFaction();
	/*!
	Sets the affiliated faction to the provided one or clears current one if null.
	\param faction The new affiliated faction
	*/
	proto external void SetAffiliatedFaction(Faction faction);
	/*!
	Sets affiliated faction by the faction identifier as registered in FactionManager.
	Invalid or empty key will clear the currently affiliated faction.
	\param factionKey The identifier of the faction
	*/
	proto external void SetAffiliatedFactionByKey(FactionKey factionKey);
	proto external FactionKey GetDefaultFactionKey();
	proto external FactionKey GetAffiliatedFactionKey();

	// callbacks

	/*!
	Called after all components are initialized.
	\param owner Entity this component is attached to.
	*/
	event protected void OnPostInit(IEntity owner);
	event protected void OnCompartmentEntering(IEntity vehicle, IEntity occupant, BaseCompartmentSlot compartment, bool move);
	event protected void OnCompartmentEntered(IEntity vehicle, IEntity occupant, BaseCompartmentSlot compartment, bool move);
	event protected void OnCompartmentLeft(IEntity vehicle, IEntity occupant, BaseCompartmentSlot compartment, bool move);
	event protected void OnFactionChanged(Faction previous, Faction current);
}

/*!
\}
*/
