[BaseContainerProps(configRoot: true)]
class SCR_EntityTooltipConfig
{
	[Attribute(desc: "List of all enity tooltips that is assigned in the SCR_EntityTooltipEditorUIComponent")]
	protected ref array<ref SCR_EntityTooltipDetailType> m_aDetailTypes;

	//------------------------------------------------------------------------------------------------
	//! Get notification display data
	//! \param[out] data All the notification display data
	//! \return Count of data array
	int GetTooltipData(out notnull array<ref SCR_EntityTooltipDetailType> data)
	{
		data.Clear();
		
		foreach (SCR_EntityTooltipDetailType tooltip: m_aDetailTypes)
			data.Insert(tooltip);
		
		return m_aDetailTypes.Count();
	}
}
