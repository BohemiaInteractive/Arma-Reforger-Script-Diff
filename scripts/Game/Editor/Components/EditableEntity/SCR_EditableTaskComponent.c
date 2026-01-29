[ComponentEditorProps(category: "GameScripted/Editor (Editables)", description: "", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_EditableTaskComponentClass: SCR_EditableDescriptorComponentClass
{
	[Attribute("#AR-Tasks_Objective", desc: "Name of objective type eg: Attack objective. Used among in, among other things, in notifications", category: "Visualization")]
	protected  LocalizedString m_sObjectiveTypeName;
	
	//------------------------------------------------------------------------------------------------
	//! Get objective type name. The name is the same for each prefab objective type
	//! \return Objective type name
	LocalizedString GetObjectiveTypeName()
	{
		return m_sObjectiveTypeName;
	}
}

//! @ingroup Editable_Entities

//! Editable SCR_BaseTask.
class SCR_EditableTaskComponent: SCR_EditableDescriptorComponent
{	
	protected SCR_EditorTask m_Task;
	protected Faction m_TargetFaction;
	protected int m_iTextIndex;
	
	protected SCR_AttachableTask m_AttachableTask;
	protected SCR_EditableEntityComponent m_AttachedTo;
	
	[RplProp(onRplName: "OnAttachedToId")]
	protected RplId m_AttachedToId;
	
	
	//------------------------------------------------------------------------------------------------
	//! Attaches a entity to this editable task
	//! \param[in] attachedTo to which component attach this task - can be null to detach it
	protected void AttachTo(SCR_EditableEntityComponent attachedTo)
	{	
		//++ Remove from the existing parent first
		if (GetOwner().GetParent())
			m_AttachableTask.GetParent().RemoveChild(m_AttachableTask, true);
		
		
		//++ Add to the new parent (when defined)
		if (attachedTo)
		{
				
			//++ Do not allow to attach if target exceeds attached entities number
			if (attachedTo.GetAttachedEntities().Count() >= EditorConstants.MAX_ATTACHED_ENTITIES)
			{
				SCR_NotificationsComponent.SendLocal(ENotification.EDITOR_PLACING_BLOCKED, SCR_PlayerController.GetLocalPlayerId());
				return;
			}
			
			m_AttachableTask.SetOrigin(vector.Zero);
			attachedTo.GetOwner().AddChild(m_AttachableTask, -1);
			m_AttachableTask.SetTarget(attachedTo);
			attachedTo.Attach(this);
		}
		else
		{
			m_AttachableTask.SetTarget(null);
			if (m_AttachedTo)
				m_AttachedTo.Detach(this);
		}
		
		m_AttachedTo = attachedTo;
		m_NearestLocation = m_AttachedTo;
		UpdateText();
	}
	
	
	//------------------------------------------------------------------------------------------------
	//! Runs when an ID is attached
	protected void OnAttachedToId()
	{
		SCR_EditableEntityComponent attachedTo = SCR_EditableEntityComponent.Cast(Replication.FindItem(m_AttachedToId));
		AttachTo(attachedTo);
	}
	
	//------------------------------------------------------------------------------------------------
	override SCR_EditableEntityComponent GetAttachedTo()
	{
		return m_AttachedTo;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get type of custom texts this task should use.
	//! \return Type of texts
	ETaskTextType GetTextType()
	{
		return m_Task.GetTextType();
	}

	//------------------------------------------------------------------------------------------------
	//! Get index of custom text from SCR_TextsTaskManagerComponent.
	//! \return Index from the array of texts
	int GetTextIndex()
	{
		return m_iTextIndex;
	}

	//------------------------------------------------------------------------------------------------
	//! Set index of custom text from SCR_TextsTaskManagerComponent.
	//! \param[in] index Index from the array of texts
	void SetTextIndex(int index)
	{
		SetTextIndexBroadcast(index);
		Rpc(SetTextIndexBroadcast, index);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void SetTextIndexBroadcast(int index)
	{
		m_iTextIndex = index;
		m_Task.SetTextIndex(index);
		UpdateText();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get the task completion type
	//! \return MANUAL and ALWAYS_MENUAL means only the GM can complete the task. AUTOMATIC means that the task can auto complete and/or fail depending on the task
	EEditorTaskCompletionType GetTaskCompletionType()
	{
		return m_Task.GetTaskCompletionType();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set the task completion type
	//! \param[in] completionType MANUAL and ALWAYS_MENUAL means only the GM can complete the task. AUTOMATIC means that the task can auto complete and/or fail depending on the task
	void SetTaskCompletionType(EEditorTaskCompletionType completionType)
	{
		if (m_Task.GetTaskCompletionType() == completionType)
			return;
		
		SetTaskCompletionTypeBroadcast(completionType);
		Rpc(SetTaskCompletionTypeBroadcast, completionType);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void SetTaskCompletionTypeBroadcast(EEditorTaskCompletionType completionType)
	{
		m_Task.SetTaskCompletionType(completionType);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateText()
	{
		UpdateInfo(m_UIInfoDescriptor);
		
		m_Task.SetTextIndex(m_iTextIndex);
		m_Task.SetLocationName(m_UIInfoDescriptor.GetLocationName());
		
		SCR_TextsTaskManagerComponent textsComponent = SCR_TextsTaskManagerComponent.GetInstance();
		if (!textsComponent)
			return;
		
		SCR_UIDescription info = textsComponent.GetText(m_Task.GetTextType(), m_Task.GetTextIndex());
		m_Task.SetTaskName(info.GetUnformattedName(), {m_Task.GetLocationName()});
		m_Task.SetTaskDescription(info.GetUnformattedDescription());
		m_Task.SetTaskIconPath(m_UIInfoDescriptor.GetIconPath());
		m_Task.SetTaskIconSetName(m_UIInfoDescriptor.GetIconSetName());
		
		UpdateInfo(m_Task.GetTaskUIInfo());
	}

	//------------------------------------------------------------------------------------------------
	override protected void GetOnLocationChange(SCR_EditableEntityComponent nearestLocation)
	{
		UpdateText();
	}
	
	//------------------------------------------------------------------------------------------------
	override Faction GetFaction()
	{
		return m_TargetFaction;
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetTransform(vector transform[4], bool changedByUser = false)
	{	
		//--- If it is moved by the user and has attached task, unlink it
		
		if (m_AttachableTask && changedByUser)
			AttachTo(null);
		
		super.SetTransform(transform, changedByUser);
		UpdateNearestLocation();
	}
	
	/*
	//------------------------------------------------------------------------------------------------
	override bool Serialize(out SCR_EditableEntityComponent outTarget = null, out int outTargetIndex = -1, out EEditableEntitySaveFlag outSaveFlags = 0)
	{
		outTargetIndex = GetGame().GetFactionManager().GetFactionIndex(m_TargetFaction);
		return super.Serialize(outTarget, outTargetIndex, outSaveFlags);
	}

	//------------------------------------------------------------------------------------------------
	override void Deserialize(SCR_EditableEntityComponent target, int targetValue)
	{
		super.Deserialize(target, targetValue);
		
		SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
		if (!taskSystem)
			return;
		
		m_TargetFaction = GetGame().GetFactionManager().GetFactionByIndex(targetValue);
		taskSystem.AddTaskFaction(m_Task, m_TargetFaction.GetFactionKey());
	}
	*/

	//------------------------------------------------------------------------------------------------
	override ScriptInvoker GetOnUIRefresh()
	{
		return Event_OnUIRefresh;
	}

	//------------------------------------------------------------------------------------------------
	override bool RplSave(ScriptBitWriter writer)
	{
		if (!super.RplSave(writer))
			return false;
		
		int factionIndex;
		if (GetGame().GetFactionManager())
			factionIndex = GetGame().GetFactionManager().GetFactionIndex(m_TargetFaction);
		
		writer.WriteInt(factionIndex);
		writer.WriteInt(m_iTextIndex);
		writer.WriteRplId(m_AttachedToId);
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		if (!super.RplLoad(reader))
			return false;
		
		int factionIndex;
		reader.ReadInt(factionIndex);
		if (GetGame().GetFactionManager())
			m_TargetFaction = GetGame().GetFactionManager().GetFactionByIndex(factionIndex);
		
		reader.ReadInt(m_iTextIndex);
		reader.ReadRplId(m_AttachedToId);
		OnAttachedToId();
		
		m_Task.SetTextIndex(m_iTextIndex);
		
		UpdateText();
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override SCR_EditableEntityComponent EOnEditorPlace(out SCR_EditableEntityComponent parent, SCR_EditableEntityComponent recipient, EEditorPlacingFlags flags, bool isQueue, int playerID = 0)
	{
		if (recipient)
		{
			m_TargetFaction = recipient.GetFaction();
			if (m_TargetFaction)
			{
				//--- When the task is placed as inactive, don't assign faction yet, do it only upon manual activation
				//if (!SCR_Enum.HasFlag(flags, EEditorPlacingFlags.TASK_INACTIVE))
					//m_Task.SetTargetFaction(m_TargetFaction);
				
				UpdateNearestLocation();
				
				m_Task.SetTaskOwnership(SCR_ETaskOwnership.FACTION);
				m_Task.SetTaskVisibility(SCR_ETaskVisibility.FACTION);
				m_Task.AddOwnerFactionKey(m_TargetFaction.GetFactionKey());
			}
		}
		return this;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnParentEntityChanged(SCR_EditableEntityComponent parentEntity, SCR_EditableEntityComponent parentEntityPrev, bool changedByUser)
	{
		if (m_AttachableTask)
		{
			AttachTo(parentEntity);
			
			m_AttachedToId = Replication.FindId(parentEntity);
			Replication.BumpMe();
			
			super.OnParentEntityChanged(null, null, changedByUser); //--- Needed to continue entity registration
		}
		else
		{
			super.OnParentEntityChanged(parentEntity, parentEntityPrev, changedByUser);
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_EditableTaskComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_Task = SCR_EditorTask.Cast(ent);
		m_AttachableTask = SCR_AttachableTask.Cast(ent);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool Delete(bool changedByUser = false, bool updateNavmesh = true)
	{
		if (m_Task)
			m_Task.ShowTaskNotification(ENotification.EDITOR_TASK_DELETED, true);
		
		return super.Delete(changedByUser, updateNavmesh);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool IsAttachable()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool IsAttached()
	{
		if (GetAttachedTo())
			return true;
		
		return false;
	}
}
