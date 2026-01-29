[EntityEditorProps(category: "GameScripted/Generators", description: "Wall Generator", dynamicBox: true, visible: false)]
class WallGeneratorEntityClass : SCR_LineTerrainShaperGeneratorBaseEntityClass
{
}

class WallGeneratorEntity : SCR_LineTerrainShaperGeneratorBaseEntity
{
	/*
		Middle Object
	*/

	[Attribute(defvalue: "1", desc: "Enable middle object", category: "Middle Object")]
	protected bool m_bEnableMiddleObject;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Middle Object Prefab", "et", category: "Middle Object")]
	protected ResourceName MiddleObject;

	[Attribute(defvalue: "1", desc: "Place middle prefab at vertex", category: "Middle Object")]
	protected bool PlaceMiddleAtVertex;

	[Attribute(defvalue: "0", desc: "Place middle prefab at vertex only", category: "Middle Object")]
	protected bool PlaceMiddleAtVertexOnly;

	[Attribute("0", UIWidgets.Slider, "Middle object pre-padding, essentially a gap between previous prefab and this", params: "-5 5 0.01", category: "Middle Object")]
	protected float MiddleObjectPrePadding;

	[Attribute("0", UIWidgets.Slider,"Middle object offset to the side", params: "-5 5 0.01", category: "Middle Object")]
	protected float MiddleObjectOffsetRight;

	[Attribute("0", UIWidgets.Slider, "Middle object post-padding, essentially a gap between this asset and the next one", params: "-5 5 0.01", category: "Middle Object")]
	protected float MiddleObjectPostPadding;

	[Attribute("0", UIWidgets.Slider, "Middle object offset up/down", params: "-10 10 0.01", category: "Middle Object")]
	protected float MiddleObjectOffsetUp;

	/*
		First Object
	*/

	[Attribute(defvalue: "1", desc: "Enable first object", category: "First Object")]
	protected bool m_bEnableFirstObject;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "First Object Prefab", "et", category: "First Object")]
	protected ResourceName FirstObject;

	[Attribute("0", UIWidgets.Slider, "First object pre-padding, essentially a gap between previous vertex first object", params: "-5 5 0.01", category: "First Object")]
	protected float FirstObjectPrePadding;

	[Attribute("0", UIWidgets.Slider, "First object post-padding, essentially a gap between first object and the following one", params: "-5 5 0.01", category: "First Object")]
	protected float FirstObjectPostPadding;

	[Attribute("0", UIWidgets.Slider, "First object offset to the side", params: "-5 5 0.01", category: "First Object")]
	protected float FirstObjectOffsetRight;

	[Attribute("0", UIWidgets.Slider, "First object offset up/down", params: "-10 10 0.01", category: "First Object")]
	protected float FirstObjectOffsetUp;

	/*
		Last Object
	*/

	[Attribute(defvalue: "1", desc: "Enable last object", category: "Last Object")]
	protected bool m_bEnableLastObject;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Last Object Prefab", "et", category: "Last Object")]
	protected ResourceName LastObject;

	[Attribute("0", UIWidgets.Slider, "Last object pre-padding, essentially a gap between previous vertex first object", params: "-5 5 0.01", category: "Last Object")]
	protected float LastObjectPrePadding;

	[Attribute(uiwidget: UIWidgets.None)] // obsolete, kept for Prefabs and layers retrocompatibility
	protected float LastObjectPostPadding;

	[Attribute("0", UIWidgets.Slider,"Last object offset to the side", params: "-5 5 0.01", category: "Last Object")]
	protected float LastObjectOffsetRight;

	[Attribute("0", UIWidgets.Slider, "Last object offset up/down", params: "-10 10 0.01", category: "Last Object")]
	protected float LastObjectOffsetUp;

	/*
		Global
	*/

	[Attribute("0", UIWidgets.Slider, "Global object pre-padding, essentially a gap between previous prefab and this", params: "-2 2 0.01", category: "Global")]
	protected float PrePadding;

	[Attribute("0", UIWidgets.Slider, "Allow pre-padding on first wall asset in each line segment", params: "-2 2 0.01", category: "Global")]
	protected float PostPadding;

	[Attribute("0.5", UIWidgets.Slider, "Allow overshooting the segment line by this amount when placing assets\nOnly applies to polylines", params: "-5 5 0.01", precision: 2, category: "Global")]
	protected float m_fOvershoot;

	[Attribute(uiwidget: UIWidgets.None)] // obsolete, kept for Prefabs and layers retrocompatibility
	protected float m_fOffsetRight;

	[Attribute(uiwidget: UIWidgets.None)] // obsolete, kept for Prefabs and layers retrocompatibility
	protected float m_fOffsetUp;

	[Attribute(defvalue: "", uiwidget: UIWidgets.Object, "Contains wall groups which group Wall/Weight pairs by length", category: "Global")]
	protected ref array<ref WallLengthGroup> m_aWallGroups;

	/*
		Other
	*/

	[Attribute(defvalue: "1", desc: "Allow pre-padding on first wall asset in each line segment", category: "Other")]
	protected bool PrePadFirst;

	[Attribute(defvalue: "0", desc: "Precisely places entities to anchors while sacrificing wall assets contact", category: "Other")]
	protected bool ExactPlacement;

	[Attribute(defvalue: "0", desc: "Start the wall from the other end of the polyline", category: "Other")]
	protected bool m_bStartFromTheEnd;

	[Attribute(defvalue: "1", desc: "Rotate object so that its X axis is facing in the direction of the polyline segment", category: "Other")]
	protected bool UseXAsForward;

	[Attribute(defvalue: "0", desc: "Rotate object 180° around the Yaw axis", category: "Other")]
	protected bool Rotate180;

	[Attribute(uiwidget: UIWidgets.None)] // obsolete, kept for Prefabs and layers retrocompatibility
	protected bool UseForVerySmallObjects;

	[Attribute(uiwidget: UIWidgets.None)] // obsolete, kept for Prefabs and layers retrocompatibility
	bool m_bDebug;

	[Attribute(uiwidget: UIWidgets.None)] // obsolete, kept for Prefabs and layers retrocompatibility
	protected bool m_bSnapToTerrain;

	[Attribute(uiwidget: UIWidgets.None)] // obsolete, kept for Prefabs and layers retrocompatibility
	protected ref array<ResourceName> WallPrefabs;

#ifdef WORKBENCH

	protected static const float MIN_WALL_PIECE_SIZE = 0.001;	//!< 1mm is enough precision
	protected static const float MIN_WALL_SLOT_SIZE = 0.001;	//!< wall length, pre-padding and post-padding

	protected ref SCR_WallGroupContainer m_WallGroupContainer;

	protected static BaseWorld s_World; // tricky, but used wisely! nulled after usage

	protected static const ref map<ResourceName, float> MEASUREMENTS = new map<ResourceName, float>(); // always the same axis

	protected static const int MAX_LOOPS_SAFETY = 500; // temporary limit per section calculation

	//------------------------------------------------------------------------------------------------
	protected void Generate()
	{
		if (!m_bEnableGeneration)
		{
			Print("Wall generation is disabled for this shape - tick it back on before saving", LogLevel.NORMAL);
			return;
		}

		SetSeed();

		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		if (!worldEditorAPI || worldEditorAPI.UndoOrRedoIsRestoring())
			return;

		BaseWorld world = worldEditorAPI.GetWorld();
		if (!world)
			return;

		DeleteAllChildren();

		if (!m_ParentShapeSource || !m_ShapeNextPointHelper || !m_ShapeNextPointHelper.IsValid())
			return;

		ShapeEntity shapeEntity = ShapeEntity.Cast(worldEditorAPI.SourceToEntity(m_ParentShapeSource));
		if (!shapeEntity)
			return;

		float rotationOffset;

		int forwardAxis;
		if (UseXAsForward)
			rotationOffset = -90;
		else
			forwardAxis = 2;

		if (Rotate180)
			rotationOffset += 180;

		if (m_bStartFromTheEnd)
			rotationOffset += 180;

		MEASUREMENTS.Clear();

		array<vector> anchorPoints = m_ShapeNextPointHelper.GetAnchorPoints();

		typename type = m_ParentShapeSource.GetClassName().ToType();
		bool isPolyline = type && type.IsInherited(PolylineShapeEntity); // no "exact placement" otherwise (potential unknown shape type)

		// straight line mode variables start
		bool isStraightLineMode = isPolyline; // in case we make it a checkbox

		s_World = ((WorldEditor)Workbench.GetModule(WorldEditor)).GetApi().GetWorld();

		if (!m_WallGroupContainer) // needs s_World
			m_WallGroupContainer = new SCR_WallGroupContainer(m_aWallGroups, UseXAsForward, MiddleObject);

		if (isStraightLineMode)
			GenerateInStraightLine(anchorPoints, rotationOffset, forwardAxis);
		else
			GenerateClosestToShape(anchorPoints, rotationOffset);

		s_World = null; // see? I told you - used wisely
	}

	//------------------------------------------------------------------------------------------------
	protected void GenerateInStraightLine(notnull array<vector> anchorPoints, float rotationOffset, int forwardAxis)
	{
		map<int, ref ShapePointDataScriptBase> pointDataMap = GetFirstPointDataMap(WallGeneratorPointData);
		WallGeneratorPointData pointData;

		vector from = anchorPoints[0];
		vector to, dir, prevDir, rightVec;

		bool generate = true;
		bool exhausted, lastPoint, lastSegment, firstPass;
		ResourceName customMesh;
		float remaining;

		SCR_WallPair wall;

		float lastObjectLength;

		if (m_bEnableLastObject && LastObject)
			lastObjectLength = MeasureEntity(LastObject, forwardAxis);

		bool isGeneratorVisible = _WB_GetEditorAPI().IsEntityVisible(m_Source);

		int countMinus1 = anchorPoints.Count() - 1;
		foreach (int i, vector anchorPoint : anchorPoints)
		{
			exhausted = false;
			lastPoint = i == countMinus1;
			lastSegment = i == countMinus1 - 1;

			prevDir = dir;

			if (ExactPlacement)
			{
				if (i == 1)
					from = anchorPoint;
				else
					from = anchorPoint + (dir * PrePadding);
			}

			if (lastPoint)
			{
				to = anchorPoint;
				dir = vector.Direction(anchorPoints[i - 1], to).Normalized();
			}
			else
			{
				to = anchorPoints[i + 1];
				dir = vector.Direction(from, to).Normalized();
				if (i == 0)
					prevDir = dir;
			}

			pointData = WallGeneratorPointData.Cast(pointDataMap.Get(i));
			if (pointData)
			{
				customMesh = pointData.MeshAtPoint;
				generate = pointData.m_bGenerate;
			}
			else
			{
				customMesh = ResourceName.Empty;
			}

			remaining = vector.Distance(from, to) + m_fOvershoot;

			firstPass = true; // first segment on the polyline

			rightVec = dir * vector.Up;

			// walking the polyline line segment and populating it with assets
			// as long as there is enough room for at least the smallest wall available
			while (!exhausted)
			{
				ResourceName bestWall = ResourceName.Empty;
				float bestLen = 0;
				float prePaddingToUse = PrePadding;
				float postPaddingToUse = PostPadding;
				bool allowClipping = false;
				bool alignNext = true;
				bool prepadNext = true;
				float offsetUp = 0;
				bool custom = false;
				vector offsetRight = vector.Zero;
				bool firstPlaced = false;
				bool placeMiddle = false;
				bool placeLast = false;
				float lengthRequirement = 0;
				bool lastPlaced = false;
				bool lastInSegmentDoNotPlace = false;
				bool middleOfSegmentDoNotPlace = false;

				// first object
				if (i == 0 && firstPass && m_bEnableFirstObject && FirstObject) // !.IsEmpty()
				{
					bestWall = FirstObject;
					bestLen = MeasureEntity(FirstObject, forwardAxis);
					prePaddingToUse = FirstObjectPrePadding;
					postPaddingToUse = FirstObjectPostPadding;
					firstPlaced = true;
					offsetRight = rightVec * FirstObjectOffsetRight;
					offsetUp = FirstObjectOffsetUp;
				}
				// custom mesh from the vertex data
				else if (customMesh) // !.IsEmpty()
				{
					bestWall = customMesh;
					prePaddingToUse = pointData.PrePadding;
					postPaddingToUse = pointData.PostPadding;
					allowClipping = pointData.m_bAllowClipping;
					offsetUp = pointData.m_fOffsetUp;
					alignNext = pointData.m_bAlignWithNext;
					prepadNext = false;
					bestLen = MeasureEntity(customMesh, forwardAxis);
					customMesh = string.Empty;
					custom = true;
				}
				else if (!lastPoint)
				{
					wall = m_WallGroupContainer.GetRandomWall(remaining, m_RandomGenerator.RandFloat01());
					if (wall)
					{
						bestLen = wall.m_fWallLength;
						bestWall = wall.m_sWallAsset;
						prePaddingToUse += wall.m_fPrePadding;
						postPaddingToUse += wall.m_fPostPadding;
					}
				}
				else
				{
					// get here when the vertex has wall data with a mesh object and it's a last point on the polyline
					break;
				}

				if (bestLen + PrePadding <= 0)
				{
					prePaddingToUse = 0;
					Print("PrePadding is set too low, not using it in this instance", LogLevel.WARNING);
				}

				if (!PrePadFirst && firstPass) // do not prepad the first asset in the segment
					prePaddingToUse = 0;

				PlacePrefab(generate, bestWall, from, dir, prevDir, rotationOffset, isGeneratorVisible, bestLen, prePaddingToUse, postPaddingToUse, offsetUp, alignNext, prepadNext, false, allowClipping, offsetRight);

				remaining -= (bestLen * !allowClipping) + prePaddingToUse + postPaddingToUse; // TODO: fix bool multiplier

				placeMiddle = !custom && m_bEnableMiddleObject && MiddleObject; // !.IsEmpty();
				placeLast = lastSegment && m_bEnableLastObject && LastObject; // !.IsEmpty();

				lengthRequirement = m_WallGroupContainer.m_fSmallestWall; // the minimal space required to place the next wall asset
				if (lengthRequirement == 0)
					break;

				if (placeMiddle)
					lengthRequirement += m_WallGroupContainer.m_fMiddleObjectLength;

				if (placeLast)
					lengthRequirement += lastObjectLength;

				if (remaining < lengthRequirement)
					exhausted = true;

				lastPlaced = false;

				// no more room for 'normal' assets + placing last object
				if (exhausted && placeLast)
				{
					bestWall = LastObject;
					bestLen = lastObjectLength;
					prePaddingToUse = LastObjectPrePadding;
					postPaddingToUse = 0;
					offsetRight = rightVec * LastObjectOffsetRight;
					offsetUp = LastObjectOffsetUp;

					PlacePrefab(generate, bestWall, from, dir, prevDir, rotationOffset, isGeneratorVisible, bestLen, prePaddingToUse, postPaddingToUse, offsetUp, alignNext, prepadNext, false, allowClipping, offsetRight);
					lastPlaced = true;
					remaining -= (bestLen * !allowClipping) + prePaddingToUse + postPaddingToUse;
				}

				// placing middle object
				if (placeMiddle)
				{
					lastInSegmentDoNotPlace = exhausted && !(PlaceMiddleAtVertex || PlaceMiddleAtVertexOnly);

					// do not place middle object after first/last assets and any other assets which do not close a line segment
					middleOfSegmentDoNotPlace = (!exhausted && PlaceMiddleAtVertexOnly) || (PlaceMiddleAtVertexOnly && (lastPlaced || firstPlaced));

					if (!lastInSegmentDoNotPlace && !middleOfSegmentDoNotPlace)
					{
						from += dir * MiddleObjectPrePadding;
						offsetRight = rightVec * MiddleObjectOffsetRight;
						PlacePrefab(generate, MiddleObject, from, dir, prevDir, rotationOffset, isGeneratorVisible, m_WallGroupContainer.m_fMiddleObjectLength, MiddleObjectPrePadding, MiddleObjectPostPadding, MiddleObjectOffsetUp, true, prepadNext, true, false, offsetRight);
						remaining -= m_WallGroupContainer.m_fMiddleObjectLength + MiddleObjectPrePadding + MiddleObjectPostPadding;
					}
				}

				firstPass = false;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void GenerateClosestToShape(notnull array<vector> anchorPoints, float rotationOffset)
	{
		const bool isPolyline = false; // only used for splines as of now

		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		int anchorPointsCountMinus1 = anchorPoints.Count() - 1;
		map<int, ref ShapePointDataScriptBase> pointDataMap = GetFirstPointDataMap(WallGeneratorPointData);
		vector lastPoint = anchorPoints[anchorPointsCountMinus1];
		vector prevPoint = anchorPoints[0];

		bool generateSection = true;

		SCR_WallPair wallPair;
		WallGeneratorPointData wallGeneratorPointData;
		foreach (int anchorIndex, vector anchorPos : anchorPoints)
		{
			wallGeneratorPointData = WallGeneratorPointData.Cast(pointDataMap.Get(anchorIndex));
			if (wallGeneratorPointData)
				generateSection = wallGeneratorPointData.m_bGenerate;

			if (!generateSection) // prevent even the mesh on this point, previous generator's behaviour
				continue;

			//
			// generate anchor Prefab, whatever it is (FirstObject, MiddleObject, LastObject, PointDataObject etc)
			//

			// last anchor - generate "manually" (via CreateEntity) and leave
			if (anchorIndex == anchorPointsCountMinus1)
			{
				vector lastDirection = m_ShapeNextPointHelper.GetCurrentDirection();
				vector angles = lastDirection.VectorToAngles();
				angles[1] = Math.Repeat(angles[0] + rotationOffset, 360);
				angles[0] = 0;
				angles[2] = 0;

				vector right = lastDirection * -vector.Up;

				if (m_bEnableLastObject && LastObject)
				{
					if (
						worldEditorAPI.CreateEntity(
							LastObject,
							string.Empty,
							m_iSourceLayerID,
							m_Source,
							prevPoint
								+ LastObjectPrePadding * lastDirection
								+ LastObjectOffsetRight * right
								+ LastObjectOffsetUp * vector.Up,
							angles))
						break;
				}

				if (wallGeneratorPointData && wallGeneratorPointData.m_bGenerate && wallGeneratorPointData.MeshAtPoint)
				{
					if (
						worldEditorAPI.CreateEntity(
							wallGeneratorPointData.MeshAtPoint,
							string.Empty,
							m_iSourceLayerID,
							m_Source,
							prevPoint
								+ wallGeneratorPointData.PrePadding * lastDirection
								+ wallGeneratorPointData.m_fOffsetUp * vector.Up,
							angles))
						break;
				}

				break;
			}

			int nextAnchorIndex = anchorIndex + 1;

			bool generatedAnchorPrefab;
			if (anchorIndex == 0 && m_bEnableFirstObject && FirstObject)
			{
				if (
					CreateWallEntity(
						worldEditorAPI, s_World, FirstObject,
						prevPoint, FirstObjectOffsetUp * vector.Up + FirstObjectOffsetRight * vector.Right,
						FirstObjectPrePadding, FirstObjectPostPadding,
						rotationOffset, nextAnchorIndex))
					generatedAnchorPrefab = true;
			}

			if (!generatedAnchorPrefab && wallGeneratorPointData && wallGeneratorPointData.MeshAtPoint)
			{
				if (ExactPlacement)
				{
					m_ShapeNextPointHelper.SetOnAnchor(anchorIndex);
					prevPoint = anchorPos;
				}

				if (
					CreateWallEntity(
						worldEditorAPI, s_World, wallGeneratorPointData.MeshAtPoint,
						prevPoint, wallGeneratorPointData.m_fOffsetUp * vector.Up,
						wallGeneratorPointData.PrePadding, wallGeneratorPointData.PostPadding,
						rotationOffset, nextAnchorIndex))
					generatedAnchorPrefab = true;
			}

			if (!generatedAnchorPrefab && m_bEnableMiddleObject && (PlaceMiddleAtVertex || PlaceMiddleAtVertexOnly) && MiddleObject)
			{
				if (
					CreateWallEntity(
						worldEditorAPI, s_World, MiddleObject,
						prevPoint, MiddleObjectOffsetUp * vector.Up + MiddleObjectOffsetRight * vector.Right,
						MiddleObjectPrePadding, MiddleObjectPostPadding,
						rotationOffset, nextAnchorIndex))
					generatedAnchorPrefab = true;
			}

			//
			// generate from current point to the next anchor
			//

//			if (ExactPlacement && isPolyline && !generatedAnchorPrefab)
//			{
//				m_ShapeNextPointHelper.SetOnAnchor(anchorIndex);
//				prevPoint = anchorPos;
//			}

			float distance = float.INFINITY;
			wallPair = m_WallGroupContainer.GetRandomWall(distance, m_RandomGenerator.RandFloat01());

int maxLoops1 = MAX_LOOPS_SAFETY;
			while (wallPair)
			{
if (maxLoops1-- < 1)
{
Print("breaking while #1");
break;
}
				float length = wallPair.m_fWallLength + PrePadding + PostPadding;
				if (length < MIN_WALL_SLOT_SIZE)
				{
					PrintFormat(
						"Invalid wall length/prePadding/postPadding combination (%1/%2/%3 = %4) - using default %5",
						wallPair.m_fWallLength,
						PrePadding,
						PostPadding,
						length,
						MIN_WALL_SLOT_SIZE,
						level: LogLevel.WARNING);
					length = MIN_WALL_SLOT_SIZE;
				}

				// normal wall

int maxLoops2 = MAX_LOOPS_SAFETY;
				while (CreateWallEntity(
						worldEditorAPI, s_World, wallPair.m_sWallAsset,
						prevPoint, vector.Zero, wallPair.m_fPrePadding + PrePadding, wallPair.m_fPostPadding + PostPadding,
						rotationOffset, nextAnchorIndex))
				{
if (maxLoops2 -- < 1)
{
Print("breaking while #2");
break;
}
					if (m_bEnableMiddleObject && !PlaceMiddleAtVertexOnly && MiddleObject)
					{
						// insert a middle object after each wall
						CreateWallEntity(
							worldEditorAPI, s_World, MiddleObject,
							prevPoint, MiddleObjectOffsetUp * vector.Up + MiddleObjectOffsetRight * vector.Right, MiddleObjectPrePadding, MiddleObjectPostPadding,
							rotationOffset, nextAnchorIndex);
					}

					wallPair = m_WallGroupContainer.GetRandomWall(length, m_RandomGenerator.RandFloat01());
					if (!wallPair)
						break; // "while true" exit
				}

				// distance is too short for the current wallPair, let's retry with a shorter distance
				distance = vector.DistanceXZ(prevPoint, anchorPoints[nextAnchorIndex]); // TODO: use -shape- distance to next anchor
				wallPair = m_WallGroupContainer.GetRandomWall(distance - PrePadding - PostPadding, m_RandomGenerator.RandFloat01());
				if (!wallPair)
					break; // "while true" safety
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] entityName Prefab to measure
	//! \param[in] measureAxis 0 for X measure (default for incorrect values), 1 for Y measure, 2 for Z measure
	//! \param[in] world required
	//! \return entity side on provided axis, MIN_WALL_PIECE_SIZE if below it, -1 on failure
	static float MeasureEntity(ResourceName entityName, int measureAxis)
	{
		if (!entityName)
			return -1;

		float result;
		if (MEASUREMENTS.Find(entityName, result))
			return result;

		result = -1;

		Resource resource = Resource.Load(entityName);
		if (!resource.IsValid())
		{
			MEASUREMENTS.Insert(entityName, result);
			return result;
		}

		IEntity wallEntity;
		if (s_World) // "little" safety, soon™ SCR_WallGroupContainer will not be the one measuring entities
			wallEntity = GetGame().SpawnEntityPrefab(resource, s_World);
		else
			wallEntity = GetGame().SpawnEntityPrefab(resource, ((WorldEditor)Workbench.GetModule(WorldEditor)).GetApi().GetWorld());

		if (wallEntity)
		{
			vector minBB;
			vector maxBB;
			wallEntity.GetBounds(minBB, maxBB);
			delete wallEntity;

			if (measureAxis == 2)		// Z-axis
				result = (maxBB - minBB)[2];
			else if (measureAxis == 1)	// Y-axis
				result = (maxBB - minBB)[1];
			else						// X-axis or incorrect value
				result = (maxBB - minBB)[0];

			if (result <= 0)
			{
				Print("Wall asset length is zero, does it have a valid mesh? " + entityName, LogLevel.ERROR);
				result = -1;
			}
			else
			if (result < MIN_WALL_PIECE_SIZE)
			{
				Print("Wall asset is too small, does it have a valid mesh? " + entityName, LogLevel.ERROR);
				result = MIN_WALL_PIECE_SIZE;
			}
		}
		else
		{
			Print("Cannot measure entity " + entityName, LogLevel.WARNING);
		}

		MEASUREMENTS.Insert(entityName, result);

		return result;
	}

	//------------------------------------------------------------------------------------------------
	protected IEntitySource CreateWallEntity(
		WorldEditorAPI worldEditorAPI,
		BaseWorld world,
		ResourceName resourceName,
		inout vector startPos, // as there is Y offset, pre- and post-padding
		vector relPosOffset,
		float prePadding,
		float postPadding,
		float dirOffset,
		int anchorLimit = -1)
	{
		int axis;
		if (!UseXAsForward)
			axis = 2;

		float size = MeasureEntity(resourceName, axis) + prePadding + postPadding;
		if (size <= 0) // -1
			return null;

		vector nextPoint;
		if (!m_ShapeNextPointHelper.GetNextPoint(size, nextPoint, anchorLimit, xzMode: true))
			return null;

		if (startPos == nextPoint)
			return null;

		vector direction = vector.Direction(startPos, nextPoint).Normalized();
		if (direction == vector.Zero)
			return null;

		if (m_bSnapOffsetShapeToTheGround)
		{
			vector absPos = CoordToParent(startPos);
			absPos[1] = worldEditorAPI.GetWorld().GetSurfaceY(absPos[0], absPos[2]) + m_vShapeOffset[1];
			startPos = CoordToLocal(absPos);
		}

		startPos += prePadding * direction
			+ relPosOffset[0] * (direction * -vector.Up) + relPosOffset[1] * vector.Up; // 90° 2D rotation + altitude

		vector angles = direction.VectorToAngles();
		angles = { 0, Math.Repeat(angles[0] + dirOffset, 360), 0 };

		startPos += m_vShapeOffset[1] * vector.Up;

		IEntitySource result = worldEditorAPI.CreateEntity(resourceName, string.Empty, m_iSourceLayerID, m_Source, startPos, angles);
		startPos = nextPoint; // out
		return result;
	}

	//------------------------------------------------------------------------------------------------
	protected IEntitySource PlacePrefab(
		bool generate,
		ResourceName name,
		out vector pos,
		vector dir,
		vector prevDir,
		float rotationAdjustment,
		bool isGeneratorVisible,
		float length,
		float prePadding,
		float postPadding,
		float offsetUp,
		bool alignNext,
		bool prepadNext,
		bool snapToGround = false,
		bool allowClipping = false,
		vector offsetRight = vector.Zero)
	{
		if (!name) // .IsEmpty()
			return null;

		vector prepadDirection;
		vector orientation;

		if (alignNext)
			orientation = dir;
		else
			orientation = prevDir;

		if (prepadNext)
			prepadDirection = dir;
		else
			prepadDirection = prevDir;

		pos += prepadDirection * prePadding;
		IEntitySource ent;
		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();

		if (generate)
		{
			vector rotMat[4];
			Math3D.DirectionAndUpMatrix(orientation, vector.Up, rotMat);
			vector rot = Math3D.MatrixToAngles(rotMat);
			rot = { 0, rot[0] + rotationAdjustment, 0 };

			if (m_bSnapOffsetShapeToTheGround)
			{
				vector world = CoordToParent(pos);
				pos[1] = worldEditorAPI.GetTerrainSurfaceY(world[0], world[2]);
				if (m_ParentShapeSource)
					pos[1] = pos[1] - worldEditorAPI.SourceToEntity(m_ParentShapeSource).GetOrigin()[1];
			}

			if (snapToGround)
				ent = worldEditorAPI.CreateEntityExt(name, string.Empty, m_iSourceLayerID, m_Source, pos + offsetRight, rot, TraceFlags.ENTS);
			else
				ent = worldEditorAPI.CreateEntity(name, string.Empty, m_iSourceLayerID, m_Source, pos + offsetRight, rot);

			if (offsetUp != 0)
			{
				vector entPos;
				ent.Get("coords", entPos);
				entPos[1] = entPos[1] + offsetUp;
				if (entPos != vector.Zero)
					worldEditorAPI.SetVariableValue(ent, null, "coords", string.Format("%1 %2 %3", entPos[0], entPos[1], entPos[2]));
			}

			worldEditorAPI.SetEntityVisible(ent, isGeneratorVisible, false);
		}

		if (allowClipping)
			length = 0;

		pos += dir * Math.Max(0.1, length + postPadding);

		return ent;
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

		// caution: m_ShapeNextPointHelper is overwritten!
		if (key == "m_bStartFromTheEnd")
		{
			ShapeEntity shapeEntity = ShapeEntity.Cast(worldEditorAPI.SourceToEntity(m_ParentShapeSource));
			if (!shapeEntity)
				return false;

			array<vector> anchorPoints = {};
			array<vector> tesselatedPoints = {};
			if (!SCR_ParallelShapeHelper.GetAnchorsAndTesselatedPointsFromShape(shapeEntity, m_vShapeOffset, m_bYOffsetInShapeSpace, anchorPoints, tesselatedPoints))
			{
				PrintFormat("[WallGeneratorEntity._WB_OnKeyChanged] error getting shape points from shape with %1 points at position %2", shapeEntity.GetPointCount(), shapeEntity.CoordToParent(shapeEntity.GetOrigin()), level: LogLevel.WARNING);
				return false;
			}

			if (!m_bStartFromTheEnd) // actually activated - going from false to true (not yet saved to container)
			{
				SCR_ArrayHelperT<vector>.Reverse(anchorPoints);
				SCR_ArrayHelperT<vector>.Reverse(tesselatedPoints);
			}

			m_ShapeNextPointHelper = SCR_ShapeNextPointHelper.CreateFromPoints(anchorPoints, tesselatedPoints);
		}

		Generate();

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnShapeChangedInternal(IEntitySource shapeEntitySrc, ShapeEntity shapeEntity, array<vector> mins, array<vector> maxes)
	{
		super.OnShapeChangedInternal(shapeEntitySrc, shapeEntity, mins, maxes);

		Generate();
	}

#endif WORKBENCH
}
