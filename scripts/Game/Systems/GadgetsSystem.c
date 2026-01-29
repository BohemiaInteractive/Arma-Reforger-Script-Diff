class GadgetsSystem : GameSystem
{
	override static void InitInfo(WorldSystemInfo outInfo)
	{
		outInfo
			.SetAbstract(false)
			.AddPoint(ESystemPoint.Frame);
	}

	protected bool m_bUpdating = false;
	protected ref array<SCR_GadgetComponent> m_Components = {};
	protected ref array<SCR_GadgetComponent> m_DeletedComponents = {};
	
	//------------------------------------------------------------------------------------------------
	protected override void OnUpdatePoint(WorldUpdatePointArgs args)
	{
		float timeSlice = args.GetTimeSliceSeconds();
		
		m_bUpdating = true;
		
		foreach (SCR_GadgetComponent comp: m_Components)
		{
			comp.Update(timeSlice);
		}
		
		m_bUpdating = false;
		
		foreach (SCR_GadgetComponent comp: m_DeletedComponents)
		{
			Unregister(comp);
		}
		m_DeletedComponents.Clear();
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnDiag(float timeSlice)
	{
		DbgUI.Begin("GadgetsSystem");
		
		DbgUI.Text("Items: " + m_Components.Count());
		
		if (DbgUI.Button("Dump active components"))
		{
			foreach (SCR_GadgetComponent comp: m_Components)
			{
				Print(comp.GetOwner(), LogLevel.ERROR);
			}
		}
		
		DbgUI.End();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param component must not be null
	void Register(SCR_GadgetComponent component)
	{
		//About to be deleted
		if (component.GetOwner().IsDeleted() || (component.GetOwner().GetFlags() & EntityFlags.USER5))
			return;
		
		if (m_Components.Find(component) != -1)
			return;
		
		m_Components.Insert(component);
	}
	
	//------------------------------------------------------------------------------------------------
	void Unregister(SCR_GadgetComponent component)
	{
		int idx = m_Components.Find(component);
		if (idx == -1)
			return;
		
		if (m_bUpdating)
			m_DeletedComponents.Insert(component);
		else
			m_Components.Remove(idx);
	}
}
