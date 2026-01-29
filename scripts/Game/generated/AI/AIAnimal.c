/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup AI
\{
*/

class AIAnimalClass: AIAgentClass
{
}

class AIAnimal: AIAgent
{
	proto external int GetSoundMapTag();
	//A danger event happened in server, this calls to "ReactDanger" in clients. Returns false if it can not spawn/activate
	proto external bool CallReactToDanger();
	proto external bool ManualActivation();

	// callbacks

	//What effects should play when scared (for example, playing sounds, particles or animations)
	event void OnReactToDanger();
}

/*!
\}
*/
