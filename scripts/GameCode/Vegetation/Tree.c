class TreeClass: BaseTreeClass
{
	[Attribute("0", uiwidget: UIWidgets.ComboBox, "Type of ambient sound it should emit.", "", ParamEnumArray.FromEnum(ETreeSoundTypes))]
	int SoundType;
	
	[Attribute("0", uiwidget: UIWidgets.Object, "Distance of foliage from ground.", "")]
	int m_iFoliageHeight;
};

class Tree : BaseTree
{

};