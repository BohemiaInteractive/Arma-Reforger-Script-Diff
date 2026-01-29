/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup AI
\{
*/

class AIFlockClass: AIAnimalClass
{
}

class AIFlock: AIAnimal
{
	proto external void ActivateAllBirds(bool val);
	proto external vector CreateRandomBirdTarget(int i);
	//For the individual birds, what will be its center, this is used as the starting position as well as for some calculations
	proto external vector CreateRandomBirdCenter();

	// callbacks

	//Function called when the flock takes off
	event void OnTakeOff();
	//Function called when the flock starts the flying animation
	event void OnFlying();
	//Function called when the flock starts landing
	event void OnLanding();
	//Function called when the flock finished landing
	event void OnFinishedLanding();
}

/*!
\}
*/
