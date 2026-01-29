class SCR_HelicopterEngineAction : SCR_EngineAction
{
	//! Maximum animation signal phase
	[Attribute(defvalue: "3.0", desc: "Maximum animation signal phase to be reached within ActionDuration")]
	protected float m_fEngineTogglePhase;

	protected SignalsManagerComponent m_SignalManager;
	protected int m_iStartingSequenceSignal;
	protected float m_fStartingSequencePhase;

	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.Init(pOwnerEntity, pManagerComponent);

		m_SignalManager = SignalsManagerComponent.Cast(pOwnerEntity.FindComponent(SignalsManagerComponent));
		if (!m_SignalManager)
			return;
		m_iStartingSequenceSignal = m_SignalManager.AddOrFindSignal("startingSequence");
	}
	
	override bool CanBeShownScript(IEntity user)
	{
		return m_SignalManager && super.CanBeShownScript(user);
	}
	
	//------------------------------------------------------------------------------------------------
	override void PerformContinuousAction(IEntity pOwnerEntity, IEntity pUserEntity, float timeSlice)
	{
		float duration = Math.AbsFloat(GetActionDuration());
		if (duration > 0)
		{
			m_fStartingSequencePhase += m_fEngineTogglePhase * timeSlice / duration;
			m_fStartingSequencePhase = Math.Min(m_fStartingSequencePhase, m_fEngineTogglePhase);
		}
		else
		{
			m_fStartingSequencePhase = m_fEngineTogglePhase;
		}
		
		if (m_SignalManager)
		{
			if (m_bTargetState) 
				m_SignalManager.SetSignalValue(m_iStartingSequenceSignal, m_fStartingSequencePhase);
			else
				m_SignalManager.SetSignalValue(m_iStartingSequenceSignal, m_fEngineTogglePhase - m_fStartingSequencePhase);
		}
		
		if (float.AlmostEqual(m_fStartingSequencePhase, m_fEngineTogglePhase))
			SetState(m_bTargetState);
	}
	
	override void OnActionStart(IEntity pUserEntity)
	{
		if (m_SignalManager)
			m_fStartingSequencePhase = m_SignalManager.GetSignalValue(m_iStartingSequenceSignal);
		
		if (!m_bTargetState)
			m_iStartingSequenceSignal = m_fEngineTogglePhase - m_fStartingSequencePhase;
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetState(bool enable)
	{
		super.SetState(enable);

		if (!m_SignalManager)
			return;
		
		if (enable)
			m_SignalManager.SetSignalValue(m_iStartingSequenceSignal, m_fEngineTogglePhase);
		else
			m_SignalManager.SetSignalValue(m_iStartingSequenceSignal, 0);
	}
};
