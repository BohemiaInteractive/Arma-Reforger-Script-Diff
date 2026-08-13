[ComponentEditorProps(category: "GameScripted/Misc", description: "")]
class SCR_CigaretAnimationComponentClass : ScriptComponentClass
{
	[Attribute(defvalue: "-5000", desc: "Initial delay befor the emissive multiplier starts growing above 0")]
	protected int m_iInitialDelayValue;

	[Attribute(defvalue: "0 0 -0.036", desc: "Cigar Smoke Emitter Offset L S", params: "inf inf purpose=coords space=entity anglesVar=Angles")]
	protected vector m_vCigarSmokeEmitterOffsetLS;

	[Attribute(defvalue: "{42E2CDB811A6065F}Particles/Enviroment/Cigarette_smoke.ptc", desc: "Smoke particles emmited from the cigar", uiwidget: UIWidgets.ResourcePickerThumbnail, params: "ptc")]
	protected ResourceName m_sCigarSmokeParticles;

	[Attribute(defvalue: "{1E4DCC444ED9742B}Particles/Enviroment/CigaretteMouth_smoke.ptc", desc: "Smoke particles emmited from the mouth of the character", uiwidget: UIWidgets.ResourcePickerThumbnail, params: "ptc")]
	protected ResourceName m_sMouthSmokeParticles;

	//------------------------------------------------------------------------------------------------
	int GetInitialDelayValue()
	{
		return m_iInitialDelayValue;
	}

	//------------------------------------------------------------------------------------------------
	vector GetCigarSmokeEmitterOffsetLS()
	{
		return m_vCigarSmokeEmitterOffsetLS;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetCigarSmokeParticles()
	{
		return m_sCigarSmokeParticles;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetMouthSmokeParticles()
	{
		return m_sMouthSmokeParticles;
	}
}

class SCR_CigaretAnimationComponent : ScriptComponent
{
	protected ParticleEffectEntity m_CigarSmokeEmitter;
	protected ParticleEffectEntity m_MouthSmokeEmitter;
	protected ParametricMaterialInstanceComponent m_ParametricMaterialComp;
	protected float m_fMinDistanceSqFromCharacterFace = float.MAX;
	protected float m_fEmissiveMultiplier;

	const protected string MOUTH_EMITTER_PARENT_BONE = "Orbicularis_bottomCenterLips";

	//------------------------------------------------------------------------------------------------
	override void EOnPostFixedFrame(IEntity owner, float timeSlice)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(owner.GetParent());
		if (!character)
			return;

		float dstSq = vector.DistanceSq(owner.GetOrigin(), character.EyePosition());
		if (dstSq < m_fMinDistanceSqFromCharacterFace)
		{ // find the closest distance to the head
			m_fMinDistanceSqFromCharacterFace = dstSq;
			return;
		}
		if (float.AlmostEqual(m_fMinDistanceSqFromCharacterFace, dstSq, 0.01))
		{ // if it is at the closest distance, then we can pretend that character is sucking the contaminated air
			m_fEmissiveMultiplier = Math.Min(m_fEmissiveMultiplier + timeSlice * 1000, 1000);
			m_CigarSmokeEmitter.StopEmission();
		}
		else if (m_fEmissiveMultiplier > 0)
		{ // if its hot then cool it down
			m_fEmissiveMultiplier = Math.Max(m_fEmissiveMultiplier - timeSlice * 1000, 0);
			if (m_fEmissiveMultiplier == 0)
			{ // when it cools down resume smoke particle playback
				m_CigarSmokeEmitter.Play();
				m_MouthSmokeEmitter.Play();
			}
		}

		if (m_ParametricMaterialComp)
			m_ParametricMaterialComp.SetEmissiveMultiplier(Math.Max(m_fEmissiveMultiplier, 0));
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		m_ParametricMaterialComp = ParametricMaterialInstanceComponent.Cast(owner.FindComponent(ParametricMaterialInstanceComponent));

		SCR_CigaretAnimationComponentClass data = SCR_CigaretAnimationComponentClass.Cast(GetComponentData(owner));
		m_fEmissiveMultiplier = data.GetInitialDelayValue(); // basically a cooldown to wait for the character to finish the ignition animation
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		delete(m_CigarSmokeEmitter);
		delete(m_MouthSmokeEmitter);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnAddedToParent(IEntity child, IEntity parent)
	{
		SCR_CigaretAnimationComponentClass data = SCR_CigaretAnimationComponentClass.Cast(GetComponentData(child));
		ParticleEffectEntitySpawnParams params = new ParticleEffectEntitySpawnParams();
		params.Transform[3] = data.GetCigarSmokeEmitterOffsetLS();
		params.FollowParent = child;
		params.PlayOnSpawn = false;
		params.UseFrameEvent = true;
		params.DeleteWhenStopped = false;
		m_CigarSmokeEmitter = ParticleEffectEntity.SpawnParticleEffect(data.GetCigarSmokeParticles(), params);

		params.Transform[3] = vector.Zero;
		params.FollowParent = null;
		params.UseFrameEvent = false;
		params.Parent = parent;
		params.PivotID = parent.GetAnimation().GetBoneIndex(MOUTH_EMITTER_PARENT_BONE);
		m_MouthSmokeEmitter = ParticleEffectEntity.SpawnParticleEffect(data.GetMouthSmokeParticles(), params);

		SetEventMask(child, EntityEvent.POSTFIXEDFRAME);
	}
}
