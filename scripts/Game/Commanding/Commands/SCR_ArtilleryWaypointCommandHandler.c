[BaseContainerProps()]
class SCR_ArtilleryWaypointCommandHandler : SCR_BaseWaypointCommandHandler
{
	[Attribute("-1", desc: "Number of rounds that group will fire when issued this order.\n-1 means infinite", params: "-1 inf")]
	protected int m_iNumberOfRounds;

	[Attribute(SCR_EAIArtilleryAmmoType.HIGH_EXPLOSIVE.ToString(), UIWidgets.ComboBox, desc: "Type of the ammunition that will be fired", enumType: SCR_EAIArtilleryAmmoType)]
	protected SCR_EAIArtilleryAmmoType m_eAmmunitionType;

	//------------------------------------------------------------------------------------------------
	override void OnWaypointCreated(notnull SCR_AIWaypoint waypoint)
	{
		SCR_AIWaypointArtillerySupport artilleryWaypoint = SCR_AIWaypointArtillerySupport.Cast(waypoint);
		if (!artilleryWaypoint)
			return;

		if (artilleryWaypoint.GetAmmoType() != m_eAmmunitionType)
			artilleryWaypoint.SetAmmoType(m_eAmmunitionType);

		if (artilleryWaypoint.GetTargetShotCount() != m_iNumberOfRounds)
			artilleryWaypoint.SetTargetShotCount(m_iNumberOfRounds);
	}

	//------------------------------------------------------------------------------------------------
	override void OnWaypointIssued(notnull SCR_AIWaypoint waypoint, notnull SCR_AIGroup group)
	{
		SCR_AIWaypointArtillerySupport artilleryWaypoint = SCR_AIWaypointArtillerySupport.Cast(waypoint);
		if (!artilleryWaypoint)
			return;

		if (artilleryWaypoint.IsActive())
			return;

		artilleryWaypoint.SetActive(true);
	}
}
