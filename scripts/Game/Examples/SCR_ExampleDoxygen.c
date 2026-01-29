#ifdef UNDEFINED_MACRO
//! @defgroup Examples
//! Examples of various features, not to be used in actual game.

//! Example of how to document a class in Doxygen.
//! @ingroup Examples
//!
//! 	██████╗  ██████╗ ██╗  ██╗██╗   ██╗ ██████╗ ███████╗███╗   ██╗
//! 	██╔══██╗██╔═══██╗╚██╗██╔╝╚██╗ ██╔╝██╔════╝ ██╔════╝████╗  ██║
//! 	██║  ██║██║   ██║ ╚███╔╝  ╚████╔╝ ██║  ███╗█████╗  ██╔██╗ ██║
//! 	██║  ██║██║   ██║ ██╔██╗   ╚██╔╝  ██║   ██║██╔══╝  ██║╚██╗██║
//! 	██████╔╝╚██████╔╝██╔╝ ██╗   ██║   ╚██████╔╝███████╗██║ ╚████║
//! 	╚═════╝  ╚═════╝ ╚═╝  ╚═╝   ╚═╝    ╚═════╝ ╚══════╝╚═╝  ╚═══╝
//! >  What is not documented does not exist!
//!
//! # Why document?
//! - Provide info for other developers and modders
//! - Help your future self to remember
//! - Final sanity check before commit
//!
//! # What is Doxygen?
//! - Automated tool that converts code comments into HTML library
//! - Docs stay together with the code and can be updated alongside it
//!
//! # Local setup
//! 1. Download and install [Doxygen]
//! 2. Download and install [GraphViz]
//! 3. In the *Docs* folder, create *Generate.bat* file and put the following line inside
//! 	(update the path to Doxygen installation with your path)
//! 	~~~~
//! 	"C:\Program Files\doxygen\bin\doxygen.exe" ArmaReforgerDoxygen.cfg
//! 	~~~~
//! 4. **Run the bat file.**
//! 	Documentation will be generated in Docs\html folder
//! 	You can access it by opening index.html.
//!
//! 	Consider creating a shortcut / bookmark both for Generate.bat file
//! 	and for index.hml, so you can access them later with ease.
//!
//! # What to document?
//! - Public variables and methods
//! - Protected variables and method you expect to be overridden in inherited classes
//! - Enum values
//!
//! See also:
//! - the BIKI: https://community.bistudio.com/wiki/Doxygen
//! - the Script Editor's Doxygen Filler plugin (Ctrl+Alt+Shift+D) \see SCR_DoxygenFillerPlugin
//! - the official Doxygen website: https://www.doxygen.nl/
//!
//! [Doxygen]: http://www.doxygen.nl/download.html
//! [GraphViz]: https://www.graphviz.org/download/
class SCR_ExampleDoxygen
{
	//------------------------------------------------------------------------------------------------
	//! Example public variable.
	//! Ideally avoid using public variables and offer public methods instead.
	//! But if you must, please document them.
	int m_iExamplePublicVariable;

	//------------------------------------------------------------------------------------------------
	//! Example protected variable.
	//! Document protected vars for example when you expect them to be used in inherited classes.
	protected int m_iExampleProtectedVariable;

	//------------------------------------------------------------------------------------------------
	//! Simple example method.
	void Example()
	{
		// comments without Doxygen markup like this one are ignored
		Print("Example", LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	//! Example method with documented parameters.
	//! \param exampleParam example parameter
	void ExampleWithParam(int exampleParam)
	{
		Print("ExampleWithParam: exampleParam1=" + exampleParam, LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	//! Example method with out parameter.
	//! \param[out] outExampleParam Example parameter to be overridden by the method
	void ExampleWithOutParam(out int outExampleParam)
	{
		outExampleParam = 42;
		Print("ExampleWithOutParam: outExampleParam=" + outExampleParam, LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	//! Example method with inout parameter.
	//! \param[in,out] inOutExampleParam Example parameter to be overridden by the method
	void ExampleWithInOutParam(inout int inOutExampleParam)
	{
		int before = inOutExampleParam;
		inOutExampleParam *= 42;
		Print("ExampleWithInOutParam: before=" + before + ", after=" + inOutExampleParam, LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	//! Example method with return value.
	//! \return example value
	int ExampleWithReturnValue()
	{
		Print("ExampleWithReturnValue", LogLevel.NORMAL);
		return 42;
	}

	//------------------------------------------------------------------------------------------------
	//! Example method with a parameter, out parameter and returned value.
	//! \param exampleParam Example parameter
	//! \param[out] outExampleParam Example parameter to be overridden by the method
	//! \return Example value
	int ExampleWithEverything(int exampleParam, out int outExampleParam)
	{
		Print("ExampleWithEverything: exampleParam=" + exampleParam + ", outExampleParam=" + outExampleParam, LogLevel.NORMAL);
		outExampleParam = exampleParam;
		return exampleParam + outExampleParam;
	}
}

//! Example of how to document an enum in Doxygen.
//! @ingroup Examples
enum SCR_EExampleDoxygen
{
	EXAMPLE_1, //!< First value
	EXAMPLE_2, //!< Second value
}
#endif // UNDEFINED_MACRO
