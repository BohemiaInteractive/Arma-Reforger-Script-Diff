[BaseContainerProps()]
class SCR_ScenarioFrameworkIsInitialized : SCR_ScenarioFrameworkActivationConditionBase
{
	[Attribute(desc: "Checked layer")]
	ref SCR_ScenarioFrameworkGetLayerBase m_LayerBaseGetter;
	
	//------------------------------------------------------------------------------------------------
	override bool Init(IEntity entity)
	{
		if (m_bDebug)
			Print("[SCR_ScenarioFrameworkIsInitialized.Init] debug line (" + __FILE__ + " L" + __LINE__ + ")", LogLevel.WARNING);
		
		if (!m_LayerBaseGetter)
			return false;
		
		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_LayerBaseGetter.Get());
		if (!entityWrapper)
		{
			PrintFormat("ScenarioFramework Condition: Entity A not found for condition %1.", this, level:LogLevel.ERROR);
			return false;
		}
		
		IEntity wrappedEntity = entityWrapper.GetValue();
		if (!wrappedEntity)
		{
			PrintFormat("ScenarioFramework Condition: Entity A not found for condition %1.", this, level:LogLevel.ERROR);
			return false;
		}
		
		SCR_ScenarioFrameworkLayerBase layerBase = SCR_ScenarioFrameworkLayerBase.Cast(wrappedEntity.FindComponent(SCR_ScenarioFrameworkLayerBase));
		if (layerBase)
			return layerBase.GetIsInitiated();
		
		return false;
	}
}