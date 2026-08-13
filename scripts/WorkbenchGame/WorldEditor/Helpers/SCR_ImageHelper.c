#ifdef WORKBENCH
class SCR_ImageHelper
{
	protected static string s_sImageFilePath;

	protected static const string DEFAULT_FILE_PATH = "image.png";
	protected static const string PNG_DOTTED_EXTENSION = ".png";
	protected static const string DDS_DOTTED_EXTENSION = ".dds";
	protected static const string EDDS_DOTTED_EXTENSION = ".edds";
	protected static const string META_DOTTED_EXTENSION = ".meta";

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] filePath
	//! \param[in] width
	//! \param[in] height
	//! \param[in] data in ARGB format (0xAARRGGBB, e.g 0xFF0000FF for blue)
	//! \return true on success, false otherwise
	static bool TEMP_SaveImageDataWrapper(string filePath, int width, int height, notnull array<int> data)
	{
		int dataCount = data.Count();
		if (dataCount < 1)
		{
			Print("[SCR_ImageHelper.TEMP_SaveImageDataWrapper] provided data is empty", LogLevel.ERROR);
			return false;
		}

		if (dataCount != width * height)
		{
			Print("[SCR_ImageHelper.TEMP_SaveImageDataWrapper] data / width × height mismatch", LogLevel.ERROR);
			return false;
		}

		array<int> dataCopy = {};
		dataCopy.Resize(dataCount);

		foreach (int i, int argb : data)
		{
			int a = argb & 0xFF000000;
			int b = (argb & 0x000000FF) << 16;
			int g = argb & 0x0000FF00;
			int r = (argb & 0x00FF0000) >> 16;

			dataCopy[i] = a | b | g | r;
		}

		return TexTools.SaveImageData(filePath, width, height, dataCopy);
	}

	//------------------------------------------------------------------------------------------------
	//! Get image data from floats in a non-specific range
	//! \param[in] values full range floats
	//! \param[in] min if known, please provide
	//! \param[in] max if known, please provide
	//! \return int array of ARGB greyscale values
	static array<int> GetImageDataBlackAndWhiteFromRange(notnull array<float> values, float min = float.INFINITY, float max = float.INFINITY)
	{
		int valuesCount = values.Count();
		if (valuesCount < 1)
			return {};

		bool checkForMin = min == float.INFINITY || min == -float.INFINITY || min > max;
		bool checkForMax = max == float.INFINITY || max == -float.INFINITY || min > max;
		if (checkForMin || checkForMax)
		{
			float initialMin = min;
			float initialMax = max;

			min = values[0];
			max = min;
			foreach (float value : values)
			{
				if (value == float.INFINITY)
				{
					value = float.MAX;
					max = value;
					continue;
				}
				else
				if (value == -float.INFINITY)
				{
					value = -float.MAX;
					min = value;
					continue;
				}

				if (min > value)
					min = value;
				else
				if (max < value)
					max = value;
			}
		}

		array<int> result = {};
		result.Resize(valuesCount);

		if (min == max)
		{
			for (int i; i < valuesCount; ++i)
			{
				result[i] = 0xFF000000; // black
			}
		}
		else
		{
			foreach (int i, float value : values)
			{
				int iValue = Math.Round(Math.InverseLerp(min, max, value) * 255);
				result[i] = 0xFF000000 | (iValue << 16) | (iValue << 8) | iValue;
			}
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Get image data from ints in a non-specific range
	//! \param[in] values full range ints
	//! \param[in] min if known, please provide
	//! \param[in] max if known, please provide
	//! \return int array of ARGB greyscale values
	static array<int> GetImageDataBlackAndWhiteFromRange(notnull array<int> values, int min = int.MIN, int max = int.MAX)
	{
		int valuesCount = values.Count();
		if (valuesCount < 1)
			return {};

		bool checkForMin = min == int.MIN || min == int.MAX || min > max;
		bool checkForMax = max == int.MIN || max == int.MAX || min > max;
		if (checkForMin || checkForMax)
		{
			int initialMin = min;
			int initialMax = max;

			min = values[0];
			max = min;
			foreach (int value : values)
			{
				if (min > value)
					min = value;
				else
				if (max < value)
					max = value;
			}
		}

		array<int> result = {};
		result.Resize(valuesCount);

		if (min == max)
		{
			for (int i; i < valuesCount; ++i)
			{
				result[i] = 0xFF000000; // black
			}
		}
		else
		{
			foreach (int i, int value : values)
			{
				int iValue = Math.Round(Math.InverseLerp(min, max, value) * 255);
				result[i] = 0xFF000000 | (iValue << 16) | (iValue << 8) | iValue;
			}
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Return greyscale from 0..1 values; another method overload takes 0..255 int values
	//! \param[in] greyscaleValues01
	//! \return int array of ARGB values
	static array<int> GetImageDataBlackAndWhite(notnull array<float> greyscaleValues01)
	{
		array<int> result = {};
		result.Reserve(greyscaleValues01.Count());

		foreach (float greyscaleValue01 : greyscaleValues01)
		{
			if (greyscaleValue01 > 1)
				greyscaleValue01 = 1;
			else
			if (greyscaleValue01 < 0)
				greyscaleValue01 = 0;

			int greyscaleValue0255 = Math.Round(greyscaleValue01 * 255);
			result.Insert(0xFF000000 | (greyscaleValue0255 << 16) | (greyscaleValue0255 << 8) | greyscaleValue0255);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Return greyscale from 0..255 values; another method overload takes 0..1 float values
	//! \param[in] greyscaleValues0255
	//! \return int array of ARGB values
	static array<int> GetImageDataBlackAndWhite(notnull array<int> greyscaleValues0255)
	{
		array<int> result = {};
		result.Reserve(greyscaleValues0255.Count());

		foreach (int greyscaleValue0255 : greyscaleValues0255)
		{
			if (greyscaleValue0255 > 255)
				greyscaleValue0255 = 255;
			else
			if (greyscaleValue0255 < 0)
				greyscaleValue0255 = 0;

			result.Insert(0xFF000000 | (greyscaleValue0255 << 16) | (greyscaleValue0255 << 8) | greyscaleValue0255);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] greyscaleValues01
	//! \param[in] color if null, uses full white
	//! \return
	static array<int> GetImageDataAlphaColour(notnull array<float> greyscaleValues01, Color colour = null)
	{
		array<int> result = {};
		result.Reserve(greyscaleValues01.Count());

		int colourMask;
		if (colour)
			colourMask = colour.PackToInt() & 0x00FFFFFF; // drop the alpha
		else
			colourMask = 0x00FFFFFF;

		foreach (float greyscaleValue01 : greyscaleValues01)
		{
			if (greyscaleValue01 > 1)
				greyscaleValue01 = 1;
			else
			if (greyscaleValue01 < 0)
				greyscaleValue01 = 0;

			result.Insert(((int)(greyscaleValue01 * 255) << 24) | colourMask);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Increase resolution - each pixel will take factor × factor pixels, e.g 64×64 factor 4 = 256×256
	//! \param[in,out] imageData
	//! \param[in] factor
	//! \return true on success, false otherwise
	static bool IncreaseResolution(notnull inout array<int> imageData, int width, int height, int factor)
	{
		if (factor < 1 || width < 1 || height < 1)
			return false;

		if (factor == 1)
			return true;

		int imageDataCount = imageData.Count();
		if (imageDataCount != width * height)
		{
			Print("[SCR_ImageHelper.IncreaseResolution] imageData array size does not match width × height", LogLevel.WARNING);
			return false;
		}

		array<int> enhancedData = {};
		enhancedData.Reserve(imageDataCount * factor * factor);

		for (int rowId; rowId < height; ++rowId)
		{
			for (int rowRepeat; rowRepeat < factor; ++rowRepeat)
			{
				for (int colId; colId < width; ++colId)
				{
					int value = imageData[rowId * width + colId];
					for (int colRepeat; colRepeat < factor; ++colRepeat)
					{
						enhancedData.Insert(value);
					}
				}
			}
		}

		imageData.Copy(enhancedData);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] filePath
	//! \param[in] width
	//! \param[in] height
	//! \param[in] imageData ARGB int raw pixels
	//! \return true on success, false otherwise
	static bool SaveImageDataToPNG(string filePath, int width, int height, notnull array<int> imageData)
	{
		if (imageData.Count() != width * height)
		{
			Print("[SCR_ImageHelper.SaveImageDataToPNG] imageData array size does not match width × height", LogLevel.WARNING);
			return false;
		}

		filePath.TrimInPlace();
		if (!filePath)
			filePath = DEFAULT_FILE_PATH;

		if (!filePath.EndsWith(PNG_DOTTED_EXTENSION))
			filePath += PNG_DOTTED_EXTENSION;

		string fileWithoutExtension = FilePath.StripExtension(filePath);

		// 1. create DDS texture
		string absoluteDDSFilePath;
		if (!Workbench.GetAbsolutePath(fileWithoutExtension + DDS_DOTTED_EXTENSION, absoluteDDSFilePath, false))
			return false;

		string absoluteEDDSFilePath;
		if (!Workbench.GetAbsolutePath(fileWithoutExtension + EDDS_DOTTED_EXTENSION, absoluteEDDSFilePath, false))
			return false;

		if (!TexTools.SaveImageData(absoluteDDSFilePath, width, height, imageData))
			return false;

		// 2. register texture
		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
		if (!resourceManager)
			return false;

		resourceManager.RegisterResourceFile(absoluteDDSFilePath, true);

		MetaFile eddsMetaFile = resourceManager.GetMetaFile(absoluteEDDSFilePath);
		if (!eddsMetaFile)
			return false;

		ArmaReforgerScripted game = GetGame();
		if (!game)
		{
			Print("[SCR_ImageHelper.SaveImageDataToPNG] No Game!", LogLevel.WARNING);
			return false;
		}

		WorkspaceWidget workspaceWidget = game.GetWorkspace();
		if (!workspaceWidget)
		{
			Print("[SCR_ImageHelper.SaveImageDataToPNG] No WorkspaceWidget!", LogLevel.WARNING);
			return false;
		}

		// 3. create an image widget and load the texture in it
		ImageWidget imageWidget = ImageWidget.Cast(workspaceWidget.CreateWidget(WidgetType.ImageWidgetTypeID, WidgetFlags.CENTER, Color.White, 0, null));
		if (!imageWidget)
			return false;

		if (!imageWidget.LoadImageTexture(0, eddsMetaFile.GetResourceID(), false, true))
			return false;

		// 4. get PixelRawData and save to PNG
		s_sImageFilePath = filePath;
		if (!imageWidget.GetTextureRawData(0, GetTextureRawDataCallbackMethod)) // saves the image to s_sImageFilePath
		{
			Print("[SCR_ImageHelper.SaveImageDataToPNG] Cannot get texture raw data", LogLevel.WARNING);
			return false;
		}

		FileIO.DeleteFile(absoluteDDSFilePath);
		FileIO.DeleteFile(absoluteDDSFilePath + META_DOTTED_EXTENSION);
		FileIO.DeleteFile(absoluteEDDSFilePath);
		FileIO.DeleteFile(absoluteEDDSFilePath + META_DOTTED_EXTENSION);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] registeredImagePath
	//! \return
	static bool SetColorSpaceToSRGB(ResourceName registeredImagePath)
	{
		return SetColorSpaceTo(registeredImagePath, "ToSRGB");
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] registeredImagePath
	//! \return
	static bool SetColorSpaceToLinear(ResourceName registeredImagePath)
	{
		return SetColorSpaceTo(registeredImagePath, "ToLinear");
	}

	//------------------------------------------------------------------------------------------------
	protected static bool SetColorSpaceTo(ResourceName registeredImagePath, string colorSpace)
	{
		if (!registeredImagePath) // .IsEmpty()
		{
			Print("[SCR_ImageHelper.SetColorSpaceTo] Provided registeredImagePath is empty", LogLevel.ERROR);
			return false;
		}

		if (!colorSpace) // .IsEmpty()
		{
			Print("[SCR_ImageHelper.SetColorSpaceTo] Provided colorSpace is empty", LogLevel.ERROR);
			return false;
		}

		string imageAbsolutePath;
		if (!Workbench.GetAbsolutePath(registeredImagePath.GetPath(), imageAbsolutePath, true))
		{
			Print("[SCR_ImageHelper.SetColorSpaceTo] Cannot find " + registeredImagePath.GetPath(), LogLevel.ERROR);
			return false;
		}

		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
		if (!resourceManager)
		{
			Print("[SCR_ImageHelper.SetColorSpaceTo] Resource Manager is not available", LogLevel.ERROR);
			return false;
		}

		MetaFile metaFile = resourceManager.GetMetaFile(imageAbsolutePath);
		if (!metaFile)
		{
			Print("[SCR_ImageHelper.SetColorSpaceTo] MetaFile not found for " + imageAbsolutePath, LogLevel.ERROR);
			return false;
		}

		BaseContainerList configurations = metaFile.GetObjectArray("Configurations");
		if (!configurations || configurations.Count() < 1)
		{
			Print("[SCR_ImageHelper.SetColorSpaceTo] Invalid MetaFile: Configurations missing or empty for " + imageAbsolutePath, LogLevel.ERROR);
			return false;
		}

		if (!configurations.Get(0).Set("ColorSpace", colorSpace)) //--- Assume PC is the first
		{
			Print("[SCR_ImageHelper.SetColorSpaceTo] Invalid color space " + colorSpace, LogLevel.ERROR);
			return false;
		}

		metaFile.Save();
		return true;
	}

	//------------------------------------------------------------------------------------------------
	// GetTextureRawDataCallback
	protected static void GetTextureRawDataCallbackMethod(PixelRawData data, int imageWidth, int imageHeight, int stride)
	{
//		PrintFormat("Saving to %1 (%2x%3 stride %4)", s_sImageFilePath, imageWidth, imageHeight, stride);
		Workbench.SavePixelRawData(s_sImageFilePath, data, imageWidth, imageHeight, stride);
	}
}
#endif // WORKBENCH
