[ComponentEditorProps(category: "GameScripted/Inventory", description: "Item component that requests and displays cache locations")]
class SCR_CacheNoteComponentClass : ScriptComponentClass {}

class SCR_CacheNoteComponent : ScriptComponent
{
	[RplProp()]
	protected ref array<string> m_aLines = {};

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.INIT);
		if (!Replication.IsServer())
			return;

		if (!m_aLines.IsEmpty())
			return;

		SCR_CacheManagerComponent manager = SCR_CacheManagerComponent.GetInstance();
		if (!manager)
			return;

		manager.GetRandomCacheData(m_aLines);
		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	array<string> GetNoteLines()
	{
		return m_aLines;
	}
}