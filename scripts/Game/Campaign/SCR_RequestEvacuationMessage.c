//------------------------------------------------------------------------------------------------
//class SCR_RequestEvacuationMessage : SCR_RequestMessage
//{
//	//------------------------------------------------------------------------------------------------
//	override void OnDelivery(BaseTransceiver receiver, int freq, float quality)
//	{
//		if (m_RequesterMainBase && receiver.GetRadio() && receiver.GetRadio().GetOwner() == m_RequesterMainBase && GetTaskManager())
//		{
//			SCR_EvacuateTaskSupportEntity supportEntity = SCR_EvacuateTaskSupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_EvacuateTaskSupportEntity));
//				if (supportEntity)
//					supportEntity.RequestEvacuation(m_iRequesterID, m_vPosition);
//		}
//	}
//};
