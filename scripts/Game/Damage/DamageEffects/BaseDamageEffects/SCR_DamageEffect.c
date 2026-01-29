/*!
\addtogroup DamageEffects
\{
*/

class SCR_DamageEffect : BaseDamageEffect
{
	/*!
	\deprecated Used to check if you are a proxy or not
	\param SCR_ExtendedDamageManagerComponent dmgManager: manager containing this DamageEffect
	\return true if this DamageEffect is a proxy
	*/
	[Obsolete("Use BaseDamageEffect.IsProxy() instead")]
	bool IsProxy(SCR_ExtendedDamageManagerComponent dmgManager)
	{
		return IsProxy();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called from ApplyEffect
	protected event override void HandleConsequences(SCR_ExtendedDamageManagerComponent dmgManager, DamageEffectEvaluator evaluator)
	{
		evaluator.HandleEffectConsequences(this, dmgManager);
	}

	//------------------------------------------------------------------------------------------------
	protected event override void OnDiag(SCR_ExtendedDamageManagerComponent dmgManager)
	{
		string text = this.Type().ToString() + "\n";

		if (!IsProxy())
		{
			if (!GetInstigator().GetInstigatorEntity())
				text += text.Format("  Instigator entity: %1 \n", "None");
			else
				text += text.Format("  Instigator entity: %1 \n", GetInstigator().GetInstigatorEntity().GetID());

			text += text.Format("  TotalDamage: %1 \n", GetTotalDamage());
		}

		text += text.Format("  DamageType: %1 \n", typename.EnumToString(EDamageType, GetDamageType()));
		if (GetAffectedHitZone())
			text += text.Format("  HitZone: %1", GetAffectedHitZone().GetName());

		DbgUI.Text(text);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool Save(ScriptBitWriter w)
	{
		super.Save(w);
		
		bool isDefaultType = GetDamageType() == GetDefaultDamageType();
		w.WriteBool(isDefaultType);

		if (!isDefaultType)
			w.WriteInt(GetDamageType());
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool Load(ScriptBitReader r)
	{
		super.Load(r);
		
		bool isDefaultType = false;
		r.ReadBool(isDefaultType);
		
		int localDamageType = GetDefaultDamageType();
		if (!isDefaultType)
			r.ReadInt(localDamageType);
	
		SetDamageType(localDamageType);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	EDamageType GetDefaultDamageType()
	{
		return EDamageType.TRUE;
	}
}

/*!
\}
*/
