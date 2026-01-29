[ComponentEditorProps(category: "GameScripted/AI", description: "Component for AI Group settings")]
class SCR_AIGroupSettingsComponentClass : SCR_AISettingsBaseComponentClass
{
}

/*!
SCR_AIGroupSettingsComponent can store settings intended both for group itself and for group members.
	
Settings inheriting from SCR_AIGroupSetting are treated as usual. They are meant to be used by group.
	
Settings inheriting from SCR_AICharacterSetting are stored here but are meant for group members.
They are automatically distributed to group members when added.
When a new member joins the group, group's character settings are passed to the new member.
When a member leaves the group, settings added to it from this group are removed from it.

To add settings, call AddSetting.
*/

class SCR_AIGroupSettingsComponent : SCR_AISettingsBaseComponent
{
	protected SCR_AIGroup m_Group;
	
	[Attribute("", UIWidgets.Auto, desc: "These settings will be added as SCR_EAISettingOrigin.DEFAULT")]
	protected ref array<ref SCR_AISettingBase> m_aDefaultSettings;
	
	//----------------------------------------------------------------------------------------------------------------------------------------------
	// Internal
	
	//---------------------------------------------------------------------------------------------------
	override protected void OnSettingAdded(notnull SCR_AISettingBase setting)
	{
		SCR_AICharacterSetting characterSetting = SCR_AICharacterSetting.Cast(setting);
		if (characterSetting)
			AddSettingToMembers(characterSetting);
	}
	
	//---------------------------------------------------------------------------------------------------
	override protected void OnSettingRemoved(notnull SCR_AISettingBase setting)
	{
		// If a character setting was removed, also remove it from members
		SCR_AICharacterSetting characterSetting = SCR_AICharacterSetting.Cast(setting);
		if (characterSetting)
			RemoveSettingFromMembers(characterSetting);
	}
	
	//---------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		m_Group = SCR_AIGroup.Cast(owner);
		
		// Add default settings
		foreach (auto s : m_aDefaultSettings)
		{
			s.Internal_ConstructedAtProperty(SCR_EAISettingOrigin.DEFAULT, SCR_EAISettingFlags.SETTINGS_COMPONENT);
			AddSetting(s, false, false);
		}
		
		// Subscribe to group events
		if (m_Group)
		{		
			m_Group.GetOnAgentAdded().Insert(OnAgentAdded);
			m_Group.GetOnAgentRemoved().Insert(OnAgentRemoved);
			m_Group.GetOnCurrentWaypointChanged().Insert(OnCurrentWaypointChanged);
		}
	}
	
	//---------------------------------------------------------------------------------------------------
	//! Creates copy and adds setting to all group members.
	//! createCopy - when true, a copy of the setting object will be created. When false, this component takes ownership of passed setting object.
	//! It also makes copy of passed object for each group member, but it's not related to createCopy parameter.
	protected void AddSettingToMembers(notnull SCR_AICharacterSetting setting)
	{
		#ifdef AI_DEBUG
		string strDebugText = string.Format("AddCharacterSettingToMembers: %1, Origin: %2, Priority: %3, %4",
			setting,
			typename.EnumToString(SCR_EAISettingOrigin, setting.GetOrigin()),
			setting.GetPriority(),
			setting.GetDebugText());
		AddDebugMessage(strDebugText);
		#endif
		
		if (!m_Group)
			return;
		
		// Bail if there are no members, the rest makes no sense
		array<AIAgent> agents = {};
		m_Group.GetAgents(agents);
		
		foreach (AIAgent agent : agents)
		{
			SCR_AICharacterSettingsComponent settingsComp = SCR_AICharacterSettingsComponent.Cast(agent.FindComponent(SCR_AICharacterSettingsComponent));
			if (!settingsComp)
				continue;
			
			settingsComp.AddCharacterSetting(setting, true, false); // Create a copy - every member gets its own copy, and we keep the original
		}
	}
	
	//---------------------------------------------------------------------------------------------------
	//! Iterates all group members and removes from them all settings which are linked to passed setting
	protected void RemoveSettingFromMembers(notnull SCR_AICharacterSetting setting)
	{	
		array<AIAgent> agents = {};
		m_Group.GetAgents(agents);
		
		foreach (AIAgent agent : agents)
		{
			SCR_AICharacterSettingsComponent settingsComp = SCR_AICharacterSettingsComponent.Cast(agent.FindComponent(SCR_AICharacterSettingsComponent));
			if (!settingsComp)
				continue;
			
			settingsComp.RemoveChildSettingsOfParent(setting);
		}
	}
	
	//---------------------------------------------------------------------------------------------------
	protected void OnAgentAdded(AIAgent agent)
	{
		SCR_AICharacterSettingsComponent settingsComp = SCR_AICharacterSettingsComponent.Cast(agent.FindComponent(SCR_AICharacterSettingsComponent));
		if (!settingsComp)
			return;
		
		// Add all settings from m_aCharacterSettings and add them to that agent
		foreach (typename t, auto a : m_mSettings)
		{
			// Ignore settings which are not meant for characters
			if (!t.IsInherited(SCR_AICharacterSetting))
				continue;
			
			foreach (auto s : a)
			{
				SCR_AICharacterSetting characterSetting = SCR_AICharacterSetting.Cast(s);
				settingsComp.AddCharacterSetting(characterSetting, true); // Create copy - the new agent gets a copy, and we keep the original
			}
		}
	}
	
	//---------------------------------------------------------------------------------------------------
	protected void OnAgentRemoved(AIGroup group, AIAgent agent)
	{
		SCR_AICharacterSettingsComponent settingsComp = SCR_AICharacterSettingsComponent.Cast(agent.FindComponent(SCR_AICharacterSettingsComponent));
		if (!settingsComp)
			return;
		
		// Remove all settings in the agent which are added from the group
		foreach (typename t, auto a : m_mSettings)
		{
			// Ignore settings which are not meant for characters
			if (!t.IsInherited(SCR_AICharacterSetting))
				continue;
			
			foreach (auto s : a)
				settingsComp.RemoveChildSettingsOfParent(s);
		}
	}
	
	//---------------------------------------------------------------------------------------------------
	protected void OnCurrentWaypointChanged(AIWaypoint _currentWp, AIWaypoint _prevWp)
	{		
		// Remove all settings which are related to waypoints
		RemoveSettingsWithFlag(SCR_EAISettingFlags.WAYPOINT);
		
		// Add settings from new wp
		SCR_AIWaypoint currentWp = SCR_AIWaypoint.Cast(_currentWp);
		if (currentWp)
		{
			array<SCR_AISettingBase> wpSettings = {};
			currentWp.GetSettings(wpSettings);
			foreach (auto s : wpSettings)
			{
				AddSetting(s, true); // Create copy - the original remains in the waypoint, and is not registered here
			}
		}
	}
}