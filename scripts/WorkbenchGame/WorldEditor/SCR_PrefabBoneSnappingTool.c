#ifdef WORKBENCH
[WorkbenchToolAttribute(name: "Prefab Bone-Snapping Tool", description: "Drag & drop Prefabs from the Resource Browser\ninto the viewport to snap them to the nearest bone", awesomeFontCode: 0xF4C4)]
class SCR_PrefabBoneSnappingTool : WorldEditorTool
{
	[Attribute(defvalue: "1", desc: "Debug shapes (bone locations and names) are drawn as debug shapes")]
	protected bool m_bDrawDebugShapes;

	[Attribute(defvalue: "0.1", desc: "Bone's debug sphere radius [m]", uiwidget: UIWidgets.Slider, params: "0.1 10 0.1")]
	protected float m_fBoneDebugSphereRadius;

	protected IEntitySource m_WorldEntitySource;
	protected IEntitySource m_EditedEntitySource;
	protected const ref map<string, ref array<vector>> BONE_MATRICES = new map<string, ref array<vector>>();
	protected const ref map<string, IEntitySource> BONE_ENTITYSOURCES = new map<string, IEntitySource>();
	protected const ref map<string, ref DebugTextScreenSpace> BONE_DEBUGTEXTS = new map<string, ref DebugTextScreenSpace>();
	protected const ref map<string, Shape> BONE_DEBUGSHAPES = new map<string, Shape>();
	// matrix[0] = vectorRight
	// matrix[1] = vectorUp
	// matrix[2] = vectorDir
	// matrix[3] = pos

	protected int m_iX = INVALID_CURSOR_POS;
	protected int m_iY = INVALID_CURSOR_POS;

	protected ref SCR_DebugShapeManager m_DebugShapeManager = new SCR_DebugShapeManager();

	protected static const int INVALID_CURSOR_POS = -1;

	protected static const int BONE_SPHERE_COLOUR = 0x8855FF00;
	protected static const int BONE_SPHERE_COLOUR_HIGHLIGHT = 0xFFFFFF00;

	protected static const float BONE_TEXT_SIZE = 12; // 2D size, so in pixels
	protected static const int BONE_TEXT_COLOUR = 0xFFFFFFFF;
	protected static const int BONE_TEXT_COLOUR_BACKGROUND = 0xFF000000;

	protected static const string SCENE_ROOT_BONE = "Scene_Root";
	protected static const int ROOT_BONE_SPHERE_COLOUR = 0x88FFAA55;
	protected static const float ROOT_BONE_SPHERE_RADIUS = 0.05;

	//------------------------------------------------------------------------------------------------
	//! \return the drag&dropped entitySource or null if not found (some bug somewhere)
	protected IEntitySource GetDragAndDroppedEntitySource()
	{
		IEntitySource entitySource;
		for (int i = m_API.GetEditorEntityCount() - 1; i >= 0; --i)
		{
			entitySource = m_API.GetEditorEntity(i);
			if (entitySource == m_EditedEntitySource)
				continue;

			if (entitySource == m_WorldEntitySource)
				continue;

			return entitySource; // always pick the last one
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected void RefreshState(bool showError)
	{
		RefreshEntitySources(showError);
		if (!m_EditedEntitySource)
			return;

		RefreshBoneMatrixMap();
		RefreshDebugShapes();
		RefreshBoneEntitySourceMap();
	}

	//------------------------------------------------------------------------------------------------
	//! \return true if world and Prefab entity sources have been found, false otherwise
	protected bool RefreshEntitySources(bool showError)
	{
		int entityCount = m_API.GetEditorEntityCount();
		if (entityCount < 2)
		{
			if (showError)
			{
				string text = "Not enough or too many Entities at world's root in Prefab Edit mode; fix that then click Refresh";
				Workbench.Dialog("", text);
				Print(text, LogLevel.WARNING);
				Debug.DumpStack(text);
				Print(text, LogLevel.WARNING);
			}

			return false;
		}

		// let's see how strong this is
		m_WorldEntitySource = m_API.GetEditorEntity(0);
		m_EditedEntitySource = m_API.GetEditorEntity(1);
		if (!m_EditedEntitySource || !m_EditedEntitySource.GetAncestor())
		{
			Print("Edited Entity Source is not a Prefab", LogLevel.ERROR);
			m_EditedEntitySource = null;
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Fill BONE_MATRICES according to model's bones
	//! Also draws bone debug if the option is set
	//! Requires m_EditedEntitySource
	protected void RefreshBoneMatrixMap()
	{
		IEntity entity = m_API.SourceToEntity(m_EditedEntitySource);
		if (!entity)
		{
			Print(m_EditedEntitySource, LogLevel.WARNING);
			Print(entity, LogLevel.WARNING);
			return;
		}

		BONE_MATRICES.Clear();

		Animation animation = entity.GetAnimation();
		if (!animation)
			return;

		array<string> boneNames = {};
		animation.GetBoneNames(boneNames);
		vector mat[4];
		foreach (string boneName : boneNames)
		{
			TNodeId boneIndex = animation.GetBoneIndex(boneName);

			if (!animation.GetBoneMatrix(boneIndex, mat))
			{
				Print("cannot get bone " + boneName + "'s matrix", LogLevel.WARNING);
				continue;
			}

			BONE_MATRICES.Insert(boneName, { mat[0], mat[1], mat[2], mat[3] });
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void RefreshDebugShapes()
	{
		m_DebugShapeManager.Clear();
		BONE_DEBUGTEXTS.Clear();
		BONE_DEBUGSHAPES.Clear();
		if (!m_bDrawDebugShapes)
			return;

		BaseWorld world = m_API.GetWorld();
//		DebugTextWorldSpace text;
		DebugTextScreenSpace debugText;
		Shape debugShape;
		foreach (string boneName, array<vector> matrix : BONE_MATRICES)
		{
//			text = m_DebugShapeManager.AddText(boneName, matrix[3] + vector.Up * (m_fBoneDebugSphereRadius + BONE_TEXT_SIZE), BONE_TEXT_SIZE);
//			text.SetText(string.Empty);
//			BONE_DEBUGTEXTS.Insert(boneName, text);

			debugText = DebugTextScreenSpace.Create(world, boneName, DebugTextFlags.FACE_CAMERA, 1, 1, BONE_TEXT_SIZE, BONE_TEXT_COLOUR, BONE_TEXT_COLOUR_BACKGROUND);
			BONE_DEBUGTEXTS.Insert(boneName, debugText);

			if (boneName == SCENE_ROOT_BONE)
				debugShape = m_DebugShapeManager.AddSphere(matrix[3], m_fBoneDebugSphereRadius * 0.5, ROOT_BONE_SPHERE_COLOUR, ShapeFlags.NOOUTLINE);
			else
				debugShape = m_DebugShapeManager.AddSphere(matrix[3], m_fBoneDebugSphereRadius, BONE_SPHERE_COLOUR, ShapeFlags.NOOUTLINE);

			BONE_DEBUGSHAPES.Insert(boneName, debugShape);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Fill BONE_MATRICES according to model's bones
	//! Requires m_EditedEntitySource
	protected void RefreshBoneEntitySourceMap()
	{
		BONE_ENTITYSOURCES.Clear();
		array<IEntitySource> children = {};
		IEntitySource child;
		vector childRelPos;
		for (int i, count = m_EditedEntitySource.GetNumChildren(); i < count; ++i)
		{
			child = m_EditedEntitySource.GetChild(i);
			if (!child.Get("coords", childRelPos))
				continue;

			foreach (string boneName, array<vector> matrix : BONE_MATRICES)
			{
				if (childRelPos == matrix[3]) // pos only
				{
					BONE_ENTITYSOURCES.Insert(boneName, child);
					break;
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] newEntitySource
	protected void SnapNewEntitySource(notnull IEntitySource newEntitySource)
	{
		IEntitySource parent = newEntitySource.GetParent();
		if (parent)
			return;

		vector newEntitySourcePos;
		if (!newEntitySource.Get("coords", newEntitySourcePos))
			return;

		int boneCount = BONE_MATRICES.Count();
		if (boneCount < 2) // set as child and leave
		{
			if (boneCount > 0 && BONE_MATRICES.GetKey(0) == SCENE_ROOT_BONE)
				--boneCount;

			if (boneCount < 1)
			{
				m_API.MoveEntitiesToPrefab(m_EditedEntitySource, m_EditedEntitySource.GetAncestor(), { newEntitySource });
				return;
			}
		}

		ArmaReforgerScripted game = GetGame();
		if (!game)
			return;

		WorkspaceWidget workspace = game.GetWorkspace();
		if (!workspace)
			return;

		BaseWorld world = m_API.GetWorld();
		vector entityScreenPos = workspace.ProjWorldToScreen(newEntitySourcePos, world);
		entityScreenPos = { entityScreenPos[0], 0, entityScreenPos[1] };

		string selectedBoneName;
		array<vector> selectedMatrix;
		float resultDistanceSq = float.INFINITY;
		foreach (string boneName, array<vector> matrix : BONE_MATRICES)
		{
			if (boneName == SCENE_ROOT_BONE) // do not snap to scene root bone
				continue;

			vector boneScreenPos = workspace.ProjWorldToScreen(matrix[3], world);
			boneScreenPos = { boneScreenPos[0], 0, boneScreenPos[1] };

			float distanceSq = vector.DistanceSqXZ(entityScreenPos, boneScreenPos);
			if (distanceSq < resultDistanceSq)
			{
				selectedBoneName = boneName;
				selectedMatrix = matrix;
				resultDistanceSq = distanceSq;
			}
		}

		if (!selectedMatrix)
			return;

		if (BONE_ENTITYSOURCES.Get(selectedBoneName))
		{
			PrintFormat("Bone %1 is already occupied by an entity - move or remove it first", level: LogLevel.NORMAL);
			m_API.SetEntitySelection(BONE_ENTITYSOURCES.Get(selectedBoneName));
			return;
		}

		vector mat[3] = {
			selectedMatrix[0],
			selectedMatrix[1],
			selectedMatrix[2],
		};

		vector angles = Math3D.MatrixToAngles(mat);

		m_API.BeginEntityAction();
		m_API.SetVariableValue(newEntitySource, null, "coords", selectedMatrix[3].ToString(false));
		m_API.SetVariableValue(newEntitySource, null, "angles", string.Format("%1 %2 %3", angles[1], angles[0], angles[2]));
		m_API.MoveEntitiesToPrefab(m_EditedEntitySource, m_EditedEntitySource.GetAncestor(), { newEntitySource });
		m_API.EndEntityAction();

		BONE_ENTITYSOURCES.Set(selectedBoneName, newEntitySource);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateDebugShapes(float x, float y)
	{
		// refresh debug texts to only display those close to the cursor
		if (BONE_DEBUGTEXTS.IsEmpty())
			return;

		BaseWorld world = m_API.GetWorld();
		WorkspaceWidget workspace = GetGame().GetWorkspace();

		vector traceStart, traceEnd, traceDir;
		if (!m_API.TraceWorldPos(x, y, TraceFlags.WORLD, traceStart, traceEnd, traceDir))
			traceEnd = traceStart + traceDir * 50;

//		vector cursorPos2D = workspace.ProjWorldToScreen(traceEnd, world);
//		vector debugTextPos2D = workspace.ProjWorldToScreen(matrix[3], world);
//		float distance = vector.Distance(cursorPos, debugTextPos);
//		if (vector.Distance(cursorPos2D, debugTextPos2D) < 100)

		// DebugTextWorldSpace debugText;
		DebugTextScreenSpace debugText;
		foreach (string boneName, array<vector> matrix : BONE_MATRICES)
		{
			debugText = BONE_DEBUGTEXTS.Get(boneName);
			if (!debugText)
				break;

			if (Math3D.PointLineSegmentDistanceSqr(matrix[3], traceStart, traceEnd) <= m_fBoneDebugSphereRadius * m_fBoneDebugSphereRadius)
			{
				debugText.SetText(boneName);
				BONE_DEBUGSHAPES.Get(boneName).SetColor(BONE_SPHERE_COLOUR_HIGHLIGHT);
			}
			else
			{
				debugText.SetText(string.Empty);
				if (boneName == SCENE_ROOT_BONE)
					BONE_DEBUGSHAPES.Get(boneName).SetColor(ROOT_BONE_SPHERE_COLOUR);
				else
					BONE_DEBUGSHAPES.Get(boneName).SetColor(BONE_SPHERE_COLOUR);
			}

		}
	}

	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	// World Editor Tool events below
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	protected override void OnKeyPressEvent(KeyCode key, bool isAutoRepeat)
	{
		// in all modes
		if (key == KeyCode.KC_ESCAPE)
		{
			m_API.ClearEntitySelection();
			return;
		}
	}

//	protected override void OnKeyReleaseEvent(KeyCode key, bool isAutoRepeat);
//	protected override void OnEnterEvent(); // drag & drop triggers that, useless

	//------------------------------------------------------------------------------------------------
	protected override void OnLeaveEvent()
	{
		if (!m_EditedEntitySource)
			return;

		m_iX = INVALID_CURSOR_POS;
		m_iY = INVALID_CURSOR_POS;
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnMouseMoveEvent(float x, float y)
	{
		if (!m_EditedEntitySource)
			return;

		if (m_bDrawDebugShapes)
			UpdateDebugShapes(x, y);

		if (m_iX != INVALID_CURSOR_POS && m_iY != INVALID_CURSOR_POS)
			return;

		m_iX = x;
		m_iY = y;

		IEntitySource newEntitySource = GetDragAndDroppedEntitySource();
		if (!newEntitySource)
			return;

		Print("added " + newEntitySource.GetAncestor().GetResourceName(), LogLevel.DEBUG);
		SnapNewEntitySource(newEntitySource);
	}

	//------------------------------------------------------------------------------------------------
//	protected override void OnMouseDoubleClickEvent(float x, float y, WETMouseButtonFlag buttons);

	//------------------------------------------------------------------------------------------------
	protected override void OnMousePressEvent(float x, float y, WETMouseButtonFlag buttons)
	{
		if (m_API.IsPrefabEditMode())
			return; // normal behaviour please

		// world mode

		WorldEditor worldEditor = SCR_WorldEditorToolHelper.GetWorldEditor();
		if (!worldEditor)
			return;

		// deselect everything and select the entity under the cursor
		m_API.ClearEntitySelection();

		IEntity entity;
		vector traceStart, traceEnd, traceDir;
		if (!m_API.TraceWorldPos(x, y, TraceFlags.ENTS, traceStart, traceEnd, traceDir, entity))
			return;

		if (!entity)
			return;

		IEntitySource entitySource = m_API.EntityToSource(entity);
		if (!entitySource)
			return;

		m_API.SetEntitySelection(entitySource);
	}

//	protected override void OnMouseReleaseEvent(float x, float y, WETMouseButtonFlag buttons); // this event is not triggered on Prefab drag & drop
//	protected override void OnWheelEvent(int delta);

	//------------------------------------------------------------------------------------------------
	protected override void OnActivate()
	{
		WorldEditor worldEditor = SCR_WorldEditorToolHelper.GetWorldEditor();
		if (!worldEditor || !worldEditor.IsPrefabEditMode())
			return;

		RefreshState(false);
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnDeActivate()
	{
		m_DebugShapeManager.Clear();
	}

	//------------------------------------------------------------------------------------------------
	// entities are not yet loaded
	protected override void OnAfterLoadWorld()
	{
		RefreshState(false);
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnBeforeUnloadWorld()
	{
		m_EditedEntitySource = null;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Refresh")]
	protected void ButtonRefresh()
	{
		WorldEditor worldEditor = SCR_WorldEditorToolHelper.GetWorldEditor();
		if (!worldEditor)
			return;

		if (!worldEditor.IsPrefabEditMode())
			return;

		RefreshState(true);
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Open world-selected Prefab in Edit Mode")]
	protected void ButtonOpenPrefabInEditMode()
	{
		WorldEditor worldEditor = SCR_WorldEditorToolHelper.GetWorldEditor();
		if (!worldEditor)
			return;

		if (worldEditor.IsPrefabEditMode())
			return;

		WorldEditorAPI worldEditorAPI = worldEditor.GetApi();
		if (!worldEditorAPI)
			return;

		if (worldEditorAPI.GetSelectedEntitiesCount() != 1)
			return;

		IEntitySource selectedEntitySource = worldEditorAPI.GetSelectedEntity(0);
		if (!selectedEntitySource)
			return;

		while (selectedEntitySource.GetParent())
		{
			selectedEntitySource = selectedEntitySource.GetParent();
		}

		m_API.SetEntitySelection(selectedEntitySource);

		IEntitySource ancestorSource = selectedEntitySource.GetAncestor();
		if (!ancestorSource)
		{
			Print("Selected entity is not a Prefab", LogLevel.NORMAL);
			return;
		}

		ResourceName resourceName = ancestorSource.GetResourceName();
		if (!resourceName) // .IsEmpty()
			return;

		if (!Workbench.ScriptDialog("", "This will open the selected Prefab in Prefab Edit mode. Continue?\nSelected Prefab: " + resourceName, new SCR_OKCancelWorkbenchDialog()))
			return;

		if (!SCR_WorldEditorToolHelper.GetWorldEditor().SetOpenedResource(resourceName))
		{
			Print("Failed to open resource", LogLevel.WARNING);
			return;
		}

		RefreshState(false);
	}
}
#endif // WORKBENCH
