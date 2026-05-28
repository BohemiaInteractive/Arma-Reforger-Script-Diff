/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Entities
\{
*/

class EnvironmentProbeEntityClass: VolumeReceiverEntityClass
{
}

class EnvironmentProbeEntity: VolumeReceiverEntity
{
	/*!
	Set new probe position
	\param position
	\param invalidate 	invalidate probe data to capture them again
	*/
	proto external void SetProbePosition(vector position, bool invalidate);
	/*!
	capture probe
	*/
	proto external void Recapture();
	/*!
	Set user EV applied for both lighting parts, dynamically applied when rendering
	\param EV					new probe EV in range <-20, 20>
	\param absolute		if the EV value is absolute (aka LV) or relative to global ambient (sky)

	*/
	proto external void EnableUserEV(float EV, bool absolute);
	/*!
	Disable user EV and use automatic
	*/
	proto external void DisableUserEV();
	/*!
	Get user EV from probe actual setting
	*/
	proto external float GetUserEV(float EV);
	/*!
	Set EV for reflection, this changes the probe definition part
	\param EV	 0 is original probe value
	*/
	proto external void SetReflectionEV(float EV);
	/*!
	Set EV for difuse part, this changes the probe definition part
	\param EV		0 is original probe value
	*/
	proto external void SetDiffuseEV(float EV);
	/*!
	Get number of probe's volumes
	*/
	proto external int GetVolumesCount();
	/*!
	Get index of volume according the given name, negative value if it doesn't exist
	\param volumeName
	*/
	proto external int GetVolumeIndex(string volumeName);
	/*!
	Get name of volume according the given index, empty string if it doesn't exist
	\param index
	*/
	proto external string GetVolumeName(int index);
	/*!
	enable volume
	\param index
	\param On			true = switch it on, false = switch it off
	*/
	proto external void EnableVolume(int index, bool On);
	/*!
	Is volume enabled ?
	\param index
	*/
	proto external bool IsVolumeEnabled(int index);
}

/*!
\}
*/
