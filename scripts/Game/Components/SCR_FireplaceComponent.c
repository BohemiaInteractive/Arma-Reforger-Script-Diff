[EntityEditorProps(category: "GameScripted", color: "0 0 255 255")]
class SCR_FireplaceComponentClass : SCR_BaseInteractiveLightComponentClass
{
	[Attribute("{C0E2E7DC28B71E2C}Particles/Enviroment/Campfire_medium_normal.ptc", UIWidgets.ResourceNamePicker, "Prefab of fire particle used for a fire action.", "ptc")]
	protected ResourceName m_sParticle;	

	[Attribute("0 0.2 0", UIWidgets.EditBox, "Particle offset in local space from the origin of the entity")]
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

class SCR_FireplaceComponent : SCR_BaseInteractiveLightComponent
{
	private ParticleEffectEntity m_pFireParticle;
	protected SCR_BaseInteractiveLightComponentClass m_ComponentData;
	protected IEntity m_DecalEntity;
	protected float m_fTimer;
	protected float m_fCompDataLV;
		
	protected static const float FLICKER_STEP = 0.05;
	protected static const float FLICKER_THRESHOLD = 0.35;
	protected static const float FLICKER_FREQUENCY = 1 / 30;
		
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
		SCR_FireplaceComponentClass componentData = SCR_FireplaceComponentClass.Cast(GetComponentData(GetOwner()));
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
	
		// Show decals
		if (m_DecalEntity)
			m_DecalEntity.SetFlags(EntityFlags.VISIBLE, false);
		
		if (componentData)
			m_fCompDataLV = componentData.GetLightLV();
		
		SetEventMask(GetOwner(), EntityEvent.VISIBLE);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void TurnOff()
	{
		// Hide decals
		if (m_DecalEntity)
			m_DecalEntity.ClearFlags(EntityFlags.VISIBLE, false);
		
		//Reset fire particles
		if (m_pFireParticle)
		{
			m_pFireParticle.Stop();
		}
		
		ClearEventMask(GetOwner(), EntityEvent.VISIBLE);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		BaseSlotComponent decalComponent = BaseSlotComponent.Cast(GetOwner().FindComponent(BaseSlotComponent));
		if (decalComponent)
			m_DecalEntity = decalComponent.GetAttachedEntity();
		
		m_ComponentData = SCR_BaseInteractiveLightComponentClass.Cast(GetComponentData(GetOwner()));
		if (m_ComponentData)
			m_fCurLV = m_ComponentData.GetLightLV();
		
		if (m_DecalEntity)
			m_DecalEntity.ClearFlags(EntityFlags.VISIBLE, false);

		super.OnPostInit(owner);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnVisible(IEntity owner, int frameNumber)
	{
		super.EOnVisible(owner, frameNumber);
		
		if (GetOwner().GetWorld().IsEditMode() || !IsOn() || m_aLights.IsEmpty())
			return;
		
		float timeSlice = GetOwner().GetWorld().GetTimeSlice();
		
		m_fTimer += timeSlice;
		
		if (m_fTimer < FLICKER_FREQUENCY)
			return;

		m_fTimer = 0;
		bool canFlickerBrighter = m_fCurLV < m_fCompDataLV * (1 + FLICKER_THRESHOLD);
		bool canFlickerDimmer = m_fCurLV > m_fCompDataLV * (1 - FLICKER_THRESHOLD);
		
		if (canFlickerBrighter && canFlickerDimmer)
		{ 
			if (Math.RandomIntInclusive(0, 1) == 0)
				m_fCurLV += FLICKER_STEP;
			else
				m_fCurLV -= FLICKER_STEP;
		}
		else if (canFlickerBrighter)
			m_fCurLV += FLICKER_STEP;
		else
			m_fCurLV -= FLICKER_STEP;
		
		array<ref SCR_BaseLightData> lightData = m_ComponentData.GetLightData();
		for (int i = Math.Min(m_aLights.Count(), lightData.Count()) -1; i >= 0; i--)
		{
			auto ld = lightData[i]; // make sure that data is loaded
			if (ld)
				m_aLights[i].SetColor(Color.FromVector(ld.GetLightColor()), m_fCurLV);
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		// Can't relay on RplLoad method of base class as with the fireplace we have to execute ToggleLight method in both on / off state.
		bool isOn = false;
		reader.ReadBool(isOn);
		ToggleLight(isOn, true);
		
		return true;
	}
	
	override void OnDelete(IEntity owner)
	{
		super.OnDelete(owner);
		
		RemoveLights();
		
		if (m_pFireParticle)
			RplComponent.DeleteRplEntity(m_pFireParticle, false);
	}
}
