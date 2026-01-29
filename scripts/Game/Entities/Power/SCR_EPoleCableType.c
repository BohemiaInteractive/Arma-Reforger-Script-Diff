enum SCR_EPoleCableType
{
//	NONE,
	POWER_BUILDING,		//!< from pole to building
	POWER_LV,			//!< from pole to pole inside settlements/districts
	POWER_HV,			//!< from regional substations to settlement/districts borders
	POWER_EHV,			//!< from small power source/UHV transformer station to regional substations
	POWER_UHV,			//!< from large power source (coal plant, etc) to transformer stations
	POWER_INSULATOR,	//!< EHV/UHV insulator cable

	TELEPHONE,			//!< guess
	PA_SYSTEM,			//!< Public Address System, e.g city announcement system
	CAMERA,				//!< CCTV

//	POWER_MV,			// nope
//	INERT,
//	DECORATION,
//	COBWEB,
}
