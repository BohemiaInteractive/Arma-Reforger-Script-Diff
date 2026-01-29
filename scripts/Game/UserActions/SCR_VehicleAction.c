class SCR_VehicleAction: SCR_VehicleActionBase
{
	//! Name of signal for pairing with the user action
	[Attribute( uiwidget: UIWidgets.EditBox, desc: "Signal name")]
	private string m_sSignalName;
	private int m_iSignalIndex = -1;

	//! Signal manager to pass signals into proc anim
	private SignalsManagerComponent m_SignalsManager;

	//! Can this action be performed by the provided user entity?
	override bool CanBePerformedScript(IEntity user)
	{
		if (!m_SignalsManager) // Do nothing if there is no signal manager
			return false;

		if (m_iSignalIndex < 0)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_SignalsManager = SignalsManagerComponent.Cast(pOwnerEntity.FindComponent(SignalsManagerComponent));
		if (m_SignalsManager)
			m_iSignalIndex = m_SignalsManager.AddOrFindMPSignal(m_sSignalName, 0.5, 1, 0, SignalCompressionFunc.Range01);
	}

	//------------------------------------------------------------------------------------------------
	override bool GetState()
	{
		return m_SignalsManager && m_SignalsManager.GetSignalValue(m_iSignalIndex) > 0.5;
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetState(bool enable)
	{
		if (!m_SignalsManager)
			return;
		
		float targetValue = 0;
		if (enable)
			targetValue = 1;
		
		m_SignalsManager.SetSignalValue(m_iSignalIndex, targetValue);
	}
};