class SCR_InspectCasualtyWidget : SCR_InfoDisplayExtended
{
	const ResourceName INSPECT_CASUALTY_LAYOUT = "{11AC7D61FD4CF3F6}UI/layouts/Damage/InspectCasualtyMenu.layout";

	protected SCR_CharacterDamageManagerComponent m_CharDamageManager;
	protected Widget m_wCasualtyInspectWidget;

	IEntity m_Target;

	protected const float UPDATE_FREQ = 0.5;
	protected const float MAX_SHOW_DURATION = 5;
	protected const string TARGET_BONE = "Spine4";
	protected float m_fTimeTillUpdate;
	protected float m_fTimeTillClose;
	protected bool m_bShouldBeVisible;

	//------------------------------------------------------------------------------------------------
	override void DisplayStartDraw(IEntity owner)
	{
		m_wCasualtyInspectWidget = GetRootWidget();
		DisableWidget();
	}

	//------------------------------------------------------------------------------------------------
	override event void DisplayUpdate(IEntity owner, float timeSlice)
	{
		if (m_fTimeTillClose < 0)
			DisableWidget();
		else
			m_fTimeTillClose -= timeSlice;

		if (m_fTimeTillUpdate > 0)
		{
			m_fTimeTillUpdate -= timeSlice;
		}
		else
		{
			m_fTimeTillUpdate = UPDATE_FREQ;
			UpdateTarget();
		}

		UpdateWidget();
	}

	//------------------------------------------------------------------------------------------------
	//! Start showing the widget
	bool ShowInspectCasualtyWidget(IEntity targetCharacter)
	{
		if (!m_wCasualtyInspectWidget)
			return false;

		ChimeraCharacter char = ChimeraCharacter.Cast(targetCharacter);
		if (!char)
			return false;

		CharacterControllerComponent targetController = char.GetCharacterController();
		if (!targetController)
			return false;

		UpdateTarget();
		EnableWidget();

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Check if target is still alive and update widget if so
	protected void UpdateTarget()
	{
		if (!m_Target)
		{
			DisableWidget();
			return;
		}

		ChimeraCharacter char = ChimeraCharacter.Cast(m_Target);
		if (!char)
			return;

		CharacterControllerComponent controller = char.GetCharacterController();
		if (controller.GetLifeState() == ECharacterLifeState.DEAD)
		{
			DisableWidget();
			return;
		}

		UpdateWidgetData();
	}

	//------------------------------------------------------------------------------------------------
	//! Gather and update data of target character into widget
	protected void UpdateWidgetData()
	{
		if (!m_Target || !m_wCasualtyInspectWidget)
			return;

		string sName;
		GetCasualtyName(sName, m_Target);

		if (GetGame().GetPlatformService().GetLocalPlatformKind() == PlatformKind.PSN)
		{
			PlayerManager playerMgr = GetGame().GetPlayerManager();

			if (playerMgr && playerMgr.GetPlatformKind(playerMgr.GetPlayerIdFromControlledEntity(m_Target)) == PlatformKind.PSN)
				sName = string.Format("<color rgba=%1><image set='%2' name='%3' scale='%4'/></color>", UIColors.FormatColor(GUIColors.ENABLED), UIConstants.ICONS_IMAGE_SET, UIConstants.PLATFROM_PLAYSTATION_ICON_NAME, 2) + sName;
			else
				sName = string.Format("<color rgba=%1><image set='%2' name='%3' scale='%4'/></color>", UIColors.FormatColor(GUIColors.ENABLED), UIConstants.ICONS_IMAGE_SET, UIConstants.PLATFROM_GENERIC_ICON_NAME, 2) + sName;
		}

		float bleedingRate;
		int groupDamageIntensity;
		bool regenerating, isTourniquetted, isSalineBagged, isMorphined;
		string damageIntensity, damageIntensityText, bleedingIntensityText;
		array<bool> hZGroupsBleeding = {};

		SCR_InventoryHitZonePointUI hitZonePointUI = new SCR_InventoryHitZonePointUI();
		GetDamageInfo(hitZonePointUI, m_Target, bleedingRate, hZGroupsBleeding, groupDamageIntensity, regenerating, isTourniquetted, isSalineBagged, isMorphined);
		hitZonePointUI.GetDamageInfoTexts(EHitZoneGroup.VIRTUAL, groupDamageIntensity, bleedingRate, damageIntensity, damageIntensityText, bleedingIntensityText);

		SCR_InventoryDamageInfoUI damageInfoUI = SCR_InventoryDamageInfoUI.Cast(m_wCasualtyInspectWidget.FindHandler(SCR_InventoryDamageInfoUI));
		if (damageInfoUI)
		{
			damageInfoUI.SetName(sName);
			damageInfoUI.SetDamageStateVisible(groupDamageIntensity, regenerating, damageIntensity, damageIntensityText);
			damageInfoUI.SetBleedingStateVisible(bleedingRate > 0, bleedingIntensityText);
			damageInfoUI.SetTourniquetStateVisible(isTourniquetted);
			damageInfoUI.SetSalineBagStateVisible(isSalineBagged);
			damageInfoUI.SetMorphineStateVisible(isMorphined);
			damageInfoUI.SetFractureStateVisible(0, 0);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Update widget position and opacity
	protected void UpdateWidget()
	{
		if (!m_Target || !m_wCasualtyInspectWidget || !m_bIsEnabled)
			return;

		vector boneVector[4];
		m_Target.GetAnimation().GetBoneMatrix(m_Target.GetAnimation().GetBoneIndex(TARGET_BONE), boneVector);

		vector WPPos = boneVector[3] + m_Target.GetOrigin();
		vector pos = GetGame().GetWorkspace().ProjWorldToScreen(WPPos, GetGame().GetWorld());

		// Handle off-screen coords
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		int winX = workspace.GetWidth();
		int winY = workspace.GetHeight();
		int posX = workspace.DPIScale(pos[0]);
		int posY = workspace.DPIScale(pos[1]);

		// If widget off screen, remove widget
		if (posX < 0 || posX > winX || posY > winY || posY < 0)
		{
			DisableWidget();
			return;
		}

		FrameSlot.SetPos(m_wCasualtyInspectWidget.GetChildren(), pos[0], pos[1]);

		float dist = vector.Distance(GetGame().GetPlayerController().GetControlledEntity().GetOrigin(), WPPos);
		if (dist >= 4)
		{
			DisableWidget();
			return;
		}

		float distanceOpacityReduction = 1;

		if (dist <= 3)
			distanceOpacityReduction = 0;
		else
			distanceOpacityReduction = Math.InverseLerp(3, 4, dist);

		m_wCasualtyInspectWidget.SetOpacity(1 - distanceOpacityReduction);
	}

	//------------------------------------------------------------------------------------------------
	protected void GetDamageInfo(SCR_InventoryHitZonePointUI hitZonePointUI, IEntity targetEntity, inout float bleedingRate, inout array<bool> hZGroupsBleeding, inout int damageIntensity, inout bool regenerating, inout bool isTourniquetted, inout bool isSalineBagged, inout bool isMorphined)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(targetEntity);
		if (!character)
			return;

		SCR_CharacterDamageManagerComponent damageMan = SCR_CharacterDamageManagerComponent.Cast(character.GetDamageManager());
		if (!damageMan)
			return;

		float defaultHZHealth = damageMan.GetHealthScaled();
		bleedingRate = damageMan.GetBloodHitZone().GetTotalBleedingAmount();
		if (bleedingRate)
		{
			hZGroupsBleeding = {};
			hZGroupsBleeding.Resize(damageMan.LIMB_GROUPS.Count());

			foreach (ECharacterHitZoneGroup group : damageMan.LIMB_GROUPS)
			{
				hZGroupsBleeding[damageMan.LIMB_GROUPS.Find(group)] = damageMan.GetGroupDamageOverTime(group, EDamageType.BLEEDING) != 0;
			}
		}

		array<EHitZoneGroup> limbGroups = {};
		damageMan.GetAllLimbs(limbGroups);
		foreach (EHitZoneGroup group : limbGroups)
		{
			if (!isTourniquetted)
				isTourniquetted = damageMan.GetGroupTourniquetted(group);

			if (!isSalineBagged)
				isSalineBagged = damageMan.GetGroupSalineBagged(group);

			if (!isMorphined)
			{
				array<ref SCR_PersistentDamageEffect> effects = damageMan.GetAllPersistentEffectsOfType(SCR_MorphineDamageEffect);
				isMorphined = !effects.IsEmpty();
			}

			if (!regenerating)
				regenerating = damageMan.GetGroupDamageOverTime(group, EDamageType.HEALING) != 0 || damageMan.GetGroupDamageOverTime(group, EDamageType.REGENERATION) != 0;
		}

		if (defaultHZHealth == 1)
			damageIntensity = 0;
		else if (defaultHZHealth < hitZonePointUI.m_fLowDamageThreshold)
			damageIntensity = 4;
		else if (defaultHZHealth < hitZonePointUI.m_fMediumDamageThreshold)
			damageIntensity = 3;
		else if (defaultHZHealth < hitZonePointUI.m_fHighDamageThreshold)
			damageIntensity = 2;
		else
			damageIntensity = 1;
	}

	//------------------------------------------------------------------------------------------------
	protected void GetCasualtyName(inout string sName, IEntity targetCharacter)
	{
		string sFormat, sAlias, sSurname;
		int playerID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(targetCharacter);
		if (playerID > 0)
		{
			PlayerManager playerMgr = GetGame().GetPlayerManager();
			if (playerMgr)
				sName = SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(playerID);
		}
		else
		{
			SCR_CharacterIdentityComponent scrCharIdentity = SCR_CharacterIdentityComponent.Cast(targetCharacter.FindComponent(SCR_CharacterIdentityComponent));
			if (scrCharIdentity)
			{
				scrCharIdentity.GetFormattedFullName(sFormat, sName, sAlias, sSurname);
				sName = sName + " " + sSurname;
			}
			else
			{
				CharacterIdentityComponent charIdentity = CharacterIdentityComponent.Cast(targetCharacter.FindComponent(CharacterIdentityComponent));
				if (charIdentity && charIdentity.GetIdentity())
					sName = charIdentity.GetIdentity().GetName();
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void DisplayOnSuspended()
	{
		DisableWidget();
	}

	//------------------------------------------------------------------------------------------------
	void SetTarget(IEntity target)
	{
		m_Target = target;
	}

	//------------------------------------------------------------------------------------------------
	bool IsActive()
	{
		return m_Target && m_wCasualtyInspectWidget && m_wCasualtyInspectWidget.GetOpacity() != 0;
	}

	//------------------------------------------------------------------------------------------------
	protected void DisableWidget()
	{
		if (m_wCasualtyInspectWidget)
			m_wCasualtyInspectWidget.SetVisible(false);

		m_Target = null;
		SetEnabled(false);
		m_bShouldBeVisible = false;
		m_fTimeTillClose = MAX_SHOW_DURATION;
	}

	//------------------------------------------------------------------------------------------------
	protected void EnableWidget()
	{
		if (m_wCasualtyInspectWidget)
			m_wCasualtyInspectWidget.SetVisible(true);

		SetEnabled(true);
		m_bShouldBeVisible = true;
	}

	//------------------------------------------------------------------------------------------------
	override void DisplayOnResumed()
	{
		if (!m_bShouldBeVisible && m_wCasualtyInspectWidget)
			m_wCasualtyInspectWidget.SetVisible(false);
	}
}
