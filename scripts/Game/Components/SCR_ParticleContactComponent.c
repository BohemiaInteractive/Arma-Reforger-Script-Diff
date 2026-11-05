class SCR_ParticleContactComponentClass : ParticleContactComponentClass
{
	[Attribute(category: "Sound")]
	ref SCR_AudioSourceConfiguration m_AudioSourceConfiguration;
	
	[Attribute("true", desc: "Set surface signal", category: "Sound")]
	bool m_bSurfaceSignal;
}

class SCR_ParticleContactComponent : ParticleContactComponent
{
	//------------------------------------------------------------------------------------------------
	private void PlaySound(IEntity owner, SCR_ParticleContactComponentClass prefabData, Contact contact)
	{
		if (!prefabData.m_AudioSourceConfiguration || !prefabData.m_AudioSourceConfiguration.IsValid())
			return;
		
		SCR_SoundManagerModule soundManager = SCR_SoundManagerModule.GetInstance(owner.GetWorld());
		if (!soundManager)
			return;
				
		// Create audio source
		SCR_AudioSource audioSource = soundManager.CreateAudioSource(owner, prefabData.m_AudioSourceConfiguration, contact.Position);	
		if (!audioSource)
			return;

		// Set surface signal
		if (prefabData.m_bSurfaceSignal)
		{
			GameMaterial material = contact.Material2;
			if (material)
			{
				audioSource.SetSignalValue(SCR_AudioSource.SURFACE_SIGNAL_NAME, material.GetSoundInfo().GetSignalValue());
			}
		}
				
		// Play sound	
		soundManager.PlayAudioSource(audioSource);		
	}
	
	//------------------------------------------------------------------------------------------------			
	override void OnPostInit(IEntity owner)
	{
#ifdef ENABLE_DIAG
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_PARTICLES_CONTACT_COMPONENT, "", "Show Particle Contacts", "Particles");
#endif
	}

	//------------------------------------------------------------------------------------------------
	override void EOnContact(IEntity owner, IEntity other, Contact contact)
	{
		// Get prefab data
		SCR_ParticleContactComponentClass prefabData = SCR_ParticleContactComponentClass.Cast(GetComponentData(owner));
		if (!prefabData)
		{
			ClearEventMask(owner, EntityEvent.CONTACT);
			return;
		}
				
		// Play sound
		PlaySound(owner, prefabData, contact);
		
	#ifdef ENABLE_DIAG
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_PARTICLES_CONTACT_COMPONENT))
		{
			DebugTextWorldSpace.Create(GetOwner().GetWorld(), contact.Material2.GetName(), DebugTextFlags.CENTER, contact.Position[0], contact.Position[1], contact.Position[2]);
		}
	#endif
	}
}
