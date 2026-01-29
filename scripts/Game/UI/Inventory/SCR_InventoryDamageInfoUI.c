//------------------------------------------------------------------------------------------------
//! UI Script
//! Inventory Slot UI Layout
[EntityEditorProps(category: "GameScripted/UI/Inventory", description: "Inventory Item Info UI class")]
class SCR_InventoryDamageInfoUI : ScriptedWidgetComponent
{

	protected Widget								m_wInfoWidget;
	protected TextWidget							m_wTextName;
	protected SCR_SlotUIComponent					m_pFrameSlotUI;
	protected Widget 								m_wWidgetUnderCursor;
	protected bool 									m_bForceShow;
	protected string								m_sTextName;

	protected RichTextWidget						m_wTourniquetHintIconWidget;
	protected ImageWidget							m_wDamageIconWidget, m_wBleedingIconWidget, m_wTourniquetIconWidget, m_wSalineBagIconWidget, m_wMorphineIconWidget, m_wFractureIconWidget, m_wFractureIcon2Widget;
	protected TextWidget							m_wDamageRegenTextWidget, m_wDamageTextWidget, m_wBleedingTextWidget, m_wTourniquetTextWidget, m_wSalineBagTextWidget, m_wMorphineTextWidget, m_wTourniquetHintTextWidget, m_wFractureTextWidget, m_wFractureText2Widget;

	protected ResourceName m_sMedicalIconsImageSet 	= "{B9199157B90D6216}UI/Textures/InventoryIcons/Medical/Medical-icons.imageset";
	protected string m_sBloodIcon					= "Blood_UI";
	protected string m_sTourniquetIcon 				= "Tourniquet_UI";
	protected string m_sSalineBagIcon 				= "Saline-bag_UI";
	protected string m_sMorphineIcon 				= "Morphine_UI";
	protected string m_sFractureIcon 				= "Bone-Fracture_UI";
	protected string m_sTourniquetHintText			= "#AR-RemoveTourniquetAction";
	protected string m_sDamageRegenText 			= "#AR-DamageInfo_Regeneration";
	protected string m_sTourniquetText 				= "#AR-DamageInfo_TourniquetApplied";
	protected string m_sSalinebagText 				= "#AR-DamageInfo_SalineBagApplied";
	protected string m_sMorphineText 				= "#AR-DamageInfo_MorphineApplied";
	protected string m_sArmFractureText 			= "#AR_Inventory_ArmsDamage";
	protected string m_sLegFractureText 			= "#AR_Inventory_LegsDamage";

	//------------------------------------------------------------------------------------------------
	void Show(float fDelay = 0.0, Widget w = null, bool forceShow = false)
	{
		m_bForceShow = forceShow;
		m_wWidgetUnderCursor = w;
		if (fDelay == 0)
		{
			ShowInfoWidget(true);
		}
		else
		{
			// CallLater delays showing up of damageInfoUI
			GetGame().GetCallqueue().Remove(ShowInfoWidget);
			GetGame().GetCallqueue().CallLater(ShowInfoWidget, fDelay * 1000, false, true);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ShowInfoWidget(bool bShow)
	{
			if (m_bForceShow)
		{
			m_wInfoWidget.SetVisible(true);
			return;
		}

		if (!m_wWidgetUnderCursor)
			return;
		
		if (WidgetManager.GetWidgetUnderCursor() != m_wWidgetUnderCursor)
			return; //the cursor is on different position already
		
		m_wInfoWidget.SetVisible(true);
	}

	//------------------------------------------------------------------------------------------------
	void SetDamageStateVisible(bool visible, bool regenerating, string iconIntensity, string damageText)
	{
		if (!m_sMedicalIconsImageSet || !m_wDamageIconWidget || !m_wDamageTextWidget || !m_wDamageRegenTextWidget)
			return;
		
		if (!iconIntensity.IsEmpty())
			m_wDamageIconWidget.LoadImageFromSet(0, m_sMedicalIconsImageSet, iconIntensity);
	
		string regenText = m_sDamageRegenText;
		m_wDamageIconWidget.SetVisible(visible);

		if (!regenerating)
			regenText = "";

		m_wDamageTextWidget.SetText(damageText);
		m_wDamageRegenTextWidget.SetText(regenText);		
		m_wDamageTextWidget.SetVisible(visible);
		m_wDamageRegenTextWidget.SetVisible(visible);
	}

	//------------------------------------------------------------------------------------------------
	void SetBleedingStateVisible(bool visible, string bleedingText)
	{
		if (!m_sMedicalIconsImageSet || !m_wBleedingIconWidget || !m_wBleedingTextWidget)
			return;
		
		m_wBleedingIconWidget.LoadImageFromSet(0, m_sMedicalIconsImageSet, m_sBloodIcon);
		m_wBleedingIconWidget.SetVisible(visible);
	
		m_wBleedingTextWidget.SetVisible(visible);
		m_wBleedingTextWidget.SetText(bleedingText);
	}

	//------------------------------------------------------------------------------------------------
	void SetTourniquetStateVisible(bool visible)
	{
		if (!m_sMedicalIconsImageSet || !m_wTourniquetIconWidget || !m_wTourniquetTextWidget)
			return;

		m_wTourniquetIconWidget.LoadImageFromSet(0, m_sMedicalIconsImageSet, m_sTourniquetIcon);
		m_wTourniquetIconWidget.SetVisible(visible);
		
		m_wTourniquetTextWidget.SetText(m_sTourniquetText);
		m_wTourniquetTextWidget.SetVisible(visible);
		SetTourniquetHintVisible(visible);
	}

	//------------------------------------------------------------------------------------------------
	void SetSalineBagStateVisible(bool visible)
	{
		if (!m_sMedicalIconsImageSet || !m_wSalineBagIconWidget || !m_wSalineBagTextWidget)
			return;
		
		m_wSalineBagIconWidget.LoadImageFromSet(0, m_sMedicalIconsImageSet, m_sSalineBagIcon);
		m_wSalineBagIconWidget.SetVisible(visible);
		
		m_wSalineBagTextWidget.SetText(m_sSalinebagText);
		m_wSalineBagTextWidget.SetVisible(visible);
	}

	//------------------------------------------------------------------------------------------------
	void SetTourniquetHintVisible(bool visible)
	{
		if (!m_wTourniquetHintIconWidget || !m_wTourniquetHintTextWidget)
			return;
		
		m_wTourniquetHintIconWidget.SetVisible(visible);

		m_wTourniquetHintTextWidget.SetText(m_sTourniquetHintText);
		m_wTourniquetHintTextWidget.SetVisible(visible);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetMorphineStateVisible(bool visible)
	{
		if (!m_wMorphineIconWidget || !m_wMorphineTextWidget)
			return;
		
		m_wMorphineIconWidget.LoadImageFromSet(0, m_sMedicalIconsImageSet, m_sMorphineIcon);
		m_wMorphineIconWidget.SetVisible(visible);
		
		m_wMorphineTextWidget.SetText(m_sMorphineText);
		m_wMorphineTextWidget.SetVisible(visible);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetFractureStateVisible(bool armFractured, bool legFractured)
	{
		if (!m_wFractureIconWidget || !m_wFractureIcon2Widget || !m_wFractureTextWidget || !m_wFractureText2Widget)
			return;
		
		m_wFractureIconWidget.LoadImageFromSet(0, m_sMedicalIconsImageSet, m_sFractureIcon);
		m_wFractureIcon2Widget.LoadImageFromSet(0, m_sMedicalIconsImageSet, m_sFractureIcon);
		
		m_wFractureIconWidget.SetVisible(armFractured);
		m_wFractureIcon2Widget.SetVisible(legFractured);

		m_wFractureTextWidget.SetText(m_sArmFractureText);
		m_wFractureText2Widget.SetText(m_sLegFractureText);
				
		m_wFractureTextWidget.SetVisible(armFractured);
		m_wFractureText2Widget.SetVisible(legFractured);
	}

	//------------------------------------------------------------------------------------------------
	void Hide()
	{
		if (!m_wInfoWidget)
			return;
		
		m_wInfoWidget.SetVisible(false);
		m_wInfoWidget.SetEnabled(false);
	}

	//------------------------------------------------------------------------------------------------
	void Move(float x, float y)
	{
		if (!m_pFrameSlotUI)
			return;
		
		m_pFrameSlotUI.SetPosX(x);
		m_pFrameSlotUI.SetPosY(y);
	}

	//------------------------------------------------------------------------------------------------
	void SetName(string sName)
	{
		if (!m_wTextName)
			return;
		
		if (sName.IsEmpty())
		{
			m_wTextName.GetParent().SetVisible(false);
			return;
		}
		
		m_sTextName = sName;
		m_wTextName.SetText(sName);
		m_wTextName.GetParent().SetVisible(true);
	}

	//------------------------------------------------------------------------------------------------
	string GetName()
	{
		return m_sTextName;
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		if (!w)
			return;
		m_wInfoWidget		= w;
		m_wTextName 		= TextWidget.Cast( w.FindAnyWidget( "ItemInfo_name" ) );
		Widget wItemInfo	= m_wInfoWidget.FindAnyWidget( "ItemInfo" );
		if (!wItemInfo)
			return;
		m_pFrameSlotUI 		= SCR_SlotUIComponent.Cast(wItemInfo.FindHandler(SCR_SlotUIComponent));

		m_wDamageIconWidget 			= ImageWidget.Cast( w.FindAnyWidget( "DamageInfo_icon" ) );
		m_wDamageTextWidget 			= TextWidget.Cast( w.FindAnyWidget( "DamageInfo_text" ) );
		m_wFractureIconWidget 			= ImageWidget.Cast( w.FindAnyWidget( "FractureInfo_icon" ) );
		m_wFractureIcon2Widget 			= ImageWidget.Cast( w.FindAnyWidget( "FractureInfo_icon2" ) );
		m_wFractureTextWidget			= TextWidget.Cast( w.FindAnyWidget( "FractureInfo_text" ) );
		m_wFractureText2Widget			= TextWidget.Cast( w.FindAnyWidget( "FractureInfo_text2" ) );
		m_wDamageRegenTextWidget 		= TextWidget.Cast( w.FindAnyWidget( "DamageRegenInfo_text" ) );
		m_wBleedingIconWidget 			= ImageWidget.Cast( w.FindAnyWidget( "BleedingInfo_icon" ) );
		m_wBleedingTextWidget 			= TextWidget.Cast( w.FindAnyWidget( "BleedingInfo_text" ) );
		m_wTourniquetIconWidget 		= ImageWidget.Cast( w.FindAnyWidget( "TourniquetInfo_icon" ) );
		m_wTourniquetTextWidget 		= TextWidget.Cast( w.FindAnyWidget( "TourniquetInfo_text" ) );
		m_wTourniquetHintIconWidget 	= RichTextWidget.Cast( w.FindAnyWidget( "TourniquetHint_icon" ) );
		m_wTourniquetHintTextWidget 	= TextWidget.Cast( w.FindAnyWidget( "TourniquetHint_text" ) );
		m_wSalineBagIconWidget 			= ImageWidget.Cast( w.FindAnyWidget( "SalineBagInfo_icon" ) );
		m_wSalineBagTextWidget 			= TextWidget.Cast( w.FindAnyWidget( "SalineBagInfo_text" ) );
		m_wMorphineIconWidget			= ImageWidget.Cast( w.FindAnyWidget( "MorphineInfo_icon" ) );
		m_wMorphineTextWidget 			= TextWidget.Cast( w.FindAnyWidget( "MorphineInfo_text" ) );
	}

	//------------------------------------------------------------------------------------------------
	void Destroy()
	{
		if (m_wInfoWidget)
		{
			GetGame().GetCallqueue().Remove(ShowInfoWidget);
			m_wInfoWidget.RemoveHandler(m_pFrameSlotUI);
			m_wInfoWidget.RemoveHandler(this);
			m_wInfoWidget.RemoveFromHierarchy();
		}
	}

	//------------------------------------------------------------------------------------------------
	Widget GetInfoWidget()
	{
		return m_wInfoWidget.FindAnyWidget("size");
	}
};
