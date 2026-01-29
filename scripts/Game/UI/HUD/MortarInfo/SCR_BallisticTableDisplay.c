class SCR_BallisticTableDisplay : SCR_InfoDisplayExtended
{
	[Attribute(desc: "Names of TextWidgets that show the current, next & previous range step.")]
	protected ref array<string> m_aRangeListWidgetNames;

	[Attribute("0", desc: "0 if first value that will be presented is meant to be the closest range")]
	protected int m_iBallisticDataOffset;

	[Attribute(desc: "Ballistic table (prototype)")]
	protected ref array<ref SCR_BaseBallisticConfig> m_aBallisticConfigs;

	[Attribute("TxtShellName")]
	protected string m_sShellNameTextWidgetName;


	[Attribute("AmmoTypeIcon")]
	protected string m_sShellTypeIconWidgetName;

	[Attribute("%1 - %2")]
	protected string m_sRangeTextFormat;

	[Attribute("0.5", UIWidgets.Slider, "Time in seconds after which the Display should fade in", "0 100 0.1")]
	protected float m_fFadeInDelay;

	[Attribute("CharacterInspectTurn", desc: "Name of the action that will be used to switch to next shell")]
	protected string m_sChangeShellTypeActionName;

	[Attribute("CharacterNextFireMode", desc: "Name of the action that will be used to switch number of charge rings")]
	protected string m_sChangeShellChargeActionName;

	protected TextWidget m_wShellNameTextWidget;
	protected ImageWidget m_wAmmoTypeIcon;
	protected ref array<TextWidget> m_aRangeListWidgets = {};

	protected TurretControllerComponent m_TurretController;

	protected bool m_bHasDifferentChargeRingConfigs;
	protected float m_fTopElevationThreshold = -1;
	protected float m_fBottomElevationThreshold = -1;
	protected int m_iSelectedBallisticConfig;
	protected SCR_AdditionalGameModeSettingsComponent m_AdditionalGameSettings;

	protected const string TEXT_EMPTY = "-";

	//------------------------------------------------------------------------------------------------
	SCR_BaseBallisticConfig GetSelectedBallisticConfig()
	{
		if (!m_aBallisticConfigs || !m_aBallisticConfigs.IsIndexValid(m_iSelectedBallisticConfig))
			return null;
		
		return m_aBallisticConfigs.Get(m_iSelectedBallisticConfig);
	}
	
	//------------------------------------------------------------------------------------------------
	int GetNumberOfBallisticConfigs()
	{
		return m_aBallisticConfigs.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! Returns true when this display has at least two configs for the same shell with different init speeds
	bool GetHasDifferentChargeConfigs()
	{
		return m_bHasDifferentChargeRingConfigs;
	}

	//------------------------------------------------------------------------------------------------
	//! Method used to change what ballistic data is shown
	void ChangeShellType(float direction = 0)
	{
		if (direction == 0)
		{
			UpdateConfig(false);
			return;
		}

		SCR_BaseBallisticConfig currentConfig = m_aBallisticConfigs[m_iSelectedBallisticConfig];;
		if (!currentConfig)
			return;

		int numberOfConfigs = m_aBallisticConfigs.Count() - 1;
		int newConfigId = m_iSelectedBallisticConfig + direction;
		SCR_BaseBallisticConfig replacementConfig;
		while (newConfigId != m_iSelectedBallisticConfig)
		{
			if (newConfigId > numberOfConfigs)
				newConfigId = 0;
			else if (newConfigId < 0)
				newConfigId = numberOfConfigs;

			replacementConfig = m_aBallisticConfigs[newConfigId];
			if (replacementConfig && replacementConfig.GetProjectilePrefab() != currentConfig.GetProjectilePrefab())
				break;

			newConfigId += direction;
		}

		if (newConfigId == m_iSelectedBallisticConfig)
			return;

		m_iSelectedBallisticConfig = newConfigId;
		UpdateConfig();
	}

	//------------------------------------------------------------------------------------------------
	//! Method used to change what charge ring config is shown for current shell type
	void ChangeShellCharge(float direction = 0)
	{
		if (direction == 0)
		{
			UpdateConfig(false);
			return;
		}

		SCR_BaseBallisticConfig currentConfig = m_aBallisticConfigs[m_iSelectedBallisticConfig];
		if (!currentConfig)
			return;

		int numberOfConfigs = m_aBallisticConfigs.Count() - 1;
		int newConfigId = m_iSelectedBallisticConfig + direction;
		SCR_BaseBallisticConfig replacementConfig;
		while (newConfigId != m_iSelectedBallisticConfig)
		{
			if (newConfigId > numberOfConfigs)
				newConfigId = 0;
			else if (newConfigId < 0)
				newConfigId = numberOfConfigs;

			replacementConfig = m_aBallisticConfigs[newConfigId];
			if (replacementConfig && replacementConfig.GetProjectilePrefab() == currentConfig.GetProjectilePrefab() && replacementConfig.GetProjectileInitSpeedCoef() != currentConfig.GetProjectileInitSpeedCoef())
				break;

			newConfigId += direction;
		}

		if (newConfigId == m_iSelectedBallisticConfig)
			return;

		m_iSelectedBallisticConfig = newConfigId;

		UpdateConfig(direction != 0);
	}

	//------------------------------------------------------------------------------------------------
	//! Method used to update currently displayed ballistic config
	protected void UpdateConfig(bool playSound = true)
	{
		m_fTopElevationThreshold = -1;
		m_fBottomElevationThreshold = -1;
		UpdateBallisticInfo(GetMuzzleDirection().VectorToAngles());
		if (m_aBallisticConfigs.IsEmpty())
			return;

		SCR_BaseBallisticConfig config = m_aBallisticConfigs[m_iSelectedBallisticConfig];
		if (!config)
			return;

		if (m_wShellNameTextWidget)
			m_wShellNameTextWidget.SetText(config.GetDisplayedText());

		if (m_wAmmoTypeIcon)
		{
			if (config.GetAmmoTypeImageSet().IsEmpty() || config.GetAmmoTypeQuadName().IsEmpty())
			{
				m_wAmmoTypeIcon.SetVisible(false);
			}
			else
			{
				m_wAmmoTypeIcon.SetVisible(true);
				m_wAmmoTypeIcon.LoadImageFromSet(0, config.GetAmmoTypeImageSet(), config.GetAmmoTypeQuadName());
				m_wAmmoTypeIcon.SetSize(config.GetAmmoTypeImageSize(), config.GetAmmoTypeImageSize());
			}
		}

		if (playSound)
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_FE_ITEM_CHANGE);
	}

	//------------------------------------------------------------------------------------------------
	//! Set azimuth and elevation info
	protected array<string> GetBallisticInfo(float elevation)
	{
		if (m_iSelectedBallisticConfig < 0 || m_iSelectedBallisticConfig >= m_aBallisticConfigs.Count())
			return null;

		SCR_BaseBallisticConfig config = m_aBallisticConfigs[m_iSelectedBallisticConfig];
		if (!config)
			return null;

		if (!config.IsGenerated())
		{
			if (!config.GenerateBallisticData())
			{
				m_aBallisticConfigs.Remove(m_iSelectedBallisticConfig);
				return GetBallisticInfo(elevation);
			}
		}

		SCR_BallisticData bd = SCR_BallisticData.s_aBallistics[config.GetBallisticDataId()];
		if (!bd)
			return null;

		float angle = SCR_Math.ConvertFromRadians(elevation * Math.DEG2RAD, config.GetUnitType());

		int id;
		array<float> ballisticValues = bd.GetBallisticValuesForClosestAngle(angle, id, epsilonStep: 2);
		if (id < 0 || !ballisticValues)
			return null;

		angle = SCR_Math.ConvertToRadians(ballisticValues[1], config.GetUnitType()) * Math.RAD2DEG;
		bd.GetValues(id - 1, ballisticValues);
		if (ballisticValues)
		{
			if (config.IsDirectFire())
				m_fBottomElevationThreshold = SCR_Math.ConvertToRadians(ballisticValues[1], config.GetUnitType()) * Math.RAD2DEG;
			else
				m_fTopElevationThreshold = SCR_Math.ConvertToRadians(ballisticValues[1], config.GetUnitType()) * Math.RAD2DEG;
		}

		bd.GetValues(id + 1, ballisticValues);
		if (ballisticValues)
		{
			if (config.IsDirectFire())
				m_fTopElevationThreshold = SCR_Math.ConvertToRadians(ballisticValues[1], config.GetUnitType()) * Math.RAD2DEG;
			else
				m_fBottomElevationThreshold = SCR_Math.ConvertToRadians(ballisticValues[1], config.GetUnitType()) * Math.RAD2DEG;

			if (float.AlmostEqual(m_fBottomElevationThreshold, m_fTopElevationThreshold))
				m_fBottomElevationThreshold = 0;
			else
				m_fBottomElevationThreshold += (angle - m_fBottomElevationThreshold) * 0.49;

			m_fTopElevationThreshold += (angle - m_fTopElevationThreshold) * 0.49;
		}

		array<string> textArray = {};
		string rangeText, elevationText;
		for (int i, count = m_aRangeListWidgets.Count(); i < count; i++)
		{
			if (bd.GetValues(id + i + m_iBallisticDataOffset, ballisticValues) < 1)
			{
				textArray.Insert(TEXT_EMPTY);
				continue;
			}

			if (!ballisticValues)
			{
				textArray.Insert(TEXT_EMPTY);
				continue;
			}

			rangeText = WidgetManager.Translate(UIConstants.VALUE_UNIT_METERS, ballisticValues[0].ToString(4, 0));
			switch (config.GetUnitType())
			{
				case SCR_EOpticsAngleUnits.DEGREES:
					elevationText = WidgetManager.Translate(UIConstants.VALUE_UNIT_DEGREES, ballisticValues[1].ToString(4, 0));
					break;

				default:
					elevationText = WidgetManager.Translate(UIConstants.VALUE_UNIT_MILS, ballisticValues[1].ToString(4, 0));
					break;
			}

			textArray.Insert(string.Format(m_sRangeTextFormat, rangeText, elevationText));
		}

		return textArray;
	}
	//------------------------------------------------------------------------------------------------
	protected void UpdateBallisticInfo(vector aimRotation, bool forceRefresh = false)
	{
		if (m_aRangeListWidgets.IsEmpty())
			return;

		if (!forceRefresh && m_fBottomElevationThreshold > -1 && m_fTopElevationThreshold > -1 && aimRotation[1] < m_fTopElevationThreshold && aimRotation[1] > m_fBottomElevationThreshold)
			return;

		array<string> balisticInfo = GetBallisticInfo(aimRotation[1]);
		if (!balisticInfo || balisticInfo.IsEmpty())
		{
			super.Show(false, 0);
			return;
		}

		int balisticInfoCount = balisticInfo.Count() - 1;
		foreach (int i, TextWidget widget : m_aRangeListWidgets)
		{
			if (!widget)
				break;

			if (i > balisticInfoCount)
			{
				widget.SetVisible(false);
				continue;
			}

			widget.SetText(balisticInfo[i]);
			widget.SetVisible(true);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Get muzzle rotation vector
	protected vector GetMuzzleDirection()
	{
		BaseWeaponManagerComponent weaponManager = m_TurretController.GetWeaponManager();
		if (!weaponManager)
			return vector.Zero;

		vector transform[4];
		weaponManager.GetCurrentMuzzleTransform(transform);
		return transform[2];
	}

	//------------------------------------------------------------------------------------------------
	protected void SetupWidgets()
	{
		TextWidget widget;

		foreach (string widgetName : m_aRangeListWidgetNames)
		{
			widget = TextWidget.Cast(m_wRoot.FindAnyWidget(widgetName));

			if (widget)
				m_aRangeListWidgets.Insert(widget);
		}

		m_wShellNameTextWidget = TextWidget.Cast(m_wRoot.FindAnyWidget(m_sShellNameTextWidgetName));
		m_wAmmoTypeIcon = ImageWidget.Cast(m_wRoot.FindAnyWidget(m_sShellTypeIconWidgetName));
	}

	//------------------------------------------------------------------------------------------------
	//! Calls the SCR_InfoDisplayExtended.Show method after some delay
	//! \param[in] bool, True if it can be shown, false otherwise
	//! \param[in] float, Fade speed
	protected void ShowDelay(bool show, float speed = UIConstants.FADE_RATE_INSTANT)
	{
		super.Show(show, speed);
	}

	//------------------------------------------------------------------------------------------------
	override void Show(bool show, float speed = UIConstants.FADE_RATE_INSTANT, EAnimationCurve curve = EAnimationCurve.LINEAR)
	{
		if (m_AdditionalGameSettings && !m_AdditionalGameSettings.GetProjectileBallisticInfoVisibility())
		{
			RemoveActionListeners();
			super.Show(false);
			return;
		}

		speed = UIConstants.FADE_RATE_DEFAULT;

		// If it can be shown, do so after a delay to prevent it from fading in to early
		if (m_bCanShow)
		{
			AddActionListeners();
			GetGame().GetCallqueue().CallLater(ShowDelay, m_fFadeInDelay * 1000, false, show, speed);
		}
		else
		{
			RemoveActionListeners();
			super.Show(show, speed);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void DisplayUpdate(IEntity owner, float timeSlice)
	{
		UpdateBallisticInfo(GetMuzzleDirection().VectorToAngles());
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnSettingsChanged()
	{
		if (m_AdditionalGameSettings)
			Show(m_AdditionalGameSettings.GetProjectileBallisticInfoVisibility());

		super.OnSettingsChanged();
	}

	//------------------------------------------------------------------------------------------------
	override void DisplayStopDraw(IEntity owner)
	{
		m_aRangeListWidgets.Clear();

		if (m_AdditionalGameSettings)
			m_AdditionalGameSettings.GetOnChangeAdditionalSettingsInvoker().Remove(OnSettingsChanged);

		if (m_aBallisticConfigs.IsEmpty())
			return;

		RemoveActionListeners();
		SCR_MenuHelper.GetOnMenuClose().Remove(OnSettingsMenuClosed);
	}

	//------------------------------------------------------------------------------------------------
	protected void AddActionListeners()
	{
		InputManager inputMgr = GetGame().GetInputManager();
		if (!inputMgr)
			return;

		inputMgr.AddActionListener(m_sChangeShellTypeActionName, EActionTrigger.DOWN, ChangeShellType);

		if (m_bHasDifferentChargeRingConfigs)
			inputMgr.AddActionListener(m_sChangeShellChargeActionName, EActionTrigger.DOWN, ChangeShellCharge);
	}

	//------------------------------------------------------------------------------------------------
	protected void RemoveActionListeners()
	{
		InputManager inputMgr = GetGame().GetInputManager();
		if (!inputMgr)
			return;

		inputMgr.RemoveActionListener(m_sChangeShellTypeActionName, EActionTrigger.DOWN, ChangeShellType);

		if (m_bHasDifferentChargeRingConfigs)
			inputMgr.RemoveActionListener(m_sChangeShellChargeActionName, EActionTrigger.DOWN, ChangeShellCharge);
	}

	//------------------------------------------------------------------------------------------------
	override void DisplayStartDraw(IEntity owner)
	{
		m_TurretController = TurretControllerComponent.Cast(owner.FindComponent(TurretControllerComponent));

		if (!m_aBallisticConfigs || m_aBallisticConfigs.IsEmpty())
			m_iSelectedBallisticConfig = -1;

		if (!m_AdditionalGameSettings)
			m_AdditionalGameSettings = SCR_AdditionalGameModeSettingsComponent.GetInstance();

		m_AdditionalGameSettings.GetOnChangeAdditionalSettingsInvoker().Insert(OnSettingsChanged);

		SetupWidgets();
		ChangeShellType();
		if (m_aBallisticConfigs.IsEmpty())
			return;

		foreach (SCR_BaseBallisticConfig config : m_aBallisticConfigs)
		{
			foreach (SCR_BaseBallisticConfig secondConfig : m_aBallisticConfigs)
			{
				if (config.GetProjectileInitSpeedCoef() == secondConfig.GetProjectileInitSpeedCoef())
					continue;

				if (config.GetProjectilePrefab() != secondConfig.GetProjectilePrefab())
					continue;

				m_bHasDifferentChargeRingConfigs = true;
				break;
			}

			if (m_bHasDifferentChargeRingConfigs)
				break;
		}

		SCR_MenuHelper.GetOnMenuClose().Insert(OnSettingsMenuClosed);
	}

	//------------------------------------------------------------------------------------------------
	//! Callback method used to force refresh of the data after player leaves settings menu (f.e. after he changed the language)
	//! \param[in] menu which was just closed
	protected void OnSettingsMenuClosed(ChimeraMenuBase menu)
	{
		if (SCR_SettingsSuperMenu.Cast(menu))
			UpdateBallisticInfo(GetMuzzleDirection().VectorToAngles(), true);
	}
}
