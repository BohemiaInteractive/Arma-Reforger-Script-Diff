//! Contains various scripted 3D math functions
class SCR_Math3D
{
	//------------------------------------------------------------------------------------------------
	//! Flips X and Y axis of the vector.
	//! \param[in] toFlip
	//! \return the resulting vector
	static vector GetFixedAxisVector(vector toFlip)
	{
		return { toFlip[1], toFlip[0], toFlip[2] };
	}

	//------------------------------------------------------------------------------------------------
	//! Rotates the input from rotation to the input to rotation by a maximum of maxDegreesDelta
	//! \param[out] result
	//! \param[in] from
	//! \param[in] to
	//! \param[in] maxDegreesDelta
	static void RotateTowards(out float result[4], float from[4], float to[4], float maxDegreesDelta)
	{
		float num = Math3D.QuatAngle(from, to);
		if (float.AlmostEqual(num, 0.0))
		{
			Math3D.QuatCopy(to, result);
			return;
		}

		float t = Math.Min(1, maxDegreesDelta / num);
		Math3D.QuatLerp(result, from, to, t);
	}

	//------------------------------------------------------------------------------------------------
	//! Moves a point start in a straight line towards a target point.
	//! \param[in] start
	//! \param[in] target
	//! \param[in] maxDistanceDelta
	//! \return
	static vector MoveTowards(vector start, vector target, float maxDistanceDelta)
	{
		vector diff = target - start;
		float magnitude = diff.Length();
		if (magnitude <= maxDistanceDelta || magnitude == 0)
			return target;

		return start + diff / magnitude * maxDistanceDelta;
	}

	//------------------------------------------------------------------------------------------------
	//! Ensures the angles are in range <-units; +units>
	//! \code
	//! Print(SCR_Math3D.FixEulerVector({270, 45, 900}, 180)); // {-90, 45, 180}
	//! \endcode
	//! \param[in] angles
	//! \param[in] units
	//! \return angles in range <-units; +units>
	static vector FixEulerVector(vector angles, float units = Math.PI)
	{
		for (int i; i < 3; ++i)
		{
			angles[i] = Math.Repeat(units + angles[i], units * 2) - units;
		}

		return angles;
	}

	//------------------------------------------------------------------------------------------------
	//! Ensures the angles are in range <-180; +180>
	//! \code
	//! Print(SCR_Math3D.FixEulerVector180({270, 45, 900})); // {-90, 45, 180}
	//! \endcode
	//! \param[in] angles
	//! \return angles in range <-180; +180>
	static vector FixEulerVector180(vector angles)
	{
		for (int i; i < 3; i++)
		{
			angles[i] = Math.Repeat(180 + angles[i], 360) - 180;
		}

		return angles;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] entity is the entity you want to be affected by extrapolation.
	//! \param[in] physics is the physics that the extrapolation should calculate with.
	//! \param[in] netPosition is the last received position.
	//! \param[in] netVelocityLinear is the last received velocity.
	//! \param[in] netTeleportDistance is the max distance between position and netPosition, anything over this causes the entity to teleport.
	//! \param[in] netRotation is the last received rotation.
	//! \param[in] netVelocityAngular is the last received angular velocity.
	//! \param[in] netTeleportAng is the max angle between current rotation and replicated rotation, anything over this causes the entity to teleport.
	//! \param[in] timeSinceLastTick is the time since last synchronization of extrapolation relevant data was received, it should already be incremented by timeSlice by you!
	//! \param[in] timeSlice is the time since last frame / simulation step.
	//! \param[in] netTickInterval must NOT be zero
	static void Extrapolate(IEntity entity, Physics physics, vector netPosition, vector netVelocityLinear, float netTeleportDistance, float netRotation[4], vector netVelocityAngular, float netTeleportAng, float timeSinceLastTick, float timeSlice, float netTickInterval)
	{
		float scale = entity.GetScale();
		vector currentMatrix[4];
		entity.GetWorldTransform(currentMatrix);

		// Lerp to positions/rotations received
		vector position = currentMatrix[3];
		float rotation[4];
		Math3D.MatrixToQuat(currentMatrix, rotation);

		// Static object, ensure exact rotation/position
		if (!physics || !physics.IsDynamic())
		{
			if (rotation != netRotation)
				Math3D.QuatToMatrix(netRotation, currentMatrix);

			currentMatrix[3] = netPosition;

			entity.SetWorldTransform(currentMatrix);
			entity.SetScale(scale);
			return;
		}

		// Dynamic object, so calculate projected position/rotation based on last tick
		vector projectedPos = netPosition + netVelocityLinear * timeSinceLastTick;

		vector netVelocityAngularFlipped = GetFixedAxisVector(netVelocityAngular * timeSinceLastTick);
		float projectedRotation[4];
		float netVelocityAngularQuat[4];
		netVelocityAngularFlipped.QuatFromAngles(netVelocityAngularQuat);
		Math3D.QuatMultiply(projectedRotation, netRotation, netVelocityAngularQuat);

		// Calculate the position and rotation error
		float posError = vector.Distance(projectedPos, position);
		float rotError = Math3D.QuatAngle(projectedRotation, rotation);

		// If too far off position, teleport
		if (posError > netTeleportDistance)
		{
			entity.SetOrigin(netPosition);
			posError = 0;
		}

		// If too far off rotation, teleport
		if (rotError > netTeleportAng)
		{
			Math3D.QuatToMatrix(netRotation, currentMatrix);
			currentMatrix[3] = entity.GetOrigin();
			entity.SetWorldTransform(currentMatrix);
			rotError = 0;
		}

		float timeStep = timeSlice / netTickInterval;
		float timeStepTick = Math.Clamp(timeSlice / netTickInterval, 0, 1);

		// Adjust to account for errors in position/rotation
		if (posError > 0.01)
		{
			entity.SetOrigin(MoveTowards(position, projectedPos, posError * timeStep));
			physics.SetVelocity(physics.GetVelocity() + (projectedPos - position) * timeStepTick);
		}

		if (rotError > 0.01)
		{
			float outRot[4];
			Math3D.QuatRotateTowards(outRot, rotation, projectedRotation, (rotError * timeStep) * Math.RAD2DEG);
			Math3D.QuatToMatrix(outRot, currentMatrix);
			currentMatrix[3] = entity.GetOrigin();
			entity.SetWorldTransform(currentMatrix);

			float rotDiff[4];
			float rotInv[4];
			Math3D.QuatInverse(rotInv, rotation);
			Math3D.QuatMultiply(rotDiff, projectedRotation, rotInv);
			vector angularVelocity = Math3D.QuatToAngles(rotDiff);
			angularVelocity = FixEulerVector180(angularVelocity) * Math.DEG2RAD * timeStepTick;
			angularVelocity += physics.GetAngularVelocity();
			physics.SetAngularVelocity(angularVelocity);
		}

		entity.SetScale(scale);
	}

	//------------------------------------------------------------------------------------------------
	//! Get intersection between a line and a plane
	//! \param[in] rayPos Ray intersection position
	//! \param[in] rayVector Ray intersection direction vector
	//! \param[in] planePos Plane position
	//! \param[in] planeNormal Plane normal vector
	//! \return Intersection position
	static vector IntersectPlane(vector rayPos, vector rayVector, vector planePos, vector planeNormal)
	{
		return rayPos - rayVector * (vector.Dot(rayPos - planePos, planeNormal) / vector.Dot(rayVector, planeNormal));
	}

	//------------------------------------------------------------------------------------------------
	//! Check if two matrices are equal
	//! \param[in] matrixA
	//! \param[in] matrixB
	//! \return True when the matrices are equal
	static bool MatrixEqual(vector matrixA[4], vector matrixB[4])
	{
		return matrixA[3] == matrixB[3]
			&& matrixA[2] == matrixB[2]
			&& matrixA[1] == matrixB[1]
			&& matrixA[0] == matrixB[0];
	}

	//------------------------------------------------------------------------------------------------
	//! Check if matrix is empty
	//! \param[in] matrix
	//! \return True when all matrix vectors are zero
	static bool IsMatrixEmpty(vector matrix[4])
	{
		return matrix[3] == vector.Zero
			&& matrix[2] == vector.Zero
			&& matrix[1] == vector.Zero
			&& matrix[0] == vector.Zero;
	}

	//------------------------------------------------------------------------------------------------
	//! Check if matrix is identity matrix (right, up, forward, zero vectors)
	//! \param[in] matrix
	//! \return True when identity matrix
	static bool IsMatrixIdentity(vector matrix[4])
	{
		return matrix[3] == vector.Zero
			&& matrix[2] == vector.Forward
			&& matrix[1] == vector.Up
			&& matrix[0] == vector.Right;
	}

	//------------------------------------------------------------------------------------------------
	//! Return the smallest from each vector value
	//! \code
	//! Print(SCR_Math3D.Max({ 0, 2, 10 }, { -5, 5, 8 }, LogLevel.NORMAL); // { -5, 2, 8 }
	//! \endcode
	//! \param[in] vector A
	//! \param[in] vector B
	//! \return
	static vector Min(vector vA, vector vB)
	{
		return {
			Math.Min(vA[0], vB[0]),
			Math.Min(vA[1], vB[1]),
			Math.Min(vA[2], vB[2])
		};
	}

	//------------------------------------------------------------------------------------------------
	//! Return the largest from each vector value
	//! \code
	//! Print(SCR_Math3D.Max({ 0, 2, 10 }, { -5, 5, 8 }, LogLevel.NORMAL); // { 0, 5, 10 }
	//! \endcode
	//! \param[in] vector A
	//! \param[in] vector B
	//! \return
	static vector Max(vector vA, vector vB)
	{
		return {
			Math.Max(vA[0], vB[0]),
			Math.Max(vA[1], vB[1]),
			Math.Max(vA[2], vB[2])
		};
	}

	//------------------------------------------------------------------------------------------------
	//! Gets the shortest 3D distance between a point and a spline
	//! \param[in] points array of all points forming the spline, minimum 1 point
	//! \param[in] point point that is being checked
	//! \return distance from spline, -1 if no points are provided
	static float GetDistanceFromSpline(notnull array<vector> points, vector point)
	{
		int count = points.Count();
		if (count < 1)
			return -1;

		if (count == 1)
			return vector.Distance(point, points[0]);

		float tempDistanceSq;
		vector segmentStart = points[0];
		float minDistanceSq = vector.DistanceSq(point, segmentStart);

		foreach (int i, vector segmentEnd : points)
		{
			if (i == 0)
				continue;

			tempDistanceSq = Math3D.PointLineSegmentDistanceSqr(point, segmentStart, segmentEnd);
			if (tempDistanceSq < minDistanceSq)
				minDistanceSq = tempDistanceSq;

			segmentStart = segmentEnd;
		}

		return Math.Sqrt(minDistanceSq);
	}

	//------------------------------------------------------------------------------------------------
	//! Gets the shortest 2D distance between a point and a spline
	//! \param[in] points array of all points forming the spline, minimum 1 point
	//! \param[in] point point that is being checked
	//! \return distance from spline, -1 if no points are provided
	static float GetDistanceFromSplineXZ(notnull array<vector> points, vector point)
	{
		int count = points.Count();
		if (count < 1)
			return -1;

		if (count == 1)
			return vector.DistanceXZ(point, points[0]);

		float tempDistanceSq;
		vector segmentStart = points[0];
		float minDistanceSq = vector.DistanceSqXZ(point, segmentStart);
		segmentStart[1] = 0;	// 2D conversion
		point[1] = 0;			// 2D conversion

		foreach (int i, vector segmentEnd : points)
		{
			if (i == 0)
				continue;

			segmentEnd[1] = 0;

			tempDistanceSq = Math3D.PointLineSegmentDistanceSqr(point, segmentStart, segmentEnd);
			if (tempDistanceSq < minDistanceSq)
				minDistanceSq = tempDistanceSq;

			segmentStart = segmentEnd;
		}

		return Math.Sqrt(minDistanceSq);
	}

	//------------------------------------------------------------------------------------------------
	//! Check if point is within provided 3D distance from the shape - faster than getting distance then comparing
	//! \param[in] points the spline's points
	//! \param[in] point the position to check
	//! \param[in] distance the checked distance (should be positive)
	//! \return true if provided pos is <= distance from diff outline, false otherwise
	static bool IsPointWithinSplineDistance(notnull array<vector> points, vector point, float distance)
	{
		int count = points.Count();
		if (count < 1)
			return -1;

		if (count == 1)
			return vector.Distance(point, points[0]) <= distance;

		distance *= distance; // variable reuse
		vector segmentStart = points[0];

		foreach (int i, vector segmentEnd : points)
		{
			if (i == 0)
				continue;

			if (Math3D.PointLineSegmentDistanceSqr(point, segmentStart, segmentEnd) < distance)
				return true;

			segmentStart = segmentEnd;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Check if point is within provided 2D distance from the shape - faster than getting distance then comparing
	//! \param[in] points the spline's points
	//! \param[in] point the position to check
	//! \param[in] distance the checked distance (should be positive)
	//! \return true if provided pos is <= distance from diff outline, false otherwise
	static bool IsPointWithinSplineDistanceXZ(notnull array<vector> points, vector point, float distance)
	{
		int count = points.Count();
		if (count < 1)
			return false;

		if (count == 1)
			return vector.DistanceXZ(point, points[0]) <= distance;

		distance *= distance; // variable reuse
		vector segmentStart = points[0];
		segmentStart[1] = 0;	// 2D conversion
		point[1] = 0;			// 2D conversion

		foreach (int i, vector segmentEnd : points)
		{
			if (i == 0)
				continue;

			segmentEnd[1] = 0;	// 2D conversion

			if (Math3D.PointLineSegmentDistanceSqr(point, segmentStart, segmentEnd) < distance)
				return true;

			segmentStart = segmentEnd;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Get a rotation that rotates angle (rad) around the provided axis.
	//! \code
	//! float quat[4];
	//! SCR_Math3D.QuatAngleAxis(0.5 * Math.PI_HALF, vector.Up, quat);
	//! vector angles = Math3D.QuatToAngles( quat );
	//! Print(angles);
	//! >> vector angles = <45,0,-0>
	//! \endcode
	//! \param[in] angle Rotation angle in radians
	//! \param[in] axis The axis around which the rotation turns
	//! \param[out] quat Output quaternion
	static void QuatAngleAxis(float angle, vector axis, out float quat[4])
	{
		angle *= 0.5;
		float sin = Math.Sin(angle);

		axis.Normalize();
		quat[0] = axis[0] * sin;
		quat[1] = axis[1] * sin;
		quat[2] = axis[2] * sin;
		quat[3] = Math.Cos(angle);
	}

	//------------------------------------------------------------------------------------------------
	//! Rotates provided vector by given quaternion
	//! \param[in] quat Quaternion rotation to apply
	//! \param[in] vec Vector to rotate
	//! \return rotated vector
	static vector QuatMultiply(float quat[4], vector vec)
	{
		vector xyz = { quat[0], quat[1], quat[2] };
		vector t = 2.0 * (xyz * vec);
		return vec + quat[3] * t + (xyz * t);
	}

	//------------------------------------------------------------------------------------------------
	//! Rotates a transformation around the provided pivot point by angle (rad) around the axis
	//! \param[in] transform Input transformation to rotate
	//! \param[in] pivot The point around which the transform rotates
	//! \param[in] axis The axis in which the rotation is applied
	//! \param[in] angle The applied angle in radians
	//! \param[out] result Rotated transformation
	static void RotateAround(vector transform[4], vector pivot, vector axis, float angle, out vector result[4])
	{
		float q[4];
		QuatAngleAxis(angle, axis, q);
		result[3] = QuatMultiply(q, (transform[3] - pivot)) + pivot;

		float qt[4];
		Math3D.MatrixToQuat(transform, qt);
		Math3D.QuatMultiply(qt, q, qt);
		Math3D.QuatToMatrix(qt, result);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns a rotation matrix that makes object positioned at source position face the point at destination
	//! \param[in] source Source point the rotation looks 'from'
	//! \param[in] destination Target point the rotation looks 'at'
	//! \param[in] up Reference axis, start with 'vector.Up' if unsure
	//! \param[out] rotMat Output rotational matrix
	static void LookAt(vector source, vector destination, vector up, out vector rotMat[4])
	{
		Math3D.DirectionAndUpMatrix(vector.Direction(source, destination).Normalized(), up.Normalized(), rotMat);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns the provided vector clamped to desired magnitude.
	//! \param[in] v Input vector
	//! \param[in] magnitude The magnitude to clamp at
	//! \return clamped vector
	static vector ClampMagnitude(vector v, float magnitude)
	{
		if (v.LengthSq() > magnitude * magnitude)
			return v.Normalized() * magnitude;

		return v;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns cross product of two vector. You can optionally have them normalize before calculation.
	//! \param[in] first vector
	//! \param[in] second vector
	//! \param[in] normalizeVectors if set then both vectors will be normalized before cross product calculation.
	//! \return cross product of those two vectors
	static vector Cross(vector first, vector second, bool normalizeVectors = false)
	{
		if (normalizeVectors)
		{
			first.Normalize();
			second.Normalize();
		}

		return { 	first[1] * second[2] - first[2] * second[1],
					first[2] * second[0] - first[0] * second[2],
					first[0] * second[1] - first[1] * second[0]};
	}
}
