[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class TreeGroupClass
{
	[Attribute(defvalue: "1", uiwidget: UIWidgets.SpinBox, desc: "Weight of this group", params: "0 inf")]
	float m_fWeight;

	[Attribute(uiwidget: UIWidgets.Object, params: "noDetails")]
	ref array<ref ForestGeneratorTree> m_aTrees;
}
