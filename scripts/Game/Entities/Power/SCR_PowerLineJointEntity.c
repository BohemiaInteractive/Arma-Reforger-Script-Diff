[EntityEditorProps(category: "GameScripted/Power", description: "2019 Cable prototype through debug lines")]
class SCR_PowerLineJointEntityClass : GenericEntityClass
{
}

class SCR_PowerLineJointEntity : GenericEntity
{
	[Attribute(defvalue: "1", uiwidget: UIWidgets.Slider, params: "0 30")]
	protected float m_fSagInMeters;

	[Attribute(defvalue: "0.1", uiwidget: UIWidgets.Slider, params: "-" + Math.PI + " " + Math.PI)]
	protected float m_AngleRad;

	[Attribute(defvalue: "0.5", uiwidget: UIWidgets.Slider, params: "0 10")]
	protected float m_MovementSpeed;

#ifdef WORKBENCH

	protected static const int SEGMENTS = 64;
	protected static const int POINTS = SEGMENTS + 1;

	protected vector m_aDebugLine[POINTS];
	protected vector m_aDebugLinePivots[POINTS];
	protected vector m_vDirectionLevelNorm;
	protected vector m_vConnectedJointPos;
	protected ref array<vector> m_aCurve = {};
	protected ref array<vector> m_aCurvePivots = {};
	protected ref array<ref Shape> m_aShapes = {};

	protected float m_fTimeAccu;

	//------------------------------------------------------------------------------------------------
	bool GetJoint()
	{
		WorldEditorAPI api = _WB_GetEditorAPI();
		IEntitySource entSrc = api.EntityToSource(this);
		int childCount = entSrc.GetNumChildren();

		if (childCount > 0)
		{
			m_vConnectedJointPos = api.SourceToEntity(entSrc.GetChild(childCount - 1)).GetOrigin();
			//Print(m_vConnectedJointPos);
			return true;
		}
		else
		{
			//IEntitySource parent = entSrc.GetParent();
			//SCR_PowerLineJointEntity parentEnt = SCR_PowerLineJointEntity.Cast(api.SourceToEntity(parent));
			//parentEnt.OnChildMoved();
			return false;
		}
	}

	//------------------------------------------------------------------------------------------------
	void OnChildMoved()
	{
		//DrawDebug();
	}

	//------------------------------------------------------------------------------------------------
	void GenerateCurve()
	{
		m_aCurve.Clear();
		m_aCurvePivots.Clear();
		if (m_fSagInMeters <= 0)
			m_fSagInMeters = 0.0001;

		vector posThis = GetOrigin();
		//m_vConnectedJointPos = posThis + "50 10 0";//remove

		vector jointPos1, jointPos2;
		if (posThis[1] <= m_vConnectedJointPos[1])
		{
			jointPos1 = posThis;
			jointPos2 = m_vConnectedJointPos;
		}
		else
		{
			jointPos1 = m_vConnectedJointPos;
			jointPos2 = posThis;
		}

		m_vDirectionLevelNorm = jointPos2 - jointPos1;
		m_vDirectionLevelNorm[1] = 0;
		m_vDirectionLevelNorm.Normalize();

		float heightDifferenceM = jointPos2[1] - jointPos1[1];

		vector jointPos1Adj = jointPos1;//this pos adjusted to height of the connected joint
		jointPos1Adj[1] = jointPos2[1];
		float distanceLevel = vector.Distance(jointPos1Adj, jointPos2);

		float s1 = m_fSagInMeters / distanceLevel;				// step 1

		float c = (0.5 * 0.5) / s1;								// step 2

		float s2 = (heightDifferenceM / distanceLevel) + s1;	// step 3

		float x2 = Math.Sqrt(s2 * c);							// step 4

		float xc = 0.5 + x2;									// step 5

		float x1f = (0.5 / xc) * -1;							// step 6

		float x2f = x2 / xc;									// step 6

		float segmentSize = distanceLevel / SEGMENTS;

		float step = 1 / SEGMENTS;
		float value;

		array<float> curveYValues = {};
		for (int i; i < POINTS; i++)
		{
			//Print(i);
			float x = Math.Lerp(x1f, x2f, value) * xc;
			value += step;
			//Print(value);
			//Print(x);
			float y = (x * x) / c;
			//Print(y);
			curveYValues.Insert(y);
		}

		foreach (int i, float curveY : curveYValues)
		{
			float y = curveY * distanceLevel;
			y -= m_fSagInMeters;

			vector pos = m_vDirectionLevelNorm * (segmentSize * i);
			pos[1] = pos[1] + y;
			pos = pos + jointPos1;
			m_aCurve.Insert(pos);
			//Print(pos);
		}
	}

	//------------------------------------------------------------------------------------------------
	void GetPivots()
	{
		vector firstPoint = m_aCurve[0];
		vector lastPoint = m_aCurve[SEGMENTS];

		for (float i; i <= POINTS; i++)
		{
			vector pivot = vector.Lerp(firstPoint, lastPoint, i / SEGMENTS);
			//Print(i / POINTS);
			m_aCurvePivots.Insert(pivot);
			//Print(pos);
		}
	}

	//------------------------------------------------------------------------------------------------
	void DrawDebugShape()
	{
		for (int i = 0; i < POINTS; i++)
		{
			m_aDebugLine[i] = m_aCurve[i];
			//Print(m_aDebugLine[i]);
		}

		//m_aShapes.Insert(Shape.CreateLines(ARGB(255, 255, 255, 0), 0, m_aDebugLine, POINTS));
		Shape.CreateLines(ARGB(255, 5, 5, 5), ShapeFlags.ONCE, m_aDebugLine, POINTS);
		//m_aShapes.Insert(Shape.CreateSphere(ARGB(255, 255, 0, 0), ShapeFlags.NOZBUFFER, m_vConnectedJointPos, 0.1));
	}

	//------------------------------------------------------------------------------------------------
	void DrawDebugShapePivots()
	{
		for (int i = 0; i < POINTS; i++)
		{
			m_aDebugLinePivots[i] = m_aCurvePivots[i];
		}

		m_aShapes.Insert(Shape.CreateLines(ARGB(123, 0, 0, 0), 0, m_aDebugLinePivots, POINTS));
		//m_aShapes.Insert(Shape.CreateSphere(ARGB(255, 255, 0, 0), ShapeFlags.NOZBUFFER, m_vConnectedJointPos, 0.1));
	}

	//------------------------------------------------------------------------------------------------
	void ModifyShape(float timeSlice)
	{
		m_fTimeAccu += m_MovementSpeed * timeSlice;
		//Print(m_fTimeAccu);
		float val1 = Math.Sin(m_fTimeAccu) * 0.5 + 0.5;
		float val2 = Math.Lerp(-m_AngleRad, m_AngleRad, val1);

		float sin = Math.Sin(val2);
		float cos = Math.Cos(val2);

		//rotation matrix start
		vector rotMat[4];
		rotMat[0][0] = cos;
		rotMat[0][1] = sin;
//		rotMat[0][2] = 0;

		rotMat[1][0] = -sin;
		rotMat[1][1] = cos;
		rotMat[1][2] = 0;

//		rotMat[2][0] = 0;
//		rotMat[2][1] = 0;
		rotMat[2][2] = 1;

//		rotMat[3][0] = 0;
//		rotMat[3][1] = 0;
//		rotMat[3][2] = 0;
		//rotation matrix end

		vector rightDir = m_vDirectionLevelNorm * vector.Up;

		vector pointMat[4];
		pointMat[0] = rightDir.Normalized();
		pointMat[1] = -vector.Up;
		pointMat[2] = m_vDirectionLevelNorm;
		//Print(pointMat);
		vector resultMat[4];
		Math3D.MatrixMultiply4(pointMat, rotMat, resultMat);
		//Print(resultMat);
		for (int i = 0; i < POINTS; i++)
		{
			float length = vector.Distance(m_aCurve[i], m_aCurvePivots[i]);
			vector v = resultMat[1] * length + m_aCurvePivots[i];
			//Print(v);
			m_aCurve[i] = v;
		}
	}

	//------------------------------------------------------------------------------------------------
	void DrawDebug(float timeSlice)
	{
		if (m_vConnectedJointPos != vector.Zero)
		{
			ModifyShape(timeSlice);
			DrawDebugShape();
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool _WB_OnKeyChanged(BaseContainer src, string key, BaseContainerList ownerContainers, IEntity parent)
	{
		if (key == "m_fSagInMeters")
			src.Get(key, m_fSagInMeters);
		else
		if (key == "m_AngleRad")
			src.Get(key, m_AngleRad);
		else
		if (key == "m_MovementSpeed")
			src.Get(key, m_MovementSpeed);

		if (GetJoint())
		{
			if (m_vConnectedJointPos != vector.Zero)
			{
				m_aShapes.Clear();
				GenerateCurve();
				GetPivots();
				//DrawDebugShapePivots();
			}
		}

		m_MovementSpeed = Math.RandomFloat(m_MovementSpeed * 0.9, m_MovementSpeed * 1.1);
		m_fTimeAccu = Math.RandomFloat(0, 100);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override int _WB_GetAfterWorldUpdateSpecs(IEntitySource src)
	{
		return EEntityFrameUpdateSpecs.CALL_WHEN_ENTITY_VISIBLE;
	}

	//------------------------------------------------------------------------------------------------
	override void _WB_AfterWorldUpdate(float timeSlice)
	{
		if (m_vConnectedJointPos == vector.Zero)
		{
			if (GetJoint())
			{
				if (m_vConnectedJointPos != vector.Zero)
				{
					m_aShapes.Clear();
					GenerateCurve();
					GetPivots();
					//DrawDebugShapePivots();
				}
			}
		}

		DrawDebug(timeSlice);
	}

	//------------------------------------------------------------------------------------------------
	void SCR_PowerLineJointEntity(IEntitySource src, IEntity parent)
	{
		m_fTimeAccu = Math.RandomFloat(0, 100);
		m_MovementSpeed = Math.RandomFloat(m_MovementSpeed * 0.9, m_MovementSpeed * 1.1);
		//Print(m_AngleRad);
	}

#endif // WORKBENCH
}
