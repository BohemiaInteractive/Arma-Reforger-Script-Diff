/**
Dynamic description for SCR_PlayableFactionEditorAttribute and SCR_PlayableFactionsEditorAttribute to show how many players are killed
*/
[BaseContainerProps(), BaseContainerCustomStringTitleField("Players will be killed warning (CUSTOM)")]
class SCR_PlayableFactionKillingAttributeDynamicDescription : SCR_BaseButtonAttributeDynamicDescription
{	
	protected int m_iPlayersWillBeKilled = 0;
	protected ref array<int> m_aStartingDisabledFactions = {};
	
	protected FactionManager m_FactionManager;
	protected SCR_ToolboxComponent m_ToolBoxComponent;
	protected SCR_BaseEditorAttributeFloatStringValues m_ButtonBoxData;
	
	protected SCR_RespawnSystemComponent m_RespawnSystemComponent;
	protected SCR_PlayerDelegateEditorComponent m_PlayerDelegateManager;
	
	//------------------------------------------------------------------------------------------------
	override void InitDynamicDescription(notnull SCR_BaseEditorAttribute attribute, notnull SCR_BaseEditorAttributeUIComponent attributeUi)
	{
		super.InitDynamicDescription(attribute);
		
		if (!attribute.IsInherited(SCR_PlayableFactionEditorAttribute) && !attribute.IsInherited(SCR_PlayableFactionsEditorAttribute))
		{
			Print("'SCR_PlayableFactionDisabledAttributeDynamicDescription' is not attached to the 'SCR_PlayableFactionEditorAttribute' nor 'SCR_PlayableFactionsEditorAttribute'!", LogLevel.ERROR);
			return;
		}
			
		if (attribute.IsInherited(SCR_PlayableFactionsEditorAttribute))
		{
			SCR_ButtonBoxAttributeUIComponent buttonBoxUI = SCR_ButtonBoxAttributeUIComponent.Cast(attributeUi);
			if (!buttonBoxUI)
			{
				Print("'SCR_PlayableFactionDisabledAttributeDynamicDescription' is used for 'SCR_PlayableFactionsEditorAttribute' but attribute UI does not inherent from SCR_ButtonBoxAttributeUIComponent!", LogLevel.ERROR);
				return;
			}
				
			m_ToolBoxComponent = buttonBoxUI.GetToolboxComponent();
			if (!m_ToolBoxComponent)
			{
				Print("'SCR_PlayableFactionDisabledAttributeDynamicDescription' is used for 'SCR_PlayableFactionsEditorAttribute' but could not find SCR_ToolboxComponent!", LogLevel.ERROR);
				return;
			}
				
			m_ButtonBoxData = buttonBoxUI.GetButtonBoxData();
			if (!m_ButtonBoxData)
			{
				Print("'SCR_PlayableFactionDisabledAttributeDynamicDescription' is used for 'SCR_PlayableFactionsEditorAttribute' but could not find m_ButtonBoxData!", LogLevel.ERROR);
				return;
			}
		}
		
		if (!m_FactionManager)
			m_FactionManager = GetGame().GetFactionManager();
		
		if (!m_RespawnSystemComponent)
			m_RespawnSystemComponent = SCR_RespawnSystemComponent.GetInstance();
		
		if (!m_PlayerDelegateManager)
			m_PlayerDelegateManager = SCR_PlayerDelegateEditorComponent.Cast(SCR_PlayerDelegateEditorComponent.GetInstance(SCR_PlayerDelegateEditorComponent, true));		
		
		//~ Get factions that started out disabled
		m_aStartingDisabledFactions.Clear();
		GetDisabledFactions(attribute, m_aStartingDisabledFactions);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool IsValid(notnull SCR_BaseEditorAttribute attribute, notnull SCR_BaseEditorAttributeUIComponent attributeUi)
	{
		if (!super.IsValid(attribute, attributeUi))
			return false;
		
		if (!attribute.IsInherited(SCR_PlayableFactionEditorAttribute) && !attribute.IsInherited(SCR_PlayableFactionsEditorAttribute))
			return false;
		
		if (!m_FactionManager)
			return false;
		
		if (attribute.IsInherited(SCR_PlayableFactionsEditorAttribute))
		{
			if (!m_ToolBoxComponent || !m_ButtonBoxData)
				return false;
		}
		
		if (!m_FactionManager || !m_RespawnSystemComponent || !m_PlayerDelegateManager)
			return false;

		UpdateTotalToBeKilledPlayers(attribute);
		
		return m_iPlayersWillBeKilled > 0;
	}
	
	//------------------------------------------------------------------------------------------------
	override void GetDescriptionData(notnull SCR_BaseEditorAttribute attribute, notnull SCR_BaseEditorAttributeUIComponent attributeUi, out SCR_EditorAttributeUIInfo uiInfo, out string param1 = string.Empty, out string param2 = string.Empty, out string param3 = string.Empty)
	{
		UpdateTotalToBeKilledPlayers(attribute);
		
		uiInfo = m_DescriptionDisplayInfo;
		param1 = m_iPlayersWillBeKilled.ToString();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateTotalToBeKilledPlayers(SCR_BaseEditorAttribute attribute)
	{
		array<int> disabledFactions = {};
		m_iPlayersWillBeKilled = 0;
		
		SCR_BaseEditorAttributeVar var = attribute.GetVariableOrCopy();
		if (!var)
			return;
		
		//~ Get idisabled factions
		GetDisabledFactions(attribute, disabledFactions);
		
		Faction playerFaction;
		
		if (!disabledFactions.IsEmpty())
		{
			SCR_FactionManager factionManager = SCR_FactionManager.Cast(m_FactionManager);
			if (!factionManager) // factions and player factions depend on such faction manager
				return;
			
			array<int> players = new array<int>;
			GetGame().GetPlayerManager().GetPlayers(players);
			SCR_EditablePlayerDelegateComponent playerEditorDelegate;
	
			foreach (int playerId: players)
			{
				//If has player entity
				if(!SCR_PossessingManagerComponent.GetPlayerMainEntity(playerId))
					continue;
				
				playerFaction = factionManager.GetPlayerFaction(playerId);
				if (!playerFaction)
					continue;
				
				int factionIndex = factionManager.GetFactionIndex(playerFaction);
				
				if (!disabledFactions.Contains(factionIndex) || m_aStartingDisabledFactions.Contains(factionIndex))
					continue;
				
				//Ignore GM
				/*if (m_PlayerDelegateManager)
				{
					playerEditorDelegate = m_PlayerDelegateManager.GetDelegate(playerId);
					
					if (playerEditorDelegate)
					{
						if (!playerEditorDelegate.HasLimitedEditor())
							continue;
					}
				}*/
				
				m_iPlayersWillBeKilled++;
			}	
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void GetDisabledFactions(SCR_BaseEditorAttribute attribute, notnull out array<int> disabledFactions)
	{
		SCR_BaseEditorAttributeVar var = attribute.GetVariableOrCopy();
		if (!var)
			return;
		
		//~ Single faction get if disabled
		if (attribute.IsInherited(SCR_PlayableFactionEditorAttribute))
		{
			vector value = var.GetVector();

			if (value[0] == 0)
				disabledFactions.Insert((int)value[1]);	
		}
		//~ Multiple factions get all disabled faction
		else 
		{
			if (!m_ToolBoxComponent || !m_FactionManager || !m_ButtonBoxData)
				return;
		
			//Get all disabled buttonts and the faction associated with it and make sure the system adds a Attribute explanation if any players will die on Accept
			int count = m_ButtonBoxData.GetValueCount();
			
			int factionIndex;
			Faction faction;
			SCR_Faction scrFaction;
			
			for(int i = 0; i < count; i++)
	        {
				if (!m_ToolBoxComponent.IsItemSelected(i))
				{		
					factionIndex = m_ButtonBoxData.GetEntryFloatValue(i);
								
					faction = m_FactionManager.GetFactionByIndex(factionIndex);
					
					scrFaction = SCR_Faction.Cast(faction);
					
					//Check if currently playable
					if (scrFaction && scrFaction.IsPlayable())
						disabledFactions.Insert(factionIndex);
				}
	        }
		}
	}
};
