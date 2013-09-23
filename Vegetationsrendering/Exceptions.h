#pragma once

struct Exceptions
{
	static const char* buttonNotFound;

	static const char* configKeyNotFound;
	static const char* configFileNotFound;

	static const char* cameraNotFound;
	static const char* cameraNameInUse;

	static const char* clearFailed;
	static const char* beginSceneFailed;
	static const char* endSceneFailed;
	static const char* setRenderTargetFailed;
	static const char* getRenderTargetFailed;

	static const char* createIndexBufferFailed;
	static const char* createVertexBufferFailed;
	static const char* createStateBlockFailed;
	static const char* createVertexDeclarationFailed;

	static const char* vertexBufferLockFailed;
	static const char* vertexBufferUnlockFailed;
	static const char* vertexBufferGetDescFailed;

	static const char* getViewportFailed;
	static const char* getRenderStateFailed;
	
	static const char* setTransformFailed;
	static const char* setRenderStateFailed;
	static const char* setSamplerStateFailed;
	static const char* setFVFFailed;
	static const char* setVertexDeclarationFailed;
	static const char* setStreamSourceFreqFailed;
	static const char* setStreamSourceFailed;
	static const char* setClipPlaneFailed;
	static const char* setScissorRectFailed;

	static const char* depthStencilFailed;

	static const char* drawPrimitiveFailed;
	static const char* drawIndexedPrimitiveFailed;
	static const char* drawPrimitiveUPFailed;

	static const char* setTextureFailed;
	static const char* setMaterialFailed;
	static const char* setIndicesFailed;

	static const char* indexBufferLockFailed;
	static const char* indexBufferUnlockFailed;
	static const char* indexBufferGetDescFailed;

	static const char* setLightFailed;
	static const char* lightEnableFailed;

	static const char* createTextureFromFileFailed;
	static const char* createTextureFailed;
	static const char* textureGetDescFailed;
	static const char* textureLockRectFailed;
	static const char* textureUnlockRectFailed;
	static const char* textureGetSurfaceLevelFailed;

	static const char* createEffectFromFileFailed;
	static const char* effectOnLostDeviceFailed;
	static const char* effectOnResetDeviceFailed;
	static const char* effectSetMatrixFailed;
	static const char* effectSetMatrixArrayFailed;
	static const char* effectBeginFailed;
	static const char* effectEndFailed;
	static const char* effectBeginPassFailed;
	static const char* effectEndPassFailed;
	static const char* effectSetTextureFailed;
	static const char* effectSetIntFailed;
	static const char* effectSetVectorFailed;
	static const char* effectSetFloatFailed;
	static const char* effectSetBoolFailed;
	static const char* effectCommitChangesFailed;
	static const char* effectFailed;

	static const char* invalidArgument;

	static const char* meshGetVertexBufferFailed;
	static const char* meshGetIndexBufferFailed;
	static const char* meshFailed;

	static const char* stateBlockApplyFailed;

	static const char* createRenderToSurfaceFailed;

	static const char* notImplementedException;

	static const char* networkFailed;
	static const char* networkConnectFailed;
};
