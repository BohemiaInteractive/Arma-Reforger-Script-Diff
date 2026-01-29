/*!
\addtogroup DamageEffects
\{
*/

class SCR_DotDamageEffect : DotDamageEffect
{
	private bool m_bSkipRegenDelay;
	
	//------------------------------------------------------------------------------------------------
	//ALWAYS OVERRIDE LIKE THIS. ALWAYS OVERRIDE THIS FUNCTION
	protected override void HandleConsequences(SCR_ExtendedDamageManagerComponent dmgManager, DamageEffectEvaluator evaluator)
	{
		super.HandleConsequences(dmgManager, evaluator);

		evaluator.HandleEffectConsequences(this, dmgManager);
	}

	//------------------------------------------------------------------------------------------------
	protected override void EOnFrame(float timeSlice, SCR_ExtendedDamageManagerComponent dmgManager)
	{
		DealDot(timeSlice, dmgManager);
	}

	//------------------------------------------------------------------------------------------------
	protected event override void OnDiag(SCR_ExtendedDamageManagerComponent dmgManager)
	{
		super.OnDiag(dmgManager);

		string text = text.Format("  DOT: %1 \n", this.GetDPS());
		if (!IsProxy())
			text += text.Format("  Duration: %1s/%2s \n", this.GetCurrentDuration(), this.GetMaxDuration());

		DbgUI.Text(text);
	}

	//------------------------------------------------------------------------------------------------
	event override bool Save(ScriptBitWriter w)
	{
		super.Save(w);

		w.WriteFloat(GetDPS());
		return true;
	}

	//------------------------------------------------------------------------------------------------
	event override bool Load(ScriptBitReader r)
	{
		super.Load(r);

		float dps = 0;
		r.ReadFloat(dps);

		SetDPS(dps);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! When this is toggled PRIOR to the damage effect being added, the regen will start instantly OnEffectAdded
	void SkipRegenDelay(bool skip)
	{
		m_bSkipRegenDelay = skip;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Will return true if regen delay was enabled. 
	//! Getting this will reset the value so damage effect behaves normally after the regen delay was skipped
	bool GetSkipRegenDelay(SCR_ExtendedDamageManagerComponent dmgManager)
	{
		bool skip = m_bSkipRegenDelay;
		SkipRegenDelay(false);
		return skip;
	}

	//------------------------------------------------------------------------------------------------
	override float GetCustomDamageValue()
	{
		return GetDPS();
	}

	//------------------------------------------------------------------------------------------------
	//! Method called locally by SCR_DamageSufferingSystem to allow local computation of the DPS
	//! \param[in] timeSlice
	//! \param[in] dmgManager
	void RecalculateDPS(float timeSlice, notnull SCR_ExtendedDamageManagerComponent dmgManager);
}

/*!
\}
*/
