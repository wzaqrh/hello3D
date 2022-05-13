#pragma once
#include "core/base/declare_macros.h"
#include "core/rendersys/render_system.h"

namespace mir {
	
struct FrameBufferBlock {
	struct Lock : boost::noncopyable {
		TemplateArgs Lock(FrameBufferBlock& block, T &&...args) :mBlock(block) { mBlock.Push(std::forward<T>(args)...); }
		Lock(Lock&& other) :mBlock(other.mBlock), mCurrentCb(other.mCurrentCb) { other.mOwn = false; }
		~Lock() { if (mOwn) { mBlock.Pop(); if (mCurrentCb) mCurrentCb(mBlock.GetCurrent()); } }
		void SetCallback(std::function<void(IFrameBufferPtr)> cb) { mCurrentCb = cb; if (mCurrentCb) mCurrentCb(mBlock.GetCurrent()); }
	private:
		FrameBufferBlock& mBlock;
		bool mOwn = true;
		std::function<void(IFrameBufferPtr)> mCurrentCb;
	};
public:
	FrameBufferBlock(RenderSystem& renderSys) :mRenderSys(renderSys) {}
	void Push(IFrameBufferPtr fb) {
		mStack.push_back(mCurrent);
		if (fb) mRenderSys.SetFrameBuffer(mCurrent = fb);

		if (mCurrentCb) mCurrentCb(mCurrent);
	}
	void Push(IFrameBufferPtr fb, const Eigen::Vector4f& color, float depth, uint8_t stencil) {
		Push(fb);
		mRenderSys.ClearFrameBuffer(fb, color, depth, stencil);
	}
	void Pop() {
		BOOST_ASSERT(!mStack.empty());
		mRenderSys.SetFrameBuffer(mCurrent = mStack.back());
		mStack.pop_back();

		if (mCurrentCb) mCurrentCb(mCurrent);
	}
	void SetCallback(std::function<void(IFrameBufferPtr)> cb) {
		mCurrentCb = cb;

		if (mCurrentCb) mCurrentCb(mCurrent);
	}
	const IFrameBufferPtr& GetCurrent() const { return mCurrent; }
private:
	RenderSystem& mRenderSys;
	IFrameBufferPtr mCurrent;
	std::vector<IFrameBufferPtr> mStack;
	std::function<void(IFrameBufferPtr)> mCurrentCb;
};

struct RasterizerStateBlock {
	struct Lock : boost::noncopyable {
		Lock(RasterizerStateBlock& block) :mBlock(block), mState(block.Get()) {}
		Lock(RasterizerStateBlock& block, const RasterizerState& state) :mBlock(block), mState(state) { block.Set(state); }
		Lock(Lock&& other) :mBlock(other.mBlock), mState(other.mState) { other.mOwn = false; }
		~Lock() { if (mOwn) mBlock.Set(mState); }
		Lock& operator=(Lock&& other) = delete;
		RasterizerStateBlock* operator->() { return &mBlock; }
		void operator()(CullMode mode) { mBlock.Set(mode); }
		void operator()(FillMode mode) { mBlock.Set(mode); }
		void operator()(const DepthBias& bias) { mBlock.Set(bias); }
	private:
		RasterizerState mState;
		RasterizerStateBlock& mBlock;
		bool mOwn = true;
	};
public:
	RasterizerStateBlock(RenderSystem& renderSys) :mRenderSys(renderSys) {
		mCurrent.CullMode = mRenderSys.GetCullMode();
		mCurrent.FillMode = mRenderSys.GetFillMode();
	}
	void Set(CullMode mode) {
		if (mCurrent.CullMode != mode) {
			mCurrent.CullMode = mode;
			mRenderSys.SetCullMode(mode);
		}
	}
	void Set(FillMode mode) {
		if (mCurrent.FillMode != mode) {
			mCurrent.FillMode = mode;
			mRenderSys.SetFillMode(mode);
		}
	}
	void Set(const DepthBias& bias) {
		if (mCurrent.DepthBias != bias) {
			mCurrent.DepthBias = bias;
			mRenderSys.SetDepthBias(bias);
		}
	}
	void Set(const RasterizerState& state) {
		Set(state.FillMode);
		Set(state.CullMode);
		Set(state.DepthBias);
	}
	void operator()(CullMode mode) { Set(mode); }
	void operator()(FillMode mode) { Set(mode); }
	void operator()(const DepthBias& bias) { Set(bias); }
	const RasterizerState& Get() const { return mCurrent; }
private:
	RenderSystem& mRenderSys;
	RasterizerState mCurrent;
};

struct BlendStateBlock {
	struct Lock : boost::noncopyable {
		Lock(BlendStateBlock& block) :mBlock(block), mState(block.Get()) {}
		Lock(BlendStateBlock& block, const BlendState& state) :mBlock(block), mState(state) { block.Set(state); }
		Lock(Lock&& other) :mBlock(other.mBlock), mState(other.mState) { other.mOwn = false; }
		~Lock() { if (mOwn) mBlock.Set(mState); }
		Lock& operator=(Lock&& other) = delete;
		BlendStateBlock* operator->() { return &mBlock; }
		void operator()(const BlendState& state) { mBlock.Set(state); }
	private:
		BlendState mState;
		BlendStateBlock& mBlock;
		bool mOwn = true;
	};
public:
	BlendStateBlock(RenderSystem& renderSys) :mRenderSys(renderSys), mCurrent(renderSys.GetBlendState()) {}
	void Set(const BlendState& state) {
		if (mCurrent != state) {
			mCurrent = state;
			mRenderSys.SetBlendState(state);
		}
	}
	void operator()(const BlendState& state) { Set(state); }
	const BlendState& Get() const { return mCurrent; }
private:
	RenderSystem& mRenderSys;
	BlendState mCurrent;
};

struct DepthStateBlock {
	struct Lock : boost::noncopyable {
		Lock(DepthStateBlock& block) :mBlock(block), mState(block.Get()) {}
		Lock(DepthStateBlock& block, const DepthState& state) :mBlock(block), mState(state) { block.Set(state); }
		Lock(Lock&& other) :mBlock(other.mBlock), mState(other.mState) { other.mOwn = false; }
		~Lock() { if (mOwn) mBlock.Set(mState); }
		Lock& operator=(Lock&& other) = delete;
		DepthStateBlock* operator->() { return &mBlock; }
		void operator()(const DepthState& state) { mBlock.Set(state); }
	private:
		DepthState mState;
		DepthStateBlock& mBlock;
		bool mOwn = true;
	};
public:
	DepthStateBlock(RenderSystem& renderSys) :mRenderSys(renderSys), mCurrent(renderSys.GetDepthState()) {}
	void Set(const DepthState& state) {
		if (mCurrent != state) {
			mCurrent = state;
			mRenderSys.SetDepthState(state);
		}
	}
	void operator()(const DepthState& state) { Set(state); }
	const DepthState& Get() const { return mCurrent; }
private:
	RenderSystem& mRenderSys;
	DepthState mCurrent;
};

struct TexturesBlock {
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
public:
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
	const ITexturePtr& operator[](size_t slot) const { return Get(slot); }
private:
	RenderSystem& mRenderSys;
	ITexturePtr mTextures[16];
};

struct RenderStatesBlock {
	RenderStatesBlock(RenderSystem& rs) :FrameBuffer(rs), Textures(rs), Blend(rs), Depth(rs), Raster(rs) {}
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
	TemplateArgs RasterizerStateBlock::Lock LockRaster(T &&...args) {
		return std::move(RasterizerStateBlock::Lock(Raster, std::forward<T>(args)...));
	}
	const IFrameBufferPtr& CurrentFrameBuffer() const { return FrameBuffer.GetCurrent(); }
public:
	FrameBufferBlock FrameBuffer;
	TexturesBlock Textures;
	BlendStateBlock Blend;
	DepthStateBlock Depth;
	RasterizerStateBlock Raster;
};

}