[ComponentEditorProps(category: "GameScripted/Editor (Editables)", description: "", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_EditableSpawnPointComponentClass: SCR_EditableDescriptorComponentClass
{
}

//! @ingroup Editable_Entities

//! Editable SCR_SpawnPoint.
class SCR_EditableSpawnPointComponent : SCR_EditableDescriptorComponent
{
	protected SCR_SpawnPoint m_SpawnPoint;
	protected ref SCR_UIInfo m_UIInfoSpawnPoint;
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateText()
	{
		UpdateInfo();
		
		m_UIInfoSpawnPoint = SCR_UIInfo.CreateInfo(m_UIInfoDescriptor.GetLocationName()); //--- Simplified UI info containing only location name
		m_SpawnPoint.LinkInfo(m_UIInfoSpawnPoint);
	}

	//------------------------------------------------------------------------------------------------
	override protected void GetOnLocationChange(SCR_EditableEntityComponent nearestLocation)
	{
		UpdateText();
		
		FactionAffiliationComponent factionAffiliation = FactionAffiliationComponent.Cast(m_Owner.FindComponent(FactionAffiliationComponent));
		if (factionAffiliation)
			factionAffiliation.SetAffiliatedFaction(GetFaction());
	}

	//------------------------------------------------------------------------------------------------
	override void SetTransform(vector transform[4], bool changedByUser = false)
	{	
		super.SetTransform(transform, changedByUser);
		UpdateNearestLocation();
	}

	//------------------------------------------------------------------------------------------------
	override ScriptInvoker GetOnUIRefresh()
	{
		return Event_OnUIRefresh;
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_EditableSpawnPointComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_SpawnPoint = SCR_SpawnPoint.Cast(ent);
	}
}
