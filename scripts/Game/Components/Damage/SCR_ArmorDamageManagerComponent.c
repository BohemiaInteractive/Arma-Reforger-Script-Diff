class SCR_ArmorDamageManagerComponentClass : SCR_DamageManagerComponentClass
{
}

class SCR_ArmorDamageManagerComponent : SCR_DamageManagerComponent
{
	[Attribute(defvalue: "0.4", uiwidget: UIWidgets.EditBox, desc: "Multiplies rawdamage to be passed to character from this armor piece")]
	protected float m_fPassedDamageScale;

	[Attribute(defvalue: "0", desc: "Can this armor element be detached when it is struck")]
	protected bool m_bIsDetachable;
	
	//------------------------------------------------------------------------------------------------
	//! Armor doesn't take collisiondamage
	override bool FilterContact(IEntity owner, IEntity other, Contact contact)
	{
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! We hijack damage on armor, and use it to damage the character. Always return true on the hijack to ensure the armor itself doesn't take damage
	override bool HijackDamageHandling(notnull BaseDamageContext damageContext)
	{
		if (damageContext.damageType != EDamageType.KINETIC && damageContext.damageType != EDamageType.MELEE)
		{
			if (m_bIsDetachable)
				KnockOffTheHelmet(damageContext);

			return true;
		}

		ChimeraCharacter char;
		IEntity parent = GetOwner();
		while (parent)
		{
			char = ChimeraCharacter.Cast(parent);
			if (char)
				break;

			parent = parent.GetParent();
		}
		
		if (!char)
			return true;
		
		SCR_CharacterDamageManagerComponent charDamMan = SCR_CharacterDamageManagerComponent.Cast(char.GetDamageManager());
		if (!charDamMan)
			return true;

		charDamMan.ArmorHitEventEffects(damageContext.damageValue);
		charDamMan.ArmorHitEventDamage(damageContext.damageType, damageContext.damageValue * m_fPassedDamageScale, damageContext.instigator.GetInstigatorEntity());

		if (m_bIsDetachable)
			KnockOffTheHelmet(damageContext);
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Method used to attempt to detach player's helmet when it is struck
	//! \param[in] damageContext
	protected void KnockOffTheHelmet(notnull BaseDamageContext damageContext)
	{
		IEntity owner = GetOwner();
		ChimeraCharacter character = ChimeraCharacter.Cast(owner.GetParent());
		if (!character)
			return;

		SCR_HeadgearInventoryItemComponent hatIIC = SCR_HeadgearInventoryItemComponent.Cast(owner.FindComponent(SCR_HeadgearInventoryItemComponent));
		if (hatIIC)
			hatIIC.DetachHelmet(damageContext.damageValue, damageContext.damageType, damageContext.hitDirection, damageContext.hitPosition, character);
	}
}
