//------------------------------------------------------------------------------------------------
//! Base class for nametag entity data collection
[BaseContainerProps()]
class SCR_NameTagCollectorBase : Managed
{
	[Attribute("3", UIWidgets.CheckBox, desc: "seconds \n how often are close entities gathered using SphereQuery", "1 100", category: "NameTagsSettings")]
	protected float m_fGatherEntitiesTimer;
		
	protected float m_fQueryCooldown = 0;			// throttle to gather new entities through query	
	protected SCR_NameTagConfig m_Config;
	protected SCR_NameTagData m_ControlledTag;
	protected SCR_NameTagDisplay m_Display;
	protected TagSystem m_TagSystem;
	
	protected ref array<IEntity> m_aEntities = {};
	
	//------------------------------------------------------------------------------------------------
	//! Determine whether to update entities
	//! \param timeSlice is the OnFrame timeslice
	//! \param controlledTag is the nametag data struct of the controlled entity
	//! \return true if update was performed
	bool UpdatePerformed(float timeSlice, SCR_NameTagData controlledTag)
	{
		m_ControlledTag = controlledTag;
		m_fQueryCooldown += timeSlice;
		
		if (m_fQueryCooldown >= m_fGatherEntitiesTimer)
		{
			m_aEntities.Clear();
			m_fQueryCooldown = 0;
			RefreshEntities();
			
			return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get viable entities
	//! \return array of viable entities
	array<IEntity> GetEntities()
	{						
		return m_aEntities;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Refresh entity list 
	protected void RefreshEntities()
	{
		if (m_TagSystem)
			m_TagSystem.GetTagsInRange(m_aEntities, m_ControlledTag.m_Entity.GetOrigin(), m_Config.m_fFarthestZoneRange, ETagCategory.NameTag);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Init
	//! \param cfg is main nametag config
	void Init(SCR_NameTagConfig cfg, SCR_NameTagDisplay display)
	{
		m_Config = cfg;
		m_Display = display;
		
		ChimeraWorld world = ChimeraWorld.CastFrom(GetGame().GetWorld());
		if (world)
			m_TagSystem = TagSystem.Cast(world.FindSystem(TagSystem));
	}
};
