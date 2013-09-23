#pragma once

#include "Types.h"
#include "Input.h"
#include "BoundingFrustum.h"

#include <iostream>
#include <fstream>

class Camera
{
public:
	Camera();
	virtual ~Camera();

	virtual void init();
	virtual void release();
	virtual void update(const GameTime& gameTime);

	virtual Matrix getView() const;
	virtual Matrix getProjection(float nearPlaneDistance = 0.0f, float farPlaneDistance = 0.0f) const;

	virtual BoundingFrustum getViewFrustum() const;

	virtual const Vector3& getPosition() const;
	virtual const Quaternion& getRotation() const;
	virtual float getNearPlaneDistance() const { return nearPlaneDistance; }
	virtual float getFarPlaneDistance() const { return farPlaneDistance; }

	virtual void setPosition(const Vector3& position) { this->position = position; }
	virtual void setNearPlaneDistance(float nearPlaneDistance) { this->nearPlaneDistance = nearPlaneDistance; }
	virtual void setFarPlaneDistance(float farPlaneDistance) { this->farPlaneDistance = farPlaneDistance; }
	virtual void setAspectRatio(float aspectRatio) { this->aspectRatio = aspectRatio; }

	virtual void look(const Vector3& direction);
	virtual void lookAt(const Vector3& target);

	virtual Vector3 getForward() const;
	virtual Vector3 getUp() const;

	void toggleLock();

	void outputCameraView(std::string filename);
	void visitCameraViews(std::string filename, float visitTime);
	void pauseVisitCameraViews() { pauseMode = !pauseMode; }
	void stopVisitCameraViews();
	void goToNextCameraView();
	int getCurrentViewCounter() const { return viewCounter; }

	bool isInVisitationMode() const { return visitationMode; }
protected:
	float rotationX;
	float rotationY;

	float nearPlaneDistance;
	float farPlaneDistance;
	float fieldOfView;
	float aspectRatio;

	float mouseSpeed;
	float movementSpeed;
	float rotationSpeed;
	float mouseWheelSpeed;

	float moveFast;
	float moveSlow;

	unsigned buttonMoveForward;
	unsigned buttonMoveBackward;
	unsigned buttonMoveLeft;
	unsigned buttonMoveRight;
	unsigned buttonMoveUp;
	unsigned buttonMoveDown;

	unsigned buttonMoveFast;
	unsigned buttonMoveSlow;

	unsigned buttonLookUp;
	unsigned buttonLookDown;
	unsigned buttonLookLeft;
	unsigned buttonLookRight;

	unsigned buttonShowCursor;

	void pitch(float angle);
	void yaw(float angle);

private:
	void updateRotation();

	float getCameraViewValue();
	void readNextCameraView();

	Vector3 position;
	Quaternion rotation;

	bool locked;

	Vector3 lockedPosition;
	Quaternion lockedRotation;
	BoundingFrustum lockedViewFrustum;

	std::ifstream cameraViewInput;
	float visitTime;
	float elapsedVisitTime;
	bool visitationMode;
	bool pauseMode;
	Vector3 prevPosition;
	float prevRotationX;
	float prevRotationY;
	int viewCounter;
};