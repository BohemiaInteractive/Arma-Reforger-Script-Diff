class SCR_WeaponLoadoutData
{
	bool Active;
	int SlotIdx;
	ResourceName WeaponPrefab;
	ref array<ResourceName> Attachments = {};
}

class SCR_ClothingLoadoutData
{
	int SlotIdx;
	ResourceName ClothingPrefab;
}

class SCR_PlayerLoadoutData
{
	ref array<ref SCR_ClothingLoadoutData> Clothings = {};
	ref array<ref SCR_WeaponLoadoutData> Weapons = {};
	float LoadoutCost;
	float LoadoutPlayerSupplyAllocationCost;
	int FactionIndex;

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] instance
	//! \param[in] ctx
	//! \param[in] snapshot
	//! \return
	static bool Extract(SCR_PlayerLoadoutData instance, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
		int clothingCount = instance.Clothings.Count();
		snapshot.SerializeInt(clothingCount);

		ResourceName prefab;
		for (int i = 0; i < clothingCount; ++i)
		{
			snapshot.SerializeInt(instance.Clothings[i].SlotIdx);
			prefab = instance.Clothings[i].ClothingPrefab;
			snapshot.SerializeString(prefab);
		}

		int weaponCount = instance.Weapons.Count();
		snapshot.SerializeInt(weaponCount);
		for (int i = 0; i < weaponCount; ++i)
		{
			snapshot.SerializeBool(instance.Weapons[i].Active);
			snapshot.SerializeInt(instance.Weapons[i].SlotIdx);
			prefab = instance.Weapons[i].WeaponPrefab;
			snapshot.SerializeString(prefab);

			int attachmentsCount = instance.Weapons[i].Attachments.Count();
			snapshot.SerializeInt(attachmentsCount);
			for (int nAttachment = 0; nAttachment < attachmentsCount; ++nAttachment)
			{
				prefab = instance.Weapons[i].Attachments[nAttachment];
				snapshot.SerializeString(prefab);
			}
		}

		snapshot.SerializeFloat(instance.LoadoutCost);
		snapshot.SerializeFloat(instance.LoadoutPlayerSupplyAllocationCost);
		snapshot.SerializeInt(instance.FactionIndex);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] snapshot
	//! \param[in] ctx
	//! \param[in] instance
	//! \return
	static bool Inject(SSnapSerializerBase snapshot, ScriptCtx ctx, SCR_PlayerLoadoutData instance)
	{
		// Fill an instance with values from snapshot.
		int clothingCount;
		snapshot.SerializeInt(clothingCount);
		instance.Clothings.Clear();

		SCR_ClothingLoadoutData clothingData;
		for (int i = 0; i < clothingCount; ++i)
		{
			clothingData = new SCR_ClothingLoadoutData();

			snapshot.SerializeInt(clothingData.SlotIdx);

			ResourceName prefab;
			snapshot.SerializeString(prefab);
			clothingData.ClothingPrefab = prefab;

			instance.Clothings.Insert(clothingData);
		}

		int weaponCount;
		snapshot.SerializeInt(weaponCount);
		instance.Weapons.Clear();

		SCR_WeaponLoadoutData weaponData;
		for (int i = 0; i < weaponCount; ++i)
		{
			weaponData = new SCR_WeaponLoadoutData();

			snapshot.SerializeBool(weaponData.Active);
			snapshot.SerializeInt(weaponData.SlotIdx);

			ResourceName prefab;
			snapshot.SerializeString(prefab);
			weaponData.WeaponPrefab = prefab;

			int attachmentsCount;
			snapshot.SerializeInt(attachmentsCount);
			weaponData.Attachments.Reserve(attachmentsCount);
			for (int nAttachment = 0; nAttachment < attachmentsCount; ++nAttachment)
			{
				snapshot.SerializeString(prefab);
				weaponData.Attachments.Insert(prefab);
			}

			instance.Weapons.Insert(weaponData);
		}

		snapshot.SerializeFloat(instance.LoadoutCost);
		snapshot.SerializeFloat(instance.LoadoutPlayerSupplyAllocationCost);
		snapshot.SerializeInt(instance.FactionIndex);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] snapshot
	//! \param[in] ctx
	//! \param[in] packet
	//! \return
	static void Encode(SSnapSerializerBase snapshot, ScriptCtx ctx, ScriptBitSerializer packet)
	{
		int clothingCount;
		snapshot.SerializeBytes(clothingCount, 4);
		packet.Serialize(clothingCount, 32);

		ResourceName prefab;
		for (int i = 0; i < clothingCount; ++i)
		{
			// clothingData.SlotIdx
			snapshot.EncodeInt(packet);

			// clothingData.ClothingPrefab
			snapshot.SerializeString(prefab);
			packet.SerializeResourceName(prefab);
		}

		int weaponCount;
		snapshot.SerializeBytes(weaponCount, 4);
		packet.Serialize(weaponCount, 32);

		for (int i = 0; i < weaponCount; ++i)
		{
			// weaponData.Active
			snapshot.EncodeBool(packet);

			// weaponData.SlotIdx
			snapshot.EncodeInt(packet);

			// weaponData.WeaponPrefab
			snapshot.SerializeString(prefab);
			packet.SerializeResourceName(prefab);

			// weaponData.Attachments
			int attachmentsCount;
			snapshot.SerializeBytes(attachmentsCount, 4);
			packet.Serialize(attachmentsCount, 32);
			for (int nAttachment = 0; nAttachment < attachmentsCount; ++nAttachment)
			{
				snapshot.SerializeString(prefab);
				packet.SerializeResourceName(prefab);
			}
		}

		snapshot.EncodeFloat(packet);
		snapshot.EncodeFloat(packet);
		snapshot.EncodeInt(packet);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] packet
	//! \param[in] ctx
	//! \param[in] snapshot
	//! \return
	static bool Decode(ScriptBitSerializer packet, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
		int clothingCount;
		packet.Serialize(clothingCount, 32);
		snapshot.SerializeBytes(clothingCount, 4);

		ResourceName prefab;
		for (int i = 0; i < clothingCount; ++i)
		{
			// clothingData.SlotIdx
			snapshot.DecodeInt(packet);

			// clothingData.ClothingPrefab
			packet.SerializeResourceName(prefab);
			snapshot.SerializeString(prefab);
		}

		int weaponCount;
		packet.Serialize(weaponCount, 32);
		snapshot.SerializeBytes(weaponCount, 4);

		for (int i = 0; i < weaponCount; ++i)
		{
			// weaponData.Active
			snapshot.DecodeBool(packet);

			// weaponData.SlotIdx
			snapshot.DecodeInt(packet);

			// weaponData.WeaponPrefab
			packet.SerializeResourceName(prefab);
			snapshot.SerializeString(prefab);

			// weaponData.Attachments
			int attachmentsCount;
			packet.Serialize(attachmentsCount, 32);
			snapshot.SerializeBytes(attachmentsCount, 4);
			for (int nAttachment = 0; nAttachment < attachmentsCount; ++nAttachment)
			{
				packet.SerializeResourceName(prefab);
				snapshot.SerializeString(prefab);
			}
		}

		snapshot.DecodeFloat(packet);
		snapshot.DecodeFloat(packet);
		snapshot.DecodeInt(packet);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] lhs
	//! \param[in] rhs
	//! \param[in] ctx
	//! \return
	static bool SnapCompare(SSnapSerializerBase lhs, SSnapSerializerBase rhs, ScriptCtx ctx)
	{
		Print("Cannot use SCR_PlayerLoadoutData as a property", LogLevel.ERROR);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] instance
	//! \param[in] snapshot
	//! \param[in] ctx
	//! \return
	static bool PropCompare(SCR_PlayerLoadoutData instance, SSnapSerializerBase snapshot, ScriptCtx ctx)
	{
		Print("Cannot use SCR_PlayerLoadoutData as a property", LogLevel.ERROR);
		return true;
	}
}
