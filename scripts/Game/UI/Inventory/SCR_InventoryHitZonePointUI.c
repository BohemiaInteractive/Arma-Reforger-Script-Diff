	//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
class SCR_InventoryHitZonePointContainerUI : ScriptedWidgetComponent
{
	[Attribute("Bleeding")]
	protected string m_sBleedingHitZoneName;

	[Attribute("Damage")]
	protected string m_sDamageHitZoneName;

	protected SCR_InventoryHitZonePointUI m_pDamageHandler;

	protected ref array<HitZone> m_aGroupHitZones = {};
	protected SCR_CharacterBloodHitZone m_pBloodHitZone;
	protected ECharacterHitZoneGroup m_eHitZoneGroup;
	protected IEntity m_Player;
	protected SCR_TourniquetStorageComponent m_TourniquetStorage;
	SCR_CharacterDamageManagerComponent m_pCharDmgManager;
	protected TNodeId m_iBoneIndex;

	protected Widget m_wRoot;
	protected Widget m_wGlow;
	protected SCR_InventoryMenuUI m_pInventoryMenu;
	protected BaseInventoryStorageComponent m_pStorage;
	protected ref SCR_InventoryHitZoneUI m_pStorageUI;

	protected bool m_bSelected;
	protected bool m_bApplicableItemsShown;
	static SCR_InventoryHitZonePointContainerUI s_pSelectedPoint;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wRoot = w;

		m_wGlow = w.FindAnyWidget("Glow");
		m_wGlow.SetVisible(false);

		Widget damageW = w.FindAnyWidget(m_sDamageHitZoneName);
		m_pDamageHandler = SCR_InventoryHitZonePointUI.Cast(damageW.FindHandler(SCR_InventoryHitZonePointUI));
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (button == 0)
		{
			if (!m_bApplicableItemsShown)
				ShowApplicableItems();
			else
				HideApplicableItems();
		}

		return false;
	}

	void RemoveTourniquetFromSlot()
	{
		if (m_TourniquetStorage)
			m_TourniquetStorage.RemoveTourniquetFromSlot(m_eHitZoneGroup);
	}
	
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		Highlight(true);

		SCR_InventorySpinBoxComponent spinbox = m_pInventoryMenu.GetAttachmentSpinBoxComponent();
		if (spinbox)
		{
			int id = spinbox.FindItem(GetHitZoneName());
			if (id > -1)
				spinbox.SetCurrentItem(id);
		}
	
		m_pStorageUI.OnFocus(m_pStorageUI.GetRootWidget(), 0, 0);

		return false;
	}
		
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		Highlight(false);
		
		m_pStorageUI.OnFocusLost(m_pStorageUI.GetRootWidget(), 0, 0);

		return false;
	}

	protected void Highlight(bool highlight)
	{
		m_pDamageHandler.Highlight(highlight);
	}

	void SetGlowVisible(bool visible)
	{
		m_wGlow.SetVisible(visible);
	}

	void Select(bool select = true)
	{
		if (s_pSelectedPoint)
		{
			s_pSelectedPoint.m_bSelected = false;
			s_pSelectedPoint.Highlight(false);
		}

		m_bSelected = select;
		Highlight(select);
		if (select)
		{
			s_pSelectedPoint = this;
		}
		else
		{
			s_pSelectedPoint = null;
		}
	}

	//------------------------------------------------------------------------------------------------
	void ShowApplicableItems()
	{
		m_bApplicableItemsShown = true;
		Select(true);
		SCR_ApplicableMedicalItemPredicate predicate = new SCR_ApplicableMedicalItemPredicate();
		predicate.characterEntity = m_Player;
		predicate.hitZoneGroup = m_eHitZoneGroup;
		m_pInventoryMenu.ShowAttachmentStorage(predicate);
	}

	void HideApplicableItems()
	{
		m_bApplicableItemsShown = false;
		if (m_pInventoryMenu.GetAttachmentStorageUI())
		{
			m_pInventoryMenu.RemoveAttachmentStorage(m_pInventoryMenu.GetAttachmentStorageUI());
			m_pStorageUI.GetRootWidget().SetVisible(false);
			Select(false);
		}
	}

	//------------------------------------------------------------------------------------------------
	ECharacterHitZoneGroup GetHitZoneGroup()
	{
		return m_eHitZoneGroup;
	}

	//------------------------------------------------------------------------------------------------
	string GetHitZoneName()
	{
		switch (m_eHitZoneGroup)
		{
			case ECharacterHitZoneGroup.HEAD:
			{
				return "#AR-Inventory_Head";
			} break;

			case ECharacterHitZoneGroup.UPPERTORSO:
			{
				return "#AR-Inventory_Chest";
			} break;

			case ECharacterHitZoneGroup.LOWERTORSO:
			{
				return "#AR-Inventory_Abdomen";
			} break;

			case ECharacterHitZoneGroup.LEFTARM:
			{
				return "#AR-Inventory_LeftArm";
			} break;

			case ECharacterHitZoneGroup.RIGHTARM:
			{
				return "#AR-Inventory_RightArm";
			} break;

			case ECharacterHitZoneGroup.LEFTLEG:
			{
				return "#AR-Inventory_LeftLeg";
			} break;

			case ECharacterHitZoneGroup.RIGHTLEG:
			{
				return "#AR-Inventory_RightLeg";
			} break;
		}

		return string.Empty;
	}

	//------------------------------------------------------------------------------------------------
	void InitializeHitZoneUI(BaseInventoryStorageComponent storage, SCR_InventoryMenuUI menuManager, int hitZoneId, IEntity player, bool updateAttachmentStorage = true)
	{
		m_pStorage = storage;
		m_pInventoryMenu = menuManager;
		m_Player = player;
		
		m_TourniquetStorage = SCR_TourniquetStorageComponent.Cast(m_Player.FindComponent(SCR_TourniquetStorageComponent));
		m_TourniquetStorage.GetOnTourniquetMoved().Insert(OnTourniquetMoved);
		
		m_pCharDmgManager = SCR_CharacterDamageManagerComponent.Cast(player.FindComponent(SCR_CharacterDamageManagerComponent));
		if (!m_pCharDmgManager)
			return;

		m_eHitZoneGroup = m_pCharDmgManager.LIMB_GROUPS.Get( hitZoneId  );
		m_pDamageHandler.SetParentContainer(this);
		
		m_pBloodHitZone = m_pCharDmgManager.GetBloodHitZone();
		m_pBloodHitZone.GetOnDamageStateChanged().Insert(UpdateHitZoneState);
		m_aGroupHitZones.Insert(m_pBloodHitZone);

		UpdateHitZoneState(m_pBloodHitZone, updateAttachmentStorage);

		m_pCharDmgManager.GetOnDamageEffectAdded().Insert(UpdateHitZoneDOTAdded);
		m_pCharDmgManager.GetOnDamageEffectRemoved().Insert(UpdateHitZoneDOTRemoved);
		m_pCharDmgManager.GetHitZonesOfGroup(m_eHitZoneGroup, m_aGroupHitZones);
		m_iBoneIndex = m_Player.GetAnimation().GetBoneIndex(m_pCharDmgManager.GetBoneName(m_eHitZoneGroup));

		SCR_CharacterHitZone scrHZ;
		const int lastId = m_aGroupHitZones.Count() - 1;
		foreach (int i, HitZone hz : m_aGroupHitZones)
		{
			scrHZ = SCR_CharacterHitZone.Cast(hz);
			if (!scrHZ)
				continue;

			scrHZ.GetOnDamageStateChanged().Insert(UpdateHitZoneState);
			UpdateHitZoneState(scrHZ, updateAttachmentStorage && i == lastId); // only update attachment storage with the last HZ data to not waste time
		}

		const Widget newStorage = GetGame().GetWorkspace().CreateWidgets(m_pInventoryMenu.BACKPACK_STORAGE_LAYOUT, m_wRoot);
		m_pStorageUI = new SCR_InventoryHitZoneUI(storage, null, menuManager, 0, null, this, m_pCharDmgManager.GetGroupTourniquetted(m_eHitZoneGroup));
		newStorage.AddHandler(m_pStorageUI);
		m_pStorageUI.GetRootWidget().SetVisible(false);
	}
	
	protected void OnTourniquetMoved(int hzGroupId)
	{
		array<HitZone> hzs = {};
		m_pCharDmgManager.GetHitZonesOfGroup(hzGroupId, hzs);
		UpdateHitZoneState(SCR_HitZone.Cast(hzs[0]));
		
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateHitZoneDOTAdded(notnull SCR_DamageEffect dmgEffect)
	{
		UpdateHitZoneState(SCR_HitZone.Cast(dmgEffect.GetAffectedHitZone()));
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateHitZoneDOTRemoved(notnull SCR_DamageEffect dmgEffect)
	{
		UpdateHitZoneState(SCR_HitZone.Cast(dmgEffect.GetAffectedHitZone()));
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateBleedingHitZone(EDamageType dType, float dps, HitZone hz = null)
	{
		if (dType != EDamageType.BLEEDING)
			return;

		UpdateHitZoneState(SCR_HitZone.Cast(hz));
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateBleedingHitZoneRemoved(EDamageType dType, HitZone hz = null)
	{
		if (dType != EDamageType.BLEEDING)
			return;

		UpdateHitZoneState(SCR_HitZone.Cast(hz));
	}	
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateHitZoneState(SCR_HitZone hz, bool updateAttachmentStorage = true)
	{
		if (!hz || !m_aGroupHitZones.Contains(hz))
			return;
		
		const float health = m_pCharDmgManager.GetGroupHealthScaled(m_eHitZoneGroup);
		float bleeding;
		
		SCR_RegeneratingHitZone regenHitZone;
		foreach (HitZone groupHZ : m_aGroupHitZones)
		{
			if (groupHZ == m_pBloodHitZone)
				continue;
			
			regenHitZone = SCR_RegeneratingHitZone.Cast(groupHZ);
			if (!regenHitZone)
				continue;
			
			if (regenHitZone.GetHitZoneDamageOverTime(EDamageType.BLEEDING) <= 0)
				continue;

			bleeding = m_pCharDmgManager.GetGroupDamageOverTime(m_eHitZoneGroup, EDamageType.BLEEDING);
			break;
		}		

		const float dotHealing = -m_pCharDmgManager.GetGroupDamageOverTime(m_eHitZoneGroup, EDamageType.HEALING);
		const float dotRegen = m_pCharDmgManager.GetGroupDamageOverTime(m_eHitZoneGroup, EDamageType.REGENERATION);
		const float regen = (dotHealing + dotRegen);

		const bool tourniquetted = m_pCharDmgManager.GetGroupTourniquetted(m_eHitZoneGroup);
		const bool salineBagged = m_pCharDmgManager.GetGroupSalineBagged(m_eHitZoneGroup);

		if (m_pStorageUI)
			m_pStorageUI.SetTourniquetted(tourniquetted);
		
		const bool bleedingVisible = (bleeding > 0 || tourniquetted || salineBagged);
		const bool damageVisible = (health < 1 && bleeding == 0);
		m_pDamageHandler.GetRootWidget().SetVisible(damageVisible || bleedingVisible);
		m_pDamageHandler.UpdateHitZoneState(health, bleeding, regen, tourniquetted, salineBagged);

		SetGlowVisible(bleedingVisible);

		if (!m_pInventoryMenu)
			return;

		if (updateAttachmentStorage && (m_bSelected || m_pInventoryMenu.IsUsingGamepad()))
			ShowApplicableItems();
		
		if (bleeding > 0 || health < 1 || tourniquetted || salineBagged)
			m_pInventoryMenu.AddItemToAttachmentSelection(GetHitZoneName(), this);
		else
			m_pInventoryMenu.RemoveItemFromAttachmentSelection(GetHitZoneName());
	}

	void SetGlowColor(Color color)
	{
		if (m_wGlow)
			m_wGlow.SetColor(color);
	}

	//------------------------------------------------------------------------------------------------
	Widget GetRootWidget()
	{
		return m_wRoot;
	}

	TNodeId GetBoneIndex()
	{
		return m_iBoneIndex;
	}

	SCR_InventoryMenuUI GetInventoryHandler()
	{
		return m_pInventoryMenu;
	}

	SCR_InventoryHitZoneUI GetStorage()
	{
		return m_pStorageUI;
	}
}

//------------------------------------------------------------------------------------------------
class SCR_InventoryHitZonePointUI : ScriptedWidgetComponent
{
	protected SCR_DamageStateUIInfo m_DamageState;
	protected SCR_DamageStateUIComponent m_DamageStateUIComp;

	[Attribute("1")]
	protected bool m_bAllowHoverOver;

	[Attribute("0")]
	protected bool m_bAllowProgressAnim;

	[Attribute("0")]
	protected bool m_bBleedingHitZone;

	[Attribute()]
	protected ref array<int> m_aDOTLevels;

	[Attribute("IconOverlay")]
	protected string m_sIconOverlay;
	protected Widget m_wIconOverlay;

	[Attribute("Icon")]
	protected string m_sIcon;
	protected ImageWidget m_wIcon;

	[Attribute("Outline")]
	protected string m_sOutline;
	protected ImageWidget m_wOutline;

	[Attribute("Background")]
	protected string m_sBackground;
	protected ImageWidget m_wBackground;

	[Attribute("Tourniquet")]
	protected string m_sIconTourniquet;
	protected ImageWidget m_wIconTourniquet;

	[Attribute("Saline")]
	protected string m_sIconSalineBag;
	protected ImageWidget m_wIconSalineBag;

	protected Widget m_wRoot;
	protected ButtonWidget m_wButton;

	protected static SCR_InventoryHitZonePointUI s_pSelectedPoint;
	protected bool m_bSelected;

	protected const float OPACITY_DEFAULT = 0.4;
	protected const float OPACITY_BLOOD = 0.5;
	protected const float OPACITY_SOLID = 1;

	//------------------------------------------------------------------------------------------------
	protected ECharacterHitZoneGroup m_eHitZoneGroup;
	protected ref array<HitZone> m_aGroupHitZones = {};
	
	protected string m_sSevereDamage 	= "#AR-DamageInfo_Damage_Severe";
	protected string m_sHighDamage 		= "#AR-DamageInfo_Damage_High";
	protected string m_sMediumDamage 	= "#AR-DamageInfo_Damage_Medium";
	protected string m_sLowDamage 		= "#AR-DamageInfo_Damage_Low";
	
	protected string m_sSevereBleeding 	= "#AR-DamageInfo_Bleeding_Severe";
	protected string m_sHighBleeding 	= "#AR-DamageInfo_Bleeding_High";
	protected string m_sMediumBleeding 	= "#AR-DamageInfo_Bleeding_Medium";
	protected string m_sLowBleeding 	= "#AR-DamageInfo_Bleeding_Low";	
	
	protected string m_sSevereBloodLoss = "#AR-DamageInfo_Bloodloss_Severe";
	protected string m_sHighBloodLoss = "#AR-DamageInfo_Bloodloss_High";
	protected string m_sMediumBloodLoss = "#AR-DamageInfo_Bloodloss_Medium";
	protected string m_sLowBloodLoss = "#AR-DamageInfo_Bloodloss_Low";
	
	protected const string DAMAGE_INFO = "{55AFA256E1C20FB2}UI/layouts/Menus/Inventory/InventoryDamageInfo.layout";
	
	float m_fHighDamageThreshold = 0.75;
	float m_fMediumDamageThreshold = 0.50;
	float m_fLowDamageThreshold = 0.25;
	float m_fHighBleedingThreshold = 20;
	float m_fMediumBleedingThreshold = 20;	
	float m_fLowBleedingThreshold = 10;
	float m_fHighBloodLossThreshold = 0.4376;
	float m_fMediumBloodLossThreshold = 0.6251;	
	float m_fLowBloodLossThreshold = 0.8125;

	protected IEntity m_Player;
	protected ref SCR_InventoryHitZoneUI m_pStorageHandler;
	protected SCR_InventoryHitZonePointContainerUI m_pParentContainer;

	[Attribute("Anim")]
	protected string m_sAnimWidgetName;
	protected Widget m_wAnim;
	protected SCR_InventoryDamageInfoUI	m_DamageInfo = null;

	protected WidgetAnimationBase m_IconOpacityAnim;
	protected WidgetAnimationBase m_IconSizeAnim;
	protected WidgetAnimationBase m_IconOutlineAnim;
	protected WidgetAnimationBase m_DamageIconOpacityAnim;

	protected bool m_bIsRegenAnim;
	protected bool m_bIsBloodAnim;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wRoot = w;

		float opacity = OPACITY_DEFAULT;
		if (!m_bAllowHoverOver)
			opacity = OPACITY_SOLID;

		m_wButton = ButtonWidget.Cast(w.FindAnyWidget("Btn"));
		m_wIconOverlay = w.FindAnyWidget(m_sIconOverlay);
		m_wIcon = ImageWidget.Cast(w.FindAnyWidget(m_sIcon));
		m_wBackground = ImageWidget.Cast(w.FindAnyWidget(m_sBackground));
		m_wOutline = ImageWidget.Cast(w.FindAnyWidget(m_sOutline));
		m_wIconTourniquet = ImageWidget.Cast(w.FindAnyWidget(m_sIconTourniquet));
		m_wIconSalineBag = ImageWidget.Cast(w.FindAnyWidget(m_sIconSalineBag));
		m_wAnim = w.FindAnyWidget(m_sAnimWidgetName);

		Widget damageOverlay = w.FindAnyWidget("DamageOverlay");
		if (damageOverlay)
		{
			m_DamageStateUIComp = SCR_DamageStateUIComponent.Cast(damageOverlay.FindHandler(SCR_DamageStateUIComponent));
			if (m_DamageStateUIComp)
				m_DamageState = m_DamageStateUIComp.GetUiInfo();
		}

		Highlight(!m_bAllowHoverOver);
	}
	
	override void HandlerDeattached(Widget w)
	{
		StopBloodDropAnim();
		StopRegenAnim();
		HideVirtualHZInfo();
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		if (w.GetName() == "BigBloodIcon")
			ShowVirtualHZInfo(w, true, false);
		else if (w.GetName() == "BigDamageIcon")
			ShowVirtualHZInfo(w, false, true);		

		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		if (w.GetName() == "BigBloodIcon")
			HideVirtualHZInfo();
		else if (w.GetName() == "BigDamageIcon")
			HideVirtualHZInfo();	
		
		return false;
	}

	//------------------------------------------------------------------------------------------------
	void Select(bool select)
	{
		if (s_pSelectedPoint)
		{
			s_pSelectedPoint.m_bSelected = false;
			s_pSelectedPoint.Highlight(false);
		}

		m_bSelected = select;
		Highlight(select);
		if (select)
			s_pSelectedPoint = this;
		else
			s_pSelectedPoint = null;
	}

	//------------------------------------------------------------------------------------------------
	void Highlight(bool highlight)
	{
		ShowHitZoneInfo(highlight);
		
		if (!m_bAllowHoverOver)
			return;

		float opacity = OPACITY_SOLID;
		if (!highlight)
			opacity = OPACITY_DEFAULT;

		if (m_wIconOverlay)
			m_wIconOverlay.SetOpacity(opacity);
	}
	
	//------------------------------------------------------------------------------------------------
	protected SCR_InventoryMenuUI GetInventoryMenuUI()
	{
		SCR_InventoryMenuUI inventoryMenuUI = SCR_InventoryMenuUI.Cast(GetGame().GetMenuManager().FindMenuByPreset(ChimeraMenuPreset.Inventory20Menu));
		if (!inventoryMenuUI)
			return null;
		
		return inventoryMenuUI;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gather data for, and create, infowidget for virtual hitzones
	protected void ShowVirtualHZInfo(Widget w, bool bloodIcon = false, bool fractureIcon = false)
	{
		if (!bloodIcon && !fractureIcon)
			return;
		
		int groupDamageIntensity;
		bool regenerating;
		float bleedingRate, bloodHealth;
		bool isTourniquetted, isSalineBagged, isArmFractured, isLegFractured;
		string damageIntensity, damageIntensityText, bleedingIntensityText, bloodLevelText;
		
		GetHitZoneInfo(ECharacterHitZoneGroup.VIRTUAL, groupDamageIntensity, regenerating, bleedingRate, bloodHealth, isTourniquetted, isSalineBagged, isArmFractured, isLegFractured);
		GetDamageInfoTexts(ECharacterHitZoneGroup.VIRTUAL, groupDamageIntensity, bleedingRate, damageIntensity, damageIntensityText, bleedingIntensityText);
		GetBloodInfoText(bloodHealth, bloodLevelText);
		
		Widget localInfoWidget;
		if (!m_DamageInfo)
 	 	{
			SCR_InventoryMenuUI inventoryMenuUI = GetInventoryMenuUI();
			if (!inventoryMenuUI)
 	 			return;
			
			localInfoWidget = GetGame().GetWorkspace().CreateWidgets(DAMAGE_INFO, inventoryMenuUI.GetRootWidget());
			if (!localInfoWidget)
				return;
			
			localInfoWidget.SetVisible(true);
			localInfoWidget.SetOpacity(0);
			
			localInfoWidget.AddHandler(new SCR_InventoryDamageInfoUI());
			m_DamageInfo = SCR_InventoryDamageInfoUI.Cast(localInfoWidget.FindHandler(SCR_InventoryDamageInfoUI));
		}
	
		if (!m_DamageInfo)
 	 		return;

		m_DamageInfo.SetName("");
		if (fractureIcon)
			m_DamageInfo.SetFractureStateVisible(isArmFractured, isLegFractured);
		else if (bloodIcon)
			m_DamageInfo.SetBleedingStateVisible(bloodHealth < 1, bloodLevelText);
		
		//callLater is here because the widget needs to be created before size can be determined and location can be set
		GetGame().GetCallqueue().CallLater(EnableVirtualHZInfoWidget, 300, false, localInfoWidget, w);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Determine size and location for the infowidget
	protected void EnableVirtualHZInfoWidget(Widget localInfoWidget, Widget w)
	{
		if (!m_DamageInfo)
			return;
		
		float x, y;
		w.GetScreenPos(x, y);

		float width, height;
		w.GetScreenSize(width, height);

		float screenSizeX, screenSizeY;
		GetGame().GetWorkspace().GetScreenSize(screenSizeX, screenSizeY);

		localInfoWidget.SetOpacity(1);
				
		float infoWidth, infoHeight;
		int iMouseX, iMouseY;
		
		localInfoWidget.GetScreenSize(infoWidth, infoHeight);
		iMouseX = x;
		iMouseY = y + height;
		if (x + infoWidth > screenSizeX)
			iMouseX = screenSizeX - infoWidth - width * 0.5; // offset info if it would go outside of the screen

		m_DamageInfo.Move( GetGame().GetWorkspace().DPIUnscale( iMouseX ) - infoWidth * 0.60, GetGame().GetWorkspace().DPIUnscale( iMouseY ) );
	}
	
	protected void HideVirtualHZInfo()
	{
		if ( !m_DamageInfo )
			return;
		m_DamageInfo.Hide();
		m_DamageInfo.Destroy();
	}

	protected void ShowHitZoneInfo(bool show, bool virtualHZ = false)
	{
		if (!m_pParentContainer && !virtualHZ)
			return;

		if (!show && !virtualHZ)
		{
			m_pParentContainer.GetInventoryHandler().HideDamageInfo();
			return;
		}

		ECharacterHitZoneGroup group = ECharacterHitZoneGroup.VIRTUAL;
		if (!virtualHZ)
			group = m_pParentContainer.GetHitZoneGroup();
		
		int groupDamageIntensity;
		bool regenerating;
		float bleedingRate, bloodHealth;
		bool isTourniquetted, isSalineBagged, isArmFractured, isLegFractured;
		const bool isMorphined; // TODO: check for good const usage
		string damageIntensity, damageIntensityText, bleedingIntensityText;
		
		GetHitZoneInfo(group, groupDamageIntensity, regenerating, bleedingRate, bloodHealth, isTourniquetted, isSalineBagged, isArmFractured, isLegFractured);
		GetDamageInfoTexts(group, groupDamageIntensity, bleedingRate, damageIntensity, damageIntensityText, bleedingIntensityText);
		
		SCR_InventoryDamageUIInfo damageUIInfo;
		damageUIInfo = new SCR_InventoryDamageUIInfo(
			groupDamageIntensity != 0,
			regenerating,
			bleedingRate != 0, 
			isTourniquetted, 
			isSalineBagged,
			isMorphined,
			isArmFractured,
			isLegFractured,
			damageIntensity,
			damageIntensityText,
			bleedingIntensityText
			);
			
		string name;
		if (!virtualHZ)
			name = m_pParentContainer.GetHitZoneName();

		m_pParentContainer.GetInventoryHandler().ShowDamageInfo(name, damageUIInfo);
	}
	
	//------------------------------------------------------------------------------------------------	
	void GetDamageInfoTexts(EHitZoneGroup group, int groupDamageIntensity, float bleedingRate, out string damageIntensity, out string damageIntensityText, out string bleedingIntensityText)
	{
		if (groupDamageIntensity == 0)
		{
			damageIntensity = "";
			damageIntensityText = "";
		}
		else if (groupDamageIntensity == 1)
		{
			damageIntensity = "Wound_1_UI";
			damageIntensityText = m_sLowDamage;
		}
		else if (groupDamageIntensity == 2)
		{
			damageIntensity = "Wound_2_UI";
			damageIntensityText = m_sMediumDamage;
		}
		else if (groupDamageIntensity == 3)
		{
			damageIntensity = "Wound_3_UI";
			damageIntensityText = m_sHighDamage;
		}
		else if (groupDamageIntensity == 4)
		{
			damageIntensity = "Wound_4_UI";
			damageIntensityText = m_sSevereDamage;
		}
				
		if (bleedingRate < m_fLowBleedingThreshold)
			bleedingIntensityText = m_sLowBleeding;
		else if (bleedingRate >= m_fLowBleedingThreshold && bleedingRate < m_fMediumBleedingThreshold)
			bleedingIntensityText = m_sMediumBleeding;
		else if (bleedingRate >= m_fMediumBleedingThreshold && bleedingRate < m_fHighBleedingThreshold)
			bleedingIntensityText = m_sHighBleeding;
		else if (bleedingRate >= m_fHighBleedingThreshold)
			bleedingIntensityText = m_sSevereBleeding;
	}
	
	//------------------------------------------------------------------------------------------------	
	void GetBloodInfoText(float bloodHitZoneHealthScaled, out string bloodLossText)
	{
		if (bloodHitZoneHealthScaled > m_fLowBloodLossThreshold)
			bloodLossText = m_sLowBloodLoss;
		else if (bloodHitZoneHealthScaled > m_fMediumBloodLossThreshold)
			bloodLossText = m_sMediumBloodLoss;
		else if (bloodHitZoneHealthScaled > m_fHighBloodLossThreshold)
			bloodLossText = m_sHighBloodLoss;
		else
			bloodLossText = m_sSevereBloodLoss;
	}
	
	//------------------------------------------------------------------------------------------------	
	protected void GetHitZoneInfo(EHitZoneGroup group, out int groupDamageIntensity, out bool regenerating, out float bleedingRate, out float bloodHealth, out bool isTourniquetted, out bool isSalineBagged, out bool isArmFractured, out bool isLegFractured)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(GetGame().GetPlayerController().GetControlledEntity());
		if (!character)
			return;
		
		SCR_CharacterDamageManagerComponent damageMan = SCR_CharacterDamageManagerComponent.Cast( character.GetDamageManager() );
		if (!damageMan)
			return;		
		
		const float groupHealth = damageMan.GetGroupHealthScaled(group);
		bleedingRate = damageMan.GetGroupDamageOverTime(group, EDamageType.BLEEDING);
		isTourniquetted = damageMan.GetGroupTourniquetted(group);
		isSalineBagged = damageMan.GetGroupSalineBagged(group);
		
		if (group == ECharacterHitZoneGroup.VIRTUAL)
		{
			isArmFractured = damageMan.GetAimingDamage() > 0;
			isLegFractured = damageMan.GetMovementDamage() > 0;
			SCR_CharacterBloodHitZone hz = damageMan.GetBloodHitZone();
			if (!hz)
				return;
			
			bleedingRate = hz.GetTotalBleedingAmount();
			bloodHealth = hz.GetHealthScaled();
		}
		
		if (group == ECharacterHitZoneGroup.LEFTARM || group == ECharacterHitZoneGroup.RIGHTARM)
			isArmFractured = damageMan.GetAimingDamage() > 0;
		else if (group == ECharacterHitZoneGroup.LEFTLEG || group == ECharacterHitZoneGroup.RIGHTLEG)
			isLegFractured = damageMan.GetMovementDamage() > 0;
		
		if (damageMan.GetGroupDamageOverTime(group, EDamageType.HEALING) == 0 && damageMan.GetGroupDamageOverTime(group, EDamageType.REGENERATION) == 0)
			regenerating = false;
		else
			regenerating = true;

		if (groupHealth == 1)
			groupDamageIntensity = 0;
		else if (groupHealth < m_fLowDamageThreshold)
			groupDamageIntensity = 4;
		else if (groupHealth >= m_fLowDamageThreshold && groupHealth < m_fMediumDamageThreshold)
			groupDamageIntensity = 3;
		else if (groupHealth >= m_fMediumDamageThreshold && groupHealth < m_fHighDamageThreshold)
			groupDamageIntensity = 2;
		else if (groupHealth >= m_fHighDamageThreshold)
			groupDamageIntensity = 1;
	}

	//------------------------------------------------------------------------------------------------
	void UpdateHitZoneState(float health, float bleeding, float regen, bool tourniquetted, bool salineBagged)
	{
		if (!m_bAllowHoverOver || !m_DamageState)
			return;

		if (bleeding > 0)
		{
			if (tourniquetted || salineBagged)
				StopBloodDropAnim();
			else
				PlayBloodDropAnim();
		}
		else
		{
			StopBloodDropAnim();
		}
		
		int damageIntensity;
		float bleedRate, bloodHealth;
		bool regenerating;
		bool isArmFractured, isLegFractured;
		GetHitZoneInfo(m_eHitZoneGroup, damageIntensity, regenerating, bleedRate, bloodHealth, tourniquetted, salineBagged, isArmFractured, isLegFractured);
		if (damageIntensity > 0)
			m_DamageStateUIComp.SetVisuals(m_DamageState, damageIntensity-1);

		SetRegeneration(regen, bleeding);

		m_wIcon.SetVisible(!tourniquetted && !salineBagged);
		if (m_wIconTourniquet)
			m_wIconTourniquet.SetVisible(tourniquetted);
		if (m_wIconSalineBag)
			m_wIconSalineBag.SetVisible(salineBagged && !tourniquetted);
	}

	protected void SetRegeneration(float regen, float bleeding)
	{
		if (!m_DamageState)
			return;

		bool isRegen = (regen > 0);
		float regenSpeed = Math.AbsFloat(regen);
		if (regenSpeed < 0.3)
			regenSpeed = 0.3;

		if (isRegen && !bleeding)
			PlayRegenAnim(regenSpeed);
		else
			StopRegenAnim();

		Color bckg = m_DamageState.GetBackgroundColor();
		Color outline = m_DamageState.GetOutlineColor();

		bool glow = true;
		if (isRegen && bleeding == 0)
		{
			bckg = m_DamageState.GetBackgroundColorRegen();
			outline = m_DamageState.GetOutlineColorRegen();
			glow = false;
		}

		m_pParentContainer.SetGlowVisible(!isRegen && glow);
		m_wBackground.SetColor(bckg);
		m_wOutline.SetColor(outline);
	}

	protected void PlayRegenAnim(float regenSpeed = 1)
	{
		if (!m_DamageState)
			return;

		if (m_bIsRegenAnim && m_IconOutlineAnim)
		{
			m_IconOutlineAnim.SetSpeed(regenSpeed);
			return;
		}

		m_bIsRegenAnim = true;
		m_IconOutlineAnim = AnimateWidget.Color(m_wOutline, m_DamageState.GetOutlineColorRegen(), regenSpeed);
		if (m_IconOutlineAnim)
		{
			m_IconOutlineAnim.SetRepeat(true);
			m_IconOutlineAnim.GetOnCycleCompleted().Insert(OnAnimCycleComplete);
		}
	}

	protected void StopRegenAnim()
	{
		if (!m_bIsRegenAnim)
		 	return;

		m_bIsRegenAnim = false;
		if (m_IconOutlineAnim)
		{
			m_IconOutlineAnim.GetOnCycleCompleted().Remove(OnAnimCycleComplete);
			AnimateWidget.StopAnimation(m_IconOutlineAnim);		
		}
	}

	protected void PlayBloodDropAnim(float speed = 1)
	{
		if (m_bIsBloodAnim)
		{
			m_IconOpacityAnim.SetSpeed(speed);
			m_IconSizeAnim.SetSpeed(speed);
			m_DamageIconOpacityAnim.SetSpeed(speed);
			return;
		}

		if (m_wAnim && !m_bIsBloodAnim)
		{
			m_wAnim.SetVisible(true);

			float targetOpacity = OPACITY_BLOOD;
			float opacitySpeed = speed;
			m_IconOpacityAnim = AnimateWidget.Opacity(m_wAnim, targetOpacity, opacitySpeed);
			if (m_IconOpacityAnim)
			{
				m_IconOpacityAnim.SetRepeat(true);
				m_IconOpacityAnim.GetOnCycleCompleted().Insert(OnAnimCycleComplete);
			}

			float targetSize[2] = {32, 32};
			m_IconSizeAnim = AnimateWidget.Size(m_wAnim, targetSize, speed);
			if (m_IconSizeAnim)
			{
				m_IconSizeAnim.SetRepeat(true);
				m_IconSizeAnim.GetOnCycleCompleted().Insert(OnAnimCycleComplete);
			}

			m_bIsBloodAnim = true;
			
			m_wIcon.SetOpacity(0);
			m_DamageIconOpacityAnim	= AnimateWidget.Opacity(m_wIcon, 1, opacitySpeed);
			if (m_DamageIconOpacityAnim)
			{
				m_DamageIconOpacityAnim.SetRepeat(true);
				m_DamageIconOpacityAnim.GetOnCycleCompleted().Insert(OnAnimCycleComplete);
			}			
		}
	}

	protected void OnAnimCycleComplete(WidgetAnimationBase anim)
	{
		/* 
		DO NOT REMOVE OR YOU'LL BREAK ANIM REPEAT FUNCTIONALITY
		WidgetAnimationBase.m_OnCycleCompleted needs to exist, 
		otherwise WidgetAnimationBase.OnUpdate will return false after the cycle is finished and stop animating
		*/
	}

	protected void StopBloodDropAnim()
	{
		m_bIsBloodAnim = false;

		if (m_IconOpacityAnim)
			m_IconOpacityAnim.GetOnCycleCompleted().Remove(OnAnimCycleComplete);
		if (m_IconSizeAnim)
			m_IconSizeAnim.GetOnCycleCompleted().Remove(OnAnimCycleComplete);
		if (m_DamageIconOpacityAnim)
			m_DamageIconOpacityAnim.GetOnCycleCompleted().Remove(OnAnimCycleComplete);

		AnimateWidget.StopAnimation(m_IconOpacityAnim);
		AnimateWidget.StopAnimation(m_IconSizeAnim);
		AnimateWidget.StopAnimation(m_DamageIconOpacityAnim);

		if (m_wIcon)
			m_wIcon.SetOpacity(1);
		if (m_wAnim)
			m_wAnim.SetVisible(false);
	}

	void SetBloodLevelProgress(float bloodLevel)
	{
 		if (!m_bAllowProgressAnim || !m_wIcon)
			return;

		float mask = Math.Lerp(0.25, 0.75, 1 - bloodLevel);
		m_wIcon.SetMaskProgress(mask);
	}

	Widget GetRootWidget()
	{
		return m_wRoot;
	}

	void SetParentContainer(SCR_InventoryHitZonePointContainerUI container)
	{
		m_pParentContainer = container;
		m_eHitZoneGroup = container.GetHitZoneGroup();
	}
};
	//---- REFACTOR NOTE END ----