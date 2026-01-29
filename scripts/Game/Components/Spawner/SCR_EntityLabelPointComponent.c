[EntityEditorProps(category: "GameScripted/Components", description: "Component labeling entities and tracking their positions updates (if they are in editable composition)")]
class SCR_EntityLabelPointComponentClass : ScriptComponentClass
{
	[Attribute(uiwidget: UIWidgets.SearchComboBox, category: "Entity Spawner", desc: "Entity labels.", enums: ParamEnumArray.FromEnum(EEditableEntityLabel))]
	protected ref array<EEditableEntityLabel> m_aEntityLabels;
	
	//------------------------------------------------------------------------------------------------
	//! Outs all labels assigned to component
	//! \param[out] labels
	//! \return
	bool GetEntityLabels(out notnull array<EEditableEntityLabel> labels)
	{
		if (m_aEntityLabels)
		{
			labels.Clear();
			labels.Copy(m_aEntityLabels);
		}
		
		return m_aEntityLabels != null;
	}
}

class SCR_EntityLabelPointComponent : ScriptComponent
{
	//invokes with labelComponent and owner origin
	protected ref ScriptInvoker m_OnOwnerUpdated;

	//------------------------------------------------------------------------------------------------
	//! \return true if Component has label specified in input
	bool HasLabel(EEditableEntityLabel label)
	{
		SCR_EntityLabelPointComponentClass prefabData = SCR_EntityLabelPointComponentClass.Cast(GetComponentData(GetOwner()));
		if (!prefabData)
			return false;
		
		array<EEditableEntityLabel> labels = {};
		prefabData.GetEntityLabels(labels);
		
		return labels.Contains(label);
	}

	//------------------------------------------------------------------------------------------------
	//! \return invoker called with position updates
	ScriptInvoker GetOnOwnerUpdated()
	{
		if (!m_OnOwnerUpdated)
			m_OnOwnerUpdated = new ScriptInvoker();

		return m_OnOwnerUpdated;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnEntityUpdated(notnull SCR_EditableEntityComponent editableComponent)
	{
		IEntity owner = editableComponent.GetOwner();
		if (!owner)
			return;

		SCR_EntityLabelPointComponent labelComponent = SCR_EntityLabelPointComponent.Cast(owner.FindComponent(SCR_EntityLabelPointComponent));
		if (labelComponent && m_OnOwnerUpdated)
			m_OnOwnerUpdated.Invoke(labelComponent, owner.GetOrigin());
	}

	//------------------------------------------------------------------------------------------------
	protected override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (core)
			core.Event_OnEntityTransformChanged.Insert(OnEntityUpdated);
		else
			Print("SCR_EditableEntityCore not found! SCR_EntityLaberComponent won't be updated!", LogLevel.WARNING);
	}

	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_EntityLabelPointComponent()
	{
		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (core)
			core.Event_OnEntityTransformChanged.Remove(OnEntityUpdated);
	}
}
