/************************************************************************************

Authors     :   Bradley Austin Davis <bdavis@saintandreas.org>
Copyright   :   Copyright Brad Davis. All Rights reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

************************************************************************************/

#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <memory>
#include <exception>
#include <algorithm>

#include <Windows.h>

#define __STDC_FORMAT_MACROS 1

#define FAIL(X) throw std::runtime_error(X)

///////////////////////////////////////////////////////////////////////////////
//
// GLM is a C++ math library meant to mirror the syntax of GLSL 
//

#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

// Import the most commonly used types into the default namespace
using glm::ivec3;
using glm::ivec2;
using glm::uvec2;
using glm::mat3;
using glm::mat4;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::quat;

///////////////////////////////////////////////////////////////////////////////
//
// GLEW gives cross platform access to OpenGL 3.x+ functionality.  
//

#include <GL/glew.h>

bool checkFramebufferStatus(GLenum target = GL_FRAMEBUFFER) {
	GLuint status = glCheckFramebufferStatus(target);
	switch (status) {
	case GL_FRAMEBUFFER_COMPLETE:
		return true;
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
		std::cerr << "framebuffer incomplete attachment" << std::endl;
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
		std::cerr << "framebuffer missing attachment" << std::endl;
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
		std::cerr << "framebuffer incomplete draw buffer" << std::endl;
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
		std::cerr << "framebuffer incomplete read buffer" << std::endl;
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
		std::cerr << "framebuffer incomplete multisample" << std::endl;
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
		std::cerr << "framebuffer incomplete layer targets" << std::endl;
		break;

	case GL_FRAMEBUFFER_UNSUPPORTED:
		std::cerr << "framebuffer unsupported internal format or image" << std::endl;
		break;

	default:
		std::cerr << "other framebuffer error" << std::endl;
		break;
	}

	return false;
}

bool checkGlError() {
	GLenum error = glGetError();
	if (!error) {
		return false;
	}
	else {
		switch (error) {
		case GL_INVALID_ENUM:
			std::cerr << ": An unacceptable value is specified for an enumerated argument.The offending command is ignored and has no other side effect than to set the error flag.";
			break;
		case GL_INVALID_VALUE:
			std::cerr << ": A numeric argument is out of range.The offending command is ignored and has no other side effect than to set the error flag";
			break;
		case GL_INVALID_OPERATION:
			std::cerr << ": The specified operation is not allowed in the current state.The offending command is ignored and has no other side effect than to set the error flag..";
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			std::cerr << ": The framebuffer object is not complete.The offending command is ignored and has no other side effect than to set the error flag.";
			break;
		case GL_OUT_OF_MEMORY:
			std::cerr << ": There is not enough memory left to execute the command.The state of the GL is undefined, except for the state of the error flags, after this error is recorded.";
			break;
		case GL_STACK_UNDERFLOW:
			std::cerr << ": An attempt has been made to perform an operation that would cause an internal stack to underflow.";
			break;
		case GL_STACK_OVERFLOW:
			std::cerr << ": An attempt has been made to perform an operation that would cause an internal stack to overflow.";
			break;
		}
		return true;
	}
}

void glDebugCallbackHandler(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *msg, GLvoid* data) {
	OutputDebugStringA(msg);
	std::cout << "debug call: " << msg << std::endl;
}

//////////////////////////////////////////////////////////////////////
//
// GLFW provides cross platform window creation
//

#include <GLFW/glfw3.h>

namespace glfw {
	inline GLFWwindow * createWindow(const uvec2 & size, const ivec2 & position = ivec2(INT_MIN)) {
		GLFWwindow * window = glfwCreateWindow(size.x, size.y, "glfw", nullptr, nullptr);
		if (!window) {
			FAIL("Unable to create rendering window");
		}
		if ((position.x > INT_MIN) && (position.y > INT_MIN)) {
			glfwSetWindowPos(window, position.x, position.y);
		}
		return window;
	}
}

// A class to encapsulate using GLFW to handle input and render a scene
class GlfwApp {

protected:
	uvec2 windowSize;
	ivec2 windowPosition;
	GLFWwindow * window{ nullptr };
	unsigned int frame{ 0 };

public:
	GlfwApp() {
		// Initialize the GLFW system for creating and positioning windows
		if (!glfwInit()) {
			FAIL("Failed to initialize GLFW");
		}
		glfwSetErrorCallback(ErrorCallback);
	}

	virtual ~GlfwApp() {
		if (nullptr != window) {
			glfwDestroyWindow(window);
		}
		glfwTerminate();
	}

	virtual int run() {
		preCreate();

		window = createRenderingTarget(windowSize, windowPosition);

		if (!window) {
			std::cout << "Unable to create OpenGL window" << std::endl;
			return -1;
		}

		postCreate();

		initGl();

		while (!glfwWindowShouldClose(window)) {
			++frame;
			glfwPollEvents();
			update();
			draw();
			finishFrame();
		}

		shutdownGl();

		return 0;
	}


protected:
	virtual GLFWwindow * createRenderingTarget(uvec2 & size, ivec2 & pos) = 0;

	virtual void draw() = 0;

	void preCreate() {
		glfwWindowHint(GLFW_DEPTH_BITS, 16);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
	}


	void postCreate() {
		glfwSetWindowUserPointer(window, this);
		glfwSetKeyCallback(window, KeyCallback);
		glfwSetMouseButtonCallback(window, MouseButtonCallback);
		glfwMakeContextCurrent(window);

		// Initialize the OpenGL bindings
		// For some reason we have to set this experminetal flag to properly
		// init GLEW if we use a core context.
		glewExperimental = GL_TRUE;
		if (0 != glewInit()) {
			FAIL("Failed to initialize GLEW");
		}
		glGetError();

		if (GLEW_KHR_debug) {
			GLint v;
			glGetIntegerv(GL_CONTEXT_FLAGS, &v);
			if (v & GL_CONTEXT_FLAG_DEBUG_BIT) {
				//glDebugMessageCallback(glDebugCallbackHandler, this);
			}
		}
	}

	virtual void initGl() {
	}

	virtual void shutdownGl() {
	}

	virtual void finishFrame() {
		glfwSwapBuffers(window);
	}

	virtual void destroyWindow() {
		glfwSetKeyCallback(window, nullptr);
		glfwSetMouseButtonCallback(window, nullptr);
		glfwDestroyWindow(window);
	}

	virtual void onKey(int key, int scancode, int action, int mods) {
		if (GLFW_PRESS != action) {
			return;
		}

		switch (key) {
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, 1);
			return;
		}
	}

	virtual void update() {}

	virtual void onMouseButton(int button, int action, int mods) {}

protected:
	virtual void viewport(const ivec2 & pos, const uvec2 & size) {
		glViewport(pos.x, pos.y, size.x, size.y);
	}

private:

	static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		GlfwApp * instance = (GlfwApp *)glfwGetWindowUserPointer(window);
		instance->onKey(key, scancode, action, mods);
	}

	static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
		GlfwApp * instance = (GlfwApp *)glfwGetWindowUserPointer(window);
		instance->onMouseButton(button, action, mods);
	}

	static void ErrorCallback(int error, const char* description) {
		FAIL(description);
	}
};

//////////////////////////////////////////////////////////////////////
//
// The Oculus VR C API provides access to information about the HMD
//

#include <OVR_CAPI.h>
#include <OVR_CAPI_GL.h>

namespace ovr {

	// Convenience method for looping over each eye with a lambda
	template <typename Function>
	inline void for_each_eye(Function function) {
		for (ovrEyeType eye = ovrEyeType::ovrEye_Left;
			eye < ovrEyeType::ovrEye_Count;
			eye = static_cast<ovrEyeType>(eye + 1)) {
			function(eye);
		}
	}

	inline mat4 toGlm(const ovrMatrix4f & om) {
		return glm::transpose(glm::make_mat4(&om.M[0][0]));
	}

	inline mat4 toGlm(const ovrFovPort & fovport, float nearPlane = 0.01f, float farPlane = 10000.0f) {
		return toGlm(ovrMatrix4f_Projection(fovport, nearPlane, farPlane, true));
	}

	inline vec3 toGlm(const ovrVector3f & ov) {
		return glm::make_vec3(&ov.x);
	}

	inline vec2 toGlm(const ovrVector2f & ov) {
		return glm::make_vec2(&ov.x);
	}

	inline uvec2 toGlm(const ovrSizei & ov) {
		return uvec2(ov.w, ov.h);
	}

	inline quat toGlm(const ovrQuatf & oq) {
		return glm::make_quat(&oq.x);
	}

	inline mat4 toGlm(const ovrPosef & op) {
		mat4 orientation = glm::mat4_cast(toGlm(op.Orientation));
		mat4 translation = glm::translate(mat4(), ovr::toGlm(op.Position));
		return translation * orientation;
	}

	inline ovrMatrix4f fromGlm(const mat4 & m) {
		ovrMatrix4f result;
		mat4 transposed(glm::transpose(m));
		memcpy(result.M, &(transposed[0][0]), sizeof(float) * 16);
		return result;
	}

	inline ovrVector3f fromGlm(const vec3 & v) {
		ovrVector3f result;
		result.x = v.x;
		result.y = v.y;
		result.z = v.z;
		return result;
	}

	inline ovrVector2f fromGlm(const vec2 & v) {
		ovrVector2f result;
		result.x = v.x;
		result.y = v.y;
		return result;
	}

	inline ovrSizei fromGlm(const uvec2 & v) {
		ovrSizei result;
		result.w = v.x;
		result.h = v.y;
		return result;
	}

	inline ovrQuatf fromGlm(const quat & q) {
		ovrQuatf result;
		result.x = q.x;
		result.y = q.y;
		result.z = q.z;
		result.w = q.w;
		return result;
	}
}

class RiftManagerApp {
protected:
	ovrSession _session;
	ovrHmdDesc _hmdDesc;
	ovrGraphicsLuid _luid;

public:
	RiftManagerApp() {
		if (!OVR_SUCCESS(ovr_Create(&_session, &_luid))) {
			FAIL("Unable to create HMD session");
		}

		_hmdDesc = ovr_GetHmdDesc(_session);
	}

	~RiftManagerApp() {
		ovr_Destroy(_session);
		_session = nullptr;
	}
};

class RiftApp : public GlfwApp, public RiftManagerApp {
public:

private:
	GLuint _fbo{ 0 };
	GLuint _depthBuffer{ 0 };
	ovrTextureSwapChain _eyeTexture;

	GLuint _mirrorFbo{ 0 };
	ovrMirrorTexture _mirrorTexture;

	ovrEyeRenderDesc _eyeRenderDescs[2];

	mat4 _eyeProjections[2];

	ovrLayerEyeFov _sceneLayer;
	ovrViewScaleDesc _viewScaleDesc;

	uvec2 _renderTargetSize;
	uvec2 _mirrorSize;

	float defaultHmdToEyeOffset[2]; // 0.0294861

public:

	RiftApp() {
		using namespace ovr;
		_viewScaleDesc.HmdSpaceToWorldScaleInMeters = 1.0f;

		memset(&_sceneLayer, 0, sizeof(ovrLayerEyeFov));
		_sceneLayer.Header.Type = ovrLayerType_EyeFov;
		_sceneLayer.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;

		ovr::for_each_eye([&](ovrEyeType eye) {
			ovrEyeRenderDesc& erd = _eyeRenderDescs[eye] = ovr_GetRenderDesc(_session, eye, _hmdDesc.DefaultEyeFov[eye]);
			ovrMatrix4f ovrPerspectiveProjection =
				ovrMatrix4f_Projection(erd.Fov, 0.01f, 1000.0f, ovrProjection_ClipRangeOpenGL);
			_eyeProjections[eye] = ovr::toGlm(ovrPerspectiveProjection);
			_viewScaleDesc.HmdToEyeOffset[eye] = erd.HmdToEyeOffset;
			defaultHmdToEyeOffset[eye] = _viewScaleDesc.HmdToEyeOffset[eye].x;
			//std::cout << _viewScaleDesc.HmdToEyeOffset[eye].x << std::endl;

			ovrFovPort & fov = _sceneLayer.Fov[eye] = _eyeRenderDescs[eye].Fov;
			auto eyeSize = ovr_GetFovTextureSize(_session, eye, fov, 1.0f);
			_sceneLayer.Viewport[eye].Size = eyeSize;
			_sceneLayer.Viewport[eye].Pos = { (int)_renderTargetSize.x, 0 };

			_renderTargetSize.y = std::max(_renderTargetSize.y, (uint32_t)eyeSize.h);
			_renderTargetSize.x += eyeSize.w;
		});
		// Make the on screen window 1/4 the resolution of the render target
		_mirrorSize = _renderTargetSize;
		_mirrorSize /= 2;
	}

protected:
	GLFWwindow * createRenderingTarget(uvec2 & outSize, ivec2 & outPosition) override {
		return glfw::createWindow(_mirrorSize);
	}

	void initGl() override {
		GlfwApp::initGl();

		// Disable the v-sync for buffer swap
		glfwSwapInterval(0);

		ovrTextureSwapChainDesc desc = {};
		desc.Type = ovrTexture_2D;
		desc.ArraySize = 1;
		desc.Width = _renderTargetSize.x;
		desc.Height = _renderTargetSize.y;
		desc.MipLevels = 1;
		desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
		desc.SampleCount = 1;
		desc.StaticImage = ovrFalse;
		ovrResult result = ovr_CreateTextureSwapChainGL(_session, &desc, &_eyeTexture);
		_sceneLayer.ColorTexture[0] = _eyeTexture;
		if (!OVR_SUCCESS(result)) {
			FAIL("Failed to create swap textures");
		}

		int length = 0;
		result = ovr_GetTextureSwapChainLength(_session, _eyeTexture, &length);
		if (!OVR_SUCCESS(result) || !length) {
			FAIL("Unable to count swap chain textures");
		}
		for (int i = 0; i < length; ++i) {
			GLuint chainTexId;
			ovr_GetTextureSwapChainBufferGL(_session, _eyeTexture, i, &chainTexId);
			glBindTexture(GL_TEXTURE_2D, chainTexId);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}
		glBindTexture(GL_TEXTURE_2D, 0);

		// Set up the framebuffer object
		glGenFramebuffers(1, &_fbo);
		glGenRenderbuffers(1, &_depthBuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
		glBindRenderbuffer(GL_RENDERBUFFER, _depthBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, _renderTargetSize.x, _renderTargetSize.y);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthBuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

		ovrMirrorTextureDesc mirrorDesc;
		memset(&mirrorDesc, 0, sizeof(mirrorDesc));
		mirrorDesc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
		mirrorDesc.Width = _mirrorSize.x;
		mirrorDesc.Height = _mirrorSize.y;
		if (!OVR_SUCCESS(ovr_CreateMirrorTextureGL(_session, &mirrorDesc, &_mirrorTexture))) {
			FAIL("Could not create mirror texture");
		}
		glGenFramebuffers(1, &_mirrorFbo);
	}

	void onKey(int key, int scancode, int action, int mods) override {
		if (GLFW_PRESS == action) switch (key) {
		case GLFW_KEY_R:
			ovr_RecenterTrackingOrigin(_session);
			return;
		}

		GlfwApp::onKey(key, scancode, action, mods);
	}

	ovrPosef lastEye[2], renderEye[2];
	bool initLastEye[2] = {false, false};
	void draw() final override {
		ovrPosef eyePoses[2];
		ovrVector3f renderEyeOffset[ovrEye_Count];
		renderEyeOffset[0] = _viewScaleDesc.HmdToEyeOffset[0];
		renderEyeOffset[1] = _viewScaleDesc.HmdToEyeOffset[1];
		renderEyeOffset[0].x = std::min(std::max(defaultHmdToEyeOffset[0] - getIOD() / 2, -0.3f), 0.0f);
		renderEyeOffset[1].x = std::min(std::max(defaultHmdToEyeOffset[1] + getIOD() / 2, 0.0f), 0.3f);
		//std::cout << defaultHmdToEyeOffset[0] << " " << defaultHmdToEyeOffset[1] << std::endl;
 		//if (getViewState() == 1) { renderEyeOffset[0].x = renderEyeOffset[1].x = 0.0f; }
		/*
		if (getViewState() == 1) { 
			renderEyeOffset[0].x = (renderEyeOffset[0].x + renderEyeOffset[1].x) / 2.0f; 
			renderEyeOffset[0].y = (renderEyeOffset[0].y + renderEyeOffset[1].y) / 2.0f;
			renderEyeOffset[0].z = (renderEyeOffset[0].z + renderEyeOffset[1].z) / 2.0f;
			renderEyeOffset[1] = renderEyeOffset[0];
		}
		*/
		//std::cout << _viewScaleDesc.HmdToEyeOffset[0].x << " " << _viewScaleDesc.HmdToEyeOffset[1].x << std::endl;
		ovr_GetEyePoses(_session, frame, true, renderEyeOffset, eyePoses, &_sceneLayer.SensorSampleTime);

		int curIndex;
		ovr_GetTextureSwapChainCurrentIndex(_session, _eyeTexture, &curIndex);
		GLuint curTexId;
		ovr_GetTextureSwapChainBufferGL(_session, _eyeTexture, curIndex, &curTexId);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, curTexId, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ovr::for_each_eye([&](ovrEyeType eye) {
			// renderEye[eye] = eyePoses[eye];
			// /*
			if (!initLastEye[eye]) {
				lastEye[eye] = eyePoses[eye];
				initLastEye[eye] = true;
			}
			renderEye[eye] = lastEye[eye];
			if (getTrackingState() == 0) {
				renderEye[eye].Position = eyePoses[eye].Position;
			}
			lastEye[eye] = renderEye[eye];
			// */
			
			currentEye(eye);
			const auto& vp = _sceneLayer.Viewport[eye];
			glViewport(vp.Pos.x, vp.Pos.y, vp.Size.w, vp.Size.h);
			_sceneLayer.RenderPose[eye] = eyePoses[eye];
			glm::vec3 eyePos = glm::vec3(renderEye[eye].Position.x, renderEye[eye].Position.y, renderEye[eye].Position.z);
			offscreenRender(_eyeProjections[eye], ovr::toGlm(renderEye[eye]), _fbo, vp, eyePos);
			glm::vec3 origEyePos = glm::vec3(eyePoses[eye].Position.x, eyePoses[eye].Position.y, eyePoses[eye].Position.z);
			renderScene(_eyeProjections[eye], ovr::toGlm(eyePoses[eye]), origEyePos);
			
			//*/
			/*
			currentEye(eye);
			const auto& vp = _sceneLayer.Viewport[eye];
			glViewport(vp.Pos.x, vp.Pos.y, vp.Size.w, vp.Size.h);
			_sceneLayer.RenderPose[eye] = eyePoses[eye];
			if (eye == ovrEye_Left && getViewState() == 3) return;
			if (eye == ovrEye_Right && getViewState() == 2) return;
			renderScene(_eyeProjections[eye], ovr::toGlm(renderEye[eye]));
			*/
		});
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		ovr_CommitTextureSwapChain(_session, _eyeTexture);
		ovrLayerHeader* headerList = &_sceneLayer.Header;
		ovr_SubmitFrame(_session, frame, &_viewScaleDesc, &headerList, 1);

		GLuint mirrorTextureId;
		ovr_GetMirrorTextureBufferGL(_session, _mirrorTexture, &mirrorTextureId);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, _mirrorFbo);
		glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mirrorTextureId, 0);
		glBlitFramebuffer(0, 0, _mirrorSize.x, _mirrorSize.y, 0, _mirrorSize.y, _mirrorSize.x, 0, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	}
	float getDefaultIOD(int idx) { return defaultHmdToEyeOffset[idx]; }

	virtual void offscreenRender(const glm::mat4 & projection, const glm::mat4 & headPose, GLuint _fbo, const ovrRecti & vp, const glm::vec3 & eyePos) = 0;
	virtual void renderScene(const glm::mat4 & projection, const glm::mat4 & headPose, const glm::vec3 & eyePos) = 0;
	virtual void currentEye(ovrEyeType eye) = 0;
	virtual int getViewState() = 0;
	virtual int getTrackingState() = 0;
	virtual float getIOD() = 0;
	virtual float getCubeSize() = 0;
};

//////////////////////////////////////////////////////////////////////
//
// The remainder of this code is specific to the scene we want to 
// render.  I use oglplus to render an array of cubes, but your 
// application would perform whatever rendering you want
//


#include <time.h>
#include "Shader.h"
#include "Cube.h"
#include "Skybox.h"
#include "Cave.h"
#include "Line.h"
struct SimScene {
	Cave * cave;
	Cube * cube;
	Skybox * skybox;
	Skybox * riftskybox;
	Line * linel1;
	Line * linel2;
	Line * linel3;
	Line * linel4;
	Line * linel5;
	Line * linel6;
	Line * linel7;
	Line * liner1;
	Line * liner2;
	Line * liner3;
	Line * liner4;
	Line * liner5;
	Line * liner6;
	Line * liner7;
	GLint cubeShaderProgram, skyboxShaderProgram, lineShaderProgram;

	bool buttonAPressed = false, buttonBPressed = false, buttonXPressed = false, rightHandTriggerPressed = false;
	int buttonA = 0, buttonB = 0, buttonX = 0;
	float IOD = 0.0f, cubeSize = 0.03f, cubeX = 0.0f, cubeZ = -0.5f;
	int random_num = rand() % 6;
	bool randomGened = false;

#define CUBE_VERTEX_SHADER_PATH "C:/Users/degu/Desktop/CSE190Project3/Minimal/shader.vert"
#define CUBE_FRAGMENT_SHADER_PATH "C:/Users/degu/Desktop/CSE190Project3/Minimal/shader.frag"

#define SKYBOX_VERTEX_SHADER_PATH "C:/Users/degu/Desktop/CSE190Project3/Minimal/skybox.vert"
#define SKYBOX_FRAGMENT_SHADER_PATH "C:/Users/degu/Desktop/CSE190Project3/Minimal/skybox.frag"

#define LINE_VERTEX_SHADER_PATH "C:/Users/degu/Desktop/CSE190Project3/Minimal/LineShader.vert"
#define LINE_FRAGMENT_SHADER_PATH "C:/Users/degu/Desktop/CSE190Project3/Minimal/LineShader.frag"

public:
	static glm::mat4 P; // P for projection
	static glm::mat4 V; // V for view
	int curEyeIdx;
	GLuint lFBO, lrenderedTexture, lRBO;
	GLuint rFBO, rrenderedTexture, rRBO;
	GLuint bFBO, brenderedTexture, bRBO;

	SimScene() {
		srand(time(0));
		cubeShaderProgram = LoadShaders(CUBE_VERTEX_SHADER_PATH, CUBE_FRAGMENT_SHADER_PATH);
		skyboxShaderProgram = LoadShaders(SKYBOX_VERTEX_SHADER_PATH, SKYBOX_FRAGMENT_SHADER_PATH);
		lineShaderProgram = LoadShaders(LINE_VERTEX_SHADER_PATH, LINE_FRAGMENT_SHADER_PATH);

		// left
		glGenFramebuffers(1, &lFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, lFBO);

		glGenTextures(1, &lrenderedTexture);
		glBindTexture(GL_TEXTURE_2D, lrenderedTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2048, 2048, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lrenderedTexture, 0);

		glGenRenderbuffers(1, &lRBO);
		glBindRenderbuffer(GL_RENDERBUFFER, lRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 2048, 2048);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, // 1. fbo target: GL_FRAMEBUFFER
			GL_DEPTH_ATTACHMENT, // 2. attachment point
			GL_RENDERBUFFER, // 3. rbo target: GL_RENDERBUFFER
			lRBO); // 4. rbo ID

		//right
		glGenFramebuffers(1, &rFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, rFBO);

		glGenTextures(1, &rrenderedTexture);
		glBindTexture(GL_TEXTURE_2D, rrenderedTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2048, 2048, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, rrenderedTexture, 0);

		glGenRenderbuffers(1, &rRBO);
		glBindRenderbuffer(GL_RENDERBUFFER, rRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 2048, 2048);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, // 1. fbo target: GL_FRAMEBUFFER
			GL_DEPTH_ATTACHMENT, // 2. attachment point
			GL_RENDERBUFFER, // 3. rbo target: GL_RENDERBUFFER
			rRBO); // 4. rbo ID

		// bottom
		glGenFramebuffers(1, &bFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, bFBO);

		glGenTextures(1, &brenderedTexture);
		glBindTexture(GL_TEXTURE_2D, brenderedTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2048, 2048, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brenderedTexture, 0);

		glGenRenderbuffers(1, &bRBO);
		glBindRenderbuffer(GL_RENDERBUFFER, bRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 2048, 2048);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, // 1. fbo target: GL_FRAMEBUFFER
			GL_DEPTH_ATTACHMENT, // 2. attachment point
			GL_RENDERBUFFER, // 3. rbo target: GL_RENDERBUFFER
			bRBO); // 4. rbo ID

		cave = new Cave();
		//cave->toWorld = glm::mat4(1.0f);
		cave->toWorld = glm::rotate(glm::mat4(1.0f), -0.785398f, glm::vec3(0.0f, 1.0f, 0.0f));
		skybox = new Skybox();
		skybox->toWorld = glm::mat4(1.0f);
		riftskybox = new Skybox();
		riftskybox->toWorld = glm::mat4(1.0f);
		riftskybox->useCubemap(2);
		cube = new Cube();
		cube->toWorld = glm::translate(glm::mat4(1.0f), glm::vec3(cubeX, 0.0f, cubeZ)) * glm::scale(glm::mat4(1.0f), glm::vec3(cubeSize, cubeSize, cubeSize));
		linel1 = new Line();
		linel2 = new Line();
		linel3 = new Line();
		linel4 = new Line();
		linel5 = new Line();
		linel6 = new Line();
		linel7 = new Line();
		liner1 = new Line();
		liner2 = new Line();
		liner3 = new Line();
		liner4 = new Line();
		liner5 = new Line();
		liner6 = new Line();
		liner7 = new Line();
	}

	void update() {
		cube->toWorld = glm::translate(glm::mat4(1.0f), glm::vec3(cubeX, 0.0f, cubeZ)) * glm::scale(glm::mat4(1.0f), glm::vec3(cubeSize, cubeSize, cubeSize));
	}

	void preRender(const glm::mat4 & projection, const glm::mat4 & modelview, GLuint _fbo, const ovrRecti & vp, const glm::vec3 & eyePos) {
		// render scene to texture
		//------------------------left
		glBindFramebuffer(GL_FRAMEBUFFER, lFBO);
		glViewport(0, 0, 2048, 2048);
		glClearColor(0.f, 0.f, 0.f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		float nearPlane = 0.01f, farPlane = 1000.0f;
		//if (buttonX == 1 && curEyeIdx == 0) hasLeft = true;
		//if (buttonX == 1 && curEyeIdx == 1) hasRight = true;
		if (buttonX == 1 && !randomGened) {
			random_num = rand() % 6;
			randomGened = true;
		}

		vec3 pa = glm::vec3(cave->toWorld * vec4(-2.0f, -2.0f, 2.0f, 1.0f));
		vec3 pb = glm::vec3(cave->toWorld * vec4(-2.0f, -2.0f, -2.0f, 1.0f));
		vec3 pc = glm::vec3(cave->toWorld * vec4(-2.0f, 2.0f, 2.0f, 1.0f));
		if (buttonX == 0 || curEyeIdx * 3 != random_num) {
			glUseProgram(skyboxShaderProgram);
			skybox->draw(skyboxShaderProgram, getProjection(eyePos, pa, pb, pc, nearPlane, farPlane), modelview);
			glUseProgram(cubeShaderProgram);
			cube->draw(cubeShaderProgram, getProjection(eyePos, pa, pb, pc, nearPlane, farPlane), modelview);
		}
		// line update
		if (curEyeIdx == 0) {
			linel1->update(pc, eyePos, false);
			linel2->update(pa, eyePos, false);
		}
		else {
			liner1->update(pc, eyePos, true);
			liner2->update(pa, eyePos, true);
		}

		//----------------------right
		glBindFramebuffer(GL_FRAMEBUFFER, rFBO);
		glViewport(0, 0, 2048, 2048);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		pa = glm::vec3(cave->toWorld * vec4(-2.0f, -2.0f, -2.0f, 1.0f));
		pb = glm::vec3(cave->toWorld * vec4(2.0f, -2.0f, -2.0f, 1.0f));
		pc = glm::vec3(cave->toWorld * vec4(-2.0f, 2.0f, -2.0f, 1.0f));
		if (buttonX == 0 || curEyeIdx * 3 + 1 != random_num) {
			glUseProgram(skyboxShaderProgram);
			skybox->draw(skyboxShaderProgram, getProjection(eyePos, pa, pb, pc, nearPlane, farPlane), modelview);
			glUseProgram(cubeShaderProgram);
			cube->draw(cubeShaderProgram, getProjection(eyePos, pa, pb, pc, nearPlane, farPlane), modelview);
		}
		// line update
		if (curEyeIdx == 0) {
			// linel3->update(pc, eyePos, false);
			linel3->update(pc, eyePos, false);
			linel4->update(pa, eyePos, false);
			linel5->update(pb + (pc - pa), eyePos, false);
			linel6->update(pb, eyePos, false);
		}
		else {
			liner3->update(pc, eyePos, true);
			liner4->update(pa, eyePos, true);
			liner5->update(pb + (pc - pa), eyePos, true);
			liner6->update(pb, eyePos, true);
		}

		//-------------------------bottom
		glBindFramebuffer(GL_FRAMEBUFFER, bFBO);
		glViewport(0, 0, 2048, 2048);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		pa = glm::vec3(cave->toWorld * vec4(-2.0f, -2.0f, 2.0f, 1.0f));
		pb = glm::vec3(cave->toWorld * vec4(2.0f, -2.0f, 2.0f, 1.0f));
		pc = glm::vec3(cave->toWorld * vec4(-2.0f, -2.0f, -2.0f, 1.0f));
		if (buttonX == 0 || curEyeIdx * 3 + 2 != random_num) {
			glUseProgram(skyboxShaderProgram);
			skybox->draw(skyboxShaderProgram, getProjection(eyePos, pa, pb, pc, nearPlane, farPlane), modelview);
			glUseProgram(cubeShaderProgram);
			cube->draw(cubeShaderProgram, getProjection(eyePos, pa, pb, pc, nearPlane, farPlane), modelview);
		}
		// line update
		if (curEyeIdx == 0) {
			linel7->update(pb, eyePos, false);
		}
		else {
			liner7->update(pb, eyePos, true);
		}

		// restore fbo
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
		glViewport(vp.Pos.x, vp.Pos.y, vp.Size.w, vp.Size.h);
	}

	glm::mat4 getProjection(glm::vec3 eyePos, glm::vec3 pa, glm::vec3 pb, glm::vec3 pc, float n, float f) {
		vec3 vr = glm::normalize(pb - pa);
		vec3 vu = glm::normalize(pc - pa);
		vec3 vn = glm::normalize(glm::cross(vr, vu));
		vec3 va = pa - eyePos;
		vec3 vb = pb - eyePos;
		vec3 vc = pc - eyePos;
		float d = -glm::dot(vn, va);
		float l = glm::dot(vr, va) * n / d;
		float r = glm::dot(vr, vb) * n / d;
		float b = glm::dot(vu, va) * n / d;
		float t = glm::dot(vu, vc) * n / d;

		glm::mat4 P = glm::mat4(2 * n / (r - l), 0.0f, 0.0f, 0.0f,
								0.0f, 2 * n / (t - b), 0.0f, 0.0f,
								(r + l) / (r - l), (t + b) / (t - b), -(f + n) / (f - n), -1.0f,
								0.0f, 0.0f, -2 * f*n / (f - n), 0.0f);
		P = glm::frustum(l, r, b, t, n, f);
		glm::mat4 M = glm::mat4(vr.x, vr.y, vr.z, 0.0f,
								vu.x, vu.y, vu.z, 0.0f, 
								vn.x, vn.y, vn.z, 0.0f, 
								0.0f, 0.0f, 0.0f, 1.0f);
		glm::mat4 T = glm::translate(glm::vec3(-eyePos.x, -eyePos.y, -eyePos.z));
		return P*glm::transpose(M)*T;
	}

	void render(const mat4 & projection, const mat4 & modelview, const glm::vec3 & eyePos) {
		// render texture to cave
		glUseProgram(skyboxShaderProgram);
		riftskybox->draw(skyboxShaderProgram, projection, modelview);
		glUseProgram(cubeShaderProgram);
		cave->draw(cubeShaderProgram, projection, modelview, lrenderedTexture, rrenderedTexture, brenderedTexture);
		glUseProgram(lineShaderProgram);
		/*
		vec3 pc = glm::vec3(cave->toWorld * vec4(-2.0f, 2.0f, 2.0f, 1.0f));
		if (curEyeIdx == 0) {
			//linel1->update(pc, eyePos, false);
		}
		*/
		linel1->draw(lineShaderProgram, projection, modelview);
		linel2->draw(lineShaderProgram, projection, modelview);
		linel3->draw(lineShaderProgram, projection, modelview);
		linel4->draw(lineShaderProgram, projection, modelview);
		linel5->draw(lineShaderProgram, projection, modelview);
		linel6->draw(lineShaderProgram, projection, modelview);
		linel7->draw(lineShaderProgram, projection, modelview);
		liner1->draw(lineShaderProgram, projection, modelview);
		liner2->draw(lineShaderProgram, projection, modelview);
		liner3->draw(lineShaderProgram, projection, modelview);
		liner4->draw(lineShaderProgram, projection, modelview);
		liner5->draw(lineShaderProgram, projection, modelview);
		liner6->draw(lineShaderProgram, projection, modelview);
		liner7->draw(lineShaderProgram, projection, modelview);
	}

	void currentEye(int eyeIdx) {
		curEyeIdx = eyeIdx;
		//cave->useCubemap(curEyeIdx);
		skybox->useCubemap(curEyeIdx);
	}

private:

};

class SimApp : public RiftApp {
	std::shared_ptr<SimScene> simScene;

public:
	SimApp() {}
	glm::mat4 lastHeadPose;
	glm::mat4 rightHandPose;
	glm::vec3 triggerPose;
	glm::mat4 lastRightHand;
protected:

	void initGl() override {
		RiftApp::initGl();
		// Enable depth buffering
		glEnable(GL_DEPTH_TEST);
		// Related to shaders and z value comparisons for the depth buffer
		glDepthFunc(GL_LEQUAL);
		// Set polygon drawing mode to fill front and back of each polygon
		// You can also use the paramter of GL_LINE instead of GL_FILL to see wireframes
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		// Disable backface culling to render both sides of polygons
		glDisable(GL_CULL_FACE);
		// Set clear color
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		ovr_RecenterTrackingOrigin(_session);
		simScene = std::shared_ptr<SimScene>(new SimScene());
	}

	void shutdownGl() override {
	}

	void update() override {
		ovrInputState inputState;
		double displayMidpointSeconds = ovr_GetPredictedDisplayTime(_session, frame);
		ovrTrackingState trackState = ovr_GetTrackingState(_session, displayMidpointSeconds, ovrTrue);
		if (OVR_SUCCESS(ovr_GetInputState(_session, ovrControllerType_Touch, &inputState))) {
			if (inputState.Buttons & ovrButton_A) simScene->buttonAPressed = true;
			else if (simScene->buttonAPressed) {
				simScene->buttonA = (simScene->buttonA + 1) % 4; simScene->buttonAPressed = false;
			}
			if (inputState.Buttons & ovrButton_B) simScene->buttonBPressed = true;
			else if (simScene->buttonBPressed) {
				simScene->buttonB = (simScene->buttonB + 1) % 2; simScene->buttonBPressed = false;
			}
			if (inputState.Buttons & ovrButton_X) simScene->buttonXPressed = true;
			else if (simScene->buttonXPressed) {
				simScene->buttonX = (simScene->buttonX + 1) % 2; simScene->buttonXPressed = false, simScene->randomGened = false;
			}

			if (inputState.HandTrigger[ovrHand_Right] > 0.5f) simScene->rightHandTriggerPressed = true;
			else simScene->rightHandTriggerPressed = false;

			ovrPosef rightPose = trackState.HandPoses[ovrHand_Right].ThePose;
			rightHandPose = ovr::toGlm(rightPose);
			triggerPose = glm::vec3(rightPose.Position.x, rightPose.Position.y, rightPose.Position.z);

			if (inputState.Buttons & ovrButton_RThumb) {
				simScene->cubeX = 0.0f;
				simScene->cubeZ = -0.5f;
			}
			else {
				if (inputState.Thumbstick[ovrHand_Right].x > 0.5f) simScene->cubeX += 0.001f;
				else if (inputState.Thumbstick[ovrHand_Right].x < -0.5f) simScene->cubeX -= 0.001f;
				if (inputState.Thumbstick[ovrHand_Right].y > 0.5f) simScene->cubeZ -= 0.001f;
				else if (inputState.Thumbstick[ovrHand_Right].y < -0.5f) simScene->cubeZ += 0.001f;
			}
			if (inputState.Buttons & ovrButton_LThumb) simScene->cubeSize = 0.03f;
			else {
				if (inputState.Thumbstick[ovrHand_Left].x > 0.5f) simScene->cubeSize = std::min(simScene->cubeSize + 0.001f, 0.1f);
				else if (inputState.Thumbstick[ovrHand_Left].x < -0.5f) simScene->cubeSize = std::max(simScene->cubeSize - 0.001f, 0.001f);
			}
		}
		simScene->update();
	}

	void offscreenRender(const glm::mat4 & projection, const glm::mat4 & headPose, GLuint _fbo, const ovrRecti & vp, const glm::vec3 & eyePos) {
		if (simScene->rightHandTriggerPressed) {
			glm::mat4 no_rot = glm::mat4(1.0f);
			if (getTrackingState() == 0) {
				no_rot[3] = rightHandPose[3];
				//no_rot = rightHandPose;
				lastRightHand = rightHandPose;
			}
			else {
				no_rot[3] = lastRightHand[3];
			}
			vec3 adjustedEyePose = glm::vec3(no_rot[3][0], no_rot[3][1], no_rot[3][2]);
			adjustedEyePose.x += getDefaultIOD(simScene->curEyeIdx);
			simScene->preRender(projection, glm::inverse(no_rot), _fbo, vp, adjustedEyePose);
		}
		else {
			simScene->preRender(projection, glm::inverse(headPose), _fbo, vp, eyePos);
		}
	}

	void renderScene(const glm::mat4 & projection, const glm::mat4 & headPose, const glm::vec3 & eyePos) override {
		simScene->render(projection, glm::inverse(headPose), eyePos);
	}

	void currentEye(ovrEyeType eye) {
		if (eye == ovrEye_Left) {
			simScene->currentEye(0);
		}
		else {
			simScene->currentEye(1);
		}
	}
	int getViewState() { return simScene->buttonA; }
	int getTrackingState() { return simScene->buttonB; }
	float getIOD() { return simScene->IOD; }
	float getCubeSize() { return simScene->cubeSize; }
};

// Execute our example class
int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	int result = -1;
	try {
		if (!OVR_SUCCESS(ovr_Initialize(nullptr))) {
			FAIL("Failed to initialize the Oculus SDK");
		}
		AllocConsole();
		freopen("conin$", "r", stdin);
		freopen("conout$", "w", stdout);
		freopen("conout$", "w", stderr);
		result = SimApp().run();
	}
	catch (std::exception & error) {
		OutputDebugStringA(error.what());
		std::cerr << error.what() << std::endl;
	}
	ovr_Shutdown();
	return result;
}