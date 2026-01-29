[BaseContainerProps(), BaseContainerCustomTitleField("m_sDisplayName")]
class SCR_WeaponsTooltipDetail : SCR_EntityTooltipDetail
{
	//[Attribute(defvalue: "3")]
	//protected int m_iColumns;
	
	//protected Widget m_Grid;
	//protected ResourceName m_WeaponLayout;
	
	[Attribute("#AR-Editor_TooltipDetail_Weapons_None")]
	protected LocalizedString m_sUnarmedText;
	
	protected TextWidget m_Text;
	protected BaseWeaponManagerComponent m_WeaponManager;
	
	//------------------------------------------------------------------------------------------------
	override bool NeedUpdate()
	{
		return false;//m_WeaponManager != null; //--- ToDo: More optimized
	}

	//------------------------------------------------------------------------------------------------
	override void UpdateDetail(SCR_EditableEntityComponent entity)
	{
		if (!m_WeaponManager || !m_Text)
			return;
		
		//WorkspaceWidget workspace = GetGame().GetWorkspace();
		
		array<WeaponSlotComponent> weapons = {};
		m_WeaponManager.GetWeaponsSlots(weapons);
		WeaponUIInfo info;
		string text;
		//int n;
		foreach (WeaponSlotComponent weapon : weapons)
		{
			if (weapon.GetWeaponSlotType() == "grenade")
				continue;
			
			info = WeaponUIInfo.Cast(weapon.GetUIInfo());
			if (!info)
				continue;
			
			text += info.GetName() + "\n";
			
//			ImageWidget weaponWidget = ImageWidget.Cast(workspace.CreateWidgets(m_WeaponLayout, m_Grid));
//			weaponWidget.LoadImageTexture(0, info.GetWeaponIconPath());
//
//			GridSlot.SetRow(weaponWidget, Math.Floor(n / m_iColumns));
//			GridSlot.SetColumn(weaponWidget, n % m_iColumns);
//			n++;
		}
		
		if (!text.IsEmpty())
			m_Text.SetText(text);
		else
			m_Text.SetText(m_sUnarmedText);
		
	}

	//------------------------------------------------------------------------------------------------
	override bool InitDetail(SCR_EditableEntityComponent entity, Widget widget)
	{
		m_WeaponManager = BaseWeaponManagerComponent.Cast(entity.GetOwner().FindComponent(BaseWeaponManagerComponent));
		if (!m_WeaponManager) 
			return false;
		
		m_Text = TextWidget.Cast(widget);
		if (!m_Text)
			return false;
		
		DamageManagerComponent damageManager = DamageManagerComponent.Cast(entity.GetOwner().FindComponent(DamageManagerComponent));
		if (damageManager)
		{
			if (damageManager.GetState() == EDamageState.DESTROYED)
				return false;
		}
		
		return true;

//		m_Grid = widget;
//		if (!m_Grid)
//			return false;
//
//		m_WeaponLayout = SCR_LayoutTemplateComponent.GetLayout(widget);
//		if (m_WeaponLayout.IsEmpty())
//			return false;
//
//		return true;
	}
}
