class SCR_ForceImpulseCallback : ScriptedInventoryOperationCallback
{
	vector m_vPosition;
	vector m_vForceOffset;
	vector m_vForceImpulse;
	SCR_HeadgearInventoryItemComponent m_ItemComponent;

	//------------------------------------------------------------------------------------------------
	protected override void OnComplete()
	{
		if (!m_ItemComponent)
			return;

		m_ItemComponent.ApplyForce(m_vPosition, m_vForceOffset, m_vForceImpulse);
	}
}