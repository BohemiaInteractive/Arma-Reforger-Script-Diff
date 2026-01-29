[ComponentEditorProps(category: "GameScripted/Editor (Editables)", description: "", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_EditablePreviewComponentClass : SCR_EditableEntityComponentClass
{
}

//! @ingroup Editable_Entities

//! Special configuration to show icon of SCR_BasePreviewEntity ghost preview.
class SCR_EditablePreviewComponent : SCR_EditableEntityComponent
{
	protected SCR_EditableEntityComponent m_SourceEntity;
	protected EEditableEntityType m_EntityTypeInternal;
	protected ref SCR_UIInfo m_UIInfoInternal;
	
	//------------------------------------------------------------------------------------------------
	//! Initialise the entity from another entity (copy its visualization).
	//! \param[in] from Original entity
	void InitFromEntity(SCR_EditableEntityComponent from)
	{
		if (!from || !from.IsRegistered())
			return;
		
		m_SourceEntity = from;
		m_EntityTypeInternal = from.GetEntityType();
		m_vIconPos = from.m_vIconPos;
		m_iIconBoneIndex = from.m_iIconBoneIndex;
		RestoreParentEntity();
	}

	//------------------------------------------------------------------------------------------------
	//! Initialise the entity from entity source.
	//! \param[in] from Entity source
	void InitFromSource(IEntityComponentSource from)
	{
		if (!from)
			return;
		
		m_UIInfoInternal = SCR_EditableEntityComponentClass.GetInfo(from);
		from.Get("m_EntityType", m_EntityTypeInternal);
		from.Get("m_vIconPos", m_vIconPos);
		
		string iconBoneName;
		from.Get("m_sIconBoneName", iconBoneName);
		if (m_Owner.GetAnimation())
			m_iIconBoneIndex = m_Owner.GetAnimation().GetBoneIndex(iconBoneName);
		else
			m_iIconBoneIndex = -1;
		
		RestoreParentEntity();
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetPos(out vector pos)
	{
		if (m_Owner && (m_Owner.GetFlags() & EntityFlags.VISIBLE))
			return super.GetPos(pos);
		else
			return false;
	}

	//------------------------------------------------------------------------------------------------
	override void UpdateGameHierarchy(IEntity parent, IEntity child, bool toAdd)
	{
		//--- Game hierarchy is managed by SCR_BasePreviewEntity
	}

	//------------------------------------------------------------------------------------------------
	override EEditableEntityType GetEntityType(IEntity owner = null)
	{
		return m_EntityTypeInternal;
	}

	//------------------------------------------------------------------------------------------------
	override SCR_UIInfo GetInfo(IEntity owner = null)
	{
		if (m_UIInfoInternal)
			return m_UIInfoInternal;
		
		SCR_UIInfo info;
		if (m_SourceEntity)
			info = m_SourceEntity.GetInfo(owner);
		else
			info = super.GetInfo(owner);
		
		return info;
	}
}
