/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup UI
\{
*/

sealed class RenderTargetWidget: Widget
{
	//!when period > 1 then every n-th frame will be rendered. Offset is initial counter.
	proto external void SetRefresh(int period, int offset);
	proto external int GetRefreshPeriod();
	//! Sets max FPS limit. When value is <=0, there is no limit
	proto external void SetMaxFPS(int fps);
	//! Gets max FPS limit. When value is <=0, there is no limit
	proto external int GetMaxFPS();
	proto external void SetResolutionScale(float xscale, float yscale);
	proto external void SetClearColor(bool useClearColor, int color);
	proto external void SetBlendMode(RenderTargetWidgetBlendMode blendMode);
	proto external void SetFormat(RenderTargetWidgetFormat format);
	proto external void AllowMSAAFromSetup(bool allowMSAA);
	proto external void SetWorld(BaseWorld world, int camera);
}

/*!
\}
*/
