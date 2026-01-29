[ComponentEditorProps(category: "GameScripted/Editor (Editables)", description: "", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_EditableWaypointComponentClass : SCR_EditableEntityComponentClass
{
}

//! @ingroup Editable_Entities

//! Special configuration for editable waypoint.
class SCR_EditableWaypointComponent : SCR_EditableEntityComponent
{
	[RplProp(onRplName: "OnPreWaypointIdRpl")]
	protected RplId m_PrevWaypointId;

	[RplProp()]
	protected bool m_bIsCurrent;

	[RplProp()]
	protected int m_iIndex;

	protected SCR_EditableEntityComponent m_Group;
	protected SCR_EditableEntityComponent m_PrevWaypoint;

	protected SCR_AIWaypoint m_AIWaypoint;
	protected SCR_EditableEntityComponent m_AttachedTo;

	[RplProp(onRplName: "OnAttachedToId")]
	protected RplId m_AttachedToId;

	//------------------------------------------------------------------------------------------------
	override SCR_EditableEntityComponent GetAttachedTo()
	{
		return m_AttachedTo;
	}

	//------------------------------------------------------------------------------------------------
	//! Attaches an entity to this waypoint task
	//! \param[in] attachedTo to which component attach this waypoint - can be null to detach it
	protected void AttachTo(SCR_EditableEntityComponent attachedTo)
	{
		m_AttachedTo = attachedTo;

		//++ Remove from the existing parent first
		if (GetOwner().GetParent())
			m_AIWaypoint.GetParent().RemoveChild(m_AIWaypoint, true);

		//++ Add to the new parent (when defined)
		if (m_AttachedTo)
		{
			//++ Do not allow to attach if target exceeds attached entities number
			if (m_AttachedTo.GetAttachedEntities().Count() >= EditorConstants.MAX_ATTACHED_ENTITIES)
			{
				SCR_NotificationsComponent.SendLocal(ENotification.EDITOR_PLACING_BLOCKED, SCR_PlayerController.GetLocalPlayerId());
				return;
			}
			
			m_AIWaypoint.SetOrigin(vector.Zero);
			m_AttachedTo.GetOwner().AddChild(m_AIWaypoint, -1);

			SCR_EntityWaypoint entityWaypoint = SCR_EntityWaypoint.Cast(m_AIWaypoint);
			if (entityWaypoint)
				entityWaypoint.SetEntity(m_AttachedTo.GetOwner());

			m_AttachedTo.Attach(this);
		}
		else
		{
			SCR_EntityWaypoint entityWaypoint = SCR_EntityWaypoint.Cast(m_AIWaypoint);
			if (entityWaypoint)
				entityWaypoint.SetEntity(null);
			if (m_AttachedTo)
				m_AttachedTo.Detach(this);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Runs when an ID is attached
	protected void OnAttachedToId()
	{
		SCR_EditableEntityComponent attachedTo = SCR_EditableEntityComponent.Cast(Replication.FindItem(m_AttachedToId));
		AttachTo(attachedTo);
	}

	//------------------------------------------------------------------------------------------------
	//! Assign order index of the waypoint and whether it's current or not.
	//! \param[in] index Order in group's waypoints, starting with 0
	//! \param[in] isCurrent True if the waypoint is current
	//! \param[in] prevWaypoint
	void SetWaypointIndex(int index, bool isCurrent, SCR_EditableEntityComponent prevWaypoint)
	{
		m_iIndex = index;
		m_bIsCurrent = isCurrent;
		m_PrevWaypoint = prevWaypoint;
		m_PrevWaypointId = Replication.FindId(prevWaypoint);
		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	//! Get order index of the waypoint.
	//! \return Index starting with 0
	int GetWaypointIndex()
	{
		return m_iIndex;
	}

	//------------------------------------------------------------------------------------------------
	//! Check if the waypoint is group's current waypoint.
	//! \return True when current
	bool IsCurrent()
	{
		return m_bIsCurrent;
	}

	//------------------------------------------------------------------------------------------------
	//! Get previous waypoint.
	//! \return Waypoint or group (when there is no previous waypoint)
	SCR_EditableEntityComponent GetPrevWaypoint()
	{
		if (m_PrevWaypoint)
			return m_PrevWaypoint;
		else
			return m_Group;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPreWaypointIdRpl()
	{
		m_PrevWaypoint = SCR_EditableEntityComponent.Cast(Replication.FindItem(m_PrevWaypointId));
	}

	//------------------------------------------------------------------------------------------------
	override bool CanDuplicate(out notnull set<SCR_EditableEntityComponent> outRecipients)
	{
		SCR_EditableEntityComponent groupComponent = GetAIGroup();
		if (groupComponent)
			outRecipients.Insert(groupComponent);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override Faction GetFaction()
	{
		if (!m_Group)
			return null;

		Faction faction = m_Group.GetFaction();
		return faction;
	}

	//------------------------------------------------------------------------------------------------
	override SCR_EditableEntityComponent GetAIGroup()
	{
		return m_Group;
	}

	//------------------------------------------------------------------------------------------------
	override void OnParentEntityChanged(SCR_EditableEntityComponent parentEntity, SCR_EditableEntityComponent parentEntityPrev, bool changedByUser)
	{
		if (!parentEntity)
			return;
		
		
		if (m_AIWaypoint && m_Group && m_Group != parentEntity)
		{
			AttachTo(parentEntity);

			m_AttachedToId = Replication.FindId(parentEntity);
			Replication.BumpMe();
			
			super.OnParentEntityChanged(parentEntityPrev, parentEntityPrev, changedByUser); // Needed to register
			return;
		}

		EEditableEntityType parentType = parentEntity.GetEntityType();
		switch (parentType)
		{
			case EEditableEntityType.CHARACTER:
			case EEditableEntityType.GROUP:
			{
				SCR_EditableEntityComponent group = parentEntity.GetAIGroup();
				if (!group)
					break;

				m_Group = group;
				super.OnParentEntityChanged(group, parentEntityPrev, changedByUser);
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override void SetTransform(vector transform[4], bool changedByUser = false)
	{
		DetachFromTarget();
		super.SetTransform(transform, changedByUser);
	}

	//------------------------------------------------------------------------------------------------
	override void SetTransformBroadcast(vector transform[4])
	{
		DetachFromTarget();
		super.SetTransformBroadcast(transform);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Detach the waypoint from the target, runs both in client and server
	protected void DetachFromTarget()
	{
		if (m_AIWaypoint)
			AttachTo(null);
	}

	//------------------------------------------------------------------------------------------------
	override bool GetPos(out vector pos)
	{
		if (m_Group)
			return super.GetPos(pos);
		else
			return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanSetParent(SCR_EditableEntityComponent parentEntity)
	{
		//--- When the group was not assigned yet and the target is a GROUP or a CHARACTER, always allow them to be a parent
		if (!m_Group)
		{
			switch (parentEntity.GetEntityType())
			{
				case EEditableEntityType.GROUP:
				case EEditableEntityType.CHARACTER:
				{
					return true;
				}
			}
		}

		return super.CanSetParent(parentEntity);
	}

	//------------------------------------------------------------------------------------------------
	override SCR_EditableEntityComponent EOnEditorPlace(out SCR_EditableEntityComponent parent, SCR_EditableEntityComponent recipient, EEditorPlacingFlags flags, bool isQueue, int playerID = 0)
	{
		if (recipient)
		{
			AIGroup group = AIGroup.Cast(recipient.GetOwner());
			if (group)
			{
				if (!isQueue)
				{
					SCR_EditableGroupComponent editableGroup = SCR_EditableGroupComponent.Cast(recipient);
					if (editableGroup && !editableGroup.AreCycledWaypointsEnabled())
						editableGroup.ClearWaypoints();
				}

				group.AddWaypoint(AIWaypoint.Cast(GetOwner()));
			}
		}
		return this;
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		super.OnDelete(owner);

		//--- Manually remove the waypoint entity from group's list of waypoints - it doesn't happen automatically
		if (m_Group && IsServer())
		{
			AIGroup aiGroup = AIGroup.Cast(m_Group.GetOwner());
			if (aiGroup)
				aiGroup.RemoveWaypoint(AIWaypoint.Cast(owner));
		}
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
	
	//------------------------------------------------------------------------------------------------
	SCR_AIWaypoint GetAIWaypoint()
	{
		return m_AIWaypoint;
	}

	//------------------------------------------------------------------------------------------------
	void SCR_EditableWaypointComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_AIWaypoint = SCR_AIWaypoint.Cast(ent);
	}

}
