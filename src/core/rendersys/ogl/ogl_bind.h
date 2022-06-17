#pragma once
#include <windows.h>
#include <glad/glad.h>

namespace mir {

struct BindVaoScope {
	BindVaoScope(GLuint id) {
		glBindVertexArray(id);
	}
	~BindVaoScope() {
		glBindVertexArray(0);
	}
};

struct BindVboScope {
	BindVboScope(GLuint id) {
		glBindBuffer(GL_ARRAY_BUFFER, id);
	}
	~BindVboScope() {
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
};

struct BindVioScope {
	BindVioScope(GLuint id) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
	}
	~BindVioScope() {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
};

struct BindUboScope {
	BindUboScope(GLuint id) {
		glBindBuffer(GL_UNIFORM_BUFFER, id);
	}
	~BindUboScope() {
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
};

struct BindTexture {
	GLenum mTarget;
	BindTexture(GLenum target, GLuint id) {
		mTarget = target;
		glBindTexture(mTarget, id);
	}
	~BindTexture() {
		glBindTexture(mTarget, 0);
	}
};

struct BindFrameBuffer {
	GLenum mTarget;
	BindFrameBuffer(GLenum target, GLuint id) {
		mTarget = target;
		glBindFramebuffer(mTarget, id);
	}
	~BindFrameBuffer() {
		glBindFramebuffer(mTarget, 0);
	}
};

}
