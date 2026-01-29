[ComponentEditorProps(category: "GameScripted/Editor (Editables)", description: "")]
class SCR_EditableEntityMaterialChildComponentClass: SCR_EditableEntityBaseChildComponentClass
{
}

//! @ingroup Editable_Entities

//! Change material based on specific states
class SCR_EditableEntityMaterialChildComponent : SCR_EditableEntityBaseChildComponent
{
	//[Attribute()]
	//private ref array<ref SCR_EditableEntityMaterialChildState> m_States;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "", "emat")]
	private ResourceName m_DefaultMaterial;
	
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "", "emat")]
	private ResourceName m_StateMaterial;
	
	//------------------------------------------------------------------------------------------------
	protected void SetMaterial(ResourceName material)
	{
		if (!m_Owner)
			return;
		
		SCR_VirtualAreaEntity area = SCR_VirtualAreaEntity.Cast(m_Owner);
		if (area)
		{
			area.SetMaterial(material);
			return;
		}
		VObject obj = m_Owner.GetVObject();
		string remap = "";
		string materials[256];
		int numMats = obj.GetMaterials(materials);
		for (int i = 0; i < numMats; i++)
		{
			remap += string.Format("$remap '%1' '%2';", materials[i], material);
		}
		m_Owner.SetObject(obj, remap);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnStateChanged(EEditableEntityState states, EEditableEntityState changedState, bool toSet)
	{
		if ((states & m_State) > 0)
			SetMaterial(m_StateMaterial);
		else
			SetMaterial(m_DefaultMaterial);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		if (SCR_Global.IsEditMode(owner))
			return; //--- Run-time only

		//SetMaterial(m_DefaultMaterial);
		UpdateFromCurrentState();
	}
}
