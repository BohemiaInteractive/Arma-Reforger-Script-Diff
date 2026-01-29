class SCR_HeatmapHelper
{
	
	static const int COLOUR_MODE_GREYSCALE = 0;
	static const int COLOUR_MODE_THERMAL = 1;
	static const int COLOUR_MODE_ALPHA = 2;

	static const int MAX_MODE_RAW = 0;
	static const int MAX_MODE_2AVG = 1;
	static const int MAX_MODE_2MEDIAN = 2;

	static const int MAX_RESOLUTION = 8192;

	protected static const int MIN_GRADIENT_RGB = 8;

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] imagePath
	//! \param[in] imageData array values in range -0..maxToUse+ - resolution is deduced from this value (image is expected to be square)
	//! \param[in] colourMode
	//! \param[in] maxValueMode
	//! \param[in] highlightValuesAboveMax
	//! \param[in] resolutionFactor
	//! \return true on success, false otherwise
	// TODO: maxRaw/maxAvg/maxMedian here
	static bool CreateHeatmapImageFromData(string imagePath, notnull array<int> imageData, int colourMode = COLOUR_MODE_GREYSCALE, bool invertColour = false, int maxValueMode = MAX_MODE_RAW, bool highlightValuesAboveMax = false, int resolutionFactor = 1)
	{
		int imageDataCount = imageData.Count();
		float resolution = Math.Sqrt(imageDataCount);
		if (resolution != (int)resolution)
		{
			int idealResolution = Math.Round(resolution);
			if (idealResolution < 1)
				idealResolution = 1;

			PrintFormat("Invalid image data: resolution is not square (√%1 ≈ %2, expected a perfect int e.g %3×%3 = %4)", imageDataCount, resolution, idealResolution, idealResolution * idealResolution, level: LogLevel.ERROR);
			return false;
		}

		int maxToUse;
		if (maxValueMode == MAX_MODE_2AVG)
		{
			Debug.BeginTimeMeasure();
			
			int maxAvg;
			int div;
			foreach (int value : imageData)
			{
				if (value > 0)
				{
					maxAvg += value;
					++div;
				}
			}

			if (div > 0)
				maxToUse = 2 * maxAvg / div;

			Debug.EndTimeMeasure("Defining average");
		}
		else
		if (maxValueMode == MAX_MODE_2MEDIAN)
		{
			Debug.BeginTimeMeasure();

			array<int> sortedValues = {};
			sortedValues.Reserve(resolution * resolution); // at best
			foreach (int index, int value : imageData)
			{
				if (value > 0)
					sortedValues.Insert(value);
			}

			sortedValues.Sort();
			maxToUse = sortedValues[sortedValues.Count() * 0.5] * 2;

			Debug.EndTimeMeasure("Defining median");
		}

		if (maxToUse <= 0) // raw
		{
			Debug.BeginTimeMeasure();

			foreach (int value : imageData)
			{
				if (value > maxToUse)
					maxToUse = value;
			}

			Debug.EndTimeMeasure("Defining raw max");
		}

		if (maxToUse < MIN_GRADIENT_RGB && colourMode == COLOUR_MODE_THERMAL)
			PrintFormat("Less than max %1 values per pixel, thermal gradient quality may be poor", MIN_GRADIENT_RGB, level: LogLevel.WARNING);


		Debug.BeginTimeMeasure();

		int colour;
		float intensity01;
		int iIntensity;
		foreach (int index, int value : imageData)
		{
			if (value > 0) // faster?
				intensity01 = (float)value / maxToUse;
			else
				intensity01 = 0;

			if (colourMode == COLOUR_MODE_GREYSCALE)
			{
				iIntensity = Math.Round(intensity01 * 255);

				// let's clamp
				if (iIntensity > 255)
				{
					if (highlightValuesAboveMax)
						colour = 0xFFFF0000;	// red
					else
						colour = 0xFFFFFFFF;	// white
				}
				else
				if (iIntensity <= 0)
				{
					if (invertColour)
						colour = 0xFFFFFFFF;
					else
						colour = 0xFF000000;
				}
				else
				{
					if (invertColour)
						iIntensity = 255 - iIntensity;

					colour = 0xFF000000
						| iIntensity * 0x10000
						| iIntensity * 0x100
						| iIntensity; // * 0x1
				}
			}
			else
			if (colourMode == COLOUR_MODE_THERMAL)
			{
				// based on https://stackoverflow.com/questions/7706339/grayscale-to-red-green-blue-matlab-jet-color-scale
				if (intensity01 > 1)
				{
					if (highlightValuesAboveMax)
						colour = 0xFFFFFFFF;	// white
					else
						colour = 0xFFFF0000;	// red
				}
				else
				if (intensity01 <= 0)
				{
					if (invertColour)
						colour = 0xFFFF0000;		// red
					else
						colour = 0xFF000000;		// black
				}
				else
				{
					if (invertColour)
						intensity01 = 1 - intensity01;

					float r, g, b;
					if (intensity01 < 0.25)
					{
						// r = 0;
						g = 4 * intensity01;
						b = 1;
					}
					else
					if (intensity01 < 0.5)
					{
						// r = 0;
						g = 1;
						b = 1 + 4 * (0.25 - intensity01);
					}
					else
					if (intensity01 < 0.75)
					{
						r = 4 * (intensity01 - 0.5);
						g = 1;
						// b = 0;
					}
					else
					{
						r = 1;
						g = 1 + 4 * (0.75 - intensity01);
						// b = 0;
					}

					colour = 0xFF000000
						| ((int)(255 * r) * 0x10000)
						| ((int)(255 * g) * 0x100)
						| ((int)(255 * b)); //  * 0x1
				}
			}
			else
//			if (colourMode == COLOUR_MODE_ALPHA)
			{
				iIntensity = Math.Round(intensity01 * 255);

				// let's clamp
				if (iIntensity > 255)
				{
					if (highlightValuesAboveMax)
						colour = 0xFFFF0000;	// red
					else
						colour = 0xFFFFFFFF;	// white
				}
				else
				if (iIntensity <= 0)
				{
					if (invertColour)
						colour = 0x00000000;
					else
						colour = 0x00FFFFFF;
				}
				else
				{
					if (invertColour)
						colour = 0x00000000 | iIntensity * 0x1000000;
					else
						colour = 0x00FFFFFF | iIntensity * 0x1000000;
				}
			}

			imageData[index] = colour;
		}

		Debug.EndTimeMeasure(string.Format("Setting pixel values for %1×%1 image", resolution, resolution));

		if (resolutionFactor < 1)
			resolutionFactor = 1;

		while (resolution * resolutionFactor > MAX_RESOLUTION)
		{
			if (resolutionFactor < 2) // should not happen with provided options
			{
				int newResolution = resolution;
				while (newResolution > 1 && newResolution * resolutionFactor > MAX_RESOLUTION)
				{
					newResolution /= 2;
				}

				PrintFormat(
					"Resolution %1 is greater than max allowed %2 resolution; using %3",
					resolution,
					MAX_RESOLUTION,
					newResolution,
					level: LogLevel.WARNING);

				resolution = newResolution;
			}
			else
			{
				int newResolutionFactor = resolutionFactor;
				while (newResolutionFactor > 1 && resolution * newResolutionFactor > MAX_RESOLUTION)
				{
					newResolutionFactor *= 0.5;
				}

				PrintFormat(
					"Resolution %1 multiplied by %2 (%3) is greater than max allowed %4 resolution; changing multiplier to %5",
					resolution,
					resolutionFactor,
					resolution * resolutionFactor,
					MAX_RESOLUTION,
					newResolutionFactor,
					level: LogLevel.WARNING);

				resolutionFactor = newResolutionFactor;
			}
		}

		if (resolutionFactor > 1)
		{
			Debug.BeginTimeMeasure();
			if (SCR_ImageHelper.IncreaseResolution(imageData, resolution, resolution, resolutionFactor))
			{
				Debug.EndTimeMeasure(string.Format("Increasing image side by a factor of %1 (from %2×%2 to %3×%3) - SUCCESS", resolutionFactor, resolution, resolution * resolutionFactor));
				resolution *= resolutionFactor;
			}
			else
			{
				Debug.EndTimeMeasure(string.Format("Increasing image side by a factor of %1 (from %2×%2 to %3×%3) - FAILURE", resolutionFactor, resolution, resolution * resolutionFactor));
			}
		}

		Debug.BeginTimeMeasure();
		bool result = SCR_ImageHelper.TEMP_SaveImageDataWrapper(imagePath, resolution, resolution, imageData);
		Debug.EndTimeMeasure("Writing image " + imagePath);

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] multipliers default { 1, 2, 4, 8 }
	//! \return a string to be used with SCR_ParamEnumArray.FromString
	static string GetResolutionFactorEnum(array<int> multipliers = null)
	{
		array<int> multipliersRef = multipliers;
		int multipliersCount;
		if (multipliersRef)
			multipliersCount = multipliersRef.Count();

		if (multipliersCount < 1)
		{
			multipliersRef = { 1, 2, 4, 8 };
			multipliersCount = 4;
		}

		if (multipliersCount > 0)
		{
			multipliersRef.Sort();
			for (int i = multipliersCount - 1; i >= 0; --i)
			{
				int multiplier = multipliersRef[i];
				if (multiplier < 1 || multipliersRef.Find(multiplier) != i)
				{
					multipliersRef.Remove(i);
					--multipliersCount;
				}
			}
		}

		if (multipliersCount > 1)
			multipliersRef.Sort();

		string result;
		foreach (int i, int multiplier : multipliersRef)
		{
			if (i > 0)
				result += string.Format(";%1,×%1,e.g 64×64 becomes %2×%2", multiplier, 64 * multiplier);
			else
			if (multiplier == 1)
				result += "1,1:1 resolution,1 data point = 1 pixel";
			else
				result += string.Format("%1,×%1,e.g 64×64 becomes %2×%2", multiplier, 64 * multiplier);
		}

		return result;
	}
}
