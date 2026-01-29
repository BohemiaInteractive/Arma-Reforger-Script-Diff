#ifdef WORKBENCH
[WorkbenchToolAttribute(name: "Entity Replacer", wbModules: { "WorldEditor" }, shortcut: "Ctrl+H", awesomeFontCode: 0xF362,
description:
"- Use Ctrl+Click to add to the 'To Replace' array\n"
+ "- Use Ctrl+Shift+Click to remove from the 'To Replace' array\n"
+ "\n"
+ "- Use Alt+Click to add to the 'Replace With' array\n"
+ "- Use Alt+Shift+Click to remove from the 'Replace With' array\n"
+ "\n"
+ "- Use Shift+Click to add to selection\n"
+ "- Click anywhere to deselect everything\n"
+ "- In order to Replace from Selected Entities, selected objects types must still be added to the 'To Replace' array!")]
class SCR_EntityReplacerTool : WorldEditorTool
{
	[Attribute(desc: "Which entities will be ", category: "Replacement Options")]
	protected ref array<ref EntityReplacerTool_ToReplace> m_aToReplace;

	[Attribute(category: "Replacement Options")]
	protected ref array<ref EntityReplacerTool_ReplaceWith> m_aReplaceWith;

	[Attribute(defvalue: "0", desc: "If checked, will replace in ToReplace → ReplaceWith order (item 1 replaced with item 1, etc). If not, select a random ReplaceWith entry.\nIf there are more ToReplace than ReplaceWith entries, the overflow will be selected randomly.", category: "Replacement Options")]
	protected bool m_bReplaceInOrder;

	[Attribute(defvalue: "0", desc: "Only search/replace in the currently active layer", category: "Replacement Options")]
	protected bool m_bActiveLayerOnly;

	[Attribute(defvalue: "2000", desc: "Maximum replacement entities selected after the operation. If set to 0, will not change selection.", params: "0 10000 100", category: "Performance")]
	protected int m_iMaxSelectedEntities;

	protected static const ref array<IEntitySource> ENTITIES_TO_PARSE = {};
	protected static const ref RandomGenerator RANDOM_GENERATOR = new RandomGenerator();

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Replace from Selected Entities")]
	protected void ReplaceFromSelectedEntities()
	{
		ReplaceFromEntities(false);
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Replace from All Entities")]
	protected void ReplaceFromAllEntities()
	{
		ReplaceFromEntities(true);
	}

	//------------------------------------------------------------------------------------------------
	protected void ReplaceFromEntities(bool allEntities = true)
	{
		// can't happen with a WorldEditorTool?
		if (!SCR_Global.IsEditMode())
		{
			Print("World Editor not in Edit mode", LogLevel.WARNING);
			return;
		}

		RemoveInvalidResourceNames();
		WorldEditorTool.UpdatePropertyPanel();

		if (m_aToReplace.IsEmpty())
		{
			Print("No Entities to replace defined", LogLevel.WARNING);
			return;
		}

		if (m_aReplaceWith.IsEmpty())
		{
			Print("No replacement Entities defined", LogLevel.WARNING);
			return;
		}

		if (!m_API)
		{
			Print("No World Editor API", LogLevel.WARNING);
			return;
		}

		BaseWorld baseWorld = m_API.GetWorld();

		Debug.BeginTimeMeasure();
		if (allEntities)
		{
			GetAllEntities(baseWorld);
		}
		else
		{
			ENTITIES_TO_PARSE.Clear();
			for (int i = 0, cnt = m_API.GetSelectedEntitiesCount(); i < cnt; i++)
			{
				ENTITIES_TO_PARSE.Insert(m_API.GetSelectedEntity(i));
			}
		}
		Debug.EndTimeMeasure(string.Format("%1 entities found", ENTITIES_TO_PARSE.Count()));

		if (ENTITIES_TO_PARSE.IsEmpty())
		{
			Print("No entities found, stopping", LogLevel.NORMAL);
			return;
		}

		// array of entities to be LATER deleted (children references etc)
		array<IEntitySource> sourcesToDelete = {};

		m_API.BeginEntityAction("Replace Entities");

		// create first
		Debug.BeginTimeMeasure();
		array<IEntitySource> createdEntitySources = CreateReplacements(baseWorld, sourcesToDelete);
		Debug.EndTimeMeasure(string.Format("%1 entities created", createdEntitySources.Count()));

		// delete then
		Debug.BeginTimeMeasure();
		m_API.DeleteEntities(sourcesToDelete);
		Debug.EndTimeMeasure(string.Format("%1 entities deleted", createdEntitySources.Count()));

		if (m_iMaxSelectedEntities > 0)
			SelectEntities(createdEntitySources);

		m_API.EndEntityAction();
		ENTITIES_TO_PARSE.Clear();

		Print("Replacement done", LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	protected array<IEntitySource> CreateReplacements(BaseWorld baseWorld, notnull out array<IEntitySource> toDelete)
	{
		array<float> weights = {};
		foreach (EntityReplacerTool_ReplaceWith replaceWith : m_aReplaceWith)
		{
			weights.Insert(replaceWith.m_fReplacementWeight);
		}

		int currentLayerId = m_API.GetCurrentEntityLayerId();
		int replaceWithCount = m_aReplaceWith.Count();

		IEntitySource parentSource, childSource, createdEntitySource;
		BaseContainer ancestor;
		ResourceName resourceName;
		int replaceIndex, entityLayerId;
		string name;
		vector coords, angles, worldPos, finalCoords;

		array<IEntitySource> createdEntitySources = {};
		float terrainY;

		int totalChildOperationTime, childTick;
		int totalCreationOperationTime = System.GetTickCount();
		foreach (IEntitySource originalEntitySource : ENTITIES_TO_PARSE)
		{
			entityLayerId = originalEntitySource.GetLayerID();
			if (m_bActiveLayerOnly && entityLayerId != currentLayerId)
				continue;

			ancestor = originalEntitySource.GetAncestor();
			if (!ancestor)
				continue;

			resourceName = ancestor.GetResourceName();
			replaceIndex = GetToReplaceIndex(resourceName);
			if (replaceIndex < 0)
				continue;

			// chances of replacement
			if (m_aToReplace[replaceIndex].m_fReplacementPercentage * 0.01 < RANDOM_GENERATOR.RandFloat01())
				continue;

			if (!m_bReplaceInOrder || replaceIndex > replaceWithCount - 1)
				replaceIndex = SCR_ArrayHelper.GetWeightedIndex(weights, RANDOM_GENERATOR.RandFloat01());

			IEntity originalEntity  = m_API.SourceToEntity(originalEntitySource);
			coords = originalEntity.GetOrigin();
			parentSource = originalEntitySource.GetParent();
			if (parentSource)
				coords = SCR_BaseContainerTools.GetLocalCoords(parentSource, coords);
			angles = originalEntity.GetAngles();

			name = originalEntity.GetName();
			originalEntity.SetName(string.Empty); // prevents conflict

			// using CreateEntity and not CreateEntityExt for exact placement
			createdEntitySource = m_API.CreateEntity(m_aReplaceWith[replaceIndex].m_sResourceName, name, entityLayerId, parentSource, coords, angles);
			
			if (parentSource)
				worldPos = m_API.SourceToEntity(parentSource).CoordToParent(coords);
			else
				worldPos = coords;

			if (m_API.SourceToEntity(createdEntitySource).GetFlags() & EntityFlags.RELATIVE_Y)
			{
				terrainY = baseWorld.GetSurfaceY(worldPos[0], worldPos[2]);

				if (parentSource)
					finalCoords = coords;
				else
					finalCoords = worldPos;

				finalCoords[1] = finalCoords[1] - terrainY;
				m_API.SetVariableValue(createdEntitySource, null, "coords", finalCoords.ToString(false));
			}

			childTick = System.GetTickCount();
			// move children to new parent
			for (int j = 0, childrenCount = originalEntitySource.GetNumChildren(); j < childrenCount; j++)
			{
				childSource = originalEntitySource.GetChild(j);
				if (!childSource)
					continue;

				m_API.ParentEntity(originalEntitySource, childSource, true);
			}
			totalChildOperationTime += System.GetTickCount() - childTick;

			createdEntitySources.Insert(createdEntitySource);
			toDelete.Insert(originalEntitySource);
		}
		totalCreationOperationTime = System.GetTickCount() - totalCreationOperationTime;

		Print("creation operation time = " + totalCreationOperationTime + "ms", LogLevel.NORMAL);

		int pct;
		if (totalCreationOperationTime > 0)
			pct = Math.Round(totalChildOperationTime * 10000.0 / totalCreationOperationTime) * 0.01;

		Print("children operation time = " + totalChildOperationTime + "ms (" + pct + "pct)", LogLevel.NORMAL);

		return createdEntitySources;
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnMousePressEvent(float x, float y, WETMouseButtonFlag buttons)
	{
		if (buttons != WETMouseButtonFlag.LEFT)
			return;

		string worldPath;
		m_API.GetWorldPath(worldPath);
		if (worldPath.IsEmpty())
			return;

		IEntity entity;
		vector start, end, normal;
		m_API.TraceWorldPos(x, y, TraceFlags.ENTS | TraceFlags.WORLD, start, end, normal, entity);
		if (entity && entity.Type() == GenericTerrainEntity)
			entity = null;

		bool toReplaceOperation = GetModifierKeyState(ModifierKey.CONTROL);
		bool replaceWithOperation = GetModifierKeyState(ModifierKey.ALT);
		bool isShiftPressed = GetModifierKeyState(ModifierKey.SHIFT);

		IEntitySource entitySource = m_API.EntityToSource(entity);
		
		// allow selection - also explains how to use the tool on wrong controls combination
		if (toReplaceOperation == replaceWithOperation)
		{
			if (toReplaceOperation && replaceWithOperation) // both bools kept for readability
			{
				Print("Use Ctrl OR Alt, not both at the same time", LogLevel.NORMAL);
				Print(string.Empty, LogLevel.NORMAL);
				Print("Use Ctrl to add to the 'To Replace' array", LogLevel.NORMAL);
				Print("Use Alt to add to the 'Replace With' array", LogLevel.NORMAL);
				Print("Use Shift -with- Ctrl/Alt to remove from said array", LogLevel.NORMAL);
				return;
			}

			if (entitySource)
			{
				if (isShiftPressed)
					m_API.AddToEntitySelection(entitySource);
				else
					m_API.SetEntitySelection(entitySource);
			}
			else
			{
				m_API.ClearEntitySelection();
			}

			m_API.UpdateSelectionGui();
			return;
		}

		if (!entity)
			return;

		if (!entitySource)
		{
			Print("no entity source", LogLevel.NORMAL);
			return;
		}

		BaseContainer ancestor = entitySource.GetAncestor();
		if (!ancestor)
		{
			Print("no entity ancestor", LogLevel.NORMAL);
			return;
		}

		ResourceName resourceName = ancestor.GetResourceName();
		if (resourceName.IsEmpty())
		{
			Print("empty resource name", LogLevel.NORMAL);
			return;
		}

		RemoveInvalidResourceNames();

		if (toReplaceOperation)
			ToReplaceOperation(resourceName, isShiftPressed);
		else
			ReplaceWithOperation(resourceName, isShiftPressed);

		WorldEditorTool.UpdatePropertyPanel();
	}

	//------------------------------------------------------------------------------------------------
	protected int GetToReplaceIndex(ResourceName resourceName)
	{
		foreach (int i, EntityReplacerTool_ToReplace toReplace : m_aToReplace)
		{
			if (toReplace.m_sResourceName == resourceName)
				return i;
		}
		return -1;
	}

	//------------------------------------------------------------------------------------------------
	protected int GetReplaceWithIndex(ResourceName resourceName)
	{
		foreach (int i, EntityReplacerTool_ReplaceWith replaceWith : m_aReplaceWith)
		{
			if (replaceWith.m_sResourceName == resourceName)
				return i;
		}
		return -1;
	}

	//------------------------------------------------------------------------------------------------
	protected void ToReplaceOperation(ResourceName resourceName, bool isDeletion)
	{
		int index = GetToReplaceIndex(resourceName);
		if (isDeletion)
		{
			if (index > -1)
				m_aToReplace.RemoveOrdered(index);
			else
				Print(resourceName + " not present in 'To Replace' array", LogLevel.NORMAL);
		}
		else
		{
			if (index < 0)
			{
				EntityReplacerTool_ToReplace toAdd = new EntityReplacerTool_ToReplace();
				toAdd.m_sResourceName = resourceName;
				toAdd.m_fReplacementPercentage = EntityReplacerTool_ToReplace.DEFAULT_REPLACEMENT_PERCENTAGE;
				m_aToReplace.Insert(toAdd);
			}
			else
			{
				Print(resourceName + " already present in 'To Replace' array", LogLevel.NORMAL);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void ReplaceWithOperation(ResourceName resourceName, bool isDeletion)
	{
		int index = GetReplaceWithIndex(resourceName);
		if (isDeletion)
		{
			if (index > -1)
				m_aReplaceWith.RemoveOrdered(index);
			else
				Print(resourceName + " not present in 'Replace With' array", LogLevel.NORMAL);
		}
		else
		{
			if (index < 0)
			{
				EntityReplacerTool_ReplaceWith toAdd = new EntityReplacerTool_ReplaceWith();
				toAdd.m_sResourceName = resourceName;
				toAdd.m_fReplacementWeight = EntityReplacerTool_ReplaceWith.DEFAULT_REPLACEMENT_WEIGHT;
				m_aReplaceWith.Insert(toAdd);
			}
			else
			{
				Print(resourceName + " already present in 'Replace With' array", LogLevel.NORMAL);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void RemoveInvalidResourceNames()
	{
		for (int i = m_aToReplace.Count() - 1; i >= 0; i--)
		{
			if (!m_aToReplace[i] || m_aToReplace[i].m_sResourceName.IsEmpty())
				m_aToReplace.Remove(i);
		}

		for (int i = m_aReplaceWith.Count() - 1; i >= 0; i--)
		{
			if (!m_aReplaceWith[i] || m_aReplaceWith[i].m_sResourceName.IsEmpty())
				m_aReplaceWith.Remove(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Fills ENTITIES_TO_PARSE with found entities
	protected void GetAllEntities(BaseWorld baseWorld)
	{
		ENTITIES_TO_PARSE.Clear();
		vector minPos, maxPos;
		baseWorld.GetBoundBox(minPos, maxPos);
		baseWorld.QueryEntitiesByAABB(minPos, maxPos, InsertEntity);
	}

	//------------------------------------------------------------------------------------------------
	protected bool InsertEntity(IEntity entity)
	{
		if (entity)
			ENTITIES_TO_PARSE.Insert(m_API.EntityToSource(entity));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void SelectEntities(notnull array<IEntitySource> entities)
	{
		m_API.ClearEntitySelection();
		for (int i, cnt = Math.Min(m_iMaxSelectedEntities, entities.Count()); i < cnt; i++)
		{
			m_API.AddToEntitySelection(entities[i]);
		}
		m_API.UpdateSelectionGui();
	}

	//------------------------------------------------------------------------------------------------
	protected array<IEntity> GetEntitiesFromSource(notnull array<IEntitySource> sources)
	{
		array<IEntity> result = {};
		foreach (IEntitySource source : sources)
		{
			result.Insert(m_API.SourceToEntity(source));
		}
		return result;
	}
}

// TODO: SCR_
[BaseContainerProps(), SCR_BaseContainerCustomTitleResourceName("m_sResourceName", true)]
class EntityReplacerTool_ToReplace
{
	static const float DEFAULT_REPLACEMENT_PERCENTAGE = 100;

	[Attribute(uiwidget: UIWidgets.ResourcePickerThumbnail, params: "et")]
	ResourceName m_sResourceName;

	[Attribute(desc: "Defines how many of detected Entities of said type will be replaced", defvalue: DEFAULT_REPLACEMENT_PERCENTAGE.ToString(), uiwidget: UIWidgets.Slider, params: "0 100 0.1", precision: 1)]
	float m_fReplacementPercentage;
}

// TODO: SCR_
[BaseContainerProps(), SCR_BaseContainerCustomTitleResourceName("m_sResourceName", true)]
class EntityReplacerTool_ReplaceWith
{
	static const float DEFAULT_REPLACEMENT_WEIGHT = 50;

	[Attribute(uiwidget: UIWidgets.ResourcePickerThumbnail, params: "et")]
	ResourceName m_sResourceName;

	[Attribute(desc: "Weight in replacement randomisation (more weight = more chance to be selected)", defvalue: DEFAULT_REPLACEMENT_WEIGHT.ToString(), params: "0 100 0.1")]
	float m_fReplacementWeight;
}
#endif // WORKBENCH
