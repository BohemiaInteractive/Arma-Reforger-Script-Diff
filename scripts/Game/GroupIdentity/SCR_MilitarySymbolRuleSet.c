[BaseContainerProps(configRoot: true)]
class SCR_MilitarySymbolRuleSet
{
	[Attribute()]
	protected ref array<ref SCR_GroupIdentityRuleIdentity> m_aRulesIdentity;
	
	[Attribute()]
	protected ref array<ref SCR_GroupIdentityRuleDimension> m_aRulesDimension;
	
	[Attribute()]
	protected ref array<ref SCR_GroupIdentityRuleIcon> m_aRulesIcon;
	
	[Attribute()]
	protected ref array<ref SCR_GroupIdentityRuleAmplifier> m_aRulesAmplifier;
	
	protected ref SCR_SortedArray<SCR_GroupIdentityRule> m_aRulesIndentitySorted = new SCR_SortedArray<SCR_GroupIdentityRule>();
	protected ref SCR_SortedArray<SCR_GroupIdentityRule> m_aRulesDimensionSorted = new SCR_SortedArray<SCR_GroupIdentityRule>();
	protected ref SCR_SortedArray<SCR_GroupIdentityRule> m_aRulesIconSorted = new SCR_SortedArray<SCR_GroupIdentityRule>();
	protected ref SCR_SortedArray<SCR_GroupIdentityRule> m_aRulesAmplifierSorted = new SCR_SortedArray<SCR_GroupIdentityRule>();
	
	/*!
	Update military symbol based on faction's current state.
	\param outSymbol Symbol to be updated
	\param faction Evaluated faction
	*/
	void UpdateSymbol(out notnull SCR_MilitarySymbol outSymbol, SCR_Faction faction)
	{
		SCR_GroupIdentityRuleData data = SCR_GroupIdentityRuleData.CreateData(faction);
		
		ProcessRules(m_aRulesIndentitySorted,	outSymbol, data);
		ProcessRules(m_aRulesDimensionSorted,	outSymbol, data);
		ProcessRules(m_aRulesIconSorted,		outSymbol, data);
		ProcessRules(m_aRulesAmplifierSorted,	outSymbol, data);
	}
	/*!
	Update military symbol based on group's current state.
	\param outSymbol Symbol to be updated
	\param group Evaluated group
	*/
	void UpdateSymbol(out notnull SCR_MilitarySymbol outSymbol, SCR_AIGroup group)
	{
		SCR_GroupIdentityRuleData data = SCR_GroupIdentityRuleData.CreateData(group);
		
		ProcessRules(m_aRulesIndentitySorted,	outSymbol, data);
		ProcessRules(m_aRulesDimensionSorted,	outSymbol, data);
		ProcessRules(m_aRulesIconSorted,		outSymbol, data);
		ProcessRules(m_aRulesAmplifierSorted,	outSymbol, data);
	}
	protected void ProcessRules(SCR_SortedArray<SCR_GroupIdentityRule> rules, out notnull SCR_MilitarySymbol outSymbol, SCR_GroupIdentityRuleData data)
	{
		for (int i = rules.Count() - 1; i >= 0; i--)
		{
			if (rules[i].SetSymbol(outSymbol, data))
				break;
		}
	}
	void SCR_MilitarySymbolRuleSet()
	{
		for (int i = m_aRulesIdentity.Count() - 1; i >= 0; i--)
		{
			m_aRulesIndentitySorted.Insert(m_aRulesIdentity[i].GetPriority(), m_aRulesIdentity[i]);
		}
		for (int i = m_aRulesDimension.Count() - 1; i >= 0; i--)
		{
			m_aRulesDimensionSorted.Insert(m_aRulesDimension[i].GetPriority(), m_aRulesDimension[i]);
		}
		for (int i = m_aRulesIcon.Count() - 1; i >= 0; i--)
		{
			m_aRulesIconSorted.Insert(m_aRulesIcon[i].GetPriority(), m_aRulesIcon[i]);
		}
		for (int i = m_aRulesAmplifier.Count() - 1; i >= 0; i--)
		{
			m_aRulesAmplifierSorted.Insert(m_aRulesAmplifier[i].GetPriority(), m_aRulesAmplifier[i]);
		}
	}
};
class SCR_GroupIdentityRuleData
{
	SCR_AIGroup m_Group;
	SCR_Faction m_Faction;
	EVehicleType m_VehicleTypes;
	ref map<EWeaponType, float> m_WeaponTypes = new map<EWeaponType, float>();
	ref map<EEditableEntityLabel, float> m_aLabels = new map<EEditableEntityLabel, float>();
	int m_iMemberCount;
	
	static SCR_GroupIdentityRuleData CreateData(SCR_Faction faction)
	{
		SCR_GroupIdentityRuleData data = new SCR_GroupIdentityRuleData();
		data.m_Faction = faction;
		return data;
	}
	static SCR_GroupIdentityRuleData CreateData(SCR_AIGroup group)
	{
		SCR_GroupIdentityRuleData data = new SCR_GroupIdentityRuleData();
		
		data.m_Faction = SCR_Faction.Cast(group.GetFaction());
		data.m_iMemberCount = group.GetAgentsCount();
		
		//--- Weapon types
		array<AIAgent> agents = {};
		data.m_iMemberCount = group.GetAgents(agents);
		if (data.m_iMemberCount > 0)
		{
			IEntity member;
			BaseWeaponManagerComponent weaponManager;
			array<WeaponSlotComponent> weapons = {};
			array<BaseMuzzleComponent> muzzles = {};
			array<EEditableEntityLabel> labels = {};
			data.m_WeaponTypes.Clear();
			for (int i; i < data.m_iMemberCount; i++)
			{
				if (!agents[i])
					continue;
				
				member = agents[i].GetControlledEntity();
				weaponManager = BaseWeaponManagerComponent.Cast(member.FindComponent(BaseWeaponManagerComponent));
				if (weaponManager)
				{
					weaponManager.GetWeaponsSlots(weapons);
					foreach (WeaponSlotComponent slot : weapons)
					{
						//--- Get weapon type
						EWeaponType weaponType = slot.GetWeaponType();
						if (weaponType == EWeaponType.WT_NONE)
							continue;
						
						float weaponTypeCount;
						if (data.m_WeaponTypes.Find(weaponType, weaponTypeCount))
							data.m_WeaponTypes[weaponType] = weaponTypeCount + 1;
						else
							data.m_WeaponTypes.Insert(weaponType, 1);
						
						//-- Get muzzle types (e.g., underslung grenade launcher)
						for (int m = 0, mCount = slot.GetMuzzlesList(muzzles); m < mCount; m++)
						{
							//--- Convert muzzle types to weapon types (ToDo: Not hardcoded?)
							EWeaponType muzzleWeaponType = -1;
							switch (muzzles[m].GetMuzzleType())
							{
								case EMuzzleType.MT_RPGMuzzle: 
									muzzleWeaponType = EWeaponType.WT_ROCKETLAUNCHER;
									break;
								case EMuzzleType.MT_UGLMuzzle: 
									muzzleWeaponType = EWeaponType.WT_GRENADELAUNCHER;
									break;
							}
							if (muzzleWeaponType != -1 && muzzleWeaponType != weaponType)
							{
								if (data.m_WeaponTypes.Find(muzzleWeaponType, weaponTypeCount))
									data.m_WeaponTypes[muzzleWeaponType] = weaponTypeCount + 1;
								else
									data.m_WeaponTypes.Insert(muzzleWeaponType, 1);
							}
						}
					}
				}
				
				SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(member.FindComponent(SCR_EditableEntityComponent));
				if (editableEntity)
				{
					SCR_EditableEntityUIInfo info = SCR_EditableEntityUIInfo.Cast(editableEntity.GetInfo());
					if (info)
					{
						labels.Clear();
						for (int l = 0, lCount = info.GetEntityLabels(labels); l < lCount; l++)
						{
							EEditableEntityLabel label = labels[l];
							
							float labelCount;	
							if (data.m_aLabels.Find(label, labelCount))
								data.m_aLabels[label] = labelCount + 1;
							else
								data.m_aLabels.Insert(label, 1);
						}
					}
				}
			}
			
			//--- Convert absolute numbers to ratios
			//Print("------------");
			foreach (int weaponType, float count: data.m_WeaponTypes)
			{
				data.m_WeaponTypes[weaponType] = count / data.m_iMemberCount;
				//PrintFormat("%1: %2", typename.EnumToString(EWeaponType, weaponType), m_WeaponTypes[weaponType]);
			}
			foreach (int label, float count: data.m_aLabels)
			{
				data.m_aLabels[label] = count / data.m_iMemberCount;
				//PrintFormat("%1: %2", typename.EnumToString(EEditableEntityLabel, label), m_aLabels[label]);
			}
		}
		
		//--- Vehicle types
		array<IEntity> vehicles = {};
		group.GetGroupUtilityComponent().m_VehicleMgr.GetAllVehicleEntities(vehicles);
		
		Vehicle vehicle;
		for (int i, count = vehicles.Count(); i < count; i++)
		{
			vehicle = Vehicle.Cast(vehicles[i]);
			if (vehicle)
				data.m_VehicleTypes |= vehicle.m_eVehicleType;
		}
		return data;
	}
};

[BaseContainerProps()]
class SCR_GroupIdentityRule
{
	[Attribute()]
	protected int m_iPriority;
	
	int GetPriority()
	{
		return m_iPriority;
	}
	bool SetSymbol(out notnull SCR_MilitarySymbol outSymbol, SCR_GroupIdentityRuleData data)
	{
		return false;
	}
	
	protected bool IsCompatible(notnull SCR_MilitarySymbol symbol, SCR_GroupIdentityRuleData data)
	{
		return true;
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////
//--- Identity
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EMilitarySymbolIdentity, "m_Identity")]
class SCR_GroupIdentityRuleIdentity: SCR_GroupIdentityRule
{
	[Attribute("0", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EMilitarySymbolIdentity))]
	protected EMilitarySymbolIdentity m_Identity;
	
	override bool SetSymbol(out notnull SCR_MilitarySymbol outSymbol, SCR_GroupIdentityRuleData data)
	{
		if (IsCompatible(outSymbol, data))
		{
			outSymbol.SetIdentity(m_Identity);
			return true;
		}
		return false;
	}
};
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EMilitarySymbolIdentity, "m_Identity")]
class SCR_GroupIdentityRuleIdentityFaction: SCR_GroupIdentityRuleIdentity
{
	[Attribute()]
	protected string m_sFactionKey;
	
	override protected bool IsCompatible(notnull SCR_MilitarySymbol symbol, SCR_GroupIdentityRuleData data)
	{
		return data.m_Faction && data.m_Faction.IsInherited(m_sFactionKey);
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////
//--- Dimension
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EMilitarySymbolDimension, "m_Dimension")]
class SCR_GroupIdentityRuleDimension: SCR_GroupIdentityRule
{
	[Attribute("0", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EMilitarySymbolDimension))]
	protected EMilitarySymbolDimension m_Dimension;
	
	override bool SetSymbol(out notnull SCR_MilitarySymbol outSymbol, SCR_GroupIdentityRuleData data)
	{
		if (IsCompatible(outSymbol, data))
		{
			outSymbol.SetDimension(m_Dimension);
			return true;
		}
		return false;
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////
//--- Icon
[BaseContainerProps(), SCR_BaseContainerCustomTitleFlags(EMilitarySymbolIcon, "m_Icon")]
class SCR_GroupIdentityRuleIcon: SCR_GroupIdentityRule
{
	[Attribute("0", uiwidget: UIWidgets.Flags, enums: ParamEnumArray.FromEnum(EMilitarySymbolIcon))]
	protected EMilitarySymbolIcon m_Icon;
	
	override bool SetSymbol(out notnull SCR_MilitarySymbol outSymbol, SCR_GroupIdentityRuleData data)
	{
		if (IsCompatible(outSymbol, data))
		{
			outSymbol.SetIcons(m_Icon);
			return true;
		}
		return false;
	}
};
[BaseContainerProps(), SCR_BaseContainerCustomTitleFlags(EMilitarySymbolIcon, "m_Icon")]
class SCR_GroupIdentityRuleIconWeapon: SCR_GroupIdentityRuleIcon
{
	[Attribute("0", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EWeaponType))]
	protected EWeaponType m_WeaponType;
	
	[Attribute("0.5", uiwidget: UIWidgets.Slider, params: "0 1 0.01", desc: "Condition is met only if ratio of weapons defined by 'Weapon Type' in the group is higher than this limit.\nE.g., Four-man group with two MGs will have MG ratio of 0.5\nUnarmed will never check ratio.")]
	protected float m_fMinRatio;
	
	[Attribute("0", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EWeaponType), desc: "Condition is failed if any of these weapon types have ratio bigger than the one of 'Weapon Type', even when it's above 'Min Limit'.\nUnarmed will never check ratio.")]
	protected ref array<EWeaponType> m_ComparedWeaponTypes;
	
	override protected bool IsCompatible(notnull SCR_MilitarySymbol symbol, SCR_GroupIdentityRuleData data)
	{
		//~ If check for no weapons and weapons are empty it succeeds
		if (m_WeaponType == EWeaponType.WT_NONE && data.m_WeaponTypes.IsEmpty())
			return true;
		
		//--- Check if the ratio is above limit
		float ratio;
		if (data.m_WeaponTypes.Find(m_WeaponType, ratio) && ratio >= m_fMinRatio)
		{
			//--- Check if the ratio is larger than related ratios (e.g., group of 2x MG and 2x AT is neither MG nor AT, but generic one)
			float comparedRatio;
			for (int i = m_ComparedWeaponTypes.Count() - 1; i >= 0; i--)
			{
				if (data.m_WeaponTypes.Find(m_ComparedWeaponTypes[i], comparedRatio) && comparedRatio >= ratio)
					return false;
			}
			return true;
		}
		return false;
	}
};
[BaseContainerProps(), SCR_BaseContainerCustomTitleFlags(EMilitarySymbolIcon, "m_Icon")]
class SCR_GroupIdentityRuleIconLabel: SCR_GroupIdentityRuleIcon
{
	[Attribute("0", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EEditableEntityLabel))]
	protected EEditableEntityLabel m_Label;
	
	[Attribute("0.5", uiwidget: UIWidgets.Slider, params: "0 1 0.01", desc: "Condition is met only if ratio of labels defined by 'Label' in the group is higher than this limit.\nE.g., Four-man group with two medics will have medic ratio of 0.5")]
	protected float m_fMinRatio;
	
	[Attribute("0", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EEditableEntityLabel), desc: "Condition is failed if any of these labels have ratio bigger than the one of 'Label', even when it's above 'Min Limit'")]
	protected ref array<EEditableEntityLabel> m_ComparedLabels;
	
	override protected bool IsCompatible(notnull SCR_MilitarySymbol symbol, SCR_GroupIdentityRuleData data)
	{
		//--- Check if the ratio is above limit
		float ratio;
		if (data.m_aLabels.Find(m_Label, ratio) && ratio >= m_fMinRatio)
		{
			//--- Check if the ratio is larger than related ratios (e.g., group of 2x MG and 2x AT is neither MG nor AT, but generic one)
			float comparedRatio;
			for (int i = m_ComparedLabels.Count() - 1; i >= 0; i--)
			{
				if (data.m_aLabels.Find(m_ComparedLabels[i], comparedRatio) && comparedRatio >= ratio)
					return false;
			}
			return true;
		}
		return false;
	}
};
[BaseContainerProps(), SCR_BaseContainerCustomTitleFlags(EMilitarySymbolIcon, "m_Icon")]
class SCR_GroupIdentityRuleIconVehicle: SCR_GroupIdentityRuleIcon
{
	[Attribute("0", uiwidget: UIWidgets.Flags, enums: ParamEnumArray.FromEnum(EVehicleType))]
	protected EVehicleType m_VehicleType;
	
	override protected bool IsCompatible(notnull SCR_MilitarySymbol symbol, SCR_GroupIdentityRuleData data)
	{
		return SCR_Enum.HasPartialFlag(data.m_VehicleTypes, m_VehicleType);
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////
//--- Amplifier
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EMilitarySymbolAmplifier, "m_Amplifier")]
class SCR_GroupIdentityRuleAmplifier: SCR_GroupIdentityRule
{
	[Attribute("0", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EMilitarySymbolAmplifier))]
	protected EMilitarySymbolAmplifier m_Amplifier;
	
	override bool SetSymbol(out notnull SCR_MilitarySymbol outSymbol, SCR_GroupIdentityRuleData data)
	{
		if (IsCompatible(outSymbol, data))
		{
			outSymbol.SetAmplifier(m_Amplifier);
			return true;
		}
		return false;
	}
};
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EMilitarySymbolAmplifier, "m_Amplifier")]
class SCR_GroupIdentityRuleAmplifierCount: SCR_GroupIdentityRuleAmplifier
{
	[Attribute()]
	protected int m_iMinCount;
	
	override protected bool IsCompatible(notnull SCR_MilitarySymbol symbol, SCR_GroupIdentityRuleData data)
	{
		return data.m_iMemberCount >= m_iMinCount;
	}
};