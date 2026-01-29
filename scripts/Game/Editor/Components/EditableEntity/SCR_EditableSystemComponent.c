[ComponentEditorProps(category: "GameScripted/Editor (Editables)", description: "", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_EditableSystemComponentClass : SCR_EditableEntityComponentClass
{
}

//! @ingroup Editable_Entities

//! Editable entity which can contain location description.
class SCR_EditableSystemComponent : SCR_EditableEntityComponent
{
	protected SCR_FactionAffiliationComponent m_FactionAffiliationComponent;
	protected ref ScriptInvoker Event_OnUIRefresh = new ScriptInvoker();
	
	//------------------------------------------------------------------------------------------------
	protected void OnFactionChanged(FactionAffiliationComponent owner, Faction previousFaction, Faction newFaction)
	{
		Event_OnUIRefresh.Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	override Faction GetFaction()
	{
		if (m_FactionAffiliationComponent)
			return m_FactionAffiliationComponent.GetAffiliatedFaction();

		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	override ScriptInvoker GetOnUIRefresh()
	{
		return Event_OnUIRefresh;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		m_FactionAffiliationComponent = SCR_FactionAffiliationComponent.Cast(owner.FindComponent(SCR_FactionAffiliationComponent));
		if (m_FactionAffiliationComponent)
			m_FactionAffiliationComponent.GetOnFactionChanged().Insert(OnFactionChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	// destructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void ~SCR_EditableSystemComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		if (m_FactionAffiliationComponent)
			m_FactionAffiliationComponent.GetOnFactionChanged().Insert(OnFactionChanged);
	}
}
