class SCR_ScenarioFrameworkNotorietyComponentClass : SCR_BaseGameModeComponentClass
{
}

class SCR_ScenarioFrameworkNotorietyComponent : SCR_BaseGameModeComponent
{
	[Attribute(desc: "Starting notoriety value", params: "0 inf 0.01")]
	protected float m_fNotoriety;
	
	[Attribute(defvalue: "1", desc: "Time between updates", params: "1 inf")]
	protected float m_fUpdateDelay;
	
	[Attribute(defvalue: "1", desc: "Base Notoriety Loss with each update", params: "0 inf 0.01")]
	protected float m_fBaseNotorietyLoss;
	
	[Attribute(desc: "Alert Levels")]
	protected ref array<ref SCR_ScenarioFrameworkNotorietyLevel> m_aNotorietyLevels;
	
	[Attribute(defvalue: "1", desc: "Alert Enabled")]
	bool m_bAlertEnabled;
	
	protected SCR_ScenarioFrameworkNotorietyLevel m_CurrentAlertState;
	
	//------------------------------------------------------------------------------------------------
	static SCR_ScenarioFrameworkNotorietyComponent GetInstance()
	{
		BaseGameMode gamemode = GetGame().GetGameMode();
		if (!gamemode)
			return null;
		
		return SCR_ScenarioFrameworkNotorietyComponent.Cast(gamemode.FindComponent(SCR_ScenarioFrameworkNotorietyComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	float GetNotorietyValue()
	{
		return m_fNotoriety;
	}
	
	//------------------------------------------------------------------------------------------------
	void AddNotoriety(float value)
	{
		m_fNotoriety += value;
	}
	
	//------------------------------------------------------------------------------------------------
	void EvaluateNotoriety()
	{
		if (!m_bAlertEnabled)
		{
			m_CurrentAlertState = null;
			return;
		}
		
		if (!m_aNotorietyLevels)
			return;
		
		if (m_CurrentAlertState)
			m_fNotoriety -= m_fBaseNotorietyLoss * m_CurrentAlertState.m_fDeteriorationMultiplier;
		else if (m_fNotoriety > 0)
			m_fNotoriety -= m_fBaseNotorietyLoss;
		
		SCR_ScenarioFrameworkNotorietyLevel newAlertState;
		
		for (int i, count = m_aNotorietyLevels.Count(); i < count; i++)
		{
			if (m_aNotorietyLevels[i] && m_fNotoriety >= m_aNotorietyLevels[i].m_iAlertMeterThreshold)
			{
				newAlertState = m_aNotorietyLevels[i];
				continue;
			}
			
			break;
		}
		
		if (newAlertState == m_CurrentAlertState)
			return;
		
		if ((!newAlertState || (m_CurrentAlertState && newAlertState.m_iAlertMeterThreshold < m_CurrentAlertState.m_iAlertMeterThreshold)) && m_CurrentAlertState.m_aActionsDecrease)
		{
			foreach (SCR_ScenarioFrameworkActionBase action : m_CurrentAlertState.m_aActionsDecrease)
			{
				action.OnActivate(null);
			}
		}
		
		if (newAlertState && newAlertState.m_aActions)
		{
			foreach (SCR_ScenarioFrameworkActionBase action : newAlertState.m_aActions)
			{
				action.OnActivate(null);
			}
		}
		
		m_CurrentAlertState = newAlertState;
	}
	
	//------------------------------------------------------------------------------------------------
	array<ref SCR_ScenarioFrameworkActionBase> GetAllStateActions()
	{
		array<ref SCR_ScenarioFrameworkActionBase> m_aSubActions = {};
		
		foreach (SCR_ScenarioFrameworkNotorietyLevel alertState : m_aNotorietyLevels)
		{
			if (alertState.m_aActions)
			{
				foreach (SCR_ScenarioFrameworkActionBase action : alertState.m_aActions)
				{
					m_aSubActions.Insert(action);
				}
			}
			
			if (alertState.m_aActions)
			{
				foreach (SCR_ScenarioFrameworkActionBase action : alertState.m_aActionsDecrease)
				{
					m_aSubActions.Insert(action);
				}
			}
		}
		
		return m_aSubActions;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnPostInit(IEntity owner)
	{
		if (!GetGame().InPlayMode())
			return;
		
		SetEventMask(owner, EntityEvent.INIT);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void EOnInit(IEntity owner)
	{
		SCR_ScenarioFrameworkSystem.GetCallQueuePausable().CallLater(EvaluateNotoriety, m_fUpdateDelay*1000, true);
		EvaluateNotoriety();
	}
}