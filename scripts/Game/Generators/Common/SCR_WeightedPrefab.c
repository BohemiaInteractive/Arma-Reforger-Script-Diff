[BaseContainerProps(), SCR_BaseContainerResourceTitleField("m_sPrefab")]
class SCR_WeightedPrefab
{
	[Attribute(desc: "Prefab to weigh in", uiwidget: UIWidgets.ResourcePickerThumbnail, params: "et")]
	ResourceName m_sPrefab;

	[Attribute(defvalue: "1", desc: "Weight\n0 = disabled\n1 = normal weight\n> 1 = greater priority", params: "0 inf 0.001")]
	float m_fWeight;
}
