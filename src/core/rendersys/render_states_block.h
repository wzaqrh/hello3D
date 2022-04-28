#pragma once
#include "core/base/declare_macros.h"
#include "core/rendersys/render_system.h"

namespace mir {
	
struct FrameBufferBlock {
	FrameBufferBlock(RenderSystem& renderSys) :mRenderSys(renderSys) {}
	void Push(IFrameBufferPtr fb) {
		mStack.push_back(mCurrent);
		if (fb) mRenderSys.SetFrameBuffer(mCurrent = fb);
	}
	void Push(IFrameBufferPtr fb, const Eigen::Vector4f& color, float depth, uint8_t stencil) {
		Push(fb);
		mRenderSys.ClearFrameBuffer(fb, color, depth, stencil);
	}
	void Pop() {
		BOOST_ASSERT(!mStack.empty());
		mRenderSys.SetFrameBuffer(mCurrent = mStack.back());
		mStack.pop_back();
	}
	struct Lock : boost::noncopyable {
		TemplateArgs Lock(FrameBufferBlock& block, T &&...args) :mBlock(block) { mBlock.Push(std::forward<T>(args)...); }
		Lock(Lock&& other) :mBlock(other.mBlock) { other.mOwn = false; }
		~Lock() { if (mOwn) mBlock.Pop(); }
	private:
		FrameBufferBlock& mBlock;
		bool mOwn = true;
	};
private:
	RenderSystem& mRenderSys;
	IFrameBufferPtr mCurrent;
	std::vector<IFrameBufferPtr> mStack;
};

struct BlendStateBlock {
	BlendStateBlock(RenderSystem& renderSys) :mRenderSys(renderSys), mCurrent(renderSys.GetBlendState()) {}
	void Set(const BlendState& state) {
		if (mCurrent != state) {
			mCurrent = state;
			mRenderSys.SetBlendState(state);
		}
	}
	void operator()(const BlendState& state) { Set(state); }
	const BlendState& Get() const { return mCurrent; }
	struct Lock : boost::noncopyable {
		Lock(BlendStateBlock& block) :mBlock(block), mState(block.Get()) {}
		Lock(BlendStateBlock& block, const BlendState& state) :mBlock(block), mState(state) { block.Set(state); }
		Lock(Lock&& other) :mBlock(other.mBlock) ,mState(other.mState) { other.mOwn = false; }
		~Lock() { if (mOwn) mBlock.Set(mState); }
		Lock& operator=(Lock&& other) = delete;
		BlendStateBlock* operator->() { return &mBlock; }
		void operator()(const BlendState& state) { mBlock.Set(state); }
	private:
		BlendState mState;
		BlendStateBlock& mBlock;
		bool mOwn = true;
	};
private:
	RenderSystem& mRenderSys;
	BlendState mCurrent;
};

struct DepthStateBlock {
	DepthStateBlock(RenderSystem& renderSys) :mRenderSys(renderSys), mCurrent(renderSys.GetDepthState()) {}
	void Set(const DepthState& state) {
		if (mCurrent != state) {
			mCurrent = state;
			mRenderSys.SetDepthState(state);
		}
	}
	void operator()(const DepthState& state) { Set(state); }
	const DepthState& Get() const { return mCurrent; }
	struct Lock : boost::noncopyable {
		Lock(DepthStateBlock& block) :mBlock(block), mState(block.Get()) {}
		Lock(DepthStateBlock& block, const DepthState& state) :mBlock(block), mState(state) { block.Set(state); }
		Lock(Lock&& other) :mBlock(other.mBlock) ,mState(other.mState) { other.mOwn = false; }
		~Lock() { if (mOwn) mBlock.Set(mState); }
		Lock& operator=(Lock&& other) = delete;
		DepthStateBlock* operator->() { return &mBlock; }
		void operator()(const DepthState& state) { mBlock.Set(state); }
	private:
		DepthState mState;
		DepthStateBlock& mBlock;
		bool mOwn = true;
	};
private:
	RenderSystem& mRenderSys;
	DepthState mCurrent;
};

struct TexturesBlock {
	TexturesBlock(RenderSystem& renderSys) :mRenderSys(renderSys) {}
	void Set(size_t slot, ITexturePtr state) {
		BOOST_ASSERT(slot < 16);
		if (mTextures[slot] != state) {
			mTextures[slot] = state;
			mRenderSys.SetTexture(slot, state);
		}
	}
	void Sets(size_t slot, const ITexturePtr states[], size_t count) {
		for (size_t i = 0; i < count; ++i)
			mTextures[slot + i] = states[i];
		mRenderSys.SetTextures(slot, states, count);
	}
	void operator()(size_t slot, ITexturePtr state) { Set(slot, state); }
	void operator()(size_t slot, const ITexturePtr states[], size_t count) { Sets(slot, states, count); }
	const ITexturePtr& Get(size_t slot) const { BOOST_ASSERT(slot < 16); return mTextures[slot]; }
	struct Lock : boost::noncopyable {
		Lock(TexturesBlock& block, size_t slot, const ITexturePtr& state) :mBlock(block), mSlot(slot), mState(block.Get(slot)) { block.Set(slot, state); }
		Lock(Lock&& other) :mBlock(other.mBlock), mSlot(other.mSlot), mState(other.mState) { other.mOwn = false; }
		~Lock() { if (mOwn) mBlock.Set(mSlot, mState); }
		Lock& operator=(Lock&& other) = delete;
		TexturesBlock* operator->() { return &mBlock; }
		void operator()(size_t slot, ITexturePtr state) { mBlock.Set(slot, state); }
	private:
		size_t mSlot;
		ITexturePtr mState;
		TexturesBlock& mBlock;
		bool mOwn = true;
	};
private:
	RenderSystem& mRenderSys;
	ITexturePtr mTextures[16];
};

struct RenderStatesBlock {
	RenderStatesBlock(RenderSystem& rs) :FrameBuffer(rs), Blend(rs), Depth(rs), Textures(rs) {}
	TemplateArgs FrameBufferBlock::Lock LockFrameBuffer(T &&...args) {
		return std::move(FrameBufferBlock::Lock(FrameBuffer, std::forward<T>(args)...));
	}
	TemplateArgs TexturesBlock::Lock LockTexture(T &&...args) {
		return std::move(TexturesBlock::Lock(Textures, std::forward<T>(args)...));
	}
	TemplateArgs BlendStateBlock::Lock LockBlend(T &&...args) {
		return std::move(BlendStateBlock::Lock(Blend, std::forward<T>(args)...));
	}
	TemplateArgs DepthStateBlock::Lock LockDepth(T &&...args) {
		return std::move(DepthStateBlock::Lock(Depth, std::forward<T>(args)...));
	}
public:
	FrameBufferBlock FrameBuffer;
	TexturesBlock Textures;
	BlendStateBlock Blend;
	DepthStateBlock Depth;
};

}