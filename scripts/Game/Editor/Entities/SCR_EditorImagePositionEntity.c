[EntityEditorProps(category: "GameScripted/Editor", description: "Image generator for pecific subset of entities", color: "255 0 0 255")]
class SCR_EditorImagePositionEntityClass : GenericEntityClass
{
}

/** @ingroup Editor_Entities
*/

/*!

*/
class SCR_EditorImagePositionEntity : GenericEntity
{
	protected const float ANIM_TIME_STEP = 30.0;
	protected const string ANIM_ARM_IK = "ArmIK";
	// Default: exact AABB fit (margin 1.0). Safe for tight-BB objects (vehicles, bunkers,
	// characters) with no clipping risk. Loose-BB compositions (where AABB corners are
	// empty) will have some visible empty space — unavoidable without per-prefab tuning.
	protected const float FIT_VIEW_MARGIN = 1.0;
	// Small-object padding: below this BB diagonal, add padding so tiny items don't fill frame edge-to-edge
	protected const float FIT_VIEW_SMALL_THRESHOLD = 2.0; // BB diagonal below this gets padding
	protected const float FIT_VIEW_SMALL_PADDING = 0.80;  // Margin scale at 0m diagonal (0.8 = 20% padding)
	// Debug: log mesh population and drops in FilterAndUnionMeshes. Turn off when satisfied.
	protected const bool DEBUG_OUTLIER_FILTER = true;

	[Attribute("0", uiwidget: UIWidgets.SearchComboBox, "Labels used for matching group prefabs to this position.", "", ParamEnumArray.FromEnum(EEditableEntityLabel), category: "Configuration")]
	protected ref array<EEditableEntityLabel> m_Labels;

	[Attribute("2", uiwidget:UIWidgets.Slider, "Delay between creating the entity and taking a screenshot.", params: "0.1 5 0.5", category: "Configuration")]
	protected float m_fDelay;

	[Attribute("0", desc: "Order in which the position will be evaluated.\nHigher numbers are processed first.", category: "Configuration")]
	protected int m_iPriority;

	[Attribute(uiwidget:UIWidgets.ResourcePickerThumbnail, params: "xob", category: "Configuration")]
	protected ResourceName m_PreviewMesh;

	[Attribute(desc: "", category: "Configuration")]
	protected bool m_bEnablePhysics;

	[Attribute(desc: "Position the entity by center of its bounding box, not by center of the prefab.", category: "Configuration")]
	protected bool m_bUseBoundingCenter;

	[Attribute(desc: "When enabled, the camera will automatically move backward to fit the prefab in view.", category: "Configuration")]
	protected bool m_bMustFitInView;

	[Attribute("0", uiwidget:UIWidgets.Slider, "Geometric step base (meters) for snapping camera distance. When > 0, distances snap to a geometric series (ratio √2) for consistent framing across similar-sized assets — but introduces up to ±20%% framing variance.\n0 = exact fit (recommended for most cases).", params: "0 10 0.5", category: "Configuration")]
	protected float m_fFitViewStep;

	[Attribute("0", uiwidget:UIWidgets.Slider, "Distance offset (meters) applied after auto-fit.\nNegative = move camera closer, positive = move further away.", params: "-40 40 0.5", category: "Configuration")]
	protected float m_fFitViewOffset;

	[Attribute("0", uiwidget:UIWidgets.Slider, "Vertical offset (meters) applied after auto-fit.\nPositive = move camera up, negative = move down.", params: "-10 10 0.1", category: "Configuration")]
	protected float m_fFitViewVerticalOffset;

	[Attribute("1", desc: "When enabled, DoF material parameters (FocusDistance, FocalLength) are automatically scaled based on the adjusted camera distance.\nDisable to use the emat's authored values as-is.", category: "Configuration")]
	protected bool m_bAdjustDoF;

	[Attribute("0", uiwidget:UIWidgets.Slider, "Max aspect ratio for auto-fit bounds. Any dimension larger than ratio × max(other dimensions) is clamped, centered on midpoint.\nLegacy override — per-mesh outlier rejection now handles thin outliers automatically. Set > 0 only as escape hatch.\n0 = disabled.", params: "0 10 0.1", category: "Configuration")]
	protected float m_fFitViewMaxAspect;

	[Attribute("0", uiwidget:UIWidgets.Slider, "Max bounds extent (meters) in any dimension for auto-fit. Any dimension larger than this is clamped, centered on midpoint.\nUse for wide/spread-out compositions where the camera backs up too far.\n0 = disabled.", params: "0 100 0.5", category: "Configuration")]
	protected float m_fFitViewMaxExtent;

	[Attribute("1", desc: "When enabled, bounds below the spawn point Y are clipped.\nPrevents underground foundations/bases from inflating bounds into hidden space.", category: "Configuration")]
	protected bool m_bIgnoreUnderground;

	// hidden (unused)

	[Attribute(uiwidget: UIWidgets.None, params: "agr", category: "Animation")]
	protected ResourceName m_sPosesGraph;

	[Attribute(uiwidget: UIWidgets.None, params: "asi", category: "Animation")]
	protected ResourceName m_sPosesInstance;

	[Attribute(uiwidget: UIWidgets.None, category: "Animation")]
	protected string m_sStartNode;

	[Attribute(uiwidget: UIWidgets.None, category: "Animation")]
	protected string m_sPoseVar;

	[Attribute(uiwidget: UIWidgets.None, category: "Animation")]
	protected int m_iPoseID;

	[Attribute(uiwidget: UIWidgets.None, category: "Animation", params: "1 7 1")]
	protected int m_iArmIK;

	[Attribute(uiwidget:UIWidgets.None, params: "anm", category: "Animation")]
	protected ResourceName m_sArmIKResource;

	[Attribute("-1", uiwidget: UIWidgets.None, category: "Animation", enums: SCR_Enum.GetList(EWeaponType, ParamEnum("<Unchanged>", "-1")))]
	protected EWeaponType m_eForceWeaponType;

	// /hidden

	[Attribute("-4", uiwidget:UIWidgets.Slider, params: "-90 90 1", category: "Environment")]
	protected float m_fLatitude;

	[Attribute("71", uiwidget:UIWidgets.Slider, params: "-180 180 1", category: "Environment")]
	protected float m_fLongitude;

	[Attribute("0.415", uiwidget:UIWidgets.Slider, params: "0 1 0.01", category: "Environment")]
	protected float m_fTime;

	[Attribute("1985", uiwidget: UIWidgets.Slider, params: "1899 2050 1", category: "Environment")]
	protected int m_iYear;

	[Attribute("5", uiwidget: UIWidgets.Slider, params: "1 12 1", category: "Environment")]
	protected int m_iMonth;

	[Attribute("8", uiwidget: UIWidgets.Slider, params: "1 31 1", category: "Environment")]
	protected int m_iDay;

	[Attribute(defvalue: "Clear", uiwidget: UIWidgets.ComboBox, category: "Environment", enums: { ParamEnum("Clear", "Clear"), ParamEnum("Cloudy", "Cloudy"), ParamEnum("Overcast", "Overcast"), ParamEnum("Rainy", "Rainy") }, desc: "Area shape")]
	private string m_sWeatherState;

	[Attribute(desc: "Default pose that should be used if no faction specific pose is defined", category: "Animation - Character")]
	protected ref SCR_EditorImagePositionCharacterPose m_DefaultPose;

	[Attribute(desc: "List of pose for each faction. Use default pose if selected character faction pose is not defined", category: "Animation - Character")]
	protected ref array<ref SCR_EditorImagePositionCharacterPose> m_aFactionPoses;

#ifdef WORKBENCH
	protected ref array<CameraBase> m_aCameras = {};
	protected ref array<string> m_aCameraChildNames = {};
	protected SCR_EditorImagePositionEntity m_Parent;
	protected ref SCR_SortedArray<SCR_EditorImagePositionEntity> m_aSubPositions = new SCR_SortedArray<SCR_EditorImagePositionEntity>();
	protected IEntity m_Entity;
	protected ref array<IEntity> m_aCurrentNearbyEntities = {};
	protected ref array<IEntity> m_aOriginalNearbyEntities = {};
	protected CameraBase m_AdjustedCamera;
	protected vector m_vOriginalCameraPos;
	protected vector m_vBoundsOrigin;
	protected string m_sNewWeaponMesh;
	protected string m_sCurrentWeaponMesh;

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetDelay()
	{
		return m_fDelay;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetPriority()
	{
		return m_iPriority;
	}

	//------------------------------------------------------------------------------------------------
	//! \return True if this position has any labels (used for group matching).
	bool HasLabels()
	{
		return !m_Labels.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	//! Check if all of this position's labels are present in the given label set.
	//! Used for matching group prefabs to suitable positions.
	bool IsSuitable(array<EEditableEntityLabel> labels)
	{
		foreach (EEditableEntityLabel label : m_Labels)
		{
			if (!labels.Contains(label))
				return false;
		}
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Find a sub-position suitable for a group member with the given labels.
	SCR_EditorImagePositionEntity FindSuitableSubPosition(SCR_SortedArray<SCR_EditorImagePositionEntity> subPositions, array<EEditableEntityLabel> labels)
	{
		for (int i = subPositions.Count() - 1; i >= 0; i--)
		{
			if (subPositions.GetValue(i).IsSuitable(labels))
				return subPositions.GetValue(i);
		}
		return null;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] prefab
	//! \return
	bool ActivatePosition(ResourceName prefab)
	{
		//--- Resolve cameras by name (discovered from entity source in constructor)
		if (m_aCameras.IsEmpty())
		{
			foreach (string cameraName : m_aCameraChildNames)
			{
				CameraBase camera = CameraBase.Cast(GetWorld().FindEntityByName(cameraName));
				if (camera && !m_aCameras.Contains(camera))
					m_aCameras.Insert(camera);
			}

			if (m_aCameras.IsEmpty())
			{
				Debug.Error2(Type().ToString(), string.Format("No camera entity found for position '%1'! (searched %2 name(s))", GetPositionName(), m_aCameraChildNames.Count()));
				return false;
			}
		}

		//--- Prevent AI groups from creating members themselves, do it manually here
		SCR_AIGroup.IgnoreSpawning(true);

		//--- Create prefab
		EntitySpawnParams spawnParams = new EntitySpawnParams();
		GetTransform(spawnParams.Transform);
		m_Entity = GetGame().SpawnEntityPrefab(Resource.Load(prefab), GetWorld(), spawnParams);
		if (!m_Entity)
		{
			Debug.Error2(Type().ToString(), string.Format("Error when creating prefab '%1'!", prefab));
			return false;
		}

		//--- Orient composition to terrain
		SCR_SlotCompositionComponent composition = SCR_SlotCompositionComponent.Cast(m_Entity.FindComponent(SCR_SlotCompositionComponent));
		if (composition)
		{
			SCR_EditableEntityComponent editableComposition = SCR_EditableEntityComponent.GetEditableEntity(m_Entity);
			editableComposition.SetTransformWithChildren(spawnParams.Transform);
			//composition.OrientToTerrain();
		}

		//--- Set environment
		ChimeraWorld world = GetWorld();
		TimeAndWeatherManagerEntity envManager = world.GetTimeAndWeatherManager();
		if (envManager)
		{
			int h, m, s;
			envManager.SetCurrentLatitude(m_fLatitude);
			envManager.SetCurrentLongitude(m_fLongitude);
			envManager.TimeToHoursMinutesSeconds(m_fTime * 24, h, m, s);
			envManager.SetHoursMinutesSeconds(h, m, s, false);
			envManager.SetDate(m_iYear, m_iMonth, m_iDay, true);

			//Set weather
			BaseWeatherStateTransitionManager weatherTransitionManager = envManager.GetTransitionManager();
			if (weatherTransitionManager)
			{
				WeatherStateTransitionNode transitionNode = weatherTransitionManager.CreateStateTransition(m_sWeatherState, 0, 1);
				transitionNode.SetLooping(true);

				weatherTransitionManager.EnqueueStateTransition(transitionNode, false);
				weatherTransitionManager.RequestStateTransitionImmediately(transitionNode);
			}

			//int hX, mX, sX;
			//envManager.GetHoursMinutesSeconds(hX, mX, sX);
			//PrintFormat("%1 == %2, %3 == %4, %5 == %6", h, hX, m, mX, s, sX);
		}

		//--- Move group members to sub-positions
		SCR_AIGroup group = SCR_AIGroup.Cast(m_Entity);
		if (group)
		{
			/*
			bool success = ActivateGroupPosition(group, spawnParams);
			if (!success)
				return false;
			*/

			group.SetMemberSpawnDelay(0);
			group.SpawnUnits();

			GetGame().GetCallqueue().CallLater(ActivateGroupPosition, 1000, false, group, spawnParams);
		}
		else
		{
			SCR_EditableCharacterComponent character = SCR_EditableCharacterComponent.Cast(m_Entity.FindComponent(SCR_EditableCharacterComponent));
			if (character)
				ActivateCharacterPosition(prefab);

			//--- Position the entity by its bounding center, not mesh origin
			if (m_bUseBoundingCenter)
			{
				vector min, max;
				SCR_Global.GetWorldBoundsWithChildren(m_Entity, min, max);
				vector bCenter = min + (max - min) * 0.5;
				m_Entity.SetOrigin(2 * m_Entity.GetOrigin() - bCenter);
				m_Entity.Update();
			}
		}

		//--- Handle physics: explicitly disable to prevent settling/movement
		//--- unless m_bEnablePhysics is set (e.g., for entities that need to drop)
		Physics phys = m_Entity.GetPhysics();
		if (phys)
			phys.SetActive(m_bEnablePhysics);

		//--- Activate camera
		CameraBase camera = m_aCameras[0];

		vector bbCenter;
		if (m_bMustFitInView && camera)
		{
			vector min, max;

			//--- Characters: use only root entity bounds (body mesh), skip children
			//--- to avoid outliers like radio antennas inflating the bounds.
			//--- Compositions with linked children: gather bounds from linked entities.
			//--- Everything else: default depth walk.
			SCR_EditableCharacterComponent characterComponent = SCR_EditableCharacterComponent.Cast(m_Entity.FindComponent(SCR_EditableCharacterComponent));
			if (characterComponent)
			{
				// Try depth 0 first (non-cloned character has body mesh at root)
				GetVisualBounds(m_Entity, min, max, 0);
				// Fallback: cloned preview entities may wrap the body deeper in the hierarchy
				if (min[0] >= max[0])
					GetVisualBounds(m_Entity, min, max, 2);
			}
			else
			{
				SCR_EditorLinkComponent linkComponent = SCR_EditorLinkComponent.Cast(m_Entity.FindComponent(SCR_EditorLinkComponent));
				if (linkComponent && linkComponent.IsSpawned())
				{
					// Collect meshes from the parent and every linked child, then filter+union once
					// so the outlier test sees the whole composition as one population.
					m_vBoundsOrigin = m_Entity.GetOrigin();
					array<vector> mins = {};
					array<vector> maxs = {};
					array<string> names = {};
					CollectMeshBoundsRecursive(m_Entity, mins, maxs, names, 1);
					array<IEntity> linkedChildren = linkComponent.GetLinkedChildren();
					if (linkedChildren)
					{
						foreach (IEntity linkedChild : linkedChildren)
						{
							if (linkedChild)
								CollectMeshBoundsRecursive(linkedChild, mins, maxs, names, 1);
						}
					}
					FilterAndUnionMeshes(mins, maxs, names, min, max);
				}
				else
				{
					GetVisualBounds(m_Entity, min, max);
				}
			}

			// Validate bounds — no meshes found means min/max stayed at initialization values
			if (min[0] >= max[0])
			{
				PrintFormat("FitView: no valid meshes found for '%1', using default camera", prefab, level: LogLevel.WARNING);
			}
			else
			{
				// Clamp bounds aspect ratio to prevent thin outliers (antennas, flag poles)
				// from forcing the camera too far back.
				if (m_fFitViewMaxAspect > 0)
					ClampBoundsAspectRatio(min, max, m_fFitViewMaxAspect);

				// Clip bounds to a max_extent cube centered on the POSITION ENTITY's origin.
				// This anchors framing to where the user placed the position entity (the visual
				// center of the composition), rather than to the BB's geometric midpoint which
				// can land in empty space with asymmetric mesh distribution.
				if (m_fFitViewMaxExtent > 0)
				{
					vector center = GetOrigin();
					float half = m_fFitViewMaxExtent * 0.5;
					for (int i = 0; i < 3; i++)
					{
						float lo = center[i] - half;
						float hi = center[i] + half;
						if (min[i] < lo)
							min[i] = lo;
						if (max[i] > hi)
							max[i] = hi;
					}
				}

				// Clip bounds below spawn plane — foundations/bases are hidden under fake ground
				if (m_bIgnoreUnderground)
				{
					float spawnY = GetOrigin()[1];
					if (min[1] < spawnY)
						min[1] = spawnY;
				}

				bbCenter = (min + max) * 0.5;
				FitCameraToEntity(camera, min, max);
			}
		}

		if (camera)
		{
			CameraManager cameraManager = GetGame().GetCameraManager();
			if (cameraManager)
				cameraManager.SetCamera(camera);

			//--- Adjust DoF parameters to match the new camera distance
			if (m_bMustFitInView && m_bAdjustDoF)
				AdjustDepthOfField(camera, bbCenter);
		}
		else
		{
			Debug.Error2(Type().ToString(), string.Format("No camera found on position '%1'!", GetPositionName()));
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//!
	void DeactivatePosition()
	{
		RestoreCamera();

		UpdateNearbyEntities();
		foreach (IEntity entity : m_aCurrentNearbyEntities)
		{
			if (!m_aOriginalNearbyEntities.Contains(entity))
				delete entity;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Automatically position camera so the entity fits the view.
	//! - Aims at the BB center (or position origin if user used m_fFitViewMaxExtent to clip)
	//! - Margin scales with BB size: small objects use tight fit, larger compositions
	//!   use overshoot to clip empty AABB corners
	//! - No step snapping by default — every entity frames consistently
	protected void FitCameraToEntity(notnull CameraBase cam, vector bbMin, vector bbMax)
	{
		float halfFovV = cam.GetVerticalFOV() * 0.5;
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		float aspectRatio = workspace.GetWidth() / (float)workspace.GetHeight();

		// Exact AABB fit by default. Very small objects get padding so they don't
		// fill frame edge-to-edge (looks cramped).
		float margin = FIT_VIEW_MARGIN;
		float bbDiagonal = vector.Distance(bbMin, bbMax);
		if (bbDiagonal < FIT_VIEW_SMALL_THRESHOLD)
		{
			float sizeScale = FIT_VIEW_SMALL_PADDING + (1.0 - FIT_VIEW_SMALL_PADDING) * (bbDiagonal / FIT_VIEW_SMALL_THRESHOLD);
			margin *= sizeScale;
		}

		float tanV = Math.Tan(halfFovV * Math.DEG2RAD) * margin;
		float tanH = tanV * aspectRatio;

		// Save original position for restoration
		vector camTransform[4];
		cam.GetTransform(camTransform);
		m_vOriginalCameraPos = camTransform[3];
		m_AdjustedCamera = cam;

		// Step 1: Aim camera at the BB center. Shift laterally so the aim point projects
		// to the frame center. For wide/asymmetric compositions, use m_fFitViewMaxExtent
		// to clip the BB around the position origin (which shifts the aim toward origin).
		vector aimPoint = (bbMin + bbMax) * 0.5;
		vector localAim = cam.CoordToLocal(aimPoint);
		camTransform[3] = camTransform[3] + camTransform[0] * localAim[0] + camTransform[1] * localAim[1];
		cam.SetTransform(camTransform);

		// Step 2: Build 8 corners of the bounding box
		array<vector> corners = {
			bbMin,
			Vector(bbMin[0], bbMin[1], bbMax[2]),
			Vector(bbMin[0], bbMax[1], bbMin[2]),
			Vector(bbMax[0], bbMin[1], bbMin[2]),
			Vector(bbMax[0], bbMax[1], bbMin[2]),
			Vector(bbMax[0], bbMin[1], bbMax[2]),
			Vector(bbMin[0], bbMax[1], bbMax[2]),
			bbMax
		};

		// Step 3: Calculate exact forward/backward offset to fit all BB corners.
		// Positive = move backward (entity too large), negative = move forward (entity too small).
		float maxOffset;
		for (int i = 0; i < 8; i++)
		{
			vector localPos = cam.CoordToLocal(corners[i]);
			float absX = Math.AbsFloat(localPos[0]);
			float absY = Math.AbsFloat(localPos[1]);
			float z = localPos[2];

			float requiredZ = Math.Max(absX / tanH, absY / tanV);
			float offset = requiredZ - z;
			if (i == 0 || offset > maxOffset)
				maxOffset = offset;
		}

		// Optional geometric step snapping (disabled by default).
		// Only use if you need discrete distance steps across similar-sized assets —
		// this sacrifices exact fit for stepping consistency.
		if (m_fFitViewStep > 0)
		{
			cam.GetTransform(camTransform);
			float currentDist = cam.CoordToLocal(aimPoint)[2];
			float idealDist = currentDist + maxOffset;
			if (idealDist > 0)
			{
				const float ratio = 1.41421; // sqrt(2)
				float n = Math.Log2(idealDist / m_fFitViewStep) / Math.Log2(ratio);
				float snappedDist = m_fFitViewStep * Math.Pow(ratio, Math.Round(n));
				maxOffset = snappedDist - currentDist;
			}
		}

		// Apply user offset (negative = closer, positive = further)
		maxOffset = maxOffset + m_fFitViewOffset;

		// Clamp: don't let the camera move forward past the aim point.
		cam.GetTransform(camTransform);
		float maxForward = vector.Dot(aimPoint - camTransform[3], camTransform[2]);
		if (maxOffset < -maxForward)
			maxOffset = -maxForward;

		// Step 4: Move camera along its forward direction + apply vertical offset
		if (maxOffset != 0)
			camTransform[3] = camTransform[3] - camTransform[2] * maxOffset;
		if (m_fFitViewVerticalOffset != 0)
			camTransform[3][1] = camTransform[3][1] + m_fFitViewVerticalOffset;
		cam.SetTransform(camTransform);
	}

	//------------------------------------------------------------------------------------------------
	//! Clamp any bounds dimension that exceeds ratio × max(other dimensions).
	//! Clamping is centered on the midpoint so the visual core stays framed.
	protected void ClampBoundsAspectRatio(inout vector min, inout vector max, float ratio)
	{
		vector size = max - min;
		vector center = (min + max) * 0.5;

		for (int i = 0; i < 3; i++)
		{
			float otherMax;
			for (int j = 0; j < 3; j++)
			{
				if (j != i && size[j] > otherMax)
					otherMax = size[j];
			}

			float limit = otherMax * ratio;
			if (size[i] > limit)
			{
				float half = limit * 0.5;
				min[i] = center[i] - half;
				max[i] = center[i] + half;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Get world bounds of entity hierarchy, including only entities with renderable meshes.
	//! Collects each mesh's bounds individually, runs outlier filter, then unions.
	//! Limits recursion depth to avoid inflated bounds from deep sub-children (physics, slots).
	protected void GetVisualBounds(IEntity entity, out vector min, out vector max, int maxDepth = 2)
	{
		m_vBoundsOrigin = entity.GetOrigin();
		array<vector> mins = {};
		array<vector> maxs = {};
		array<string> names = {};
		CollectMeshBoundsRecursive(entity, mins, maxs, names, maxDepth);
		FilterAndUnionMeshes(mins, maxs, names, min, max);
	}

	//------------------------------------------------------------------------------------------------
	//! Walk entity hierarchy and append each renderable mesh's world bounds + resource name.
	protected void CollectMeshBoundsRecursive(IEntity entity, inout array<vector> mins, inout array<vector> maxs, inout array<string> names, int depth)
	{
		if (!entity)
			return;

		VObject vObject = entity.GetVObject();
		if (vObject && vObject.ToMeshObject() && !vObject.GetResourceName().IsEmpty())
		{
			vector entityMin, entityMax;
			entity.GetWorldBounds(entityMin, entityMax);

			// Skip entities whose bounds are far from the root (e.g., inventory items at world origin)
			vector boundsCenter = (entityMin + entityMax) * 0.5;
			if (vector.DistanceSq(boundsCenter, m_vBoundsOrigin) <= 2500) // 50m
			{
				mins.Insert(entityMin);
				maxs.Insert(entityMax);
				names.Insert(vObject.GetResourceName());
			}
		}

		if (depth <= 0)
			return;

		IEntity child = entity.GetChildren();
		while (child)
		{
			CollectMeshBoundsRecursive(child, mins, maxs, names, depth - 1);
			child = child.GetSibling();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Iterative greedy outlier filter, then union remaining meshes.
	//!
	//! Each iteration runs two passes:
	//! - Single-drop: for each mesh, "what if we remove it?" The mesh whose removal shrinks
	//!   the union the most is dropped if the shrink exceeds SINGLE_THRESHOLD.
	//! - Pair-drop fallback: if no single mesh qualifies, try every pair (i, j) for shrinkage.
	//!   Catches matched foundations (two dirt mounds, mirrored bases) where each mesh shields
	//!   the other from the single-drop test.
	//!
	//! Shrinkage is summed across X/Y/Z so wide foundations (multi-dim outliers) are preferred
	//! over tall subjects (single-dim).
	//!
	//! Skip when mesh count < 3 — no reliable "crowd" to compare against.
	//! Cap drops at floor(N/2) so we never strip more than half the meshes.
	protected void FilterAndUnionMeshes(array<vector> mins, array<vector> maxs, array<string> names, out vector min, out vector max)
	{
		min = Vector(float.MAX, float.MAX, float.MAX);
		max = -Vector(float.MAX, float.MAX, float.MAX);

		int count = mins.Count();
		if (count == 0)
			return;

		// Mark dropped meshes so we don't have to mutate the input arrays.
		array<bool> dropped = {};
		dropped.Resize(count);

		const float SINGLE_THRESHOLD = 0.30; // 30% of current union extent sum
		const float PAIR_THRESHOLD = 0.20;   // catches matched foundations (e.g. two dirt covers) where each mesh shields the other from single-drop
		int maxDrops = count / 2;
		int dropCount = 0;

		// Compute initial union over all meshes.
		ComputeUnion(mins, maxs, dropped, min, max);

		if (DEBUG_OUTLIER_FILTER)
		{
			PrintFormat("[FitView] mesh count=%1, initial extent=(%2, %3, %4)", count, max[0] - min[0], max[1] - min[1], max[2] - min[2], level: LogLevel.NORMAL);
			for (int k = 0; k < count; k++)
			{
				vector ms = maxs[k] - mins[k];
				PrintFormat("[FitView]   #%1 size=(%2, %3, %4) center=(%5, %6, %7) %8", k, ms[0], ms[1], ms[2], (mins[k][0] + maxs[k][0]) * 0.5, (mins[k][1] + maxs[k][1]) * 0.5, (mins[k][2] + maxs[k][2]) * 0.5, names[k], level: LogLevel.NORMAL);
			}
		}

		while (count >= 3 && dropCount < maxDrops)
		{
			vector size = max - min;
			float extentSum = size[0] + size[1] + size[2];
			if (extentSum <= 0)
				break;

			// Pass 1: best single-drop candidate.
			int singleIdx = -1;
			float singleShrink;

			for (int i = 0; i < count; i++)
			{
				if (dropped[i])
					continue;

				vector minWithout, maxWithout;
				dropped[i] = true;
				ComputeUnion(mins, maxs, dropped, minWithout, maxWithout);
				dropped[i] = false;

				if (minWithout[0] >= maxWithout[0])
					continue;

				float shrink = ShrinkAmount(min, max, minWithout, maxWithout);

				if (singleIdx < 0 || shrink > singleShrink)
				{
					singleIdx = i;
					singleShrink = shrink;
				}
			}

			if (singleIdx >= 0 && singleShrink > SINGLE_THRESHOLD * extentSum)
			{
				if (DEBUG_OUTLIER_FILTER)
					PrintFormat("[FitView] DROP single #%1 shrink=%2 (%3%% of %4) %5", singleIdx, singleShrink, singleShrink / extentSum * 100, extentSum, names[singleIdx], level: LogLevel.NORMAL);
				dropped[singleIdx] = true;
				dropCount++;
				ComputeUnion(mins, maxs, dropped, min, max);
				continue;
			}

			if (DEBUG_OUTLIER_FILTER && singleIdx >= 0)
				PrintFormat("[FitView] best single #%1 shrink=%2 (%3%% of %4) below threshold %5%%", singleIdx, singleShrink, singleShrink / extentSum * 100, extentSum, SINGLE_THRESHOLD * 100, level: LogLevel.NORMAL);

			// Pass 2: best pair-drop fallback. Only run when we still have headroom for two drops.
			if (dropCount + 2 > maxDrops)
			{
				if (DEBUG_OUTLIER_FILTER)
					PrintFormat("[FitView] pair-drop skipped: would exceed maxDrops=%1 (dropCount=%2)", maxDrops, dropCount, level: LogLevel.NORMAL);
				break;
			}

			int pairA = -1, pairB = -1;
			float pairShrink;

			for (int i = 0; i < count; i++)
			{
				if (dropped[i])
					continue;

				for (int j = i + 1; j < count; j++)
				{
					if (dropped[j])
						continue;

					vector minWithout, maxWithout;
					dropped[i] = true;
					dropped[j] = true;
					ComputeUnion(mins, maxs, dropped, minWithout, maxWithout);
					dropped[i] = false;
					dropped[j] = false;

					if (minWithout[0] >= maxWithout[0])
						continue;

					float shrink = ShrinkAmount(min, max, minWithout, maxWithout);

					if (pairA < 0 || shrink > pairShrink)
					{
						pairA = i;
						pairB = j;
						pairShrink = shrink;
					}
				}
			}

			if (pairA < 0 || pairShrink <= PAIR_THRESHOLD * extentSum)
			{
				if (DEBUG_OUTLIER_FILTER && pairA >= 0)
					PrintFormat("[FitView] best pair (#%1, #%2) shrink=%3 (%4%% of %5) below threshold %6%%", pairA, pairB, pairShrink, pairShrink / extentSum * 100, extentSum, PAIR_THRESHOLD * 100, level: LogLevel.NORMAL);
				break;
			}

			if (DEBUG_OUTLIER_FILTER)
				PrintFormat("[FitView] DROP pair (#%1, #%2) shrink=%3 (%4%% of %5)\n        #%1: %6\n        #%2: %7", pairA, pairB, pairShrink, pairShrink / extentSum * 100, extentSum, names[pairA], names[pairB], level: LogLevel.NORMAL);

			dropped[pairA] = true;
			dropped[pairB] = true;
			dropCount = dropCount + 2;
			ComputeUnion(mins, maxs, dropped, min, max);
		}

		if (DEBUG_OUTLIER_FILTER)
			PrintFormat("[FitView] final extent=(%1, %2, %3) drops=%4", max[0] - min[0], max[1] - min[1], max[2] - min[2], dropCount, level: LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	//! Sum of inward movement of all six AABB faces when the candidate union shrinks from
	//! (min, max) to (minWithout, maxWithout).
	protected float ShrinkAmount(vector min, vector max, vector minWithout, vector maxWithout)
	{
		float shrink = 0;
		for (int d = 0; d < 3; d++)
		{
			shrink = shrink + Math.Max(0, minWithout[d] - min[d]);
			shrink = shrink + Math.Max(0, max[d] - maxWithout[d]);
		}
		return shrink;
	}

	//------------------------------------------------------------------------------------------------
	//! Compute the AABB union over all meshes whose dropped flag is false.
	protected void ComputeUnion(array<vector> mins, array<vector> maxs, array<bool> dropped, out vector min, out vector max)
	{
		min = Vector(float.MAX, float.MAX, float.MAX);
		max = -Vector(float.MAX, float.MAX, float.MAX);

		int count = mins.Count();
		for (int i = 0; i < count; i++)
		{
			if (dropped[i])
				continue;

			vector m = mins[i];
			vector M = maxs[i];
			min[0] = Math.Min(min[0], m[0]);
			min[1] = Math.Min(min[1], m[1]);
			min[2] = Math.Min(min[2], m[2]);
			max[0] = Math.Max(max[0], M[0]);
			max[1] = Math.Max(max[1], M[1]);
			max[2] = Math.Max(max[2], M[2]);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Adjust DoF parameters (FocusDistance, FocalLength, FocalLengthNear) proportionally
	//! to the new camera distance. Reads original values from the emat resource and scales
	//! them by the ratio of new/original distance to the bounding box center.
	protected void AdjustDepthOfField(CameraBase cam, vector bbCenter)
	{
		SCR_PostProcessCameraComponent ppComponent = SCR_PostProcessCameraComponent.Cast(cam.FindComponent(SCR_PostProcessCameraComponent));
		if (!ppComponent)
			return;

		SCR_CameraPostProcessEffect dofEffect = ppComponent.FindEffect(PostProcessEffectType.DepthOfFieldBokeh);
		if (!dofEffect)
			return;

		float newFocusDistance = vector.Distance(cam.GetOrigin(), bbCenter);

		//--- Read original values from the emat resource
		ResourceName matPath = dofEffect.GetMaterialPath();
		if (matPath.IsEmpty())
			return;

		Resource matRes = Resource.Load(matPath);
		if (!matRes || !matRes.IsValid())
			return;

		BaseContainer matContainer = matRes.GetResource().ToBaseContainer();
		if (!matContainer)
			return;

		float originalFocusDistance, originalFocalLength, originalFocalLengthNear;
		matContainer.Get("FocusDistance", originalFocusDistance);
		matContainer.Get("FocalLength", originalFocalLength);
		matContainer.Get("FocalLengthNear", originalFocalLengthNear);

		if (originalFocusDistance <= 0)
			return;

		float ratio = newFocusDistance / originalFocusDistance;

		dofEffect.SetParam("FocusDistance", newFocusDistance);
		dofEffect.SetParam("FocalLength", originalFocalLength * ratio);
		dofEffect.SetParam("FocalLengthNear", 0);
	}

	//------------------------------------------------------------------------------------------------
	//! Restore camera to its original position after FitCameraToEntity adjusted it.
	protected void RestoreCamera()
	{
		if (!m_AdjustedCamera)
			return;

		vector camTransform[4];
		m_AdjustedCamera.GetTransform(camTransform);
		camTransform[3] = m_vOriginalCameraPos;
		m_AdjustedCamera.SetTransform(camTransform);
		m_AdjustedCamera = null;
	}

	protected void AddSubPosition(SCR_EditorImagePositionEntity subPosition)
	{
		m_aSubPositions.Insert(subPosition.GetPriority(), subPosition);
	}
	protected void UpdateNearbyEntities()
	{
		m_aCurrentNearbyEntities.Clear();
		GetWorld().QueryEntitiesByAABB(GetOrigin() + vector.One * -128, GetOrigin() + vector.One * 128, QueryEntitiesCallback);
	}
	protected bool QueryEntitiesCallback(IEntity e)
	{
		m_aCurrentNearbyEntities.Insert(e);
		return true;
	}
	protected string GetPositionName()
	{
		if (GetName().IsEmpty())
			return GetOrigin().ToString();
		else
			return GetName();
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] entity
	event void EOnImagePositonActivate(IEntity entity)
	{
	}

	override void EOnInit(IEntity owner)
	{
		if (SCR_Global.IsEditMode(this))
			return;

		SCR_EditorImageGeneratorEntity manager = SCR_EditorImageGeneratorEntity.GetInstance();
		if (!manager)
		{
			Debug.Error2(Type().ToString(), "SCR_EditorImageGeneratorEntity is missing in the world!");
			return;
		}

		if (m_Parent)
		{
			//--- Sub-position
			m_Parent.AddSubPosition(this);
		}
		else
		{
			//--- Main position — register with the manager; cameras are found later
			//--- in ActivatePosition when the entity tree is fully built.
			manager.AddPosition(this);
		}

		//--- Get nearby entities
		UpdateNearbyEntities();
		m_aOriginalNearbyEntities.Copy(m_aCurrentNearbyEntities);
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] parent
	void SCR_EditorImagePositionEntity(IEntitySource src, IEntity parent)
	{
		m_Parent = SCR_EditorImagePositionEntity.Cast(parent);

		if (SCR_Global.IsEditMode(this))
		{
			if (m_PreviewMesh)
			{
				Resource resource = Resource.Load(m_PreviewMesh);
				if (!resource.IsValid())
				{
					Print("Cannot load " + m_PreviewMesh + " | " + FilePath.StripPath(__FILE__) + ":" + __LINE__, LogLevel.WARNING);
					return;
				}

				SetObject(resource.GetResource().ToVObject(), "");
			}

			return;
		}

		//--- Discover camera child names from entity source
		//--- (GetChildren() doesn't work for layer-defined hierarchies at runtime)
		for (int i = 0, count = src.GetNumChildren(); i < count; i++)
		{
			IEntitySource childSrc = src.GetChild(i);
			if (childSrc.GetClassName().ToType().IsInherited(CameraBase))
				m_aCameraChildNames.Insert(childSrc.GetName());
		}

		SetEventMask(EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_EditorImagePositionEntity()
	{
		WorldEditorAPI api = _WB_GetEditorAPI();
		if (api && api.IsEntitySelected(api.EntityToSource(this)))
			SCR_EditorImageGeneratorEntity.AddSelectedPosition(this);
	}

	//------------------------------------------------------------------------------------------------
	override int _WB_GetAfterWorldUpdateSpecs(IEntitySource src)
	{
		return EEntityFrameUpdateSpecs.CALL_WHEN_ENTITY_VISIBLE;
	}

	//------------------------------------------------------------------------------------------------
	override void _WB_AfterWorldUpdate(float timeSlice)
	{
		if (m_Labels.IsEmpty())
			return;

		string name;
		for (int i = 0, count = m_Labels.Count(); i < count; i++)
		{
			if (i != 0)
			{
				if (i % 2 == 0)
					name += ",\n";
				else
					name += ", ";
			}

			name += typename.EnumToString(EEditableEntityLabel, m_Labels[i]);
		}

		float fontSize = 12;
		if (m_Parent)
			fontSize = 8;

		vector pos = GetOrigin();
		DebugTextWorldSpace.Create(GetWorld(), name, DebugTextFlags.CENTER | DebugTextFlags.FACE_CAMERA | DebugTextFlags.ONCE, pos[0], pos[1], pos[2], fontSize, Color.WHITE, ARGBF(1, 0.5, 0, 1));
	}

	//------------------------------------------------------------------------------------------------
	// Character position
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	protected void ActivateCharacterPosition(ResourceName prefab)
	{
		// Check faction and get pose
		SCR_EditorImagePositionCharacterPose pose = CurrentPose(m_Entity);
		if (!pose)
			return; // No pose defined — spawn character as-is without pose/weapon customization

		Resource characterResource = Resource.Load(prefab);
		ResourceName weaponIK = CharacterWeaponIK(characterResource, pose);

		// Select weapon
		EWeaponType forceWeaponType = pose.GetForceWeaponType();

		if (forceWeaponType >= 0)
		{
			InventoryStorageManagerComponent inventoryStorage = InventoryStorageManagerComponent.Cast(m_Entity.FindComponent(InventoryStorageManagerComponent));
			if (inventoryStorage)
			{
				array<IEntity> items = {};
				for (int i = inventoryStorage.GetItems(items) - 1; i >= 0; i--)
				{
					BaseWeaponComponent weapon = BaseWeaponComponent.Cast(items[i].FindComponent(BaseWeaponComponent));
					if (!weapon)
						continue;

					if (weapon.GetWeaponType() == forceWeaponType)
					{
						m_sNewWeaponMesh = items[i].GetVObject().GetResourceName();
						break;
					}
				}
			}

			BaseWeaponManagerComponent weaponManager = BaseWeaponManagerComponent.Cast(m_Entity.FindComponent(BaseWeaponManagerComponent));
			if (weaponManager)
			{
				BaseWeaponComponent currentWeapon = weaponManager.GetCurrentWeapon();
				if (currentWeapon)
					m_sCurrentWeaponMesh = currentWeapon.GetOwner().GetVObject().GetResourceName();
			}
		}

		vector transform[4];
		m_Entity.GetTransform(transform);
		CloneCharacter(m_Entity, transform);

		SetPose(m_Entity, pose, weaponIK);
	}

	//------------------------------------------------------------------------------------------------
	//! Return pose used for current character
	protected SCR_EditorImagePositionCharacterPose CurrentPose(notnull IEntity entity, SCR_EditorImagePositionEntity position = null)
	{
		if (!position)
			position = this;

		// Get faction key
		SCR_EditableCharacterComponent editable = SCR_EditableCharacterComponent.Cast(entity.FindComponent(SCR_EditableCharacterComponent));
		if (!editable)
		{
			Print("No editable entity for: " + entity.GetName(), LogLevel.WARNING);
			return null;
		}

		SCR_UIInfo uiInfoBase = editable.GetInfo();
		if (!uiInfoBase)
		{
			return null;
		}

		SCR_EditableEntityUIInfo uiInfo = SCR_EditableEntityUIInfo.Cast(editable.GetInfo());
		if (!uiInfo)
		{
			Print("No uiinfo for: " + entity.GetName(), LogLevel.WARNING);
			return null;
		}

		FactionKey factionKey = uiInfo.GetFactionKey();

		// Find pose based on faction
		foreach (SCR_EditorImagePositionCharacterPose pose : position.m_aFactionPoses)
		{
			if (pose.GetFactionKey() == factionKey)
				return pose;
		}

		// Use default if no faction pose is defined
		return position.m_DefaultPose;
	}

	//------------------------------------------------------------------------------------------------
	//! Set character pose animation and holding weapon animation
	protected void SetPose(IEntity entity, notnull SCR_EditorImagePositionCharacterPose pose, ResourceName armsIK)
	{
		ResourceName poseGraph = pose.GetPoseGraph();
		ResourceName poseInstance = pose.GetPosesInstance();
		string startNode = pose.GetStartNode();

		if (poseGraph.IsEmpty() && poseInstance.IsEmpty() && startNode.IsEmpty())
			return;

		PreviewAnimationComponent animComponent = PreviewAnimationComponent.Cast(entity.FindComponent(PreviewAnimationComponent));
		animComponent.SetGraphResource(entity, poseGraph, poseInstance, startNode);

		// Perform one frame step to apply animations immediately
		animComponent.UpdateFrameStep(entity, 1.0 / ANIM_TIME_STEP);

		// Find variable that we want to change
		int poseVar = animComponent.BindIntVariable(pose.GetPosVar());
		if (poseVar == -1)
		{
			Debug.Error2(Type().ToString(), string.Format("Unable to set character pose at positon '%1'!", GetPositionName()));
			return;
		}

		// change pose
		animComponent.SetIntVariable(poseVar, pose.GetPosId());

		// Set hands IK
		int armIkVar = pose.BindAnimValueInteger(animComponent, ANIM_ARM_IK);
		if (armIkVar != -1)
		{
			animComponent.SetIntVariable(armIkVar, pose.GetArmIK());
			animComponent.SetHandsIKPose(entity, armsIK);
		}

		// Perform one frame step to submit graph variable change
		animComponent.UpdateFrameStep(entity, 1.0 / ANIM_TIME_STEP);
	}

	//------------------------------------------------------------------------------------------------
	//! Find anim IK of currently held player weapon
	protected ResourceName CharacterWeaponIK(Resource characterResource, SCR_EditorImagePositionCharacterPose pose)
	{
		//Resource characterResource = Resource.Load(prefab);

		array<ref array<IEntityComponentSource>> weaponSlotComponents = {};
		array<string> componentTypeArray = { "CharacterWeaponSlotComponent" };
		int weaponSlotCount = SCR_BaseContainerTools.FindComponentSources(characterResource, componentTypeArray, weaponSlotComponents);

		array<IEntityComponentSource> weaponSlotComponentSources = weaponSlotComponents.Get(0);
		if (!weaponSlotComponentSources)
			return ResourceName.Empty;

		foreach	(IEntityComponentSource weaponSlotComponent : weaponSlotComponentSources)
		{
			ResourceName weaponPrefab;
			if (!weaponSlotComponent.Get("WeaponTemplate", weaponPrefab))
				return ResourceName.Empty;

			if (!weaponPrefab)
				continue;

			Resource resource = Resource.Load(weaponPrefab);
			if (!resource.IsValid())
				continue;

			IEntitySource weaponSource = SCR_BaseContainerTools.FindEntitySource(resource);
			if (!weaponSource)
				continue;

			// Check type
			IEntityComponentSource weaponComponentSource = SCR_BaseContainerTools.FindComponentSource(resource, WeaponComponent);
			if (!weaponComponentSource)
				continue;

			EWeaponType forcedWeaponType = pose.GetForceWeaponType();

			if (forcedWeaponType != -1)
			{
				EWeaponType weaponType;
				weaponComponentSource.Get("WeaponType", weaponType);

				if (weaponType != pose.GetForceWeaponType())
					continue;
			}

			// Get anim
			IEntityComponentSource weaponEntitySource = SCR_ComponentHelper.GetInventoryItemComponentSource(weaponSource);

			BaseContainer attributesContainer = weaponEntitySource.GetObject("Attributes");

			BaseContainer animAttributes = attributesContainer.GetObject("ItemAnimationAttributes");

			ResourceName animIkResourceName;
			animAttributes.Get("AnimationIKPose", animIkResourceName);
			return animIkResourceName;
		}

		return ResourceName.Empty;
	}

	//------------------------------------------------------------------------------------------------
	protected void CloneCharacter(out IEntity character, vector transform[4])
	{
		InventoryItemComponent inventoryComponent = InventoryItemComponent.Cast(character.FindComponent(InventoryItemComponent));
		if (!inventoryComponent)
			return;

		//--- Clone character using inventory preview function so we can play animations on it
		IEntity clone = inventoryComponent.CreatePreviewEntity(GetWorld(), GetWorld().GetCurrentCameraId());
		if (!clone)
			return;

		//--- Swap weapons
		if (m_sNewWeaponMesh != m_sCurrentWeaponMesh)
		{
			IEntity currentWeapon, newWeapon;
			TNodeId currentPivot, newPivot;
			vector currentLocalTransform[4], newLocalTransform[4];

			IEntity child = clone.GetChildren();
			while (child)
			{
				VObject mesh = child.GetVObject();
				if (mesh)
				{
					if (mesh.GetResourceName() == m_sCurrentWeaponMesh)
					{
						currentWeapon = child;
						currentPivot = child.GetPivot();
						child.GetLocalTransform(currentLocalTransform);
					}
					if (mesh.GetResourceName() == m_sNewWeaponMesh)
					{
						newWeapon = child;
						newPivot = child.GetPivot();
						child.GetLocalTransform(newLocalTransform);
					}
				}
				child = child.GetSibling();
			}

			clone.AddChild(newWeapon, currentPivot);
			newWeapon.SetLocalTransform(currentLocalTransform);

			clone.AddChild(currentWeapon, newPivot);
			currentWeapon.SetLocalTransform(newLocalTransform);
		}

		SCR_EntityHelper.DeleteEntityAndChildren(character);
		character = clone;
		character.SetTransform(transform);
	}

	//------------------------------------------------------------------------------------------------
	// Groups position
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	protected bool ActivateGroupPosition(SCR_AIGroup group, EntitySpawnParams spawnParams)
	{
		//--- Make sure group AI members are spawned instantly, not asynchronously
		//group.SetMemberSpawnDelay(0);
		//group.SpawnUnits();

		array<AIAgent> agents = {};
		int agentCount = group.GetAgents(agents);

		if (agentCount > m_aSubPositions.Count())
		{
			//Print(string.Format("Cannot capture group @\"%1\"! It has %2 members, but the position '%3' has only %4 sub-positions!", prefab, agentCount, GetPositionName(), m_aSubPositions.Count()), LogLevel.WARNING);
			return false;
		}

		m_Entity = GetGame().SpawnEntity(GenericEntity, GetWorld(), spawnParams);
		IEntity member;
		vector transform[4];
		SCR_SortedArray<SCR_EditorImagePositionEntity> subPositions = new SCR_SortedArray<SCR_EditorImagePositionEntity>();
		subPositions.CopyFrom(m_aSubPositions);
		SCR_EditorImagePositionEntity subPosition;
		bool failed = false;

		for (int i = 0; i < agentCount; i++)
		{
			member = agents[i].GetControlledEntity();

			//--- Get member labels and find suitable sub-position
			SCR_EditableEntityComponent editableMember = SCR_EditableEntityComponent.GetEditableEntity(member);
			SCR_EditableEntityUIInfo info = SCR_EditableEntityUIInfo.Cast(editableMember.GetInfo());
			array<EEditableEntityLabel> memberLabels = {};
			if (info)
				info.GetEntityLabels(memberLabels);

			subPosition = FindSuitableSubPosition(subPositions, memberLabels);
			if (subPosition)
			{
				// Prepare position values
				SCR_EditorImagePositionCharacterPose pose = CurrentPose(member, subPosition);
				if (!pose)
				{
					PrintFormat("Skipping group member @\"%1\" — no pose defined on sub-position '%2'", member.GetPrefabData().GetPrefabName(), subPosition.GetPositionName(), level: LogLevel.WARNING);
					continue;
				}

				Resource characterResource = BaseContainerTools.CreateContainerFromInstance(member);
				ResourceName weaponIK = CharacterWeaponIK(characterResource, pose);

				// Apply position
				subPositions.RemoveValues(subPosition);
				subPosition.GetTransform(transform);

				CloneCharacter(member, transform);
				//m_Entity.AddChild(member, -1, EAddChildFlags.RECALC_LOCAL_TRANSFORM); //--- Crashes the game! Not needed, area garbage collection removes clones anyway.
				subPosition.SetPose(member, pose, weaponIK);
				subPosition.EOnImagePositonActivate(member);
			}
			else
			{
				PrintFormat("Cannot capture group member @\"%1\"! Unable to find suitable sub-position for %1!", member.GetPrefabData().GetPrefabName(), level: LogLevel.WARNING);
				failed = true;
			}
		}
		SCR_EntityHelper.DeleteEntityAndChildren(group);

		if (failed)
			return false;

		EOnImagePositonActivate(m_Entity);
		return true;
	}
#endif
}
