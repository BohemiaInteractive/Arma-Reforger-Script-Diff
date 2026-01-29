[ComponentEditorProps(category: "GameScripted/Editor", description: "Entity transformation (moving and rotating). Works only with SCR_EditorBaseEntity!", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_CampaignBuildingTransformingEditorComponentClass : SCR_TransformingEditorComponentClass
{

}

/** @ingroup Editor_Components
*/

class SCR_CampaignBuildingTransformingEditorComponent : SCR_TransformingEditorComponent
{
	protected SCR_EditableEntityComponent m_LastRejectedPivot;
	protected SCR_CampaignBuildingPlacingObstructionEditorComponent m_ObstructionComponent;
	protected SCR_CampaignBuildingPlacingHQBaseEditorComponent m_HQBaseEditorComponent;
	protected SCR_EditablePreviewEntity m_PreviewEntity;
	protected SCR_PreviewEntityEditorComponent m_PreviewEntityEditorComponent;

	//------------------------------------------------------------------------------------------------
	void OnPreviewCreated(SCR_EditablePreviewEntity previewEnt)
	{
		m_PreviewEntity = previewEnt;
		m_ObstructionComponent.OnPreviewCreated(m_PreviewEntity);
		m_PreviewManager.GetOnPreviewCreate().Remove(OnPreviewCreated);
	}

	//------------------------------------------------------------------------------------------------
	void OnPreviewDeleted()
	{
		m_PreviewManager.GetOnPreviewDelete().Remove(OnPreviewDeleted);
	}
	//------------------------------------------------------------------------------------------------
	override void StartEditing(SCR_EditableEntityComponent pivot, notnull set<SCR_EditableEntityComponent> entities, vector transform[4])
	{
		if (!pivot || IsEditing(pivot) || m_LastRejectedPivot == pivot)
			return;

		SCR_CampaignBuildingCompositionComponent compositionComponent = SCR_CampaignBuildingCompositionComponent.Cast(pivot.GetOwner().FindComponent(SCR_CampaignBuildingCompositionComponent));
		if (!compositionComponent)
			return;

		if (compositionComponent.IsInteractionLocked())
		{
			SCR_NotificationsComponent.SendLocal(ENotification.EDITOR_COMPOSITION_UNDER_CONSTRUCTION);
			m_LastRejectedPivot = pivot;
			return;
		}

		m_LastRejectedPivot = null;

		m_PreviewManager.GetOnPreviewCreate().Insert(OnPreviewCreated);
		m_PreviewManager.GetOnPreviewDelete().Insert(OnPreviewDeleted);

		super.StartEditing(pivot, entities, transform);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	override void StartEditingServer(int pivotId, array<int> entityIds, vector dummyVector, vector transform[4], bool isUnderwater, EEditorTransformVertical verticalMode)
	{
		super.StartEditingServer(pivotId, entityIds, dummyVector, transform, isUnderwater, verticalMode);

		IEntity child = m_EditedPivot.GetOwner().GetChildren();
		SCR_CampaignBuildingLayoutComponent layoutComponent;

		while (child && !layoutComponent)
		{
			layoutComponent = SCR_CampaignBuildingLayoutComponent.Cast(child.FindComponent(SCR_CampaignBuildingLayoutComponent));

			if (!layoutComponent)
				child = child.GetSibling();
		}

		if (layoutComponent)
		{
			layoutComponent.LockCompositionInteraction();
			layoutComponent.DeletePreview();
			SCR_EntityHelper.DeleteEntityAndChildren(layoutComponent.GetOwner());
		}
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	override void StartEditingOwner()
	{
		if (!m_PreviewManager)
			return;

		//--- Transforming was meanwhile canceled, ignore
		if (!m_StatesManager.SetIsWaiting(false) || m_StatesManager.GetState() != EEditorState.TRANSFORMING)
		{
			CancelEditing();
			return;
		}

		IEntity owner = m_EditedPivot.GetOwner();
		if (!owner)
			return;

		vector transform[4];
		owner.GetWorldTransform(transform);

		array<vector> offsets = {};
		ResourceName resName = owner.GetPrefabData().GetPrefabName();
		//--- Create preview
		SCR_BasePreviewEntity previewEntity = m_PreviewManager.CreatePreview(resName, offsets, transform);

		Event_OnTransformationStart.Invoke(m_aEditedEntities);

		SCR_TransformingEditorComponentClass prefabData = SCR_TransformingEditorComponentClass.Cast(GetEditorComponentData());
		if (prefabData)
			SCR_BaseEditorEffect.Activate(prefabData.GetEffectsTransforationStart(), this, entities: m_aEditedEntities);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnEditorActivate()
	{
		super.EOnEditorActivate();

		m_PreviewEntityEditorComponent = SCR_PreviewEntityEditorComponent.Cast(FindEditorComponent(SCR_PreviewEntityEditorComponent, true, true));

		m_ObstructionComponent = SCR_CampaignBuildingPlacingObstructionEditorComponent.Cast(FindEditorComponent(SCR_CampaignBuildingPlacingObstructionEditorComponent, true, true));
		m_HQBaseEditorComponent = SCR_CampaignBuildingPlacingHQBaseEditorComponent.Cast(FindEditorComponent(SCR_CampaignBuildingPlacingHQBaseEditorComponent, true, true));
	}

	//------------------------------------------------------------------------------------------------
	//! Test if the composition is moved out of the building mode and if so, deny the transfer.
	bool IsInBuildingRadiusVector(vector previewOrigin)
	{
		SCR_CampaignBuildingEditorComponent editorComponent = SCR_CampaignBuildingEditorComponent.Cast(SCR_CampaignBuildingEditorComponent.GetInstance(SCR_CampaignBuildingEditorComponent, true));
		if (!editorComponent)
			return false;

		SCR_CampaignBuildingProviderComponent providerComponent = editorComponent.GetProviderComponent();
		if (!providerComponent)
			return false;

		if (vector.Distance(previewOrigin, providerComponent.GetOwner().GetOrigin()) > providerComponent.GetBuildingRadius())
			return false;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool ConfirmEditing()
	{
		vector transform[4];
		m_PreviewManager.GetPreviewTransform(transform);

		if (!m_ObstructionComponent.CanCreate() || !IsInBuildingRadiusVector(transform[3]) || m_HQBaseEditorComponent && !m_HQBaseEditorComponent.CanCreate())
		{
			CancelEditing();
			return false;
		}

		super.ConfirmEditing();

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! The variable has to be set to null if the editing was blocked because of the temporary move of the composition by another player
	void ReleaseLastRejectedPivot()
	{
		m_LastRejectedPivot = null;
	}
	
	SCR_EditableEntityComponent GetEditedPivot()
	{
		return m_EditedPivot;
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	override void ConfirmEditingServer(SCR_EditorPreviewParams params, int playerID)
	{
		IEntity composition = m_EditedPivot.GetOwner();

		super.ConfirmEditingServer(params, playerID);

		SCR_CampaignBuildingCompositionComponent compositionComponent = SCR_CampaignBuildingCompositionComponent.Cast(composition.FindComponent(SCR_CampaignBuildingCompositionComponent));
		if (!compositionComponent)
			return;
		
		// m_EditedPivot can't be used as it's set to null after super. is called. However this code can't be call before.
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(composition.FindComponent(SCR_EditableEntityComponent));
		if (!editableEntity)
			return;

		compositionComponent.SpawnCompositionLayout(compositionComponent.GetPrefabId(), editableEntity);
		compositionComponent.SetInteractionLockServer(false);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	override void CancelEditingServer()
	{
		SCR_CampaignBuildingLayoutComponent layoutComponent;

		if (m_EditedPivot)
		{
			IEntity composition = m_EditedPivot.GetOwner();
			SCR_CampaignBuildingCompositionComponent compositionComponent = SCR_CampaignBuildingCompositionComponent.Cast(composition.FindComponent(SCR_CampaignBuildingCompositionComponent));
			if (!compositionComponent)
				return;

			compositionComponent.SpawnCompositionLayout(compositionComponent.GetPrefabId(), m_EditedPivot);
			compositionComponent.SetInteractionLockServer(false);
		}

		super.CancelEditingServer();
	}
}
