class PersistentEntityLinkItem
{
	string StoreName;
	ref BufferSerializationSaveContext Context;
}

class SCR_EditorLinkComponentSerializer : ScriptedComponentSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return SCR_EditorLinkComponent;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationSaveContext context)
	{
		const SCR_EditorLinkComponent editorLink = SCR_EditorLinkComponent.Cast(component);
		const array<IEntity> children = editorLink.GetLinkedChildren();
		if (!children)
			return ESerializeResult.DEFAULT;

		auto entriesContainer = editorLink.GetComponentSource(owner).GetObjectArray("m_aEntries");
		if (children.Count() != entriesContainer.Count())
			return ESerializeResult.ERROR;

		array<ref PersistentEntityLinkItem> changed();
		array<string> removed();

		foreach (int idx, auto child : children)
		{
			if (child && !GetSystem().IsTracked(child))
				continue;

			const BaseContainer container = entriesContainer.Get(idx);
			const string storeName = SCR_ResourceNameUtils.GetPrefabGUID(container.GetResourceName());

			if (!child)
			{
				removed.Insert(storeName);
				continue;
			}

			PersistentEntityLinkItem item();
			item.StoreName = storeName;
			item.Context = BufferSerializationSaveContext.Create(context);
			item.Context.StartObject();

			// Ignore default data from prefab
			ResourceName prefab;
			container.Get("m_Prefab", prefab);
			
			vector prefabPos;
			container.Get("m_vPosition", prefabPos);

			vector prefabAngles;
			container.Get("m_vAngles", prefabAngles);

			// Pitch Yaw Roll from prefab to Yaw Pitch Roll used at runtime
			float yaw = prefabAngles[1];
			prefabAngles[1] = prefabAngles[0];
			prefabAngles[0] = yaw;

			float scale;
			container.Get("m_fScale", scale);

			SerializerDefaultSpawnData defaults();
			defaults.Prefab = prefab;
			Math3D.AnglesToMatrix(prefabAngles, defaults.Transform);
			Math3D.MatrixScale(defaults.Transform, scale);
			defaults.Transform[3] = prefabPos;

			const ESerializeResult result = GetSystem().SerializeEntity(child, item.Context, defaults);
			if (result == ESerializeResult.ERROR)
				return ESerializeResult.ERROR;

			if (result == ESerializeResult.DEFAULT)
				continue;

			item.Context.EndObject();
			changed.Insert(item);
		}

		if (changed.IsEmpty() && removed.IsEmpty())
			return ESerializeResult.DEFAULT;

		context.WriteValue("version", 1);

		if (!changed.IsEmpty() || !context.CanSeekMembers())
		{
			int count = changed.Count();
			context.StartMap("changed", count);
			foreach (PersistentEntityLinkItem item : changed)
			{
				context.WriteMapKey(item.StoreName);
				item.Context.Apply(context);
			}
			context.EndMap();
		}

		if (!removed.IsEmpty() || !context.CanSeekMembers())
			context.Write(removed);

		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationLoadContext context)
	{
		SCR_EditorLinkComponent editorLink = SCR_EditorLinkComponent.Cast(component);

		int version;
		context.ReadValue("version", version);

		map<string, IEntity> linkedChildrenLookup();
		const array<IEntity> children = editorLink.GetLinkedChildren();
		if (!children)
			return false;

		auto entriesContainer = editorLink.GetComponentSource(owner).GetObjectArray("m_aEntries");
		if (children.Count() != entriesContainer.Count())
			return false;

		foreach (int idx, auto child : children)
		{
			const string storeName = SCR_ResourceNameUtils.GetPrefabGUID(entriesContainer.Get(idx).GetResourceName());
			linkedChildrenLookup.Insert(storeName, child);
		}

		int changedCount = 0;
		if (context.StartMap("changed", changedCount))
		{
			string keyBuffer;
			for (int i = 0; i < changedCount; ++i)
			{
				context.ReadMapKey(i, keyBuffer);
				context.StartObject(keyBuffer);
				IEntity changeEntitiy = linkedChildrenLookup.Get(keyBuffer);
				if (changeEntitiy)
				{
					const IEntity resultEnt = GetSystem().DeserializeLoadEntity(changeEntitiy, context, false);
					if (resultEnt != changeEntitiy)
						return false;
				}
				context.EndObject();
			}
			context.EndMap();
		}

		array<string> removed();
		context.Read(removed);
		foreach (auto removeStoreName : removed)
		{
			IEntity removeEntity = linkedChildrenLookup.Get(removeStoreName);
			SCR_EntityHelper.DeleteEntityAndChildren(removeEntity);
		}

		return true;
	}
}
