/*
[Obsolete("Only used for backwards compatiblity for GM saves. Will be removed entirely.")]
class SCR_EditableFactionStruct: JsonApiStruct
{
	//--- Serialized (names shortened to save memory)
	protected string fk; //--- Faction key
	protected ref array<ref SCR_EditorAttributeStruct> at = {};
	
	static void SerializeFactions(out notnull array<ref SCR_EditableFactionStruct> outEntries, SCR_EditorAttributeList attributeList)
	{
		SCR_DelegateFactionManagerComponent delegateManager = SCR_DelegateFactionManagerComponent.GetInstance();
		if (!delegateManager)
			return;
		
		//--- Clear existing array
		outEntries.Clear();
		
		SCR_SortedArray<SCR_EditableFactionComponent> delegates = new SCR_SortedArray<SCR_EditableFactionComponent>();
		for (int i, count = delegateManager.GetSortedFactionDelegates(delegates); i < count; i++)
		{
			SerializeFaction(delegates[i], outEntries, attributeList);
		}
	}
	protected static void SerializeFaction(SCR_EditableFactionComponent faction, out notnull array<ref SCR_EditableFactionStruct> outEntries, SCR_EditorAttributeList attributeList)
	{
		SCR_EditableFactionStruct entry = new SCR_EditableFactionStruct();
		entry.fk = faction.GetFaction().GetFactionKey();
		outEntries.Insert(entry);
		
		SCR_EditorAttributeStruct.SerializeAttributes(entry.at, attributeList, faction);
	}
	static void DeserializeFactions(notnull array<ref SCR_EditableFactionStruct> entries, SCR_EditorAttributeList attributeList = null)
	{
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return;
		
		SCR_DelegateFactionManagerComponent delegateManager = SCR_DelegateFactionManagerComponent.GetInstance();
		if (!delegateManager)
			return;
		
		Faction faction;
		SCR_EditableFactionComponent factionDelegate;
		foreach (int id, SCR_EditableFactionStruct entry: entries)
		{
			faction = factionManager.GetFactionByKey(entry.fk);
			factionDelegate = delegateManager.GetFactionDelegate(faction);
			if (factionDelegate)
			{
				SCR_EditorAttributeStruct.DeserializeAttributes(entry.at, attributeList, factionDelegate);
			}
			else
			{
				Print(string.Format("SCR_EditableFactionStruct: Cannot load faction '%1', it's not configured in FactionManager!", entry.fk), LogLevel.WARNING);
			}
		}
		
		SCR_SortedArray<SCR_EditableFactionComponent> delegates = new SCR_SortedArray<SCR_EditableFactionComponent>();
		for (int i, count = delegateManager.GetSortedFactionDelegates(delegates); i < count; i++)
		{
		}
	}
	static void LogFactions(notnull array<ref SCR_EditableFactionStruct> entries, SCR_EditorAttributeList attributeList = null)
	{
		Print("  SCR_EditableFactionStruct: " + entries.Count());
		foreach (int id, SCR_EditableFactionStruct entry: entries)
		{
			PrintFormat("    %1", entry.fk);
			
			SCR_EditorAttributeStruct.LogAttributes(entry.at, attributeList, "    ");
		}
	}
	void SCR_EditableFactionStruct()
	{
		RegV("fk");
		RegV("at");
	}
};
*/