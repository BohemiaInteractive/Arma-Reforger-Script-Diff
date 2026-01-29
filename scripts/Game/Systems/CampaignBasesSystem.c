class CampaignBasesSystem : GameSystem
{
	override static void InitInfo(WorldSystemInfo outInfo)
	{
		outInfo
			.SetAbstract(false)
			.AddPoint(ESystemPoint.FixedFrame);
	}

	protected ref array<SCR_CampaignMilitaryBaseComponent> m_Components = {};
	
	//------------------------------------------------------------------------------------------------
	override protected void OnUpdatePoint(WorldUpdatePointArgs args)
	{
		float timeSlice = args.GetTimeSliceSeconds();
		
		foreach (SCR_CampaignMilitaryBaseComponent comp: m_Components)
		{
			comp.Update(timeSlice);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnDiag(float timeSlice)
	{
		DbgUI.Begin("CampaignBasesSystem");
		
		DbgUI.Text("Items: " + m_Components.Count());
		
		if (DbgUI.Button("Dump active components"))
		{
			foreach (SCR_CampaignMilitaryBaseComponent comp: m_Components)
			{
				Print(comp.GetOwner(), LogLevel.ERROR);
			}
		}
		
		DbgUI.End();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param component must not be null
	void Register(SCR_CampaignMilitaryBaseComponent component)
	{
		//About to be deleted
		if (component.GetOwner().IsDeleted() || (component.GetOwner().GetFlags() & EntityFlags.USER5))
			return;
		
		if (m_Components.Find(component) != -1)
			return;
		
		m_Components.Insert(component);
	}
	
	//------------------------------------------------------------------------------------------------
	void Unregister(SCR_CampaignMilitaryBaseComponent component)
	{
		int idx = m_Components.Find(component);
		if (idx == -1)
			return;
		
		m_Components.Remove(idx);
	}
}
