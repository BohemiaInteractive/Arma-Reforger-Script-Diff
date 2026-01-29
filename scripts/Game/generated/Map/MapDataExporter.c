/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Map
\{
*/

class MapDataExporter
{
	proto external DataExportErrorType ExportData(EMapDataType type, string exportPath, string worldPath, float minimumHillHeight = 50, bool bIgnoreGeneratorAreas = true, ExcludeGenerateFlags generateFlags = ExcludeGenerateFlags.ExcludeGenerateNone, ExcludeSaveFlags saveFlags = ExcludeSaveFlags.ExcludeSaveNone);
	proto external DataExportErrorType SetupColors( notnull Color landColorBright, notnull Color landColorDark, notnull Color oceanColorBright, notnull Color oceanColorDark, notnull Color forestAreaColor, notnull Color otherAreaColor );
	proto external DataExportErrorType ExportRasterization(string exportPath, string worldPath, float scaleLand, float scaleOcean, float heightScale, float depthScale, float depthLerpMeters, float shadeIntensity, float heightIntensity, bool bIncludeGeneratorAreas, float forestAreaIntensity, float otherAreaIntensity);
}

/*!
\}
*/
