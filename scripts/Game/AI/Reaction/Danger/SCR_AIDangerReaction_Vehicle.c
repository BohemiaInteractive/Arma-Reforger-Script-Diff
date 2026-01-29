[BaseContainerProps()]
class SCR_AIDangerReaction_Vehicle : SCR_AIDangerReaction
{
	#ifdef AI_DEBUG
	protected ref array<ref Shape> m_aShapes = {};
	#endif
	
	//----------------------------------------------------------------------------------------------------------------------------------------------------------
	override bool PerformReaction(notnull SCR_AIUtilityComponent utility, notnull SCR_AIThreatSystem threatSystem, AIDangerEvent dangerEvent, int dangerEventCount)
	{
		SCR_ChimeraAIAgent agent = SCR_ChimeraAIAgent.Cast(utility.GetOwner());
				
		IEntity vehicleEntity = dangerEvent.GetVictim();
		if (!vehicleEntity)
			return false;
		
		/*
			Disabled until vehicle front light tweaks
		
		// Call driver behavior about incoming vehicle if active
		SCR_AIIdleBehavior_Driver driverBehavior = SCR_AIIdleBehavior_Driver.Cast(utility.FindActionOfType(SCR_AIIdleBehavior_Driver));
		if (driverBehavior)
			driverBehavior.OnVehicleApproaching();
		*/
		
		// Ignore if we already have an action to move from that vehicle
		if (SCR_AIMoveFromDangerBehavior.ExistsBehaviorForEntity(utility, vehicleEntity))
			return false;
		
		SCR_BaseCompartmentManagerComponent compManager = SCR_BaseCompartmentManagerComponent.Cast(vehicleEntity.FindComponent(SCR_BaseCompartmentManagerComponent));
		if (!compManager)
			return false;
		
		array<IEntity> occupants = {};
		compManager.GetOccupantsOfType(occupants, ECompartmentType.PILOT);
		
		// Ignore if no driver
		if (occupants.Count() == 0)
			return false;
		
		// Different functionality if we are in vehicle
		if (utility.m_AIInfo.HasUnitState(EUnitState.PILOT))
		{
			
			IEntity myVehicle = utility.m_OwnerEntity.GetRootParent();
			
			bool incomingCollision = VehicleIntersectsMyPath(myVehicle, vehicleEntity);
			
			if (incomingCollision)
			{
				if (!SCR_AIMoveFromDangerBehavior.ExistsBehaviorForEntity(utility, vehicleEntity))
				{
					SCR_AIPilotMoveFromIncomingVehicleBehavior behavior = new SCR_AIPilotMoveFromIncomingVehicleBehavior(
						utility, null, vehicleEntity.GetOrigin(), vehicleEntity);
					utility.AddAction(behavior);
				}
			}
			
			return true;
		}
		else if (!utility.m_AIInfo.HasUnitState(EUnitState.IN_VEHICLE))
		{
			// On foot
			// Ignore if the behavior would fail immediately after start
			if (!SCR_AIMoveFromIncomingVehicleBehavior.ExecuteBehaviorCondition(utility, vehicleEntity))
				return false;
	
			SCR_AIMoveFromDangerBehavior behavior = new SCR_AIMoveFromIncomingVehicleBehavior(utility, null, vector.Zero, dangerEntity: vehicleEntity);
			utility.AddAction(behavior);
			
			return true;
		}
		
		return false;
	}
	
	//----------------------------------------------------------------------------------------------------------------------------------------------------------
	protected bool VehicleIntersectsMyPath(notnull IEntity myVehicle, notnull IEntity otherVehicle)
	{
		Physics myPhysics = myVehicle.GetPhysics();
		Physics otherPhysics = otherVehicle.GetPhysics();
		
		// Without physics, how can we know?
		if (!myPhysics || !otherPhysics)
			return false;
		
		vector myVelWorld = myPhysics.GetVelocity();
		vector otherVelWorld = otherPhysics.GetVelocity();
		
		// otherVehicle velocity relative to myVehicle
		vector relVelWorld = otherVelWorld - myVelWorld;
		
		vector myBbMinLocal;
		vector myBbMaxLocal;
		myVehicle.GetBounds(myBbMinLocal, myBbMaxLocal);
		vector myBbCenterLocal = 0.5 * (myBbMinLocal + myBbMaxLocal);
		vector myBbCenterWorld = myVehicle.CoordToParent(myBbCenterLocal);
		
		vector otherBbMinOtherLocal; // In other vehicle local space
		vector otherBbMaxOtherLocal;
		otherVehicle.GetBounds(otherBbMinOtherLocal, otherBbMaxOtherLocal);
		vector otherBbCenterOtherLocal = 0.5 * (otherBbMinOtherLocal + otherBbMaxOtherLocal);
		vector otherBbCenterWorld = otherVehicle.CoordToParent(otherBbCenterOtherLocal);
		
		// Estimate vehicle 'radiuses' based on widths of BB
		float myR = 0.5*(myBbMaxLocal[0] - myBbMinLocal[0]);
		float otherR = 0.5*(otherBbMaxOtherLocal[1] - otherBbMinOtherLocal[1]);
		
		float intersectionTime;
		bool intersectionTestResult = MovingPointSphereIntersectionTest(otherBbCenterWorld - myBbCenterWorld, relVelWorld, myR + otherR, intersectionTime);
		
		const float maxTimeToCollision_s = 10.0;
		bool testResult = intersectionTestResult && (intersectionTime < maxTimeToCollision_s);
		
		/*
		#ifdef AI_DEBUG
		m_aShapes.Clear();
		
		int mySphereColor = Color.GREEN;
		if (testResult)
			mySphereColor = Color.RED;
		Shape shapeMySphere = Shape.CreateSphere(mySphereColor, ShapeFlags.VISIBLE | ShapeFlags.WIREFRAME, myBbCenterWorld, otherR);
		m_aShapes.Insert(shapeMySphere);
		
		Shape shapeMySphereCombined = Shape.CreateSphere(mySphereColor, ShapeFlags.VISIBLE | ShapeFlags.WIREFRAME, myBbCenterWorld, myR + otherR);
		m_aShapes.Insert(shapeMySphereCombined);
		
		vector velLineEnd = otherBbCenterWorld + maxTimeToCollision_s*otherVelWorld;
		Shape shapeOtherVel = Shape.Create(ShapeType.LINE, Color.RED, ShapeFlags.DEFAULT, otherBbCenterWorld, velLineEnd);
		m_aShapes.Insert(shapeOtherVel);
		
		vector velLineEndSize = Vector(0.15, 0.15, 0.15);
		Shape shapeOtherVelEnd = Shape.Create(ShapeType.BBOX, Color.RED, ShapeFlags.DEFAULT, velLineEnd - velLineEndSize, velLineEnd + velLineEndSize);
		m_aShapes.Insert(shapeOtherVelEnd);
		
		if (testResult)
		{
			vector impactPosWorld = otherBbCenterWorld + intersectionTime * otherVelWorld;
			vector impactPosSize = Vector (0.5, 0.5, 0.5);
			Shape shapeImpactPos = Shape.Create(ShapeType.BBOX, Color.RED, ShapeFlags.DEFAULT, impactPosWorld - impactPosSize, impactPosWorld + impactPosSize);
			m_aShapes.Insert(shapeImpactPos);
		}
		#endif
		*/
		
		return testResult;
	}
	
	//----------------------------------------------------------------------------------------------------------------------------------------------------------
	/*!
	Tests intersection of a linearly moving point and a sphere located at 0,0,0.
	In general there are up to two intersection moments in this problem.
	Thus the function returns true if there will be an intersection, but only if intersection time is positive (in future).
	If two intersections are detected in future, it returns the nearest one.
	p - starting position of point
	v - velocity of point
	r - radius of sphere
	outT - output, time of collision.
	*/
	protected static bool MovingPointSphereIntersectionTest(vector p, vector v, float r, out float outT)
	{	
		// This is a quadratic equation, it has up to two roots. First check if it has roots at all.
		
		float px = p[0];
		float py = p[1];
		float pz = p[2];
		float vx = v[0];
		float vy = v[1];
		float vz = v[2];
		
		float vx2 = vx*vx;
		float vy2 = vy*vy;
		float vz2 = vz*vz;
		float vLen2 = vx2 + vy2 + vz2;
		
		if (vLen2 == 0)
		{
			outT = 0;
			return false;
		}
		
		// Calculate D
		float _a = px*vy - py*vx;
		float _b = px*vz - pz*vx;
		float _c = py*vz - pz*vy;
		
		float D = r*r*vLen2 - (_a*_a) - (_b*_b) - (_c*_c);
		
		// No solutions in real numbers
		if (D < 0)
		{
			outT = 0;
			return false;
		}
			
		float tBase = -(px*vx + py*vy + pz*vz)/vLen2;
		
		if (D == 0)
		{
			// One intersection
			if (tBase >= 0)
			{
				outT = tBase;
				return true;
			}
			else
			{
				outT = 0;
				return false;
			}
		}
		
		float sqrtD = Math.Sqrt(D);
		float t0 = tBase + sqrtD/vLen2;
		float t1 = tBase - sqrtD/vLen2;
		
		// Two solutions, we need only positive one, but smallest if there are two positives
		if (t0 < 0 && t1 < 0)
		{
			// Both intersections are in the past
			outT = 0;
			return false;
		}
		else if (t0 >= 0 && t1 >= 0)
		{
			// Both are positive (in future), return smallest one
			outT = Math.Min(t0, t1);
			return true;
		}
		else
		{
			// One is positive and another isn't, return the positive one
			if (t0 > t1)
				outT = t0;
			else
				outT = t1;
			return true;
		}
	}
};
