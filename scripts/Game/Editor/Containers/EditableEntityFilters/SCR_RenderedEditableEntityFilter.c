[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityState, "m_State")]
/** @ingroup Editor_Components_Entities
*/
/*!
@example
*/
class SCR_RenderedEditableEntityFilter : SCR_BaseEditableEntityFilter
{
	[Attribute(defvalue: "15")]
	protected float m_HeightMin;
	
	[Attribute(defvalue: "150")]
	protected float m_HeightMax;
	
	[Attribute(defvalue: "0.1")]
	protected float m_CoefMin;
	
	[Attribute(defvalue: "1")]
	protected float m_CoefMax;
	
	[Attribute(defvalue: "0.5", uiwidget: UIWidgets.Auto, desc: "How long it takes to update all entities (they are processed asynchronously to save performance)")]
	protected float m_fRefreshDuration;
	
	[Attribute(defvalue: "250", uiwidget: UIWidgets.Auto, desc: "Refresh all entities in one frame when the camera instantly moves further than this distance.")]
	protected float m_fTeleportDistance;

	protected BaseWorld m_World;
	protected vector m_vCameraPos;
	protected float m_fCameraDisCoef = 1;
	protected float m_fDelay;
	protected SCR_LayersEditorComponent m_LayersManager;
	
	protected ref set<SCR_EditableEntityComponent> m_aCacheEntities;
	protected float m_iBatchSize;
	protected int m_iBatchCount;
	protected float m_iBatchIndex;
	protected bool m_bInstantRefresh;
	
	float GetCameraDisCoef()
	{
		return m_fCameraDisCoef;
	}
	
	protected void CacheCameraPos()
	{
		vector cameraMatrix[4];
		m_World.GetCurrentCamera(cameraMatrix);
		m_bInstantRefresh = vector.DistanceSq(m_vCameraPos, cameraMatrix[3]) > m_fTeleportDistance;
		m_vCameraPos = cameraMatrix[3];
		
		float heightATL = m_vCameraPos[1] - Math.Max(m_World.GetSurfaceY(m_vCameraPos[0], m_vCameraPos[2]), 0);
		m_fCameraDisCoef = Math.Clamp(Math.InverseLerp(m_HeightMin, m_HeightMax, heightATL), 0, 1);
		m_fCameraDisCoef = Math.Lerp(m_CoefMin, m_CoefMax, m_fCameraDisCoef);
	}
	protected void OnEntityRefreshed(SCR_EditableEntityComponent entity)
	{
		Add(entity, true);
	}
	
	override void EOnEditorFrame(float timeSlice)
	{
		if (!m_World || !GetPredecessor() || !m_aCacheEntities) return;
		
		CacheCameraPos();
		
		if (m_bInstantRefresh)
		{
			//--- Refresh instantly when the camera teleports
			m_iBatchCount = 1;
			m_iBatchIndex = 0;
			m_iBatchSize = GetPredecessor().GetEntities(m_aCacheEntities);
		}
		else if (m_iBatchIndex == m_iBatchCount)
		{
			//--- Refresh continuously to save performance
			m_iBatchCount = m_fRefreshDuration / timeSlice;
			m_iBatchIndex = 0;
			m_iBatchSize = GetPredecessor().GetEntities(m_aCacheEntities) / m_iBatchCount;
		}
		
		//--- Update batch of all entities
		SCR_EditableEntityComponent entity;
		for (int i = m_iBatchIndex * m_iBatchSize; i < (m_iBatchIndex + 1) * m_iBatchSize; i++)
		{
			entity = m_aCacheEntities[i];
			if (!entity) continue;

			if (IsNear(entity))
			{
				if (!entity.HasEntityState(GetState()))
					Add(entity, true);
			}
			else
			{
				if (entity.HasEntityState(GetState()))
					Remove(entity, true);
			}
		}

		m_iBatchIndex++;
	}
	bool IsNear(SCR_EditableEntityComponent entity)
	{
		//--- When in a layer, always show direct layer children
		if (m_LayersManager && m_LayersManager.GetCurrentLayer() && m_LayersManager.GetCurrentLayer() == entity.GetParentEntity())
			return true;

		//--- Show only nearby entities
		vector worldPos;
		return entity.GetPos(worldPos) && vector.DistanceSq(worldPos, m_vCameraPos) < entity.GetMaxDrawDistanceSq() * m_fCameraDisCoef;
	}

	override void EOnEditorActivate()
	{
		m_aCacheEntities = new set<SCR_EditableEntityComponent>;
		m_iBatchCount = 0;
		m_iBatchIndex = 0;
		m_World = GetGame().GetWorld();
		CacheCameraPos();
		
		m_LayersManager = SCR_LayersEditorComponent.Cast(SCR_LayersEditorComponent.GetInstance(SCR_LayersEditorComponent));
		
		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (core)
			core.Event_OnEntityRefreshed.Insert(OnEntityRefreshed);
	}
	override void EOnEditorDeactivate()
	{
		m_aCacheEntities = null;
		
		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (core)
			core.Event_OnEntityRefreshed.Remove(OnEntityRefreshed);
	}
	override bool CanAdd(SCR_EditableEntityComponent entity)
	{
		return IsNear(entity);
	}
	
	void SCR_RenderedEditableEntityFilter()
	{
		m_fTeleportDistance = m_fTeleportDistance * m_fTeleportDistance;
	}
};