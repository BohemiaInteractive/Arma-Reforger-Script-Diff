/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\brief By inheriting of this class you define a settings module.
\code
// my settings module definition
class MyGameSettings: ModuleGameSettings
{
	[Attribute()]
	int speed;
}

// write my settings (ideally in setting menu?)
void ChangeMySettings()
{
	// approach #1
	GetGame().GetGameUserSettings().GetModule("MyGameSettings").Set("speed", Math.RandomInt(0, 100));

	// approach #2
	MyGameSettings settings = new MyGameSettings();
	settings.speed = Math.RandomInt(0, 100);
	BaseContainerTools.ReadFromInstance(settings, GetGame().GetGameUserSettings().GetModule("MyGameSettings"));

	// notify system about change
	GetGame().UserSettingsChanged(); // -> here is also OnSpeedChanged() called
	GetGame().SaveUserSettings(); // this is also call automatically during game exit, call it manually only on very important cases (like leaving settings menu)
}

// register to receive notification about changes
void SomewhereInInit()
{
	GetGame().OnUserSettingsChangedInvoker().Insert(OnSpeedChanged);
}

// read my settings
void OnSpeedChanged()
{
	// approach #1
	int speed;
	GetGame().GetGameUserSettings().GetModule("MyGameSettings").Get("speed", speed);
	Print("#1 Speed changed = " + speed);

	// approach #2
	MyGameSettings settings = new MyGameSettings();
	BaseContainerTools.WriteToInstance(settings, GetGame().GetGameUserSettings().GetModule("MyGameSettings"));
	Print("#2 Speed changed = " + settings.speed);
}
\endcode
*/
class ModuleGameSettings: ScriptAndConfig
{
}
