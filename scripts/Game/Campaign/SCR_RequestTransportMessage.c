//------------------------------------------------------------------------------------------------
//class SCR_RequestTransportMessage : SCR_RequestMessage
//{
//	vector m_vTargetPosition;
//	
//	//------------------------------------------------------------------------------------------------
//	void SetTargetPosition(vector targetPosition)
//	{
//		m_vTargetPosition = targetPosition;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	override void OnDelivery(BaseTransceiver receiver, int freq, float quality)
//	{
//		if (m_RequesterMainBase && receiver.GetRadio() && receiver.GetRadio().GetOwner() == m_RequesterMainBase && GetTaskManager())
//		{
//			SCR_TransportTaskSupportEntity supportEntity = SCR_TransportTaskSupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_TransportTaskSupportEntity));
//			if (supportEntity)
//				supportEntity.RequestTransport(m_iRequesterID, m_vPosition, m_vTargetPosition);
//		}
//	}
//};
