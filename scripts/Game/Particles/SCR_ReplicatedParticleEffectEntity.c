[EntityEditorProps(category: "GameScripted/Particles", description: "")]
class SCR_ReplicatedParticleEffectEntityClass : ReplicatedParticleEffectEntityClass
{
}

class SCR_ReplicatedParticleEffectEntity : ReplicatedParticleEffectEntity
{
	protected ref ScriptInvokerInt2 m_OnStateChanged;
	
	//------------------------------------------------------------------------------------------------
	protected override void OnStateChanged(EParticleEffectState oldState, EParticleEffectState newState)
	{
		if (!m_OnStateChanged)
			return;
		
		m_OnStateChanged.Invoke(oldState, newState);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Get scriptinvoker on state changed. First int oldState and second int newState
	ScriptInvokerInt2 GetOnStateChanged()
	{
		if (!m_OnStateChanged)
			m_OnStateChanged = new ScriptInvokerInt2();
		
		return m_OnStateChanged;
	}
}
