//! @ingroup Editor_Containers

//! Network packet of variables for entity placing and transformation.
class SCR_EditorPreviewParams
{
	SCR_EditableEntityComponent m_Parent;
	RplId m_ParentID = RplId.Invalid();
	bool m_bParentChanged;
	
	RplId m_TargetRplID = RplId.Invalid();
	EntityID m_TargetStaticID;
	SCR_EditableEntityComponent m_Target;
	EEditableEntityInteraction m_TargetInteraction;
	
	vector m_vTransform[4];
	vector m_Offset;
	bool m_bIsUnderwater;
	EEditorTransformVertical m_VerticalMode;
	EEditorPlacingFlags m_PlacingFlags;
	SCR_EditableEntityComponent m_CurrentLayer; //--- No replicated, used to extract data from SCR_PlacingEditorComponent.SpawnEntityResource()
	
	//------------------------------------------------------------------------------------------------
	static void Encode(SSnapSerializerBase snapshot, ScriptCtx hint, ScriptBitSerializer packet) 
	{
		snapshot.Serialize(packet, 84);
	}

	//------------------------------------------------------------------------------------------------
	static bool Decode(ScriptBitSerializer packet, ScriptCtx hint, SSnapSerializerBase snapshot) 
	{
		return snapshot.Serialize(packet, 84);
	}

	//------------------------------------------------------------------------------------------------
	static bool SnapCompare(SSnapSerializerBase lhs, SSnapSerializerBase rhs, ScriptCtx hint) 
	{
		return lhs.CompareSnapshots(rhs, 84);
	}

	//------------------------------------------------------------------------------------------------
	static bool PropCompare(SCR_EditorPreviewParams prop, SSnapSerializerBase snapshot, ScriptCtx hint) 
	{
		return snapshot.Compare(prop.m_vTransform, 48)
			&& snapshot.Compare(prop.m_ParentID, 4)
			&& snapshot.Compare(prop.m_bParentChanged, 4)
			&& snapshot.Compare(prop.m_VerticalMode, 4)
			&& snapshot.Compare(prop.m_bIsUnderwater, 4)
			&& snapshot.Compare(prop.m_TargetRplID, 4)
			&& snapshot.Compare(prop.m_TargetStaticID, 8)
			&& snapshot.Compare(prop.m_TargetInteraction, 4)
			&& snapshot.Compare(prop.m_PlacingFlags, 4);
	}

	//------------------------------------------------------------------------------------------------
	static bool Extract(SCR_EditorPreviewParams prop, ScriptCtx hint, SSnapSerializerBase snapshot) 
	{
		snapshot.SerializeBytes(prop.m_vTransform, 48);
		snapshot.SerializeBytes(prop.m_ParentID, 4);
		snapshot.SerializeBytes(prop.m_bParentChanged, 4);
		snapshot.SerializeBytes(prop.m_VerticalMode, 4);
		snapshot.SerializeBytes(prop.m_bIsUnderwater, 4);
		snapshot.SerializeBytes(prop.m_TargetRplID, 4);
		snapshot.SerializeBytes(prop.m_TargetStaticID, 8);
		snapshot.SerializeBytes(prop.m_TargetInteraction, 4);
		snapshot.SerializeBytes(prop.m_PlacingFlags, 4);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	static bool Inject(SSnapSerializerBase snapshot, ScriptCtx hint, SCR_EditorPreviewParams prop) 
	{
		snapshot.SerializeBytes(prop.m_vTransform, 48);
		snapshot.SerializeBytes(prop.m_ParentID, 4);
		snapshot.SerializeBytes(prop.m_bParentChanged, 4);
		snapshot.SerializeBytes(prop.m_VerticalMode, 4);
		snapshot.SerializeBytes(prop.m_bIsUnderwater, 4);
		snapshot.SerializeBytes(prop.m_TargetRplID, 4);
		snapshot.SerializeBytes(prop.m_TargetStaticID, 8);
		snapshot.SerializeBytes(prop.m_TargetInteraction, 4);
		snapshot.SerializeBytes(prop.m_PlacingFlags, 4);

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get world transformation matrix. If offset is used, it will be applied to it.
	//! \param[out] outTransform Variable to be filled with transformation matrix
	void GetWorldTransform(out vector outTransform[4])
	{
		if (m_Offset == vector.Zero)
		{
			//--- Exact position
			//outTransform = m_vTransform; //--- Doesn't work, reference is lost
			Math3D.MatrixCopy(m_vTransform, outTransform);
		}
		else
		{
			//--- Offset when multiple entities are being placed (e.g., waypoints for a group)
			vector coefMatrix[4] = {m_vTransform[0], m_vTransform[1], m_vTransform[2], vector.Zero};
			vector offsetMatrix[4] = { vector.Zero, vector.Zero, vector.Zero, m_Offset};
			Math3D.MatrixMultiply4(coefMatrix, offsetMatrix, offsetMatrix);
			outTransform = {m_vTransform[0], m_vTransform[1], m_vTransform[2], m_vTransform[3] + offsetMatrix[3]};
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Convert replication-friendly values to actual variables.
	//! \return True if the deserialization was completed successfully
	bool Deserialize()
	{		
		m_Parent = SCR_EditableEntityComponent.Cast(Replication.FindItem(m_ParentID));
		if (!m_Parent && m_ParentID.IsValid())
		{
			Print(string.Format("Cannot deserialize entity, parent with RplId = %1 not found!", m_ParentID), LogLevel.ERROR);
			return false;
		}
		
		m_Target = SCR_EditableEntityComponent.Cast(Replication.FindItem(m_TargetRplID));
		
		if (!m_Target)
			m_Target = SCR_EditableEntityComponent.GetEditableEntity(GetGame().GetWorld().FindEntityByID(m_TargetStaticID));

		if (!m_Target && m_TargetRplID.IsValid())
		{
			Print(string.Format("Cannot deserialize entity, target neither with RplId = %1 nor with EntityID = %2 found!", m_TargetRplID, m_TargetStaticID), LogLevel.ERROR);
			return false;
		}
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param target
	void SetTarget(SCR_EditableEntityComponent target)
	{
		if (target && target.GetOwner())
		{
			m_TargetRplID = Replication.FindId(target);
			if (!m_TargetRplID.IsValid())
				m_TargetStaticID = target.GetOwner().GetID();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Create params for entity placement.
	//! \param transform Transformation matrix where the entity will be created
	//! \param parentID Replication ID of parent entity
	//! \param verticalMode Alignment rules
	//! \param isUnderwater True if the entity is being placed under water surface
	//! \param targetID Replication ID of entity on/inside which the new entity is being spawned
	//! \param targetInteraction Type of interaction with the target entity
	//! \return Params packet
	static SCR_EditorPreviewParams CreateParams(
		vector transform[4],
		RplId parentID = Replication.INVALID_ID,
		EEditorTransformVertical verticalMode = EEditorTransformVertical.SEA,
		bool isUnderwater = false,
		SCR_EditableEntityComponent target = null,
		EEditableEntityInteraction targetInteraction = EEditableEntityInteraction.NONE
	)
	{
		SCR_EditorPreviewParams params = new SCR_EditorPreviewParams;
		Math3D.MatrixCopy(transform, params.m_vTransform);
		params.m_ParentID = parentID;
		params.m_VerticalMode = verticalMode;
		params.m_bIsUnderwater = isUnderwater;
		params.m_TargetInteraction = targetInteraction;
		params.SetTarget(target);
		return params;
	}

	//------------------------------------------------------------------------------------------------
	//! Create params packet for network transmission.
	//! \param previewManager Manager from  which individual params will be extracted - must not be null
	//! \param parent Currently selected parent layer
	//! \param parentChanged True if current layer changed while editing
	//! \return Params packet or null if an error was encountered
	// TODO: notnull or nullcheck previewManager
	static SCR_EditorPreviewParams CreateParamsFromPreview(SCR_PreviewEntityEditorComponent previewManager, SCR_EditableEntityComponent parent = null, bool parentChanged = false)
	{
		//--- Check if target interaction is valid
		if (previewManager.GetTarget() && previewManager.GetTargetInteraction() == EEditableEntityInteraction.NONE)
		{
			SCR_NotificationsComponent.SendLocal(ENotification.EDITOR_TRANSFORMING_INCORRECT_TARGET);
			return null;
		}
		
		SCR_EditorPreviewParams params = new SCR_EditorPreviewParams;
		
		previewManager.GetPreviewTransform(params.m_vTransform);
		params.m_VerticalMode = previewManager.GetVerticalModeReal();
		params.m_bIsUnderwater = previewManager.IsUnderwater();
		params.m_TargetInteraction = previewManager.GetTargetInteraction();
		params.SetTarget(previewManager.GetTarget());
		params.m_ParentID = Replication.FindId(parent);
		params.m_bParentChanged = parentChanged;
		
		return params;
	}
}
