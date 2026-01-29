class SCR_PersistentBarbedWireDamageEffect : SCR_SpecialCollisionDamageEffect
{
	[Attribute(defvalue: "3", uiwidget: UIWidgets.EditBox, desc: "At this velocity max damage is incurred by the character \n x = m/s", category: "Barbed wire damage")]
	protected float m_fMaxDamageSpeed;
	
	[Attribute("0 1 1 0", uiwidget: UIWidgets.GraphDialog, desc: "How damage will scale with distance", params: "1 1 0 0")]
	protected ref Curve m_cSpeedDamageCurve;
	
	[Attribute(desc: "Effect that will be applied as a result of being damaged by this damage effect")]
	protected ref SCR_BarbedWireCuttingDamageEffect m_ResultingEffect;
	
	protected static const float DAMAGE_APPLY_THRESHOLD = 2;
	
	private float m_fMovedDamage;
	
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
		
		SetActive(true);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void EOnFrame(float timeSlice, SCR_ExtendedDamageManagerComponent dmgManager)
	{
		if (!m_Controller)
			return;
		
		float newMovedDamage;
		CharacterInputContext context = m_Controller.GetInputContext();
		vector characterVelocity = m_Controller.GetVelocity();
		characterVelocity[1] = context.GetJump();//for character Y of GetVelocity() is always 0

		if (characterVelocity[1] == 0 && (m_Controller.IsFalling() || m_Controller.IsClimbing()))
			characterVelocity[1] = 1;

		if (context && (context.IsMoving() || m_Controller.IsRoll() || characterVelocity[1] != 0))
			newMovedDamage = timeSlice * LegacyCurve.Curve(ECurveType.CurveProperty2D, characterVelocity.Length() / m_fMaxDamageSpeed, m_cSpeedDamageCurve)[1];
		
		m_fMovedDamage += newMovedDamage * 10;		

		if (m_fMovedDamage < DAMAGE_APPLY_THRESHOLD)
			return;
		
		ApplyEffect(dmgManager);
		m_fMovedDamage = 0;
	}
	
	//------------------------------------------------------------------------------------------------
	override EDamageType GetDefaultDamageType()
	{
		return EDamageType.MELEE;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_BarbedWireCuttingDamageEffect GetResultingEffect()
	{
		return m_ResultingEffect;
	}
}
