[EntityEditorProps(category: "GameScripted/ZoneRestriction", description: "Editor restriction zone to restrict players from leaving an specific area", color: "255 0 0 255")]
class SCR_EditorRestrictionZoneEntityClass: GenericEntityClass
{
};
class SCR_EditorRestrictionZoneEntity: GenericEntity
{
	[Attribute("6", desc: "Will warn the player for going the given distance from the zone center")]
	protected float m_fWarnRadius;
	
	[Attribute("12", desc: "Will kill the player for going the given distance from the zone center. Needs to be equal or higher then m_fWarnRadius")]
	protected float m_fZoneRadius;
	
	[Attribute("18", desc: "The distance when a player is concidered teleported. The player will not die if moved out of the zone by the given distance")]
	protected float m_fZoneTeleportedRadius;
	
	[Attribute("1", desc: "Which warning UIinfo needs to be shown on the screen as defined in 'SCR_RestrictionZoneWarningHUDComponent'", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(ERestrictionZoneWarningType))]
	protected ERestrictionZoneWarningType m_iWarningType;
	
	protected float m_fWarnRadiusSq;
	protected float m_fZoneRadiusSq;
	protected float m_fZoneTeleportedRadiusSq;

	/*!
	Get the radius of the Restriction circle. 
	\return float radius
	*/
	float GetRestrictionZoneRadius()
	{
		return m_fZoneRadius;
	}
	
	/*!
	Get the radius of the Restriction circle squared. 
	\return float radius squared
	*/
	float GetRestrictionZoneRadiusSq()
	{
		return m_fZoneRadiusSq;
	}
	
	/*!
	Get the radius of the warning circle. 
	\return float warning radius
	*/
	float GetWarningZoneRadius()
	{
		return m_fWarnRadius;
	}
	
	/*!
	Get the radius of the warning circle squared. 
	\return float warning radius squared
	*/
	float GetWarningZoneRadiusSq()
	{
		return m_fWarnRadiusSq;
	}
	
	/*!
	Get the radius of the teleported circle squared. 
	\return float teleported radius squared
	*/
	float GetTeleportedZoneRadiusSq()
	{
		return m_fZoneTeleportedRadiusSq;
	}
	
	/*!
	Get worning type to be displayed on UI
	\return Warning type enum
	*/
	ERestrictionZoneWarningType GetWarningType()
	{
		return m_iWarningType;
	}
	
	override protected void EOnInit(IEntity owner)
	{		
		BaseGameMode gamemode = GetGame().GetGameMode();
		
		if (!gamemode)
			return;
		
		SCR_PlayersRestrictionZoneManagerComponent restrictionZoneManager = SCR_PlayersRestrictionZoneManagerComponent.Cast(gamemode.FindComponent(SCR_PlayersRestrictionZoneManagerComponent));
		
		if (!restrictionZoneManager)
			return;
		
		if (m_fZoneRadius <= 0)
			m_fZoneRadius = 10;
		
		if (m_fWarnRadius > m_fZoneRadius)
			m_fWarnRadius = m_fZoneRadius;
		
		m_fZoneRadiusSq = Math.Pow(m_fZoneRadius, 2);
		m_fWarnRadiusSq = Math.Pow(m_fWarnRadius, 2);
		m_fZoneTeleportedRadiusSq = Math.Pow(m_fZoneTeleportedRadius, 2);
		
		restrictionZoneManager.AddRestrictionZone(this);
	}

	void SCR_EditorRestrictionZoneEntity(IEntitySource src, IEntity parent)
	{
		if (!Replication.IsServer() || SCR_Global.IsEditMode(this))
			return;
		
		SetEventMask(EntityEvent.INIT);
	}
	
	void ~SCR_EditorRestrictionZoneEntity()
	{
		if (!Replication.IsServer() || SCR_Global.IsEditMode(this))
			return;
		
		BaseGameMode gamemode = GetGame().GetGameMode();
		
		if (!gamemode)
			return;
		
		SCR_PlayersRestrictionZoneManagerComponent restrictionZoneManager = SCR_PlayersRestrictionZoneManagerComponent.Cast(gamemode.FindComponent(SCR_PlayersRestrictionZoneManagerComponent));
		
		if (!restrictionZoneManager)
			return;
		
		restrictionZoneManager.RemoveRestrictionZone(this);
	}
};
