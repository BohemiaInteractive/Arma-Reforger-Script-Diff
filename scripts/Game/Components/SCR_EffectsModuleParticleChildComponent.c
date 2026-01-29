[ComponentEditorProps(category: "GameScripted/Misc", description: "")]
class SCR_EffectsModuleParticleChildComponentClass : SCR_EffectsModuleChildComponentClass
{
}

class SCR_EffectsModuleParticleChildComponent : SCR_EffectsModuleChildComponent
{
	[Attribute("Trigger", desc: "Signal for particle sfx")]
	protected string m_sParticleSoundEffectSignal;
	
	protected SCR_ReplicatedParticleEffectEntity m_ParticleEntity;
	
	//------------------------------------------------------------------------------------------------
	//~ When particle state changes update the audio
	protected void OnParticleStateChanged(EParticleEffectState oldState, EParticleEffectState newState)
	{
		SignalsManagerComponent signalManager = SignalsManagerComponent.Cast(GetOwner().FindComponent(SignalsManagerComponent));
		if (!signalManager)
			return;
		
		int index = signalManager.AddOrFindSignal(m_sParticleSoundEffectSignal);
		if (index < 0)
			return;
		
		//~ Play sound if playing state. Otherwise always stop playing
		signalManager.SetSignalValue(index, newState == EParticleEffectState.PLAYING);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void EditorOnParentRemoved(SCR_EffectsModuleComponent effectModule, SCR_EffectsModule effectConfig)
	{
		if (!m_ParticleEntity)
			delete GetOwner();
		
		RemovedFromParentBroadcast();
		Rpc(RemovedFromParentBroadcast);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RemovedFromParentBroadcast()
	{
		if (!m_ParticleEntity)
			return;
		
		m_ParticleEntity.StopEmission();
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		m_ParticleEntity = SCR_ReplicatedParticleEffectEntity.Cast(GetOwner());
		if (m_ParticleEntity)
		{
			m_ParticleEntity.GetOnStateChanged().Insert(OnParticleStateChanged);
			
			//~ Start playing Audio if state is already set to playing
			if (m_ParticleEntity.GetState() == EParticleEffectState.PLAYING)
				OnParticleStateChanged(EParticleEffectState.STOPPED, EParticleEffectState.PLAYING);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		if (m_ParticleEntity)
		{
			m_ParticleEntity.GetOnStateChanged().Remove(OnParticleStateChanged);
		}
		super.OnDelete(owner);
	}
}
