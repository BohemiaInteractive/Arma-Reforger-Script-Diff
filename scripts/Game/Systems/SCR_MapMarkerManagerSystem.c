//------------------------------------------------------------------------------------------------
class SCR_MapMarkerManagerSystem : GameSystem
{
	override static void InitInfo(WorldSystemInfo outInfo)
	{
		outInfo
			.SetAbstract(false)
			.SetLocation(ESystemLocation.Client)
			.AddPoint(ESystemPoint.PostFrame);
	}

	protected ref array<SCR_MapMarkerManagerComponent> m_Components = {};
	
	//------------------------------------------------------------------------------------------------
	override protected void OnUpdatePoint(WorldUpdatePointArgs args)
	{
		float timeSlice = args.GetTimeSliceSeconds();
		
		foreach (SCR_MapMarkerManagerComponent comp: m_Components)
		{
			comp.Update(timeSlice);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnDiag(float timeSlice)
	{
		DbgUI.Begin("SCR_MapMarkerManagerSystem");
		
		DbgUI.Text("Items: " + m_Components.Count());
		
		if (DbgUI.Button("Dump active components"))
		{
			foreach (SCR_MapMarkerManagerComponent comp: m_Components)
			{
				Print(comp.GetOwner(), LogLevel.ERROR);
			}
		}
		
		DbgUI.End();
	}
	
	//------------------------------------------------------------------------------------------------
	void Register(SCR_MapMarkerManagerComponent component)
	{
		//About to be deleted
		if (component.GetOwner().IsDeleted() || (component.GetOwner().GetFlags() & EntityFlags.USER5))
			return;
		
		if (m_Components.Find(component) != -1)
			return;
		
		m_Components.Insert(component);
	}
	
	//------------------------------------------------------------------------------------------------
	void Unregister(SCR_MapMarkerManagerComponent component)
	{
		int idx = m_Components.Find(component);
		if (idx == -1)
			return;
		
		m_Components.Remove(idx);
	}
}