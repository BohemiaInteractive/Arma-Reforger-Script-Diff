/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Radio
\{
*/

class BaseTransceiver: ScriptAndConfig
{
	proto external int GetFrequency();
	/*!
	Returns maximum tunable frequency of the radio
	\return Frequency in KHz.
	*/
	proto external int GetMinFrequency();
	/*!
	Returns minimum tunable frequency of the radio
	\return Frequency in KHz.
	*/
	proto external int GetMaxFrequency();
	/*!
	Sets frequency. Supports proxies and server.
	\param[in] freq Frequency in KHz.
	*/
	proto external void SetFrequency(int freq);
	/*!
	Frequency resolution for all transceivers.
	\return Frequency in KHz.
	*/
	proto external int GetFrequencyResolution();
	/*!
	Returns true if the transceiver is muted.
	*/
	proto external bool IsMuted();
	/*!
	Toggles the mute state of the transceiver.
	*/
	proto external bool SetMuteState(bool state);
	/*!
	Gets maximum transmitting range of radio.
	\return Distance in meters.
	*/
	proto external float GetRange();
	/*!
	Sets maximum transmitting range of radio.
	Has effect only on owner (server).
	\param range: Distance in meters. Must be > 1m otherwise 1m is used.
	*/
	proto external void SetRange(float range);
	proto external BaseRadioComponent GetRadio();
	//! Makes transmission of message on tuned frequency
	proto external void BeginTransmission(BaseRadioMessage message);
	//! Makes transmission of message on given frequency (kHz), ignoring the tuned one
	proto external void BeginTransmissionFreq(BaseRadioMessage message, int frequency);
}

/*!
\}
*/
