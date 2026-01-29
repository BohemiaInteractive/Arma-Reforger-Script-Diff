/*!
@ingroup Editor_Containers_Backend

Class which carries saved data for the editor.
Managed by SCR_DSSessionCallback.
*/
/*
[BaseContainerProps(), Obsolete("Only used for backwards compatiblity for GM saves. Will be removed entirely.")]
class SCR_EditorStruct : SCR_JsonApiStruct
{
	[Attribute(desc: "Array of attributes evaluated during saving. Can include global as well as entity attributes.")]
	protected ref SCR_EditorAttributeList m_AttributeList;
	
	[Attribute("1", desc: "Enable to save the list of playable factions and their attributes.")]
	protected bool m_bSaveFactions;
	
	[Attribute("1", desc: "Enable to save scenario attributes like weather or time of day.")]
	protected bool m_bSaveMissionAttributes;
	
	[Attribute(uiwidget: UIWidgets.Flags, enums: ParamEnumArray.FromEnum(EEditableEntityFlag), desc: "When defined, only entities with *all* of these flags will be saved.\nLeave empty to save all interactive entities.")]
	protected EEditableEntityFlag m_eRequiredEntityFlags;
	
	//protected string m_sBuildVersion;
	protected ref SCR_EditorMetaStruct m_Meta = new SCR_EditorMetaStruct();
	protected ref array<ref SCR_EditableFactionStruct> m_aFactions = {};
	protected ref array<ref SCR_EditableEntityStruct> m_aEntities = {};
	protected ref array<ref SCR_EditorAttributeStruct> m_aAttributes = {};
	
	override void Log()
	{
		Print("--- SCR_EditorStruct ------------------------");
		
		m_Meta.Log();
		SCR_EditableEntityStruct.LogEntities(m_aEntities, m_AttributeList);
		
		if (m_bSaveFactions)
			SCR_EditableFactionStruct.LogFactions(m_aFactions, m_AttributeList);
		
		if (m_bSaveMissionAttributes)
			SCR_EditorAttributeStruct.LogAttributes(m_aAttributes, m_AttributeList);
		
		Print("---------------------------------------------");
	}

	override bool Serialize()
	{
		m_Meta.Serialize();
		SCR_EditableEntityStruct.SerializeEntities(m_aEntities, m_AttributeList, m_eRequiredEntityFlags);
		
		if (m_bSaveFactions)
			SCR_EditableFactionStruct.SerializeFactions(m_aFactions, m_AttributeList);
		
		if (m_bSaveMissionAttributes)
			SCR_EditorAttributeStruct.SerializeAttributes(m_aAttributes, m_AttributeList, GetGame().GetGameMode());
		
		Print("SUCCESS: SCR_EditorStruct.SaveEditor()", LogLevel.VERBOSE);
		return true;
	}

	override bool Deserialize()
	{
		m_Meta.Deserialize();
		SCR_EditableEntityStruct.DeserializeEntities(m_aEntities, m_AttributeList);
		
		if (m_bSaveFactions)
			SCR_EditableFactionStruct.DeserializeFactions(m_aFactions, m_AttributeList);
		
		if (m_bSaveMissionAttributes)
			SCR_EditorAttributeStruct.DeserializeAttributes(m_aAttributes, m_AttributeList, GetGame().GetGameMode());
		
		Print("SUCCESS: SCR_EditorStruct.LoadEditor()", LogLevel.VERBOSE);
		return true;
	}

	override void ClearCache()
	{
		m_Meta.ClearCache();
		m_aEntities.Clear();
		
		if (m_bSaveFactions)
			m_aFactions.Clear();
		
		if (m_bSaveMissionAttributes)
			m_aAttributes.Clear();
	}

	void ResetEditor()
	{
		SCR_EditableEntityStruct.ClearEntities(m_aEntities);
	}

	void SCR_EditorStruct()
	{
		RegV("m_Meta");
		RegV("m_aEntities");
		
		if (m_bSaveFactions)
			RegV("m_aFactions");
		
		if (m_bSaveMissionAttributes)
			RegV("m_aAttributes");
	}
};
*/