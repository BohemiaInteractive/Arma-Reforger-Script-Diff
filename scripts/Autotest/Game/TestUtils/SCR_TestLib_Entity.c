modded class SCR_TestLib
{
	static ref array<IEntity> s_aTestEntities = {};
	
	/* leaving the code here for standalone enfusion testing internally
	//! Spawns entity for testing, entites spawned that way can be easily cleaned up.
	static IEntity SpawnEntity(ResourceName resName, vector origin, vector orientation = vector.Zero)
	{
		vector transform[4];
		Math3D.AnglesToMatrix(Vector(orientation[1], orientation[0], orientation[2]), transform);
		transform[3] = origin;

		Resource res = Resource.Load(resName);
		EntitySpawnParams params = EntitySpawnParams();
		params.Transform = transform;

		return SpawnEntity(res, params);
	}

	//! Spawns entity for testing, entites spawned that way can be easily cleaned up.
	//! Override in the game to use the correct API and insert into s_aTestEntities
	static IEntity SpawnEntity(Resource res, EntitySpawnParams params)
	{
		IEntity entity = GetGame().SpawnEntityPrefab(res, null, params);
		if (entity)
			RegisterForDeletion(entity);

		return entity;
	}*/


	//! Check if entity is approximately at the specified position
	static bool IsEntityNearPos(notnull IEntity entity, vector targetPos, float epsilon = 0.1, bool printDebug = true, bool drawDebugArrow = true)
	{
		vector entityPos = entity.GetOrigin();
		if (drawDebugArrow)
		{
			Shape.CreateArrow(entityPos, targetPos, 0.1, Color.RED, ShapeFlags.ONCE | ShapeFlags.NOZBUFFER); // Direction to end pos
			Shape.CreateSphere(Color.BLUE, ShapeFlags.ONCE | ShapeFlags.WIREFRAME | ShapeFlags.NOZBUFFER, entityPos, 0.1); // Current player pos
			Shape.CreateSphere(Color.GREEN, ShapeFlags.ONCE | ShapeFlags.WIREFRAME | ShapeFlags.NOZBUFFER, targetPos, epsilon); // End pos
		}

		float dist = vector.Distance(entityPos, targetPos);
		if (printDebug)
			Log(string.Format("IsEntityNearPos - allowedErr: %1, actual: %2, entity Pos: %3", epsilon, dist.ToString(-1, 2), entityPos));

		return dist <= epsilon;
	}
	
	//! Forces the entity to "look at" specified entity.
	static void SetEntityLookAtEntity(IEntity entityLooking, IEntity entityToLookAt, bool log = true)
	{
		if (log)
			Log(string.Format("SetEntityLookAtEntity: %1", entityToLookAt));

		SetEntityLookAtPoint(entityLooking, entityToLookAt.GetOrigin(), log: false);
	}

	//! Forces the entity to "look at" specified point.
	static void SetEntityLookAtPoint(IEntity entity, vector point, bool log = true)
	{
		if (log)
			Log(string.Format("SetEntityLookAtPoint: %1", point));

		vector origin = entity.GetOrigin();

		vector rotMatrix[4];
		vector lookDir = point - origin;
		Math3D.DirectionAndUpMatrix(lookDir.Normalized(), vector.Up.Normalized(), rotMatrix);
	
		rotMatrix[3] = entity.GetOrigin(); //pos
		entity.SetTransform(rotMatrix);

		Shape.CreateArrow(origin, point, 0.3333, Color.BLUE, ShapeFlags.ONCE);
	}

	//! Returns false if entity is too far from line.
	static bool IsEntityInBoundsInLine(vector startPos, vector endPos, IEntity entity, float distanceTolerance = 1.0)
	{
		return IsPositionInBoundsInLine(startPos, endPos, entity.GetOrigin(), distanceTolerance);
	}

	//! Returns false if position is too far from line.
	static bool IsPositionInBoundsInLine(vector startPos, vector endPos, vector currentPos, float distanceTolerance = 1.0)
	{
		// Check input values
		if (startPos == endPos)
		{
			Print("SCR_TestLib: isEntityTooFarFromLine(): Arguments 'startPos' and 'endPos' are the same!", LogLevel.ERROR);
			return false;
		}

		if (distanceTolerance <= 0)
		{
			Print("SCR_TestLib: isEntityTooFarFromLine(): Argument 'distanceTolerance' has to be bigger than zero!", LogLevel.ERROR);
			return false;
		}

		Shape.CreateSphere(Color.RED, ShapeFlags.ONCE | ShapeFlags.NOZBUFFER, startPos, 0.1);
		Shape.CreateSphere(Color.GREEN, ShapeFlags.ONCE | ShapeFlags.NOZBUFFER, endPos, 0.1);
		Shape.CreateSphere(Color.BLUE, ShapeFlags.ONCE | ShapeFlags.NOZBUFFER, currentPos, 0.1);
		Shape.CreateArrow(startPos, endPos, 1, Color.GREEN, ShapeFlags.ONCE | ShapeFlags.NOZBUFFER);

		// Core calculations + How it works:
		// A) is it too far from the line?
		// B) is it too far from the start of the line or end of the line?

		// A) GOAL = get length of side a (vectorPC)
		// 1. Project vectorAC onto vectorAB to get the length of vectorAP.
		// 2. Get vectorAB of size 1, then increase its magnitude by the length of vectorAP.
		// 3. Take this vectorAP and add it to startPos to get location of pointP.
		// 4. Calculate vector between pointP and pointC, then get length of this vectorPC.
		// 5. Is this lengthPC > toleratedDistance? > return true (aka. entity is too far from the line)
		//
		//    B
		//    |
		//    |  a?
		//    P-----C
		//    |90  /
		// c  |   / p
		//    |  /
		//    | /
		//    A/
		//
		vector vectorAB = endPos - startPos;
		vector vectorAC = currentPos - startPos;
		float lengthAB = vectorAB.Length();
		// Project vectorAC onto vectorAB to get side c (length of c = (b . c) / |c|)
		float lengthAP = vector.Dot(vectorAB, vectorAC) / lengthAB;
		vector pointP = startPos + ((vectorAB / lengthAB) * lengthAP);

		// B) GOAL = calculate the dot product between an end of the line, if it is positive the entity got beyond that end (startPos / endPos).
		//
		//             C
		//              \       dot+
		//               \
		// an End point---o---- dot0
		//                ↑\
		//                | \   dot-
		//                |  C
		//
		vector vectorBC = currentPos - endPos;
		float dotAC = vector.Dot((vectorAB.Normalized() * -1), vectorAC.Normalized());
		float dotBC = vector.Dot(vectorAB.Normalized(), vectorBC.Normalized());

		if (dotAC > 0)
		{
			if (vectorAC.Length() > distanceTolerance)
			{
				// Is beyond startPos and is too far from line.
				Shape.CreateArrow(currentPos, startPos, 1, Color.RED, ShapeFlags.ONCE);
				return false;
			}

			Shape.CreateArrow(currentPos, startPos, 1, Color.YELLOW, ShapeFlags.ONCE);
		}
		else if (dotBC > 0)
		{
			if (vectorBC.Length() > distanceTolerance)
			{
				// Is beyond endPos and is too far from line.
				Shape.CreateArrow(currentPos, endPos, 1, Color.RED, ShapeFlags.ONCE);
				return false;
			}

			Shape.CreateArrow(currentPos, endPos, 1, Color.YELLOW, ShapeFlags.ONCE);
		}
		else if ((currentPos - pointP).Length() > distanceTolerance)
		{
			// Is too far from line.
			Shape.CreateArrow(currentPos, pointP, 1, Color.RED, ShapeFlags.ONCE);
			return false;
		}
		else
		{
			Shape.CreateArrow(currentPos, pointP, 1, Color.YELLOW	, ShapeFlags.ONCE);
		}

		return true;
	}

	//! Check if entity is approximately at the specified angle (simplified without using trigonometry)
	static bool IsEntityFacingDirection(notnull IEntity entity, vector targetDirection, float epsilon = 0.1, bool printDebug = true)
	{
		if (entity.GetAngles() == vector.Zero || targetDirection == vector.Zero)
		{
			Log(string.Format("Supplied directions cannot be <0,0,0>. Entity direction: %1, target direction: %2", entity.GetAngles(), targetDirection), LogLevel.ERROR);
			return false;
		}

		vector axisDifference = entity.GetAngles() - targetDirection;
		if (printDebug)
			Log(string.Format("IsEntityNearAngle - Angle difference: %1 (Tolerance: %2), Entity angle: %3, Target angle: %4 ", axisDifference.ToString(), epsilon, entity.GetAngles().ToString(), targetDirection.ToString()));

		for (int i = 0; i < 3; i++)
		{
			if (!float.AlmostEqual(axisDifference[i], 0, epsilon))
				return false;
		}

		return true;
	}
}