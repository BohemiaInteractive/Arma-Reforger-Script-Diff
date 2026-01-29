class SCR_EditableEntityComponentSerializer : ScriptedComponentSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return SCR_EditableEntityComponent;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationSaveContext context)
	{
		const SCR_EditableEntityComponent editable = SCR_EditableEntityComponent.Cast(component);

		if (editable.HasEntityFlag(EEditableEntityFlag.NON_SERIALIZABLE))
			return ESerializeResult.DEFAULT;

		IEntity parentEntity;
		const SCR_EditableEntityComponent parent = editable.GetParentEntity();
		if (parent)
		{
			parentEntity = parent.GetOwner();

			// For linked component it would be unnecessary so save the parent id as that is implictly known
			const SCR_EditorLinkComponent editorLink = SCR_EditorLinkComponent.Cast(parentEntity.FindComponent(SCR_EditorLinkComponent));
			if (editorLink && editorLink.HasChild(owner))
				parentEntity = null;
		}

		const UUID parentId = GetSystem().GetId(parentEntity);

		auto slotManager = SCR_CompositionSlotManagerComponent.GetInstance();
		const bool slotted = slotManager && slotManager.IsInSlot(owner);

		UUID author;
		int lastUpdated;
		if (Replication.IsRunning())
		{
			author = editable.GetAuthorUID();
			lastUpdated = editable.GetAuthorLastUpdated();
		}
		else
		{
			author = UUID.NULL_UUID;
		}

		if (parentId.IsNull() && !slotted && author.IsNull())
			return ESerializeResult.DEFAULT;

		context.WriteValue("version", 1);
		context.WriteDefault(parentId, UUID.NULL_UUID);
		context.WriteDefault(slotted, false);
		context.WriteDefault(author, UUID.NULL_UUID);
		if (!author.IsNull())
			context.WriteDefault(lastUpdated, 0);

		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationLoadContext context)
	{
		SCR_EditableEntityComponent editable = SCR_EditableEntityComponent.Cast(component);

		int version;
		context.Read(version);

		UUID parentId;
		context.ReadDefault(parentId, UUID.NULL_UUID);
		if (!parentId.IsNull())
		{
			Tuple1<SCR_EditableEntityComponent> parentContext(editable);
			PersistenceWhenAvailableTask parentTask(OnParentAvailable, parentContext);
			GetSystem().WhenAvailable(parentId, parentTask);
		}

		bool slotted;
		context.ReadDefault(slotted, false);
		if (slotted)
		{
			auto slotManager = SCR_CompositionSlotManagerComponent.GetInstance();
			if (slotManager)
				slotManager.SetOccupant(owner.GetOrigin(), owner);
		}

		UUID author;
		context.ReadDefault(author, UUID.NULL_UUID);
		if (!author.IsNull())
		{
			int lastUpdated;
			context.ReadDefault(lastUpdated, 0);

			SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
			if (core)
			{
				SCR_EditableEntityAuthor data = core.FindAuthorByIdentity(author);
				if (data)
				{
					data.m_iEntityCount++;
					editable.SetAuthor(data);
					editable.SetAuthorUpdatedTime(lastUpdated);
				}
			}
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected static void OnParentAvailable(Managed instance, PersistenceDeferredDeserializeTask task, bool expired, Managed context)
	{
		auto parentEntity = IEntity.Cast(instance);
		if (!parentEntity)
			return;

		SCR_EditableEntityComponent parentEditable = SCR_EditableEntityComponent.Cast(parentEntity.FindComponent(SCR_EditableEntityComponent));
		if (!parentEditable)
			return;

		auto parentContext = Tuple1<SCR_EditableEntityComponent>.Cast(context);
		if (parentContext.param1)
			parentEditable.SetParentEntity(parentContext.param1);
	}
}
