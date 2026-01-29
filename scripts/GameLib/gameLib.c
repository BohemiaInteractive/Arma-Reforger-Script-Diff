typedef int PlayerId;

//! \deprecated Use WorldSystem instead.
typedef WorldSystem BaseSystem;

//! \deprecated Use WorldSystemPoint instead.
typedef WorldSystemPoint ESystemPoint;

//! \deprecated Use WorldSystemLocation instead.
typedef WorldSystemLocation ESystemLocation;

//! Game singleton instance
Game g_Game;

/**
	\brief Enum is filled by C++ by data in project config PhysicsSettings.LayerPresets
*/
enum EPhysicsLayerPresets
{
	Main,
	Cover,
	Character,
	Projectile,
	Vehicle,
	VehicleCast,
	VehicleFire,
	BasicVehicleObstacles,
	VehicleFireView,
	Building,
	BuildingNoNavmesh,
	BuildingView,
	BuildingViewNoNavmesh,
	BuildingFire,
	BuildingFireNoNavmesh,
	BuildingFireView,
	BuildingFireViewNoNavmesh,
	ItemFireView,
	DoorFireView,
	Weapon,
	WeaponFire,
	Terrain,
	TreeFireView,
	CharNoCollide,
	FireGeo,
	RockFireView,
	Debris,
	Tree,
	Door,
	TreePart,
	Interaction,
	Ladder,
	Bush,
	Foliage,
	Wheel,
	PropFireView,
	PropFireViewNoNavmesh,
	PropView,
	PropViewNoNavmesh,
	Prop,
	PropNoNavmesh,
	CharacterAI,
	Glass,
	GlassFire,
	FireView,
	ViewGeo,
	VehicleComplex,
	VehicleSimple,
	VehicleRotorDisc,
	CharacterFireGeoRagdoll,
	InteractionFireGeo,
	MineTrigger,
	MineTriggerFire,
	MineTriggerComplex,
	Liquids
}

/**
	\brief Enum is filled by C++ by data in project config PhysicsSettings.Layers
*/
enum EPhysicsLayerDefs
{
	Default,
	Static,
	VehicleCast,
	Cover,
	Character,
	Projectile,
	Vehicle,
	Terrain,
	Dynamic,
	Debris,
	Ragdoll,
	Vegetation,
	CharNoCollide,
	Camera,
	FireGeometry,
	ViewGeometry,
	Ladder,
	Interaction,
	Foliage,
	CharCollide,
	CharacterAI,
	Navmesh,
	Water,
	NavmeshVehicle,
	VehicleSimple,
	VehicleComplex,
	Unused,
	Mine,
	Weapon
}

enum ETransformMode
{
	WORLD, ///< Set world transformation
	LOCAL, ///< Set local transformation
	OFFSET ///< Offset actual transformation
}

/**
	\brief Additional parameters for entity spawning
*/
class EntitySpawnParams
{
	ETransformMode TransformMode;
	vector Transform[4];
	IEntity Parent;
	float Scale;
	
	void EntitySpawnParams()
	{
		TransformMode = ETransformMode.LOCAL;
		Math3D.MatrixIdentity4(Transform);
		Scale = 1;
	}
}

/*!
Interface for classes implementing animation or progress bar of the loading
screen. To customize the implementation, override Game::CreateLoadingAnim()
function to return your own implementation of LoadingAnim. You should
perform any expensive setup, such as loading UI layout, in the constructor
to avoid freezes due to loading of resources when game shows the loading
screen.

\warning	Loading and initialization of resources are done asynchronously.
			It is generally safe to access only types related to
			WorkspaceWidget stored in LoadingAnim (such as child widgets
			loaded using this WorkspaceWidget) and other variables that
			cannot be changed from somewhere else (such as local variables
			and member variables that are primitive types).
*/
class LoadingAnim
{
	WorkspaceWidget m_WorkspaceWidget;

	void LoadingAnim(WorkspaceWidget workspaceWidget)
	{
		m_WorkspaceWidget = workspaceWidget;
	}

	/*!
	Called just before loading animation is shown. You shouldn't perform any
	expensive operations here (such as loading images and UI layouts) to avoid
	freezes when loading screen is shown.

	\warning	This function is called concurrently with loading and
				initialization of resources (including scripted entities and
				components). Do not access anything that may be modified from
				somewhere else.
	*/
	void Show()
	{
	}

	/*!
	Used for updating the loading screen animations and progress bars.

	Progress value is based on how many resources are still left in loading queue. However, loading
	new resources can cause more things to be added to the queue, so this value can jump up and down
	and these cases need to be handled, otherwise progress may appear to go backwards.

	Some platforms require that loading screen is visible for at least certain minimum amount of time.
	To account for this, minDurationRatio provides percentage of this minimum duration that has already
	elapsed.

	\warning	This function is called concurrently with loading and initialization
				of resources (including scripted entities and components). Do not access
				anything that may be modified somewhere else.

	@param timeSlice		Time slice in seconds.
	@param progress			Unmodified loading progress value between 0 and 1. This value may decrease.
	@param minDurationRatio	Ratio of minimal duration that has already elapsed, between 0 and 1. This value never decreases.
	*/
	void Update(float timeSlice, float progress, float minDurationRatio)
	{
	}

	/*!
	Called when loading animation has finished and should be hidden.

	\warning	This function is called concurrently with loading and
				initialization of resources (including scripted entities and
				components). Do not access anything that may be modified from
				somewhere else.
	*/
	void Hide()
	{
	}
}

class BaseLoadingAnim: LoadingAnim
{
	protected const float MAX_ANGLE = 360;
	protected static int s_NumReloads;  ///< Sets from engine side, number is increased after each game reinitialization  
	protected Widget m_wRoot;
	protected ImageWidget m_wProgressImage;
	static protected float s_fProgress = 0;
	
	void Load()
	{
		m_wRoot = CreateLayout(m_WorkspaceWidget);
		m_wProgressImage = ImageWidget.Cast(m_wRoot.FindAnyWidget("Throbber"));
	}
	
	//------------------------------------------------------------------------------------------------
	void Unload()
	{
		delete m_wRoot;
	}
	
	//------------------------------------------------------------------------------------------------
	void BaseLoadingAnim(WorkspaceWidget workspaceWidget)
	{
	}
	
	//------------------------------------------------------------------------------------------------
	void ~BaseLoadingAnim()
	{
		Unload();
	}
	
	//------------------------------------------------------------------------------------------------
	Widget CreateLayout(WorkspaceWidget workspaceWidget)
	{
		return workspaceWidget.CreateWidgets("{60A59D01C794CF8F}ui/baseLoadingScreen.layout");
	}
	
	//------------------------------------------------------------------------------------------------
	override void Update(float timeSlice, float progress, float minDurationRatio)
	{
		if (!m_wProgressImage)
			return;
	
		s_fProgress += timeSlice * MAX_ANGLE * 0.2;
		if (s_fProgress >= MAX_ANGLE)
			s_fProgress -= MAX_ANGLE;

		m_wProgressImage.SetRotation(s_fProgress);
	}
	
	override void Show()
	{
		if (!m_wRoot)
			Load();
	}
	
	override void Hide()
	{
		Unload();
	}
}

void GameLibInit()
{

}

//------------------------------------------------------------------------------------------------
//! Default string table item
class DefaultStringTableItem: ScriptStringTableItem
{
	static override string GetTargetPrefix() { return "Target_"; }
	
	[Attribute(category:"Default", desc: "English (United States)\nServes as a source for translated texts.", uiwidget: UIWidgets.EditBoxWithButton)]
	string Target_en_us;
}

