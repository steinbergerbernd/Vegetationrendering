#pragma once

struct ConfigKeys
{
	static const char* logPath;
	
	static const char* terrainEffectPath;
	static const char* terrainWidth;
	static const char* terrainLength;
	static const char* terrainBumpiness;
	static const char* terrainQuality;

	static const char* cameraNearPlaneDistance;
	static const char* cameraFarPlaneDistance;
	static const char* cameraFieldOfView;
	static const char* cameraMouseSpeed;
	static const char* cameraMouseWheelSpeed;
	static const char* cameraMovementSpeed;
	static const char* cameraRotationSpeed;
	static const char* cameraMoveFast;
	static const char* cameraMoveSlow;

	static const char* mouseSmoothing;

	static const char* buttonMoveForward;
	static const char* buttonMoveBackward;
	static const char* buttonMoveLeft;
	static const char* buttonMoveRight;
	static const char* buttonMoveUp;
	static const char* buttonMoveDown;
	static const char* buttonMoveFast;
	static const char* buttonMoveSlow;

	static const char* buttonLookLeft;
	static const char* buttonLookRight;
	static const char* buttonLookUp;
	static const char* buttonLookDown;

	static const char* fbxEffectPath;

	static const char* lightDirection;
	static const char* lightAmbient;
	static const char* lightDiffuse;
	static const char* lightSpecular;

	static const char* grassPatchCount;
	static const char* grassBorder;
	static const char* grassShininess;
	static const char* grassMinDistance;
	static const char* grassMaxDistance;
	static const char* grassBladesPerPatch;
	static const char* grassVerticesPerBlade;
	static const char* grassNearPlane;
	static const char* grassFarPlane;
};
