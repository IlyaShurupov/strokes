

#include "controller.h"

void camera_controller(GLFWwindow* window, camera* cam) {

	glm::vec<2, double, glm::defaultp> mousepos;
	glfwGetCursorPos(window, &mousepos.x, &mousepos.y);

	static glm::vec<2, double, glm::defaultp> prev_pos = mousepos;

	vec2 delta = prev_pos - mousepos;
	float degree_X = delta.x;
	float degree_Y = delta.y;

	// Move forward
	vec3 forward = normalize(cam->target - cam->pos);
	vec3 right = normalize(cross(forward, vec3(0, 1, 0)));
	vec3 up = normalize(cross(right, forward));

	vec3 rot_axis;

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_3) == GLFW_PRESS) {
		glm::mat4 rot_mat = glm::rotate(glm::mat4(1.0f), glm::radians(-degree_Y), right);
		cam->pos = vec3(vec4(cam->pos, 1.0f) * rot_mat);

		rot_mat = glm::rotate(glm::mat4(1.0f), glm::radians(-degree_X), vec3(0, 1, 0));
		cam->pos = vec3(vec4(cam->pos, 1.0f) * rot_mat);
	}

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS) {
		cam->pos += delta.y * (forward * glm::vec3(0.03));
	}

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		vec3 move = right * vec3(0.03 * delta.x) + up * vec3(0.03 * -delta.y);
		cam->pos += move;
		cam->target += move;
	}

	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
		cam->reset();
	}

	prev_pos = mousepos;
}