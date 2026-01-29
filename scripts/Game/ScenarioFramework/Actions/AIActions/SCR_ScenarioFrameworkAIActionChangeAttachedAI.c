[BaseContainerProps(), SCR_ContainerAIActionTitle()]
class SCR_ScenarioFrameworkAIActionChangeAttachedAI : SCR_ScenarioFrameworkAIAction
{
	[Attribute(desc: "New target entity, usually selected by name to be hooked into target SlotAI. It is optional, if left empty, this action will just remove all references from target SlotAI.")];
	ref SCR_ScenarioFrameworkGet m_Getter;
	
	[Attribute(desc: "Name of the group entity that will be set for future reference (Optional)")]
	string m_sOldAIName;
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		super.OnActivate();
		
		SCR_ScenarioFrameworkSlotAI slotAI = SCR_ScenarioFrameworkSlotAI.Cast(m_IEntity.FindComponent(SCR_ScenarioFrameworkSlotAI));
		SCR_ScenarioFrameworkSlotTaskAI slotTaskAI = SCR_ScenarioFrameworkSlotTaskAI.Cast(m_IEntity.FindComponent(SCR_ScenarioFrameworkSlotTaskAI));
		if (slotAI && slotTaskAI)
			return;
		
		if (!m_sOldAIName.IsEmpty())
		{
			if (GetGame().GetWorld().FindEntityByName(m_sOldAIName))
			{
				string IDWithSuffix = m_sOldAIName;
				int suffixNumber;
				while (GetGame().GetWorld().FindEntityByName(IDWithSuffix))
	            {
					suffixNumber++;
					Print(string.Format("ScenarioFramework AIActionChangeAttachedAI: Entity of name %1 was found. The suffix _%2 will be added.", m_sOldAIName, suffixNumber), LogLevel.WARNING);
					IDWithSuffix = m_sOldAIName + "_" + suffixNumber.ToString();
	            }
					
				m_AIGroup.SetName(IDWithSuffix); 
			}
			else
			{
				m_AIGroup.SetName(m_sOldAIName); 
			}
		}
		
		if (m_Getter)
		{
			SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
			if (entityWrapper)
			{
				IEntity entity = entityWrapper.GetValue();
				if (entity)
				{
					SCR_AIGroup group = SCR_AIGroup.Cast(entity);
					if (group)
					{
						if (slotAI)
						{
							slotAI.m_Entity = entity;
							slotAI.m_AIGroup = group;	
						}
						else
						{
							slotTaskAI.m_Entity = entity;
							slotTaskAI.m_AIGroup = group;	
							if (slotTaskAI.m_TaskLayer && slotTaskAI.m_TaskLayer.m_Task)
									slotTaskAI.m_TaskLayer.m_Task.HookTaskAsset(entity);
						}
						
						return;
					}
					else
					{
						ChimeraCharacter character = ChimeraCharacter.Cast(entity);
						if (character)
						{
							if (slotAI)
							{
								slotAI.m_Entity = character;
							}
							else
							{
								slotTaskAI.m_Entity = character;	
								if (slotTaskAI.m_TaskLayer && slotTaskAI.m_TaskLayer.m_Task)
									slotTaskAI.m_TaskLayer.m_Task.HookTaskAsset(entity);
							}
							
							return;
						}
					}
				}
			}
		}
		
		if (slotAI)
		{
			slotAI.m_Entity = null;
			slotAI.m_AIGroup = null;
		}
		else
		{
			slotTaskAI.m_Entity = null;
			slotTaskAI.m_AIGroup = null;
		}
	}
}