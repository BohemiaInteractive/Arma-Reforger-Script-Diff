enum SCR_ScenarioFrameworkEActivationType
{
	SAME_AS_PARENT = 0,
	ON_TRIGGER_ACTIVATION,
	ON_AREA_TRIGGER_ACTIVATION,
	ON_INIT,						//when the game mode is initiated
	ON_TASKS_INIT,					//when the  game mode starts creating tasks
	CUSTOM1,						//won't spawn until something will try to spawn the object with CUSTOM as parameter
	CUSTOM2,
	CUSTOM3,
	CUSTOM4,
}