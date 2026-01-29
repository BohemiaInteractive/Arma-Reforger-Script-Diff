/*
Function/method modifiers:
proto - prototyping of internal function (C++ side)
native - native call convention of internal function (C++ side)
volatile - internal function that may call back to script (hint for
  compiler that context need to be saved on stack)
private - function may not be called from script
event - hint for tools that the function should be exposed as
 Entity script event.

Variable modifiers:
owned - modifier for returing internal functions. Tells to script-VM,
that returning variable (string or array) must not be released
out - modifier for function parameters. It tells that variable will
 be changed by function call (used mainly by internal functions)
inout - modifier for function parameters. It tells that variable will
 be used and then changed by function call (used mainly by internal functions)

const - constants. May not be modified.
reference - hint for tools (Material editor), that the variable may be used
 as parameter in material
\code
	//some example "reference" variables for use in material editor
	reference float g_testVariable1;
	reference float g_testVariable2;
	reference float g_testVariable3;

	class TestClass
	{
		//some example "reference" variables for use in material editor
		reference float testVar1;
		reference float testVar2;
		reference float testVar3;
	}
\endcode
*/

/*===================================================================*/
/*							Enforce engine API										*/
/*===================================================================*/

class ProfileData
{
	float FPS;
	float FrameTimeMs;
	float RenderTimeMs;
	float GPUTimeMs;
	float SimulationTimeMs;
	private void ProfileData();
	private void ~ProfileData();
}

//----------------------------------------------
int VectorToRGBA( vector vec, float h)
{
	float x,y,z;
	int r,g,b,a;

	x = vec[0];
	y = vec[1];
	z = vec[2];

	x = x * 127.0 + 128.0;
	y = y * 127.0 + 128.0;
	z = z * 127.0 + 128.0;
	h = h * 255.0;

	a = (int)h << 24;
	r = (int)x << 16;
	g = (int)y << 8;
	b = z;

	return r | g | b | a;
}


//-----------------------------------------------------------------
proto int ARGB(int a, int r, int g, int b);

//-----------------------------------------------------------------
//! Converts <0.0, 1.0> ARGB into color
proto int ARGBF(float fa, float fr, float fg, float fb);

//-----------------------------------------------------------------
proto int ABGR(int a, int r, int g, int b);

//-----------------------------------------------------------------
proto int ABGRF(float fa, float fr, float fg, float fb);

//-----------------------------------------------------------------
int AWHITE(int a)
{
	return a << 24 | 0xffffff;
}

//-------------------------------------------------------------------------
class Link<Class T>
{
	proto private native void Init(T init);
	proto private native Object Get();

	void Release()
	{
		T obj = Get();
		if(obj)
			obj.Release();
	}
	void Link(T init)
	{
		Init(init);
	}

	T Ptr()
	{
		return Get();
	}

	bool IsNull()
	{
		if(!Get())
			return true;

		return false;
	}
}
