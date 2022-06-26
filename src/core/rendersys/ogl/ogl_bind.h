#pragma once
#include <windows.h>
#include <glad/glad.h>
#include "core/rendersys/ogl/ogl_utils.h"

namespace mir {

struct BindVaoScope {
	BindVaoScope(GLuint id) {
		CheckHR(glBindVertexArray(id));
	}
	~BindVaoScope() {
		CheckHR(glBindVertexArray(0));
	}
};

struct BindVboScope {
	BindVboScope(GLuint id) {
		CheckHR(glBindBuffer(GL_ARRAY_BUFFER, id));
	}
	~BindVboScope() {
		CheckHR(glBindBuffer(GL_ARRAY_BUFFER, 0));
	}
};

struct BindVioScope {
	BindVioScope(GLuint id) {
		CheckHR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id));
	}
	~BindVioScope() {
		CheckHR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
	}
};

struct BindUboScope {
	BindUboScope(GLuint id) {
		CheckHR(glBindBuffer(GL_UNIFORM_BUFFER, id));
	}
	~BindUboScope() {
		CheckHR(glBindBuffer(GL_UNIFORM_BUFFER, 0));
	}
};

struct BindTexture {
	GLenum mTarget;
	BindTexture(GLenum target, GLuint id) {
		mTarget = target;
		CheckHR(glBindTexture(mTarget, id));
	}
	~BindTexture() {
		CheckHR(glBindTexture(mTarget, 0));
	}
};

struct BindFrameBuffer {
	GLenum mTarget;
	BindFrameBuffer(GLenum target, GLuint id) {
		mTarget = target;
		CheckHR(glBindFramebuffer(mTarget, id));
	}
	~BindFrameBuffer() {
		CheckHR(glBindFramebuffer(mTarget, 0));
	}
};

}
