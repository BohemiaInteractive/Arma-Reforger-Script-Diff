class SCR_DebugShapeManager
{
	protected ref set<ref Shape> m_Shapes = new set<ref Shape>();
	protected ref set<ref DebugTextWorldSpace> m_ScreenSpaceTexts = new set<ref DebugTextWorldSpace>();

	protected static const int DEFAULT_SHAPE_COLOUR = Color.RED;
	protected static const ShapeFlags DEFAULT_SHAPE_FLAGS = ShapeFlags.NOZBUFFER | ShapeFlags.TRANSP;

	protected static const int DEFAULT_TEXT_COLOUR = Color.WHITE;
	protected static const int DEFAULT_TEXT_BACKGROUND_COLOUR = 0x88000000;
	protected static const DebugTextFlags DEFAULT_TEXT_FLAGS = DebugTextFlags.CENTER | DebugTextFlags.FACE_CAMERA | DebugTextFlags.IN_WORLD;

	//------------------------------------------------------------------------------------------------
	//! Create an axis-aligned bounding box
	//! \param[in] min
	//! \param[in] max
	//! \param[in] colour the shape's colour
	//! \param[in] additionalFlags additional Shape flags
	//! \return the created bounding box
	Shape AddBBox(vector min, vector max, int colour = DEFAULT_SHAPE_COLOUR, ShapeFlags additionalFlags = 0)
	{
		Shape shape = Shape.Create(ShapeType.BBOX, colour, DEFAULT_SHAPE_FLAGS | additionalFlags, min, max);
		m_Shapes.Insert(shape);
		return shape;
	}

	//------------------------------------------------------------------------------------------------
	//! Create a straight line
	//! \param[in] from origin
	//! \param[in] to destination
	//! \param[in] colour the shape's colour
	//! \param[in] additionalFlags additional Shape flags
	//! \return the created line
	Shape AddLine(vector from, vector to, int colour = DEFAULT_SHAPE_COLOUR, ShapeFlags additionalFlags = 0)
	{
		vector points[2] = { from, to };
		Shape shape = Shape.CreateLines(colour, DEFAULT_SHAPE_FLAGS | additionalFlags, points, 2);
		m_Shapes.Insert(shape);
		return shape;
	}

	//------------------------------------------------------------------------------------------------
	//! Create a polyline
	//! \param[in] points from 2 up to 50 points (array will be clipped)
	//! \param[in] colour the shape's colour
	//! \return the created polyline or null on error (e.g not enough points)
	Shape AddPolyLine(notnull array<vector> points, int colour = DEFAULT_SHAPE_COLOUR)
	{
		int count = points.Count();
		if (count < 2)
			return null;

		if (count > 50)
			count = 50;

		vector pointsS[50];
		for (int i; i < count; i++)
		{
			pointsS[i] = points[i];
		}

		Shape shape = Shape.CreateLines(colour, DEFAULT_SHAPE_FLAGS, pointsS, count);
		m_Shapes.Insert(shape);
		return shape;
	}

	//------------------------------------------------------------------------------------------------
	//! Create an arrow
	//! \param[in] from arrow's origin
	//! \param[in] to arrrow's destination, the pointy thing
	//! \param[in] customSize if set above zero arrow will use this value instead of 1/3 of arrow length - for a relative width, use vector.Distance(from, to) * wantedRatio
	//! \param[in] colour the shape's colour
	//! \param[in] additionalFlags additional Shape flags
	//! \return the created arrow
	Shape AddArrow(vector from, vector to, float customSize = 0, int colour = DEFAULT_SHAPE_COLOUR, ShapeFlags additionalFlags = 0)
	{
		if (customSize <= 0)
			customSize = vector.Distance(from, to) * 0.333;

		Shape shape = Shape.CreateArrow(from, to, customSize, colour, DEFAULT_SHAPE_FLAGS | additionalFlags);
		m_Shapes.Insert(shape);
		return shape;
	}

	//------------------------------------------------------------------------------------------------
	//! Create a horizontal circle
	//! \param[in] centre
	//! \param[in] radius
	//! \param[in] colour the shape's colour
	//! \return the created circle
	Shape AddCircleXZ(vector centre, float radius, int colour = DEFAULT_SHAPE_COLOUR)
	{
		Shape shape = CreateCircle(centre, vector.Up, radius, colour, radius * Math.PI, DEFAULT_SHAPE_FLAGS);
		m_Shapes.Insert(shape);
		return shape;
	}

	//------------------------------------------------------------------------------------------------
	//! Create a horizontal arc (portion of a circle)
	//! \param[in] centre
	//! \param[in] angleStartRad counter-clockwise radians
	//! \param[in] coveredAngleRad counter-clockwise radians - can be negative
	//! \param[in] radius
	//! \param[in] colour the shape's colour
	//! \return the created arc
	Shape AddCircleArcXZ(vector centre, float angleStartRad, float coveredAngleRad, float radius, int colour = DEFAULT_SHAPE_COLOUR)
	{
		if (coveredAngleRad < 0)
		{
			angleStartRad += coveredAngleRad;
			coveredAngleRad = -coveredAngleRad;
		}

		if (angleStartRad < 0 || angleStartRad > Math.PI2)
			angleStartRad = Math.Repeat(angleStartRad, Math.PI2);

		if (/* coveredAngleRad < 0 || */ coveredAngleRad > Math.PI2)
			coveredAngleRad = Math.Repeat(coveredAngleRad, Math.PI2);

		vector forward = { Math.Cos(angleStartRad), 0, Math.Sin(angleStartRad) };

		Shape shape = CreateCircleArc(centre, vector.Up, forward, 0, coveredAngleRad * Math.RAD2DEG, radius, colour, radius * (Math.PI2 - coveredAngleRad), DEFAULT_SHAPE_FLAGS);
		m_Shapes.Insert(shape);
		return shape;
	}

	//------------------------------------------------------------------------------------------------
	//! Create a horizontal circle slice
	//! \param[in] centre
	//! \param[in] angleStartRad counter-clockwise radians
	//! \param[in] coveredAngleRad counter-clockwise radians
	//! \param[in] radius
	//! \param[in] colour the shape's colour
	//! \return the created pie slice
	Shape AddCircleSliceXZ(vector centre, float angleStartRad, float coveredAngleRad, float radius, int colour = DEFAULT_SHAPE_COLOUR)
	{
		if (coveredAngleRad < 0)
		{
			angleStartRad -= coveredAngleRad;
			coveredAngleRad *= -1;
		}

		if (angleStartRad < 0 || angleStartRad > Math.PI2)
			angleStartRad = Math.Repeat(angleStartRad, Math.PI2);

		if (coveredAngleRad < 0 || coveredAngleRad > Math.PI2)
			coveredAngleRad = Math.Repeat(coveredAngleRad, Math.PI2);

		vector forward = { Math.Cos(angleStartRad), 0, Math.Sin(angleStartRad) };

		float angleDeg = coveredAngleRad * Math.RAD2DEG;
		Shape shape = CreateCircleSlice(centre, vector.Up, forward, 0, angleDeg, radius, colour, radius * (Math.PI2 - coveredAngleRad), DEFAULT_SHAPE_FLAGS);
		m_Shapes.Insert(shape);
		return shape;
	}

	//------------------------------------------------------------------------------------------------
	//! Create a rectangle
	//! \param[in] origin
	//! \param[in] vectorDir
	//! \param[in] length
	//! \param[in] width
	//! \param[in] colour the shape's colour
	//! \return the created rectangle
	Shape AddRectangle(vector origin, vector vectorDir, float length, float width, int colour = DEFAULT_SHAPE_COLOUR)
	{
		vector endPos = origin + vectorDir.Normalized() * length;

		width *= 0.5;

		float directionRad = Math.Atan2(vectorDir[2], vectorDir[0]);
		vector leftWidth = { Math.Cos(directionRad + Math.PI_HALF) * width, 0, Math.Sin(directionRad + Math.PI_HALF) * width };
		vector rightWidth = { Math.Cos(directionRad - Math.PI_HALF) * width, 0, Math.Sin(directionRad - Math.PI_HALF) * width };
		vector points[5] = {
			endPos + leftWidth,		// top-right
			endPos + rightWidth,	// bottom-right
			origin + rightWidth,	// bottom-left
			origin + leftWidth,		// top-left
			endPos + leftWidth,		// top-right
		};

		Shape shape = Shape.CreateLines(colour, DEFAULT_SHAPE_FLAGS, points, 5);
		m_Shapes.Insert(shape);
		return shape;
	}

	//------------------------------------------------------------------------------------------------
	//! Create an axis-aligned horizontal rectangle
	//! \param[in] min
	//! \param[in] max
	//! \param[in] colour the shape's colour
	//! \return the created horizontal rectangle
	Shape AddAABBRectangleXZ(vector min, vector max, int colour = DEFAULT_SHAPE_COLOUR)
	{
		vector points[5];
		points[0] = Vector(min[0], min[1], max[2]);	// top-left
		points[1] = Vector(max[0], max[1], max[2]);	// top-right
		points[2] = Vector(max[0], max[1], min[2]);	// bottom-right
		points[3] = Vector(min[0], min[1], min[2]);	// bottom-left
		points[4] = Vector(min[0], min[1], max[2]);	// top-left
		Shape shape = Shape.CreateLines(colour, DEFAULT_SHAPE_FLAGS, points, 5);
		m_Shapes.Insert(shape);
		return shape;
	}

	//------------------------------------------------------------------------------------------------
	//! Create a horizontal rectangle
	//! \param[in] origin
	//! \param[in] directionRad in counter-clockwise radians
	//! \param[in] length
	//! \param[in] width
	//! \param[in] colour the shape's colour
	//! \return the created horizontal rectangle
	Shape AddRectangleXZ(vector origin, float directionRad, float length, float width, int colour = DEFAULT_SHAPE_COLOUR)
	{
		vector endPos = origin + { Math.Cos(directionRad) * length, 0, Math.Sin(directionRad) * length };

		width *= 0.5;
		vector leftWidth = { Math.Cos(directionRad + Math.PI_HALF) * width, 0, Math.Sin(directionRad + Math.PI_HALF) * width };
		vector rightWidth = { Math.Cos(directionRad - Math.PI_HALF) * width, 0, Math.Sin(directionRad - Math.PI_HALF) * width };
		vector points[5] = {
			endPos + leftWidth,		// top-right
			endPos + rightWidth,	// bottom-right
			origin + rightWidth,	// bottom-left
			origin + leftWidth,		// top-left
			endPos + leftWidth,		// top-right
		};

		Shape shape = Shape.CreateLines(colour, DEFAULT_SHAPE_FLAGS, points, 5);
		m_Shapes.Insert(shape);
		return shape;
	}

	//------------------------------------------------------------------------------------------------
	//! Create two parallel lines on the left and on the right of the provided virtual line
	//! \param[in] origin
	//! \param[in] vectorDir can be non-normalised
	//! \param[in] length
	//! \param[in] width
	//! \param[in] colour the shape's colour
	//! \return an array with the two created lines
	array<ref Shape> AddParallelLines(vector origin, vector vectorDir, float length, float width, int colour = DEFAULT_SHAPE_COLOUR)
	{
		vector endPos = origin + vectorDir.Normalized() * length;

		width *= 0.5; // width 100 = -50 / +50
		float directionRad = Math.Atan2(vectorDir[2], vectorDir[0]);
		vector leftWidth = { Math.Cos(directionRad + Math.PI_HALF) * width, 0, Math.Sin(directionRad + Math.PI_HALF) * width };
		vector rightWidth = { Math.Cos(directionRad - Math.PI_HALF) * width, 0, Math.Sin(directionRad - Math.PI_HALF) * width };

		return {
			AddLine(origin + leftWidth, endPos + leftWidth, colour),
			AddLine(origin + rightWidth, endPos + rightWidth, colour),
		};
	}

	//------------------------------------------------------------------------------------------------
	//! Method used to create a boundary shape
	//! \param[in] boundsPositionsWorldSpace list points defining the base of the boundary
	//! \param[in] height of the walls
	//! \param[in] upDirection
	//! \param[in] colour
	//! \param[in] additionalFlags
	//! \return created shape
	Shape AddBounds(notnull array<vector> boundsPositionsWorldSpace, float height, vector upDirection = vector.Up, int colour = DEFAULT_SHAPE_COLOUR, ShapeFlags additionalFlags = 0)
	{
		vector verts[400];
		int numberOfVerts;
		int lastId = boundsPositionsWorldSpace.Count() - 1;
		vector heightOffset = upDirection * height;
		vector nextPosition;
		foreach (int i, vector position : boundsPositionsWorldSpace)
		{
			verts[numberOfVerts] = position;
			numberOfVerts++;
			if (i < lastId)
				nextPosition = boundsPositionsWorldSpace[i + 1];
			else
				nextPosition = boundsPositionsWorldSpace[0];

			verts[numberOfVerts] = nextPosition + heightOffset; // diagonal
			numberOfVerts++;
			verts[numberOfVerts] = nextPosition; // right edge
			numberOfVerts++;
			verts[numberOfVerts] = position; // bottom edge
			numberOfVerts++;
			verts[numberOfVerts] = position + heightOffset; // left edge
			numberOfVerts++;
			verts[numberOfVerts] = nextPosition + heightOffset; // top edge
			numberOfVerts++;
		}

		Shape shape = Shape.CreateLines(colour, ShapeFlags.DEFAULT | additionalFlags, verts, numberOfVerts);
		m_Shapes.Insert(shape);
		return shape;
	}

	//------------------------------------------------------------------------------------------------
	//! Create a sphere
	//! \param[in] centre world position of the sphere's centre
	//! \param[in] radius radius in metres
	//! \param[in] colour the shape's colour
	//! \param[in] additionalFlags additional Shape flags
	//! \return the created sphere
	Shape AddSphere(vector centre, float radius, int colour = DEFAULT_SHAPE_COLOUR, ShapeFlags additionalFlags = 0)
	{
		Shape shape = Shape.CreateSphere(colour, DEFAULT_SHAPE_FLAGS | additionalFlags, centre, radius);
		m_Shapes.Insert(shape);
		return shape;
	}

	//------------------------------------------------------------------------------------------------
	//! Create a trapezoidal prism
	//! \param[in] centerOfTopBasePos position in worldspace that is the center of the smaller (top) base
	//! \param[in] directionUp normalized direction in worldspace that points toward larger (bottom) base
	//! \param[in] smallBaseRadius
	//! \param[in] largeBaseRadius
	//! \param[in] height distance between smaller (top) base and center of larger (bottom) base
	//! \param[in] subdivisions min 2, max 50 (will be clamped if needed)
	//! \param[in] colour the shape's colour
	//! \param[in] additionalFlags additional Shape flags
	Shape AddTrapezoidalPrism(vector centerOfTopBasePos, vector directionUp, float smallBaseRadius, float largeBaseRadius, float height, int subdivisions = 6, int colour = DEFAULT_SHAPE_COLOUR, ShapeFlags additionalFlags = 0)
	{
		if (subdivisions < 2)
			subdivisions = 2;

		if (subdivisions > 50)
			subdivisions = 50;

		vector forward = directionUp.Perpend();
		forward.Normalize();

		vector right = directionUp * forward;
		right.Normalize();

		vector mat[3] = { right, directionUp, forward };

		float sectionDeg = 360 / subdivisions;
		subdivisions++;

		vector points[400];
		int pointsCount;
		vector point;
		vector bigBasePos = centerOfTopBasePos + vector.Up.Multiply3(mat) * height;
		for (int i; i < subdivisions; i++)
		{
			point = vector.FromYaw(sectionDeg * i) * smallBaseRadius;
			point = point.Multiply3(mat);
			points[pointsCount] = point + centerOfTopBasePos;
			pointsCount++;

			point = vector.FromYaw(sectionDeg * i) * largeBaseRadius;
			point = point.Multiply3(mat);
			points[pointsCount] = point + bigBasePos;
			pointsCount++;

			point = vector.FromYaw(sectionDeg * (i + 1)) * largeBaseRadius;
			point = point.Multiply3(mat);
			points[pointsCount] = point + bigBasePos;
			pointsCount++;

			points[pointsCount] = points[pointsCount - 2];
			pointsCount++;

			points[pointsCount] = points[pointsCount - 4];
			pointsCount++;

			point = vector.FromYaw(sectionDeg * (i + 1)) * smallBaseRadius;
			point = point.Multiply3(mat);
			points[pointsCount] = point + centerOfTopBasePos;
			pointsCount++;
		}

		Shape shape = Shape.CreateLines(colour, DEFAULT_SHAPE_FLAGS | additionalFlags, points, pointsCount);
		m_Shapes.Insert(shape);
		return shape;
	}

	//------------------------------------------------------------------------------------------------
	//! Create a world space debug text label
	//! \param[in] text
	//! \param[in] worldPos
	//! \param[in] size in metres
	//! \param[in] colour
	//! \param[in] backgroundColour
	//! \return
	DebugTextWorldSpace AddText(string text, vector worldPos, float size = 2.0, int colour = DEFAULT_TEXT_COLOUR, int backgroundColour = DEFAULT_TEXT_BACKGROUND_COLOUR)
	{
#ifdef WORKBENCH
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		BaseWorld world = worldEditor.GetApi().GetWorld();
#else	// normal game
		BaseWorld world = GetGame().GetWorld();
#endif	// WORKBENCH

		if (!world)
		{
			Print("[SCR_DebugShapeManager.AddText] cannot determine world (" + __FILE__ + " L" + __LINE__ + ")", LogLevel.WARNING);
			return null;
		}

		if (size < 0.1)
			size = 0.1;

		if (size > 10)
			size = 10;

		DebugTextWorldSpace debugText = DebugTextWorldSpace.Create(world, text, DEFAULT_TEXT_FLAGS, worldPos[0], worldPos[1], worldPos[2], size, colour, backgroundColour);
		m_ScreenSpaceTexts.Insert(debugText);
		return debugText;
	}

	//------------------------------------------------------------------------------------------------
	//! Add an external shape to be held by the manager
	//! \param[in] shape
	void Add(notnull Shape shape)
	{
		m_Shapes.Insert(shape);
	}

	//------------------------------------------------------------------------------------------------
	//! Add an external text to be held by the manager
	//! \param[in] text
	void Add(notnull DebugTextWorldSpace text)
	{
		m_ScreenSpaceTexts.Insert(text);
	}

	//------------------------------------------------------------------------------------------------
	//! Remove the provided shape, if managed
	//! \param[in] shape
	void Remove(notnull Shape shape)
	{
		m_Shapes.RemoveItem(shape);
	}

	//------------------------------------------------------------------------------------------------
	//! Remove the provided text, if managed
	//! \param[in] shape
	void Remove(notnull DebugTextWorldSpace text)
	{
		m_ScreenSpaceTexts.RemoveItem(text);
	}

	//------------------------------------------------------------------------------------------------
	//! Remove all stored shapes and texts
	void Clear()
	{
		m_Shapes.Clear();
		m_ScreenSpaceTexts.Clear();
	}

	//------------------------------------------------------------------------------------------------
	//! Remove all stored shapes
	void ClearShapes()
	{
		m_Shapes.Clear();
	}

	//------------------------------------------------------------------------------------------------
	//! Remove all stored texts
	void ClearTexts()
	{
		m_ScreenSpaceTexts.Clear();
	}
}
