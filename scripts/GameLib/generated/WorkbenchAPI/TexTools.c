/*
===========================================
Do not modify, this script is generated
===========================================
*/

#ifdef WORKBENCH

/*!
\addtogroup WorkbenchAPI
\{
*/

sealed class TexTools
{
	private void TexTools();
	private void ~TexTools();

	/*!
	Save raw pixels (ABGR stored in int) to dds file.
	\attention Pixels must be in ABGR color order
	Usage:
	\code
		string filePath = "c:\\textures\\test.dds";
		int data[256];

		// generate same gradient
		for (int x = 0; x < 16; x++)
		for (int y = 0; y < 16; y++)
		{
			int clr = y * 16 + 15;
			data[x * 16 + y] = ABGR(255, clr, clr, clr);
		}

		// save dds to file
		if (TexTools.SaveImageData(filePath, 16, 16, data) == false)
		{
			Print("Can't save image", LogLevel.ERROR);
			return;
		}
	\endcode
	*/
	static proto bool SaveImageData(string filePath, int width, int height, notnull array<int> data);
	/*!
	Repair borders. Real data pictures are from `3,3` to `sizeX-3,sizeY-3`.
	Rest of the image must be copied due to DXT compression from the border
	lines -> in 4x4 DXT block must be just four colors, the other reason is
	the mip-mapping!
	*/
	static proto void RepairTerrainTextureBorders(int width, int height, notnull inout array<int> data);
}

/*!
\}
*/

#endif // WORKBENCH
