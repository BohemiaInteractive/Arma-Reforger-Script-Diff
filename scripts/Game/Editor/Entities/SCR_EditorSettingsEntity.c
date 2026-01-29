[EntityEditorProps(category: "GameScripted/Editor", description: "World settings for the editor", color: "251 91 0 255", icon: "WBData/EntityEditorProps/entityEditor.png")]
class SCR_EditorSettingsEntityClass: SCR_EditorBaseEntityClass
{
};

/** @ingroup Editor_Entities
*/

/*!
Editor settings to override default values in SCR_EditorManagerCore.
*/
class SCR_EditorSettingsEntity : SCR_EditorBaseEntity
{
	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, desc: "Individual editor manager", params: "et", category: "Editor Settings")]
	protected ResourceName m_EditorManagerPrefab;
	
	[Attribute(desc: "True to disable the editor completely.\nFOR DEVELOPMENT PURPOSES ONLY!", category: "Editor Settings")]
	protected bool m_bDisableEditor;
	
	[Attribute(desc: "When enabled, using the editor in this scenario is a legal operation, part of game rules.\nFor example this is enabled in Game Master scenarios which are about GM building the gameplay,\nbut it's disabled in Conflict where only admin can access the editor.", category: "Editor Settings")]
	protected bool m_bIsUnlimitedEditorLegal;
	
	[Attribute(desc: "When enabled, each player will receive editor modes defined by 'Base Modes' attribute instead of the modes defined in core config.", category: "Editor Settings")]
	protected bool m_bOverrideBaseModes;
	
	[Attribute("0", UIWidgets.Flags, "Editor modes added to every pllayer when they connect.", enums: ParamEnumArray.FromEnum(EEditorMode), category: "Editor Settings")]
	protected EEditorMode m_BaseModes;
	
	[Attribute("", UIWidgets.SearchComboBox, "Skip Streaming Rules", enums: ParamEnumArray.FromEnum(EEditableEntityType), category: "Editor Settings")]
	protected ref array<EEditableEntityType> m_SkipStreamingRules;
	
	/*!
	\return Local instance of the editor settings entity
	*/
	static SCR_EditorSettingsEntity GetInstance()
	{
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (core)
			return core.GetSettingsEntity();
		else
			return null;
	}
	
	/*!
	Get editor manager prefab.
	\param basePrefab Prefab used when the entity doesn't define custom prefab
	\return Editor manager prefab.
	*/
	ResourceName GetPrefab(ResourceName basePrefab)
	{
		if (m_EditorManagerPrefab.IsEmpty()) return basePrefab;
		return m_EditorManagerPrefab;
	}
	
	/*!
	Check if editor is disabled.
	FOR DEVELOPMENT PURPOSES ONLY!
	\return True when disabled
	*/
	bool IsEditorDisabled()
	{
		return m_bDisableEditor;
	}
	
	/*!
	Check if unlimited editor (e.g, Game Master) is legal in this scenario, i.e., part of game rules.
	For example this is enabled in Game Master scenarios which are about GM building the gameplay,
	but it's disabled in Conflict where only admin can access the editor.
	\return True when legal
	*/
	bool IsUnlimitedEditorLegal()
	{
		return m_bIsUnlimitedEditorLegal;
	}
	
	/*!
	When set to true, connecting players will receive editor modes define by these settings, instead of use defaults from SCR_EditorManagerCore.
	\value True to enable override
	*/
	void EnableBaseOverride(bool value)
	{
		m_bOverrideBaseModes = value;
	}
	/*!
	Check if base editor modes are overridden.
	\return True when override is active
	*/
	bool IsBaseOverrideEnabled()
	{
		return m_bOverrideBaseModes;
	}
	/*!
	Get base editor modes in this world.
	\param modes Variable to be filled with editor mode flags.
	\return True if the override is enabled
	*/
	bool GetBaseModes(out EEditorMode modes = 0)
	{
		modes = m_BaseModes;
		return m_bOverrideBaseModes;
	}
	/*!
	Set base editor modes in this world.
	\param modes Editor mode flags.
	\param validate When true, remove non-base modes from already existing editor managers
	*/
	void SetBaseModes(EEditorMode modes, bool validate = true)
	{
		m_BaseModes = modes;
		
		//--- Validate all existing editor modes
		if (validate)
		{
			SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
			if (core)
			{
				array<SCR_EditorManagerEntity> editorManagers = {};
				core.GetEditorEntities(editorManagers);
				foreach (SCR_EditorManagerEntity editorManager: editorManagers)
				{
					editorManager.SetEditorModes(EEditorModeAccess.BASE, m_BaseModes, false);
				}
			}
		}
	}
	
	/*!
	Get which EEditableEntityType to not stream in to client when he opens editor.
	\param skipStreamingRules The array of EEditableEntityType for which to skip streaming logic.
	\return True if there are any streaming rules set.
	*/
	bool GetSkipStreamingRules(out notnull array<EEditableEntityType> skipStreamingRules)
	{		
		if (!m_SkipStreamingRules)
			return false;
		
		skipStreamingRules.InsertAll(m_SkipStreamingRules);
		return !m_SkipStreamingRules.IsEmpty();
	}
	
	void SCR_EditorSettingsEntity(IEntitySource src, IEntity parent)
	{
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (core)
		{
			core.SetSettingsEntity(this);
		} 
	}
};