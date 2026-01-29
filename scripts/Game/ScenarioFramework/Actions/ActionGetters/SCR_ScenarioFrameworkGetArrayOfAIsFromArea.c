[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkGetArrayOfAIsFromArea : SCR_ScenarioFrameworkGet
{
	[Attribute(defvalue: "", desc: "Name of areas")]
	ref array<string> m_aAreasName;
	
	[Attribute(defvalue: "", desc: "Ignored Layers")]
	ref array<string> m_aIgnoredLayers;
	
	protected ref array<IEntity> m_aEntities = {};
	
	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkParamBase Get()
	{
		foreach (string name : m_aAreasName)
		{
			ProcessArea(name);
		}
		
		return new SCR_ScenarioFrameworkParam<array<IEntity>>(m_aEntities);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ProcessArea(string areaName)
	{
		IEntity areaEntity = GetGame().GetWorld().FindEntityByName(areaName);
		if (!areaEntity)
			return;
		
		SCR_ScenarioFrameworkArea area = SCR_ScenarioFrameworkArea.Cast(areaEntity.FindComponent(SCR_ScenarioFrameworkArea));
		if (area)
			ProcessLayerBase(area);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ProcessLayerBase(SCR_ScenarioFrameworkLayerBase layerBase)
	{
		array<SCR_ScenarioFrameworkLayerBase> layerBases = {};
		layerBase.GetChildren(layerBases);
		if (layerBases.IsEmpty())
			return;
			
		foreach (SCR_ScenarioFrameworkLayerBase layer : layerBases)
		{
			ProcessLayerBase(layer);
			
			if (m_aIgnoredLayers && m_aIgnoredLayers.Contains(layer.GetName()))
				continue;
			
			typename type = layer.Type();
			if (type == SCR_ScenarioFrameworkSlotAI || type == SCR_ScenarioFrameworkSlotTaskAI)
				m_aEntities.Insert(layer.GetOwner());
		}
	}
}