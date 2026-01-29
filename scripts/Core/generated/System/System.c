/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup System
\{
*/

sealed class System
{
	private void System();
	private void ~System();

	//! Switches memory validation (huge slowdown! Use with care only for certain section of code!)
	static proto void MemoryValidation(bool enable);
	//! Number of allocations of physical memory.
	static proto int MemoryAllocationCount();
	//! Total allocation of physical memory in kilobytes.
	static proto int MemoryAllocationKB();
	/*!
	Returns command line argument.
	\param param Name of a command line argument
	\param[out] val Value of the param or empty string if the param was not found
	\return True if param is present, False if it was not found
	\code
		string param;
		System.GetCLIParam("world", param); // return a value when program executed with param -world something
	\endcode
	*/
	static proto bool GetCLIParam(string param, out string val);
	/*!
	Returns whether command line parameter is present.
	\param paramName Name of the command line parameter.
	\return True if param is present, False otherwise.
	\code
		if (System.IsCLIParam("verbose")) // Prints "something" when program executed with param -verbose
		{
			Print("something");
		}
	\endcode
	*/
	static proto bool IsCLIParam(string paramName);
	//! Copy text from clipboard (works only on PC)
	static proto string ImportFromClipboard();
	//! Set text to clipboard (works only on PC)
	static proto void ExportToClipboard(string text);
	static proto string GetProfileName();
	static proto string GetMachineName();
	//! Obtain value of SystemTime as number of seconds since midnight 1970-1-1 UTC.
	static proto int GetUnixTime();
	/*!
	Returns current system time (real world time) in local timezone.
	\code
		int hour = 0;
		int minute = 0;
		int second = 0;

		System.GetHourMinuteSecond(hour, minute, second);

		Print(hour);
		Print(minute);
		Print(second);

		>> hour = 16
		>> minute = 38
		>> second = 7
	\endcode
	*/
	static proto void GetHourMinuteSecond(out int hour, out int minute, out int second);
	/*!
	Returns current system time (real world time) in UTC.
	\code
		int hour = 0;
		int minute = 0;
		int second = 0;

		System.GetHourMinuteSecondUTC(hour, minute, second);

		Print(hour);
		Print(minute);
		Print(second);

		>> hour = 15
		>> minute = 38
		>> second = 7
	\endcode
	*/
	static proto void GetHourMinuteSecondUTC(out int hour, out int minute, out int second);
	/*!
	Returns current system date (real world date) in local timezone.
	\code
		int year = 0;
		int month = 0;
		int day = 0;

		System.GetYearMonthDay(year, month, day);

		Print(year);
		Print(month);
		Print(day);

		>> year = 2015
		>> month = 3
		>> day = 24
	\endcode
	*/
	static proto void GetYearMonthDay(out int year, out int month, out int day);
	/*!
	Returns current system date (real world date) in UTC.
	\code
		int year = 0;
		int month = 0;
		int day = 0;

		System.GetYearMonthDayUTC(year, month, day);

		Print(year);
		Print(month);
		Print(day);

		>> year = 2015
		>> month = 3
		>> day = 24
	\endcode
	*/
	static proto void GetYearMonthDayUTC(out int year, out int month, out int day);
	static proto ProfileData GetProfileData();
	/*!
	\brief Returns number of milliseconds that have elapsed since the game was started.
	*/
	static proto int GetTickCount(int prev = 0);
	static proto string GetAdapterName();
	static proto void GetNativeResolution(out int width, out int height);
	static proto void GetRenderingResolution(out int width, out int height);
	static proto void GetSupportedResolutions(out notnull array<int> widths, out notnull array<int> heights);
	/*!
	Takes screenshot and stores it in a BMP format at specified path.
	\param path Path where screenshot should be written. If you use absolute path or $NamedFileSystem:
							path, it will be used directly without any changes. If you use relative path, it will be
							relative to $profile: file system. Finally, if path is empty string, screenshot will be
							saved to "$profile:ScreenShots/DATETIME.bmp" where DATETIME will be replaced by current
							date and time.
	\return			true if file can be written
	*/
	static proto bool MakeScreenshot(string path);
	/*!
	Take screenshot on specified position and size. dstWidth x dstHeight point to final image size,
	if they are zero then they equal to srcWidth x srcHeight. The callback receives raw image data
	which are valid in the time of callback call only
	\param scriptCallback	callback to script function which receives the data
	\param posX						X position of screenshot
	\param posY						Y position of screenshot
	\param srcWidth				width of image to be taken
	\param srcHeight			height of image to be taken
	\param dstWidth				width of destination image, 0 means the srcWidth is used
	\param dstHeight			height of destination image, 0 means the srcHeight is used
	*/
	static proto void MakeScreenshotRawData(ScreenshotRawDataCallback scriptCallback, int posX, int posY, int srcWidth, int srcHeight, int dstWidth, int dstHeight);
	/*!
	Take screenshot on specified position and size. dstWidth x dstHeight point to final image size,
	if they are zero then they equal to srcWidth x srcHeight. The callback receives a full texture
	which can be used for any purpose
	\param scriptCallback	callback to script function which receives the data
	\param posX						X position of screenshot
	\param posY						Y position of screenshot
	\param srcWidth				width of image to be taken
	\param srcHeight			height of image to be taken
	\param dstWidth				width of destination image, 0 means the srcWidth is used
	\param dstHeight			height of destination image, 0 means the srcHeight is used
	*/
	static proto void MakeScreenshotTexture(ScreenshotTextureCallback scriptCallback, int posX, int posY, int srcWidth, int srcHeight, int dstWidth, int dstHeight);
	/*!
	Returns actual fps (average in last 10 frames)
	*/
	static proto float GetFPS();
	/*!
	Returns average time a frame took in seconds (average over last 10 frames)
	*/
	static proto float GetFrameTimeS();
	/*!
	Return current platform
	*/
	static proto EPlatform GetPlatform();
	/*!
	Checks if the app runs in console mode (no rendering, audio or input)
	@return True if the app runs in console mode. False otherwise.
	*/
	static proto bool IsConsoleApp();
	/*!
	get final image contrast
	*/
	static proto float GetFinalImageContrast();
	/*!
	get final image brightness
	*/
	static proto float GetFinalImageBrightness();
	/*!
	get final image gamma
	*/
	static proto float GetFinalImageGamma();
	/*!
	set final image attributes
	\param gamma			gamma of image
	\param brightness	brightness of image
	\param contrast		contrast of image
	*/
	static proto float SetFinalImageAttributes(float gamma, float brightness, float contrast);
}

/*!
\}
*/
