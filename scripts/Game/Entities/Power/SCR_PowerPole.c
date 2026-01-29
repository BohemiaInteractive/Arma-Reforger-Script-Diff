[EntityEditorProps(category: "GameScripted/Power", description: "This is the power pole entity.", color: "0 255 0 255", visible: false, dynamicBox: true)]
class SCR_PowerPoleClass : PowerPoleEntityClass
{
	/*
		Cable Slots
	*/

	[Attribute(category: "[Prefab-Wide] Cable Slots")]
	ref array<ref SCR_PoleCableSlotGroup> m_aCableSlotGroups; // since 2024-04-02
}

class SCR_PowerPole : PowerPoleEntity
{
	/*
		[OLD] Power Cable Slots
	*/

	[Attribute(uiwidget: UIWidgets.None, desc: "[OBSOLETE (use Cable Slot Groups above)] Slots for connecting with other power poles", category: "[OLD] Power Cable Slots")]
	protected ref array<ref SCR_PowerPoleSlotBase> m_aSlots; // obsolete since 2024-04-02 - hidden since 2024-08-07

	// Connected powerlines handling variables
	protected static const float CONNECTED_POLE_SEARCH_RADIUS = 3.0; // meters to search for connected powerlines (matching building destruction pattern)

#ifdef WORKBENCH

	//------------------------------------------------------------------------------------------------
	//! Get the closest non-empty cable slot group for each available cable type
	//! \param[in] worldPos the position from which to find the closest slots
	//! \param[in] isSameLine true to get same line-accepting slots, false to get external line-accepting slots
	//! \return a cableType-slotGroup map - never returns null
	map<SCR_EPoleCableType, ref SCR_PoleCableSlotGroup> GetClosestCableSlotGroupsForEachCableType(vector worldPos, bool isSameLine)
	{
		map<SCR_EPoleCableType, ref SCR_PoleCableSlotGroup> result = new map<SCR_EPoleCableType, ref SCR_PoleCableSlotGroup>();
		map<SCR_EPoleCableType, float> distancesSq = new map<SCR_EPoleCableType, float>();

		SCR_PowerPoleClass prefabData = SCR_PowerPoleClass.Cast(GetPrefabData());
		if (!prefabData)
			return result;

		foreach (SCR_PoleCableSlotGroup group : prefabData.m_aCableSlotGroups)
		{
			vector avgPos = vector.Zero;
			int count = group.m_aSlots.Count();
			if (count < 1)
				continue;

			if (group.m_eConnectivity != SCR_EPoleCableSlotConnectivity.ALL_LINES)
			{
				// SCR_EPoleCableSlotConnectivity.ALL_LINES = OK
				// SCR_EPoleCableSlotConnectivity.SAME_LINE + isSameLine = OK
				// SCR_EPoleCableSlotConnectivity.EXTERNAL_LINE + !isSameLine = OK
				// anything else skips
				if ((group.m_eConnectivity == SCR_EPoleCableSlotConnectivity.SAME_LINE) != isSameLine)
					continue;
			}

			if (group.m_vAnchorOverride == vector.Zero)
			{
				foreach (SCR_PoleCableSlot slot : group.m_aSlots)
				{
					avgPos += slot.m_vPosition;
				}

				avgPos /= count;
			}
			else
			{
				avgPos = group.m_vAnchorOverride;
			}

			float distanceSq = vector.DistanceSq(worldPos, CoordToParent(avgPos));
			float storedGroupDistanceSq = distancesSq.Get(group.m_eCableType);
			if (storedGroupDistanceSq == 0) // not present - I believe faster than Contains, Get, Set/Insert
			{
				distancesSq.Insert(group.m_eCableType, distanceSq);
				result.Insert(group.m_eCableType, group);
			}
			else
			if (distanceSq < storedGroupDistanceSq)
			{
				distancesSq.Set(group.m_eCableType, distanceSq);
				result.Set(group.m_eCableType, group);
			}
		}

		return result;
	}

	//
	// temp debug shapes for cable slot placement
	//

	protected static bool s_bDisplayCableSlots;
	protected static ref SCR_DebugShapeManager s_DebugShapeManager;

	protected static const float MIN_AVG_ANCHOR_DIST = 0.5;
	protected static const int DEBUG_SLOT_POS_COLOUR_1 = Color.DARK_GREEN & 0x88FFFFFF;
	protected static const int DEBUG_SLOT_POS_COLOUR_2 = Color.DARK_GREEN & 0x55FFFFFF;
	protected static const int DEBUG_PRECISION_LINE_COLOUR = Color.RED;
	protected static const float DEBUG_PRECISION_LINE_SIZE = 0.1;

	protected static const int DEBUG_ANCHOR_POS_COLOUR = Color.ORANGE & 0xBBFFFFFF;
	protected static const int DEBUG_ANCHOR_LINE_COLOUR = Color.ORANGE & 0x66FFFFFF;
	protected static const float DEBUG_SLOT_POS_SIZE_1 = 0.05;
	protected static const float DEBUG_SLOT_POS_SIZE_2 = 0.25;
	protected static const float DEBUG_ANCHOR_POS = 0.075;

	//------------------------------------------------------------------------------------------------
	override bool _WB_OnKeyChanged(BaseContainer src, string key, BaseContainerList ownerContainers, IEntity parent)
	{
		bool result = super._WB_OnKeyChanged(src, key, ownerContainers, parent);
		if (!src || _WB_GetEditorAPI().UndoOrRedoIsRestoring())
			return result;

		typename srcType = src.GetClassName().ToType();
		if (
			!srcType ||
			(key != "m_vPosition" && key != "m_vAnchorOverride" && key != "m_eConnectivity") ||
			(!srcType.IsInherited(SCR_PoleCableSlot) && !srcType.IsInherited(SCR_PoleCableSlotGroup)))
		{
			if (s_DebugShapeManager)
				s_DebugShapeManager.Clear();

			s_bDisplayCableSlots = false;
			return result;
		}

		s_bDisplayCableSlots = true;

		if (s_DebugShapeManager)
			UpdateDebugShapes();

		return result;
	}

//	why these two do not work reliably, IDK
//	_WB_AfterWorldUpdate	// sometimes doesn't work
//	thread					// never works / crashes WB

	//------------------------------------------------------------------------------------------------
	override int _WB_GetAfterWorldUpdateSpecs(IEntitySource src)
	{
		return EEntityFrameUpdateSpecs.CALL_WHEN_ENTITY_VISIBLE;
	}

	//------------------------------------------------------------------------------------------------
	override void _WB_AfterWorldUpdate(float timeSlice)
	{
		if (s_bDisplayCableSlots && _WB_GetEditorAPI() && _WB_GetEditorAPI().IsPrefabEditMode())
			UpdateDebugShapes();
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateDebugShapes()
	{
		SCR_PowerPoleClass prefabData = SCR_PowerPoleClass.Cast(GetPrefabData());
		if (!prefabData)
			return;

		array<vector> lines = {
			vector.FromYaw(45) * DEBUG_PRECISION_LINE_SIZE,
			vector.Up * DEBUG_PRECISION_LINE_SIZE,
			vector.FromYaw(-45) * DEBUG_PRECISION_LINE_SIZE,
		};

		s_DebugShapeManager.Clear();
		foreach (SCR_PoleCableSlotGroup group : prefabData.m_aCableSlotGroups)
		{
			foreach (SCR_PoleCableSlot slot : group.m_aSlots)
			{
				s_DebugShapeManager.AddSphere(slot.m_vPosition, DEBUG_SLOT_POS_SIZE_1, DEBUG_SLOT_POS_COLOUR_1, ShapeFlags.DEPTH_DITHER | ShapeFlags.NOOUTLINE);
				s_DebugShapeManager.AddSphere(slot.m_vPosition, DEBUG_SLOT_POS_SIZE_2, DEBUG_SLOT_POS_COLOUR_2, ShapeFlags.DEPTH_DITHER | ShapeFlags.NOOUTLINE);
				foreach (vector line : lines)
				{
					s_DebugShapeManager.AddLine(slot.m_vPosition + line, slot.m_vPosition - line, DEBUG_PRECISION_LINE_COLOUR, ShapeFlags.DEPTH_DITHER);
				}
			}

			vector anchor = group.m_vAnchorOverride;
			int count = group.m_aSlots.Count();
			bool drawAnchor = anchor != vector.Zero || count > 1;

			if (anchor == vector.Zero && count > 1)
			{
				foreach (SCR_PoleCableSlot slot : group.m_aSlots)
				{
					anchor += slot.m_vPosition;
				}

				anchor /= count;

				foreach (SCR_PoleCableSlot slot : group.m_aSlots)
				{
					if (vector.Distance(anchor, slot.m_vPosition) < MIN_AVG_ANCHOR_DIST)
					{
						drawAnchor = false;
						break;
					}
				}
			}

			if (anchor != vector.Zero || count > 1)
			{
				if (drawAnchor)
					s_DebugShapeManager.AddSphere(anchor, DEBUG_ANCHOR_POS, DEBUG_ANCHOR_POS_COLOUR, ShapeFlags.DEPTH_DITHER | ShapeFlags.NOOUTLINE);

				// lines will be drawn
				foreach (SCR_PoleCableSlot slot : group.m_aSlots)
				{
					s_DebugShapeManager.AddLine(slot.m_vPosition, anchor, DEBUG_ANCHOR_LINE_COLOUR, ShapeFlags.DEPTH_DITHER);
				}
			}
		}
	}

#endif // WORKBENCH

	//------------------------------------------------------------------------------------------------
	//! Override to handle cascading destruction when entering FirstDestructionPhase
	override void OnStateChanged(int destructibleState, ScriptBitReader frameData, bool JIP)
	{
		super.OnStateChanged(destructibleState, frameData, JIP);

		// Only handle connected powerlines on first destruction phase (state 1)
		if (destructibleState == 1 && !JIP)
		{
			HandleConnectedPowerlines();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Handle connected powerlines when this pole is destroyed
	//! Based on HandleConnectedPowerlines in SCR_DestructibleBuildingComponent
	protected void HandleConnectedPowerlines()
	{
		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return;

		// Get cable slot group positions like in SCR_DestructibleBuildingComponent
		array<vector> polePositions = GetCableSlotGroupPositions();

		// Query for powerlines around each cable slot group position and delete them
		foreach (vector polePosition : polePositions)
		{
			world.QueryEntitiesBySphere(polePosition, CONNECTED_POLE_SEARCH_RADIUS, ProcessFoundPowerline, FilterPowerlineEntity);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Get cable slot group positions, following the exact pattern from SCR_DestructibleBuildingComponent
	protected array<vector> GetCableSlotGroupPositions()
	{
		array<vector> positions = {};

		// Get prefab data to access cable slot groups
		SCR_PowerPoleClass prefabData = SCR_PowerPoleClass.Cast(GetPrefabData());
		if (!prefabData || !prefabData.m_aCableSlotGroups)
		{
			// Fallback to entity origin if no cable slot groups
			positions.Insert(GetOrigin());
			return positions;
		}

		// Iterate through cable slot groups and average slot positions per group
		foreach (SCR_PoleCableSlotGroup slotGroup : prefabData.m_aCableSlotGroups)
		{
			if (!slotGroup || !slotGroup.m_aSlots)
				continue;

			vector avgLocalPos = vector.Zero;
			int validSlotCount = 0;

			// Sum all slot positions in this group
			foreach (SCR_PoleCableSlot slot : slotGroup.m_aSlots)
			{
				if (!slot)
					continue;

				avgLocalPos += slot.m_vPosition;
				validSlotCount++;
			}

			// Calculate average and transform to world space
			if (validSlotCount > 0)
			{
				avgLocalPos = avgLocalPos / validSlotCount;
				vector avgWorldPos = CoordToParent(avgLocalPos);
				positions.Insert(avgWorldPos);
			}
		}

		return positions;
	}

	//------------------------------------------------------------------------------------------------
	//! Filter callback to only process PowerlineEntity types during sphere query
	//! This is more efficient than checking entity type in ProcessFoundPowerline
	protected bool FilterPowerlineEntity(notnull IEntity entity)
	{
		return entity.Type() == PowerlineEntity;
	}

	//------------------------------------------------------------------------------------------------
	//! Process each found powerline entity and delete it
	//! Simplified since filtering is done in FilterPowerlineEntity
	protected bool ProcessFoundPowerline(notnull IEntity entity)
	{
		delete entity;
		return true;
	}


	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] parent
	void SCR_PowerPole(IEntitySource src, IEntity parent)
	{
#ifdef WORKBENCH

		if (_WB_GetEditorAPI() && _WB_GetEditorAPI().IsPrefabEditMode())
		{
			if (!s_DebugShapeManager)
				s_DebugShapeManager = new SCR_DebugShapeManager();
		}
#endif // WORKBENCH
	}
}
