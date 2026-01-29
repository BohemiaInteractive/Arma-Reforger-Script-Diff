[BaseContainerProps()]
class SCR_CustomTriggerConditionsFactionControl : SCR_CustomTriggerConditions
{	
	[Attribute(desc: "Factions triggering control over trigger radius")]
	protected ref array<FactionKey> m_aControlFactionKeys;
	
	[Attribute(desc: "Ignored factions, not counting towards control check")]
	protected ref array<FactionKey> m_aIgnoredFactionKeys;
	
	[Attribute(uiwidget: UIWidgets.ComboBox, enumType: SCR_EScenarioFrameworkComparisonOperator)]
	protected SCR_EScenarioFrameworkComparisonOperator m_eComparisonOperator;
	
	[Attribute("0.5", UIWidgets.Slider, "Control faction ratio over rest of checked factions", params: "0 1 0.1")]
	protected float m_fControlRatio;
	
	[Attribute(desc: "(optional) Custom Trigger getter")]
	protected ref SCR_ScenarioFrameworkGet m_TriggerGetter;
	
	protected SCR_FactionAliasComponent m_FactionAliasComponent;
	//------------------------------------------------------------------------------------------------
	//! Performs specified logic checks when called
	//! \param[in] trigger
	override bool Init(IEntity entity)
	{
		if (!m_aControlFactionKeys || m_aControlFactionKeys.IsEmpty())
		{
			PrintFormat("SCR_CustomTriggerConditionsFactionControl: No control faction keys provided for condition %1.", this, level: LogLevel.ERROR);
			return false;
		}
		
		if (!m_FactionAliasComponent)
		{
			FactionManager factionManager = GetGame().GetFactionManager();
			if (factionManager)
				m_FactionAliasComponent = SCR_FactionAliasComponent.Cast(GetGame().GetFactionManager().FindComponent(SCR_FactionAliasComponent));
		}
		
		if (m_TriggerGetter)
		{
			SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_TriggerGetter.Get());
			if (!entityWrapper)
			{
				PrintFormat("ScenarioFramework Action: Issue with Getter detected for Condition %1.", this, level:LogLevel.ERROR);
				return false;
			}
			
			entity = entityWrapper.GetValue();
		}
		
		SCR_ScenarioFrameworkTriggerEntity trigger = SCR_ScenarioFrameworkTriggerEntity.Cast(entity);
		if (!trigger)
		{
			PrintFormat("SCR_CustomTriggerConditionsFactionControl: Provided entity is not SCR_ScenarioFrameworkTriggerEntity for condition %1.", this, level: LogLevel.ERROR);
			return false;
		}
		
		array <IEntity> entities = {};
		trigger.GetEntitiesInside(entities);
		int controlFactionCount, opposingFactionCount;
		FactionAffiliationComponent factionComp;
		FactionKey factionKey;
		
		foreach (IEntity ent : entities)
		{
			factionComp = FactionAffiliationComponent.Cast(ent.FindComponent(FactionAffiliationComponent));
			if (!factionComp)
				continue;
			
			factionKey = factionComp.GetAffiliatedFactionKey();
			//Check, if faction is ignored first
			if (m_aIgnoredFactionKeys && CheckFactionKey(factionKey, m_aIgnoredFactionKeys))
				continue;
			
			if (CheckFactionKey(factionKey, m_aControlFactionKeys))
				controlFactionCount++;
			else
				opposingFactionCount++;
		}
		
		float ratio = controlFactionCount / Math.Max(controlFactionCount + opposingFactionCount, 1);
		
		return (
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.LESS_THAN) 			&& (ratio < m_fControlRatio)) 	||
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.LESS_OR_EQUAL) 		&& (ratio <= m_fControlRatio)) 	||
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.EQUAL) 				&& (ratio == m_fControlRatio)) 	||
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.GREATER_OR_EQUAL) 	&& (ratio >= m_fControlRatio)) 	||
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.GREATER_THEN) 		&& (ratio > m_fControlRatio)));
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool CheckFactionKey(FactionKey factionKey, notnull array<FactionKey> arrayToCheck)
	{	
		if (m_FactionAliasComponent)
		{
			array<string> aliases = m_FactionAliasComponent.GetAllFactionKeyAliases(factionKey);
			foreach (string alias : aliases)
			{
				if (arrayToCheck.Contains(alias))
					return true;
			}
		}
		
		return arrayToCheck.Contains(factionKey);
	}
}