[ComponentEditorProps(category: "GameScripted/Editor", description: "Placing HQ base. Disallow placement of HQ composition in defined cases.", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_CampaignBuildingPlacingHQBaseEditorComponentClass : SCR_BaseEditorComponentClass
{
}

class SCR_CampaignBuildingPlacingHQBaseEditorComponent : SCR_BaseEditorComponent
{
	protected SCR_EditablePreviewEntity m_PreviewEntity;
	protected SCR_CampaignBuildingManagerComponent m_BuildingManagerComponent;
	protected SCR_GameModeCampaign m_Campaign;
	protected SCR_PreviewEntityEditorComponent m_PreviewManager;
	protected SCR_CampaignBuildingTransformingEditorComponent m_TransformingComponent;
	protected SCR_CampaignFactionCommanderHandlerComponent m_FactionCommanderHandler;

	protected bool m_bIsSelectedPrefabHQ;
	protected int m_iMinDistanceFromHQSq;
	protected int m_iBaseEstablishingRadius;
	protected Faction m_Faction;

	//------------------------------------------------------------------------------------------------
	override protected void EOnEditorOpen()
	{
		m_Campaign = SCR_GameModeCampaign.GetInstance();
		if (!m_Campaign || !m_Campaign.GetEstablishingBasesEnabled())
			return;

		m_BuildingManagerComponent = SCR_CampaignBuildingManagerComponent.Cast(m_Campaign.FindComponent(SCR_CampaignBuildingManagerComponent));
		if (!m_BuildingManagerComponent)
			return;

		m_FactionCommanderHandler = SCR_CampaignFactionCommanderHandlerComponent.Cast(SCR_FactionCommanderHandlerComponent.GetInstance());
		if (!m_FactionCommanderHandler)
			return;

		int baseMinDistance = m_FactionCommanderHandler.GetBaseMinDistance(SCR_ECampaignBaseType.BASE);
		m_iBaseEstablishingRadius = m_FactionCommanderHandler.GetBaseEstablishingRadius();
		int minDistanceFromHQ = baseMinDistance - m_iBaseEstablishingRadius;
		if (minDistanceFromHQ < 0)
			minDistanceFromHQ = 0;

		m_iMinDistanceFromHQSq = minDistanceFromHQ * minDistanceFromHQ;

		m_PreviewManager = SCR_PreviewEntityEditorComponent.Cast(FindEditorComponent(SCR_PreviewEntityEditorComponent, true, true));
		if (!m_PreviewManager)
			return;

		m_TransformingComponent = SCR_CampaignBuildingTransformingEditorComponent.Cast(FindEditorComponent(SCR_CampaignBuildingTransformingEditorComponent, true, true));
		m_Faction = SCR_FactionManager.SGetLocalPlayerFaction();
		m_PreviewManager.GetOnPreviewCreate().Insert(OnPreviewCreated);
	}

	//------------------------------------------------------------------------------------------------
	override protected void EOnEditorClose()
	{
		if (m_PreviewManager)
			m_PreviewManager.GetOnPreviewCreate().Remove(OnPreviewCreated);
	}

	//------------------------------------------------------------------------------------------------
	//! Method called once the preview is spawned to initiate all the variables
	void OnPreviewCreated(notnull SCR_EditablePreviewEntity previewEntity)
	{
		m_PreviewEntity = previewEntity;
		m_bIsSelectedPrefabHQ = IsSelectedPrefabHQ();
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsSelectedPrefabHQ()
	{
		ResourceName resName = m_PreviewManager.GetPreviewPrefab();
		Resource entityPrefab = Resource.Load(resName);
		if (!entityPrefab.IsValid())
			return false;

		IEntitySource entitySource = SCR_BaseContainerTools.FindEntitySource(entityPrefab);
		if (!entitySource)
			return false;

		IEntityComponentSource entityComponentSource = SCR_BaseContainerTools.FindComponentSource(entitySource, SCR_EditableEntityComponent);
		if (!entityComponentSource)
			return false;

		SCR_EditableEntityUIInfo info = SCR_EditableEntityComponentClass.GetInfo(entityComponentSource);
		if (!info)
			return false;

		array<EEditableEntityLabel> entityLabels = {};
		info.GetEntityLabels(entityLabels);
		return entityLabels.Contains(EEditableEntityLabel.SERVICE_HQ);
	}

	//------------------------------------------------------------------------------------------------
	//! Check the preview position. Is suitable to build the composition here?
	bool CanCreate(out ENotification outNotification = -1, out SCR_EPreviewState previewStateToShow = SCR_EPreviewState.PLACEABLE)
	{
		if (IsNearAnyHQ(outNotification))
		{
			previewStateToShow = SCR_EPreviewState.BLOCKED;
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	// Check if the preview is outside of the establish radius.
	protected bool IsPreviewOutOfTaskRange(out ENotification outNotification = -1)
	{
		float nearestTaskDistance;
		SCR_EstablishBaseTaskEntity task = SCR_EstablishBaseTaskEntity.Cast(SCR_CampaignTaskHelper.GetNearestTask(m_PreviewEntity.GetOrigin(), m_Faction, SCR_EstablishBaseTaskEntity, nearestTaskDistance));

		if (!task || nearestTaskDistance > m_iBaseEstablishingRadius)
		{
			outNotification = ENotification.GROUP_TASK_ESTABLISH_BASE_INVALID_AREA;
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected IEntity GetTransformingEntity()
	{
		if (!m_TransformingComponent)
			return null;

		SCR_EditableEntityComponent editablePivot = m_TransformingComponent.GetEditedPivot();
		if (!editablePivot)
			return null;

		return editablePivot.GetOwner();
	}

	//------------------------------------------------------------------------------------------------
	// Check if the preview is near any HQ composition.
	//! \param[out] outNotification
	//! \return true if is near any HQ
	bool IsNearAnyHQ(out ENotification outNotification = -1)
	{
		if (!m_Campaign || !m_Campaign.GetEstablishingBasesEnabled())
			return false;

		if (!m_PreviewEntity || !m_bIsSelectedPrefabHQ || !m_Faction)
			return false;

		if (IsPreviewOutOfTaskRange(outNotification))
			return true;

		array<SCR_CampaignBuildingCompositionComponent> hqs = m_BuildingManagerComponent.GetHQBuildingCompositions();
		if (!hqs)
			return false;

		IEntity transformingEntity = GetTransformingEntity();

		foreach (SCR_CampaignBuildingCompositionComponent hq : hqs)
		{
			if (!hq || hq.IsCompositionSpawned())
				continue;

			// skip checking for transforming entity
			if (transformingEntity && transformingEntity == hq.GetOwner())
				continue;

			// check placed HQ holograms
			if ((vector.DistanceSqXZ(hq.GetOwner().GetOrigin(), m_PreviewEntity.GetOrigin()) <= m_iMinDistanceFromHQSq))
			{
				outNotification = ENotification.GROUP_TASK_ESTABLISH_BASE_TOO_CLOSE_ANOTHER_BASE;
				return true;
			}
		}

		array<SCR_MilitaryBaseComponent> bases = {};
		SCR_MilitaryBaseSystem.GetInstance().GetBases(bases);

		SCR_CampaignMilitaryBaseComponent campaignBase;
		int minDistanceFromBase;

		foreach (SCR_MilitaryBaseComponent base : bases)
		{
			campaignBase = SCR_CampaignMilitaryBaseComponent.Cast(base);

			if (!campaignBase || !campaignBase.IsInitialized())
				continue;

			minDistanceFromBase = m_FactionCommanderHandler.GetBaseMinDistance(campaignBase) - m_iBaseEstablishingRadius;
			if (minDistanceFromBase < 0)
				minDistanceFromBase = 0;

			if (vector.DistanceSqXZ(base.GetOwner().GetOrigin(), m_PreviewEntity.GetOrigin()) <= (minDistanceFromBase * minDistanceFromBase))
			{
				outNotification = ENotification.GROUP_TASK_ESTABLISH_BASE_TOO_CLOSE_ANOTHER_BASE;
				return true;
			}
		}

		return false;
	}
}
