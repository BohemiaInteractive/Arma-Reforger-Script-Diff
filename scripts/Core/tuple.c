/*!
\defgroup Tools Tools
Helpful functions & classes
\{
*/

//-----------------------------------------------------------------------------
/*!
Base Tuple class with no parameters. Used as general purpose parameter
overloaded with Tuple1 to Tuple4 templates.
*/
class Tuple: Managed
{
	bool Serialize(Serializer ctx)
	{
		return false;
	}

	bool Deserializer(Serializer ctx)
	{
		return false;
	}
}

/*!
Tuple class template with one parameter.
Usage:
\code
	Tuple paramA = new Tuple1<int>(55);
	Tuple paramB = new Tuple1<string>("Hello");
\endcode
*/
class Tuple1<Class T1> extends Tuple
{
	T1 param1;

	void Tuple1(T1 p1)
	{
		param1 = p1;
	}

	override bool Serialize(Serializer ctx)
	{
		return ctx.Write(param1);
	}

	override bool Deserializer(Serializer ctx)
	{
		return ctx.Read(param1);
	}
}

/*!
Tuple class template with two parameters (Pair).
Usage:
\code
	Tuple param = new Tuple2<float, string>(3.14, "Pi");
\endcode
*/
class Tuple2<Class T1, Class T2> extends Tuple
{
	T1 param1;
	T2 param2;

	void Tuple2(T1 p1, T2 p2)
	{
		param1 = p1;
		param2 = p2;
	}

	override bool Serialize(Serializer ctx)
	{
		return ctx.Write(param1) && ctx.Write(param2);
	}

	override bool Deserializer(Serializer ctx)
	{
		return ctx.Read(param1) && ctx.Read(param2);
	}
}

/*!
Tuple class template with three parameters.
Usage:
\code
	Tuple param = new Tuple3<float, string, bool>(2.89, "Lala", true);
\endcode
*/
class Tuple3<Class T1, Class T2, Class T3> extends Tuple
{
	T1 param1;
	T2 param2;
	T3 param3;

	void Tuple3(T1 p1, T2 p2, T3 p3)
	{
		param1 = p1;
		param2 = p2;
		param3 = p3;
	}

	override bool Serialize(Serializer ctx)
	{
		return ctx.Write(param1) && ctx.Write(param2) && ctx.Write(param3);
	}

	override bool Deserializer(Serializer ctx)
	{
		return ctx.Read(param1) && ctx.Read(param2) && ctx.Read(param3);
	}
}

/*!
Tuple class template with four parameters.
Usage:
\code
	Tuple param = new Tuple4<int, bool, float, string>(100, false, 79.9, "Test");
\endcode
*/
class Tuple4<Class T1, Class T2, Class T3, Class T4> extends Tuple
{
	T1 param1;
	T2 param2;
	T3 param3;
	T4 param4;

	void Tuple4(T1 p1, T2 p2, T3 p3, T4 p4)
	{
		param1 = p1;
		param2 = p2;
		param3 = p3;
		param4 = p4;
	}

	override bool Serialize(Serializer ctx)
	{
		return ctx.Write(param1) && ctx.Write(param2) && ctx.Write(param3) && ctx.Write(param4);
	}

	override bool Deserializer(Serializer ctx)
	{
		return ctx.Read(param1) && ctx.Read(param2) && ctx.Read(param3) && ctx.Read(param4);
	}
}

//! Tuple class template with five parameters.
class Tuple5<Class T1, Class T2, Class T3, Class T4, Class T5> extends Tuple
{
	T1 param1;
	T2 param2;
	T3 param3;
	T4 param4;
	T5 param5;

	void Tuple5(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5)
	{
		param1 = p1;
		param2 = p2;
		param3 = p3;
		param4 = p4;
		param5 = p5;
	}

	override bool Serialize(Serializer ctx)
	{
		return ctx.Write(param1) && ctx.Write(param2) && ctx.Write(param3) && ctx.Write(param4) && ctx.Write(param5);
	}

	override bool Deserializer(Serializer ctx)
	{
		return ctx.Read(param1) && ctx.Read(param2) && ctx.Read(param3) && ctx.Read(param4) && ctx.Read(param5);
	}
}

//! Tuple class template with six parameters.
class Tuple6<Class T1, Class T2, Class T3, Class T4, Class T5, Class T6> extends Tuple
{
	T1 param1;
	T2 param2;
	T3 param3;
	T4 param4;
	T5 param5;
	T6 param6;

	void Tuple6(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6)
	{
		param1 = p1;
		param2 = p2;
		param3 = p3;
		param4 = p4;
		param5 = p5;
		param6 = p6;
	}

	override bool Serialize(Serializer ctx)
	{
		return ctx.Write(param1) && ctx.Write(param2) && ctx.Write(param3) && ctx.Write(param4) && ctx.Write(param5) && ctx.Write(param6);
	}

	override bool Deserializer(Serializer ctx)
	{
		return ctx.Read(param1) && ctx.Read(param2) && ctx.Read(param3) && ctx.Read(param4) && ctx.Read(param5) && ctx.Read(param6);
	}
}

/*!
\}
*/
