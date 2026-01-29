[BaseContainerProps()]
class SCR_PowerPoleSlotBase : Managed
{
	[Attribute(defvalue: "0 0 0", desc: "Power Pole cable connection", params: "inf inf purpose=coords space=entity")]
	vector m_vSlotA;
}

[BaseContainerProps()]
class SCR_PowerPoleSlot : SCR_PowerPoleSlotBase
{
	[Attribute(desc: "Power Pole cable opposed connection", params: "inf inf purpose=coords space=entity")]
	vector m_vSlotB;
}

[BaseContainerProps()]
class SCR_PowerPoleSlotSingle : SCR_PowerPoleSlotBase
{
}
