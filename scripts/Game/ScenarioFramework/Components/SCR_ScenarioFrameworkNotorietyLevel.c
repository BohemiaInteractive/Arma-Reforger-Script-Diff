[BaseContainerProps()]
class SCR_ScenarioFrameworkNotorietyLevel
{
	[Attribute(defvalue: "0", desc: "Alert Meter Threshold", params: "0 inf")]
	int m_iAlertMeterThreshold;
	
	[Attribute(defvalue: "1", desc: "Notoriety deteroriation multiplier", params: "0 inf 0.01")]
	float m_fDeteriorationMultiplier;
	
	[Attribute(desc: "Actions to launch when threshold is reached")]
	ref array<ref SCR_ScenarioFrameworkActionBase> m_aActions;
	
	[Attribute(desc: "Actions to launch when level is Decreased")]
	ref array<ref SCR_ScenarioFrameworkActionBase> m_aActionsDecrease;
}