/*!
\defgroup World World
\{
*/

typedef func QueryEntitiesCallback;
bool QueryEntitiesCallback(IEntity e);

typedef func TraceFilterCallback;
bool TraceFilterCallback(notnull IEntity e, vector start = "0 0 0", vector dir = "0 0 0");

typedef func AsyncTraceFinishedCallback;
void AsyncTraceFinishedCallback(notnull TraceParam param = null, float fraction = 0);

/*!
Post-process effect type.
\attention Keep enum names in sync with post-process effect material class names. Postfix "Effect" is added automatically.
*/
enum PostProcessEffectType
{
	None,
	UnderWater,
	SSAO,
	DepthOfField,
	HBAO,
	RotBlur,
	GodRays,
	Rain,
	FilmGrain,
	RadialBlur,
	ChromAber,
	WetDistort,
	DynamicBlur,
	ColorGrading,
	Colors,
	HDR,
	SMAA,
	FXAA,
	Median,
	SunMask,
	GaussFilter,
	SSR,
	DepthOfFieldBokeh,
	ThermalImaging
}

class SharedItem: pointer
{

}

TraceParam MakeTraceParam(vector start, vector end, TraceFlags flags)
{
	TraceParam param = new TraceParam();
	param.Start = start;
	param.End = end;
	param.Flags = flags;
	return param;
}

/*!
\}
*/
