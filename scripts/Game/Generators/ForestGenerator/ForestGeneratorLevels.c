[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class ForestGeneratorLevel
{
	[Attribute(uiwidget: UIWidgets.Object, desc: "Tree groups to spawn in this forest generator level")]
	ref array<ref TreeGroupClass> m_aTreeGroups;

	[Attribute(defvalue: "1", desc: "Generate this tree level or not")]
	bool m_bGenerate;

	[Attribute(defvalue: "1", desc: "How many trees per hectare should be generated", params: "0 inf", uiwidget: UIWidgets.SpinBox)]
	float m_fDensity;

	[Attribute(defvalue: "", category: "Forest", desc: "Curve defining level's outline scaling; from inside (left, forest core) to outside (right, forest outline)", uiwidget: UIWidgets.GraphDialog, params: "100 " + (SCALE_CURVE_MAX_VALUE - SCALE_CURVE_MIN_VALUE) + " 0 " + SCALE_CURVE_MIN_VALUE)]
	ref Curve m_aOutlineScaleCurve;

	[Attribute(defvalue: "0", category: "Forest", desc: "Distance from shape over which the scaling occurs [m]", uiwidget: UIWidgets.Slider, params: "0 100 0.1")]
	float m_fOutlineScaleCurveDistance;

	SCR_EForestGeneratorLevelType m_eType;
	ref array<float> m_aGroupProbas = {};

	static const float SCALE_CURVE_MAX_VALUE = 1;
	static const float SCALE_CURVE_MIN_VALUE = 0;
}

[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class ForestGeneratorOutline : ForestGeneratorLevel
{
	[Attribute(defvalue: "1", uiwidget: UIWidgets.SpinBox, desc: "How separated are the tree groups")]
	float m_fClusterStrength;

	[Attribute(defvalue: "15", uiwidget: UIWidgets.SpinBox, desc: "In what radius should trees around be taken into account for clusters [m]", params: "0 inf")]
	float m_fClusterRadius;

	[Attribute("0", UIWidgets.ComboBox, enumType: SCR_EForestGeneratorOutlineType, desc: "Outline type")]
	SCR_EForestGeneratorOutlineType m_eOutlineType;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.SpinBox, desc: "Minimal distance from spline [m]", params: "0 inf")]
	float m_fMinDistance;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.SpinBox, desc: "Maximal distance from spline [m]", params: "0 inf")]
	float m_fMaxDistance;

	//------------------------------------------------------------------------------------------------
	// constructor
	void ForestGeneratorOutline()
	{
		m_eType = SCR_EForestGeneratorLevelType.OUTLINE;
	}
}

//! intermediate class for Top and Bottom (tree) levels
[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class SCR_ForestGeneratorTreeLevel : ForestGeneratorLevel
{
	[Attribute(defvalue: "0", desc: "Allow this forest level to overlap and be generated in the outline area; can be positive or negative")]
	float m_fOutlineOverlap;
}

[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class ForestGeneratorTopLevel : SCR_ForestGeneratorTreeLevel
{
	[Attribute(defvalue: "1", uiwidget: UIWidgets.SpinBox, desc: "How separated are the tree groups?")]
	float m_fClusterStrength;

	[Attribute(defvalue: "15", uiwidget: UIWidgets.SpinBox, desc: "In what radius should trees around be taken into account for clusters? [m]", params: "0 inf")]
	float m_fClusterRadius;

	//------------------------------------------------------------------------------------------------
	// constructor
	void ForestGeneratorTopLevel()
	{
		m_eType = SCR_EForestGeneratorLevelType.TOP;
	}
}

[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class ForestGeneratorBottomLevel : SCR_ForestGeneratorTreeLevel
{
	//------------------------------------------------------------------------------------------------
	// constructor
	void ForestGeneratorBottomLevel()
	{
		m_eType = SCR_EForestGeneratorLevelType.BOTTOM;
	}
}

enum SCR_EForestGeneratorLevelType
{
	TOP,
	BOTTOM,
	OUTLINE,
}

enum SCR_EForestGeneratorOutlineType
{
	SMALL,
	MIDDLE,
}
