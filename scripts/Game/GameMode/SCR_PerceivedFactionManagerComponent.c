[ComponentEditorProps(category: "GameScripted/GameMode", description: "")]
class SCR_PerceivedFactionManagerComponentClass : SCR_BaseGameModeComponentClass
{
}

class SCR_PerceivedFactionManagerComponent : SCR_BaseGameModeComponent
{	
	[Attribute(SCR_EPerceivedFactionOutfitType.DISABLED.ToString(), desc: "Decides what the faction outfit calculation is.\n\DISABLED: Perceived faction is never set by outfit\nHIGHEST_VALUE: The perveived faction is what ever the highest combined value is\nFULL_OUTFIT: Perceived faction will be unknown unless all clothing is of the same faction (Needs at least a torso and pants to be seens as a faction)",  category: "Settings", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(SCR_EPerceivedFactionOutfitType)), RplProp(onRplName: "OnPerceivedFactionOutfitTypeChanged")]
	protected SCR_EPerceivedFactionOutfitType m_eCharacterPerceivedFactionOutfitType;
	
	[Attribute("0", desc: "The punishment when killing a hostile character while the player is perceived as an unknown or other faction then their own.\n\nNote that warcrime will result as each kill counting as if they killed a friendly which will eventually cause a player to be kicked", uiwidget: UIWidgets.Flags, enumType: SCR_EDisguisedKillingPunishment, category: "Settings"), RplProp(onRplName: "OnPunishKillingWhileDisguisedChanged")]
	protected SCR_EDisguisedKillingPunishment m_ePunishmentKillingWhileDisguised;
	
	[Attribute(desc: "If character has no outfit faction it will set default faction. Otherwise it will set outfit faction to null\n\nIf Type is FULL_OUTFIT it will always set it to null regardless of this value", category: "Settings")]
	protected bool m_bNoOutfitFactionSetsDefaultFaction;
	
	[Attribute(desc: "Hints for on perceived faction changed. To inform the player about the system and what it means to not be of the perceived faction", category: "Hints")]
	protected ref SCR_GeneralHintStorage m_PerceivedFactionHints;
	
	[Attribute("0", desc: "If AI should perceive the Perceived faction of a player character when their outfit changes their perceived faction.\n\nThis is an experimental feature!", category: "AI Settings"), RplProp(onRplName: "OnPerceivedFactionChangesAffectsAIChanged")]
	protected bool m_bPerceivedFactionChangesAffectsAI;
	
	[Attribute("CIV", desc: "For AI only when m_bPerceivedFactionChangesAffectsAI is true. FactionKey of a faction which will be set if the player's faction can not be determined. Such as when the player is naked. Keep this empty if you want the faction to be NULL.\n\nThis setting is only used when m_bNoOutfitFactionSetsDefaultFaction is set to false and if that faction exists", category: "AI Settings")]
	protected FactionKey m_sAIFallbackFaction;
	
	protected Faction m_FallbackFaction;
	
	protected ref ScriptInvokerInt m_OnPerceivedFactionOutfitTypeChanged;
	protected ref ScriptInvokerInt m_OnPunishKillingWhileDisguisedChanged;
	protected ref ScriptInvokerBool m_OnPerceivedFactionChangesAffectsAI;
	
	protected static SCR_PerceivedFactionManagerComponent s_Instance;
	
	//------------------------------------------------------------------------------------------------
	static SCR_PerceivedFactionManagerComponent GetInstance()
	{
		return s_Instance;
	}
	
	//------------------------------------------------------------------------------------------------
	Faction GetFallbackFaction()
	{
		return m_FallbackFaction;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called by SCR_CharacterFactionAffiliationComponent On Perceived faction changed. To show hint to player
	void ShowPerceivedFactionChangedHint(Faction playerPerceivedFaction)
	{
		ShowHint();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ShowHint()
	{
		//! No hints set
		if (!m_PerceivedFactionHints)
			return;
		
		//! Perceived faction is disabled
		if (m_eCharacterPerceivedFactionOutfitType == SCR_EPerceivedFactionOutfitType.DISABLED)
			return;
		
		//! No hint manager
		SCR_HintManagerComponent hintManager = SCR_HintManagerComponent.GetInstance();
		if (!hintManager)
			return;
		
		//~ Team killing is not punished so do not set hint
		SCR_AdditionalGameModeSettingsComponent addtionGameModeSettings = SCR_AdditionalGameModeSettingsComponent.GetInstance();
		if (addtionGameModeSettings && !addtionGameModeSettings.IsTeamKillingPunished())
			return;
		
		SCR_HintUIInfo hint;
		
		SCR_EDisguisedKillingPunishment flags = GetPunishmentKillingWhileDisguisedFlags();
		
		//! Get the correct hint depending if killing others while disguised is not punished, is a warcrime or just loses XP
		if (flags == 0)
			hint = m_PerceivedFactionHints.GetHintByType(EHint.GAMEPLAY_PERCEIVED_FACTION_CHANGED);
		else if (SCR_Enum.HasFlag(flags, SCR_EDisguisedKillingPunishment.WARCRIME))
			hint = m_PerceivedFactionHints.GetHintByType(EHint.GAMEPLAY_PERCEIVED_FACTION_CHANGED_PUNISH_WARCRIME);
		else if (SCR_Enum.HasFlag(flags, SCR_EDisguisedKillingPunishment.XP_LOSS))
			hint = m_PerceivedFactionHints.GetHintByType(EHint.GAMEPLAY_PERCEIVED_FACTION_CHANGED_PUNISH_XP);
		
		if (!hint)
			return;
		
		//! Show the hint
		hintManager.Show(hint);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get the current Perceived faction outfit type. Which determintes how the perceived faction is calculated
	SCR_EPerceivedFactionOutfitType GetCharacterPerceivedFactionOutfitType()
	{
		return m_eCharacterPerceivedFactionOutfitType;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Change the Perceived faction outfit type. Which determintes how the perceived faction is calculated
	//! param[in] type New type to set
	//! param[in] playerID Optional to show notification
	void SetCharacterPerceivedFactionOutfitType_S(SCR_EPerceivedFactionOutfitType type, int playerID = -1)
	{
		if (m_eCharacterPerceivedFactionOutfitType == type || !GetGameMode().IsMaster())
			return;
		
		m_eCharacterPerceivedFactionOutfitType = type;
		
		if (playerID > 0)
		{
			if (type == SCR_EPerceivedFactionOutfitType.DISABLED)
				SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_PERCEIVED_FACTION_TYPE_DISABLED, playerID);
			else if (type == SCR_EPerceivedFactionOutfitType.HIGHEST_VALUE)
				SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_PERCEIVED_FACTION_TYPE_HIGHEST_VALUE, playerID);
			else if (type == SCR_EPerceivedFactionOutfitType.FULL_OUTFIT)
				SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_PERCEIVED_FACTION_TYPE_FULL_OUTFIT, playerID);
		}
			
		Replication.BumpMe();

		OnPerceivedFactionOutfitTypeChanged();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPerceivedFactionOutfitTypeChanged()
	{
		if (m_OnPerceivedFactionOutfitTypeChanged)
			m_OnPerceivedFactionOutfitTypeChanged.Invoke(m_eCharacterPerceivedFactionOutfitType);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Get on Player perceived action type changed script invoker
	ScriptInvokerInt GetPerceivedFactionOutfitTypeChanged()
	{
		if (!m_OnPerceivedFactionOutfitTypeChanged)
			m_OnPerceivedFactionOutfitTypeChanged = new ScriptInvokerInt();
		
		return m_OnPerceivedFactionOutfitTypeChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return true if unknown perceived faction when using type HIGHEST_VALUE the perceived will instead be set to default rather than unknown.
	bool DoesNoOutfitSetFactionToDefault()
	{
		return m_bNoOutfitFactionSetsDefaultFaction;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return The punishment flags set which dictate what the punishment is for killing enemies while disguised
	SCR_EDisguisedKillingPunishment GetPunishmentKillingWhileDisguisedFlags()
	{
		return m_ePunishmentKillingWhileDisguised;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set if killing other characters while perceived as unknown or other faction will count as a team kill
	//! param[in] punishKillingWhileDisguised If true punishes players that kill others while disguised
	//! param[in] playerID Optional Player ID to send notification
	void SetPunishmentKillingWhileDisguisedFlags_S(SCR_EDisguisedKillingPunishment punishmentFlags, int playerID = -1)
	{
		if (GetPunishmentKillingWhileDisguisedFlags() == punishmentFlags || !GetGameMode().IsMaster())
			return;
		
		m_ePunishmentKillingWhileDisguised = punishmentFlags;
		
		if (playerID > 0)
			SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_PERCEIVED_FACTION_PUNISHMENT_KILLING_SET, playerID, m_ePunishmentKillingWhileDisguised);
			
		Replication.BumpMe();

		OnPunishKillingWhileDisguisedChanged();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPunishKillingWhileDisguisedChanged()
	{
		if (m_OnPunishKillingWhileDisguisedChanged)
			m_OnPunishKillingWhileDisguisedChanged.Invoke(m_ePunishmentKillingWhileDisguised);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Get script when punished if player is disguised changes
	ScriptInvokerInt GetOnPunishKillingWhileDisguisedChanged()
	{
		if (!m_OnPunishKillingWhileDisguisedChanged)
			m_OnPunishKillingWhileDisguisedChanged = new ScriptInvokerInt();
		
		return m_OnPunishKillingWhileDisguisedChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPlayerSpawnFinalize_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnHandlerComponent handlerComponent, SCR_SpawnData data, IEntity entity)
	{
		SCR_CharacterFactionAffiliationComponent affiliationComponent = SCR_CharacterFactionAffiliationComponent.Cast(entity.FindComponent(SCR_CharacterFactionAffiliationComponent));
		if (!affiliationComponent)
			return;
		
		//~ Init only for players. Will go over player outfit and set the faction values in one go with calling replication only once for all items
		affiliationComponent.InitPlayerOutfitFaction_S();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPlayerKilled(notnull SCR_InstigatorContextData instigatorContextData)
	{
		SCR_CharacterFactionAffiliationComponent affiliationComponent = SCR_CharacterFactionAffiliationComponent.Cast(instigatorContextData.GetVictimEntity().FindComponent(SCR_CharacterFactionAffiliationComponent));
		if (!affiliationComponent)
			return;
		
		//~ Init only for players. Will go over player outfit and set the faction values in one go with calling replication only once for all items
		affiliationComponent.DisableUpdatingPerceivedFaction_S();
	}
	
	//------------------------------------------------------------------------------------------------
	bool DoesPerceivedFactionChangesAffectsAI()
	{
		return m_bPerceivedFactionChangesAffectsAI;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetPerceivedFactionChangesAffectsAI(bool affectsAI, int PlayerID = -1)
	{
		if (affectsAI == m_bPerceivedFactionChangesAffectsAI || !GetGameMode().IsMaster())
			return;
		
		m_bPerceivedFactionChangesAffectsAI = affectsAI;
		
		OnPerceivedFactionChangesAffectsAIChanged();
		Replication.BumpMe();
		
		//~ TODO: Notification
		//if (playerID > 0)
		//	SCR_NotificationsComponent.SendToEveryone(ENotification., playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnPerceivedFactionChangesAffectsAIChanged()
	{
		if (m_OnPerceivedFactionChangesAffectsAI)
			m_OnPerceivedFactionChangesAffectsAI.Invoke(m_bPerceivedFactionChangesAffectsAI);
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerBool GetOnPerceivedFactionChangesAffectsAIChanged()
	{
		if (!m_OnPerceivedFactionChangesAffectsAI)
			m_OnPerceivedFactionChangesAffectsAI = new ScriptInvokerBool();
		
		return m_OnPerceivedFactionChangesAffectsAI;
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Inventory is updated before this is called so it will call an extra replication if this is added. Meaning it has no use
	/*override void OnPlayerDeleted(int playerId, IEntity player)
	{
		if (!player)
			return;
	
		SCR_CharacterFactionAffiliationComponent affiliationComponent = SCR_CharacterFactionAffiliationComponent.Cast(player.FindComponent(SCR_CharacterFactionAffiliationComponent));
		if (!affiliationComponent)
			return;
		
		//~ Init only for players. Will go over player outfit and set the faction values in one go with calling replication only once for all items
		affiliationComponent.DisableUpdatingPerceivedFaction_S();
	}*/
	
	//------------------------------------------------------------------------------------------------
	void SCR_PerceivedFactionManagerComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		if (s_Instance)
		{
			Print("'SCR_PerceivedFactionManagerComponent' exists twice in the world!", LogLevel.WARNING);
			return;
		}
			
		s_Instance = this;
	}
	
	#ifdef ENABLE_DIAG
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_CHARACTER_SHOW_FACTION_OUTFIT))
			CharacterOutfitDebug();
	}
	#endif

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		#ifdef ENABLE_DIAG
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_CHARACTER_SHOW_FACTION_OUTFIT, "", "Show Perceived Faction", "Character");
		SetEventMask(owner, EntityEvent.FRAME);
		#endif
		
		SetEventMask(owner, EntityEvent.INIT);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		if (!GetGame().InPlayMode())
			return;
		
		if (m_sAIFallbackFaction.IsEmpty())
			return;
		
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return;
		
		m_FallbackFaction = factionManager.GetFactionByKey(m_sAIFallbackFaction);
		if (!m_FallbackFaction)
			Print("SCR_PerceivedFactionManagerComponent trying to set fall back faction '" + m_sAIFallbackFaction + "' but this faction could not be found so it is null instead.", LogLevel.WARNING);
	}
	
	#ifdef ENABLE_DIAG
	//------------------------------------------------------------------------------------------------
	protected void CharacterOutfitDebug()
	{		
		DbgUI.Begin("Player Perceived Faction", 0, 0);
		
		IEntity player = SCR_PlayerController.GetLocalControlledEntity();
		if (!player)
		{
			DbgUI.Text("No Player");
			DbgUI.End();
			
			return;
		}
		
		DbgUI.Text("Faction Outfit setting: " + typename.EnumToString(SCR_EPerceivedFactionOutfitType, GetCharacterPerceivedFactionOutfitType()));
		DbgUI.Text("");
		DbgUI.Text("");
		DbgUI.Text("");
		DbgUI.Text("--------");
		DbgUI.Text("");
		
		if (GetCharacterPerceivedFactionOutfitType() == SCR_EPerceivedFactionOutfitType.DISABLED)
		{
			DbgUI.End();
			return;
		}
		
		SCR_CharacterFactionAffiliationComponent charFactionAffiliation = SCR_CharacterFactionAffiliationComponent.Cast(player.FindComponent(SCR_CharacterFactionAffiliationComponent));
		if (!charFactionAffiliation)
		{
			DbgUI.Text("Player is missing SCR_CharacterFactionAffiliationComponent");
			DbgUI.End();
			
			return;
		}
		
		if (!charFactionAffiliation.HasPerceivedFaction())
		{
			DbgUI.Text("Unable to get Perceived faction:");
			
			SCR_PossessingManagerComponent possessingManager = SCR_PossessingManagerComponent.GetInstance();
			
			if (!possessingManager || possessingManager.IsPossessing(SCR_PlayerController.GetLocalPlayerId()))
				DbgUI.Text("   - Player is possessing an AI");
			else 
				DbgUI.Text("   - Player spawned was not called: Was player spawned on play in workbench?");
			
			DbgUI.End();
			
			return;
		}
		
		Faction currentOutfitFaction = charFactionAffiliation.GetPerceivedFaction();
		
		if (!currentOutfitFaction)
			DbgUI.Text("Perceived Faction: UNKNOWN");
		else 
			DbgUI.Text("Perceived Faction: " + currentOutfitFaction.GetFactionKey());
		
		
		DbgUI.Text("Disguise Type: " + typename.EnumToString(SCR_ECharacterDisguiseType, charFactionAffiliation.GetCharacterDisguiseType()));
		
		map<Faction, int> outfitFactionValues = new map<Faction, int>();
		int count = charFactionAffiliation.GetCharacterOutfitValues(outfitFactionValues);
		
		DbgUI.Text("");
		DbgUI.Text("Outfit values:");
		
		if (count <= 0)
		{
			DbgUI.Text("   - None");
		}
		else 
		{
			foreach(Faction outfitFaction, int outfitvalue : outfitFactionValues)
			{
				DbgUI.Text("   - " + outfitFaction.GetFactionKey() + ": " + outfitvalue);
			}
		}
		
		if (GetCharacterPerceivedFactionOutfitType() == SCR_EPerceivedFactionOutfitType.FULL_OUTFIT && !charFactionAffiliation.IsCharacterWearingFullOutfit())
		{
			DbgUI.Text("");
			DbgUI.Text("Missing:");
			
			//~ Check if both torso and pants on
			EquipedLoadoutStorageComponent characterStorage = EquipedLoadoutStorageComponent.Cast(player.FindComponent(EquipedLoadoutStorageComponent));
			
			if (!characterStorage.GetClothFromArea(LoadoutJacketArea))
				DbgUI.Text("   - Jacket");
			
			if (!characterStorage.GetClothFromArea(LoadoutPantsArea))
				DbgUI.Text("   - Pants");
			
			//if (!characterStorage.GetClothFromArea(LoadoutBootsArea))
			//	DbgUI.Text("   - Shoes");
		}
		
		DbgUI.Text("");
		DbgUI.Text("");
		DbgUI.Text("");
		DbgUI.Text("--------");
		
		DbgUI.Text("");
		DbgUI.Text("Killing while disguised punishment: " + SCR_Enum.FlagsToString(SCR_EDisguisedKillingPunishment, GetPunishmentKillingWhileDisguisedFlags(), noValue: "NONE"));
		
		DbgUI.Text("");
		DbgUI.Text("AI reacts to perceived faction: " + DoesPerceivedFactionChangesAffectsAI().ToString() + " ");
		DbgUI.Text("");
		
		if (DoesPerceivedFactionChangesAffectsAI())
		{
			if (m_FallbackFaction && !DoesNoOutfitSetFactionToDefault())
				DbgUI.Text("Faction AI perceives if no outfit values: " + m_sAIFallbackFaction + " ");
			else 
				DbgUI.Text("Faction AI perceives if no outfit values: DEFAULT FACTION");
		}
		
		DbgUI.End();
	}
	#endif
}

enum SCR_EPerceivedFactionOutfitType
{
	DISABLED = 0, //!< Perceived faction outfit system is disabled
	HIGHEST_VALUE = 10, //!< The faction is set to the highest value of all combined clothing
	FULL_OUTFIT = 20, //!<  The faction is unknown until all clothing items are of the same faction and at least a pants and torso piece are equipt
}

enum SCR_EDisguisedKillingPunishment
{
	WARCRIME = 1 << 0, //!< Killing players while disguised is a warcrime, resulting in an eventual kick if they continue doing so
	XP_LOSS = 1 << 1, //!< Killing players while disguised will case a lose in XP
}

