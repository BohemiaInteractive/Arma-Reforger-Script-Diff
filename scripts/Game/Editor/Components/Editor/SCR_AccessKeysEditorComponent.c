//#define DISABLE_ADMIN_ACCESS //--- Enable this to prevent admin from seeing all entities

[ComponentEditorProps(category: "GameScripted/Editor", description: "Attribute for managing access keys. Works only with SCR_EditorBaseEntity!", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_AccessKeysEditorComponentClass : SCR_BaseEditorComponentClass
{
}

//! @ingroup Editor_Components

//! Manager of editor access keys.
//!
//! Only entities with at least one matching key will be made available for the editor.
class SCR_AccessKeysEditorComponent : SCR_BaseEditorComponent
{	
	[Attribute("1", UIWidgets.Flags, "", enums: ParamEnumArray.FromEnum(EEditableEntityAccessKey))]
	private EEditableEntityAccessKey m_AccessKey;
	
	ref ScriptInvoker Event_OnChanged = new ScriptInvoker();
	
	#ifdef WORKBENCH
		private bool m_bIsAdmin = true;
	#endif
	
	//------------------------------------------------------------------------------------------------
	//! Add access key.
	//! \param[in] accessKey Key to be added
	void AddAccessKey(EEditableEntityAccessKey accessKey)
	{
		Rpc(AddAccessKeyServer, accessKey);
	}

	//------------------------------------------------------------------------------------------------
	//! Remove access key.
	//! \param[in] accessKey Key to be removed
	//!
	void RemoveAccessKey(EEditableEntityAccessKey accessKey)
	{
		Rpc(RemoveAccessKeyServer, accessKey);
	}
	
	//------------------------------------------------------------------------------------------------
	//--- Manipulate access keys of the editor
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void AddAccessKeyServer(EEditableEntityAccessKey accessKey)
	{
		if (HasAccess(accessKey) || IsAdmin())
			return;

		m_AccessKey = m_AccessKey | accessKey;
		Rpc(AccessKeyChangedOwner, m_AccessKey);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RemoveAccessKeyServer(EEditableEntityAccessKey accessKey)
	{
		if (!HasAccess(accessKey) || IsAdmin())
			return;

		m_AccessKey = m_AccessKey &~ accessKey;
		Rpc(AccessKeyChangedOwner, m_AccessKey);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void AccessKeyChangedOwner(EEditableEntityAccessKey accessKey)
	{
		m_AccessKey = accessKey;
		Event_OnChanged.Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get access key.
	//! \return Editor's access key
	EEditableEntityAccessKey GetAccessKey()
	{
		return m_AccessKey;
	}

	//------------------------------------------------------------------------------------------------
	//! Has access to given key.
	//! \accessKey Key to be checked
	//! \return True when has access
	bool HasAccess(EEditableEntityAccessKey accessKey)
	{
		return m_AccessKey & accessKey;
	}
	
	//--- Manipulate access key of an entity
	//------------------------------------------------------------------------------------------------
	//! Add access key to an entity.
	//! \param[in] entity Affected entity
	//! \param[in] accessKey Key to be added
	void AddEntityAccessKey(SCR_EditableEntityComponent entity, EEditableEntityAccessKey accessKey)
	{
		Rpc(AddEntityAccessKeyServer, Replication.FindId(entity), accessKey);
	}
	//------------------------------------------------------------------------------------------------
	//! Remove access key from an entity.
	//! \param[in] entity Affected entity
	//! \param[in] accessKey Key to be removed
	void RemoveEntityAccessKey(SCR_EditableEntityComponent entity, EEditableEntityAccessKey accessKey)
	{
		Rpc(RemoveEntityAccessKeyServer, Replication.FindId(entity), accessKey);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void AddEntityAccessKeyServer (int entityID, EEditableEntityAccessKey accessKey)
	{
		SCR_EditableEntityComponent entity = SCR_EditableEntityComponent.Cast(Replication.FindItem(entityID));
		if (entity)
			entity.AddAccessKey(accessKey);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RemoveEntityAccessKeyServer (int entityID, EEditableEntityAccessKey accessKey)
	{
		SCR_EditableEntityComponent entity = SCR_EditableEntityComponent.Cast(Replication.FindItem(entityID));
		if (entity)
			entity.RemoveAccessKey(accessKey);
	}
	
	//--- Debug
	//------------------------------------------------------------------------------------------------
	protected string GetKeyDebug()
	{
		string output = "";
		if (IsAdmin())
		{
			output = "<ADMIN>";
		}
		else
		{
			typename enumType = EEditableEntityAccessKey;
			int enumCount = enumType.GetVariableCount();
			for (int i = 0; i < enumCount; i++)
			{
				int val;
				if (enumType.GetVariableType(i) == int && enumType.GetVariableValue(null, i, val))
				{
					if (HasAccess(val))
					{
						if (!output.IsEmpty())
							output += ", ";

						output += enumType.GetVariableName(i);
					}
				}
			}
		}
		return string.Format("Editor access keys: %1", output);
	}

	//------------------------------------------------------------------------------------------------
	//! Print out all entities with compatible access keys.
	void LogAvailableEntities()
	{
		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (!core)
			return;
		
		Print("--------------------------------------------------", LogLevel.DEBUG);
		set<SCR_EditableEntityComponent> entities = new set<SCR_EditableEntityComponent>();
		core.GetAllEntities(entities, true);
		foreach (SCR_EditableEntityComponent entity: entities)
		{
			if (entity.IsLayer())
				entity.Log("", m_AccessKey);
		}
		Print("--------------------------------------------------", LogLevel.DEBUG);
	}

	//------------------------------------------------------------------------------------------------
	//! Print out access keys.
	void LogAccessKey()
	{
		Print(GetKeyDebug(), LogLevel.DEBUG);
	}
	
	#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	override bool IsAdmin()
	{
		return m_bIsAdmin && super.IsAdmin();
	}
	#endif
	
	//------------------------------------------------------------------------------------------------
	override void EOnEditorDebug(array<string> debugTexts)
	{
		debugTexts.Insert(GetKeyDebug());
	}

	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{		
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_LOG_AVAILABLE))
		{
			LogAvailableEntities();
			DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_LOG_AVAILABLE, false);
		}
		
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_LOG_ACCESS_KEYS))
		{
			LogAccessKey();
			DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_LOG_ACCESS_KEYS, false);
		}
		
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_ACCESS_KEY_TOGGLE))
		{
			typename keysType = EEditableEntityAccessKey;
			int keyValue;
			keysType.GetVariableValue(null, DiagMenu.GetRangeValue(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_ACCESS_KEY), keyValue);
			if (!HasAccess(keyValue))
			{
				AddAccessKey(keyValue);
			}
			else
			{
				RemoveAccessKey(keyValue);
			}
			LogAccessKey();
			DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_ACCESS_KEY_TOGGLE, false);
		}
		
		#ifdef WORKBENCH
			if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_ACCESS_KEY_ADMIN) != m_bIsAdmin)
			{
				m_bIsAdmin = DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_ACCESS_KEY_ADMIN);
				if (m_bIsAdmin)
					m_AccessKey = int.MAX;
				else
					m_AccessKey = EEditableEntityAccessKey.DEFAULT;
			
				Event_OnChanged.Invoke();
			}
		#endif
	}

	//------------------------------------------------------------------------------------------------
	//--- JIP on server
	override bool RplSave(ScriptBitWriter writer)
	{
		writer.Write(m_AccessKey, 32);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//--- JIP on client
	override bool RplLoad(ScriptBitReader reader)
	{
		reader.Read(m_AccessKey, 32);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void EOnEditorActivate()
	{
		SetEventMask(GetOwner(), EntityEvent.FRAME);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnEditorInit()
	{
		if (SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore) == null)
			return;
		
		typename keysType = EEditableEntityAccessKey;
		int keysCount = keysType.GetVariableCount();
		
		//--- Unlock everything for admin
		if (IsAdmin())
			m_AccessKey = int.MAX;
		
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_LOG_AVAILABLE, "", "Log Available", "Editable Entities");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_LOG_ACCESS_KEYS, "", "Log Access Keys", "Editable Entities");
		DiagMenu.RegisterRange(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_ACCESS_KEY, "", "Access Key", "Editable Entities", string.Format("0, %1, 0, 1", keysCount - 1));
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_ACCESS_KEY_TOGGLE, "", "Access Key: Toggle", "Editable Entities");
		DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_LOG_ACCESS_KEYS, false);
		DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_ACCESS_KEY_TOGGLE, false);
		
#ifdef WORKBENCH
		m_bIsAdmin = IsAdmin();
		if (m_bIsAdmin)
		{
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_ACCESS_KEY_ADMIN, "", "Admin Access", "Editable Entities");
			DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_ACCESS_KEY_ADMIN, m_bIsAdmin);
		}
#endif
	}

	//------------------------------------------------------------------------------------------------
	override protected void EOnEditorDelete()
	{
		DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_LOG_AVAILABLE);
		DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_LOG_ACCESS_KEYS);
		DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_ACCESS_KEY);
		DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_ACCESS_KEY_TOGGLE);
		DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_ACCESS_KEY_ADMIN);
		Event_OnChanged = null;
	}
}
