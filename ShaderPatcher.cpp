#include "include/fxlib/FXLib.h"
#include "include/fxlib/ShaderPatcher.h"

using namespace FX;

ShaderPatcher::Lines ShaderPatcher::sourceToLines(const std::string& shaderSource)
{
	Lines result;

	std::stringstream stream{ shaderSource };

	std::string line;
	while (std::getline(stream, line))
	{
		//trim(line);
		if (!line.empty())
			result.push_back(line);
	}

	return result;
}

std::string ShaderPatcher::joinLines(const Lines& shaderLines)
{
	std::stringstream shaderSource;

	for (auto& line : shaderLines)
	{
		shaderSource << line << '\n';
	}

	std::string result = shaderSource.str();
	return result;
}

ShaderPatcher::ShaderPatcher(const std::string& shaderSource)
{
	shaderLines = sourceToLines(shaderSource);
	cursor = getVersionLine(shaderLines);
}

ShaderPatcher& ShaderPatcher::addLine(const std::string& content)
{
	if (cursor == shaderLines.end())
		cursor = getVersionLine(shaderLines);

	if (cursor != shaderLines.end())
	{
		cursor += 1;
		cursor = shaderLines.insert(cursor, content);
	}

	return *this;
}
ShaderPatcher& ShaderPatcher::addLineInMain(const std::string& content, bool end)
{
	auto line = getMainLine(shaderLines, end);

	if (line != shaderLines.end())
	{
		line += (end ? -1 : 1);

		shaderLines.insert(line, content);
	}

	return *this;
}

ShaderPatcher& ShaderPatcher::define(const std::string& name)
{
	addLine(std::format("#define {}", name));
	return *this;
}

ShaderPatcher& ShaderPatcher::define(const std::string& name, const std::string& value)
{
	addLine(std::format("#define {} {}", name, value));
	return *this;
}

ShaderPatcher& ShaderPatcher::include(const std::string& shaderSource)
{
	// remove the version from the input shaderSource
	std::string source = shaderSource;
	Lines lines = sourceToLines(source);
	auto verLine = getVersionLine(lines);
	if (verLine != lines.end())
		lines.erase(verLine);
	source = joinLines(lines);

	addLine("// #include start");
	addLine(source);
	addLine("// #include end");

	return *this;
}

ShaderPatcher& ShaderPatcher::addFunction(const std::string& name, const std::string& code)
{
	addLine(std::format("bool func_{}()", name));
	addLine("{");
	addLine(code);
	addLine("}");

	return *this;
}
ShaderPatcher& ShaderPatcher::beforeMain(const std::string& function)
{
	return addLineInMain(std::format("if (!func_{}()) return;", function), false);
}
ShaderPatcher& ShaderPatcher::afterMain(const std::string& function)
{
	return addLineInMain(std::format("func_{}()", function), true);
}

std::string ShaderPatcher::getSource()
{
	return joinLines(shaderLines);
}

ShaderPatcher::Lines::iterator ShaderPatcher::getVersionLine(Lines& shaderLines)
{
	for (Lines::iterator it = shaderLines.begin(); it != shaderLines.end(); ++it)
	{
		std::string str = *it;
		utils::trim(str);
		utils::toLower(str);
		if (str.starts_with("#version "))
			return it;
	}
	return shaderLines.begin();
}
ShaderPatcher::Lines::iterator ShaderPatcher::getMainLine(Lines& shaderLines, bool end)
{
	bool inMain = false;
	int depth = 0;
	for (Lines::iterator it = shaderLines.begin(); it != shaderLines.end(); ++it)
	{
		std::string str = *it;
		utils::trim(str);
		utils::toLower(str);
		if (str.starts_with("void main()"))
		{
			inMain = true;
		}
		if (inMain && str.find('{') != std::string::npos)
		{
			if (!end)
			{
				return it;
			}
			else
			{
				++depth;
			}
		}
		if (end && inMain && str.find('}') != std::string::npos)
		{
			--depth;
			if (depth == 0)
			{
				return it;
			}
		}
	}
	return shaderLines.end();
}
