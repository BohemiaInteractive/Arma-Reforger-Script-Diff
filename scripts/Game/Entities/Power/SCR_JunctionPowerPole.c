[EntityEditorProps(category: "GameScripted/Power/OBSOLETE", description: "This is an obsolete junction power pole entity.", color: "0 255 0 255", visible: false, dynamicBox: true)]
class SCR_JunctionPowerPoleClass : SCR_PowerPoleClass
{
}

class SCR_JunctionPowerPole : SCR_PowerPole
{
	[Attribute(uiwidget: UIWidgets.None, desc: "[OBSOLETE (use Cable Slot Groups above)] Slots for connecting with other power poles in a junction", category: "[OLD] Power Cable Slots")]
	protected ref array<ref SCR_PowerPoleSlotBase> m_aJunctionSlots; // obsolete since 2024-04-02, hidden since 2024-08-07
}
