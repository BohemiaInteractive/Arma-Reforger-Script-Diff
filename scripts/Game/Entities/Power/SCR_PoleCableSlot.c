[BaseContainerProps(), SCR_BaseContainerStaticTitleField("Cable Slot")]
class SCR_PoleCableSlot
{
	[Attribute(defvalue: "0 0 0", desc: "Connection position", params: "inf inf 0 purpose=coords space=entity")]
	vector m_vPosition;

	// a randomisation here is possible (radius, other position(s), per-axis random, etc)
}
