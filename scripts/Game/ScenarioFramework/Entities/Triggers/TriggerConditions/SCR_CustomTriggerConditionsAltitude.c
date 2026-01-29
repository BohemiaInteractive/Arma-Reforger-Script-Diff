//Baseclass that is supposed to be extended and filled with custom conditions
[BaseContainerProps()]
class SCR_CustomTriggerConditionsAltitude : SCR_CustomTriggerConditions
{
	[Attribute(desc: "Entity to check.")]
	ref SCR_ScenarioFrameworkGet m_Getter;
	
	[Attribute(desc: "Checked altitude.", category: "Trigger")]
	float m_fAltitude;
	
	[Attribute("0", UIWidgets.ComboBox, "Operator", "", ParamEnumArray.FromEnum(SCR_EScenarioFrameworkComparisonOperator))]
	protected SCR_EScenarioFrameworkComparisonOperator m_eComparisonOperator;
	
	//------------------------------------------------------------------------------------------------
	//! Performs specified logic checks when called
	//! \param[in] trigger
	override bool Init(IEntity entity)
	{
		if (m_bDebug)
			Print("[SCR_CustomTriggerConditionsAltitude.Init] debug line (" + __FILE__ + " L" + __LINE__ + ")", LogLevel.WARNING);
		
		array <IEntity> entities = {};
		if (m_Getter)
		{
			SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
			if (!entityWrapper)
			{
				SCR_ScenarioFrameworkParam<array<IEntity>> entityArrayWrapper = SCR_ScenarioFrameworkParam<array<IEntity>>.Cast(m_Getter.Get());
				if (!entityArrayWrapper)
				{
					PrintFormat("ScenarioFramework Condition: Issue with Getter detected for condition %1.", this, level: LogLevel.ERROR);
					return false;
				}

				entities = entityArrayWrapper.GetValue();
			}
			
			IEntity wrappedEntity = entityWrapper.GetValue();
			if (wrappedEntity)
				entities.Insert(wrappedEntity);
		}
		else
		{
			SCR_ScenarioFrameworkTriggerEntity trigger = SCR_ScenarioFrameworkTriggerEntity.Cast(entity);
			if (!trigger)
				return false;
			
			trigger.GetEntitiesInside(entities);
		}
		
		if (!entities || entities.IsEmpty())
		{
			PrintFormat("ScenarioFramework Condition: Array Getter for %1 has no elements to work with.", this, level: LogLevel.ERROR);
			return false;
		}
		
		float aglHeight;
		foreach (IEntity ent : entities)
		{
			if (!ent)
				continue;
			
			aglHeight = SCR_TerrainHelper.GetHeightAboveTerrain(ent.GetOrigin());
			if (
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.LESS_THAN) 			&& (aglHeight < m_fAltitude)) 	||
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.LESS_OR_EQUAL) 		&& (aglHeight <= m_fAltitude)) 	||
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.EQUAL) 				&& (aglHeight == m_fAltitude)) 	||
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.GREATER_OR_EQUAL) 	&& (aglHeight >= m_fAltitude)) 	||
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.GREATER_THEN) 		&& (aglHeight > m_fAltitude)) 
			)
			return true;
		}
		
		return false;
	}
	
}