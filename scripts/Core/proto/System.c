/*!
\defgroup System System
Enforce script system methods
\{
*/

//! Object used for holding filtering params for ResourceDatabase.SearchResources() method
class SearchResourcesFilter : Managed
{
	string rootPath; // Exact path format expected (e.g. "$ArmaReforger:Prefabs/Weapons")
	ref array<string> fileExtensions; // File extensions without leading dot (e.g. "emat")
	ref array<string> searchStr;
	ref array<string> tags;
	bool recursive = true; // When enabled, we wil search whole sub-tree
}

//! Callback used for processing results from ResourceDatabase.SearchResources() method
typedef func SearchResourcesCallback;
void SearchResourcesCallback(ResourceName resourceName, string exactPath = "");

typedef func FindFilesCallback;
void FindFilesCallback(string fileName, FileAttribute attributes = 0, string filesystem = string.Empty);

/*!
screenshot callback API, returned data are in RGBA8 format -> each channel has 8 bits and four channels are used 
thus size in bytes for one pixel is 4 and the int type can be used for simpler work 

example how to access pixel data on further c++ side:

	for (int y = 0; y < height; y++)
	{
		int const* row = (int const*) (((char const*) data) + stride);
		for (int x = 0; x < width; x++)
		{
			int pixel = row[x];
			...
		}
	}

\param pixels		pointer to raw pixels
\param width 		width of image data
\param height		height of image data
\param stride		stride is offset to next row - it doesn't need to be same like image width due to GPU data alignment rules
*/
typedef func ScreenshotRawDataCallback;
void ScreenshotRawDataCallback(PixelRawData data, int imageWidth, int imageHeight, int stride);
                             
/*!
screenshot callback API, returned texture in RGBA8 format
*/
typedef func ScreenshotTextureCallback;
void ScreenshotTextureCallback(ScreenshotTextureData data);

/*!
texture data callback API, returned texture in RGBA8 format
*/
typedef func GetTextureRawDataCallback;
void GetTextureRawDataCallback(PixelRawData data, int imageWidth, int imageHeight, int stride);



class MemoryStatsSnapshot: Managed
{
	proto native static int GetStatsCount();
	proto native static string GetStatName(int idx);
	proto native int GetStatValue(int idx);
}

//! Handle to a running process.
class ProcessHandle: handle64
{
}

/*!
\}
*/
