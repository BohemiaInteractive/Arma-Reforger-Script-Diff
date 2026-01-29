[BaseContainerProps(), BaseContainerCustomTitleField("m_sDisplayName")]
class SCR_DeathDateToolDetail: SCR_EntityTooltipDetail
{	
	[Attribute("RowHolder")]
	protected string m_sRowHolderName;
	
	[Attribute(defvalue: "0.5 0.5 0.5 1", desc: "Color of Text.")]
	protected ref Color m_WidgetColor;
	
	//[Attribute("{B3B00EAA6B2DC305}UI/Textures/Notifications/Notification_Player_Died.edds")]
	//protected ResourceName m_sDiedIcon;
	
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
		}
		
		if (!rowTooltip)
			return false;
		
		SCR_IdentityManagerComponent identityManager =  SCR_IdentityManagerComponent.GetInstance();
		if (!identityManager)
			return false;
		
		int currentRowIndex = 0;
		
		//~ Death/Destruction
		string format, day, month, year, time;
		
		if (identityManager.GetDeathDateAndTimeString(SCR_ExtendedCharacterIdentityComponent.Cast(extendedIdentityComponent), format, day, month, year, time))
		{
			//rowTooltip.AddIcon(currentRowIndex, m_sDiedIcon, m_WidgetColor);
			
			if (rowTooltip.AddText(currentRowIndex, format, m_WidgetColor, day, month, year))
				currentRowIndex++;
			
			//~ Death date
			rowTooltip.AddText(currentRowIndex, time, m_WidgetColor);
		}

		return currentRowIndex > 0;
	}
}