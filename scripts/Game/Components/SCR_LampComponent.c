[EntityEditorProps(category: "GameScripted", color: "0 0 255 255")]
class SCR_LampComponentClass : SCR_BaseInteractiveLightComponentClass
{
	[Attribute("{40318DDE45FF4CC3}Particles/Enviroment/Lamp_fire_normal.ptc", UIWidgets.ResourceNamePicker, "Prefab of fire particle used for a fire action.", "ptc")]
	protected ResourceName m_sParticle;	

	[Attribute("0 0.1 0", UIWidgets.EditBox, "Particle offset in local space from the origin of the entity")]
	protected vector m_vParticleOffset;
	
	//------------------------------------------------------------------------------------------------
	//! \return
	ResourceName GetParticle()
	{
		return m_sParticle;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	vector GetParticleOffset()
	{
		return m_vParticleOffset;
	}
}

class SCR_LampComponent : SCR_BaseInteractiveLightComponent
{
	private ParticleEffectEntity m_pFireParticle;
	protected SCR_BaseInteractiveLightComponentClass m_ComponentData;
	
	//------------------------------------------------------------------------------------------------
	override void ToggleLight(bool turnOn, bool skipTransition = false, bool playSound = true)
	{	
		if (m_bIsOn == turnOn)
			return;
		
		super.ToggleLight(turnOn, skipTransition, playSound);
		
		// don't continue if DS server is running in console (particles, decals)
		if (System.IsConsoleApp())
			return;
		
		if (turnOn)
		{			
			TurnOn();
		}	
		else
		{
			TurnOff();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void TurnOn()
	{
		SCR_LampComponentClass componentData = SCR_LampComponentClass.Cast(GetComponentData(GetOwner()));
		if (!m_pFireParticle && componentData) 
		{
			ParticleEffectEntitySpawnParams spawnParams = new ParticleEffectEntitySpawnParams();
			spawnParams.TargetWorld = GetOwner().GetWorld();
			spawnParams.Parent = GetOwner();
			Math3D.MatrixIdentity4(spawnParams.Transform);
			spawnParams.Transform[3] = componentData.GetParticleOffset();
			
			m_pFireParticle = ParticleEffectEntity.SpawnParticleEffect(componentData.GetParticle(), spawnParams);
		}
		else if (m_pFireParticle)
		{
			m_pFireParticle.Play();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void TurnOff()
	{
		//Reset fire particles
		if (m_pFireParticle)
		{
			m_pFireParticle.Stop();
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		// Can't relay on RplLoad method of base class as with the lamp we have to execute ToggleLight method in both on / off state.
		bool isOn = false;
		reader.ReadBool(isOn);
		ToggleLight(isOn, true);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		super.OnDelete(owner);
		
		RemoveLights();
	}
}
