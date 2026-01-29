//------------------------------------------------------------------------------------------------
//class SCR_RequestRefuelMessage : SCR_RequestMessage
//{
//	private Vehicle m_RequesterVehicle;
//	
//	//------------------------------------------------------------------------------------------------
//	override void OnDelivery(BaseTransceiver receiver, int freq, float quality)
//	{
//		if (m_RequesterMainBase && receiver.GetRadio() && receiver.GetRadio().GetOwner() == m_RequesterMainBase && GetTaskManager())
//		{
//			SCR_RefuelTaskSupportEntity supportClass = SCR_RefuelTaskSupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_RefuelTaskSupportEntity));
//			if (supportClass)
//				supportClass.RequestRefuel(m_iRequesterID, m_RequesterVehicle, m_vPosition);
//		}
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void SetRequesterVehicle(Vehicle vehicle)
//	{
//		m_RequesterVehicle = vehicle;
//	}
//};
