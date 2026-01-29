[EntityEditorProps(category: "GameScripted/Spectating", description: "This brief script description.", color: "0 0 255 255")]
class SCR_UnitDisplayManagerClass: GenericEntityClass
{
};

// TODO: Naming conventions
class SCR_UnitDisplaySettings
{
	// Whether shape should use faction color alpha or not
	static bool s_bShapeUseAlpha = true;
	// Whether shape should get bigger the further camera is
	static bool s_bShapeUseScaling = true;
	// Whether there should be text drawn on close proximity with a unit	
	static bool s_bShapeUseText = true;
	// Whether there should be a healthbar drawn on close proximity with a unit
	static bool s_bShapeUseHealthbar  = true;
	// Maximum distance in metres at which text will be shown (if enabled)
	static float s_fShapeTextDistance = 15.0;
	// Offset of the text from entity's origin in Y axis
	static float s_fShapeTextHeightOffset = 2.35;
	// Size of the text. (0.2 is a good value)
	static float s_fShapeTextSize = 0.1;
	// Offset of the shape from entity's origin in Y axis
	static float s_fShapeHeightOffset = 2.15;
	// Health bar width
	static float s_fShapeHealthbarWidth = 1.5;
	// Health bar height
	static float s_fShapeHealthbarHeight = 0.03;
	
	// Radius of the shape
	static float s_fShapeRadius = 0.125;
	// The distance from camera at which (and below) the shape will retain it's minimum size
	static float s_fShapeSizeGoal = 15.0;
	static float s_fShapeSizeMinimum = 1.0;
	static float s_fShapeSizeMaximum = 17.5;
	
};

//------------------------------------------------------------------------------------------------
class SCR_UnitDisplayManager : GenericEntity
{
	[Attribute("1", UIWidgets.CheckBox, "Whether shape should use alpha directly from faction color.", "")]
	bool m_bShapeUseAlpha;
	[Attribute("1", UIWidgets.CheckBox, "Whether shape should scale based on camera distance.", "")]
	bool m_bShapeUseScaling;
	[Attribute("1", UIWidgets.CheckBox, "Whether info text should be drawn when close to a unit.", "")]
	bool m_bShapeUseText;
	[Attribute("1", UIWidgets.CheckBox, "Whether health bar should be drawn when close to a unit.", "")]
	bool m_bShapeUseHealthbar;
	[Attribute("15.0", UIWidgets.Slider, "Maximum distance in metres at which text will be shown (if enabled)", "0 100 0.1")]
	float m_fShapeTextDistance;
	[Attribute("2.35", UIWidgets.Slider, "Offset of the text from entity's origin in Y axis", "0 100 0.1")]
	float m_fShapeTextHeightOffset;
	
	[Attribute("0.12", UIWidgets.Slider, "Size of the text. (0.1, 0.2 are good values example)", "0 1 0.01")]
	float m_fShapeTextSize;
	[Attribute("2.15", UIWidgets.Slider, "Offset of the shape from entity's origin in Y axis", "0 100 0.1")]
	float m_fShapeHeightOffset;

	[Attribute("1.5", UIWidgets.Slider, "Health bar width, 1.5 is a fair value", "0 10 0.01")]
	float m_fShapeHealthbarWidth;
	[Attribute("0.03", UIWidgets.Slider, "Health bar height, 0.03, 0.05 are a fair value", "0 1 0.0025")]
	float m_fShapeHealthbarHeight;
	
	[Attribute("0.125", UIWidgets.Slider, "Radius of the shape, a value of 0.125 is fine", "0 1 0.0025")]
	float m_fShapeRadius;
	
	
	[Attribute("15", UIWidgets.Slider, "The distance in metres from camera at which (and below) the shape will retain it's minimum size", "0 100 0.1")]
	float m_fShapeSizeGoal;
	[Attribute("1.0", UIWidgets.Slider, "Shape's minimum size", "0 10 0.1")]
	float m_fShapeSizeMinimum;
	[Attribute("17.5", UIWidgets.Slider, "Shape's maximum size", "0 100 0.1")]
	float m_fShapeSizeMaximum;
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		// Ugly, but guarantees this works even without a manager in the world
		// TODO: Solve in a nicer way (some sort of defaults)
		SCR_UnitDisplaySettings.s_bShapeUseAlpha = m_bShapeUseAlpha;
		SCR_UnitDisplaySettings.s_bShapeUseScaling = m_bShapeUseScaling;
		SCR_UnitDisplaySettings.s_bShapeUseText = m_bShapeUseText;
		SCR_UnitDisplaySettings.s_bShapeUseHealthbar = m_bShapeUseHealthbar;
		SCR_UnitDisplaySettings.s_fShapeTextDistance = m_fShapeTextDistance;
		SCR_UnitDisplaySettings.s_fShapeTextHeightOffset = m_fShapeTextHeightOffset;
		SCR_UnitDisplaySettings.s_fShapeTextSize = m_fShapeTextSize;
		SCR_UnitDisplaySettings.s_fShapeHeightOffset = m_fShapeHeightOffset;
		SCR_UnitDisplaySettings.s_fShapeHealthbarWidth = m_fShapeHealthbarWidth;
		SCR_UnitDisplaySettings.s_fShapeHealthbarHeight = m_fShapeHealthbarHeight;
		SCR_UnitDisplaySettings.s_fShapeRadius = m_fShapeRadius;
		SCR_UnitDisplaySettings.s_fShapeSizeGoal = m_fShapeSizeGoal;
		SCR_UnitDisplaySettings.s_fShapeSizeMinimum = m_fShapeSizeMinimum;
		SCR_UnitDisplaySettings.s_fShapeSizeMaximum = m_fShapeSizeMaximum;
	}

	//------------------------------------------------------------------------------------------------
	void SCR_UnitDisplayManager(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_UnitDisplayManager()
	{
	}

};
