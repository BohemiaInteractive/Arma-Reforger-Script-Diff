// This screen effect is used in the SCR_HudManagerComponent of {6E2BB64764E3BE9B}Prefabs/Characters/Core/DefaultPlayerController.et
// And it operates on what is in {BFBCF7C2CF2E36EE}UI/layouts/HUD/ScreenEffects/ScreenEffects.layout
class SCR_PoisonScreenEffect : SCR_BaseScreenEffect
{
	[Attribute(defvalue: "0.17", desc: "Min alpha mask progress value", params: "0 1 0.01")]
	protected float m_fMinEnabledAlpha;

	[Attribute(defvalue: "0.45", desc: "Max alpha mask progress value", params: "0 1 0.01")]
	protected float m_fMaxEnabledAlpha;

	[Attribute(defvalue: "0.4", desc: "Minimal opacity value that will be used by the poison widgets", params: "0 1 0.01")]
	protected float m_fMinEnabledEffectOpacity;

	[Attribute(defvalue: "1", desc: "Base animation speed for the opacity", params: "0.01 inf 0.01")]
	protected float m_fAnimationSpeed;

	[Attribute(defvalue: "1.1", desc: "Max additional speed that can be applied to the opacity animation", params: "0.01 inf 0.01")]
	protected float m_fMaxRandomAnimationSpeedDifference;

	[Attribute(defvalue: "0.1", desc: "Multiplier applied to the currently total amount of poison damge, to convert it into the opacity value", params: "0.01 inf 0.01")]
	protected float m_fDamageToOpacityFactor;

	protected ImageWidget m_wPoisonEffect1;
	protected ImageWidget m_wPoisonEffect2;

	protected SCR_DamageSufferingSystem m_DamageSystem;
	protected ref set<DotDamageEffect> m_DamageEffects;

	//------------------------------------------------------------------------------------------------
	override void DisplayStartDraw(IEntity owner)
	{
		m_wPoisonEffect1 = ImageWidget.Cast(m_wRoot.FindAnyWidget("PoisonVignette1"));
		m_wPoisonEffect2 = ImageWidget.Cast(m_wRoot.FindAnyWidget("PoisonVignette2"));
		m_DamageSystem = SCR_DamageSufferingSystem.GetInstance();
		SetEnabled(false);
	}

	//------------------------------------------------------------------------------------------------
	override void ClearEffects()
	{
		if (m_DamageEffects)
			m_DamageEffects.Clear();

		if (!m_wPoisonEffect1 || !m_wPoisonEffect2)
			return;

		AnimateWidget.StopAllAnimations(m_wPoisonEffect1);
		AnimateWidget.StopAllAnimations(m_wPoisonEffect2);
		if (m_wPoisonEffect1.GetMaskProgress() > 0)
		{
			AnimateWidget.Opacity(m_wPoisonEffect1, 0, 1);
			AnimateWidget.Opacity(m_wPoisonEffect2, 0, 1);
			AnimateWidget.AlphaMask(m_wPoisonEffect1, 0, 1);
			AnimateWidget.AlphaMask(m_wPoisonEffect2, 0, 1).GetOnCompleted().Insert(HideEffects);
			return;
		}

		HideEffects();
	}

	//------------------------------------------------------------------------------------------------
	//!
	protected void HideEffects()
	{
		if (!m_wPoisonEffect1 || !m_wPoisonEffect2)
			return;

		m_wPoisonEffect1.SetMaskProgress(0);
		m_wPoisonEffect2.SetMaskProgress(0);
		m_wPoisonEffect2.GetParent().SetVisible(false);
	}

	//------------------------------------------------------------------------------------------------
	//!
	protected void StartEffects()
	{
		if (!m_wPoisonEffect1 || !m_wPoisonEffect2)
			return;

		UpdateAnimation(m_wPoisonEffect1);
		UpdateAnimation(m_wPoisonEffect2);

		m_wPoisonEffect1.GetParent().SetVisible(true);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] widget
	protected void UpdateAnimation(notnull ImageWidget widget)
	{
		float speedFactor = 1;
		float targetOpacity = 1;
		if (widget.GetOpacity() > m_fMinEnabledEffectOpacity)
		{
			targetOpacity = m_fMinEnabledEffectOpacity;
			speedFactor = 0.5;
		}

		AnimateWidget.Opacity(widget, targetOpacity, (m_fAnimationSpeed + SCR_Math.RandomFloat(0, m_fMaxRandomAnimationSpeedDifference)) * speedFactor);
	}

	//------------------------------------------------------------------------------------------------
	override void SetEnabled(bool isEnabled)
	{
		if (isEnabled)
			StartEffects();
		else
			ClearEffects();

		super.SetEnabled(isEnabled);
	}

	//------------------------------------------------------------------------------------------------
	override void DisplayControlledEntityChanged(IEntity from, IEntity to)
	{
		SetEnabled(false);
		HideEffects();

		ChimeraCharacter character = ChimeraCharacter.Cast(to);
		if (!character)
			return;

		SCR_CharacterDamageManagerComponent dmgManager = SCR_CharacterDamageManagerComponent.Cast(character.GetDamageManager());
		if (!dmgManager)
			return;

		const HitZone defaultHitZone = dmgManager.GetDefaultHitZone();

		array<ref SCR_PersistentDamageEffect> effects = {};
		if (dmgManager.FindAllDamageEffectsOfTypeOnHitZone(SCR_PoisonDamageEffect, defaultHitZone, effects) < 1)
			return;

		// find all damage effects that are important for this screen effect and add it
		SCR_PoisonDamageEffect poisonEffect;
		foreach (SCR_PersistentDamageEffect effect : effects)
		{
			poisonEffect = SCR_PoisonDamageEffect.Cast(effect);
			if (!poisonEffect)
				continue;

			// add them but dont start the effect as its will be done afterwards just once
			OnDamageEffectAdded(poisonEffect, false);
		}

		if (!m_DamageEffects || m_DamageEffects.IsEmpty())
			return;

		// now that we are sure that there is a reason to tick this effect start it
		SetEnabled(true);
	}

	//------------------------------------------------------------------------------------------------
	//! Method used to inform the screen effect that this damage effect would like to enable it
	//! \param[in] dmgEffect
	//! \param[in] updateVisuals
	void OnDamageEffectAdded(notnull DotDamageEffect dmgEffect, bool updateVisuals = true)
	{
		if (!m_DamageEffects)
			m_DamageEffects = new set<DotDamageEffect>();

		if (!m_DamageEffects.Insert(dmgEffect))
			return;

		if (!updateVisuals)
			return;

		SetEnabled(true);
	}

	//------------------------------------------------------------------------------------------------
	//! Method used to inform the screen effect that this effect should not matter for it
	//! \param[in] dmgEffect
	void OnDamageEffectRemoved(notnull DotDamageEffect dmgEffect)
	{
		if (m_DamageEffects.RemoveItem(dmgEffect))
		{
			if (m_DamageEffects.IsEmpty())
				SetEnabled(false);
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void DisplayUpdate(IEntity owner, float timeSlice)
	{
		if (!m_DamageEffects || m_DamageEffects.IsEmpty())
		{
			SetEnabled(false);
			return;
		}

		float newMaskProgress;
		//const Widget parent = m_wPoisonEffect1.GetMaskProgress();
		newMaskProgress = Math.Max(m_wPoisonEffect1.GetMaskProgress(), m_fMinEnabledAlpha);
		EvaluateDamageEffects(newMaskProgress);
		newMaskProgress = Math.Min(Math.Lerp(m_wPoisonEffect1.GetMaskProgress(), newMaskProgress, timeSlice), m_fMaxEnabledAlpha);

		m_wPoisonEffect1.SetMaskProgress(newMaskProgress);
		m_wPoisonEffect2.SetMaskProgress(newMaskProgress);

		if (!AnimateWidget.IsAnimating(m_wPoisonEffect1))
			UpdateAnimation(m_wPoisonEffect1);

		if (!AnimateWidget.IsAnimating(m_wPoisonEffect2))
			UpdateAnimation(m_wPoisonEffect2);
	}

	//------------------------------------------------------------------------------------------------
	//! Method used to determine new opacity value
	//! \param[in,out] newMaskProgress
	protected void EvaluateDamageEffects(inout float newMaskProgress)
	{
		SCR_BatchedPoisonDamageEffects batchedData = SCR_BatchedPoisonDamageEffects.Cast(m_DamageSystem.GetBatchedDataForLocalCharacter(SCR_PoisonDamageEffect));
		if (batchedData)
			newMaskProgress = batchedData.m_fDamageValue * m_fDamageToOpacityFactor;
	}
}
