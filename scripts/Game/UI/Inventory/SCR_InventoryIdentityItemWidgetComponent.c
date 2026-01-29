class SCR_InventoryIdentityItemWidgetComponent : SCR_ScriptedWidgetComponent
{
	[Attribute("CharacterIdentityHolder")]
	protected string m_sCharacterIdentityHolder;
	
	[Attribute("GenericIdentityHolder")]
	protected string m_sGenericIdentityHolder;
	
	[Attribute("NameHolder")]
	protected string m_sNameHolder;
	
	[Attribute("Name")]
	protected string m_sNameText;
	
	[Attribute("CallsignHolder")]
	protected string m_sCallsignHolder;
	
	[Attribute("Callsign")]
	protected string m_sCallsignText;
	
	[Attribute("CreationHolder")]
	protected string m_sCreationHolder;
	
	[Attribute("CreationDate")]
	protected string m_sCreationDateText;
	
	[Attribute("BirthdayIcon")]
	protected string m_sBirthdayIcon;
	
	[Attribute("BloodTypeHolder")]
	protected string m_sBloodTypeHolder;
	
	[Attribute("BloodType")]
	protected string m_sBloodTypeText;
	
	[Attribute("PlaceOfCreation")]
	protected string m_sPlaceOfCreationText;

	[Attribute("BioHolder")]
	protected string m_sBioHolder;
	
	[Attribute("Bio")]
	protected string m_sBioText;
	
	[Attribute("ValuableIntelHolder")]
	protected string m_sValuableIntelHolder;
	
	//------------------------------------------------------------------------------------------------
	void SetIdentityData(notnull SCR_IdentityInventoryItemComponent identityItemComponent, notnull SCR_ExtendedIdentityComponent linkedIdentity, SCR_IdentityBio identityBio)
	{		
		SCR_IdentityManagerComponent identityManager = SCR_IdentityManagerComponent.GetInstance();		
		
		ShowValuableIntelHint(identityManager);
		
		Widget characterIdentityHolder = GetRootWidget().FindAnyWidget(m_sCharacterIdentityHolder);
		Widget genericIdentityHolder = GetRootWidget().FindAnyWidget(m_sGenericIdentityHolder);
		
		Widget activeIdentityHolder, inactiveIdentityHolder;
		
		//~ Set character identity
		if (linkedIdentity.GetIdentityType() == SCR_EIdentityType.CHARACTER)
		{
			activeIdentityHolder = characterIdentityHolder;
			inactiveIdentityHolder = genericIdentityHolder;
		}
		//~ Set generic identity
		else 
		{
			activeIdentityHolder = genericIdentityHolder;
			inactiveIdentityHolder = characterIdentityHolder;
		}
		
		if (inactiveIdentityHolder)
			inactiveIdentityHolder.SetVisible(false);
			
		//~ Could not find Identity Holder
		if (!activeIdentityHolder)
			return;
		
		//~ Set name
		Widget nameHolder = activeIdentityHolder.FindAnyWidget(m_sNameHolder);
		if (nameHolder)
		{				
			bool nameSet = false;
			
			TextWidget nameText = TextWidget.Cast(nameHolder.FindAnyWidget(m_sNameText));
			
			if (nameText)
			{
				//~ Get player name instead of Identity name
				SCR_ExtendedCharacterIdentityComponent extendedCharacterIdentity = SCR_ExtendedCharacterIdentityComponent.Cast(linkedIdentity);
				if (identityManager && extendedCharacterIdentity && identityManager.ShowPlayerNameOnIdentityItem())
				{
					if (extendedCharacterIdentity.GetPlayerID() > 0)
					{
						string playerName = SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(extendedCharacterIdentity.GetPlayerID());
						nameText.SetText(playerName);
						
						nameSet = !playerName.IsEmpty();
					}
					//~ Get player name via linked identity owner instead
					else
					{
						IEntity identityOwner = linkedIdentity.GetOwner();
						if (identityOwner)
						{
							int ownerPlayerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(identityOwner);
							if (ownerPlayerId > 0)
							{
								string ownerName = SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(ownerPlayerId);
								
								nameText.SetText(ownerName);
								nameSet = !ownerName.IsEmpty();
							}					
						}	
					}
				}
				
				//~ Use Identity name instead
				if (!nameSet)
				{
					SCR_CharacterIdentityComponent characterIdentity = SCR_CharacterIdentityComponent.Cast(linkedIdentity.GetOwner().FindComponent(SCR_CharacterIdentityComponent));
					if (characterIdentity)
					{
						string format, fistName, alias, surName;
						characterIdentity.GetFormattedFullName(format, fistName, alias, surName);
						
						nameText.SetTextFormat(format, fistName, alias, surName);
						nameSet = true;
					}
					else 
					{
						SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(linkedIdentity.GetOwner().FindComponent(SCR_EditableEntityComponent));
						if (editableEntity && editableEntity.GetEntityType() != EEditableEntityType.CHARACTER)
						{
							nameText.SetText(editableEntity.GetDisplayName());
							nameSet = true;
						}
					}
				}
				
				
			}

			if (!nameSet)
				nameHolder.SetVisible(false);
		}
		
		//~ Set Callsign
		Widget callsignHolder = activeIdentityHolder.FindAnyWidget(m_sCallsignHolder);
		if (callsignHolder)
		{
			bool showCallsign = true;
			
			FactionAffiliationComponent factionAffiliation = FactionAffiliationComponent.Cast(linkedIdentity.GetOwner().FindComponent(FactionAffiliationComponent));
			if (factionAffiliation)
			{
				SCR_Faction faction = SCR_Faction.Cast(factionAffiliation.GetAffiliatedFaction());
				
				if (faction)
					showCallsign = faction.IsMilitary();
			}
			
			TextWidget callsignText = TextWidget.Cast(callsignHolder.FindAnyWidget(m_sCallsignText));
			if (showCallsign && callsignText)
			{
				SCR_CallsignBaseComponent callsignComp = SCR_CallsignBaseComponent.Cast(linkedIdentity.GetOwner().FindComponent(SCR_CallsignBaseComponent));
				if (callsignComp)
				{
					string company, platoon, squad, character, format;
					
					if (callsignComp.GetCallsignNames(company, platoon, squad, character, format))
						callsignText.SetTextFormat(format, company, platoon, squad, character);
					else 
						callsignHolder.SetVisible(false);
					
				}
				else
				{
					callsignHolder.SetVisible(false);
				} 
			}
			else 
			{
				callsignHolder.SetVisible(false);
			}
		}
		
		//~ Creation / Birthday
		Widget creationHolder = activeIdentityHolder.FindAnyWidget(m_sCreationHolder);
		if (creationHolder)
		{
			TextWidget creationDateText = TextWidget.Cast(creationHolder.FindAnyWidget(m_sCreationDateText));
			if (identityManager && creationDateText)
			{
				string format, day, month, year, age;
				if (identityManager.GetCreationdayString(linkedIdentity, format, day, month, year, age))
				{
					creationDateText.SetTextFormat(format, day, month, year, age);
					
					if (identityManager.IsCreationDay(linkedIdentity))
					{
						Widget birthDayWidget = creationHolder.FindAnyWidget(m_sBirthdayIcon);
						if (birthDayWidget)
							birthDayWidget.SetVisible(true);
					}
				}
				else 
				{
					creationDateText.SetVisible(false);
				}
			}
			
			//~ Place of creation / Place of birth
			TextWidget placeOfCreationText = TextWidget.Cast(creationHolder.FindAnyWidget(m_sPlaceOfCreationText));
			if (placeOfCreationText)
			{
				SCR_ExtendedIdentity extendedIdentity = linkedIdentity.GetExtendedIdentity();
				SCR_UIInfo uiInfo;
				
				if (extendedIdentity)
					uiInfo = extendedIdentity.GetPlaceOfOriginUIInfo();
				
				if (uiInfo && !uiInfo.GetName().IsEmpty())
					uiInfo.SetNameTo(placeOfCreationText);
				else
					placeOfCreationText.SetVisible(false);	
			}
			
			//~ None set hide holder
			if ((!creationDateText || !creationDateText.IsVisible()) && (!placeOfCreationText || !placeOfCreationText.IsVisible()))
				creationHolder.SetVisible(false);
		}
		
		//~ Blood type
		Widget bloodTypeHolder = activeIdentityHolder.FindAnyWidget(m_sBloodTypeHolder);
		if (bloodTypeHolder)
		{
			TextWidget bloodTypeText = TextWidget.Cast(bloodTypeHolder.FindAnyWidget(m_sBloodTypeText));
			if (identityManager && bloodTypeText)
			{
				SCR_ExtendedCharacterIdentity extendedIdentity = SCR_ExtendedCharacterIdentity.Cast(linkedIdentity.GetExtendedIdentity());
				if (extendedIdentity)
				{
					SCR_UIInfo uiInfo = identityManager.GetBloodTypeUIInfo(extendedIdentity.GetBloodType());
					if (uiInfo)
						uiInfo.SetNameTo(bloodTypeText);
					else 
						bloodTypeHolder.SetVisible(false);
				}
				else 
				{
					bloodTypeHolder.SetVisible(false);
				}
			}
			else 
			{
				bloodTypeHolder.SetVisible(false);
			}
		}
		
		//~ Set Bio
		Widget bioHolder = activeIdentityHolder.FindAnyWidget(m_sBioHolder);
		if (bioHolder)
		{
			if (identityBio)
			{
				TextWidget bioHolderText = TextWidget.Cast(bioHolder.FindAnyWidget(m_sBioText));
				if (bioHolderText && !identityBio.GetBioText().IsEmpty())
				{
					bioHolderText.SetText(identityBio.GetBioText());
				}
				else 
				{
					bioHolder.SetVisible(false);
				}
			}
			else 
			{
				bioHolder.SetVisible(false);
			}
		}
		
		//~ Set ValuableIntel
		Widget ValuableIntelHolder = activeIdentityHolder.FindAnyWidget(m_sValuableIntelHolder);
		if (ValuableIntelHolder)
			ValuableIntelHolder.SetVisible(identityItemComponent.HasValuableIntel());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ShowValuableIntelHint(SCR_IdentityManagerComponent identityManager)
	{
		if (!identityManager || !identityManager.IsGenerateValuableIntelEnabled())
			return;
	
		//~ Show hint when hovering over the item
		identityManager.ShowHint(EHint.GAMEPLAY_PERSONAL_BELONGINGS);
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool GetCreationDay(notnull SCR_ExtendedIdentityComponent linkedIdentity, out string format, out string day, out string month, out string year, out string age, out bool isCreationDay)
	{
		SCR_IdentityManagerComponent identityManager =  SCR_IdentityManagerComponent.GetInstance();
		if (!identityManager)
			return false;
		
		if (!identityManager.GetCreationdayString(linkedIdentity, format, day, month, year, age))
			return false;

		isCreationDay = identityManager.IsCreationDay(linkedIdentity);
		
		return true;
	}
}
