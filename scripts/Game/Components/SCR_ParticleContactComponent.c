class SCR_ParticleContactComponentClass : ScriptComponentClass
{
	[Attribute(category: "Sound")]
	ref SCR_AudioSourceConfiguration m_AudioSourceConfiguration;
	
	[Attribute("true", desc: "Set surface signal", category: "Sound")]
	bool m_bSurfaceSignal;
	
	[Attribute("false", desc: "If to play particle effect", category: "VFX")]
	bool m_bPlayParticle;
	
	[Attribute("", UIWidgets.ResourcePickerThumbnail, desc: "Particle effect", params: "ptc", category: "VFX")]
	ResourceName m_Particle;
	
	[Attribute("0", uiwidget: UIWidgets.ComboBox, "Desired type of Game Material effect", "", ParamEnumArray.FromEnum(EParticleEffectInfoType), category: "VFX")]
	int m_iGameMaterialEffect;
	
	[Attribute("0", desc: "Index of Material Effect type", category: "VFX")]
	int m_iEffectIndex;
	
	[Attribute("true", desc: "Disable OnContact after the first contact")]
	bool m_bFirstContactOnly;
	
	[Attribute("false", desc: "Particle oriented to surface", category: "VFX")]
	bool m_bParticleOriented;
}

enum EParticleEffectInfoType
{
	NONE = 0,
	VEHICLE = 1,
	BLAST = 2,
}

class SCR_ParticleContactComponent : ScriptComponent
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
		SCR_ParticleContactComponentClass prefabData = SCR_ParticleContactComponentClass.Cast(GetComponentData(owner));
		if (!prefabData)
			return;
				
		SetEventMask(owner, EntityEvent.CONTACT);
		
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
		
		//Play VFX
		if (prefabData.m_bPlayParticle)
		{
			
			//Play game material effect	
			if (prefabData.m_iGameMaterialEffect == 1)
			{
				GameMaterial material = contact.Material2;
				ParticleEffectInfo effectInfo = material.GetParticleEffectInfo();
				
				if (effectInfo)
				{
					prefabData.m_Particle = effectInfo.GetVehicleDustResource(prefabData.m_iEffectIndex);	
				}
			} 
			else if (prefabData.m_iGameMaterialEffect == 2)
			{
				GameMaterial material = contact.Material2;
				ParticleEffectInfo effectInfo = material.GetParticleEffectInfo();
				
				if (effectInfo)
				{
					prefabData.m_Particle = effectInfo.GetBlastResource(prefabData.m_iEffectIndex);	
				}
			}
			
			if (prefabData.m_Particle != string.Empty)
			{
				ParticleEffectEntitySpawnParams spawnParams = new ParticleEffectEntitySpawnParams();
				spawnParams.UseFrameEvent = true;
				
				if (prefabData.m_bParticleOriented)	
					Math3D.AnglesToMatrix(contact.Normal, spawnParams.Transform);

				spawnParams.Transform[3] = contact.Position;
				ParticleEffectEntity.SpawnParticleEffect(prefabData.m_Particle, spawnParams);
				
#ifdef ENABLE_DIAG
					if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_PARTICLES_CONTACT_COMPONENT))
					{
						DebugTextWorldSpace.Create(GetOwner().GetWorld(), contact.Material2.GetName(), DebugTextFlags.CENTER, contact.Position[0], contact.Position[1], contact.Position[2]);
					}
#endif
			}
		}
				
		// Disable OnContact after the first contact
		if (prefabData.m_bFirstContactOnly)
			ClearEventMask(owner, EntityEvent.CONTACT);
	}
}
