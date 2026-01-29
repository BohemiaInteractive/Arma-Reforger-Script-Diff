//! Properties exposed in the wall generator property grid after adding a new wall length group
[BaseContainerProps(), SCR_BaseContainerResourceTitleField("m_sWallAsset", "Wall %1")]
class WallWeightPair
{
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Prefab", "et")]
	ResourceName m_sWallAsset;

	[Attribute("1", UIWidgets.EditBox, params: "0 inf")]
	float m_fWeight;

	[Attribute("0", UIWidgets.Slider, params: "-10 10 0.01")]
	float m_fPrePadding;

	[Attribute("0", UIWidgets.Slider, params: "-10 10 0.01")]
	float m_fPostPadding;
}
