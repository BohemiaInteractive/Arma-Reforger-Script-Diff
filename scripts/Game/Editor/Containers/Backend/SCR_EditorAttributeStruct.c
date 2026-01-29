/*!
@ingroup Editor_Containers_Backend

Saved data for editor attribute.
*/
/*
[Obsolete("Only used for backwards compatiblity for GM saves. Will be removed entirely.")]
class SCR_EditorAttributeStruct: JsonApiStruct
{
	//--- Serialized (names shortened to save memory)
	protected int id; //--- ID
	protected int ty; //--- Type
	protected float v0; //--- Var 0
	protected float v1; //--- Var 1
	protected float v2; //--- Var 2
	
	static void SerializeAttributes(out notnull array<ref SCR_EditorAttributeStruct> outEntries, SCR_EditorAttributeList attributeList = null, Managed item = null)
	{
		outEntries.Clear();
		
		SCR_BaseEditorAttribute attribute;
		SCR_BaseEditorAttributeVar var;
		for (int i = 0, count = attributeList.GetAttributesCount(); i < count; i++)
		{
			attribute = attributeList.GetAttribute(i);
			if (!attribute.IsServer() || !attribute.IsSerializable())
				continue;
			
			var = attribute.ReadVariable(item, null);
			if (!var)
				continue;
			
			SCR_EditorAttributeStruct entry = new SCR_EditorAttributeStruct();
			outEntries.Insert(entry);
			entry.id = i;
			
			entry.v0 = var.GetVector()[0];
			entry.v1 = var.GetVector()[1];
			entry.v2 = var.GetVector()[2];
		}
	}

	static void DeserializeAttributes(notnull array<ref SCR_EditorAttributeStruct> entries, SCR_EditorAttributeList attributeList = null, Managed item = null)
	{
		SCR_BaseEditorAttribute attribute;
		SCR_BaseEditorAttributeVar var;
		
		foreach (SCR_EditorAttributeStruct entry: entries)
		{
			attribute = attributeList.GetAttribute(entry.id);
			if (!attribute.IsSerializable())
				continue;
			
			var = SCR_BaseEditorAttributeVar.CreateVector(Vector(entry.v0, entry.v1, entry.v2));
			attribute.WriteVariable(item, var, null, -1);
		}
	}

	static void LogAttributes(out notnull array<ref SCR_EditorAttributeStruct> entries, SCR_EditorAttributeList attributeList = null, string prefix = "")
	{
		Print(prefix + "  SCR_EditorAttributeStruct: " + entries.Count());
		foreach (SCR_EditorAttributeStruct entry: entries)
		{
			PrintFormat(prefix + "    %1: %2, %3, %4", attributeList.GetAttribute(entry.id).ClassName(), entry.v0, entry.v1, entry.v2);
		}
	}
	void SCR_EditorAttributeStruct()
	{
		RegV("id");
		RegV("v0");
		RegV("v1");
		RegV("v2");
	}
};
*/