[BaseContainerProps(), BaseContainerCustomTitleField("m_sDisplayName")]
class SCR_CreationDateTooltipDetail: SCR_EntityTooltipDetail
{	
	[Attribute("RowHolder")]
	protected string m_sRowHolderName;
	
	[Attribute(defvalue: "0.5 0.5 0.5 1", desc: "Color of Text.")]
	protected ref Color m_WidgetColor;
	
	[Attribute("{DAD5433E63B93061}UI/Textures/MainMenu/Icons/EventNotification_64x_UI.edds")]
	protected ResourceName m_sCreationDayIcon;
	
	[Attribute("1", desc: "If true will show an icon and given text when it is the entities birthday/creation day")]
	protected bool m_bShowWidgetOnCreationDay;
	
	//------------------------------------------------------------------------------------------------
	override bool CreateDetail(SCR_EditableEntityComponent entity, Widget parent, TextWidget label, bool setFrameslot = true)
	{
		if (label)
			label.SetColor(m_WidgetColor);
		
		return super.CreateDetail(entity, parent, label, setFrameslot);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool InitDetail(SCR_EditableEntityComponent entity, Widget widget)
	{		
		SCR_ExtendedIdentityComponent extendedIdentityComponent = SCR_ExtendedIdentityComponent.Cast(entity.GetOwner().FindComponent(SCR_ExtendedIdentityComponent));
		if (!extendedIdentityComponent)
			return false;
		
		//~ Check if allowed to show bio
		BaseContainer editorSettings = GetGame().GetGameUserSettings().GetModule("SCR_EditorSettings");
		bool showBioTooltip = true;
		
		if (editorSettings)
			editorSettings.Get("m_bShowIdentityBioTooltip", showBioTooltip);
		
		if (!showBioTooltip)
			return false;
		
		SCR_RowTooltipEditorUIComponent rowTooltip = SCR_RowTooltipEditorUIComponent.Cast(widget.FindHandler(SCR_RowTooltipEditorUIComponent));
		if (!rowTooltip)
		{
			Widget rowHolder = widget.FindAnyWidget(m_sRowHolderName);
			if (rowHolder)
				rowTooltip = SCR_RowTooltipEditorUIComponent.Cast(rowHolder.FindHandler(SCR_RowTooltipEditorUIComponent));
			
			if (!rowTooltip)
				return false;
		}
		
		SCR_IdentityManagerComponent identityManager =  SCR_IdentityManagerComponent.GetInstance();
		if (!identityManager)
			return false;
		
		int currentRowIndex;
		
		//~ Set date of birth/creation
		string format, day, month, age, year;
		if (identityManager.GetCreationdayString(extendedIdentityComponent, format, day, month, year, age))
		{
			//~ Is Birthday/Creation day
			if (m_bShowWidgetOnCreationDay && identityManager.IsCreationDay(extendedIdentityComponent))
			{
				if (rowTooltip.AddIcon(currentRowIndex, m_sCreationDayIcon, m_WidgetColor))
					currentRowIndex++;
			}
			
			if (rowTooltip.AddText(currentRowIndex, format, m_WidgetColor, day, month, year, age))
				currentRowIndex++;
		}
		
		//~ Place of creation
		SCR_ExtendedIdentity extendedIdentity = extendedIdentityComponent.GetExtendedIdentity();
		if (extendedIdentity)
		{
			SCR_UIInfo uiInfo = extendedIdentity.GetPlaceOfOriginUIInfo();
			if (uiInfo && uiInfo.GetName())
			{
				if (rowTooltip.AddText(currentRowIndex, uiInfo.GetName(), m_WidgetColor))
					currentRowIndex++;
			}
		}
		
		return currentRowIndex > 0;
	}
};