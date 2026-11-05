class SCR_InventoryMagazineSlotUI : SCR_InventorySlotUI
{
	protected SCR_MagazineComponent m_MagazineComponent;
	protected OverlayWidget m_wIncompatibilityOverlay;
	protected OverlayWidget m_wCompatibilityOverlay;
	protected OverlayWidget m_wMagazineOverlay;

	protected const string PARENT_WIDGET_NAME = "Overlay0";
	protected const string COMPATIBLITY_WIDGET = "Compatible";
	protected const string SELECTION_WIDGET_NAME = "SelectedIcon";
	protected const string INCOMPATIBLITY_WIDGET = "Incompatible";
	protected const string MAGAZINE_OVERLAY_LAYOUT = "{7B9876270D453A58}UI/layouts/Menus/Inventory/InventoryMagazineSlot.layout";

	//------------------------------------------------------------------------------------------------
	SCR_MagazineComponent GetMagazineComponent()
	{
		return m_MagazineComponent;
	}

	//------------------------------------------------------------------------------------------------
	override void UpdateReferencedComponent(InventoryItemComponent pComponent, SCR_ItemAttributeCollection attributes = null)
	{
		super.UpdateReferencedComponent(pComponent, attributes);

		if (!m_widget)
			return;

		IEntity mag = pComponent.GetOwner();
		m_MagazineComponent = SCR_MagazineComponent.Cast(mag.FindComponent(SCR_MagazineComponent));
		if (m_wMagazineOverlay)
			return;

		Widget parent = m_widget.FindAnyWidget(PARENT_WIDGET_NAME);
		if (!parent)
			return;

		m_wMagazineOverlay = OverlayWidget.Cast(GetGame().GetWorkspace().CreateWidgets(MAGAZINE_OVERLAY_LAYOUT, parent));
		m_wIncompatibilityOverlay = OverlayWidget.Cast(m_wMagazineOverlay.FindAnyWidget(INCOMPATIBLITY_WIDGET));
		m_wCompatibilityOverlay = OverlayWidget.Cast(m_wMagazineOverlay.FindAnyWidget(COMPATIBLITY_WIDGET));
	}

	//------------------------------------------------------------------------------------------------
	override void OnChangeLockState()
	{
		super.OnChangeLockState();

		if (m_pItem && !m_pItem.IsLocked() && m_pStorageUI)
			m_pStorageUI.Refresh();
	}

	//------------------------------------------------------------------------------------------------
	override bool OnDrop(SCR_InventorySlotUI slot)
	{
		if (!slot || IsSlotBlocked())
			return false;

		SCR_MagazineComponent sourceMagazine;
		if (slot == this)
		{
			if (m_iStackNumber <= 1)
				return false;

			int ammoCount = m_MagazineComponent.GetAmmoCount();
			if (ammoCount <= 0 || ammoCount >= m_MagazineComponent.GetMaxAmmoCount())
				return false;

			array<IEntity> items = {};
			m_pStorageUI.GetAllItems(items, m_pStorageUI.GetStorage());
			IEntity thisMag = m_MagazineComponent.GetOwner();
			EntityPrefabData thisMagPrefabData = thisMag.GetPrefabData();
			foreach (IEntity item : items)
			{
				if (item == thisMag)
					continue;

				if (item.GetPrefabData() != thisMagPrefabData)
					continue;

				sourceMagazine = SCR_MagazineComponent.Cast(item.FindComponent(SCR_MagazineComponent));
				if (sourceMagazine.GetAmmoCount() == ammoCount) // since this is the same prefab, thus it must have it if this magazine has it, so no null check
					break;

				sourceMagazine = null; // clear in case that we wont find the right one, to ensure that we wont start repacking with random mag
			}

			if (!sourceMagazine)
				return false;
		}
		else
		{
			SCR_InventoryMagazineSlotUI sourceSlot = SCR_InventoryMagazineSlotUI.Cast(slot);
			if (!sourceSlot)
				return false;

			sourceMagazine = sourceSlot.GetMagazineComponent();
			if (!sourceMagazine)
				return false;

			if (!m_MagazineComponent.CanBeRepacked(sourceMagazine))
				return false;
		}

		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		character.AskStartRepackingMagazines(sourceMagazine, m_MagazineComponent);

		SCR_InventoryMenuUI menu = SCR_InventoryMenuUI.Cast(GetGame().GetMenuManager().FindMenuByPreset(ChimeraMenuPreset.Inventory20Menu));
		if (menu && menu.IsUsingGamepad())
		{
			menu.Action_DeselectItem();
			menu.FocusOnSlotInStorage(m_pStorageUI, m_pStorageUI.GetSlotId(this));
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void SetSlotVisible(bool bVisible)
	{
		super.SetSlotVisible(bVisible);
		if (!bVisible)
			return;

		if (!m_wSelectedEffect)
			return;

		Widget icon = m_wSelectedEffect.FindAnyWidget(SELECTION_WIDGET_NAME);
		if (icon)
			icon.SetVisible(false); // hide it as it will look odd when we will overlay other icon on top of it

		if (!m_MagazineComponent || !m_wItemLockThrobber)
			return;

		InventoryItemComponent iic = InventoryItemComponent.Cast(m_MagazineComponent.GetOwner().FindComponent(InventoryItemComponent));
		if (iic)
			m_wItemLockThrobber.SetVisible(iic.IsLocked());
	}

	//------------------------------------------------------------------------------------------------
	//! Updates ammo count display or whole container ui if slot is used to display a stack
	//! \param[out] refreshedWholeStorageUi true means that whole storage ui was refreshed, and thus there is no need to process any further slots in this sotrage
	void UpdateAmmoCount(out bool refreshedWholeStorageUi)
	{
		if (m_iStackNumber > 1)
		{
			refreshedWholeStorageUi = true;
			m_pStorageUI.Refresh(); // recreate all slots to move magazine which ammo count changed to a different slot in order to not show that whole stack is changing ammo count
			return;
		}

		if (!m_MagazineComponent)
			return;

		m_aAmmoCountActual = m_MagazineComponent.GetAmmoCount();
		m_aAmmoCountMax = m_MagazineComponent.GetMaxAmmoCount();			
		
		if (m_ProgressBar)
		{
			if (m_aAmmoCountMax == 1)
			{
				m_wProgressBar.SetVisible(false);
				return;
			}
			
			m_ProgressBar.FlipColors();
			m_ProgressBar.SetProgressRange(0, m_aAmmoCountMax);
			m_ProgressBar.SetCurrentProgress(m_aAmmoCountActual);		
		}
	}

	//------------------------------------------------------------------------------------------------
	SCR_ECompatibilityState GetCompatibilityState()
	{
		if (m_wCompatibilityOverlay && m_wCompatibilityOverlay.IsVisible())
			return SCR_ECompatibilityState.COMPATIBLE;

		if (m_wIncompatibilityOverlay && m_wIncompatibilityOverlay.IsVisible())
			return SCR_ECompatibilityState.INCOMPATIBLE;

		return SCR_ECompatibilityState.NOT_SET;
	}

	//------------------------------------------------------------------------------------------------
	void SetSlotCompatibilityState(bool compatible)
	{
		if (compatible)
			SetSlotCompatibilityState(SCR_ECompatibilityState.COMPATIBLE);
		else
			SetSlotCompatibilityState(SCR_ECompatibilityState.INCOMPATIBLE);
	}

	//------------------------------------------------------------------------------------------------
	void SetSlotCompatibilityState(SCR_ECompatibilityState state = SCR_ECompatibilityState.NOT_SET)
	{
		if (!m_widget)
			return;

		if (!m_wIncompatibilityOverlay || !m_wCompatibilityOverlay)
		{
			string missingPart = INCOMPATIBLITY_WIDGET;
			if (!m_wCompatibilityOverlay)
				missingPart = COMPATIBLITY_WIDGET;

			Debug.Error("SCR_InventoryMagazineSlotUI was trying to set its compatbility state, but noticed that it is missing " + missingPart + " widget!");
			return;
		}

		switch (state)
		{
			case SCR_ECompatibilityState.COMPATIBLE:
				m_wCompatibilityOverlay.SetVisible(true);

				if (m_pStorageUI && m_pStorageUI.IsWarningOverlayVisible())
					m_pStorageUI.SetStorageAsHighlighted(true, EInvInsertFailReason.OK);

				m_wIncompatibilityOverlay.SetVisible(false);
				return;

			case SCR_ECompatibilityState.INCOMPATIBLE:
				m_wCompatibilityOverlay.SetVisible(false);
				m_wIncompatibilityOverlay.SetVisible(true);
				m_bBlocked = true;
				return;

			default:
				m_wCompatibilityOverlay.SetVisible(false);
				m_wIncompatibilityOverlay.SetVisible(false);
				m_bBlocked = false;
		}
	}
}
