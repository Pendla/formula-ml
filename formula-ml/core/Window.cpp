#include "Window.h"

#include <glm/glm.hpp>

#include <iostream>

#include <core/Keyboard.h>
#include <core/util/Util.h>
#include <core/util/GLUtil.h>
#include <core/util/ImageIO.h>

Window::Window() {
	
	if (!glfwInit()) {
		throw std::runtime_error("GLFW Initialization failed.");
	}
	
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	window = glfwCreateWindow(1280, 720, "Formula ML", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		throw std::runtime_error("Window creation failed.");
	}
	glfwMakeContextCurrent(window);

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		throw std::runtime_error("glewInit failed");
	}
	glfwSetKeyCallback(window, key_callback);

	glfwGetFramebufferSize(window, &screen_width, &screen_height);
    fbo = gfx::create_framebuffer(screen_width, screen_height, true);
	
}

Window::~Window() {
}

void Window::setState(WindowState * s) {
	state = s;
}

void Window::run() {
    glClearColor(0.f, 0.f, 0.f, 1.0f);
	
    glBindFramebuffer(GL_FRAMEBUFFER, fbo.id);
    glViewport(0, 0, screen_width, screen_height);

    long last_frame = util::current_time();
    int fps = 0;
    long fps_timer = 0;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    glutil::gl_error_check("BEFORE SET LINE WIDTH");
    glLineWidth(1.0f);
    glutil::gl_error_check("AFTER SET LINE WIDTH");

	while (!glfwWindowShouldClose(window)) {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo.id);
        glViewport(0, 0, screen_width, screen_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        long dt = util::current_time() - last_frame;
        last_frame = util::current_time();

        fps++;
        fps_timer += dt;
        if (fps_timer >= 1000) {
            //printf("%i \n", fps);
            fps = 0;
            fps_timer -= 1000;
        }
        if (state) {
			//state->update((float)dt / 1000);
            state->run(0.01f);
		}
        
        //Blit the framebuffer to screen.
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);   
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo.id);
        glBlitFramebuffer(0, 0, fbo.width, fbo.height, 0, 0, fbo.width, fbo.height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        
		glfwSwapBuffers(window);
		glfwPollEvents();
        glfwSwapInterval(1);

        glutil::gl_error_check("WINDOW FRAME END");
	}

	glfwTerminate();

}