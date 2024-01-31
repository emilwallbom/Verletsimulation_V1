// window.h
#pragma once
extern GLFWwindow *window;
// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;
int window_init();
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
