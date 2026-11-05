//! Commands config root
[BaseContainerProps(configRoot: true)]
class SCR_RallyVehicleCustomizationConfig
{
	[Attribute("", UIWidgets.Object, "Vehicles")]
	protected ref array<ref SCR_RallyVehicleCustomizationData> m_sVehicles;

	[Attribute("", UIWidgets.Object, "Character customization")]
	protected ref array<ref SCR_RallyCharacterCustomizationSlot> m_sCharacterCustomizationOptions;

	//------------------------------------------------------------------------------------------------
	array<ref SCR_RallyVehicleCustomizationData> GetVehicles()
	{
		return m_sVehicles;
	}

	//------------------------------------------------------------------------------------------------
	array<ref SCR_RallyCharacterCustomizationSlot> GetCharacterCustomizationOptions()
	{
		return m_sCharacterCustomizationOptions;
	}
}

[BaseContainerProps(), BaseContainerCustomTitleField("m_sVehicleTypeName")]
class SCR_RallyVehicleCustomizationData
{
	[Attribute()]
	string m_sVehicleFullName;

	[Attribute()]
	string m_sVehicleBrandName;

	[Attribute()]
	string m_sVehicleTypeName;

	[Attribute()]
	string m_sVehicleDescription;

	[Attribute(defvalue:"", uiwidget: UIWidgets.ResourceNamePicker, params: "et", desc: "Vehicle this config applies to")]
	protected ResourceName m_sVehicle;

	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, params: "et", desc: "Available prefabs of different variants of same vehicle")]
	ref array<ResourceName> m_sVehicleVariants;

	[Attribute()]
	protected ref array<ref SCR_RallyPartCustomizationData> m_aParts;

	[Attribute()]
	protected ref SCR_RallyVehicleVisualAttributes m_visualAttributes;

	//------------------------------------------------------------------------------------------------
	array<ref SCR_RallyPartCustomizationData> GetParts()
	{
		return m_aParts; // TODO: return pointer array
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetVehicleResourceName()
	{
		return m_sVehicle;
	}

	//------------------------------------------------------------------------------------------------
	SCR_RallyVehicleVisualAttributes GetVisualAttributes()
	{
		return m_visualAttributes;
	}
}

[BaseContainerProps()]
class SCR_RallyPartCustomizationData
{
	[Attribute("0", UIWidgets.ComboBox, enumType: SCR_ERallyCustomizationParts)]
	SCR_ERallyCustomizationParts partType;

	[Attribute(defvalue:"", uiwidget: UIWidgets.ResourceNamePicker, params: "et", desc: "Available prefabs for given part")]
	ref array<ResourceName> m_aAvailableParts;

	//------------------------------------------------------------------------------------------------
	SCR_ERallyCustomizationParts GetPartType()
	{
		return partType;
	}
}

[BaseContainerProps(), BaseContainerCustomTitleField("slotName")]
class SCR_RallyCharacterCustomizationSlot
{
	[Attribute(desc: "System name of the slot as defined in characters loadout manager")]
	protected string slotName;

	[Attribute()]
	protected ref array<ref SCR_RallyCharacterCustomizationPart> m_aAvailableParts;

	//------------------------------------------------------------------------------------------------
	string GetSlotName()
	{
		return slotName;
	}

	//------------------------------------------------------------------------------------------------
	array<ref SCR_RallyCharacterCustomizationPart> GetAvailableParts()
	{
		return m_aAvailableParts;
	}
}

[BaseContainerProps(), BaseContainerCustomTitleField("partDisplayName")]
class SCR_RallyCharacterCustomizationPart
{
	[Attribute()]
	string partDisplayName;

	[Attribute(defvalue:"", uiwidget: UIWidgets.ResourceNamePicker, params: "et", desc: "Available prefabs for given slot")]
	ResourceName m_sPartPrefab;

	//------------------------------------------------------------------------------------------------
	string GetPartDisplayName()
	{
		return partDisplayName;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetPartPath()
	{
		return m_sPartPrefab;
	}
}

[BaseContainerProps()]
class SCR_RallyVehicleVisualAttributes
{
	[Attribute("5", UIWidgets.Slider, "Speed attribute displayed in vehicle selection rated 1 to 10", "1 10 1")]
	protected int m_iSpeed;

	[Attribute("5", UIWidgets.Slider, "Acceleration attribute displayed in vehicle selection rated 1 to 10", "1 10 1")]
	protected int m_iAcceleration;

	[Attribute("5", UIWidgets.Slider, "Handling attribute displayed in vehicle selection rated 1 to 10", "1 10 1")]
	protected int m_iHandling;

	[Attribute("5", UIWidgets.Slider, "Weight attribute displayed in vehicle selection rated 1 to 10", "1 10 1")]
	protected int m_iWeight;

	//------------------------------------------------------------------------------------------------
	int GetSpeed()
	{
		return m_iSpeed;
	}

	//------------------------------------------------------------------------------------------------
	int GetAcceleration()
	{
		return m_iAcceleration;
	}

	//------------------------------------------------------------------------------------------------
	int GetHandling()
	{
		return m_iHandling;
	}

	//------------------------------------------------------------------------------------------------
	int GetWeight()
	{
		return m_iWeight;
	}
}
