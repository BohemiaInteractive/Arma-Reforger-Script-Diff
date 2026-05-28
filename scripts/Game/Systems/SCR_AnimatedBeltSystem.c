/*
System to move any ammobelt UV's that are currently being fired
*/

class SCR_AnimatedBeltSystem : GameSystem
{
	override static void InitInfo(WorldSystemInfo outInfo)
	{
		outInfo
			.SetAbstract(false)
			.AddPoint(ESystemPoint.Frame);
	}

	protected ref array<SCR_AnimatedBeltComponent> m_aComponents = {};
	protected ref array<SCR_AnimatedBeltComponent> m_aComponentsToAdd = {};
	protected ref array<SCR_AnimatedBeltComponent> m_aComponentsToDel = {};
	private bool m_bSimulationRunning;

	//------------------------------------------------------------------------------------------------
	protected override void OnInit()
	{
		Enable(false);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnUpdatePoint(WorldUpdatePointArgs args)
	{
		float timeSlice = args.GetTimeSliceSeconds();
		bool nullValuePresent;

		m_bSimulationRunning = true;
		foreach (SCR_AnimatedBeltComponent comp : m_aComponents)
		{
			if (!comp)
			{
				nullValuePresent = true;
				continue;
			}
			
			comp.Update(timeSlice);
		}
		m_bSimulationRunning = false;
		
		// Remove any null referenes
		if (nullValuePresent)
		{
			for (int i = m_aComponents.Count() - 1; i >= 0; i--)
			{
				if (!m_aComponents[i])
					m_aComponents.Remove(i);
			}
		}

		// Simulation stopped running, time to update m_aComponents if necessary
		foreach (SCR_AnimatedBeltComponent comp : m_aComponentsToAdd)
		{
			if (comp)
				RegisterInternal(comp);
		}
		foreach (SCR_AnimatedBeltComponent comp : m_aComponentsToDel)
		{
			if (comp)
				UnregisterInternal(comp);
		}
		m_aComponentsToAdd.Clear();
		m_aComponentsToDel.Clear();
	}

	//------------------------------------------------------------------------------------------------
	private void RegisterInternal(notnull SCR_AnimatedBeltComponent component)
	{
		int index = component.GetBeltSystemIdx();
		if (index >= 0)
			return;
		
		index = m_aComponents.Insert(component);
		component.SetBeltSystemIdx(index);

		if (!IsEnabled())
			Enable(true);
	}

	//------------------------------------------------------------------------------------------------
	void Register(notnull SCR_AnimatedBeltComponent component)
	{
		if (m_bSimulationRunning)
		{
			m_aComponentsToAdd.Insert(component);
			return;
		}
		
		RegisterInternal(component);
	}

	//------------------------------------------------------------------------------------------------
	private void UnregisterInternal(notnull SCR_AnimatedBeltComponent component)
	{
		int index = component.GetBeltSystemIdx();
		if (index < 0)
			return;
		
		// Assign the last item in the array out current system index if possible
		if (!m_aComponents.IsEmpty() && m_aComponents[m_aComponents.Count()-1] != component)
			m_aComponents[m_aComponents.Count()-1].SetBeltSystemIdx(index);
		// Swap indices with the last item in the array
		m_aComponents.Remove(index);
		// Invalidate current component's index
		component.SetBeltSystemIdx(-1);
		
		if (m_aComponents.IsEmpty())
			Enable(false);
	}

	//------------------------------------------------------------------------------------------------
	void Unregister(notnull SCR_AnimatedBeltComponent component)
	{
		if (m_bSimulationRunning)
		{
			m_aComponentsToDel.Insert(component);
			return;
		}
		
		UnregisterInternal(component);
	}
}
