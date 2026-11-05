class TreeClass: BaseTreeClass
{
	[Attribute("0", uiwidget: UIWidgets.ComboBox, "Type of ambient sound it should emit.", "", ParamEnumArray.FromEnum(ETreeSoundTypes))]
	int SoundType;
	
	[Attribute("0", uiwidget: UIWidgets.Object, "Distance of foliage from ground.", "")]
	int m_iFoliageHeight;

	[Attribute(defvalue: "0", desc: "Can be removed by players through the use of their shovel.")]
	bool m_bDestructibleByHand;

	[Attribute(defvalue: "0.8", desc: "If it can be removed by hand, the useraction will appear at this height.", params: "0 inf 0.01")]
	float m_fDestroyActionHeight;
}

class Tree : BaseTree
{

}