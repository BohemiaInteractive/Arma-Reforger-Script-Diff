[EntityEditorProps(category: "GameScripted/Generators", description: "Crowd Generator", dynamicBox: true, visible: false)]
class SCR_CrowdGeneratorEntityClass : SCR_AreaGeneratorBaseEntityClass
{
}

class SCR_CrowdGeneratorEntity : SCR_AreaGeneratorBaseEntity
{
	/*
		Crowd
	*/

	[Attribute(category: "Crowd", defvalue: "0.333", uiwidget: UIWidgets.Slider, desc: "Crowd density [per m²]", params: "0 2")]
	protected float m_fDensity;

	[Attribute(category: "Crowd", defvalue: "50", uiwidget: UIWidgets.Slider, desc: "Max crowd count - hard limit independent from density and shape's size", params: "1 500")]
	protected int m_iMaxCount;

	[Attribute(category: "Crowd", defvalue: "0.5", uiwidget: UIWidgets.Slider, desc: "Minimum distance between two crowd members", params: "0.25 5")]
	protected float m_fClearance;

	[Attribute(category: "Crowd", defvalue: "1", uiwidget: UIWidgets.ComboBox, desc: "Use a distribution's central density point", enums: SCR_ParamEnumArray.FromString("Uniform distribution;Random radius,Bias towards centre;Gaussian normal distribution"))]
	protected int m_iDensityMode;

	[Attribute(category: "Crowd", defvalue: "0 0 0", desc: "Density point's relative offset", params: "inf inf 0 purpose=coords space=entity")]
	protected vector m_vDensityPointOffset;

	[Attribute(category: "Crowd", defvalue: "0 0 0", desc: "Crowd's focus point offset", params: "inf inf 0 purpose=coords space=entity")] // TODO: detect road and orient towards it?
	protected vector m_vAttentionPoint;

	[Attribute(category: "Crowd", defvalue: "2.5", uiwidget: UIWidgets.Slider, desc: "Random yaw (positive or negative, e.g 10 = -5..+5)", params: "0 360 0.1")]
	protected float m_fRandomYawOffset;

	[Attribute(category: "Crowd", defvalue: "5", uiwidget: UIWidgets.Slider, desc: "Percentage of crowd to begin at an absolutely random (360°) direction", params: "0 100 0.001")]
	protected float m_fFullRandomYawPercentage;

	[Attribute(category: "Crowd", desc: "Crowd elements")]
	protected ref array<ref SCR_WeightedPrefab> m_aMembers;

	[Attribute(category: "Crowd", defvalue: "0", desc: "Play volume sound")]
	protected bool m_bPlayVolumeSound;

	[Attribute(category: "Crowd", desc: "Sound entity", params: "et")]
	protected ResourceName m_sSound;

	/*
		Debug
	*/

	[Attribute(category: "Debug", defvalue: "0", desc: "Debug")]
	protected bool m_bDebug;

#ifdef WORKBENCH

	protected static ref SCR_DebugShapeManager s_DebugShapeManager;

	protected static const int MIN_ATTEMPTS = 10; // (wantedSpawns * wantedSpawns) max MIN_ATTEMPTS

	protected static const int DEBUG_COLOUR_SPAWN = 0x8800CC00;
	protected static const int DEBUG_COLOUR_AREA = 0xFFCC0000;
	protected static const int DEBUG_COLOUR_ATTENTION = 0xFFCC00CC;
	protected static const int DEBUG_COLOUR_ATTENTION_LINES = 0x88CC00CC;
	protected static const int DEBUG_COLOUR_YAW = 0x88CCCC00;

	protected static const float DEBUG_LINE_HEIGHT = 5;
	protected static const float DEBUG_LOS_HEIGHT = 1.725;
	protected static const float DEBUG_CIRCLE_RADIUS = 0.5;
	protected static const float DEBUG_YAW_HEIGHT = DEBUG_LOS_HEIGHT - 0.25;
	protected static const float DEBUG_YAW_LENGTH = 0.5;

	//------------------------------------------------------------------------------------------------
	protected void Generate()
	{
		if (!m_bEnableGeneration)
		{
			Print("Crowd generation is disabled for this shape - tick it back on before saving", LogLevel.NORMAL);
			return;
		}

		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		if (!worldEditorAPI || worldEditorAPI.UndoOrRedoIsRestoring())
			return;

		BaseWorld world = worldEditorAPI.GetWorld();
		if (!world)
			return;

		DeleteAllChildren();

		if (!m_ParentShapeSource)
			return;

		SetSeed();

		CreateCrowd(worldEditorAPI);
	}

	//------------------------------------------------------------------------------------------------
	protected void CreateCrowd(notnull WorldEditorAPI worldEditorAPI)
	{
		if (s_DebugShapeManager)
			s_DebugShapeManager.Clear();

		int membersCount = m_aMembers.Count();
		if (membersCount < 1)
			return;

		array<float> weights = {};
		weights.Reserve(membersCount);

		float totalWeight;
		foreach (SCR_WeightedPrefab weightedPrefab : m_aMembers)
		{
			if (!weightedPrefab.m_sPrefab)
				continue;

			weights.Insert(weightedPrefab.m_fWeight);
			totalWeight += weightedPrefab.m_fWeight;
		}

		if (totalWeight <= 0)
			return;

		bool isShapeClosed;
		m_ParentShapeSource.Get("IsClosed", isShapeClosed);
		Debug.BeginTimeMeasure();
		array<vector> localPoints = GetTesselatedShapePoints(m_ParentShapeSource, isShapeClosed: isShapeClosed);
		Debug.EndTimeMeasure("Obtaining anchor points");
		if (localPoints.Count() < 3)
			return;

		Debug.BeginTimeMeasure();
		array<float> polygon = {};
		SCR_Math2D.Get2DPolygon(localPoints, polygon);
		Debug.EndTimeMeasure("Obtaining 2D polygon");

		Debug.BeginTimeMeasure();

		float minX, maxX, minZ, maxZ;
		for (int i, count = polygon.Count(); i < count; i += 2) // step 2
		{
			if (i == 0)
			{
				minX = polygon[i];
				maxX = minX;
				minZ = polygon[i + 1];
				maxZ = minZ;
				continue;
			}

			float x = polygon[i];
			float z = polygon[i + 1];

			if (minX > x)
				minX = x;
			else
			if (maxX < x)
				maxX = x;

			if (minZ > z)
				minZ = z;
			else
			if (maxZ < z)
				maxZ = z;
		}

		vector min = { minX, 0, minZ };
		vector max = { maxX, 0, maxZ };

		float totalWidth = maxX - minX;
		float totalHeight = maxZ - minZ;
		float middleX = minX + 0.5 * totalWidth;
		float middleZ = minZ + 0.5 * totalHeight;
		vector middlePos = { middleX, 0, middleZ };
		vector attentionPoint;
		if (m_vAttentionPoint != vector.Zero)
			attentionPoint = m_vAttentionPoint;
		else
			attentionPoint = middlePos;

		vector middlePosOffset;
		if (m_vDensityPointOffset != vector.Zero)
		{
			middlePosOffset = m_vDensityPointOffset;
			if (m_iDensityMode > 0)
			{
				totalWidth += Math.AbsFloat(middlePos[0] - middlePosOffset[0]) * 2;
				totalHeight += Math.AbsFloat(middlePos[2] - middlePosOffset[2]) * 2;
			}
		}
		else
		{
			middlePosOffset = middlePos;
		}

		float halfDiagonal = 0.5 * Math.Sqrt(totalWidth * totalWidth + totalHeight * totalHeight); // a² + b² = c², kids!

		Debug.EndTimeMeasure(
			string.Format(
				"Obtaining min/max (rectangular) areaSqm (<%1,%2> x <%3,%4> = %5 m²)",
				minX, maxX, minZ, maxZ, (totalWidth * totalHeight).ToString(lenDec: 2)));

		Debug.BeginTimeMeasure();
		float areaSqm = SCR_Math2D.GetPolygonArea(polygon);
		Debug.EndTimeMeasure("Obtaining areaSqm (" + areaSqm.ToString(-1, 2) + " m²)");
		if (areaSqm < 1)
			return;

		int wantedMembers = Math.Round(areaSqm * m_fDensity);
		if (wantedMembers < 1)
		{
			PrintFormat("No crowd generated: the density (%1/m²) is too low for the area (%2 m²)", m_fDensity, areaSqm.ToString(lenDec: 2), level: LogLevel.WARNING);
			return;
		}

		if (wantedMembers > m_iMaxCount) // performance safety
		{
			PrintFormat("Attempt to generate %1/%2 crowd members", wantedMembers, m_iMaxCount, level: LogLevel.NORMAL);
			wantedMembers = m_iMaxCount;
		}

		if (s_DebugShapeManager)
		{
			if (m_iDensityMode == 0)
			{
				s_DebugShapeManager.AddAABBRectangleXZ(CoordToParent(min), CoordToParent(max), DEBUG_COLOUR_AREA);
			}
			else // density point
			{
				s_DebugShapeManager.AddArrow(CoordToParent(middlePosOffset + vector.Up * DEBUG_LINE_HEIGHT), CoordToParent(middlePosOffset), 1, DEBUG_COLOUR_AREA);
				s_DebugShapeManager.AddCircleXZ(CoordToParent(middlePosOffset), DEBUG_CIRCLE_RADIUS, DEBUG_COLOUR_AREA);
				s_DebugShapeManager.AddCircleXZ(CoordToParent(middlePosOffset), halfDiagonal, DEBUG_COLOUR_AREA);
			}

			s_DebugShapeManager.AddLine(CoordToParent(attentionPoint + vector.Up * DEBUG_LINE_HEIGHT), CoordToParent(attentionPoint), DEBUG_COLOUR_ATTENTION);
			s_DebugShapeManager.AddCircleXZ(CoordToParent(attentionPoint), DEBUG_CIRCLE_RADIUS, DEBUG_COLOUR_ATTENTION);
		}

		float clearanceSq = m_fClearance * m_fClearance;
		float halfRandomYaw = m_fRandomYawOffset * 0.5;

		array<vector> existingPositions = {};
		existingPositions.Reserve(wantedMembers);

		const int maxAttempts = Math.Max(MIN_ATTEMPTS, wantedMembers * wantedMembers);
		int createdMembers;

		Debug.BeginTimeMeasure();

		int attemptId = 1;
		for (; attemptId <= maxAttempts; ++attemptId)
		{
			vector localPos = GetRandomPositionInPolygon(polygon, min, max, middlePosOffset, halfDiagonal);
			while (!Math2D.IsPointInPolygon(polygon, localPos[0], localPos[2]))
			{
				if (++attemptId > maxAttempts)
					break;

				localPos = GetRandomPositionInPolygon(polygon, min, max, middlePosOffset, halfDiagonal);
			}

			if (attemptId > maxAttempts)
			{
				PrintFormat("Max attempts count reached (%1/%2) is the area too small?", attemptId, maxAttempts, level: LogLevel.WARNING);
				break;
			}

			bool isTooClose;
			foreach (vector existingPosition : existingPositions)
			{
				if (vector.DistanceSqXZ(localPos, existingPosition) < clearanceSq)
				{
					isTooClose = true;
					break;
				}
			}

			if (isTooClose)
				continue;

			existingPositions.Insert(localPos);

			vector worldPos = CoordToParent(localPos);

			float y;
			if (!worldEditorAPI.TryGetTerrainSurfaceY(worldPos[0], worldPos[2], y))
				continue;

			worldPos[1] = y;

			// direction

			float yaw;
			float random = m_RandomGenerator.RandFloatXY(0, 100);
			if (m_fFullRandomYawPercentage != 0 && random <= m_fFullRandomYawPercentage)
			{
				yaw = m_RandomGenerator.RandFloatXY(0, 360);
			}
			else
			{
				yaw = vector.Direction(localPos, attentionPoint).ToYaw();
				if (m_fRandomYawOffset > 0)
					yaw += m_RandomGenerator.RandFloatXY(-halfRandomYaw, halfRandomYaw);
				else
					m_RandomGenerator.RandFloatXY(0, 1); // for generation to keep the same RandFloatXY count
			}

			if (s_DebugShapeManager)
			{
				vector direction = vector.FromYaw(yaw);
				s_DebugShapeManager.AddLine(CoordToParent(localPos + vector.Up * DEBUG_LOS_HEIGHT), CoordToParent(attentionPoint + vector.Up * DEBUG_LOS_HEIGHT), DEBUG_COLOUR_ATTENTION_LINES);
				s_DebugShapeManager.AddArrow(CoordToParent(localPos + vector.Up * DEBUG_YAW_HEIGHT), CoordToParent(localPos + vector.Up * DEBUG_YAW_HEIGHT + DEBUG_YAW_LENGTH * direction), 0, DEBUG_COLOUR_YAW);
			}

			int index = SCR_ArrayHelper.GetWeightedIndex(weights, m_RandomGenerator.RandFloat01());

			ResourceName resourceName = SCR_EditableEntityComponentClass.GetRandomVariant(m_aMembers[index].m_sPrefab);
			if (worldEditorAPI.CreateEntity(resourceName, "", m_iSourceLayerID, m_Source, CoordToLocal(worldPos), { 0, yaw, 0 }))
			{
				++createdMembers;

				if (s_DebugShapeManager)
					s_DebugShapeManager.AddLine(worldPos + vector.Up * DEBUG_LINE_HEIGHT, worldPos, DEBUG_COLOUR_SPAWN);

				if (createdMembers >= wantedMembers)
					break;
			}
			else
			{
				PrintFormat("Cannot create Prefab #%1: %2", index, resourceName, level: LogLevel.ERROR);
			}
		}

		Debug.EndTimeMeasure(string.Format("Creating crowd of %1/%2 members (%3/%4 attempts)", createdMembers, wantedMembers, attemptId, maxAttempts));

		// generate sound volume entity
		if (m_bPlayVolumeSound && m_sSound)
		{
			const int size = existingPositions.Count();
			if (size < 1)
				return;

			vector dimensions;
			vector position;

			if (size == 1)
			{
				dimensions = vector.One;
				position = existingPositions[0];
			}
			else
			{
				// Calculate bouding volume
				vector soundABBMins = { float.MAX, 0, float.MAX };
				vector soundsABBMaxs = -soundABBMins;

				foreach (vector existingPosition : existingPositions)
				{
					if (existingPosition[0] < soundABBMins[0])
						soundABBMins[0] = existingPosition[0];

					if (existingPosition[2] < soundABBMins[2])
						soundABBMins[2] = existingPosition[2];

					if (existingPosition[0] > soundsABBMaxs[0])
						soundsABBMaxs[0] = existingPosition[0];

					if (existingPosition[2] > soundsABBMaxs[2])
						soundsABBMaxs[2] = existingPosition[2];
				}

				dimensions[0] = soundsABBMaxs[0] - soundABBMins[0];
				dimensions[1] = 2;
				dimensions[2] = soundsABBMaxs[2] - soundABBMins[2];

				// place into center of the bounding volume
				position[0] = soundABBMins[0] + 0.5 * dimensions[0];
				position[1] = 1;
				position[2] = soundABBMins[2] + 0.5 * dimensions[2];
			}

			IEntitySource entitySource = worldEditorAPI.CreateEntity(m_sSound, string.Empty, m_iSourceLayerID, m_Source, position, vector.Zero);
			if (entitySource)
			{
				array<ref ContainerIdPathEntry> path = { new ContainerIdPathEntry(string.ToString(SCR_CrowdSoundComponent)) };
				worldEditorAPI.SetVariableValue(entitySource, path, "m_vDimensions", dimensions.ToString(false));

				const float area = dimensions[0] * dimensions[2];
				const float crowdSize = area * m_fDensity;
				worldEditorAPI.SetVariableValue(entitySource, path, "m_fCrowdSize", crowdSize.ToString(false));
			}
			else
			{
				PrintFormat("Cannot create %1 entity (Prefab %2) for %3", string.ToString(SCR_CrowdSoundComponent), m_sSound, Debug.GetEntityLinkString(this), level: LogLevel.ERROR);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] polygon
	//! \param[in] min
	//! \param[in] max
	//! \param[in] middlePosOffset
	//! \param[in] halfDiagonal
	//! \return
	protected vector GetRandomPositionInPolygon(notnull array<float> polygon, vector min, vector max, vector middlePosOffset, float halfDiagonal)
	{
		// Gaussian normal distribution
		if (m_iDensityMode == 2)
		{
			float angle = m_RandomGenerator.RandFloatXY(0, Math.PI2);
			float distance = m_RandomGenerator.RandGaussFloat(halfDiagonal / 3, 0);
			if (distance < 0)
				distance = 0;
			else
			if (distance > halfDiagonal)
				distance = halfDiagonal;

			return middlePosOffset + { Math.Cos(angle) * distance, 0, Math.Sin(angle) * distance };
		}

		// random radius
		if (m_iDensityMode == 1)
			return m_RandomGenerator.GenerateRandomPointInRadius(0, halfDiagonal, middlePosOffset, false);

		// uniform distribution
		// if (m_iDensityMode == 0)
		return m_RandomGenerator.GenerateRandomPoint(polygon, min, max);
	}

	//------------------------------------------------------------------------------------------------
	protected override void _WB_OnParentChange(IEntitySource src, IEntitySource prevParentSrc)
	{
		super._WB_OnParentChange(src, prevParentSrc);

		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		if (!worldEditorAPI || worldEditorAPI.UndoOrRedoIsRestoring())
			return;

		Generate();
	}

	//------------------------------------------------------------------------------------------------
	protected override bool _WB_OnKeyChanged(BaseContainer src, string key, BaseContainerList ownerContainers, IEntity parent)
	{
		if (!super._WB_OnKeyChanged(src, key, ownerContainers, parent))
			return false;

		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		if (!worldEditorAPI || worldEditorAPI.UndoOrRedoIsRestoring())
			return false;

		src = worldEditorAPI.EntityToSource(this); // src is not fresh enough
		BaseContainerTools.WriteToInstance(this, src); // required for changes to be considered without having to reload the world

		Generate();

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected override void _WB_OnInit(inout vector mat[4], IEntitySource src)
	{
		super._WB_OnInit(mat, src);

		if (m_bDebug)
			s_DebugShapeManager = new SCR_DebugShapeManager();
		else
			s_DebugShapeManager = null;

		if (m_ParentShapeSource && m_ParentShapeSource.GetNumChildren() == 1 && m_ParentShapeSource.GetChild(0) == src)
		{
			EntityFlags flags;
			if (m_ParentShapeSource.Get("Flags", flags))
				m_ParentShapeSource.Set("Flags", flags | EntityFlags.EDITOR_ONLY);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnShapeChangedInternal(IEntitySource shapeEntitySrc, ShapeEntity shapeEntity, array<vector> mins, array<vector> maxes)
	{
		super.OnShapeChangedInternal(shapeEntitySrc, shapeEntity, mins, maxes);

		Generate();
	}

#endif // WORKBENCH
}
