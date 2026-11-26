[BaseContainerProps(configRoot: true), SCR_BaseContainerCustomTitleField("m_sInputFilterClass")]
class SCR_InputFilterConfiguration
{
	[Attribute(defvalue: "InputFilterValue", desc: "Input Filter Class")]
	string m_sInputFilterClass;

	[Attribute(defvalue: "0", desc: "Can Invert")]
	bool m_bCanInvert;

	[Attribute(defvalue: "0", desc: "Can Adjust Multiplier")]
	bool m_bCanAdjustMultiplier;

	[Attribute(defvalue: "0", desc: "Can Adjust Offset")]
	bool m_bCanAdjustOffset;

	[Attribute(defvalue: "0", desc: "Can Adjust Threshold")]
	bool m_bCanAdjustThreshold;

	[Attribute(defvalue: "0", desc: "Can Adjust Hold Duration")]
	bool m_bCanAdjustHoldDuration;

	[Attribute(defvalue: "0", desc: "Can Adjust Click Duration")]
	bool m_bCanAdjustClickDuration;

	[Attribute(defvalue: "0", desc: "Can Adjust Deadzones")]
	bool m_bCanAdjustDeadzones;
}
