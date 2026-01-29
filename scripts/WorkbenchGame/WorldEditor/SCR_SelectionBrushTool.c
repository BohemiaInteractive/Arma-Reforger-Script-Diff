#ifdef WORKBENCH
[WorkbenchToolAttribute(
	name: "Object Selection Brush",
	description: "Select objects by 2D or 3D radius - click or click and drag\n" +
		"Escape to deselect everything\n" +
		"Space to switch to selection eraser mode\n" +
		"Shift to temporarily bypass the filter\n" +
		"Ctrl+Click to forcefully add/remove an entity to the selection",
	// shortcut: "S", // unused
	wbModules: { "WorldEditor" },
	awesomeFontCode: 0xF111)]
class SCR_SelectionBrushTool : WorldEditorTool
{
	/*
		Category: Brush
	*/

	[Attribute(defvalue: "10", uiwidget: UIWidgets.Slider, desc: "Brush radius", params: string.Format("%1 %2 %3", RADIUS_MIN, RADIUS_MAX, RADIUS_STEP), category: "Brush")]
	protected float m_fRadius;

	[Attribute(defvalue: "1", desc: "Use 3D detection, otherwise use 2D", category: "Brush")]
	protected bool m_bDetectBySphere;

	[Attribute(defvalue: "0", desc: "Snap brush to objects, otherwise ignore them", category: "Brush")]
	protected bool m_bSnapToObjects;

	[Attribute(desc: "Find objects using world Trace instead of Entities with MeshObjects - 2-3× the performance cost", category: "Brush")]
	protected bool m_bUseTraceDetection;

	/*
		Category: Selection
	*/

	[Attribute(defvalue: SCR_ESelectionBrushToolLayer.ALL_LAYERS.ToString(), uiwidget: UIWidgets.ComboBox, enumType: SCR_ESelectionBrushToolLayer, category: "Selection")]
	SCR_ESelectionBrushToolLayer m_eLayerSelection;

	[Attribute(defvalue: "0", desc: "Select the topmost 3D parent, otherwise select the 3D world entity", category: "Selection")]
	protected bool m_bSelectParentOnly;

	[Attribute(defvalue: "2000", desc: "Performance-related selection limit - 0 for no limit", uiwidget: UIWidgets.Slider, params: "0 10000 100", category: "Selection")]
	protected int m_iMaxSelectedEntities;

	/*
		Category: Filter
	*/

	[Attribute(desc: "Define a list of objects to select - can take a SCR_ObjectBrushArrayConfig .conf", category: "Filter")]
	protected ref SCR_SelectionBrushConfig m_ObjectsFilter;

	protected ref TraceSphere m_TraceSphere;

	protected bool m_bIsWorldValid;
	protected bool m_bIsMouseHeldDown;
	protected bool m_bIsInRemovalMode;
	protected bool m_bSmiley;
	protected bool m_bLimitReachedWarning;

	//! first shape is the 2D circle
	//! second and third shape are horizontal/vertical lines
	//! fourth shape is either the sphere or the cylinder
	//! following shapes are altitude marks
	protected ref array<ref Shape> m_aBrushShapes;

	protected vector m_vBrushShapePos; //!< used to re-create the brush on scrollwheel

	protected static const int BRUSH_SHAPE_INDEX = 3;
	protected static const int BRUSH_COLOUR_ON_HOLD			= ARGB(64, 128, 128, 128);	//!< moving without click
	protected static const int BRUSH_COLOUR_NORMAL			= ARGB(128, 128, 128, 0);	//!< clicked, no detection
	protected static const int BRUSH_COLOUR_NORMAL_2D		= ARGB(255, 0, 255, 0);		//!< 2D circle
	protected static const int BRUSH_COLOUR_GUIDE_2D		= ARGB(64, 0, 255, 0);		//!< cylinder guide colour
	protected static const int BRUSH_COLOUR_CAN_ADD			= ARGB(128, 128, 64, 0);	//!< "entities are around"
	protected static const int BRUSH_COLOUR_ADDED			= ARGB(192, 128, 0, 0);		//!< "added an entity"
	protected static const int BRUSH_COLOUR_REMOVAL			= ARGB(128, 64, 64, 255);	//!< "removal mode"
	protected static const int BRUSH_COLOUR_REMOVED			= ARGB(192, 0, 0, 128);		//!< "removed an entity"
	protected static const int BRUSH_COLOUR_ALTITUDE		= ARGB(255, 255, 0, 0);		//!< ALT altitude indicator
	protected static const int BRUSH_GUIDE_2D_HEIGHT		= 1000;
	protected static const float BRUSH_ALTITUDE_TOLERANCE	= 0.5;						//!< won't show ATL indicator if ATL Y is below this tolerance

	protected static const float RADIUS_MIN = 1;
	protected static const float RADIUS_MAX = 100;
	protected static const float RADIUS_STEP = 1;

	protected static const int BRUSH_2D_Y_SEARCH = 10000; // 5k up, 5k down
	// protected static const string MESHOBJECT_CLASSNAME = ((typename)MeshObject).ToString();
	protected static const string MESHOBJECT_CLASSNAME = "MeshObject";

	//------------------------------------------------------------------------------------------------
	//! Event triggering on Mouse click - see WorldEditorTool.OnMousePressEvent()
	//! Filtered to work only for left-click here
	//! Creates/Refreshes the Obstacles Detector and deals with Click / Alt+Click
	//! \param[in] x
	//! \param[in] y
	//! \param[in] buttons
	protected override void OnMousePressEvent(float x, float y, WETMouseButtonFlag buttons)
	{
		if (!m_bIsWorldValid)
			return;

		if (buttons != WETMouseButtonFlag.LEFT)
			return;

		vector traceStart, traceEnd, traceDir;
		TraceFlags flags = TraceFlags.WORLD;
		if (m_bSnapToObjects)
			flags |= TraceFlags.ENTS;

		m_API.TraceWorldPos(x, y, flags, traceStart, traceEnd, traceDir);

		if (traceEnd == vector.Zero)
			return;

		m_bIsMouseHeldDown = true;

		IEntitySource entity = m_API.GetEntityUnderCursor();
		if (entity && GetModifierKeyState(ModifierKey.CONTROL))
		{
			if (m_API.IsEntitySelected(entity))
				m_API.RemoveFromEntitySelection(entity);
			else
				m_API.AddToEntitySelection(entity);
		}
		else
		{
			m_bSmiley = Math.RandomFloat01() > 0.999;
			CreateBrush(traceEnd);
			if (m_bIsInRemovalMode)
				DeselectAroundCursor(traceEnd);
			else
				SelectAroundCursor(traceEnd);

			m_vBrushShapePos = traceEnd;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Event triggering on Mouse movement - see WorldEditorTool.OnMouseMoveEvent()
	//! Create/Delete entities and draw Alt+Click additional shapes
	//! \param[in] x
	//! \param[in] y
	//! \param[in] buttons
	protected override void OnMouseMoveEvent(float x, float y)
	{
		if (!m_bIsWorldValid)
			return;

		vector traceStart, traceEnd, traceDir;
		TraceFlags flags = TraceFlags.WORLD;
		if (m_bSnapToObjects)
			flags |= TraceFlags.ENTS;

		m_API.TraceWorldPos(x, y, flags, traceStart, traceEnd, traceDir);

		if (traceEnd == vector.Zero)
		{
			m_aBrushShapes = null;
			return;
		}

		CreateBrush(traceEnd);
		m_vBrushShapePos = traceEnd;

		if (m_bIsMouseHeldDown)
		{
			if (m_bIsInRemovalMode)
				DeselectAroundCursor(traceEnd);
			else
				SelectAroundCursor(traceEnd);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Event triggering on Mouse scroll wheel - see WorldEditorTool.OnWheelEvent()
	//! Used to set Brush's radius (Ctrl)
	//! \param[in] delta the scroll wheel difference value
	protected override void OnWheelEvent(int delta)
	{
		// adjusts m_fRadius value using a CTRL + Scrollwheel keybind
		if (GetModifierKeyState(ModifierKey.CONTROL))
		{
			m_fRadius = AdjustValueUsingScrollWheel(delta, m_fRadius, RADIUS_MIN, RADIUS_MAX, RADIUS_STEP);
			UpdatePropertyPanel();

			if (m_aBrushShapes)
				CreateBrush(m_vBrushShapePos);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Event triggering on Mouse click release - see WorldEditorTool.OnMouseReleaseEvent()
	//! Filtered to work only for left-click here
	//! Clears "held mouse click" flag
	//! \param[in] x
	//! \param[in] y
	//! \param[in] buttons
	protected override void OnMouseReleaseEvent(float x, float y, WETMouseButtonFlag buttons)
	{
		if (!m_bIsMouseHeldDown)
			return;

		if (buttons != WETMouseButtonFlag.LEFT)
			return;

		m_bIsMouseHeldDown = false;
		if (m_aBrushShapes)
			SetBrushColor(BRUSH_COLOUR_ON_HOLD);

		if (m_bLimitReachedWarning)
			m_bLimitReachedWarning = false;
		else
			Print("" + m_API.GetSelectedEntitiesCount() + "/" + m_iMaxSelectedEntities + " entities selected", LogLevel.NORMAL);

		m_API.UpdateSelectionGui();
	}

	//------------------------------------------------------------------------------------------------
	//! Event triggering on keyboard (normal) key press - see WorldEditorTool.OnKeyPressEvent()
	//! Used to switch to Delete mode (Space) or cancel Alt+Click (Esc)
	//! \param[in] key
	//! \param[in] isAutoRepeat
	protected override void OnKeyPressEvent(KeyCode key, bool isAutoRepeat)
	{
		/**/ if (key == KeyCode.KC_ESCAPE)
		{
			m_API.ClearEntitySelection();
			m_API.UpdateSelectionGui();
		}
		else if (key == KeyCode.KC_DELETE)
		{
			DeleteSelectedEntities();
		}
		else if (key == KeyCode.KC_SPACE)
		{
			m_bIsInRemovalMode = true;
			CreateBrush(m_vBrushShapePos);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Event triggering on keyboard (normal) key release - see WorldEditorTool.OnKeyReleaseEvent()
	//! Used to switch from Deselect mode (Space)
	//! \param[in] key
	//! \param[in] isAutoRepeat
	protected override void OnKeyReleaseEvent(KeyCode key, bool isAutoRepeat)
	{
		if (key == KeyCode.KC_SPACE)
		{
			m_bIsInRemovalMode = false;
			CreateBrush(m_vBrushShapePos);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnActivate()
	{
		m_bIsWorldValid = IsWorldValid();
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnDeActivate()
	{
		m_aBrushShapes = null;
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnAfterLoadWorld()
	{
		m_bIsWorldValid = IsWorldValid();
	}

	//------------------------------------------------------------------------------------------------
	protected void SelectAroundCursor(vector cursorWorldPos)
	{
		array<IEntity> entities = GetAllEntitiesAroundCursor(cursorWorldPos);

		IEntity entity;
		IEntitySource entitySource;
		bool selectFromAllLayers = m_eLayerSelection == SCR_ESelectionBrushToolLayer.ALL_LAYERS;
		int currentLayerId = m_API.GetCurrentEntityLayerId();

		for (int i = entities.Count() - 1; i >= 0; i--)
		{
			entity = entities[i];
			entitySource = m_API.EntityToSource(entity);
			if (!entitySource)
				continue;

			bool remove = false;

			if (!(selectFromAllLayers || CanSelectEntityFromItsLayer(currentLayerId, entitySource)))	// layer check
				remove = true;

			if (!remove && vector.Distance(cursorWorldPos, entity.GetOrigin()) > m_fRadius)				// distance check
				remove = true;

			if (!remove && !m_bUseTraceDetection && !HasMeshObject(entitySource))						// MeshObject check - only on noTrace search
				remove = true;

			if (!remove && m_bSelectParentOnly && entitySource != GetTopMostParentWithMeshObject(entitySource)) //parent-only check
				remove = true;

			if (remove)
				entities.Remove(i);
		}

		if (entities.IsEmpty())
		{
			SetBrushColor(BRUSH_COLOUR_NORMAL);
			return;
		}

		if (m_ObjectsFilter && m_ObjectsFilter.m_aPrefabs && !GetModifierKeyState(ModifierKey.SHIFT))
		{
			for (int i = entities.Count() - 1; i >= 0; i--)
			{
				ResourceName entityPrefab = entities[i].GetPrefabData().GetPrefabName();
				bool found = false;
				foreach (ResourceName prefab : m_ObjectsFilter.m_aPrefabs)
				{
					if (prefab == string.Empty) // not .IsEmpty() for performance
						continue;

					if (entityPrefab == prefab)
					{
						found = true;
						break;
					}
				}

				if (!found)
					entities.Remove(i);
			}
		}

		int selectedEntitiesCount = m_API.GetSelectedEntitiesCount();
		foreach (IEntity entity2 : entities)
		{
			IEntitySource src2 = m_API.EntityToSource(entity2);
			
			if (m_iMaxSelectedEntities > 0 && selectedEntitiesCount >= m_iMaxSelectedEntities)
				break;

			if (m_API.IsEntitySelected(src2))
				continue;

			m_API.AddToEntitySelection(src2);
			selectedEntitiesCount++;
		}

		if (selectedEntitiesCount != m_API.GetSelectedEntitiesCount())
			SetBrushColor(BRUSH_COLOUR_ADDED);
		else
			SetBrushColor(BRUSH_COLOUR_CAN_ADD);

		if (m_iMaxSelectedEntities > 0 && !m_bLimitReachedWarning && selectedEntitiesCount >= m_iMaxSelectedEntities)
		{
			PrintFormat("%1/%2 entities selected", selectedEntitiesCount, m_iMaxSelectedEntities, level: LogLevel.WARNING);
			m_bLimitReachedWarning = true;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool CanSelectEntityFromItsLayer(int currentLayerId, notnull IEntitySource entitySource)
	{
		return m_eLayerSelection == SCR_ESelectionBrushToolLayer.ALL_LAYERS ||
			(currentLayerId == entitySource.GetLayerID()) == (m_eLayerSelection == SCR_ESelectionBrushToolLayer.CURRENT_LAYER);
	}

	//------------------------------------------------------------------------------------------------
	protected bool HasMeshObject(notnull IEntitySource entitySource)
	{
		IEntityComponentSource componentSource;
		for (int i, count = entitySource.GetComponentCount(); i < count; i++)
		{
			componentSource = entitySource.GetComponent(i);
			if (componentSource.GetClassName() == MESHOBJECT_CLASSNAME)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! return provided entitySource if no parent, whether or not it has a MeshObject component or not
	protected IEntitySource GetTopMostParentWithMeshObject(notnull IEntitySource entitySource)
	{
		IEntitySource parent = entitySource.GetParent();

		while (parent)
		{
			if (HasMeshObject(parent))
				entitySource = parent;

			parent = parent.GetParent();
		}

		return entitySource;
	}

	//------------------------------------------------------------------------------------------------
	// this one does not use Trace, config or layer info
	protected void DeselectAroundCursor(vector cursorWorldPos)
	{
		array<IEntitySource> entities = {};
		
		for (int i = m_API.GetSelectedEntitiesCount() - 1; i >= 0; i--)
		{
			IEntitySource entity = m_API.GetSelectedEntity(i);
			if (m_bDetectBySphere)
			{
				if (vector.Distance(cursorWorldPos, m_API.SourceToEntity(entity).GetOrigin()) <= m_fRadius)
					entities.Insert(entity);
			}
			else
			{
				if (vector.DistanceXZ(cursorWorldPos, m_API.SourceToEntity(entity).GetOrigin()) <= m_fRadius)
					entities.Insert(entity);
			}
		}

		foreach (IEntitySource entity : entities)
		{
			m_API.RemoveFromEntitySelection(entity);
		}

		if (entities.IsEmpty())
			SetBrushColor(BRUSH_COLOUR_REMOVAL);
		else
			SetBrushColor(BRUSH_COLOUR_REMOVED);
	}

	//------------------------------------------------------------------------------------------------
	protected array<IEntity> GetAllEntitiesAroundCursor(vector cursorWorldPos)
	{
		if (m_bUseTraceDetection)
		{
			if (m_bDetectBySphere)
			{
				m_TraceSphere.Start = cursorWorldPos;
				// m_TraceSphere.End = cursorWorldPos;
				m_TraceSphere.Radius = m_fRadius;

				return SCR_WorldEditorToolHelper.TracePositionEntitiesBySphere(m_API.GetWorld(), m_TraceSphere);
			}
			else // 2D detection
			{
				m_TraceSphere.Start = cursorWorldPos + { 0, BRUSH_2D_Y_SEARCH * 0.5, 0 };
				m_TraceSphere.End = cursorWorldPos - { 0, BRUSH_2D_Y_SEARCH * 0.5, 0 };
				m_TraceSphere.Radius = m_fRadius;

				return SCR_WorldEditorToolHelper.TraceMoveEntitiesBySphere(m_API.GetWorld(), m_TraceSphere);
			}
		}
		else // AABB detection
		{
			if (m_bDetectBySphere)
			{
				return SCR_WorldEditorToolHelper.QueryEntitiesBySphere(m_API.GetWorld(), cursorWorldPos, m_fRadius);
			}
			else
			{
				vector minAABB = cursorWorldPos - { m_fRadius, BRUSH_2D_Y_SEARCH * 0.5, m_fRadius };
				vector maxAABB = cursorWorldPos + { m_fRadius, BRUSH_2D_Y_SEARCH * 0.5, m_fRadius };
				return SCR_WorldEditorToolHelper.QueryEntitiesByAABB(m_API.GetWorld(), minAABB, maxAABB);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Helps getting proper new value for a property
	//! \param[in] delta the scrollwheel value (obtained in OnWheelEvent) that is a multiple of 120
	//! \param[in] currentValue the value from which to start
	//! \param[in] min the min value
	//! \param[in] max the max value
	//! \param[in] step the step by which delta's converted value will be multiplied
	//! \return the min-max clamped new value
	protected float AdjustValueUsingScrollWheel(float delta, float currentValue, float min, float max, float step)
	{
		// delta returns multiples of 120 - converting it into a more useable value of multiples of 1
		float value = currentValue + (delta / 120) * step;

		if (value < min)
			return min;

		if (value > max)
			return max;

		return value;
	}

	//------------------------------------------------------------------------------------------------
	protected void DeleteSelectedEntities()
	{
		int selectedEntitiesCount = m_API.GetSelectedEntitiesCount();
		bool manageEditAction = SCR_WorldEditorToolHelper.BeginEntityAction();
		Debug.BeginTimeMeasure();
		for (int i = 0; i < selectedEntitiesCount; i++)
		{
			IEntitySource entity = m_API.GetSelectedEntity(i);
			if (entity) // deleted objects can have children selected too, having them deleted
				m_API.DeleteEntity(entity);
		}
		Debug.EndTimeMeasure("Deleted " + selectedEntitiesCount + " entities");
		SCR_WorldEditorToolHelper.EndEntityAction(manageEditAction);
	}

	//------------------------------------------------------------------------------------------------
	protected void CreateBrush(vector worldPos)
	{
		int colour = BRUSH_COLOUR_ON_HOLD;
		if (m_bIsMouseHeldDown)
			colour = BRUSH_COLOUR_NORMAL;

		m_aBrushShapes = {};

		// the 2D circle is always present
		m_aBrushShapes.Insert(CreateCircle(worldPos, vector.Up, m_fRadius, BRUSH_COLOUR_NORMAL_2D, m_fRadius * Math.PI2, ShapeFlags.NOZBUFFER));

		// horizontal line
		vector points[2] = {
			worldPos + { -m_fRadius, 0, 0 },
			worldPos + { m_fRadius, 0, 0 },
		};
		m_aBrushShapes.Insert(Shape.CreateLines(BRUSH_COLOUR_NORMAL_2D, ShapeFlags.NOZBUFFER, points, 2));

		// vertical line
		points = {
			worldPos + { 0, 0, -m_fRadius },
			worldPos + { 0, 0, m_fRadius },
		};
		m_aBrushShapes.Insert(Shape.CreateLines(BRUSH_COLOUR_NORMAL_2D, ShapeFlags.NOZBUFFER, points, 2));

		// 3D sphere / 2D cylynder
		if (m_bDetectBySphere)
			m_aBrushShapes.Insert(Shape.CreateSphere(colour, ShapeFlags.TRANSP | ShapeFlags.DOUBLESIDE, worldPos, m_fRadius));
		else
			m_aBrushShapes.Insert(Shape.CreateCylinder(colour, ShapeFlags.TRANSP | ShapeFlags.NOOUTLINE, worldPos, m_fRadius, BRUSH_GUIDE_2D_HEIGHT));

		// altitude indicator
		float yPos = 100;
		if (m_bSnapToObjects && m_API.TryGetTerrainSurfaceY(worldPos[0], worldPos[2], yPos) && worldPos[1] - yPos > BRUSH_ALTITUDE_TOLERANCE)
		{
			float floorRadius = m_fRadius * 0.5;
			vector floorPos = { worldPos[0], yPos, worldPos[2] };
			array<vector> pointsV = {
				worldPos,								// centre
				floorPos,
				worldPos + { 0, 0, m_fRadius },			// top
				floorPos + { 0, 0, floorRadius },
				worldPos + { m_fRadius, 0, 0 },			// right
				floorPos + { floorRadius, 0, 0 },
				worldPos + { 0, 0, -m_fRadius },		// bottom
				floorPos + { 0, 0, -floorRadius },
				worldPos + { -m_fRadius, 0, 0 },		// left
				floorPos + { -floorRadius, 0, 0 },
			};

			for (int i = 0, count = pointsV.Count(); i < count; i += 2) // step 2
			{
				points = {
					pointsV[i],
					pointsV[i + 1],
				};
				m_aBrushShapes.Insert(Shape.CreateLines(BRUSH_COLOUR_ALTITUDE, ShapeFlags.NOZBUFFER, points, 2));
			}

			m_aBrushShapes.Insert(CreateCircle(floorPos, vector.Up, floorRadius, BRUSH_COLOUR_ALTITUDE, floorRadius * Math.PI2, ShapeFlags.NOZBUFFER));
		}

		if (m_bIsInRemovalMode)
			SetBrushColor(BRUSH_COLOUR_REMOVAL);

		if (m_bSmiley)
		{
			Shape shape;
			for (int i = 1; i < 4; i++)
			{
				shape = m_aBrushShapes[i];
				shape.SetColor(0x00000000);
				shape.SetFlags(ShapeFlags.TRANSP | ShapeFlags.NOOUTLINE);
			}

			m_aBrushShapes.Insert(CreateCircle(worldPos + { -m_fRadius * 0.4, 0, m_fRadius * 0.25 }, vector.Up, m_fRadius * 0.2, BRUSH_COLOUR_NORMAL_2D, m_fRadius * 0.25 * Math.PI2, ShapeFlags.NOZBUFFER));
			m_aBrushShapes.Insert(CreateCircle(worldPos + { m_fRadius * 0.4, 0, m_fRadius * 0.25 }, vector.Up, m_fRadius * 0.2, BRUSH_COLOUR_NORMAL_2D, m_fRadius * 0.25 * Math.PI2, ShapeFlags.NOZBUFFER));
			m_aBrushShapes.Insert(CreateCircleArc(worldPos, vector.Up, vector.Forward, 135, 225, m_fRadius * 0.75, BRUSH_COLOUR_NORMAL_2D, m_fRadius * Math.PI2, ShapeFlags.NOZBUFFER));
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SetBrushColor(int colour)
	{
		if (!m_aBrushShapes)
			return; // should -not- happen...

		// colour &= 0x00FFFFFF; // remove alpha
		// colour |= 0x3F000000; // set alpha
		m_aBrushShapes[BRUSH_SHAPE_INDEX].SetColor(colour);
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsWorldValid()
	{
		if (!m_API) // required for Ctrl+Shift+R reloading Tools
			return false;

		string worldPath;
		m_API.GetWorldPath(worldPath);
		return !worldPath.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	protected void SCR_SelectionBrushTool()
	{
		m_TraceSphere = new TraceSphere();
		m_TraceSphere.Flags = TraceFlags.ENTS; // not interested in tracing world here
		m_TraceSphere .LayerMask = EPhysicsLayerPresets.Main;

		m_bIsWorldValid = IsWorldValid();
	}
}

[BaseContainerProps(configRoot: true)]
class SCR_SelectionBrushConfig
{
	[Attribute(uiwidget: UIWidgets.ResourcePickerThumbnail, desc: "Only the listed Prefabs will be selected", params: "et")]
	ref array<ResourceName> m_aPrefabs;
}

enum SCR_ESelectionBrushToolLayer
{
	ALL_LAYERS,
	CURRENT_LAYER,
	INACTIVE_LAYERS,
}
#endif // WORKBENCH
