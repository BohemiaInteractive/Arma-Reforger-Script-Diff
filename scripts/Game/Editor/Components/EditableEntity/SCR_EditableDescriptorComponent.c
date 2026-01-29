void ScriptInvoker_EditableDescriptorLocationChangeMethod(SCR_EditableCommentComponent location);
typedef func ScriptInvoker_EditableDescriptorLocationChangeMethod;
typedef ScriptInvokerBase<ScriptInvoker_EditableDescriptorLocationChangeMethod> ScriptInvoker_EditableDescriptorLocationChange;

[ComponentEditorProps(category: "GameScripted/Editor (Editables)", description: "", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_EditableDescriptorComponentClass: SCR_EditableSystemComponentClass
{
}

//! @ingroup Editable_Entities

//! Editable entity which can contain location description.
class SCR_EditableDescriptorComponent : SCR_EditableSystemComponent
{
	[Attribute("300", UIWidgets.Slider, "Detect locations within this radius.", params: "0 10000 1")]
	protected float m_fMaxLocationSize;
	
	protected LocalizedString m_sLocationName;
	protected EntityID m_NearestLocationID;
	protected SCR_EditableEntityComponent m_NearestLocation;
	protected ref SCR_EditableDescriptorUIInfo m_UIInfoDescriptor;
	protected ref ScriptInvoker_EditableDescriptorLocationChange m_OnChange;

	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvoker_EditableDescriptorLocationChange GetOnChange()
	{
		if (!m_OnChange)
			m_OnChange = new ScriptInvoker_EditableDescriptorLocationChange();

		return m_OnChange;
	}

	//------------------------------------------------------------------------------------------------
	//! Update location name.
	//! \param pos Position from which the nearest location will be searched for.
	//! \return True if there was a change
	bool UpdateNearestLocation(vector pos = vector.Zero)
	{
		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (!core)
			return false;
		
		//--- Find nearest location
		if (pos == vector.Zero)
			pos = GetOwner().GetOrigin();
		
		EntityID nearestLocationID;
		SCR_EditableEntityComponent nearestLocation = core.FindNearestEntity(pos, EEditableEntityType.COMMENT, EEditableEntityFlag.LOCAL);
		if (nearestLocation)
		{
			//--- Check if it's close enough
			vector locationPos;
			if (nearestLocation.GetPos(locationPos) && vector.DistanceSqXZ(locationPos, pos) < m_fMaxLocationSize)
			{
				//--- Terminate if the location didn't change
				nearestLocationID = nearestLocation.GetOwner().GetID();
				if (nearestLocationID == m_NearestLocationID)
					return false;
			}
		}
		
		//--- Update
		UpdateNearestLocationBroadcast(nearestLocationID);
		Rpc(UpdateNearestLocationBroadcast, nearestLocationID);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void UpdateNearestLocationBroadcast(EntityID nearestLocationID)
	{	
		m_NearestLocationID = nearestLocationID;
		
		IEntity nearestLocationEntity = GetGame().GetWorld().FindEntityByID(m_NearestLocationID);
		m_NearestLocation = SCR_EditableEntityComponent.GetEditableEntity(nearestLocationEntity);
		
		GetOnLocationChange(m_NearestLocation);
		if (m_OnChange)
			m_OnChange.Invoke(SCR_EditableCommentComponent.Cast(m_NearestLocation));
	}

	//------------------------------------------------------------------------------------------------
	protected void GetOnLocationChange(SCR_EditableEntityComponent nearestLocation);

	//------------------------------------------------------------------------------------------------
	protected void UpdateInfo(SCR_UIDescription from = null)
	{
		if (!m_UIInfoDescriptor)
		{
			m_UIInfoDescriptor = new SCR_EditableDescriptorUIInfo();
			m_UIInfoDescriptor.CopyFrom(GetInfo());
			SetInfoInstance(m_UIInfoDescriptor);
		}
		
		//--- Copy custom texts
		if (from)
			m_UIInfoDescriptor.CopyFrom(from);
		
		if (m_NearestLocation)
			m_UIInfoDescriptor.SetLocationName(m_NearestLocation.GetInfo().GetName());
		else
			m_UIInfoDescriptor.SetLocationName(GetOwner().GetOrigin());
		
		Event_OnUIRefresh.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	override bool RplSave(ScriptBitWriter writer)
	{
		if (!super.RplSave(writer))
			return false;
		
		writer.WriteEntityId(m_NearestLocationID);
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		if (!super.RplLoad(reader))
			return false;
		
		reader.ReadEntityId(m_NearestLocationID);
		
		UpdateNearestLocationBroadcast(m_NearestLocationID);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_EditableDescriptorComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_fMaxLocationSize *= m_fMaxLocationSize;
	}
}
