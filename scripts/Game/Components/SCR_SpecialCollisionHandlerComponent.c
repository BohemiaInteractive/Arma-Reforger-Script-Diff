class SCR_SpecialCollisionHandlerComponentClass : ScriptComponentClass
{
	[Attribute()]
	protected ref array<ref SCR_SpecialCollisionDamageEffect> m_aSpecialCollisions;

	[Attribute(desc: "Value used to set the SpecialContact signal which is used by audio. If left at 0 contact with this entity will not alter the signal.", params: "0 inf")]
	protected int m_iContactType;

	[Attribute("0", desc: "Value used to set the SpecialContactEntityHeight signal.\nWhen set to 0 then height is calculated from entity bounding box", params: "0 inf")]
	protected float m_fContactHeightOverride;

	//------------------------------------------------------------------------------------------------
	// Add all damage effects on this component to given array
	void GetSpecialCollisionDamageEffects(notnull inout array<SCR_SpecialCollisionDamageEffect> damageEffects)
	{
		foreach (SCR_SpecialCollisionDamageEffect damageEffect : m_aSpecialCollisions)
		{
			damageEffects.Insert(damageEffect)
		}
	}

	//------------------------------------------------------------------------------------------------
	int GetContactType()
	{
		return m_iContactType;
	}

	//------------------------------------------------------------------------------------------------
	float GetContactHeightOverride()
	{
		return m_fContactHeightOverride;
	}

	//------------------------------------------------------------------------------------------------
	//! This sets the value in share memory space, which affects all instances of this prefab
	//! \param[in] height
	void SetContactHeight(float height)
	{
		m_fContactHeightOverride = height;
	}
}

class SCR_SpecialCollisionHandlerComponent : ScriptComponent
{
	//------------------------------------------------------------------------------------------------
	float GetContactHeight()
	{
		IEntity owner = GetOwner();
		SCR_SpecialCollisionHandlerComponentClass data = SCR_SpecialCollisionHandlerComponentClass.Cast(GetComponentData(owner));
		if (!data)
			return 0;

		if (data.GetContactHeightOverride() > 0)
			return data.GetContactHeightOverride();

		vector mins, maxs;
		owner.GetBounds(mins, maxs);
		float newHeight = maxs[1] - mins[1];
		data.SetContactHeight(newHeight);
		return newHeight;
	}

	//------------------------------------------------------------------------------------------------
	//! Method called when character starts colliding with this entity
	//! Method is executed on the client that owns the character as well as on the server
	//! \param[in] character which got in contact with this entity
	void OnContactStart(notnull SCR_ChimeraCharacter character);

	//------------------------------------------------------------------------------------------------
	//! Method called when character stops colliding with this entity
	//! Method is executed on the client that owns the character as well as on the server
	//! \param[in] character which got in contact with this entity
	void OnContactEnd(notnull SCR_ChimeraCharacter character);
}