class SCR_DeployableSpawnPointSystem : GameSystem
{
	override static void InitInfo(WorldSystemInfo outInfo)
	{
		outInfo
			.SetAbstract(false)
			.AddPoint(ESystemPoint.FixedFrame);
	}

	protected ref array<SCR_BaseDeployableSpawnPointComponent> m_aComponents = {};
	
	//------------------------------------------------------------------------------------------------
	override protected void OnUpdatePoint(WorldUpdatePointArgs args)
	{
		float timeSlice = args.GetTimeSliceSeconds();
		
		foreach (SCR_BaseDeployableSpawnPointComponent comp: m_aComponents)
		{
			comp.Update(timeSlice);
		}
	}
	
	override void OnDiag(float timeSlice)
	{
		DbgUI.Begin("SCR_DeployableSpawnPointSystem");
		
		DbgUI.Text("Comps: " + m_aComponents.Count());
		
		if (DbgUI.Button("Dump active components"))
		{
			foreach (SCR_BaseDeployableSpawnPointComponent comp: m_aComponents)
			{
				Print(comp.GetOwner(), LogLevel.ERROR);
			}
		}
		
		DbgUI.End();
	}
	
	void Register(SCR_BaseDeployableSpawnPointComponent component)
	{
		//About to be deleted
		if (component.GetOwner().IsDeleted() || (component.GetOwner().GetFlags() & EntityFlags.USER5))
			return;
		
		if (m_aComponents.Contains(component))
			return;
		
		m_aComponents.Insert(component);
	}
	
	void Unregister(SCR_BaseDeployableSpawnPointComponent component)
	{
		m_aComponents.RemoveItem(component);
	}
}