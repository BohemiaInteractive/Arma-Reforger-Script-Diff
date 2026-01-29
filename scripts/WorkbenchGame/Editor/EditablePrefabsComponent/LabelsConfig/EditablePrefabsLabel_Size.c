[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityLabel, "m_Label")]
class EditablePrefabsLabel_Size : EditablePrefabsLabel_Base
{
	[Attribute()]
	protected float sizeXS;

	[Attribute()]
	protected float sizeS;

	[Attribute()]
	protected float sizeM;

	[Attribute()]
	protected float sizeL;

	[Attribute()]
	protected float sizeXL;

	[Attribute()]
	protected bool m_IgnoreY;

	[Attribute()]
	protected bool m_DebugPrintSize;

	//------------------------------------------------------------------------------------------------
	override bool GetLabelValid(WorldEditorAPI api, IEntitySource entitySource, IEntityComponentSource componentSource, string targetPath, EEditableEntityType entityType, notnull array<EEditableEntityLabel> authoredLabels, out EEditableEntityLabel label)
	{
		// Skip groups & authored compositions (SLOTSIZE label)
		if (entityType == EEditableEntityType.GROUP || ContainsSlotSizeLabel(authoredLabels))
			return false;

		ResourceName prefab = entitySource.GetResourceName();

		vector prefabBoundsMin = vector.One * float.MAX;
		vector prefabBoundsMax = vector.One * -float.MAX;

		GetEntitySourceBounds(api, entitySource, prefabBoundsMin, prefabBoundsMax);

		if (m_IgnoreY)
		{
			prefabBoundsMin[1] = 0;
			prefabBoundsMax[1] = 0;
		}

		vector prefabBounds = prefabBoundsMax - prefabBoundsMin;

		float prefabSize = prefabBounds.Length();

		if (m_DebugPrintSize)
			Print(string.Format("%1", prefabSize), LogLevel.DEBUG);

		if (prefabSize == 0)
		{
			Print("Calculated editable entity size is 0", LogLevel.NORMAL);
			return false;
		}

		if (prefabSize >= sizeXL)
			label = EEditableEntityLabel.SIZE_XL;
		else if (prefabSize >= sizeL)
			label = EEditableEntityLabel.SIZE_L;
		else if (prefabSize >= sizeM)
			label = EEditableEntityLabel.SIZE_M;
		else if (prefabSize >= sizeS)
			label = EEditableEntityLabel.SIZE_S;
		else
			label = EEditableEntityLabel.SIZE_XS;

		if (ContainsSizeLabel(authoredLabels))
		{
			// Authored labels contains size label and differs from calculated label, show warning
			if (!authoredLabels.Contains(label))
				Print("Calculated size " + typename.EnumToString(EEditableEntityLabel, label) + " differs from authored size label", LogLevel.WARNING);

			// Dont add calculated label if authored size label exists.
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool ContainsSlotSizeLabel(notnull array<EEditableEntityLabel> authoredLabels, EEditableEntityLabel optSizeLabelCheck = EEditableEntityLabel.NONE)
	{
		// AuthoredLabels contains any SlotSize label
		return authoredLabels.Contains(EEditableEntityLabel.SLOT_FLAT_SMALL) ||
			authoredLabels.Contains(EEditableEntityLabel.SLOT_FLAT_MEDIUM) ||
			authoredLabels.Contains(EEditableEntityLabel.SLOT_FLAT_LARGE) ||
			authoredLabels.Contains(EEditableEntityLabel.SLOT_ROAD_SMALL) ||
			authoredLabels.Contains(EEditableEntityLabel.SLOT_ROAD_MEDIUM) ||
			authoredLabels.Contains(EEditableEntityLabel.SLOT_ROAD_LARGE);
	}

	//------------------------------------------------------------------------------------------------
	protected bool ContainsSizeLabel(notnull array<EEditableEntityLabel> authoredLabels, EEditableEntityLabel optSizeLabelCheck = EEditableEntityLabel.NONE)
	{
		// AuthoredLabels contains any size label
		return authoredLabels.Contains(EEditableEntityLabel.SIZE_XL) ||
			authoredLabels.Contains(EEditableEntityLabel.SIZE_L) ||
			authoredLabels.Contains(EEditableEntityLabel.SIZE_M) ||
			authoredLabels.Contains(EEditableEntityLabel.SIZE_S) ||
			authoredLabels.Contains(EEditableEntityLabel.SIZE_XS);
	}

	//------------------------------------------------------------------------------------------------
	protected string GetFileName(ResourceName prefab)
	{
		string name = FilePath.StripPath(prefab);
		return FilePath.StripExtension(name);
	}

	//------------------------------------------------------------------------------------------------
	protected void GetEntitySourceBounds(WorldEditorAPI api, IEntitySource source, out vector boundsMin, out vector boundsMax)
	{
		IEntity entity = api.SourceToEntity(source);
		if (!entity)
			return;

		VObject object = entity.GetVObject();
		if (object)
		{
			string ext;
			FilePath.StripExtension(object.GetResourceName(), ext);
			if (ext == "xob")
			{
				vector entityMin, entityMax;
				//Print("EntityPos check: " + entity.GetWorldTransformAxis(3));
				entity.GetWorldBounds(entityMin, entityMax);
				//entityMin = entity.CoordToParent(entityMin);
				//entityMax = entity.CoordToParent(entityMax);

				boundsMin[0] = Math.Min(boundsMin[0], entityMin[0]);
				boundsMin[1] = Math.Min(boundsMin[1], entityMin[1]);
				boundsMin[2] = Math.Min(boundsMin[2], entityMin[2]);

				boundsMax[0] = Math.Max(boundsMax[0], entityMax[0]);
				boundsMax[1] = Math.Max(boundsMax[1], entityMax[1]);
				boundsMax[2] = Math.Max(boundsMax[2], entityMax[2]);
			}
		}

		//Print(string.Format("BoundsMin: %1, BoundsMax: %2", boundsMin, boundsMax), LogLevel.NORMAL);

		int childrenCount = source.GetNumChildren();
		for (int i = 0; i < childrenCount; i++)
		{
			GetEntitySourceBounds(api, source.GetChild(i), boundsMin, boundsMax);
		}
	}
}
