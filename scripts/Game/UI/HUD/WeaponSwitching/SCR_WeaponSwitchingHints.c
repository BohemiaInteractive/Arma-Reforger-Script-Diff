//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// TODO: Remove this dead code
class SCR_WeaponSwitchingHints: ScriptedWidgetComponent
{
	Widget m_wChangeAmmo;
	Widget m_wChangeMuzzle;
	Widget m_wSafety;
	Widget m_wRaiseWeapon;
	EventHandlerManagerComponent m_EventHandlerManager;

	BaseWeaponComponent m_CurrentWeapon;

	override void HandlerAttached(Widget w)
	{
		GetGame().GetInputManager().AddActionListener("CharacterChangeAmmo", EActionTrigger.DOWN, Action_ChangeAmmo);
		GetGame().GetInputManager().AddActionListener("CharacterChangeMuzzle", EActionTrigger.DOWN, Action_ChangeMuzzle);

		m_wChangeAmmo = w.FindAnyWidget("ChangeAmmo");
		m_wChangeMuzzle = w.FindAnyWidget("ChangeMuzzle");
		m_wSafety = w.FindAnyWidget("Safety");
		m_wRaiseWeapon = w.FindAnyWidget("RaiseWeapon");

		ChimeraCharacter character = ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		if (!character)
			return;

		m_EventHandlerManager = EventHandlerManagerComponent.Cast(character.FindComponent(EventHandlerManagerComponent));
		if (m_EventHandlerManager)
			m_EventHandlerManager.RegisterScriptHandler("OnWeaponChanged", this, OnWeaponChanged, true);

		BaseWeaponManagerComponent weaponManager = BaseWeaponManagerComponent.Cast(character.FindComponent(BaseWeaponManagerComponent));
		if (!weaponManager)
			return;

		m_CurrentWeapon = weaponManager.GetCurrent();
		OnWeaponChanged(m_CurrentWeapon);
	}

	override void HandlerDeattached(Widget w)
	{
		if (m_EventHandlerManager)
			m_EventHandlerManager.RemoveScriptHandler("OnWeaponChanged", this, OnWeaponChanged, true);
	}


	void OnWeaponChanged(BaseWeaponComponent weapon)
	{
		m_CurrentWeapon = weapon;

		// If the current item is not a weapon, hide all widgets
		if (!weapon)
		{
			m_wRaiseWeapon.SetVisible(false);
			m_wChangeMuzzle.SetVisible(false);
			m_wChangeAmmo.SetVisible(false);
			m_wSafety.SetVisible(false);
			return;
		}

		// Ammo
		// TODO: How do I get the current ammo types???
		
		
		m_wRaiseWeapon.SetVisible(true);

		// Muzzles
		array<BaseMuzzleComponent> a;
		int count = weapon.GetMuzzlesList(a);
		m_wChangeMuzzle.SetVisible(count > 1);

		// Safety
		bool hasSafetyMode = false;
		BaseMuzzleComponent muzzle = weapon.GetCurrentMuzzle();
		if (muzzle)
		{
			array<BaseFireMode> fireModes;
			muzzle.GetFireModesList(fireModes);
			foreach (BaseFireMode mode : fireModes)
			{
				if (mode.GetFiremodeType() != EWeaponFiremodeType.Safety)
					continue;

				hasSafetyMode = true;
				break;
			}
		}
		m_wSafety.SetVisible(hasSafetyMode);
	}

	void Action_ChangeAmmo()
	{
		if (!m_CurrentWeapon || !m_wChangeAmmo.IsVisible())
			return;

		// MUST BE IMPLEMENTED BY PROGRAMMER!
	}

	void Action_ChangeMuzzle()
	{
		if (!m_CurrentWeapon || !m_wChangeMuzzle.IsVisible())
			return;

		// MUST BE IMPLEMENTED BY PROGRAMMER!
	}
};
//---- REFACTOR NOTE END ----
