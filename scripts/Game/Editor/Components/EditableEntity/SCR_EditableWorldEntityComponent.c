[ComponentEditorProps(category: "GameScripted/Editor (Editables)", description: "", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_EditableWorldEntityComponentClass : SCR_EditableDescriptorComponentClass
{
}

//! @ingroup Editable_Entities

//! Editable world entity, managed by SCR_WorldEntityDelegateEditorComponent.
class SCR_EditableWorldEntityComponent : SCR_EditableDescriptorComponent
{
	GenericEntity m_OwnerLink; //--- Point at world entity this editable entity represents, not to its actual owner
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] owner
	void SetOwnerScripted(GenericEntity owner)
	{
		m_OwnerLink = owner;
	}

	//------------------------------------------------------------------------------------------------
	override GenericEntity GetOwnerScripted()
	{
		return m_OwnerLink;
	}
}
