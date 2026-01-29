[BaseContainerProps()]
class SCR_BuildingLinkRegion
{
	[Attribute("", UIWidgets.EditBox, "Index of the parent region")]
	int m_iParent;
	[Attribute("", UIWidgets.EditBox, "List of indexes of the child regions")]
	ref array<int> m_aChildren;
};
