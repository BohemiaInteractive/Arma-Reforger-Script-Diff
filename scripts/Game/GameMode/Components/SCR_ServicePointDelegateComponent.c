class SCR_ServicePointDelegateComponentClass : ScriptComponentClass
{
}

// Holds UI-related info for service points so they can handle the data even after the service itself got streamed out
class SCR_ServicePointDelegateComponent : ScriptComponent
{
	[Attribute(SCR_EServicePointType.SUPPLY_DEPOT.ToString(), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_EServicePointType))]
	protected SCR_EServicePointType m_eType;

	[Attribute(defvalue: "0", uiwidget: UIWidgets.SearchComboBox, desc: "Type", enums: ParamEnumArray.FromEnum(EEditableEntityLabel))]
	protected EEditableEntityLabel m_eBuildingLabel;

	protected SCR_MilitaryBaseComponent m_ParentBase;

	[RplProp(onRplName: "OnParentBaseIdSet")]
	protected int m_iParentBaseId;

	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_EServicePointType GetType()
	{
		return m_eType;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	EEditableEntityLabel GetLabel()
	{
		return m_eBuildingLabel;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] id
	void SetParentBaseId(RplId id)
	{
		m_iParentBaseId = id;
		Replication.BumpMe();
		OnParentBaseIdSet();
	}

	//------------------------------------------------------------------------------------------------
	void OnParentBaseIdSet()
	{
		m_ParentBase = SCR_MilitaryBaseComponent.Cast(Replication.FindItem(m_iParentBaseId));

		if (!m_ParentBase)
			return;

		m_ParentBase.RegisterServiceDelegate(this);

		if (RplSession.Mode() == RplMode.Dedicated)
			return;

		SCR_ServicePointMapDescriptorComponent mapDescr = SCR_ServicePointMapDescriptorComponent.Cast(GetOwner().FindComponent(SCR_ServicePointMapDescriptorComponent));

		if (!mapDescr)
			return;

		mapDescr.SetParentBase(m_ParentBase);
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		super.OnDelete(owner);

		if (m_ParentBase)
			m_ParentBase.UnregisterServiceDelegate(this);
	}
}
