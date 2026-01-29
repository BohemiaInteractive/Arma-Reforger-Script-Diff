[EntityEditorProps(category: "GameScripted/Generators", description: "Prefab Generator", dynamicBox: true, visible: false)]
class PrefabGeneratorEntityClass : SCR_LineTerrainShaperGeneratorBaseEntityClass
{
}

// TODO: make it use SCR_ObstacleDetector?
class PrefabGeneratorEntity : SCR_LineTerrainShaperGeneratorBaseEntity
{
	/*
		Category: Mirrors
	*/

	[Attribute(category: "Mirrors", defvalue: "0", desc: "Generate mirrored Prefabs - matching according to the shape (duplicate values are ignored).\n- Empty = normal Prefab generation\n- Number # = mirror at +# and -#\n- Use 0 to have the usual central line", params: "0 inf 0.01", precision: 2)]
	protected ref array<float> m_aMirrorDistances;

	[Attribute(category: "Mirrors", defvalue: "0", desc: "Use a random (weighted) Prefab instead of the same Prefab on both sides\nThis does not apply when Perlin Asset Distribution is used")]
	protected bool m_bMirrorRandomPrefabs;

	/*
		Category: Prefabs
	*/

	[Attribute(category: "Prefabs", uiwidget: UIWidgets.ResourcePickerThumbnail, desc: "Prefab list", params: "et")]
	protected ref array<ResourceName> m_PrefabNames; // m_a

	[Attribute(category: "Prefabs", defvalue: "0", desc: "Weights", params: "0 inf")]
	protected ref array<float> m_Weights; // m_a

	[Attribute(category: "Prefabs", defvalue: "0", desc: "Only place Prefabs to shape's anchor points")]
	protected bool m_bOnlyToVertices;

	[Attribute(category: "Prefabs", defvalue: "5", desc: "Distance between spawned assets along the spline/polyline", params: "0 inf")]
	protected float m_fDistance;

	[Attribute(category: "Prefabs", defvalue: "1", desc: "Orient Prefabs to the shape's direction - if Yaw Rotation Step below is set, it will start from shape's direction")]
	protected bool m_bAlignWithShape;

	[Attribute(category: "Prefabs", defvalue: "0", desc: "Only when aligning with shape")]
	protected bool m_bUseXAsForward;

	[Attribute(category: "Prefabs", defvalue: "0", desc: "Rotate 180°")]
	protected bool m_bFlipForward;

	[Attribute(category: "Prefabs", defvalue: "0", desc: "Rotation step starting from 0 / shape direction (if Align With Shape above is used)\n- 0 = disabled, default Prefab yaw behaviour\n- 45 = 0, 45, 90, 135, 180, 225, 270 or 335°\n- 90 = 0, 90, 180 or 270°\n- 135 = 0, 135 or 270° (NOT 405-45°)\n- 180 = front or back", params: "0 180")]
	protected float m_fYawRotationStep;

	/*
		Category: Offset
	*/

	[Attribute(uiwidget: UIWidgets.None)] // obsolete, kept for Prefabs and layers retrocompatibility (since 2024-10-01)
	protected float m_fOffsetRight;

	[Attribute(category: "Offset", defvalue: "0", desc: "How much offset variability we want in meters when using offset from the center")]
	protected float m_fOffsetVariance;

	[Attribute(category: "Offset", defvalue: "0", desc: "Gap from the centerline")]
	protected float m_fGap;

	[Attribute(category: "Offset", defvalue: "0", desc: "Random Spacing Offset")]
	protected bool m_fRandomSpacing; // m_b

	[Attribute(uiwidget: UIWidgets.None)] // obsolete, kept for Prefabs and layers retrocompatibility (since 2024-10-01)
	protected float m_fOffsetUp;

	[Attribute(category: "Offset", defvalue: "0", desc: "Forward offset")]
	protected float m_fOffsetForward;

	/*
		Category: Perlin
	*/

	[Attribute(category: "Perlin", defvalue: "0", desc: "Prefab spawn density uses Perlin distribution")]
	protected bool m_bPerlinDens;

	[Attribute(category: "Perlin", defvalue: "0", desc: "Spawns prefabs if perlin value is above this threshold")]
	protected float m_fPerlinThreshold;

	[Attribute(category: "Perlin", defvalue: "0", desc: "Prefab spawn type uses Perlin distribution. Prefabs in the prefab array are stacked up from lower to higher indicies on the Y axis")]
	protected bool m_bPerlinAssetDistribution;

	[Attribute(category: "Perlin", defvalue: "0", desc: "Prefab size uses Perlin")]
	protected bool m_bPerlinSize;

	[Attribute(category: "Perlin", defvalue: "40", desc: "Perlin frequency, higher values mean smoother transitions")]
	protected float m_fPerlinFrequency;

	[Attribute(category: "Perlin", defvalue: "0", desc: "Perlin seed", params: "0 1000")]
	protected float m_fPerlinSeed;

	[Attribute(category: "Perlin", defvalue: "1", desc: "Perlin Amplitude")]
	protected float m_fPerlinAmplitude;

	[Attribute(category: "Perlin", defvalue: "0", desc: "Perlin Phase Offset")]
	protected float m_fPerlinOffset;

	[Attribute(category: "Perlin", defvalue: "0", desc: "Disables asset generation bellow perlin threshold")]
	protected bool m_fPerlinThrowAway; // m_b

	/*
		Category: Debug
	*/

	[Attribute(category: "Debug", defvalue: "0", desc: "Draw developer debug")]
	protected bool m_bDrawDebug;

#ifdef WORKBENCH

	protected ref array<ref SCR_PrefabGeneratorAssetPoint> m_aPoints = {};

	protected static ref array<ref Shape> s_aDebugShapes = {};

	protected static const float PERLIN_STEP_LENGTH = 0.01;

	//------------------------------------------------------------------------------------------------
	protected override bool _WB_OnKeyChanged(BaseContainer src, string key, BaseContainerList ownerContainers, IEntity parent)
	{
		if (!super._WB_OnKeyChanged(src, key, ownerContainers, parent))
			return false;

		if (key == "coords")
			return false;

		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		if (!worldEditorAPI || worldEditorAPI.UndoOrRedoIsRestoring())
			return false;

		IEntitySource thisSrc = worldEditorAPI.EntityToSource(this);
		IEntitySource parentSrc = thisSrc.GetParent();
		BaseContainerTools.WriteToInstance(this, thisSrc);

		OnShapeChanged(parentSrc, ShapeEntity.Cast(parent), {}, {});
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnShapeInitInternal(IEntitySource shapeEntitySrc, ShapeEntity shapeEntity)
	{
		super.OnShapeInitInternal(shapeEntitySrc, shapeEntity);

		if (!shapeEntity)
			shapeEntity = ShapeEntity.Cast(_WB_GetEditorAPI().SourceToEntity(shapeEntitySrc));

		OnShapeChanged(shapeEntitySrc, shapeEntity, {}, {});
	}

	//------------------------------------------------------------------------------------------------
	//! Get a perlin value given specific parameters
	protected float SamplePerlin(float time, float frequency = 20, float seed = 1, float amplitude = 1, float phaseOffset = 0)
	{
		if (frequency == 0 || amplitude == 0) // division/multiplication by zero protection
			return 0;

		return Math.PerlinNoise(time / frequency, seed * 100, seed * 1000) * amplitude;
	}

	//------------------------------------------------------------------------------------------------
	//! generates array of positions(+other data) where each asset is supposed to spawn
	protected void GenerateAssetPoints(notnull array<ref SCR_PrefabGeneratorPointMeta> metas)
	{
		float stepDistance = m_fDistance;

		if (m_bPerlinDens)
			stepDistance = PERLIN_STEP_LENGTH;

		float distanceWalked;
		if (m_fOffsetForward)
			stepDistance = m_fOffsetForward;

		float space;
		float moveDist;
		bool firstPoint = true;

		for (int i, countMinus1 = metas.Count() - 1; i < countMinus1; ++i)
		{
			vector posThis = metas[i].m_vPos;
			vector forward = vector.Direction(posThis, metas[i + 1].m_vPos);
			vector lastPos = posThis;
			float dist = forward.Length();
			space += dist;

			// check for disabled generation on line segment
			bool draw = metas[i].m_bGenerate;
			SCR_PrefabGeneratorAssetPoint genPoint;

			while (space >= stepDistance)
			{
				float leftOver = space - dist;
				if (leftOver < 0)
					leftOver = 0;

				if (firstPoint)
				{
					moveDist = 0;
					stepDistance = 0;
					firstPoint = false;
					if (m_fOffsetForward)
					{
						stepDistance = m_fOffsetForward;
						moveDist = dist - (space - stepDistance);
					}
				}
				else
				{
					stepDistance = m_fDistance;

					if (m_fRandomSpacing)
					{
						stepDistance = m_fDistance + m_RandomGenerator.RandFloatXY(-m_fDistance * 0.5, m_fDistance * 0.5);
						if (leftOver> stepDistance)
							leftOver = 0;
					}

					moveDist = stepDistance - leftOver;
				}

				space -= moveDist + leftOver;
				distanceWalked += stepDistance;

				vector newPos;
				float perlinValue = SamplePerlin(distanceWalked + m_fPerlinOffset, m_fPerlinFrequency, m_fPerlinSeed, m_fPerlinAmplitude);

				newPos = lastPos + forward.Normalized() * moveDist;
				lastPos = newPos;
				genPoint = new SCR_PrefabGeneratorAssetPoint();
				genPoint.m_vPos = newPos;
				genPoint.m_vForward = forward;
				genPoint.m_fPerlinWeight = perlinValue;

				if (m_bPerlinDens)
				{
					float invValue;
					if (m_fPerlinThreshold == 1) // prevent an exception
						invValue = Math.InverseLerp(0, 1, perlinValue);
					else
						invValue = Math.InverseLerp(m_fPerlinThreshold, 1, perlinValue);

					invValue = Math.Clamp(invValue, 0, 1);
					stepDistance = Math.Lerp(m_fDistance, 1, invValue); // TODO: expose values
				}

				bool throwAway = false;
				if (m_fPerlinThrowAway)
				{
					if (perlinValue < m_fPerlinThreshold)
					{
						stepDistance = PERLIN_STEP_LENGTH;
						throwAway = true;
					}
				}

				genPoint.m_bDraw = draw && !throwAway;
				m_aPoints.Insert(genPoint);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected array<ref SCR_PrefabGeneratorPointMeta> GetMetaListLineFromOffsetShape()
	{
		array<ref SCR_PrefabGeneratorPointMeta> result = {};

		array<vector> anchorPoints = m_ShapeNextPointHelper.GetAnchorPoints();
		map<int, ref ShapePointDataScriptBase> pointDataMap = GetFirstPointDataMap(PrefabGeneratorPointData);
		PrefabGeneratorPointData pointData;
		SCR_PrefabGeneratorPointMeta pointMeta;
		foreach (int i, vector point : anchorPoints)
		{
			pointMeta = new SCR_PrefabGeneratorPointMeta();
			pointMeta.m_vPos = point;

			pointData = PrefabGeneratorPointData.Cast(pointDataMap.Get(i));
			if (pointData)
				pointMeta.m_bGenerate = pointData.m_bGenerate;
			else
				pointMeta.m_bGenerate = true;

			result.Insert(pointMeta);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	protected array<ref SCR_PrefabGeneratorPointMeta> GetMetaListSplineFromShapeOffset()
	{
		array<ref SCR_PrefabGeneratorPointMeta> result = {};

		bool generate = true;
		SCR_PrefabGeneratorPointMeta tmpMeta;
		array<int> anchorTesselatedIndices = m_ShapeNextPointHelper.GetAnchorPointIndices();
		map<int, ref ShapePointDataScriptBase> pointDataMap = GetFirstPointDataMap(PrefabGeneratorPointData);
		PrefabGeneratorPointData pointData;
		foreach (int i, vector tesselatedPoint : m_ShapeNextPointHelper.GetTesselatedPoints())
		{
			if (anchorTesselatedIndices.Contains(i))
			{
				pointData = PrefabGeneratorPointData.Cast(pointDataMap.Get(i));
				if (pointData)
					generate = pointData.m_bGenerate;
			}

			tmpMeta = new SCR_PrefabGeneratorPointMeta();
			tmpMeta.m_bGenerate = generate;
			tmpMeta.m_vPos = tesselatedPoint;
			result.Insert(tmpMeta);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnShapeChangedInternal(IEntitySource shapeEntitySrc, ShapeEntity shapeEntity, array<vector> mins, array<vector> maxes)
	{
		s_aDebugShapes.Clear();
		m_aPoints.Clear();

		if (!shapeEntitySrc)
			return;

		if (!m_bEnableGeneration) // should rather be in Generate()
		{
			Print("Prefab generation is disabled for this shape - tick it back on before saving", LogLevel.NORMAL);
			return;
		}

		SetSeed();

		// TODO: move this code to a "PrepareData" method or something
		BaseContainerList points = shapeEntitySrc.GetObjectArray("Points");
		if (points != null) // TODO: save a scope
		{
			array<ref SCR_PrefabGeneratorPointMeta> pointsMetaLine = GetMetaListLineFromOffsetShape();

			int pointCount = points.Count();
			if (pointCount < 1)
				return;

			bool isShapeClosed;
			shapeEntitySrc.Get("IsClosed", isShapeClosed);

			if (pointCount > 2 && isShapeClosed)
				pointsMetaLine.Insert(pointsMetaLine[0]);

			typename shapeTypename = shapeEntitySrc.GetClassName().ToType();
			if (!m_bOnlyToVertices && m_fDistance > 0 && shapeTypename && shapeTypename.IsInherited(PolylineShapeEntity))
			{
				GenerateAssetPoints(pointsMetaLine);
				if (m_bDrawDebug)
					DrawCurveDebug(pointsMetaLine);
			}
			else if (!m_bOnlyToVertices && m_fDistance > 0 && shapeTypename && shapeTypename.IsInherited(SplineShapeEntity))
			{
				array<ref SCR_PrefabGeneratorPointMeta> pointsMetaSpline = GetMetaListSplineFromShapeOffset();

				if (pointCount > 2 && isShapeClosed)
					pointsMetaSpline.Insert(pointsMetaSpline[0]);

				GenerateAssetPoints(pointsMetaSpline);

				if (m_bDrawDebug)
					DrawCurveDebug(pointsMetaSpline);
			}
			else if (m_bOnlyToVertices) // TODO: move up so there are no !m_bOnlyToVertices checks
			{
				if (m_bDrawDebug)
					DrawCurveDebug(pointsMetaLine);

				// First point special care
				SCR_PrefabGeneratorAssetPoint genPoint = new SCR_PrefabGeneratorAssetPoint();
				genPoint.m_vPos = pointsMetaLine[0].m_vPos;
				genPoint.m_bDraw = pointsMetaLine[0].m_bGenerate;

				if (isShapeClosed && pointCount > 2)
					genPoint.m_vForward = pointsMetaLine[1].m_vPos - pointsMetaLine[pointCount - 1].m_vPos.Normalized();
				else if (pointCount > 1)
					genPoint.m_vForward = vector.Direction(pointsMetaLine[0].m_vPos, pointsMetaLine[1].m_vPos).Normalized();

				m_aPoints.Insert(genPoint);

				// Middle and end points
				for (int i = 1, count = pointsMetaLine.Count(); i < count; i++)
				{
					genPoint = new SCR_PrefabGeneratorAssetPoint();
					genPoint.m_vPos = pointsMetaLine[i % pointCount].m_vPos;

					genPoint.m_bDraw = pointsMetaLine[i].m_bGenerate;

					if (isShapeClosed || i < count - 1)
						genPoint.m_vForward = vector.Direction(pointsMetaLine[i - 1].m_vPos, pointsMetaLine[(i + 1) % pointCount].m_vPos).Normalized();
					else if (i == count - 1)
						genPoint.m_vForward = vector.Direction(pointsMetaLine[i - 1].m_vPos, pointsMetaLine[i % pointCount].m_vPos).Normalized();

					m_aPoints.Insert(genPoint);
				}
			}
			else
			{
				return;
			}
		}

		Generate();
	}

	//------------------------------------------------------------------------------------------------
	//! Generate the assets
	protected void Generate()
	{
		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		if (!worldEditorAPI || worldEditorAPI.UndoOrRedoIsRestoring())
			return;

		int prefabNamesCountMinus1 = m_PrefabNames.Count() - 1;
		if (prefabNamesCountMinus1 < 0)
			return;

		// adjust weights array size with 1
		int weightsCountMinus1 = m_Weights.Count() - 1;
		if (prefabNamesCountMinus1 > weightsCountMinus1)
		{
			for (int i = weightsCountMinus1 + 1; i <= prefabNamesCountMinus1; ++i)
			{
				m_Weights.Insert(1);
				weightsCountMinus1 ++;
			}
		}

		// set empty Prefabs a weight of zero
		foreach (int i, ResourceName prefabName : m_PrefabNames)
		{
			if (!prefabName) // .IsEmpty()
				m_Weights[i] = 0;
		}

		bool hasMoreThanZeroWeight;
		foreach (float weight : m_Weights)
		{
			if (weight > 0)
			{
				hasMoreThanZeroWeight = true;
				break;
			}
		}

		if (!hasMoreThanZeroWeight)
			return;

		set<float> mirrorDistances = new set<float>();
		foreach (float mirrorDistance : m_aMirrorDistances)
		{
			mirrorDistances.Insert(mirrorDistance);
		}

		if (mirrorDistances.IsEmpty())
			mirrorDistances.Insert(0);

		DeleteAllChildren();

		map<ResourceName, ref SCR_RandomisationEditorData> randomValuesMap = new map<ResourceName, ref SCR_RandomisationEditorData>();
		SCR_RandomisationEditorData randomnessData;

		int maxRandomYawStep;
		bool useYawStep = m_fYawRotationStep > 0;
		if (useYawStep)
			maxRandomYawStep = 360 / m_fYawRotationStep; // int, so it floors the value (2.9 = 2)

		bool isGeneratorVisible = worldEditorAPI.IsEntityVisible(m_Source);

		vector upOffset = m_vShapeOffset[1] * vector.Up;

		IEntitySource entitySource;
		BaseContainerList editorData;
		foreach (SCR_PrefabGeneratorAssetPoint localPoint : m_aPoints)
		{
			if (!localPoint.m_bDraw)
				continue;

			vector relPos = localPoint.m_vPos;
			vector rot;
			vector mat[4];
			Math3D.DirectionAndUpMatrix(localPoint.m_vForward, vector.Up, mat);
			vector right = mat[0];
			vector angles = Math3D.MatrixToAngles(mat);

			if (m_fOffsetVariance != 0)
			{
				float offsetRandom = m_RandomGenerator.RandFloatXY(-m_fOffsetVariance * 0.5 + m_fGap * 0.5, m_fOffsetVariance * 0.5 - m_fGap * 0.5);
				if (offsetRandom < 0)
					offsetRandom -= m_fGap * 0.5;
				else
					offsetRandom += m_fGap * 0.5;

				relPos += offsetRandom * right;
			}

			if (m_bAlignWithShape)
			{
				if (m_bUseXAsForward)
					angles[0] = angles[0] - 90;

				if (m_bFlipForward)
					angles[0] = angles[0] + 180;

				rot = { angles[1], angles[0], angles[2] };
			}

			float value01;
			if (m_bPerlinAssetDistribution)
			{
				float perlinValue = Math.Clamp(localPoint.m_fPerlinWeight, m_fPerlinThreshold, 1);
				if (m_fPerlinThreshold == 1) // prevent an exception
					value01 = Math.InverseLerp(0, 1, perlinValue);
				else
					value01 = Math.InverseLerp(m_fPerlinThreshold, 1, perlinValue);
			}
			else
			{
				value01 = m_RandomGenerator.RandFloat01();
			}

			int index = Math.Clamp(SCR_ArrayHelper.GetWeightedIndex(m_Weights, value01), 0, prefabNamesCountMinus1);
			ResourceName prefabToUse = m_PrefabNames[index];

			foreach (int i, float mirrorDistance : mirrorDistances)
			{
				vector worldPos = CoordToParent(relPos + mirrorDistance * right); // right mirror element
				if (m_bSnapOffsetShapeToTheGround)
					worldPos[1] = worldEditorAPI.GetTerrainSurfaceY(worldPos[0], worldPos[2]) + m_vShapeOffset[1];
				else
					worldPos += upOffset;

				vector finalRelPos = CoordToLocal(worldPos);

				entitySource = worldEditorAPI.CreateEntity(prefabToUse, string.Empty, m_iSourceLayerID, m_Source, finalRelPos, rot);

				if (!randomValuesMap.Find(prefabToUse, randomnessData))
				{
					randomnessData = SCR_RandomisationEditorData.CreateFromEntitySource(entitySource);
					if (randomnessData && (m_bAlignWithShape || useYawStep))
						randomnessData.m_bRandomYaw = false;

					randomValuesMap.Insert(prefabToUse, randomnessData);
				}

				if (randomnessData)
				{
					if (m_bPerlinSize && i == 0) // set "random" scale to Perlin value
					{
						float perlinValue = Math.Clamp(localPoint.m_fPerlinWeight, m_fPerlinThreshold, 1);

						float invValue;
						if (m_fPerlinThreshold == 1) // prevent an exception
							invValue = Math.InverseLerp(0, 1, perlinValue);
						else
							invValue = Math.InverseLerp(m_fPerlinThreshold, 1, perlinValue);

						float resultScale = Math.Lerp(randomnessData.m_vRandomScale[0], randomnessData.m_vRandomScale[1], invValue);
						randomnessData.m_vRandomScale = { resultScale, resultScale, 0 };
					}

					randomnessData.RandomiseExt(worldEditorAPI, entitySource, m_RandomGenerator);
				}

				if (useYawStep)
				{
					// I never knew my real yaw
					float stepYaw = Math.Repeat(m_RandomGenerator.RandInt(0, maxRandomYawStep) * m_fYawRotationStep, 360);
					if (entitySource.Get("angles", angles))
					{
						angles[1] = Math.Repeat(angles[1] + stepYaw, 360);
						worldEditorAPI.SetVariableValue(entitySource, null, "angles", angles.ToString(false));
					}
				}

				if (!isGeneratorVisible)
					worldEditorAPI.SetEntityVisible(entitySource, false, false);

				if (mirrorDistance == 0)
					continue;

				if (m_fOffsetVariance != 0)
				{
					float offsetRandom = m_RandomGenerator.RandFloatXY(-m_fOffsetVariance * 0.5 + m_fGap * 0.5, m_fOffsetVariance * 0.5 - m_fGap * 0.5);
					if (offsetRandom < 0)
						offsetRandom -= m_fGap * 0.5;
					else
						offsetRandom += m_fGap * 0.5;

					relPos = localPoint.m_vPos + offsetRandom * right;
				}

				worldPos = CoordToParent(relPos + mirrorDistance * -right); // left mirror element
				if (m_bSnapOffsetShapeToTheGround)
					worldPos[1] = worldEditorAPI.GetTerrainSurfaceY(worldPos[0], worldPos[2]) + m_vShapeOffset[1];
				else
					worldPos += upOffset;

				finalRelPos = CoordToLocal(worldPos);

				if (m_bMirrorRandomPrefabs && !m_bPerlinAssetDistribution)
				{
					value01 = m_RandomGenerator.RandFloat01();
					index = Math.Clamp(SCR_ArrayHelper.GetWeightedIndex(m_Weights, value01), 0, prefabNamesCountMinus1);
					prefabToUse = m_PrefabNames[index];
				}

				entitySource = worldEditorAPI.CreateEntity(prefabToUse, string.Empty, m_iSourceLayerID, m_Source, finalRelPos, rot);

				if (randomnessData)
					randomnessData.RandomiseExt(worldEditorAPI, entitySource, m_RandomGenerator);

				if (useYawStep)
				{
					float stepYaw = Math.Repeat(m_RandomGenerator.RandInt(0, maxRandomYawStep) * m_fYawRotationStep, 360);
					if (entitySource.Get("angles", angles))
					{
						angles[1] = Math.Repeat(angles[1] + stepYaw, 360);
						worldEditorAPI.SetVariableValue(entitySource, null, "angles", string.Format("%1 %2 %3", angles[0], angles[1], angles[2]));
					}
				}

				if (!isGeneratorVisible)
					worldEditorAPI.SetEntityVisible(entitySource, false, false);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Draws debug curve
	protected void DrawCurveDebug(array<ref SCR_PrefabGeneratorPointMeta> metas)
	{
		const int DEBUG_CURVE_LINE_SIZE = 8192;
		const float DEBUG_Y_MULTIPLIER = 5;
		static vector m_PerlinDebugLine[DEBUG_CURVE_LINE_SIZE];
		static vector m_ZeroDebugLine[DEBUG_CURVE_LINE_SIZE];
		static vector m_EdgeDebugLine[DEBUG_CURVE_LINE_SIZE];
		static vector m_ThresholdDebugLine[DEBUG_CURVE_LINE_SIZE];

		const float stepDistance = 0.1;
		float distanceWalked;
		float space;
		float moveDist;

		array<vector> line = {};
		int itemsNum;

		vector matWrld[4];
		GetWorldTransform(matWrld);

		int i;
		for (int countMinus1 = metas.Count() - 1; i < countMinus1; ++i)
		{
			vector posThis = metas[i].m_vPos;
			vector lastPos = posThis;
			vector forward = vector.Direction(posThis, metas[i + 1].m_vPos);
			float dist = forward.Length();
			space += dist;

			m_ZeroDebugLine[i] = posThis.Multiply4(matWrld);

			if (i == countMinus1 - 1)
				m_ZeroDebugLine[i + 1] = metas[i + 1].m_vPos.Multiply4(matWrld);

			while (space >= stepDistance)
			{
				float leftOver = space - dist;
				if (leftOver < 0)
					leftOver = 0;

				moveDist = stepDistance - leftOver;

				space -= moveDist + leftOver;
				distanceWalked += stepDistance;
				vector stepPos;
				vector perlinPointPos;

				stepPos = lastPos + forward.Normalized() * moveDist;
				lastPos = stepPos;

				vector mat[4];
				Math3D.DirectionAndUpMatrix(forward, vector.Up, mat);
				vector right = mat[0];

				float perlinValue = SamplePerlin(distanceWalked + m_fPerlinOffset, m_fPerlinFrequency, m_fPerlinSeed, m_fPerlinAmplitude);
				perlinPointPos = stepPos + right * perlinValue * DEBUG_Y_MULTIPLIER;

				if (itemsNum < DEBUG_CURVE_LINE_SIZE)
				{
					m_PerlinDebugLine[itemsNum] = perlinPointPos.Multiply4(matWrld);
					m_EdgeDebugLine[itemsNum] = (stepPos + right * DEBUG_Y_MULTIPLIER).Multiply4(matWrld);
					m_ThresholdDebugLine[itemsNum] = (stepPos + right * DEBUG_Y_MULTIPLIER * m_fPerlinThreshold).Multiply4(matWrld);
					itemsNum++;
				}
			}
		}

		s_aDebugShapes.Insert(Shape.CreateLines(ARGB(255, 255, 255, 0), ShapeFlags.NOZBUFFER, m_PerlinDebugLine, itemsNum));
		s_aDebugShapes.Insert(Shape.CreateLines(ARGB(255, 0, 255, 0), ShapeFlags.NOZBUFFER, m_ZeroDebugLine, i + 1));
		s_aDebugShapes.Insert(Shape.CreateLines(ARGB(255, 255, 0, 0), ShapeFlags.NOZBUFFER, m_EdgeDebugLine, itemsNum));
		s_aDebugShapes.Insert(Shape.CreateLines(ARGB(255, 0, 0, 255), ShapeFlags.NOZBUFFER, m_ThresholdDebugLine, itemsNum));
	}

#endif // WORKBENCH

	//------------------------------------------------------------------------------------------------
	// constructor
	protected void PrefabGeneratorEntity(IEntitySource src, IEntity parent);
}
