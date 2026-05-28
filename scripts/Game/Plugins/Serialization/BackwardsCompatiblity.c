[Obsolete("Use SerializationContext instead.")]
class BaseSerializationContext : SerializationContext {}

[Obsolete("Use LoadContext instead.")]
class BaseSerializationLoadContext : LoadContext {}

[Obsolete("Use SaveContext instead.")]
class BaseSerializationSaveContext : SaveContext {}

typedef BufferSaveContext BufferSerializationSaveContext;

[Obsolete("Use LoadContainerContext instead.")]
class ContainerSerializationLoadContext : LoadContainerContext {}

[Obsolete("Use SaveContainerContext instead.")]
class ContainerSerializationSaveContext : SaveContainerContext {}

[Obsolete("Use SCR_PersistenceBinarySaveContext instead.")]
class SCR_PersistenceBinSaveContext : SCR_PersistenceBinarySaveContext {}

[Obsolete("Use SCR_PersistenceBinaryLoadContext instead.")]
class SCR_PersistenceBinLoadContext : SCR_PersistenceBinaryLoadContext {}

[Obsolete("Use BinarySaveContext instead.")]
class SCR_BinSaveContext : BinarySaveContext {}

[Obsolete("Use BinaryLoadContext instead.")]
class SCR_BinLoadContext : BinaryLoadContext {}

[Obsolete("Use JsonSaveContext instead.")]
class SCR_JsonSaveContext : JsonSaveContext 
{
	[Obsolete("Use SaveToString instead.")]
	string ExportToString()
	{
		return SaveToString();
	}
}

[Obsolete("Use JsonLoadContext instead.")]
class SCR_JsonLoadContext : JsonLoadContext 
{
	[Obsolete("Use LoadFromString instead.")]
	bool ImportFromString(string jsonData)
	{
		return LoadFromString(jsonData);
	}
}

[Obsolete("Use SerializationContainer instead.")]
class BaseSerializationContainer : SerializationContainer {}

[Obsolete("Use BaseJsonSaveContainer instead.")]
class BaseJsonSerializationSaveContainer : BaseJsonSaveContainer {}

[Obsolete("Use LoadContainer instead.")]
class BaseSerializationLoadContainer : LoadContainer {}

[Obsolete("Use SaveContainer instead.")]
class BaseSerializationSaveContainer : SaveContainer {}

[Obsolete("Use BinaryLoadContainer instead.")]
class BinSerializationLoadContainer : BinaryLoadContainer {}

[Obsolete("Use BinarySaveContainer instead.")]
class BinSerializationSaveContainer : BinarySaveContainer {}

[Obsolete("Use ScriptedLoadContainer instead.")]
class ScriptedSerializationLoadContainer : ScriptedLoadContainer  {}

[Obsolete("Use ScriptedSaveContainer instead.")]
class ScriptedSerializationSaveContainer : ScriptedSaveContainer {}