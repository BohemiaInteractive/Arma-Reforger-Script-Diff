class HandleUpdateVONControllersSystem: GameSystem
{
	override static void InitInfo(WorldSystemInfo outInfo)
	{
		outInfo
			.SetAbstract(false)
			.AddPoint(ESystemPoint.Frame);
	}

	protected ref array<SCR_VONController> m_Components = {};
	
	override protected void OnUpdatePoint(WorldUpdatePointArgs args)
	{
		float timeSlice = args.GetTimeSliceSeconds();
		
		foreach (SCR_VONController comp: m_Components)
		{
			comp.Update(timeSlice);
		}
	}
	
	override protected void OnDiag(float timeSlice)
	{
		DbgUI.Begin("HandleUpdateVONControllersSystem");
		
		DbgUI.Text("Items: " + m_Components.Count());
		
		if (DbgUI.Button("Dump active components"))
		{
			foreach (SCR_VONController comp: m_Components)
			{
				Print(comp.GetOwner(), LogLevel.ERROR);
			}
		}
		
		DbgUI.End();
	}
	
	void Register(SCR_VONController component)
	{
		//About to be deleted
		if (component.GetOwner().IsDeleted() || (component.GetOwner().GetFlags() & EntityFlags.USER5))
			return;
		
		if (m_Components.Find(component) != -1)
			return;
		
		m_Components.Insert(component);
	}
	
	void Unregister(SCR_VONController component)
	{
		int idx = m_Components.Find(component);
		if (idx == -1)
			return;
		
		m_Components.Remove(idx);
	}
}