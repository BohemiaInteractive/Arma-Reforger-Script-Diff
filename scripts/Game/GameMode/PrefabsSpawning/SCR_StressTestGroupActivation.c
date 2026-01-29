class SCR_StressTestGroupActivationClass : GenericEntityClass
{
}

class SCR_StressTestGroupActivation : GenericEntity
{
	[Attribute( defvalue: "1.0", uiwidget: UIWidgets.Slider, desc: "Activate groups every [s]", params: "0 5 0.1" )]
	float m_fNextGroupTime;
	
	ref private array<IEntity> m_entities = {};
	private int m_iIndex;
	private float m_fTimeElapsed;	
	
	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] parent
	void SCR_StressTestGroupActivation(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT | EntityEvent.FRAME);
		SetFlags(EntityFlags.ACTIVE, false);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		BaseWorld world = GetGame().GetWorld();
		if (world)
			world.GetActiveEntities(m_entities);
		m_iIndex = 0;
		m_fTimeElapsed = 0;		
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (m_fTimeElapsed > m_fNextGroupTime)
		{
			for (int i = m_iIndex, length = m_entities.Count(); i < length; i++)
			{
				SCR_AIGroup group = SCR_AIGroup.Cast(m_entities[i]);
				if (group)
				{
					if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_PRINT_GROUP_INFO))
						PrintFormat("Group %1 activated at location %2",group,group.GetOrigin());

					group.SpawnUnits();
					m_iIndex = i + 1;
					m_fTimeElapsed = 0;
					break;
				}
				if (i == m_entities.Count() - 1)
				{
					ClearEventMask(EntityEvent.FRAME);
					if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_PRINT_GROUP_INFO))
						Print("SCR_StressTestGroup: activation of groups completed");
				}
			}
		}

		m_fTimeElapsed += timeSlice;		
	}
}
