// Small container to attached to each item in the list box
class SCR_WorkshopAddonPresetListItemData
{
	string m_sPresetName; // Ogirinal name of preset
	
	void SCR_WorkshopAddonPresetListItemData(string name)
	{
		m_sPresetName = name;
	}
}