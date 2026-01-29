class SCR_CollisionDamageContainer
{
	IEntity m_Other;
	IEntity m_OtherPilot;
	vector m_vOwnerOriginAtThePointOfImpact;
	vector m_vOtherOriginAtThePointOfImpact;
	vector m_vImpactNormal;
	float m_fImpulse;
	float m_fTotalMomentumOwner;
	float m_fTotalMomentumOther;
	float m_fDamageShare = 1;
	int m_iOtherResponseIndex
	bool m_bOtherIsDynamic;
	bool m_bOtherIsDestructible;

	protected int m_iCollisionsCounter;
	protected vector m_vImpactPosition;
	protected vector m_vOwnerVelocityBefore;
	protected vector m_vOtherVelocityBefore;

	//------------------------------------------------------------------------------------------------
	//! \return averaged poisition of impacts in local space of the owner
	vector GetAverageImpactPosition()
	{
		return m_vImpactPosition / m_iCollisionsCounter;
	}

	//------------------------------------------------------------------------------------------------
	//! Adds a provided position to previous positions for the calculation of the average position
	//! \param[in] position
	void UpdateImpactPosition(vector position)
	{
		m_iCollisionsCounter++;
		m_vImpactPosition += position;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	vector GetOwnerVelocityBefore()
	{
		return m_vOwnerVelocityBefore;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	vector GetOtherVelocityBefore()
	{
		return m_vOtherVelocityBefore;
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] owner
	//! \param[in] other
	//! \param[in] impulse
	//! \param[in] velocityBefore1
	//! \param[in] velocityAfter1
	//! \param[in] velocityBefore2
	//! \param[in] velocityAfter2
	//! \param[in] impactPosition in local space of the owner
	//! \param[in] impactNormal
	void SCR_CollisionDamageContainer(notnull IEntity owner, notnull IEntity other, float impulse, vector velocityBefore1, vector velocityAfter1, vector velocityBefore2, vector velocityAfter2, vector impactPosition, vector impactNormal)
	{
		m_Other = other;
		m_fImpulse = impulse;
		m_vOwnerVelocityBefore = velocityBefore1;
		m_vOtherVelocityBefore = velocityBefore2;
		m_vImpactNormal = impactNormal;
		m_vImpactPosition = impactPosition;
		m_iCollisionsCounter = 1;

		m_vOwnerOriginAtThePointOfImpact = owner.GetOrigin();
		m_vOtherOriginAtThePointOfImpact = other.GetOrigin();

		Physics ownerPhysics = owner.GetPhysics();
		if (!ownerPhysics)
			return;

		float ownerMass = ownerPhysics.GetMass();
		float momentumB = velocityBefore1.Length() * ownerMass;
		float momentumA = velocityAfter1.Length() * ownerMass;
		float dotMultiplier = vector.Dot(velocityAfter1.Normalized(), velocityBefore1.Normalized());

		m_fTotalMomentumOwner = Math.AbsFloat(momentumB - momentumA * dotMultiplier);

		Vehicle vehicle = Vehicle.Cast(other);
		if (vehicle)
			m_OtherPilot = vehicle.GetPilot();
		else
			m_bOtherIsDestructible = other.FindComponent(SCR_DestructionDamageManagerComponent) != null;

		Physics otherPhysics = other.GetPhysics();
		if (!otherPhysics)
			return;

		m_iOtherResponseIndex = otherPhysics.GetResponseIndex();
		m_bOtherIsDynamic = otherPhysics.IsDynamic();

		float otherMass = otherPhysics.GetMass();
		if (otherMass > 0)
			m_fDamageShare -= ownerMass / (ownerMass + otherMass);

		momentumB = velocityBefore2.Length() * otherMass;
		momentumA = velocityAfter2.Length() * otherMass;
		dotMultiplier = vector.Dot(velocityAfter2.Normalized(), velocityBefore2.Normalized());
		m_fTotalMomentumOther = Math.AbsFloat(momentumB - momentumA * dotMultiplier);
	}
}
