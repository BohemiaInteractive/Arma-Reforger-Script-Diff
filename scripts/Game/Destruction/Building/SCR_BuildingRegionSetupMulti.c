[BaseContainerProps()]
class SCR_BuildingRegionSetupMulti : SCR_BuildingRegionSetup
{
	[Attribute("", UIWidgets.EditBox, "List of indexes of the regions these settings apply to")]
	ref array<int> m_aIndexes;
};
