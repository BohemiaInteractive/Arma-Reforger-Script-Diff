//! TODO: When systems can be replicated, just move overything from SCR_NotificationsComponent
class NotificationsSystem: GameSystem
{
	override static void InitInfo(WorldSystemInfo outInfo)
	{
		outInfo
			.SetAbstract(false)
			.AddPoint(ESystemPoint.Frame);
	}

	protected bool m_bUpdating = false;
	protected ref array<SCR_NotificationsComponent> m_Components = {};
	protected ref array<SCR_NotificationsComponent> m_DeletedComponents = {};
	
	override protected void OnUpdatePoint(WorldUpdatePointArgs args)
	{
		float timeSlice = args.GetTimeSliceSeconds();
		
		m_bUpdating = true;
		
		foreach (SCR_NotificationsComponent comp: m_Components)
		{
			comp.Update(timeSlice);
		}
		
		m_bUpdating = false;
		
		foreach (SCR_NotificationsComponent comp: m_DeletedComponents)
		{
			Unregister(comp);
		}
		m_DeletedComponents.Clear();
	}
	
	override protected void OnDiag(float timeSlice)
	{
		DbgUI.Begin("NotificationsSystem");
		
		DbgUI.Text("Items: " + m_Components.Count());
		
		if (DbgUI.Button("Dump active components"))
		{
			foreach (SCR_NotificationsComponent comp: m_Components)
			{
				Print(comp.GetOwner(), LogLevel.ERROR);
			}
		}
		
		DbgUI.End();
	}
	
	void Register(SCR_NotificationsComponent component)
	{
		//About to be deleted
		if (component.GetOwner().IsDeleted() || (component.GetOwner().GetFlags() & EntityFlags.USER5))
			return;
		
		if (m_Components.Find(component) != -1)
			return;
		
		m_Components.Insert(component);
	}
	
	void Unregister(SCR_NotificationsComponent component)
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