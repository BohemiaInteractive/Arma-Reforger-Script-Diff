/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup World
\{
*/

class BaseTimeAndWeatherManagerEntityClass: BaseWeatherManagerEntityClass
{
}

/*!
Manager entity responsible for managing in-game time and weather,
providing the script and gamecode with usable in-game API.
*/
class BaseTimeAndWeatherManagerEntity: BaseWeatherManagerEntity
{
	//Returns true if the hour is inside the nighttime.
	proto external bool IsNightHour(float hour24);
	//Returns true if the hour is inside the daytime.
	proto external bool IsDayHour(float hour24);
	// Defines a callback that should be invoked at the specified time (format should be "HHMMSS" and TimeAndWeatherManager periodicity should be set accordingly)
	proto void SetTimeEvent(string time, Managed inst, func callback, bool delayed = true, bool singleUse = false);
	/*!
	Returns all information about current weather - rain, fog, overcast, wind.
	If swayFrequency and location are set, sway is calculated as well (works as with trees and grass).
	Mind that wetness and water accumulation effects are WIP and they will not be set yet.
	You can use
	ref LocalWeatherSituation weatherSituation = new LocalWeatherSituation();
	to create the object and pass it as an argument.
	*/
	proto external bool TryGetCompleteLocalWeather(LocalWeatherSituation lws, float swayFrequency, vector location);
}

/*!
\}
*/
