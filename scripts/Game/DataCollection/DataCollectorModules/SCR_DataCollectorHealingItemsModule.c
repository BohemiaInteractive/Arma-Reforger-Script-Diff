[BaseContainerProps()]
class ConsumableTypeStats : Managed
{
	[Attribute("0", UIWidgets.ComboBox, "Item type", "", ParamEnumArray.FromEnum(SCR_EConsumableType) )]
	protected SCR_EConsumableType m_eConsumableType;

	[Attribute("0", UIWidgets.ComboBox, "Stat to affect when used on self", "", ParamEnumArray.FromEnum(SCR_EDataStats) )]
	protected SCR_EDataStats m_eDataStatsSelf;

	[Attribute("0", UIWidgets.ComboBox, "Stat to affect when used on a different character", "", ParamEnumArray.FromEnum(SCR_EDataStats) )]
	protected SCR_EDataStats m_eDataStatsFriend;
	
	//------------------------------------------------------------------------------------------------
	SCR_EConsumableType GetConsumableType()
	{
		return m_eConsumableType;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_EDataStats GetStat(bool self)
	{
		if (self)
			return m_eDataStatsSelf;
		
		return m_eDataStatsFriend;
	}
};

[BaseContainerProps()]
class SCR_DataCollectorHealingItemsModule : SCR_DataCollectorModule
{
	[Attribute("", UIWidgets.Object, "Healing Items")]
	protected ref array<ref ConsumableTypeStats> m_aConsumableTypes;
	
	protected ref map<int, IEntity> m_mTrackedPlayers = new map<int, IEntity>();

	//------------------------------------------------------------------------------------------------
	protected override void AddInvokers(IEntity player)
	{
		super.AddInvokers(player);
		if (!player)
			return;

		SCR_ChimeraCharacter chimeraPlayer = SCR_ChimeraCharacter.Cast(player);
		if (!chimeraPlayer)
			return;

		SCR_CharacterControllerComponent characterController = SCR_CharacterControllerComponent.Cast(chimeraPlayer.GetCharacterController());
		if (!characterController)
			return;

		characterController.m_OnItemUseEndedInvoker.Insert(HealingItemUsed);
	}

	//------------------------------------------------------------------------------------------------
	protected override void RemoveInvokers(IEntity player)
	{
		super.RemoveInvokers(player);
		if (!player)
			return;

		SCR_ChimeraCharacter chimeraPlayer = SCR_ChimeraCharacter.Cast(player);
		if (!chimeraPlayer)
			return;

		SCR_CharacterControllerComponent characterController = SCR_CharacterControllerComponent.Cast(chimeraPlayer.GetCharacterController());
		if (!characterController)
			return;

		characterController.m_OnItemUseEndedInvoker.Remove(HealingItemUsed);
	}

	//------------------------------------------------------------------------------------------------
	protected void HealingItemUsed(IEntity item, bool ActionCompleted, ItemUseParameters animParams)
	{
		if (!item || !ActionCompleted)
			return;

		SCR_ConsumableItemComponent consumableItem = SCR_ConsumableItemComponent.Cast(item.FindComponent(SCR_ConsumableItemComponent));
		if (!consumableItem)
			return;

		IEntity user = consumableItem.GetCharacterOwner();
		if (!user)
			return;

		IEntity target = consumableItem.GetTargetCharacter();
		if (!target)
			target = user;

		int userID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(user);
		if (userID == 0) // Non-player character
			return;

		SCR_EConsumableType typeId = consumableItem.GetConsumableType();
		
		for (int i = m_aConsumableTypes.Count() - 1; i >= 0; i--)
		{
			if (m_aConsumableTypes[i].GetConsumableType() == typeId)
			{
				GetGame().GetDataCollector().GetPlayerData(userID).AddStat(m_aConsumableTypes[i].GetStat(target == user));
				return;
			}
		}
		
		Print("SCR_DataCollectorHealingItemsModule:HealingItemUsed: Error: Unidentified Healing item typeId. Value is " + typeId + ".", LogLevel.WARNING);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerSpawned(int playerID, IEntity controlledEntity)
	{
		IEntity ent = m_mTrackedPlayers.Get(playerID);
		if (ent)
			RemoveInvokers(ent);
		
		AddInvokers(controlledEntity);
		
		m_mTrackedPlayers.Insert(playerID, controlledEntity);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerDisconnected(int playerID, IEntity controlledEntity = null)
	{
		m_mTrackedPlayers.Remove(playerID);
	}

#ifdef ENABLE_DIAG
	//------------------------------------------------------------------------------------------------
	override void OnControlledEntityChanged(IEntity from, IEntity to)
	{
		super.OnControlledEntityChanged(from, to);

		if (to)
		{
			int playerID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(to);
			m_mTrackedPlayers.Insert(playerID, to);
		}
		else if (from)
		{
			int playerID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(from);
			m_mTrackedPlayers.Remove(playerID);
		}
	}
#endif

	//------------------------------------------------------------------------------------------------
	override void Update(float timeTick)
	{
		//If there's no data collector, do nothing
		if (!GetGame().GetDataCollector())
			return;

		m_fTimeSinceUpdate += timeTick;

		if (m_fTimeSinceUpdate < m_fUpdatePeriod)
			return;

		SCR_PlayerData playerData;
		int playerId;

		for (int i = m_mTrackedPlayers.Count() - 1; i >= 0; i--)
		{
			playerId = m_mTrackedPlayers.GetKey(i);
			playerData = GetGame().GetDataCollector().GetPlayerData(playerId);

			//DEBUG display
#ifdef ENABLE_DIAG
			if (m_StatsVisualization)
			{
				m_StatsVisualization.Get(SCR_EHealingItemsModuleStats.BANDAGESSELF).SetText(playerData.GetStat(SCR_EDataStats.BANDAGE_SELF).ToString());
				m_StatsVisualization.Get(SCR_EHealingItemsModuleStats.BANDAGESFRIENDLIES).SetText(playerData.GetStat(SCR_EDataStats.BANDAGE_FRIENDLIES).ToString());
				m_StatsVisualization.Get(SCR_EHealingItemsModuleStats.TOURNIQUETSSELF).SetText(playerData.GetStat(SCR_EDataStats.TOURNIQUET_SELF).ToString());
				m_StatsVisualization.Get(SCR_EHealingItemsModuleStats.TOURNIQUETSFRIENDLIES).SetText(playerData.GetStat(SCR_EDataStats.TOURNIQUET_FRIENDLIES).ToString());
				m_StatsVisualization.Get(SCR_EHealingItemsModuleStats.SALINESELF).SetText(playerData.GetStat(SCR_EDataStats.SALINE_SELF).ToString());
				m_StatsVisualization.Get(SCR_EHealingItemsModuleStats.SALINEFRIENDLIES).SetText(playerData.GetStat(SCR_EDataStats.SALINE_FRIENDLIES).ToString());
				m_StatsVisualization.Get(SCR_EHealingItemsModuleStats.MORPHINESELF).SetText(playerData.GetStat(SCR_EDataStats.MORPHINE_SELF).ToString());
				m_StatsVisualization.Get(SCR_EHealingItemsModuleStats.MORPHINEFRIENDLIES).SetText(playerData.GetStat(SCR_EDataStats.MORPHINE_FRIENDLIES).ToString());
			}
#endif
		}
		m_fTimeSinceUpdate = 0;
	}

#ifdef ENABLE_DIAG
	//------------------------------------------------------------------------------------------------
	override void CreateVisualization()
	{
		super.CreateVisualization();
		if (!m_StatsVisualization)
			return;

		CreateEntry("Bandages on self: ", 0, SCR_EHealingItemsModuleStats.BANDAGESSELF);
		CreateEntry("Bandages on friendlies: ", 0, SCR_EHealingItemsModuleStats.BANDAGESFRIENDLIES);
		CreateEntry("Tourniquets on self: ", 0, SCR_EHealingItemsModuleStats.TOURNIQUETSSELF);
		CreateEntry("Tourniquets on friendlies: ", 0, SCR_EHealingItemsModuleStats.TOURNIQUETSFRIENDLIES);
		CreateEntry("saline on self: ", 0, SCR_EHealingItemsModuleStats.SALINESELF);
		CreateEntry("saline on friendlies: ", 0, SCR_EHealingItemsModuleStats.SALINEFRIENDLIES);
		CreateEntry("Morphine on self: ", 0, SCR_EHealingItemsModuleStats.MORPHINESELF);
		CreateEntry("Morphine on friendlies: ", 0, SCR_EHealingItemsModuleStats.MORPHINEFRIENDLIES);
	}
#endif
};

#ifdef ENABLE_DIAG
enum SCR_EHealingItemsModuleStats
{
	BANDAGESSELF,
	BANDAGESFRIENDLIES,
	TOURNIQUETSSELF,
	TOURNIQUETSFRIENDLIES,
	SALINESELF,
	SALINEFRIENDLIES,
	MORPHINESELF,
	MORPHINEFRIENDLIES
};
#endif
