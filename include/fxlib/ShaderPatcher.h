#pragma once

#include "FXLib.h"

namespace FX
{
	class FXLIB_API ShaderPatcher
	{
	private:
		using Lines = std::vector<std::string>;

		Lines shaderLines;
		Lines::iterator cursor;

		static Lines sourceToLines(const std::string& shaderSource);
		static std::string joinLines(const Lines& shaderLines);

		static Lines::iterator getVersionLine(Lines& shaderLines);
		static Lines::iterator getMainLine(Lines& shaderLines, bool end = false);

	public:
		ShaderPatcher(const std::string& shaderSource);

		// use with caution
		ShaderPatcher& addLine(const std::string& content);
		// use with caution
		ShaderPatcher& addLineInMain(const std::string& content, bool end = false);

		ShaderPatcher& define(const std::string& name);
		ShaderPatcher& define(const std::string& name, const std::string& value);
		ShaderPatcher& include(const std::string& shaderSource);
		// makes a bool function (for beforeMain())!
		ShaderPatcher& addFunction(const std::string& name, const std::string& code);
		// will return if the function returns false
		ShaderPatcher& beforeMain(const std::string& function);
		ShaderPatcher& afterMain(const std::string& function);

		std::string getSource();
	};
}
