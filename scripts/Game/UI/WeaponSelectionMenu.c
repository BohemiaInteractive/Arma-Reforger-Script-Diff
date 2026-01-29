//#define DISABLE_WEAPON_SWITCHING

// Todo: PerFrame Optimization and Refactor
class SCR_WeaponSelectionMenu : SCR_InfoDisplay
{
	private BaseWeaponManagerComponent m_WeaponManager;
	private SCR_InventoryStorageManagerComponent m_inventory20ManagerComp
	private GenericEntity m_Owner;
	private BaseControllerComponent m_Controller;
	private InputManager m_InputManager;
	private BaseInteractionHandlerComponent m_InteractionMenu;
	private Widget m_wVerticalLayout;
	private Widget m_wHighlightedWeapon;

	private float m_fLastInteractionTime = 0;
	private float m_fLastSelectionTime = 0;
	
	private ref array<WeaponSlotComponent> m_aWeaponSlotsUnsorted = new array<WeaponSlotComponent>;
	private ref array<WeaponSlotComponent> m_aWeaponSlotsSorted = new array<WeaponSlotComponent>;
	private ref array<IEntity> m_aWeaponEntities = new array<IEntity>;
	private ref array<Widget> m_aWeaponWidgets = new array<Widget>;
	
	protected static ref ScriptInvoker Event_OnSelectWeapon = new ScriptInvoker;
	
	[Attribute("{80102F7397A6DFDC}UI/layouts/HUD/WeaponSelection/WeaponSelectionCategoryNumber.layout", UIWidgets.ResourceNamePicker, "Layout of category number widget","layout")]
	ResourceName m_sNumberWidgetLayout;
	
	[Attribute("{F6725A5E6A624F5F}UI/layouts/HUD/WeaponSelection/WeaponSlot.layout", UIWidgets.ResourceNamePicker, "Scoreboard layout","layout")]
	ResourceName m_sWeaponWidgetLayout;
	
	[Attribute("100", UIWidgets.EditBox, "Speed of scrolling between weapons (delay in ms)")]
	private float m_fSelectionTimeout;
	
	[Attribute("5000", UIWidgets.EditBox, "Weapon selection auto closing time (in ms)")]
	private float m_fInteractionTimeout;
	
	[Attribute("1 1 1 0.10", UIWidgets.ColorPicker, "")]
	protected ref Color m_ColorSelected;
	
	[Attribute("0 0 0 1", UIWidgets.ColorPicker, "")]
	protected ref Color m_ColorChildSelected;
	
	[Attribute("0 0 0 0.1", UIWidgets.ColorPicker, "")]
	protected ref Color m_ColorDefault;
	
	[Attribute("1 1 1 1", UIWidgets.ColorPicker, "")]
	protected ref Color m_ColorChildDefault;
	
	[Attribute("5", UIWidgets.EditBox, "")]
	protected float m_fHighlightAnimationSpeed;
	
	[Attribute(SCR_SoundEvent.FOCUS, UIWidgets.EditBox, "")]
	protected ResourceName m_sSoundSwitch;

	[Attribute("{0054C496786770A6}UI/Textures/Icons/icons_keyboard_32.imageset", UIWidgets.ResourceNamePicker, "")]
	protected ResourceName m_sWeaponNumberImageset;
	
	//------------------------------------------------------------------------------------------------
	void Init(IEntity owner)
	{
		m_Owner = GenericEntity.Cast(owner);
		if (!m_Owner || !m_wRoot)
			return;
		
		// Get action menu
		// Todo: Cache workspace
		ArmaReforgerScripted game = GetGame();
		if (game)
		{
			PlayerController pc = game.GetPlayerController();
			if (pc)
			{
				m_InteractionMenu = BaseInteractionHandlerComponent.Cast(pc.FindComponent(BaseInteractionHandlerComponent));
			}
			
			HUDManagerComponent hudManager = game.GetHUDManager();
			if (hudManager)
			{
				array<SCR_InfoDisplay> displays = hudManager.GetHUDElements();
				foreach (SCR_InfoDisplay display : displays)
				{
					SCR_WeaponSelectionMenu weaponSwitching = SCR_WeaponSelectionMenu.Cast(display);
					if (weaponSwitching && weaponSwitching != this)
					{
						weaponSwitching.Show(false, UIConstants.FADE_RATE_DEFAULT);
						break;
					}
				}
			}
		}

		m_InputManager = GetGame().GetInputManager();
		
		if (Vehicle.Cast(m_Owner))
		{
			BaseVehicleNodeComponent nodeComponent = BaseVehicleNodeComponent.Cast(m_Owner.FindComponent(BaseVehicleNodeComponent));
			if (!nodeComponent)
				return;
			m_Controller = BaseControllerComponent.Cast(nodeComponent.FindComponent(BaseControllerComponent));
			m_WeaponManager = BaseWeaponManagerComponent.Cast(nodeComponent.FindComponent(BaseWeaponManagerComponent));

		}
		else
		{
			m_Controller = BaseControllerComponent.Cast(m_Owner.FindComponent(BaseControllerComponent));
			m_WeaponManager = BaseWeaponManagerComponent.Cast(m_Owner.FindComponent(BaseWeaponManagerComponent));
		}
		
		if (!m_WeaponManager)
			return;
		
		array<WeaponSlotComponent> weapons = new array<WeaponSlotComponent>;
		m_WeaponManager.GetWeaponsSlots(weapons);
		
		m_aWeaponSlotsUnsorted.Clear();
		foreach (WeaponSlotComponent slot : weapons)
		{
			m_aWeaponSlotsUnsorted.Insert(slot);
		}
		
		m_aWeaponSlotsSorted = GetSortedWeaponSlots(m_aWeaponSlotsUnsorted);
		
		/*
		foreach (WeaponSlotComponent weapon: m_aWeaponSlotsSorted)
		{
			if (weapon && weapon.GetWeaponEntity())
			{
				m_Controller.SelectWeapon(weapon);
				break;
			}
		}
		*/
		
		// Hide menu and scan all widgets in slots
		m_wRoot.SetOpacity(0);
		m_wVerticalLayout = m_wRoot.FindAnyWidget("WeaponMenu");
		
		Widget child = m_wRoot.GetChildren();
		if (child)
		{
			while(child)
			{
				Widget newChild = null;
				if (child)
				{
					newChild = child.GetSibling();
					child = newChild;
				}
			}
		}
		
		// Init widgets
		CreateWeaponWidgets();
	}
	
	//------------------------------------------------------------------------------------------------
	void EquipHealingItem()
	{
		if (!m_inventory20ManagerComp)
			m_inventory20ManagerComp = SCR_InventoryStorageManagerComponent.Cast(m_Owner.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!m_inventory20ManagerComp)
			return;
		
		CharacterControllerComponent controller = CharacterControllerComponent.Cast(m_Owner.FindComponent(CharacterControllerComponent));
		if (!controller)
			return;
		
		auto bandage = m_inventory20ManagerComp.GetBandageItem();
		if (bandage)
			controller.TakeGadgetInLeftHand(bandage, 1);
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateWeaponsList()
	{
		m_aWeaponEntities.Clear();
		foreach (WeaponSlotComponent weapon: m_aWeaponSlotsUnsorted)
		{
			IEntity entity = weapon.GetWeaponEntity();
			m_aWeaponEntities.Insert(entity);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	bool DidWeaponsChange()
	{
		if (m_aWeaponSlotsUnsorted.Count() != m_aWeaponEntities.Count())
			return true;
		
		foreach (int i, WeaponSlotComponent slot : m_aWeaponSlotsUnsorted)
		{
			if (slot && slot.GetWeaponEntity() != m_aWeaponEntities.Get(i))
				return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	bool SelectHighlightedWeapon()
	{
		if (!m_Controller)
			return false;
		
		int i = m_aWeaponWidgets.Find(m_wHighlightedWeapon);
		if (i < 0 && i < m_aWeaponSlotsSorted.Count())
			return false;
		
		WeaponSlotComponent slot = m_aWeaponSlotsSorted.Get(i);
		if (!slot)
			return false;
				
		if (slot != m_WeaponManager.GetCurrent())
		{
			OnSelectWeapon(slot);
			return true;
		}
		 return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void CreateWeaponWidgets()
	{
		if (!m_wVerticalLayout)
			return;
		
		// Delete all categories
		Widget child = m_wVerticalLayout.GetChildren();
		while (child) 
		{
			Widget nextChild = child.GetSibling();
			child.RemoveFromHierarchy();
			child = nextChild;
		}
		
		m_aWeaponWidgets.Clear();
		
		foreach (WeaponSlotComponent slot: m_aWeaponSlotsSorted)
		{
			if (!slot || !slot.GetWeaponEntity())
			{
				m_aWeaponWidgets.Insert(null);
				continue;
			}
			
			// Create category
			Widget category = GetGame().GetWorkspace().CreateWidgets("{93F4C116E103AFD4}UI/layouts/HUD/WeaponSelection/WeaponCategory.layout",m_wVerticalLayout);
			if (!category)
				continue;
			
			VerticalLayoutSlot.SetPadding(category, 0,4,0,4);
			
			// Create weapon prefab
			int categoryNumber = slot.GetWeaponSlotIndex() + 1;
			
			Widget w = LoadWeaponPrefab(category, slot, categoryNumber);
			m_aWeaponWidgets.Insert(w);
			
			Widget numberRoot = GetGame().GetWorkspace().CreateWidgets(m_sNumberWidgetLayout,category);
			if (!numberRoot)
				continue;

			ImageWidget number = ImageWidget.Cast(numberRoot.FindAnyWidget("NumberText"));
			if (number)
				number.LoadImageFromSet(0, m_sWeaponNumberImageset, categoryNumber.ToString());
		}
	}

	//------------------------------------------------------------------------------------------------
	array<WeaponSlotComponent> GetSortedWeaponSlots(array<WeaponSlotComponent> weapons)
	{
		array<WeaponSlotComponent> sortedWeapons = new array<WeaponSlotComponent>;
		array<int> slotIndexes = new array<int>;

		foreach (WeaponSlotComponent slot : weapons)
		{
			if (slot)
			{
				int i = slot.GetWeaponSlotIndex() + 1;
				if (i < 1)
					i = 100;
				slotIndexes.Insert(i);
			}
			else
			{
				slotIndexes.Insert(100);
			}
		}
		
		int count = weapons.Count();
		for (int i = 0; i < count; i++)
		{
			int lowestScore = 1000;
			int slotIndex = -1;
			
			for (int j = 0; j < count;  j++)
			{
				if (slotIndexes.Get(j) <= lowestScore)
				{
					slotIndex = j;
					lowestScore = slotIndexes.Get(j);
				}
			}
			
			if (slotIndex > -1)
			{
				WeaponSlotComponent slot = weapons.Get(slotIndex);
				sortedWeapons.Insert(slot);
				slotIndexes.Set(slotIndex,1000);
			}
		}
		
		return sortedWeapons;
	}
	
	//------------------------------------------------------------------------------------------------
	Widget LoadWeaponPrefab(Widget parent, BaseWeaponComponent weapon, int category)
	{
		if (!parent)
			return null;
			
		Widget w = GetGame().GetWorkspace().CreateWidgets(m_sWeaponWidgetLayout, parent);
		if (!w)
			return null;
		
		HorizontalLayoutSlot.SetPadding(w,0,0,2,0);
		SetDefaultColor(w,false);
		
		UIInfo uiInfo = weapon.GetUIInfo();
		if (uiInfo)
		{
			TextWidget name = TextWidget.Cast(w.FindAnyWidget("Name"));
			ImageWidget icon = ImageWidget.Cast(w.FindAnyWidget("Icon"));

			if (name)
			{
				string text = uiInfo.GetName();
				text.ToUpper();
				name.SetText(text);
			}
			
			if (icon)
			{
				string path = uiInfo.GetIconPath();
				if (path != string.Empty)
				{
					icon.LoadImageTexture(0, path);
					int x, y;
					icon.GetImageSize(0,x,y);
					icon.SetSize((float)x, (float)y);
				}
			}
		}
		else
		{
			// Weapon slot is empty: Hide the widget
			w.SetVisible(false);
		}
		return w;
	}
	
	//------------------------------------------------------------------------------------------------
	void HighlightWeapon(int i)
	{
		int count = m_aWeaponWidgets.Count();
		if (i > -1 && i < count)
		{
			Widget w = m_aWeaponWidgets.Get(i);
			if (w && m_wHighlightedWeapon != w)
			{
				SetDefaultColor(m_wHighlightedWeapon);
				SetHighlightColor(w);
				SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.ITEM_SELECTED);
				m_wHighlightedWeapon = w;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void HighlightNextWeapon()
	{
		int i = GetHighlightedWeaponIndex();
		
		int count = m_aWeaponWidgets.Count();
		i++;
		for (i; i < count; i++)
		{
			if (m_aWeaponWidgets.Get(i))
				break;
			
			if (i == count)
				return;
		}
		
		HighlightWeapon(i);
	}
	
	//------------------------------------------------------------------------------------------------
	void HighlightPreviousWeapon()
	{
		int i = GetHighlightedWeaponIndex();
		i--;
		for (i; i >= 0; i--)
		{
			if (m_aWeaponWidgets.Get(i))
				break;
			
			if (i == 0)
				return;
		}
		HighlightWeapon(i);
	}
	
	//------------------------------------------------------------------------------------------------
	void HighlightCurrentWeapon()
	{
		int i = GetHighlightedWeaponIndex();
		HighlightWeapon(i);
	}
	
	//------------------------------------------------------------------------------------------------
	int GetHighlightedWeaponIndex()
	{
		int i = -1;
		if (!m_wHighlightedWeapon)
		{
			if (!m_WeaponManager)
				return -1; 
			
			WeaponSlotComponent currentSlot = m_WeaponManager.GetCurrentSlot();
			if (!currentSlot)
				return -1;
			
			i = m_aWeaponSlotsSorted.Find(currentSlot);
		}
		else
		{
			i = m_aWeaponWidgets.Find(m_wHighlightedWeapon);
		}
		
		return i;
	}

	//------------------------------------------------------------------------------------------------
	void HandleKeyboardWeaponSwitch()
	{
		int targetSlot = -1;
		
		if (m_InputManager.GetActionTriggered("SwitchWeaponCategory1"))
			targetSlot = 0;
		else if (m_InputManager.GetActionTriggered("SwitchWeaponCategory2"))
			targetSlot = 1;
		else if (m_InputManager.GetActionTriggered("SwitchWeaponCategory3"))
			targetSlot = 2;
		else if (m_InputManager.GetActionTriggered("SwitchWeaponCategory4"))
			targetSlot = 3;
		else if (m_InputManager.GetActionTriggered("SwitchWeaponCategory5"))
			targetSlot = 4;
		else if (m_InputManager.GetActionTriggered("SwitchWeaponCategory6"))
			targetSlot = 5;

		if (targetSlot < 0 || !m_aWeaponSlotsSorted)
			return;

		foreach (int i, WeaponSlotComponent slot : m_aWeaponSlotsSorted)
		{
			if (!slot)
				continue;
			
			if (targetSlot == slot.GetWeaponSlotIndex())
			{
				if (!m_Controller || !m_WeaponManager)
					return;
				
				IEntity weaponEntity = slot.GetWeaponEntity();
				WeaponSlotComponent currentlySelectedSlot = WeaponSlotComponent.Cast(m_WeaponManager.GetCurrent());
				
				// Prevent switching to empty weapon slot or the same weapon is selected
				if (weaponEntity && currentlySelectedSlot != slot)
				{
					OnSelectWeapon(slot);
					if (IsShown() && i < m_aWeaponWidgets.Count())
					{
						Widget w = m_aWeaponWidgets.Get(i);
						if (w)
							GetGame().GetWorkspace().SetFocusedWidget(w);
					}
					Show(false);
				}
			}
		
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SetDefaultColor(Widget w, bool animate = true)
	{
		HighlightWidget(w, m_ColorDefault, m_ColorChildDefault, animate);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetHighlightColor(Widget w, bool animate = true)
	{
		HighlightWidget(w, m_ColorSelected, m_ColorChildSelected, animate);
	}
	
	//------------------------------------------------------------------------------------------------
	void HighlightWidget(Widget w, Color backgroundColor, Color colorChild, bool animate = true)
	{
		if (!w)
			return;
		
		Widget background = w.FindAnyWidget("Background");
		if (!background)
			return;
		
		SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.ITEM_CONFIRMED);
		
		if (animate)
		{
			AnimateWidget.Color(background, backgroundColor, m_fHighlightAnimationSpeed);
			AnimateWidget.Color(w, colorChild, m_fHighlightAnimationSpeed);
		}
		else
		{
			background.SetColor(backgroundColor);
			w.SetColor(colorChild);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	private void RegisterListeners()
	{
		if (!m_InputManager)
			return;
		
		m_InputManager.AddActionListener("SwitchWeaponCategory1", EActionTrigger.DOWN, HandleKeyboardWeaponSwitch);
		m_InputManager.AddActionListener("SwitchWeaponCategory2", EActionTrigger.DOWN, HandleKeyboardWeaponSwitch);
		m_InputManager.AddActionListener("SwitchWeaponCategory3", EActionTrigger.DOWN, HandleKeyboardWeaponSwitch);
		m_InputManager.AddActionListener("SwitchWeaponCategory4", EActionTrigger.DOWN, HandleKeyboardWeaponSwitch);
		m_InputManager.AddActionListener("SwitchWeaponCategory5", EActionTrigger.DOWN, EquipHealingItem);
		m_InputManager.AddActionListener("SwitchWeaponCategory6", EActionTrigger.DOWN, HandleKeyboardWeaponSwitch);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Show(bool show, float speed = UIConstants.FADE_RATE_DEFAULT, EAnimationCurve curve = EAnimationCurve.LINEAR)
	{
		super.Show(show, speed);
		
		//Added this to update the weapon selection upon showing the menu
		if (show)
		{
			UpdateWeaponsList();
			CreateWeaponWidgets();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	private void OnSelectWeapon(WeaponSlotComponent slot)
	{
		if (Event_OnSelectWeapon)
			Event_OnSelectWeapon.Invoke();
		
		if (!m_Controller)
			return;
		
		CharacterControllerComponent characterController = CharacterControllerComponent.Cast(m_Controller);
		if (characterController)
		{
			characterController.SelectWeapon(slot);
			return;
		}
		
		TurretControllerComponent turretController = TurretControllerComponent.Cast(m_Controller);
		if (turretController)
		{
			IEntity player = SCR_PlayerController.GetLocalControlledEntity();
			turretController.SelectWeapon(player, slot);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! OnSelectWeapon invoker
	static ScriptInvoker GetOnSelectWeaponInvoker()
	{
		return Event_OnSelectWeapon;
	}	
	
	#define DISABLE_WEAPON_SWITCHING
	#ifndef DISABLE_WEAPON_SWITCHING
	//------------------------------------------------------------------------------------------------
	override event void UpdateValues(IEntity owner, float timeSlice)
	{
		// Todo: Rewrite this per frame update at event base system as much as possible. Idealy remove whole per frame call.
		
		//Checking this only when the selection is shown, as only then it's relevant
		if (m_bShown && DidWeaponsChange())
		{
			UpdateWeaponsList();
			CreateWeaponWidgets();
		}
		
		float cancel = m_InputManager.GetActionTriggered("CharacterCancelWeaponSwitch");
		float switching = m_InputManager.GetActionValue("CharacterSwitchWeapon");
		float radialSwitch = m_InputManager.GetActionValue("CharacterSwitchWeaponRadial");
		
		// TODO@AS: Input collision workaround, resolve
		if (Math.AbsFloat(m_InputManager.GetActionValue("CharacterSpeedAnalog")) > 0)
			switching = 0;
		
		if (m_InteractionMenu && m_InteractionMenu.IsInteractionAvailable())
		{
			Show(false);
			return;
		}
		
		if (switching != 0 || m_bShown)
		{
			m_InputManager.ActivateContext("WeaponSelectionContext", 250);
			if (m_InputManager.GetActionTriggered("WeaponSelection"))
			{
				m_fLastSelectionTime = owner.GetWorld().GetWorldTime();
				SelectHighlightedWeapon();
				Show(false);
				switching = 0;
			}
		}
		
		if (m_bShown)
		{
			if (cancel /*|| m_Player.IsInVehicle()*/)
			{
				Show(false);
				return;
			}
			
			float time = owner.GetWorld().GetWorldTime();
			if (switching == 0 && time > m_fLastInteractionTime + m_fInteractionTimeout)
			{
				SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.ITEM_CANCELLED);
				Show(false, UIConstants.FADE_RATE_SLOW);
				return;
			}
		}
		
		if (switching != 0)
		{
			float time = owner.GetWorld().GetWorldTime();
			float remainingTime = time - m_fLastSelectionTime + m_fSelectionTimeout;
			
			m_fLastInteractionTime = time;
			if (!m_bShown)
			{
				Show(true);
				HighlightCurrentWeapon();
				m_fLastSelectionTime = time;
			}
			else if (time > m_fLastSelectionTime + m_fSelectionTimeout)
			{
				m_fLastSelectionTime = time;
				if (switching > 0)
					HighlightPreviousWeapon();
				else
					HighlightNextWeapon();
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override event void OnStopDraw(IEntity owner)
	{
		m_aWeaponSlotsUnsorted.Clear();
		m_aWeaponSlotsSorted.Clear();
		
		HUDManagerComponent hudManager = GetGame().GetHUDManager();
		if (hudManager)
		{
			array<SCR_InfoDisplay> displays = hudManager.GetHUDElements();
			foreach (SCR_InfoDisplay display : displays)
			{
				SCR_WeaponSelectionMenu weaponSwitching = SCR_WeaponSelectionMenu.Cast(display);
				if (weaponSwitching && weaponSwitching != this)
				{
					weaponSwitching.Show(true, UIConstants.FADE_RATE_DEFAULT);
					break;
				}
			}
		}
		
		super.OnStopDraw(owner);
	}
	
	//------------------------------------------------------------------------------------------------
	override event void OnStartDraw(IEntity owner)
	{
		super.OnStartDraw(owner);
		Init(owner);
		RegisterListeners();
	}
	#endif
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_WeaponSelectionMenu()
	{
		
	}
};