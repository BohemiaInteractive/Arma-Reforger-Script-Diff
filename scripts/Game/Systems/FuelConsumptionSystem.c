class FuelConsumptionSystem : GameSystem
{
	override static void InitInfo(WorldSystemInfo outInfo)
	{
		outInfo
			.SetAbstract(false)
			.AddPoint(ESystemPoint.FixedFrame);
	}

	protected ref array<SCR_FuelConsumptionComponent> m_Components = {};

	//------------------------------------------------------------------------------------------------
	override bool ShouldBePaused()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnUpdatePoint(WorldUpdatePointArgs args)
	{
		float timeSlice = args.GetTimeSliceSeconds();
		
		foreach (SCR_FuelConsumptionComponent comp: m_Components)
		{
			comp.Update(timeSlice);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnDiag(float timeSlice)
	{
		DbgUI.Begin("FuelConsumptionSystem");
		
		DbgUI.Text("Items: " + m_Components.Count());
		
		if (DbgUI.Button("Dump active components"))
		{
			foreach (SCR_FuelConsumptionComponent comp: m_Components)
			{
				Print(comp.GetOwner(), LogLevel.ERROR);
			}
		}
		
		DbgUI.End();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param component must not be null
	void Register(SCR_FuelConsumptionComponent component)
	{
		//About to be deleted
		if (component.GetOwner().IsDeleted() || (component.GetOwner().GetFlags() & EntityFlags.USER5))
			return;
		
		if (m_Components.Find(component) != -1)
			return;
		
		m_Components.Insert(component);
	}
	
	//------------------------------------------------------------------------------------------------
	void Unregister(SCR_FuelConsumptionComponent component)
	{
		int idx = m_Components.Find(component);
		if (idx == -1)
			return;
		
		m_Components.Remove(idx);
	}
}
