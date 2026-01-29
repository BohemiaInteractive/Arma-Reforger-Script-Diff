class SCR_InventoryAmmoTypeIndicator : ScriptedWidgetComponent
{
	[Attribute("AmmoTypeContainer")]
	protected string m_sContainer;
	protected Widget m_wContainer;

	protected static const string WIDGET_NAME_PREFIX = "ammotype-";
	
	//------------------------------------------------------------------------------------------------	
	override void HandlerAttached(Widget w)
	{
		m_wContainer = w.FindAnyWidget(m_sContainer);
	}
	
	//------------------------------------------------------------------------------------------------	
	bool SetAmmoType(InventoryItemComponent item)
	{
		Widget child = m_wContainer.GetChildren();
		while (child)
		{
			child.SetVisible(false);
			child = child.GetSibling();
		}
	
		MagazineComponent magComp = MagazineComponent.Cast(item.GetOwner().FindComponent(MagazineComponent));
		if (magComp)
		{
			MagazineUIInfo ammoInfo = MagazineUIInfo.Cast(magComp.GetUIInfo());
			if (!ammoInfo)
				return false;
	
			SetAmmoTypeIcon(ammoInfo.GetAmmoTypeFlags());	
			return true;
		}

		WeaponComponent weaponComp = WeaponComponent.Cast(item.GetOwner().FindComponent(WeaponComponent));
		if (weaponComp)
		{
			GrenadeUIInfo ammoInfo = GrenadeUIInfo.Cast(weaponComp.GetUIInfo());
			if (!ammoInfo)
				return false;
	
			SetAmmoTypeIcon(ammoInfo.GetAmmoTypeFlags());	
			return true;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------	
	protected void SetAmmoTypeIcon(EAmmoType flags)
	{
		if (!m_wContainer)
			return;

		Widget ammoTypeWidget;
		array<int> enumValues = {};
		SCR_Enum.GetEnumValues(EAmmoType, enumValues);
		foreach (int flag : enumValues)
		{
			if (!(flag & flags))
				continue;

			ammoTypeWidget = m_wContainer.FindAnyWidget(WIDGET_NAME_PREFIX+SCR_Enum.GetEnumName(EAmmoType, flag));
			if (ammoTypeWidget)
				ammoTypeWidget.SetVisible(true);
		}
	}
}