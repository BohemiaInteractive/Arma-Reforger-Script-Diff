[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityState, "m_State")]
class SCR_FriendlyPlayerEditableEntityFilter : SCR_PlayerEditableEntityFilter
{
	protected Faction m_Faction;
	protected BaseWorld m_World;
	protected vector m_vCameraPos;
	
	protected bool m_bIsBuildMode;
	protected vector m_vBuildPos;
	protected float m_fBuildRadiusSq;
	
	protected float m_ElapsedTime;
	protected float m_fCameraDisCoef = 1;
	
	protected const float REFRESH_DURATION = 0.5;
	
	protected const int HEIGHT_MIN = 15;
	protected const int HEIGHT_MAX = 150;
	
	protected const float COEF_MIN = 0.1;
	protected const float COEF_MAX = 1;
	
	protected const int CAMERA_VIEW_DISTANCE = 500;
	protected int m_iCameraViewDistSq;

	//------------------------------------------------------------------------------------------------
	override bool CanAdd(SCR_EditableEntityComponent entity)
	{
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(entity.GetOwner());
		if (!character)
			return false;
		
		if (!super.CanAdd(entity))
			return false;
		
		if (!m_Faction)
			return false;
		
		if (!IsNear(entity))
			return false;

		return m_Faction.IsFactionFriendly(character.GetFaction());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CacheCameraPos()
	{
		vector cameraMatrix[4];
		m_World.GetCurrentCamera(cameraMatrix);
		m_vCameraPos = cameraMatrix[3];
		
		float heightATL = m_vCameraPos[1] - Math.Max(m_World.GetSurfaceY(m_vCameraPos[0], m_vCameraPos[2]), 0);
		m_fCameraDisCoef = Math.Clamp(Math.InverseLerp(HEIGHT_MIN, HEIGHT_MAX, heightATL), 0, 1);
		m_fCameraDisCoef = Math.Lerp(COEF_MIN, COEF_MAX, m_fCameraDisCoef);
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool IsNear(SCR_EditableEntityComponent entity)
	{
		vector worldPos;
		if (!entity.GetPos(worldPos))
			return false;
		
		if (m_bIsBuildMode)
			return vector.DistanceSq(worldPos, m_vBuildPos) < m_fBuildRadiusSq;
		else
			return vector.DistanceSq(worldPos, m_vCameraPos) < m_iCameraViewDistSq;
	}
	
	//------------------------------------------------------------------------------------------------
	protected SCR_EditorManagerEntity GetEditorManagerEntity()
	{
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core)
			return null;

		return core.GetEditorManager();
	}

	//------------------------------------------------------------------------------------------------
	override void EOnEditorActivate()
	{
		super.EOnEditorActivate();
		
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		if (!character)
			return;

		m_World = GetGame().GetWorld();
		m_Faction = character.GetFaction();
		m_iCameraViewDistSq = CAMERA_VIEW_DISTANCE * CAMERA_VIEW_DISTANCE;
		CacheCameraPos();
		
		SCR_EditorManagerEntity editorManager = GetEditorManagerEntity();
		if (!editorManager)
			return;

		SCR_EditorModeEntity modeEntity = editorManager.FindModeEntity(EEditorMode.BUILDING);
		if (!modeEntity)
			return;
		
		SCR_CampaignBuildingEditorComponent campaignBuildingComponent = SCR_CampaignBuildingEditorComponent.Cast(modeEntity.FindComponent(SCR_CampaignBuildingEditorComponent));
		if (!campaignBuildingComponent)
			return;
			
		ScriptedGameTriggerEntity zone = campaignBuildingComponent.GetTrigger();
		if (!zone)
			return;
		
		m_bIsBuildMode = true;
		m_fBuildRadiusSq = zone.GetSphereRadius() * zone.GetSphereRadius();
		
		vector mat[4];
		zone.GetWorldTransform(mat);
		m_vBuildPos = mat[3];
	}
	
	override void EOnEditorFrame(float timeSlice)
	{
		if (!m_World || !m_Manager)
			 return;
		
		m_ElapsedTime += timeSlice;
		CacheCameraPos();
		
		if (m_ElapsedTime > REFRESH_DURATION)
		{
			m_ElapsedTime = 0;
			
			set<SCR_EditableEntityComponent> entities = new set<SCR_EditableEntityComponent>;
			GetCore().GetAllEntities(entities);
			foreach(SCR_EditableEntityComponent entity: entities)
			{
				if (!entity.HasEntityState(GetState()))
				{
					if (CanAdd(entity))
						Add(entity, true);
				}
				else if (!IsNear(entity))
				{
					Remove(entity, true);
				}
			}
		}
	}
}