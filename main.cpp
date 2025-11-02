#include <4dm.h>

using namespace fdm;

#include "include/fxlib/FXLib.h"

// Initialize the DLLMain
initDLL

inline static bool initializedContext = false;

inline static bool allowedToLoadShaders = false;
$hookStatic(bool, ShaderManager, loadFromShaderList, const stl::string& jsonListPath)
{
	if (allowedToLoadShaders)
		return original(jsonListPath);

	// no
	return true;
}

$hook(void, StateIntro, init, StateManager& s)
{
	// initialize opengl stuff
	glewExperimental = true;
	glewInit();
	glfwInit();

	// update the window from gl3.3core context to gl4.5core context to be able to use SSBOs n other cool stuff
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, 1);
	GLFWwindow* window = glfwCreateWindow(
		StateSettings::DEFAULT_WINDOW_WIDTH, StateSettings::DEFAULT_WINDOW_HEIGHT,
		"4D Miner by Mashpoe", nullptr, nullptr);

	if (window == nullptr)
	{
		MessageBoxA(0, "Your computer does not support GL4.5 Core, which is required for FXLib to work\n:(", "Oopsie!", MB_ICONERROR);
		glfwTerminate();
		exit(0);
		return;
	}

	glfwDestroyWindow(s.window);
	s.window = window;

	glfwMakeContextCurrent(window);

	initializedContext = true;

	glfwSetInputMode(window, GLFW_STICKY_KEYS, 1);

	*reinterpret_cast<GLFWwindow**>(fdm::base + 0x29B3E8) = window; // replace the global window used in main()

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_BLEND);
	glEnable(GL_ALPHA_TEST);
	glBlendEquationSeparate(GL_FUNC_ADD, GL_MAX);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	// load assets
	allowedToLoadShaders = true;
	ShaderManager::loadFromShaderList("shaderList.json");

	FX::ParticleSystem::defaultShader = (const FX::Shader*)
		ShaderManager::load("tr1ngledev.fxlib.particleShader",
			"assets/shaders/particle.vert",
			"assets/shaders/particle.frag",
			"assets/shaders/particle.geom");

	FX::TrailRenderer::defaultShader = (const FX::Shader*)
		ShaderManager::load("tr1ngledev.fxlib.trailShader",
			"assets/shaders/trail.vert",
			"assets/shaders/trail.frag",
			"assets/shaders/trail.geom");

	original(self, s);
}

bool FX::hasInitializedContext()
{
	return initializedContext;
}
