class SCR_AICompositeActionParallel : SCR_AIActionBase
{
	protected ref array<ref AIActionBase> m_aSubactions = {};
	
	void GetSubactions(notnull array<AIActionBase> outSubactions)
	{
		outSubactions.Clear();
		foreach (auto a : m_aSubactions)
			outSubactions.Insert(a);
	}
	
	void AddAction(AIActionBase subaction)
	{
		// Bail if already added
		foreach (auto a : m_aSubactions)
		{
			if (a == subaction)
				return;
		}
		
		m_aSubactions.Insert(subaction);
		
		// If we add an action to an action which is running already, then it must switch to proper state
		if (GetActionState() == EAIActionState.RUNNING)
		{
			subaction.OnActionSelected();
		}
	}
	
	override float CustomEvaluate()
	{
		// Evaluate all subactions, return max priority
		
		float maxPriority = 0;
		int nValidActions = 0;
		
		for (int i = m_aSubactions.Count()-1; i >= 0; i--)
		{
			AIActionBase action = m_aSubactions[i];
			
			EAIActionState state = action.GetActionState();
			
			if (state == EAIActionState.COMPLETED)
			{
				// Remove
				action.OnActionRemoved();
				m_aSubactions.Remove(i);
			}
			else if (state == EAIActionState.FAILED)
			{
				// Remove
				action.OnActionRemoved();
				m_aSubactions.Remove(i);
			}
			else
			{
				float priority = action.Evaluate();
				if (priority > maxPriority)
					maxPriority = priority;
			
				nValidActions++;
			}
		}
		
		// Complete the parallel action once there are no valid actions left
		if (nValidActions == 0)
		{
			Complete();
			return 0;
		}
		
		return maxPriority;
	}
	
	override void OnActionSelected()
	{
		super.OnActionSelected();
		
		foreach (AIActionBase action : m_aSubactions)
		{
			EAIActionState state = action.GetActionState();
			
			if (state == EAIActionState.FAILED || state == EAIActionState.COMPLETED)
				continue;
			
			action.OnActionSelected();
		}
	}
	
	override void OnActionDeselected()
	{
		super.OnActionDeselected();
		
		foreach (AIActionBase action : m_aSubactions)
		{
			EAIActionState state = action.GetActionState();
			
			if (state == EAIActionState.FAILED || state == EAIActionState.COMPLETED)
				continue;
			
			action.OnActionDeselected();
		}
	}
	
	override void OnActionRemoved()
	{
		super.OnActionRemoved();
		foreach (AIActionBase action : m_aSubactions)
		{
			action.OnActionRemoved();
		}
	}
}