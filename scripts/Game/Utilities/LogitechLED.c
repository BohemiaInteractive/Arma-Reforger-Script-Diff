//! Controls the RGB lighting of Logitech keyboards and other peripherals.
sealed class LogitechLED
{
	private void LogitechLED();
	private void ~LogitechLED();
	
	//! Returns an instance of LogitechLED, or nullptr if not initialized.
	static proto native LogitechLED Get();

	/*!
	Sets the lighting on connected and supported devices.
	\param red Amount of red (0–100)
	\param green Amount of green (0–100)
	\param blue Amount of blue (0–100)
	*/
	native proto void SetLighting(int red, int green, int blue);
	
	/*!
	Saves the current lighting so it can be restored after a temporary effect
	is finished.
	\return False if initialization failed or there is no connection with
	Logitech Gaming Software.
	*/
  native proto bool SaveCurrentLighting();
	
	/*!
	Restores the last saved lighting. It should be called after a temporary
	effect is finished.
	\return False if initialization failed or there is no connection with
	Logitech Gaming Software.
	*/
	native proto bool RestoreLighting();
	
	/*!
	Stops any currently running effects (started from FlashLighting or
	PulseLighting).
	\return False if initialization failed or there is no connection with
	Logitech Gaming Software.
	*/
	native proto bool StopEffects();
	
	/*!
	Saves the current lighting, plays the flashing effect on the targeted
	devices, and finally restores the saved lighting.
	\param red Amount of red (0–100)
	\param green Amount of green (0–100)
	\param blue Amount of blue (0–100)
	\param msDuration Duration of the effect in milliseconds. Can be set to 0 to
	make the effect run until stopped through StopEffects().
	\param msInterval Duration of the flashing interval in milliseconds.
	\return False if initialization failed or there is no connection with
	Logitech Gaming Software.
	*/
	native proto bool FlashLighting(
		int red,
		int green,
		int blue,
		int msDuration,
		int msInterval
	);
	
	/*!
	Saves the current lighting, plays the pulsing effect on targeted devices,
	and finally restores the saved lighting.
	\param red Amount of red (0–100)
	\param green Amount of green (0–100)
	\param blue Amount of blue (0–100)
	\param msDuration Duration of the effect in milliseconds. Can be set to 0 to
	make the effect run until stopped through StopEffects().
	\param msInterval Duration of the pulsing interval in milliseconds.
	\return False if initialization failed or there is no connection with
	Logitech Gaming Software.
	*/
	native proto bool PulseLighting(
		int red,
		int green,
		int blue,
		int msDuration,
		int msInterval
	);
	
	/*!
	Sets the key identified by the given name to the desired color. This
	function affects only per-key backlighting on supported connected devices.
	\param keyName Enfusion key name.
	\param red Amount of red (0–100).
	\param green Amount of green (0–100).
	\param blue Amount of blue (0–100).
	\return False if an invalid key name was passed, initialization failed, or
	there is no connection with Logitech Gaming Software.
	*/
	native proto bool SetLightingForKey(
		string keyName,
		int red,
		int green,
		int blue
	);
	
	/*!
	Saves the current color on the key specified by the name passed as an
	argument. Use this function with RestoreLightingForKey to preserve the
	state of a key before applying any effect.
	\param keyName Enfusion key name.
	\return False if an invalid key name was passed, initialization failed, or
	there is no connection with Logitech Gaming Software.
	*/
	native proto bool SaveLightingForKey(string keyName);
	
	/*!
	Restores the saved color on the key specified by the name passed as an
	argument. Use this function with SaveLightingForKey to preserve the state
	of a key before applying any effect.
	\param keyName Enfusion key name.
	\return False if an invalid key name was passed, initialization failed, or
	there is no connection with Logitech Gaming Software.
	*/
	native proto bool RestoreLightingForKey(string keyName);
	
	/*!
	Starts a flashing effect on the key specified by the parameter. The key
	will flash with an interval defined by msInterval for msDuration
	milliseconds, alternating between the specified color and black. This
	function affects only per-key backlighting on supported connected devices.
	\param keyName Enfusion key name.
	\param red Amount of red (0–100).
	\param green Amount of green (0–100).
	\param blue Amount of blue (0–100).
	\param msDuration Duration of the effect in milliseconds. Can be set to 0
	to make the effect run until stopped through StopEffects() or
	StopEffectsOnKey().
	\param msInterval Duration of the flashing interval in milliseconds.
	\return False if an invalid key name was passed, initialization failed, or
	there is no connection with Logitech Gaming Software.
	*/
	native proto bool FlashSingleKey(
		string keyName,
		int red,
		int green,
		int blue,
		int msDuration,
		int msInterval
	);
	
	/*!
	Starts a pulsing effect on the key specified by the parameter. The key
	will pulse from the start color to the finish color for msDuration
	milliseconds. This function affects only per-key backlighting on supported
	connected devices.
	\param keyName Enfusion key name.
	\param startRed Amount of red (0–100).
	\param startGreen Amount of green (0–100).
	\param startBlue Amount of blue (0–100).
	\param finishRed Amount of red (0–100).
	\param finishGreen Amount of green (0–100).
	\param finishBlue Amount of blue (0–100).
	\param msDuration Duration of the effect in milliseconds.
	\param isInfinite If set to true, the effect will loop infinitely until
	stopped with a call to StopEffects() or StopEffectsOnKey().
	\return False if an invalid key name was passed, initialization failed, or
	there is no connection with Logitech Gaming Software.
	*/
	native proto bool PulseSingleKey(
		string keyName,
		int startRed,
		int startGreen,
		int startBlue,
		int finishRed,
		int finishGreen,
		int finishBlue,
		int msDuration,
		bool isInfinite
	);
	
	/*!
	Stops any ongoing effect on the key specified by the parameter. This
	function affects only per-key backlighting on supported connected devices.
	\param keyName Enfusion key name.
	\return False if an invalid key name was passed, initialization failed, or
	there is no connection with Logitech Gaming Software.
	*/
	native proto bool StopEffectsOnKey(string keyName);
}
