/*
Returns true when velocity vector is aimed towards me (angle is below 90 degrees).
*/
class SCR_AIDecoTestIsMovingTowardsMe : DecoratorTestScripted
{
	protected override bool TestFunction(AIAgent agent, IEntity controlled)
	{
		if (!controlled)
			return false;
		
		Physics phy = controlled.GetPhysics();
		
		if (!phy)
			return false;
		
		// Bail if velocity is around zero
		vector velocityWorld = phy.GetVelocity();
		float velocityAbsSq = velocityWorld.LengthSq();
		if (velocityAbsSq <= 0.001)
			return false;
		
		vector velocityDir = velocityWorld.Normalized();
		
		IEntity myEntity = agent.GetControlledEntity();
		if (!myEntity)
			return false;
		
		vector vecToMe = vector.Direction(controlled.GetOrigin(), myEntity.GetOrigin());
		vecToMe.Normalize();
		
		float cosAngle = vector.Dot(vecToMe, velocityDir);
		
		return cosAngle > 0;
	}
};