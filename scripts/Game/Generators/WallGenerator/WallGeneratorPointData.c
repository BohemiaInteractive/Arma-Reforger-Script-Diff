class WallGeneratorPointData : ShapePointDataScriptBase
{
	[Attribute(defvalue: "1", desc: "Generate or not walls after this point, until the next point data")]
	bool m_bGenerate;

	[Attribute(defvalue: "", uiwidget: UIWidgets.ResourcePickerThumbnail, desc: "Prefab", params: "et")]
	ResourceName MeshAtPoint;

	[Attribute(defvalue: "0", uiwidget: UIWidgets.Slider, params: "-10 10 0.01")]
	float PrePadding;

	[Attribute(defvalue: "0", uiwidget: UIWidgets.Slider, params: "-10 10 0.01")]
	float PostPadding;

	[Attribute(defvalue: "0", uiwidget: UIWidgets.Slider, params: "-10 10 0.01")]
	float m_fOffsetUp;

	[Attribute(defvalue: "0")]
	bool m_bAlignWithNext;

	[Attribute(defvalue: "0")]
	bool m_bAllowClipping;
}
