[EntityEditorProps(category: "GameScripted/Triggers", description: "")]
class SCR_FactionDominanceTriggerEntityClass: SCR_FactionControlTriggerEntityClass
{
}

class SCR_FactionDominanceTriggerEntity : SCR_FactionControlTriggerEntity
{
	//------------------------------------------------------------------------------------------------
	override protected void OnQueryFinished(bool bIsEmpty)
	{
		//--- Finished evaluation round. Reset variables and start again.
		if (m_bEvaluateResult)
		{
			m_iFriendlyCount = 0;
			m_iEnemyCount = 0;
			m_bEvaluateResult = false;
			return;
		}
		
		m_bResult = false;
		m_bEvaluateResult = true;
		
		float friendlyRatio;
		if (m_iEnemyCount == 0)
			friendlyRatio = 1;
		else
			friendlyRatio = m_iFriendlyCount / Math.Max(m_iFriendlyCount + m_iEnemyCount, 1);
		
		switch (m_iRatioMethod)
		{
			case 0:
			{
				m_bResult = friendlyRatio > m_fFriendlyRatioLimit;
				break;
			}
			case 1:
			{
				m_bResult = float.AlmostEqual(friendlyRatio, m_fFriendlyRatioLimit);
				break;
			}
			case 2:
			{
				m_bResult = friendlyRatio < m_fFriendlyRatioLimit;
				break;
			}
		}
		
		Event_OnQueryFinished.Invoke(this); 
		Event_OnQueryFinished.Clear();
	}
};
