//------------------------------------------------------------------------------------------------
// Bindings of C++ helpers for ForestGeneratorEntity
//------------------------------------------------------------------------------------------------

// Do not rename unless it is renamed gamecode-side as well
class ForestGeneratorDistaceAttribute
{
	DistanceType m_iDistanceType;

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] distanceType
	void ForestGeneratorDistaceAttribute(DistanceType distanceType)
	{
		m_iDistanceType = distanceType;
	}
}

class ForestGeneratorGroupIndexAttribute
{
}

//! Used to annotate a vector which signifies the first point of a capsule segment
//! If a member with the ForestGeneratorCapsuleEndAttribute is not specified,
//! this is used as an offset i.e. both ends of the segment are set to this value.
//! Only X and Z fields of the vector are used.
class ForestGeneratorCapsuleStartAttribute
{
}

//! \see ForestGeneratorCapsuleStartAttribute
class ForestGeneratorCapsuleEndAttribute
{
}

class SCR_ForestGeneratorTreeBase : ForestGeneratorTreeBase
{
	[Attribute("0.8", UIWidgets.SpinBox, "Min scale of this object", params: "0 1000 0.01")]
	float m_fMinScale;

	[Attribute("1.2", UIWidgets.SpinBox, "Max scale of this object", params: "0 1000 0.01")]
	float m_fMaxScale;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Prefab of this object", "et")]
	ResourceName m_Prefab;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.SpinBox, "Minimum required radius in the bottom layer for this object to spawn"), ForestGeneratorDistaceAttribute(DistanceType.BOTTOM)];
	float m_fBotDistance;

	[Attribute("0", uiwidget: UIWidgets.SpinBox, "Maximum random pitch angle", "0 180 1")]
	float m_fRandomPitchAngle;

	[Attribute("0", uiwidget: UIWidgets.SpinBox, "Maximum random roll angle", "0 180 1")]
	float m_fRandomRollAngle;

	[Attribute(defvalue: "0", desc: "Fixed vertical offset at which to place this object [m]")]
	float m_fVerticalOffset;

	[ForestGeneratorGroupIndexAttribute()]
	int m_iGroupIndex;

	float m_fScale = 2;
	SCR_ETreeType m_eType;

	//------------------------------------------------------------------------------------------------
	//!
	void AdjustScale();
}
