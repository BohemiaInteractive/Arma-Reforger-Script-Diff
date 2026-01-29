class RadialMenuSystem: GameSystem
{
	override static void InitInfo(WorldSystemInfo outInfo)
	{
		outInfo
			.SetAbstract(false)
			.SetUnique(true)
			.SetLocation(WorldSystemLocation.Client)
			.AddPoint(ESystemPoint.Frame);
	}

	protected ref array<SCR_RadialMenuGameModeComponent> m_Components = {};

	override protected void OnUpdatePoint(WorldUpdatePointArgs args)
	{
		float timeSlice = args.GetTimeSliceSeconds();
		
		foreach (SCR_RadialMenuGameModeComponent comp: m_Components)
		{
			comp.Update(timeSlice);
		}
	}
	
	override protected void OnDiag(float timeSlice)
	{
		DbgUI.Begin("RadialMenuSystem");
		
		DbgUI.Text("Items: " + m_Components.Count());
		
		if (DbgUI.Button("Dump active motors components"))
		{
			foreach (SCR_RadialMenuGameModeComponent comp: m_Components)
			{
				Print(comp.GetOwner(), LogLevel.ERROR);
			}
		}
		
		DbgUI.End();
	}
	
	void Register(SCR_RadialMenuGameModeComponent component)
	{
		//About to be deleted
		if (component.GetOwner().IsDeleted() || (component.GetOwner().GetFlags() & EntityFlags.USER5))
			return;
		
		if (m_Components.Find(component) != -1)
			return;
		
		m_Components.Insert(component);
	}
	
	void Unregister(SCR_RadialMenuGameModeComponent component)
	{
		int idx = m_Components.Find(component);
		if (idx == -1)
			return;
		
		m_Components.Remove(idx);
	}
}