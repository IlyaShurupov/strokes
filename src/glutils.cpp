
#include "glutils.h"

#include "shader.h"

#include "shader.h"

#include "map.h"
#include "strings.h"

#include "SOIL/soil.h"
//#include <lunasvg.h>

//using namespace lunasvg;

struct texture_drawer_data {

	HashMap<GLuint, string> textures;

	GLuint quad_VertexArrayID;
	GLuint quad_vertexbuffer;
	GLuint texID;
	GLuint rect_mat;
	ogl::shader shader;

	const GLfloat g_quad_vertex_buffer_data[18] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		1.0f,  1.0f, 0.0f,
	};

	texture_drawer_data() : shader("Passthrough", NULL, "Texture") {

		// The fullscreen quad's FBO
		glGenVertexArrays(1, &quad_VertexArrayID);
		glBindVertexArray(quad_VertexArrayID);

		glGenBuffers(1, &quad_vertexbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);

		texID = shader.getu("renderedTexture");
		rect_mat = shader.getu("RectMat");
	}

	~texture_drawer_data() {
		glDeleteBuffers(1, &quad_vertexbuffer);
		glDeleteVertexArrays(1, &quad_VertexArrayID);

		for (auto tex : textures) {
			glDeleteTextures(1, &tex->val);
		}
	}
};

texture_drawer_data* texdd = NULL;

void init_utils() {
	if (!texdd) texdd = new texture_drawer_data();
}

void finalize_utils() {
	if (texdd) delete texdd;
}

glm::mat4 get_rect_transform_mat(const glm::vec4& target, const glm::vec2& domen_size) {

	float scale_x = (target.z - target.x) / domen_size.x;
	float scale_y = (target.w - target.y) / domen_size.y;

	float move_x = (target.x / domen_size.x) - (1 - scale_x);
	float move_y = (target.y / domen_size.y) - (1 - scale_y);

	glm::mat4 out = glm::mat4(1.f);

	out[3][0] = move_x;
	out[3][1] = move_y;

	out[0][0] = scale_x;
	out[1][1] = scale_y;

	return out;
}

void draw_texture(GLuint out, GLuint in, const vec4& rec_domen, const vec4& rec_target) {

	assert(in);

	// Render to the screen
	glBindFramebuffer(GL_FRAMEBUFFER, out ? out : 0);

	glViewport(rec_target.x, rec_target.y, rec_target.z, rec_target.w); // Render on the whole framebuffer, complete from the lower left corner to the upper right

	// Use our shader
	texdd->shader.bind();

	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, in);
	// Set our "renderedTexture" sampler to use Texture Unit 0
	glUniform1i(texdd->texID, 0);

	glm::mat4 tmat = 0? get_rect_transform_mat(rec_target, rec_domen) : mat4(1);

	glUniformMatrix4fv(texdd->rect_mat, 1, GL_FALSE, &tmat[0][0]);

	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, texdd->quad_vertexbuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// Draw the triangles. 2*3 indices starting at 0 -> 2 triangles
	glDrawArrays(GL_TRIANGLES, 0, 6); 

	glDisableVertexAttribArray(0);

	texdd->shader.unbind();
}

GLuint load_texture(string name) {
	GLuint tex_2d = 0;

	if (0) {
		//auto document = Document::loadFromFile("tiger.svg");
		//auto bitmap = document->renderToBitmap();
	}
	else {
		tex_2d = SOIL_load_OGL_texture(name.cstr(), SOIL_LOAD_RGBA, SOIL_CREATE_NEW_ID,
			SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);

		if (0 == tex_2d) {
			printf("SOIL loading error: '%s'\n", SOIL_last_result());
		}
	}

	return tex_2d;
}

GLuint get_tex(const char* TexId) {
	GLuint out = 0;
	alni idx = texdd->textures.Presents(TexId);
	if (idx != -1) {
		out = texdd->textures.Get(TexId);
	}
	else {
		out = load_texture(TexId);
		texdd->textures.Put(TexId, out);
	}
	return out;
}