/*
Similar to SCR_DebugShapeManager however it is designed to grant more freedom and ease of use.
Is configured by placing a DebugShapeHelper.et in the world and configuring its SCR_DebugShapeHelperComponent.
*/

[EntityEditorProps(category: "GameScripted/Helpers", description: "Allows debug shapes to be rendered. Only spawn one-instance.")]
class SCR_DebugShapeHelperComponentClass : ScriptComponentClass
{
}

class SCR_DebugShapeHelperComponent : ScriptComponent
{
	[Attribute(desc: "Limit amount of shapes drawn per tag. Older shapes get thrown out of draw list.", defvalue: "10000", category: "Limits", params: "0 inf")]
	int m_iShapeLimit;

	protected static SCR_DebugShapeHelperComponent s_Instance;
	protected static int s_iShapeLimit = 10000;
	protected static bool s_bKeepDrawList;

	protected static ref map<string, ref array<ref Managed>> s_mTaggedShapes = new map<string, ref array<ref Managed>>();

	protected static const int DEFAULT_COLOUR = MakeTransparent(Color.YELLOW);
	protected static const ShapeFlags DEFAULT_SHAPE_FLAGS = ShapeFlags.TRANSP | ShapeFlags.DOUBLESIDE | ShapeFlags.NOOUTLINE;
	protected static const ShapeFlags DEFAULT_SHAPE_FLAGS_NOZWRITE = ShapeFlags.TRANSP | ShapeFlags.DOUBLESIDE | ShapeFlags.NOOUTLINE | ShapeFlags.NOZWRITE;
	protected static const ShapeFlags DEFAULT_TRIS_FLAGS = ShapeFlags.TRANSP | ShapeFlags.DOUBLESIDE | ShapeFlags.NOZWRITE;
	protected static const ShapeFlags DEFAULT_TEXT_FLAGS = DebugTextFlags.CENTER | DebugTextFlags.FACE_CAMERA;
	protected static const string DEBUG_TAG = "SCR_DebugShapeHelperComponent"; // Groups debug objects together.

	//------------------------------------------------------------------------------------------------
	//! Creates debug shapes for TryGetValidFloorPosition, adds spheres, boxes, or cylinders based on trace type, and colors them based on success.
	//! \param[in] startPos Start position represents the initial point for creating floor and ceiling trace shapes in 3D space.
	//! \param[in] traceParam TraceParam is used to determine what shape and dimensions where traced.
	//! \param[out] floorPos Position of floor.
	//! \param[out] ceilingPos Position of ceiling.
	//! \param[in] enoughSpace Selects color based on success.
	//! \param[in] text If non-empty, creates text.
	//! \param[in] tag Used to group debug shapes togeter. Is prefixed by "SCR_DebugShapeHelper_"
	static void CreateFloorCeilingTraceShapes(World world, vector startPos, TraceParam traceParam, vector floorPos, vector ceilingPos, bool enoughSpace, string text = "", string tag = "")
	{
		string shapeTag = string.Format("%1_%2", DEBUG_TAG, tag);
		SCR_DebugShapeHelperComponent.AddSphere(startPos, 0.1, Color.VIOLET, DEFAULT_SHAPE_FLAGS_NOZWRITE, tag: shapeTag);
		if (!SCR_StringHelper.IsEmptyOrWhiteSpace(text))
			SCR_DebugShapeHelperComponent.AddText(world, startPos, text, tag: shapeTag);

		TraceBox traceBox = TraceBox.Cast(traceParam);
		if (traceBox)
		{
			SCR_DebugShapeHelperComponent.AddBox(Vector(floorPos[0] + traceBox.Mins[0], floorPos[1], floorPos[2] + traceBox.Mins[2]), Vector(ceilingPos[0] + traceBox.Maxs[0], ceilingPos[1], ceilingPos[2] + traceBox.Maxs[2]), SCR_DebugShapeHelperComponent.GetColorBasedOnSuccess(enoughSpace), DEFAULT_SHAPE_FLAGS_NOZWRITE, tag: shapeTag);
			return;
		}

		TraceSphere traceSphere = TraceSphere.Cast(traceParam);
		if (traceSphere)
		{
			int color = SCR_DebugShapeHelperComponent.GetColorBasedOnSuccess(enoughSpace);
			vector sphereOffset = Vector(0, traceSphere.Radius, 0);
			SCR_DebugShapeHelperComponent.AddCylinderBetween(floorPos + sphereOffset, ceilingPos - sphereOffset, traceSphere.Radius, color, DEFAULT_SHAPE_FLAGS_NOZWRITE, tag: shapeTag);
			SCR_DebugShapeHelperComponent.AddSphere(floorPos + sphereOffset, traceSphere.Radius, color, DEFAULT_SHAPE_FLAGS_NOZWRITE, tag: shapeTag);
			SCR_DebugShapeHelperComponent.AddSphere(ceilingPos - sphereOffset, traceSphere.Radius, color, DEFAULT_SHAPE_FLAGS_NOZWRITE, tag: shapeTag);
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Adds a debug sphere at specified position with given radius and color.
	//! \param[in] position Represents 3D position in space for creating debug sphere.
	//! \param[in] radius Radius represents the size of the sphere in the 3D space.
	//! \param[in] color Color is an integer representing the visual appearance of the debug sphere in the scene.
	static void AddSphere(vector center, float radius, int color, ShapeFlags shapeFlags = DEFAULT_SHAPE_FLAGS, string tag = "")
	{
		Shape shape = Shape.CreateSphere(
			color,
			shapeFlags,
			center,
			radius
		);
		AddShape(shape, tag);
	}

	//------------------------------------------------------------------------------------------------
	//! Adds a debug cylinder between two points with given radius and color.
	//! \param[in] bottom Bottom is the lower point of cylinder in 3D space.
	//! \param[in] top Top is the upper point of cylinder in 3D space.
	//! \param[in] radius Radius represents the cylinder's diameter in the method.
	//! \param[in] color Color parameter represents the visual appearance of the cylinder in the scene, specified by an integer value from 0 to 2
	static void AddCylinderBetween(vector bottom, vector top, float radius, int color, ShapeFlags shapeFlags = DEFAULT_SHAPE_FLAGS, string tag = "")
	{
		AddCylinder(0.5 * (bottom + top), radius, top[1] - bottom[1], color, shapeFlags, tag);
	}

	//------------------------------------------------------------------------------------------------
	//! Adds a debug cylinder at specified position with given length, radius, and color.
	//! \param[in] position Represents 3D position in space for creating debug cylinder.
	//! \param[in] length Length represents the height or size of the cylinder in the 3D space for the method.
	//! \param[in] radius Represents cylinder radius in 3D space.
	//! \param[in] color Color is an integer representing the RGBA value for the cylinder's visual appearance.
	static void AddCylinder(vector center, float radius, float length, int color, ShapeFlags shapeFlags = DEFAULT_SHAPE_FLAGS, string tag = "")
	{
		Shape shape = Shape.CreateCylinder(
			color,
			shapeFlags,
			center,
			radius,
			length
		);
		AddShape(shape, tag);
	}

	//------------------------------------------------------------------------------------------------
	//! Adds debug box with specified min, max coordinates and color.
	//! \param[in] mins Minimum coordinates for the box in 3D space.
	//! \param[in] maxs Maxs represents the maximum coordinates for the bounding box in 3D space.
	//! \param[in] color Color is an integer representing the RGB value for the box's color in debug mode.
	static void AddBox(vector mins, vector maxs, int color, ShapeFlags shapeFlags = DEFAULT_TRIS_FLAGS, string tag = "")
	{
		vector mesh[] = {
			// -Z Mins Tri 1
			Vector(mins[0], mins[1], mins[2]),
			Vector(maxs[0], mins[1], mins[2]),
			Vector(mins[0], maxs[1], mins[2]),
			// -Z Maxs Tri 2
			Vector(maxs[0], maxs[1], mins[2]),
			Vector(mins[0], maxs[1], mins[2]),
			Vector(maxs[0], mins[1], mins[2]),
			// +Z Mins Tri 3
			Vector(mins[0], mins[1], maxs[2]),
			Vector(maxs[0], mins[1], maxs[2]),
			Vector(mins[0], maxs[1], maxs[2]),
			// +Z Maxs Tri 4
			Vector(maxs[0], maxs[1], maxs[2]),
			Vector(mins[0], maxs[1], maxs[2]),
			Vector(maxs[0], mins[1], maxs[2]),
			// -X Mins Tri 5
			Vector(mins[0], mins[1], mins[2]),
			Vector(mins[0], mins[1], maxs[2]),
			Vector(mins[0], maxs[1], mins[2]),
			// -X Maxs Tri 6
			Vector(mins[0], maxs[1], maxs[2]),
			Vector(mins[0], maxs[1], mins[2]),
			Vector(mins[0], mins[1], maxs[2]),
			// +X Mins Tri 7
			Vector(maxs[0], mins[1], mins[2]),
			Vector(maxs[0], mins[1], maxs[2]),
			Vector(maxs[0], maxs[1], mins[2]),
			// +X Maxs Tri 8
			Vector(maxs[0], maxs[1], maxs[2]),
			Vector(maxs[0], maxs[1], mins[2]),
			Vector(maxs[0], mins[1], maxs[2]),
			// -Y Mins Tri 9
			Vector(mins[0], mins[1], mins[2]),
			Vector(maxs[0], mins[1], mins[2]),
			Vector(mins[0], mins[1], maxs[2]),
			// -Y Maxs Tri 10
			Vector(maxs[0], mins[1], maxs[2]),
			Vector(mins[0], mins[1], maxs[2]),
			Vector(maxs[0], mins[1], mins[2]),
			// +Y Mins Tri 11
			Vector(mins[0], maxs[1], mins[2]),
			Vector(maxs[0], maxs[1], mins[2]),
			Vector(mins[0], maxs[1], maxs[2]),
			// +Y Maxs Tri 12
			Vector(maxs[0], maxs[1], maxs[2]),
			Vector(mins[0], maxs[1], maxs[2]),
			Vector(maxs[0], maxs[1], mins[2]),
		};
		Shape shape = Shape.CreateTris(
			color,
			shapeFlags,
			mesh,
			12
		);
		AddShape(shape, tag);
	}

	//------------------------------------------------------------------------------------------------
	//! Adds debug box with specified min, max coordinates and color, transforming all points by owner transform.
	//! \param[in] mins Minimum coordinates for the box in 3D space.
	//! \param[in] maxs Maxs represents the maximum coordinates for the bounding box in 3D space.
	//! \param[in] ownerTransform Transform matrix to apply to all points.
	//! \param[in] color Color is an integer representing the RGB value for the box's color in debug mode.
	static void AddBoxWithTransform(vector mins, vector maxs, vector ownerTransform[4], int color, ShapeFlags shapeFlags = DEFAULT_TRIS_FLAGS, string tag = "")
	{
		// First create the mesh array with local coordinates
		vector mesh[] = {
			// -Z Mins Tri 1
			Vector(mins[0], mins[1], mins[2]),
			Vector(maxs[0], mins[1], mins[2]),
			Vector(mins[0], maxs[1], mins[2]),
			// -Z Maxs Tri 2
			Vector(maxs[0], maxs[1], mins[2]),
			Vector(mins[0], maxs[1], mins[2]),
			Vector(maxs[0], mins[1], mins[2]),
			// +Z Mins Tri 3
			Vector(mins[0], mins[1], maxs[2]),
			Vector(maxs[0], mins[1], maxs[2]),
			Vector(mins[0], maxs[1], maxs[2]),
			// +Z Maxs Tri 4
			Vector(maxs[0], maxs[1], maxs[2]),
			Vector(mins[0], maxs[1], maxs[2]),
			Vector(maxs[0], mins[1], maxs[2]),
			// -X Mins Tri 5
			Vector(mins[0], mins[1], mins[2]),
			Vector(mins[0], mins[1], maxs[2]),
			Vector(mins[0], maxs[1], mins[2]),
			// -X Maxs Tri 6
			Vector(mins[0], maxs[1], maxs[2]),
			Vector(mins[0], maxs[1], mins[2]),
			Vector(mins[0], mins[1], maxs[2]),
			// +X Mins Tri 7
			Vector(maxs[0], mins[1], mins[2]),
			Vector(maxs[0], mins[1], maxs[2]),
			Vector(maxs[0], maxs[1], mins[2]),
			// +X Maxs Tri 8
			Vector(maxs[0], maxs[1], maxs[2]),
			Vector(maxs[0], maxs[1], mins[2]),
			Vector(maxs[0], mins[1], maxs[2]),
			// -Y Mins Tri 9
			Vector(mins[0], mins[1], mins[2]),
			Vector(maxs[0], mins[1], mins[2]),
			Vector(mins[0], mins[1], maxs[2]),
			// -Y Maxs Tri 10
			Vector(maxs[0], mins[1], maxs[2]),
			Vector(mins[0], mins[1], maxs[2]),
			Vector(maxs[0], mins[1], mins[2]),
			// +Y Mins Tri 11
			Vector(mins[0], maxs[1], mins[2]),
			Vector(maxs[0], maxs[1], mins[2]),
			Vector(mins[0], maxs[1], maxs[2]),
			// +Y Maxs Tri 12
			Vector(maxs[0], maxs[1], maxs[2]),
			Vector(mins[0], maxs[1], maxs[2]),
			Vector(maxs[0], maxs[1], mins[2]),
		};

		// Then apply the transformation to each point
		// We know there are 36 vertices in the mesh (12 triangles * 3 vertices)
		for (int i = 0; i < 36; i++)
		{
			mesh[i] = mesh[i].Multiply4(ownerTransform);
		}

		Shape shape = Shape.CreateTris(
			color,
			shapeFlags,
			mesh,
			12
		);
		AddShape(shape, tag);
	}

	//------------------------------------------------------------------------------------------------
	//! Adds debug text at specified origin with given text.
	//! \param[in] origin Origin is the position in 3D space where debug text is added.
	//! \param[in] text Adds debug text at specified origin with given text.
	static void AddText(World world, vector origin, string text, ShapeFlags shapeFlags = DEFAULT_TEXT_FLAGS, string tag = "")
	{
		Managed shape = DebugTextWorldSpace.Create(
			world,
			text,
			shapeFlags,
			origin[0], origin[1], origin[2],
			color: Color.WHITE,
			bgColor: Color.BLACK,
			size: 13.0
		);
		AddShape(shape, tag);
	}

	//------------------------------------------------------------------------------------------------
	//! Adds shape to tagged shapes array, removes oldest shape if limit exceeded.
	//! \param[in] shape Adds shape to tagged shapes array, removes oldest shape if limit exceeded.
	//! \param[in] tag Tag is used as an identifier for grouping shapes in the method.
	static void AddShape(notnull Managed shape, string tag = "")
	{
		if (!s_Instance)
			return;

		array<ref Managed> shapes;
		if (!s_mTaggedShapes.Find(tag, shapes))
		{
			shapes = {};
			s_mTaggedShapes.Insert(tag, shapes);
		}
		shapes.Insert(shape);
		while (shapes.Count() > s_iShapeLimit)
		{
			shapes.Remove(0);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Creates an ARGB color with 8% transparency from input color.
	//! \param[in] color Transparent color component adjustment.
	//! \return a color with alpha value set to 20% transparency.
	static int MakeTransparent(int color)
	{
		int a, r, g, b;
		Color.UnpackInt(color, a, r, g, b);
		a = 0x20;
		return ARGB(a, r, g, b);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns color based on success, green if true, red if false, with transparency.
	//! \param[in] success Success represents whether an operation was successful or not in the method.
	//! \return color based on success, green for true, red for false.
	static int GetColorBasedOnSuccess(bool success)
	{
		if (success)
			return MakeTransparent(Color.GREEN);
		else
			return MakeTransparent(Color.RED);
	}

	//------------------------------------------------------------------------------------------------
	//! Calculates debug radius from trace parameter, returns sphere radius if present, otherwise maximum dimension difference in box trace.
	//! \param[in] traceParam TraceParam represents the parameters for tracing a collision shape in 3D space.
	//! \return the radius or maximum dimension of the trace parameter, depending on its type.
	static float GetDebugRadiusFromTraceParam(TraceParam traceParam)
	{
		TraceSphere traceSphere = TraceSphere.Cast(traceParam);
		if (traceSphere)
			return traceSphere.Radius;
		TraceBox traceBox = TraceBox.Cast(traceParam);
		if (traceBox) // Uses same radius for TraceOBB
			return Math.Max(traceBox.Maxs[0] - traceBox.Mins[0], traceBox.Maxs[2] - traceBox.Mins[2]);
		PrintFormat("[SCR_DebugShapeHelperComponent] GetDebugRadiusFromTraceParam has no case for TraceParam of type %1", traceParam.Type(), LogLevel.WARNING);
		return 1;
	}

	//------------------------------------------------------------------------------------------------
	//! Removes shape with given tag.
	//! \param[in] shape Removes shape with given tag from m_mTaggedShape array.
	//! \param[in] tag Tag is used to identify shapes for removal from the list.
	static void RemoveShape(notnull Managed shape, string tag = "")
	{
		array<ref Managed> shapes;
		if (s_mTaggedShapes.Find(tag, shapes))
			shapes.RemoveItem(shape);
	}

	//------------------------------------------------------------------------------------------------
	//! Removes specified tag from m_mTagedShape list.
	//! \param[in] tag Removes tag from list of tagged shapes.
	static void RemoveTag(string tag)
	{
		array<ref Managed> shapes;
		if (!s_mTaggedShapes.Find(tag, shapes))
			return;

		s_mTaggedShapes.Remove(tag);
		shapes.Clear();
	}

	//------------------------------------------------------------------------------------------------
	//! Clears all tagged shapes from the list.
	static void Clear()
	{
		s_mTaggedShapes.Clear();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] owner The owner represents the entity being initialized in the method, which is used to set event mask and clear debug elements.
	override protected void OnPostInit(IEntity owner)
	{
		s_bKeepDrawList = false;
		s_Instance = this;

		if (IsActive())
			s_iShapeLimit = Math.Max(0, m_iShapeLimit);
		else
			s_iShapeLimit = 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Called when Entity is being to be destroyed (deleted) or component to be deleted (see Game::DeleteScriptComponent).
	//! param[in] owner Entity which owns the component
	override void OnDelete(IEntity owner)
	{
		if (!s_bKeepDrawList)
			Clear();
	}

	//------------------------------------------------------------------------------------------------
	//! Saves instance so that debug shapes can be viewed in Workbench.
	//! \param[in] owner The owner represents the entity that initializes the script.
	//! \param[in,out] mat Mat is a 4x4 matrix representing the world space transformation for the entity.
	//! \param[in] src Source entity providing initialization data for the component.
	override void _WB_OnInit(IEntity owner, inout vector mat[4], IEntitySource src)
	{
		s_Instance = this;
	}

	//------------------------------------------------------------------------------------------------
	//! Any property value has been changed. You can use editor API here and do some additional edit actions which will be part of the same "key changed" action.
	override bool _WB_OnKeyChanged(IEntity owner, BaseContainer src, string key, BaseContainerList ownerContainers, IEntity parent)
	{
		if (key == "coords")
			return true;

		if (key == "m_iShapeLimit")
		{
			s_bKeepDrawList = true;
			return false;
		}

		if (key == "Enabled")
			return false;

		return true;
	}
}
