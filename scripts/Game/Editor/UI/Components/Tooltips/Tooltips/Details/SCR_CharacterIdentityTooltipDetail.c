[BaseContainerProps(), BaseContainerCustomTitleField("m_sDisplayName")]
class SCR_CharacterIdentityTooltipDetail : SCR_EntityTooltipDetail
{
	[Attribute(defvalue: "0.5 0.5 0.5 1", desc: "Color of Text.")]
	protected ref Color m_TextColor;

	[Attribute("0", UIWidgets.SearchComboBox, "Identity type Displayed", "", ParamEnumArray.FromEnum(SCR_EIdentityTooltipType))]
	protected SCR_EIdentityTooltipType m_eDisplayType;

	//------------------------------------------------------------------------------------------------
	override bool CreateDetail(SCR_EditableEntityComponent entity, Widget parent, TextWidget label, bool setFrameslot = true)
	{
		if (label)
			label.SetColor(m_TextColor);

		return super.CreateDetail(entity, parent, label, setFrameslot);
	}

	//------------------------------------------------------------------------------------------------
	override bool InitDetail(SCR_EditableEntityComponent entity, Widget widget)
	{
		TextWidget text = TextWidget.Cast(widget);
		if (!text)
			return false;

		//~ Check if allowed to show bio
		BaseContainer editorSettings = GetGame().GetGameUserSettings().GetModule("SCR_EditorSettings");
		bool showBioTooltip = true;

		if (editorSettings)
			editorSettings.Get("m_bShowIdentityBioTooltip", showBioTooltip);

		if (!showBioTooltip)
			return false;

		text.SetColor(m_TextColor);

		SCR_IdentityManagerComponent charIdentityManager =  SCR_IdentityManagerComponent.GetInstance();
		if (!charIdentityManager)
			return false;
		
		if (m_eDisplayType == SCR_EIdentityTooltipType.NAME)
		{
			//~ It never shows full name only player name so no need to show it in GM either
			if (charIdentityManager.ShowPlayerNameOnIdentityItem() && entity.GetPlayerID() > 0)
				return false;
			
			SCR_CharacterIdentityComponent charIdentityComponent = SCR_CharacterIdentityComponent.Cast(entity.GetOwner().FindComponent(CharacterIdentityComponent));
			if (!charIdentityComponent)
				return false;

			string format, fistName, alias, surName;
			charIdentityComponent.GetFormattedFullName(format, fistName, alias, surName);

			text.SetTextFormat(format, fistName, alias, surName);
			return true;
		}

		SCR_UIInfo uiInfo;
		SCR_ExtendedCharacterIdentityComponent charIdentityComponent = SCR_ExtendedCharacterIdentityComponent.Cast(entity.GetOwner().FindComponent(SCR_ExtendedCharacterIdentityComponent));
		if (!charIdentityComponent)
			return false;

		SCR_ExtendedCharacterIdentity extendedIdentity = SCR_ExtendedCharacterIdentity.Cast(charIdentityComponent.GetExtendedIdentity());
		if (!extendedIdentity)
			return false;

		switch (m_eDisplayType)
		{
			case SCR_EIdentityTooltipType.BLOODTYPE:
			{
				uiInfo = charIdentityManager.GetBloodTypeUIInfo(extendedIdentity.GetBloodType());
				if (!uiInfo)
					return false;

				uiInfo.SetNameTo(text);
				return true;
			}
		}
		return false;
	}
}

enum SCR_EIdentityTooltipType
{
	NAME,
	BLOODTYPE,
}
