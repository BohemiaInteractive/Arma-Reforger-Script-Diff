class SCR_SpecialCollisionDamageEffect : SCR_PersistentDamageEffect
{
	protected SCR_CharacterControllerComponent m_Controller;
	protected IEntity m_ResponsibleEntity;
	
	[Attribute(defvalue: "1", uiwidget: UIWidgets.Slider, desc: "Max speed of the character in this collider", params: "0 1 0.001")]
	protected float m_fSpecialCollisionMaxSpeedLimitScaled;	
	
	[Attribute(defvalue: "0", desc: "Allow jumping or climbing when in this collider")]
	protected bool m_bSpecialCollisionAllowJumpingClimbing;

	[Attribute(desc: "Sound event that will be played when damage is dealt")]
	protected string m_sOnDamageSoundEvent;
	
	protected int m_iDamageSoundEvent = 0;

	//------------------------------------------------------------------------------------------------
	string GetDamageSoundEvent()
	{
		return m_sOnDamageSoundEvent;
	}

	//------------------------------------------------------------------------------------------------
	protected override void HandleConsequences(SCR_ExtendedDamageManagerComponent dmgManager, DamageEffectEvaluator evaluator)
	{
		super.HandleConsequences(dmgManager, evaluator);

		evaluator.HandleEffectConsequences(this, dmgManager);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnEffectAdded(SCR_ExtendedDamageManagerComponent dmgManager)
	{
		super.OnEffectAdded(dmgManager);
		
		if (!m_ResponsibleEntity)
		{
			Terminate();
			return;
		}
		
		ApplyMobilityLimits(dmgManager);
	}

	//------------------------------------------------------------------------------------------------
	override void OnEffectRemoved(SCR_ExtendedDamageManagerComponent dmgManager)
	{
		super.OnEffectRemoved(dmgManager);

		ApplyMobilityLimits(dmgManager);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetResponsibleEntity(notnull IEntity entity)
	{
		m_ResponsibleEntity = entity;
	}	
	
	//------------------------------------------------------------------------------------------------
	IEntity GetResponsibleEntity()
	{
		return m_ResponsibleEntity;
	}
	
	//------------------------------------------------------------------------------------------------
	void ApplyMobilityLimits(SCR_ExtendedDamageManagerComponent dmgManager)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(dmgManager.GetOwner());
		if (!character)
		{
			Terminate();
			return;
		}
		
		m_Controller = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
		if (!m_Controller)
		{
			Terminate();
			return;
		}
		
		float lowestLimit = 1;
		float tempLimit;
		SCR_SpecialCollisionDamageEffect specialEffect;
		
		array<ref SCR_PersistentDamageEffect> effects = {};
		effects = dmgManager.GetAllPersistentEffectsOfType(SCR_SpecialCollisionDamageEffect, true);
		foreach (SCR_PersistentDamageEffect effect : effects)
		{
			specialEffect = SCR_SpecialCollisionDamageEffect.Cast(effect);
			if (!specialEffect)
				continue;
			
			tempLimit = specialEffect.GetMaxSpeedLimitScaled();
			if (tempLimit < lowestLimit)
				lowestLimit = tempLimit;
		}
		
		m_Controller.OverrideMaxSpeed(lowestLimit);
	}
	
	//------------------------------------------------------------------------------------------------
	float GetMaxSpeedLimitScaled()
	{
		return m_fSpecialCollisionMaxSpeedLimitScaled;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetJumpingAndClimbingAllowed()
	{
		return m_bSpecialCollisionAllowJumpingClimbing;
	}

	//------------------------------------------------------------------------------------------------

	protected override void OnEffectApplied(SCR_ExtendedDamageManagerComponent dmgManager)
	{
		SCR_CharacterDamageManagerComponent characterDamageMgr = SCR_CharacterDamageManagerComponent.Cast(dmgManager);
		if (!characterDamageMgr)
			return;
		
		characterDamageMgr.PlaySoundEvent(m_sOnDamageSoundEvent);
	}

	//------------------------------------------------------------------------------------------------
	//! Coppies the necessary informations from the entity which caused this damage effect
	//! \param[in] srcEnt
	protected void CopyDataFromSourceEntity(notnull IEntity srcEnt)
	{
		SCR_SpecialCollisionHandlerComponent specialCollisionComp = SCR_SpecialCollisionHandlerComponent.Cast(srcEnt.FindComponent(SCR_SpecialCollisionHandlerComponent));
		if (!specialCollisionComp)
			return;

		SCR_SpecialCollisionHandlerComponentClass data = SCR_SpecialCollisionHandlerComponentClass.Cast(specialCollisionComp.GetComponentData(srcEnt));
		if (!data)
			return;

		array<SCR_SpecialCollisionDamageEffect> damageEffects = {};
		data.GetSpecialCollisionDamageEffects(damageEffects);
		typename thisType = Type();
		foreach (SCR_SpecialCollisionDamageEffect effect : damageEffects)
		{
			if (effect.Type() != thisType)
				continue;

			m_sOnDamageSoundEvent = effect.GetDamageSoundEvent();
			break;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override bool Save(ScriptBitWriter w)
	{
		super.Save(w);

		w.WriteBool(m_bSpecialCollisionAllowJumpingClimbing);
		w.WriteFloat01(m_fSpecialCollisionMaxSpeedLimitScaled);

		// If the responsible entity is not replicated, we don't transfer info about it.
		// This is not na error. The world can be full of entities that are not replicated.
		// It is also possible that the entity gets deleted in the meantime.
		RplComponent reponsibleEntRplComp = null;
		if (m_ResponsibleEntity)
			reponsibleEntRplComp = SCR_EntityHelper.GetEntityRplComponent(m_ResponsibleEntity);
		w.WriteBool(reponsibleEntRplComp != null);
		if (reponsibleEntRplComp)
			w.WriteRplId(reponsibleEntRplComp.Id());
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool Load(ScriptBitReader r)
	{
		super.Load(r);

		r.ReadBool(m_bSpecialCollisionAllowJumpingClimbing);
		r.ReadFloat01(m_fSpecialCollisionMaxSpeedLimitScaled);

		bool hasRplReponsibleEnt = false;
		r.ReadBool(hasRplReponsibleEnt);
		if (hasRplReponsibleEnt)
		{
			RplId reponsibleEntRplId;
			r.ReadRplId(reponsibleEntRplId);
			RplComponent reponsibleEntRplComp = RplComponent.Cast(Replication.FindItem(reponsibleEntRplId));
			if (reponsibleEntRplComp)
			{
				m_ResponsibleEntity = reponsibleEntRplComp.GetEntity();
				CopyDataFromSourceEntity(m_ResponsibleEntity);
			}
			else
			{
				m_ResponsibleEntity = null;
			}
		}

		return true;
	}
}
