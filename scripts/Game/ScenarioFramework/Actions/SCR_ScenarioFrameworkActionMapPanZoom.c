[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionMapPanZoom : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Position to pan to")]
	protected ref SCR_ScenarioFrameworkGet m_PositionGetter;
	
	[Attribute(defvalue: "0", desc: "Target Pix Per Unit", params: "0 inf 0.01")]
	float m_fTargetPixPerUnit;
	
	[Attribute(defvalue: "0.25", desc: "Zoom Time", params: "0 inf 0.01")]
	float m_fSmoothZoomTime;
	
	[Attribute(defvalue: "1", desc: "If true, invoker will be used to perform panZoom even if map is closed. Otherwise, nothing will happen.")]
	bool m_bOnMapOpen;
	
	protected vector m_vPosition;
	
	//------------------------------------------------------------------------------------------------
	void SetPosition(vector position)
	{
		m_vPosition = position;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;
		
		SCR_MapEntity mapEntity = SCR_MapEntity.GetMapInstance();
		if (!mapEntity)
			return;
		
		IEntity entity;
		if (!ValidateInputEntity(object, m_PositionGetter, entity))
			return;
		
		m_vPosition = entity.GetOrigin();
		
		if (mapEntity.IsOpen())
		{
			mapEntity.ZoomPanSmooth(m_fTargetPixPerUnit, m_vPosition[0], m_vPosition[1], m_fSmoothZoomTime);
			return;
		}
		
		if (!m_bOnMapOpen)
			return;
		
		mapEntity.GetOnMapOpen().Insert(OnMapOpen);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnMapOpen(MapConfiguration config)
	{
		SCR_MapEntity mapEntity = SCR_MapEntity.GetMapInstance();
		if (!mapEntity)
			return;
		
		mapEntity.GetOnMapOpen().Remove(OnMapOpen);
		mapEntity.ZoomPanSmooth(m_fTargetPixPerUnit, m_vPosition[0], m_vPosition[1], m_fSmoothZoomTime);
	}
}