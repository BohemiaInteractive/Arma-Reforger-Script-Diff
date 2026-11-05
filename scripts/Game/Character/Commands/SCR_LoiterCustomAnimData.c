[BaseContainerProps()]
class SCR_LoiterCustomAnimData : SCR_CustomAnimData_Properties
{
	const static ref SCR_LoiterCustomAnimData Default = new SCR_LoiterCustomAnimData();
	//! 0x7fff is ENF_MAX_INT_RAND value and this is half of it - 1, so it is below max enfusion random range
	protected const int SEED_RANGE = 16382;
	static ref SCR_LoiterItemPresets s_LoiterItemPresetContainer;

	//! list of items which were spawned locally for the playback of this animation
	protected ref array<IEntity> m_aItems;

	protected int m_iSeed;

	protected const ResourceName LOITER_ITEM_PRESETS_CONFIG = "{911269C6C40E808B}Configs/Character/LoiterItemPresets.conf";

	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_LoiterCustomAnimData()
	{
		ArmaReforgerScripted game = GetGame();
		if (!game || !game.InPlayMode())
			return;

		if (s_LoiterItemPresetContainer)
			return;

		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(game.GetGameMode());
		if (!gameMode)
			return;

		Resource configRes = Resource.Load(LOITER_ITEM_PRESETS_CONFIG);
		if (!configRes.IsValid())
			return;

		s_LoiterItemPresetContainer = SCR_LoiterItemPresets.Cast(BaseContainerTools.CreateInstanceFromContainer(configRes.GetResource().ToBaseContainer()));
		if (!s_LoiterItemPresetContainer)
			return;

		gameMode.GetOnGameEnd().Insert(ReleaseData);
	}

	//------------------------------------------------------------------------------------------------
	//! Clears static data to ensure that it doesnt persist over multiple sessions
	protected static void ReleaseData()
	{
		s_LoiterItemPresetContainer = null;
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		gameMode.GetOnGameEnd().Remove(ReleaseData);
	}

	//------------------------------------------------------------------------------------------------
	override bool IsDefault()
	{
		return super.IsDefault() && m_eItemPresetID == SCR_EAnimationItemID.NONE;
	}

	//------------------------------------------------------------------------------------------------
	//! Spawns local items for the playback of this animation
	//! \param[in] animationOwner
	void SpawnItem(notnull IEntity animationOwner)
	{
		if (m_eItemPresetID == SCR_EAnimationItemID.NONE)
			return;

		SCR_LoiterItemDataBase data = s_LoiterItemPresetContainer.GetData(m_eItemPresetID);
		if (!data)
			return;

		m_aItems = data.SpawnPrefabs(animationOwner, m_iSeed);
	}

	//------------------------------------------------------------------------------------------------
	//! Deletes items which were spawned for the playback of this animation
	void DeleteItem()
	{
		if (!m_aItems)
			return;
		
		foreach (IEntity item : m_aItems)
		{
			if (!item || item.IsDeleted())
				continue;
			
			// call later to ensure that the entity is deleted from a right thread
			GetGame().GetCallqueue().CallLater(RplComponent.DeleteRplEntity, param1: item, param2: false);
		}
		m_aItems = null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns spawned items of this animation
	array<IEntity> GetItems()
	{
		return m_aItems;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Provides this animation with items to despawn
	void SetItems(array<IEntity> itemsArray)
	{
		m_aItems = itemsArray;
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_LoiterCustomAnimData CreateInstance(TAnimGraphCommand commandID = -1, int commandValue = -1, 
		string commandName = SCR_CustomAnimData_Properties.BINDING_COMMAND_NAME, string graphBindingName = SCR_CustomAnimData_Properties.BINDING_NAME_NPC,
		ResourceName graphName = ResourceName.Empty, ResourceName graphInstanceName = ResourceName.Empty, 
		TAnimGraphCommand controlVariableID = -1, int controlVariableValue = -1, string controlVariableName = string.Empty,
		SCR_EAnimationItemID itemPreset = SCR_EAnimationItemID.NONE, bool allowRootMotion = false, bool allowEnvironmentalExit = true)
	{
		SCR_LoiterCustomAnimData inst = new SCR_LoiterCustomAnimData();
		inst.m_iCommandBindID = commandID;
		inst.m_iCommandValue = commandValue;
		inst.m_sCommandName = commandName;
		inst.m_sGraphBindingName = graphBindingName;
		inst.m_sGraphName = graphName;
		inst.m_sGraphInstanceName = graphInstanceName;
		inst.m_iControlVariableID = controlVariableID;
		inst.m_iControlVariableValue = controlVariableValue;
		inst.m_sControlVariableName = controlVariableName;
		inst.m_eItemPresetID = itemPreset;
		inst.m_bAllowRootMotion = allowRootMotion;
		inst.m_bAllowEnvironmentExit = allowEnvironmentalExit;
		inst.RandomizeSeed();
		return inst;
	}

	//------------------------------------------------------------------------------------------------
	static SCR_LoiterCustomAnimData CreateInstanceFromParent(notnull SCR_CustomAnimData_Properties parentInstance)
	{
		SCR_LoiterCustomAnimData inst = new SCR_LoiterCustomAnimData();
		inst.m_iCommandBindID = parentInstance.m_iCommandBindID;
		inst.m_iCommandValue = parentInstance.m_iCommandValue;
		inst.m_sCommandName = parentInstance.m_sCommandName;
		inst.m_sGraphBindingName = parentInstance.m_sGraphBindingName;
		inst.m_sGraphName = parentInstance.m_sGraphName;
		inst.m_sGraphInstanceName = parentInstance.m_sGraphInstanceName;
		inst.m_sControlVariableName = parentInstance.m_sControlVariableName;
		inst.m_iControlVariableID = parentInstance.m_iControlVariableID;
		inst.m_iControlVariableValue = parentInstance.m_iControlVariableValue;
		inst.m_eItemPresetID = parentInstance.m_eItemPresetID;
		inst.m_bAllowRootMotion = parentInstance.m_bAllowRootMotion;
		inst.m_bAllowEnvironmentExit = parentInstance.m_bAllowEnvironmentExit;
		inst.RandomizeSeed();
		return inst;
	}

	//------------------------------------------------------------------------------------------------
	//! Randomizes animation data seed within enfusion limits
	void RandomizeSeed()
	{
		m_iSeed = Math.RandomInt(0, SEED_RANGE + SEED_RANGE) - SEED_RANGE;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] r
	void OnRplLoad(ScriptBitReader r)
	{
		int cmdInt;
		r.ReadInt(cmdInt);
		m_iCommandBindID = cmdInt;
		
		int cmdValInt;
		r.ReadInt(cmdValInt);
		m_iCommandValue = cmdValInt;
		r.ReadInt(m_eItemPresetID);
		int varId = m_iControlVariableID;
		r.ReadInt(varId);
		r.ReadInt(m_iControlVariableValue);
		r.ReadInt(m_iSeed);

		r.ReadBool(m_bAllowRootMotion);
		r.ReadBool(m_bAllowEnvironmentExit);

		r.ReadString(m_sGraphName);
		r.ReadString(m_sGraphInstanceName);
		r.ReadString(m_sGraphBindingName);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] w
	void OnRplSave(ScriptBitWriter w)
	{
		w.WriteInt(m_iCommandBindID);
		w.WriteInt(m_iCommandValue);
		w.WriteInt(m_eItemPresetID);
		int varId;
		w.WriteInt(varId);
		m_iControlVariableID = varId;
		w.WriteInt(m_iControlVariableValue);
		w.WriteInt(m_iSeed);

		w.WriteBool(m_bAllowRootMotion);
		w.WriteBool(m_bAllowEnvironmentExit);

		w.WriteString(m_sGraphName);
		w.WriteString(m_sGraphInstanceName);
		w.WriteString(m_sGraphBindingName);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] instance
	//! \param[in] ctx
	//! \param[in] snapshot
	//! \return
	static bool Extract(SCR_LoiterCustomAnimData instance, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
		snapshot.SerializeInt(instance.m_iCommandBindID);
		snapshot.SerializeInt(instance.m_iCommandValue);
		snapshot.SerializeInt(instance.m_eItemPresetID);
		snapshot.SerializeInt(instance.m_iControlVariableID);
		snapshot.SerializeInt(instance.m_iControlVariableValue);
		snapshot.SerializeInt(instance.m_iSeed);
		snapshot.SerializeBool(instance.m_bAllowRootMotion);
		snapshot.SerializeBool(instance.m_bAllowEnvironmentExit);
		snapshot.SerializeString(instance.m_sGraphName);
		snapshot.SerializeString(instance.m_sGraphInstanceName);
		snapshot.SerializeString(instance.m_sGraphBindingName);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] snapshot
	//! \param[in] ctx
	//! \param[in] instance
	//! \return
	static bool Inject(SSnapSerializerBase snapshot, ScriptCtx ctx, SCR_LoiterCustomAnimData instance)
	{
		snapshot.SerializeInt(instance.m_iCommandBindID);
		snapshot.SerializeInt(instance.m_iCommandValue);
		snapshot.SerializeInt(instance.m_eItemPresetID);
		snapshot.SerializeInt(instance.m_iControlVariableID);
		snapshot.SerializeInt(instance.m_iControlVariableValue);
		snapshot.SerializeInt(instance.m_iSeed);
		snapshot.SerializeBool(instance.m_bAllowRootMotion);
		snapshot.SerializeBool(instance.m_bAllowEnvironmentExit);
		snapshot.SerializeString(instance.m_sGraphName);
		snapshot.SerializeString(instance.m_sGraphInstanceName);
		snapshot.SerializeString(instance.m_sGraphBindingName);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] snapshot
	//! \param[in] ctx
	//! \param[in] packet
	static void Encode(SSnapSerializerBase snapshot, ScriptCtx ctx, ScriptBitSerializer packet)
	{
		snapshot.EncodeInt(packet);
		snapshot.EncodeInt(packet);
		snapshot.EncodeInt(packet);
		snapshot.EncodeInt(packet);
		snapshot.EncodeInt(packet);
		snapshot.EncodeInt(packet);
		snapshot.EncodeBool(packet);
		snapshot.EncodeBool(packet);
		snapshot.EncodeString(packet);
		snapshot.EncodeString(packet);
		snapshot.EncodeString(packet);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] packet
	//! \param[in] ctx
	//! \param[in] snapshot
	//! \return
	static bool Decode(ScriptBitSerializer packet, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
		snapshot.DecodeInt(packet);
		snapshot.DecodeInt(packet);
		snapshot.DecodeInt(packet);
		snapshot.DecodeInt(packet);
		snapshot.DecodeInt(packet);
		snapshot.DecodeInt(packet);
		snapshot.DecodeBool(packet);
		snapshot.DecodeBool(packet);
		snapshot.DecodeString(packet);
		snapshot.DecodeString(packet);
		snapshot.DecodeString(packet);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] lhs
	//! \param[in] rhs
	//! \param[in] ctx
	//! \return
	static bool SnapCompare(SSnapSerializerBase lhs, SSnapSerializerBase rhs , ScriptCtx ctx)
	{
		return lhs.CompareSnapshots(rhs, 32)
			&& lhs.CompareStringSnapshots(rhs)
			&& lhs.CompareStringSnapshots(rhs)
			&& lhs.CompareStringSnapshots(rhs);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] instance
	//! \param[in] snapshot
	//! \param[in] ctx
	//! \return
	static bool PropCompare(SCR_LoiterCustomAnimData instance, SSnapSerializerBase snapshot, ScriptCtx ctx)
	{
		return snapshot.CompareInt(instance.m_iCommandBindID)
			&& snapshot.CompareInt(instance.m_iCommandValue)
			&& snapshot.CompareInt(instance.m_eItemPresetID)
			&& snapshot.CompareInt(instance.m_iControlVariableID)
			&& snapshot.CompareInt(instance.m_iControlVariableValue)
			&& snapshot.CompareInt(instance.m_iSeed)
			&& snapshot.CompareBool(instance.m_bAllowRootMotion)
			&& snapshot.CompareBool(instance.m_bAllowEnvironmentExit)
			&& snapshot.CompareString(instance.m_sGraphName)
			&& snapshot.CompareString(instance.m_sGraphInstanceName)
			&& snapshot.CompareString(instance.m_sGraphBindingName);
	}
}