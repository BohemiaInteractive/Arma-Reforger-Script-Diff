[BaseContainerProps(), BaseContainerCustomTitleField("m_sDescription")]
class SCR_LenseColor
{
	[Attribute(uiwidget: UIWidgets.EditBox, desc: "Lense type", category: "Flashlight")]
	string m_sDescription;

	[Attribute("1 1 1", UIWidgets.ColorPicker, desc: "Lense color", category: "Flashlight")]
	vector m_vLenseColor;

	[Attribute("5.0", UIWidgets.Slider, desc: "Light Value", "-8.0 20 1", category: "Flashlight")]
	float m_fLightValue;
}

[EntityEditorProps(category: "GameScripted/Gadgets", description: "Flashlight", color: "0 0 255 255")]
class SCR_FlashlightComponentClass : SCR_GadgetComponentClass
{
}

class SCR_FlashlightComponent : SCR_GadgetComponent
{
	[Attribute("50", UIWidgets.EditBox, desc: "Intensity of the emmissive texture", params: "0 1000", category: "Flashlight")]
	protected float m_fEmissiveIntensity;

	[Attribute(uiwidget: UIWidgets.Object, desc: "Array of lense colors", category: "Flashlight")]
	protected ref array<ref SCR_LenseColor> m_aLenseArray;

	[Attribute("0.5", UIWidgets.EditBox, desc: "In meters, light ignores objects up to set distance to avoid unwanted shadows", params: "0.1 5", category: "Light near plane")]
	protected float m_fLightNearPlaneHand;

	[Attribute("1.2", UIWidgets.EditBox, desc: "In meters, light ignores objects up to set distance to avoid unwanted shadows", params: "0.1 5", category: "Light near plane")]
	protected float m_fLightNearPlaneStrapped;

	[Attribute("0.1", UIWidgets.EditBox, desc: "In meters, light ignores objects up to set distance to avoid unwanted shadows", params: "0.1 5", category: "Light near plane")]
	protected float m_fLightNearPlaneVehicle;

	[Attribute("0 0 -0.03", UIWidgets.EditBox, desc: "When strapped light entity is being aligned with weapon to adjust light angle, this entity is offset to avoid it visibly floating", category: "Flashlight")]
	protected vector m_vFlashlightAdjustOffset;

	protected bool m_bLastLightState;	// remember the last light state for cases where you put your flashlight into storage but there is no slot available -> true = active
	protected bool m_bIsSlottedVehicle;	// char owner is in vehicle while flashlight is slotted
	protected bool m_bIsOccluded;
	protected int m_iCurrentLenseColor;
	protected vector m_vAnglesCurrent;	// current local angles
	protected vector m_vAnglesTarget;	// target local angles used to align flashlight with aiming angles while strapped
	protected vector m_ItemMat[4];		// owner transformation matrix
	protected LightEntity m_Light;
	protected ParametricMaterialInstanceComponent m_EmissiveMaterial;
	protected SCR_CompartmentAccessComponent m_CompartmentComp;
	protected SCR_CharacterControllerComponent m_CharController;

	//------------------------------------------------------------------------------------------------
	override void OnToggleActive(bool state)
	{
		m_bActivated = state;

		if (m_bActivated)
			SCR_SoundManagerModule.CreateAndPlayAudioSource(GetOwner(), SCR_SoundEvent.SOUND_FLASHLIGHT_ON);
		else
			SCR_SoundManagerModule.CreateAndPlayAudioSource(GetOwner(), SCR_SoundEvent.SOUND_FLASHLIGHT_OFF);

		UpdateLightState();
	}

	//------------------------------------------------------------------------------------------------
	//! Update state of light ON/OFF
	protected void UpdateLightState()
	{
		if (m_bActivated)
			EnableLight();
		else
			DisableLight();
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetNumberOfAvailableLenses()
	{
		if (!m_aLenseArray)
			return 0;

		return m_aLenseArray.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! Cycle through all available lenses
	//! \param[in] direction decides whether it will switch to next (true) / previous (false) lens color
	void CycleThroughLenses(bool direction)
	{
		int lastLensId = GetNumberOfAvailableLenses() - 1;
		if (lastLensId < 1)
			return;

		if (direction)
			m_iCurrentLenseColor += 1;
		else
			m_iCurrentLenseColor -= 1;

		if (m_iCurrentLenseColor < 0)
			SwitchLenses(lastLensId);
		else if (m_iCurrentLenseColor > lastLensId)
			SwitchLenses(0);
		else
			SwitchLenses(m_iCurrentLenseColor);
	}

	//------------------------------------------------------------------------------------------------
	//! Switch flashlight lenses
	//! \param[in] filter
	protected void SwitchLenses(int filter)
	{
		m_iCurrentLenseColor = filter;

		Color lightColor = Color.FromVector(m_aLenseArray[m_iCurrentLenseColor].m_vLenseColor);
		m_Light.SetColor(lightColor, m_aLenseArray[m_iCurrentLenseColor].m_fLightValue);
		if (m_EmissiveMaterial)
			m_EmissiveMaterial.SetEmissiveColor(lightColor.PackToInt());
	}

	//------------------------------------------------------------------------------------------------
	//! Spawn light entity
	protected void EnableLight()
	{
		if (m_Light)
			m_Light.SetEnabled(true);

		if (m_EmissiveMaterial)
			m_EmissiveMaterial.SetEmissiveMultiplier(m_fEmissiveIntensity);

		SwitchLenses(m_iCurrentLenseColor);

		if (m_iMode == EGadgetMode.IN_SLOT)
			SetShadowNearPlane(true);
		else if (m_iMode == EGadgetMode.IN_HAND)
			SetShadowNearPlane(false);
	}

	//------------------------------------------------------------------------------------------------
	//! Remove light
	protected void DisableLight()
	{
		if (m_Light)
			m_Light.SetEnabled(false);

		if (m_EmissiveMaterial)
			m_EmissiveMaterial.SetEmissiveMultiplier(0);

		DeactivateGadgetUpdate();
	}

	//------------------------------------------------------------------------------------------------
	//! Set near plane of light shadow to avoid blocking the light with arm/weapon
	//! \param[in] state determines mode: false - in hand / true - strapped
	//! \param[in] isLeavingVehicle is signal for removing vehicle near plane mode
	protected void SetShadowNearPlane(bool state, bool isLeavingVehicle = false)
	{
		float nearPlane;
		m_bIsSlottedVehicle = false;

		if (state)	// strapped mode
		{
			nearPlane = m_fLightNearPlaneStrapped;
			if (m_CompartmentComp && m_CompartmentComp.IsInCompartment() && !isLeavingVehicle)
			{
				nearPlane = m_fLightNearPlaneVehicle;
				m_bIsSlottedVehicle = true;
				DeactivateGadgetUpdate();
			}
			else if (m_bActivated)
			{
				ActivateGadgetUpdate();
			}
		}
		else		// in hand mode
		{
			nearPlane = m_fLightNearPlaneHand;
		}

		if (m_Light)
			m_Light.SetNearPlane(nearPlane);
	}

	//------------------------------------------------------------------------------------------------
	protected void AdjustTransform(vector offset = vector.Zero)
	{
		InventoryItemComponent inventoryItemComp = InventoryItemComponent.Cast(GetOwner().FindComponent(InventoryItemComponent));
		if (inventoryItemComp)
		{
			InventoryStorageSlot slot = inventoryItemComp.GetParentSlot();
			ItemAnimationAttributes animAttr = ItemAnimationAttributes.Cast(inventoryItemComp.FindAttribute(ItemAnimationAttributes));
			if (slot && animAttr)
			{
				vector matLS[4];
				animAttr.GetAdditiveTransformLS(matLS);
				matLS[3] = matLS[3] + m_vEquipmentSlotOffset + offset;
				slot.SetAdditiveTransformLS(matLS);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! SCR_CompartmentAccessComponent event
	protected void OnOwnerLifeStateChanged(ECharacterLifeState previousLifeState, ECharacterLifeState newLifeState)
	{
		if (newLifeState == ECharacterLifeState.ALIVE)
			ActivateGadgetUpdate();
		else
			DeactivateGadgetUpdate();
	}

	//------------------------------------------------------------------------------------------------
	//! SCR_CompartmentAccessComponent event
	protected void OnCompartmentEntered(IEntity targetEntity, BaseCompartmentManagerComponent manager, int mgrID, int slotID, bool move)
	{
		if (!m_bActivated)
			return;

		BaseCompartmentSlot compSlot = manager.FindCompartment(slotID, mgrID);
		IEntity occupant = compSlot.GetOccupant();
		if (occupant == m_CharacterOwner)
		{
			SetShadowNearPlane(true);

			if (PilotCompartmentSlot.Cast(compSlot))
				ToggleActive(false, SCR_EUseContext.FROM_ACTION);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! SCR_CompartmentAccessComponent event
	protected void OnCompartmentLeft(IEntity targetEntity, BaseCompartmentManagerComponent manager, int mgrID, int slotID, bool move)
	{
		if (!m_bActivated)
			return;

		BaseCompartmentSlot compSlot = manager.FindCompartment(slotID, mgrID);
		IEntity occupant = compSlot.GetOccupant();
		if (occupant == m_CharacterOwner)
			SetShadowNearPlane(true, true);
	}

	//------------------------------------------------------------------------------------------------
	override void OnSlotOccludedStateChanged(bool occluded)
	{
		m_bIsOccluded = occluded;
		if (m_bActivated && !m_bIsOccluded)
			EnableLight();
		else
			DisableLight();
	}

	//------------------------------------------------------------------------------------------------
	override protected void ModeSwitch(EGadgetMode mode, IEntity charOwner)
	{
		super.ModeSwitch(mode, charOwner);

		InventoryItemComponent inventoryItemComp = InventoryItemComponent.Cast(GetOwner().FindComponent(InventoryItemComponent));
		if (!inventoryItemComp)
			return;

		if (!charOwner)
		{
			if (m_CharController)
				m_CharController.m_OnLifeStateChanged.Remove(OnOwnerLifeStateChanged);

			m_CharController = null;
		}
		else
		{
			if (m_CharacterOwner)
				m_CharController = SCR_CharacterControllerComponent.Cast(m_CharacterOwner.GetCharacterController());

			if (m_CharController)
				m_CharController.m_OnLifeStateChanged.Insert(OnOwnerLifeStateChanged);
		}

		inventoryItemComp.SetTraceable(mode != EGadgetMode.IN_SLOT);

		if (m_bActivated && (mode == EGadgetMode.IN_STORAGE || mode == EGadgetMode.ON_GROUND))
		{
			m_bLastLightState = m_bActivated;
			OnToggleActive(false);//direct call since ToggleActive requries m_CharacterOwner to not be null as he then asks for propagation of the change
		}
		else if (mode == EGadgetMode.IN_SLOT || mode == EGadgetMode.IN_HAND)
		{
			if (m_bLastLightState && (!m_bActivated || m_Light && !m_Light.IsEnabled())) // LightEntity is disabled in EOnDeactivate aka when its dropped on the ground
				ToggleActive(m_bLastLightState, SCR_EUseContext.FROM_ACTION);

			SetShadowNearPlane(mode != EGadgetMode.IN_HAND);
		}

		if (mode == EGadgetMode.IN_SLOT)
		{
			SCR_EquipmentStorageSlot parentSlot = SCR_EquipmentStorageSlot.Cast(inventoryItemComp.GetParentSlot());
			if (parentSlot && parentSlot.IsOccluded())
			{
				m_bIsOccluded = true;
				DisableLight();
			}

			m_CompartmentComp = SCR_CompartmentAccessComponent.Cast(m_CharacterOwner.FindComponent(SCR_CompartmentAccessComponent));
			if (m_CompartmentComp)
			{
				m_CompartmentComp.GetOnCompartmentEntered().Insert(OnCompartmentEntered);
				m_CompartmentComp.GetOnCompartmentLeft().Insert(OnCompartmentLeft);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void ModeClear(EGadgetMode mode)
	{
		if (mode == EGadgetMode.IN_SLOT)
		{
			m_bIsOccluded = false;
			if (m_CompartmentComp)
			{
				m_CompartmentComp.GetOnCompartmentEntered().Remove(OnCompartmentEntered);
				m_CompartmentComp.GetOnCompartmentLeft().Remove(OnCompartmentLeft);
			}

			SetShadowNearPlane(false);
		}

		super.ModeClear(mode);
	}

	//------------------------------------------------------------------------------------------------
	override protected void ToggleActive(bool state, SCR_EUseContext context)
	{
		if (m_iMode == EGadgetMode.IN_STORAGE && !m_bActivated)	// trying to activate flashlight hidden in inventory
			return;

		// trying to activate flashlight while driving
		if (!m_bActivated && m_CompartmentComp && PilotCompartmentSlot.Cast(m_CompartmentComp.GetCompartment()))
			return;

		super.ToggleActive(state, context);
	}

	//------------------------------------------------------------------------------------------------
	override void ActivateAction()
	{
		if (m_CharController && m_CharController.GetLifeState() != ECharacterLifeState.ALIVE)	// if in chars inventory && char is dead/uncon
			return;

		ToggleActive(!m_bActivated, SCR_EUseContext.FROM_ACTION);
	}

	//------------------------------------------------------------------------------------------------
	override protected void ActivateGadgetUpdate()
	{
		super.ActivateGadgetUpdate();

		AdjustTransform(m_vFlashlightAdjustOffset);
	}

	//------------------------------------------------------------------------------------------------
	override protected void DeactivateGadgetUpdate()
	{
		super.DeactivateGadgetUpdate();

		if (m_iMode == EGadgetMode.IN_SLOT) // reset slot position of the gadget back to its default
			AdjustTransform();
	}

	//------------------------------------------------------------------------------------------------
	override EGadgetType GetType()
	{
		return EGadgetType.FLASHLIGHT;
	}

	//------------------------------------------------------------------------------------------------
	override bool IsVisibleEquipped()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool RplSave(ScriptBitWriter writer)
	{
		if (!super.RplSave(writer))
			return false;

		writer.WriteBool(m_bActivated);
		writer.WriteInt(m_iCurrentLenseColor);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		if (!super.RplLoad(reader))
			return false;

		reader.ReadBool(m_bActivated);
		reader.ReadInt(m_iCurrentLenseColor);

		UpdateLightState();

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnTicksOnRemoteProxy()
	{
		return true; // proxies will only tick on owners without this
	}

	//------------------------------------------------------------------------------------------------
	override void Update(float timeSlice)
	{
		if (!m_CharController)
		{
			DeactivateGadgetUpdate();
			return;
		}

		// adjust angle of the flashlight to provide usable angle within various poses
		m_Light.GetTransform(m_ItemMat);
		m_vAnglesCurrent = m_Light.GetLocalYawPitchRoll();
		m_vAnglesTarget = (Math.RAD2DEG * m_CharController.GetInputContext().GetAimingAngles() - Math3D.MatrixToAngles(m_ItemMat)) + m_vAnglesCurrent;	// diff between WS aiming and item angles, add local to the result
		m_vAnglesTarget[0] = SCR_Math.FixAngle(m_vAnglesTarget[0], 180);
		m_vAnglesTarget[1] = Math.Clamp(m_vAnglesTarget[1], 0, 30);	// only need too offset upwards, also avoid glitches with some stances
		m_vAnglesTarget[2] = 0;	// no roll

		m_vAnglesTarget[0] = Math.Lerp(m_vAnglesCurrent[0], m_vAnglesTarget[0], timeSlice* 4);
		m_vAnglesTarget[1] = Math.Lerp(m_vAnglesCurrent[1], m_vAnglesTarget[1], timeSlice* 4);
		m_Light.SetYawPitchRoll(m_vAnglesTarget); // sets local angles
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		m_Light = LightEntity.Cast(owner.GetChildren());
		m_EmissiveMaterial = ParametricMaterialInstanceComponent.Cast(owner.FindComponent(ParametricMaterialInstanceComponent));

		// Insert default entry if none are set
		if (m_aLenseArray.Count() == 0)
		{
			SCR_LenseColor defColor = new SCR_LenseColor();
			defColor.m_sDescription = "CONFIGURE ME";
			m_aLenseArray.Insert(defColor);
		}
	}
}
