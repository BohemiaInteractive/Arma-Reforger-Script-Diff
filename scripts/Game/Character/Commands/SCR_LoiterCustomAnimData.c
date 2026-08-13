[BaseContainerProps()]
class SCR_LoiterCustomAnimData : SCR_CustomAnimData_Properties
{
	[Attribute(defvalue: SCR_ELoiterItemID.NONE.ToString(), desc: "Id of the predefined item configuration which should be used during loiter animation playback.\nConfiguration is stored in {911269C6C40E808B}Configs/Character/LoiterItemPresets.conf", uiwidget: UIWidgets.ComboBox, enumType: SCR_ELoiterItemID)]
	protected SCR_ELoiterItemID m_eLoiterItemPreset;

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
		return super.IsDefault() && m_eLoiterItemPreset == SCR_ELoiterItemID.NONE;
	}

	//------------------------------------------------------------------------------------------------
	//! Spawns local items for the playback of this animation
	//! \param[in] animationOwner
	void SpawnItem(notnull IEntity animationOwner)
	{
		if (m_eLoiterItemPreset == SCR_ELoiterItemID.NONE)
			return;

		SCR_LoiterItemDataBase data = s_LoiterItemPresetContainer.GetData(m_eLoiterItemPreset);
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

		foreach(IEntity item : m_aItems)
		{
			if (!item || item.IsDeleted())
				return;

			// call later to ensure that the entity is deleted from a right thread
			GetGame().GetCallqueue().CallLater(RplComponent.DeleteRplEntity, param1: item, param2: false);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_LoiterCustomAnimData CreateInstance(TAnimGraphCommand commandID = -1, int commandValue = -1, 
		string commandName = SCR_CustomAnimData_Properties.BINDING_COMMAND_NAME, string graphBindingName = SCR_CustomAnimData_Properties.BINDING_NAME_NPC,
		ResourceName graphName = ResourceName.Empty, ResourceName graphInstanceName = ResourceName.Empty, 
		TAnimGraphCommand controlVariableID = -1, int controlVariableValue = -1, string controlVariableName = string.Empty, SCR_ELoiterItemID itemPreset = SCR_ELoiterItemID.NONE)
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
		inst.m_eLoiterItemPreset = itemPreset;
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
		
		int cmdValInt
		r.ReadInt(cmdValInt);
		m_iCommandValue = cmdValInt;
		r.ReadInt(m_eLoiterItemPreset);
		r.ReadInt(m_iSeed);
		
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
		w.WriteInt(m_eLoiterItemPreset);
		w.WriteInt(m_iSeed);
		
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
		snapshot.SerializeInt(instance.m_eLoiterItemPreset);
		snapshot.SerializeInt(instance.m_iSeed);
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
		snapshot.SerializeInt(instance.m_eLoiterItemPreset);
		snapshot.SerializeInt(instance.m_iSeed);
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
		return lhs.CompareSnapshots(rhs, 16)
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
			&& snapshot.CompareInt(instance.m_eLoiterItemPreset)
			&& snapshot.CompareInt(instance.m_iSeed)
			&& snapshot.CompareString(instance.m_sGraphName)
			&& snapshot.CompareString(instance.m_sGraphInstanceName)
			&& snapshot.CompareString(instance.m_sGraphBindingName);
	}
}