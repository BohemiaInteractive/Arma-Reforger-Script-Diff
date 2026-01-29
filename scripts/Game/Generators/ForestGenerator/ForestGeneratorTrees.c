[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class ForestGeneratorTree : SCR_ForestGeneratorTreeBase
{
	[Attribute(defvalue: "1", uiwidget: UIWidgets.SpinBox, params: "0 inf 0.001", precision: 3, desc: "Random selection weight")]
	float m_fWeight;

	int m_iDebugGroupIndex;
}

[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class ForestGeneratorTreeShort : ForestGeneratorTree
{
	//------------------------------------------------------------------------------------------------
	override void AdjustScale()
	{
		m_fBotDistance *= m_fScale;
	}
}

[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class ForestGeneratorTreeMiddle : ForestGeneratorTree
{
	[Attribute(defvalue: "5", uiwidget: UIWidgets.SpinBox, "Minimum required radius in the middle layer for this object to spawn"), ForestGeneratorDistaceAttribute(DistanceType.MID)]
	float m_fMidDistance;

	//------------------------------------------------------------------------------------------------
	override void AdjustScale()
	{
		m_fBotDistance *= m_fScale;
		m_fMidDistance *= m_fScale;
	}
}

[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class ForestGeneratorTreeTall : ForestGeneratorTree
{
	[Attribute(defvalue: "5", uiwidget: UIWidgets.SpinBox, "Minimum required radius in the middle layer for this object to spawn", params: "0 inf"), ForestGeneratorDistaceAttribute(DistanceType.MID)]
	float m_fMidDistance;

	[Attribute(defvalue: "10", uiwidget: UIWidgets.SpinBox, "Minimum required radius in the top layer for this object to spawn", params: "0 inf"), ForestGeneratorDistaceAttribute(DistanceType.TOP)]
	float m_fTopDistance;

	//------------------------------------------------------------------------------------------------
	override void AdjustScale()
	{
		m_fBotDistance *= m_fScale;
		m_fMidDistance *= m_fScale;
		m_fTopDistance *= m_fScale;
	}
}

[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class FallenTree : ForestGeneratorTree
{
	[Attribute(desc: "Capsule start, defines offset from the pivot of the object, serves for collision detection")]
	vector m_CapsuleStartInEditor;

	[ForestGeneratorCapsuleStartAttribute()]
	vector m_CapsuleStart;

	[Attribute(desc: "Capsule end, defines offset from the pivot of the object, serves for collision detection")]
	vector m_CapsuleEndInEditor;

	[ForestGeneratorCapsuleEndAttribute()]
	vector m_CapsuleEnd;

	[Attribute(defvalue: "1", desc: "This overrides the setting from template library!")]
	bool m_bAlignToNormal;

	float m_fYaw;
	protected float m_fMinDistanceFromLine = -1;

	//------------------------------------------------------------------------------------------------
	void Rotate()
	{
		m_CapsuleStart = Rotate2D(m_CapsuleStartInEditor * m_fScale, Math.DEG2RAD * m_fYaw);
		m_CapsuleEnd = Rotate2D(m_CapsuleEndInEditor * m_fScale, Math.DEG2RAD * m_fYaw);
	}

	//------------------------------------------------------------------------------------------------
	vector Rotate2D(vector vec, float rads)
	{
		float sin = Math.Sin(rads);
		float cos = Math.Cos(rads);

		return {
			vec[0] * cos - vec[2] * sin,
			0,
			vec[0] * sin + vec[2] * cos
		};
	}

	//------------------------------------------------------------------------------------------------
	float GetMinDistanceFromLine()
	{
		if (m_fMinDistanceFromLine != -1)
			return m_fMinDistanceFromLine;

		float distance = m_CapsuleStart.Length();
		if (distance > m_fMinDistanceFromLine)
			m_fMinDistanceFromLine = distance;

		distance = m_CapsuleEnd.Length();
		if (distance > m_fMinDistanceFromLine)
			m_fMinDistanceFromLine = distance;

		return m_fMinDistanceFromLine;
	}
}
