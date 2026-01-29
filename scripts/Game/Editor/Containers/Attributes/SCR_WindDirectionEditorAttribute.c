/**
Wind direction Attribute for getting and setting varriables in Editor Attribute window
*/
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_WindDirectionEditorAttribute: SCR_BaseFloatValueHolderEditorAttribute
{	
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{		
		//If opened in global attributes
		if (!IsGameMode(item)) 
			return null;
		
		GenericEntity ent = GenericEntity.Cast(item);
		ChimeraWorld world = ent.GetWorld();
		TimeAndWeatherManagerEntity weatherManager = world.GetTimeAndWeatherManager();
		if (!weatherManager) 
			return null;

		int CurrentWindDirectionIndex;
		SCR_WindDirectionInfo windDirectionInfo;
		
		weatherManager.GetWindDirectionInfoFromFloat(weatherManager.GetWindDirection(), CurrentWindDirectionIndex, windDirectionInfo);
		return SCR_BaseEditorAttributeVar.CreateInt(CurrentWindDirectionIndex);
	}
	
	
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var) 
			return;
		
		GenericEntity ent = GenericEntity.Cast(item);
		ChimeraWorld world = ent.GetWorld();
		TimeAndWeatherManagerEntity weatherManager = world.GetTimeAndWeatherManager();
		if (!weatherManager) 
			return;
		
		int windDirectionIndex = var.GetInt();
		int CurrentWindDirectionIndex;
		SCR_WindDirectionInfo windDirectionInfo;
		weatherManager.GetWindDirectionInfoFromFloat(weatherManager.GetWindDirection(), CurrentWindDirectionIndex, windDirectionInfo);
		
		//Wind did not change
		if (CurrentWindDirectionIndex == windDirectionIndex)
			return;
		
		weatherManager.GetWindDirectionInfoFromIndex(windDirectionIndex, windDirectionInfo);
		weatherManager.DelayedOverrideWindDirection(windDirectionInfo.GetWindDirectionValue(), playerID);
	}
	
	protected void CreatePresets()
	{
		ChimeraWorld world = GetGame().GetWorld();
		TimeAndWeatherManagerEntity timeManager = world.GetTimeAndWeatherManager();
		if (!timeManager) 
			return;
		
		m_aValues.Clear();
		
		array<SCR_WindDirectionInfo> orderedWindDirectionInfo = new array<SCR_WindDirectionInfo>;
		timeManager.GetOrderedWindDirectionInfoArray(orderedWindDirectionInfo);
		SCR_EditorAttributeFloatStringValueHolder value;
		
		foreach (SCR_WindDirectionInfo windDirectionInfo: orderedWindDirectionInfo)
		{
			value = new SCR_EditorAttributeFloatStringValueHolder();
			value.SetName(windDirectionInfo.GetUIInfo().GetName());
			value.SetFloatValue(windDirectionInfo.GetWindDirectionValue());
			
			m_aValues.Insert(value);
		}
	}
	
	//Call PreviewVariable of SCR_WindAutomaticEditorAttribute
	override void PreviewVariable(bool setPreview, SCR_AttributesManagerEditorComponent manager)
	{
		if (!setPreview)
			return;
		
		SCR_BaseEditorAttribute overrideAttribute = manager.GetAttributeRef(SCR_WindAutomaticEditorAttribute);
		if (!overrideAttribute)
			return;
		
		overrideAttribute.PreviewVariable(setPreview, manager);
	}

	override int GetEntries(notnull array<ref SCR_BaseEditorAttributeEntry> outEntries)
	{
		CreatePresets();		
		//outEntries.Insert(new SCR_EditorAttributeEntryOverride(weatherManager.IsWindDirectionOverridden(), SCR_WindDirectionOverrideEditorAttribute));
		return super.GetEntries(outEntries);
	}
};