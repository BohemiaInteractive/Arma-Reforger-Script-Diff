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
		
		foreach (SCR_AnimatedBeltComponent comp : m_aComponents)
		{
			if (!comp)
			{
				nullValuePresent = true;
				continue;
			}
			
			comp.Update(timeSlice);
		}
		
		if (nullValuePresent)
		{
			for (int i = m_aComponents.Count() - 1; i >= 0; i--)
			{
				if (!m_aComponents[i])
					m_aComponents.Remove(i);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void Register(SCR_AnimatedBeltComponent component)
	{
		if (!component)
			return;
		
		if (!m_aComponents.Contains(component))
			m_aComponents.Insert(component);
		
		if (!IsEnabled())
			Enable(true);
	}

	//------------------------------------------------------------------------------------------------
	void Unregister(SCR_AnimatedBeltComponent component)
	{
		m_aComponents.RemoveItem(component);
		if (m_aComponents.IsEmpty())
			Enable(false);
	}
}
