#include "Camera.h"
#include "MathHelper.h"
#include "Button.h"

Camera::Camera() : rotationX(0), rotationY(0), locked(false), visitationMode(false), pauseMode(false)
{
}


Camera::~Camera()
{
}


void Camera::init()
{
	nearPlaneDistance = Config::getValue<float>(ConfigKeys::cameraNearPlaneDistance);
	farPlaneDistance = Config::getValue<float>(ConfigKeys::cameraFarPlaneDistance);
	fieldOfView = Config::getValue<float>(ConfigKeys::cameraFieldOfView);

	mouseSpeed = Config::getValue<float>(ConfigKeys::cameraMouseSpeed);
	mouseWheelSpeed = Config::getValue<float>(ConfigKeys::cameraMouseWheelSpeed);
	movementSpeed = Config::getValue<float>(ConfigKeys::cameraMovementSpeed);
	rotationSpeed = Config::getValue<float>(ConfigKeys::cameraRotationSpeed);

	moveFast = Config::getValue<float>(ConfigKeys::cameraMoveFast);
	moveSlow = Config::getValue<float>(ConfigKeys::cameraMoveSlow);

	buttonMoveForward = Config::getValue<char>(ConfigKeys::buttonMoveForward);
	buttonMoveBackward = Config::getValue<char>(ConfigKeys::buttonMoveBackward);
	buttonMoveLeft = Config::getValue<char>(ConfigKeys::buttonMoveLeft);
	buttonMoveRight = Config::getValue<char>(ConfigKeys::buttonMoveRight);
	buttonMoveUp = Config::getValue<char>(ConfigKeys::buttonMoveUp);
	buttonMoveDown = Config::getValue<char>(ConfigKeys::buttonMoveDown);

	buttonMoveFast = Button::get(Config::getValue(ConfigKeys::buttonMoveFast));
	buttonMoveSlow = Button::get(Config::getValue(ConfigKeys::buttonMoveSlow));

	buttonLookDown = Button::get(Config::getValue(ConfigKeys::buttonLookDown));
	buttonLookUp = Button::get(Config::getValue(ConfigKeys::buttonLookUp));
	buttonLookLeft = Button::get(Config::getValue(ConfigKeys::buttonLookLeft));
	buttonLookRight = Button::get(Config::getValue(ConfigKeys::buttonLookRight));
}


void Camera::release()
{
}


void Camera::update(const GameTime& gameTime)
{
	if(cameraViewInput.is_open())
	{
		if (pauseMode) return;

		elapsedVisitTime += gameTime.elapsed;
		if(elapsedVisitTime < visitTime)
			return;
		else
			elapsedVisitTime = 0;

		readNextCameraView();

		updateRotation();

		return;
	}

	Vector2 mouseMovement = Mouse::getMovement();

	yaw(mouseMovement.x * mouseSpeed);
	pitch(mouseMovement.y * mouseSpeed);

	Matrix m = Matrix::createFromAxisAngle(Vector3::up, rotationX);

	Vector3 forward = Vector3::transform(Vector3::forward, m);
	Vector3 right = Vector3::transform(Vector3::right, m);

	Vector3 direction;

	if (Keyboard::isKeyDown(buttonMoveForward))
		direction += forward;

	if (Keyboard::isKeyDown(buttonMoveBackward))
		direction -= forward;

	if (Keyboard::isKeyDown(buttonMoveRight))
		direction += right;

	if (Keyboard::isKeyDown(buttonMoveLeft))
		direction -= right;

	if (Keyboard::isKeyDown(buttonMoveUp))
		direction += Vector3::up;

	if (Keyboard::isKeyDown(buttonMoveDown))
		direction -= Vector3::up;

	float speed = rotationSpeed * gameTime.elapsed;

	if (Keyboard::isKeyDown(buttonLookDown))
		pitch(speed);

	if (Keyboard::isKeyDown(buttonLookUp))
		pitch(-speed);

	if (Keyboard::isKeyDown(buttonLookRight))
		yaw(speed);

	if (Keyboard::isKeyDown(buttonLookLeft))
		yaw(-speed);

	float modifier = 1.0f;

	if (Keyboard::isKeyDown(buttonMoveFast))
		modifier *= moveFast;

	if (Keyboard::isKeyDown(buttonMoveSlow))
		modifier *= moveSlow;

	position += Vector3::normalize(direction) * movementSpeed * modifier * gameTime.elapsed;

	updateRotation();
}


void Camera::updateRotation()
{
	Quaternion rotX = Quaternion::createFromAxisAngle(Vector3::up, rotationX);
	Quaternion rotY = Quaternion::createFromAxisAngle(Vector3::right, rotationY);

	rotation = rotY * rotX;
}


Matrix Camera::getView() const
{
	return Matrix::createTranslation(-position) * Matrix::invert(Matrix::createFromQuaternion(rotation));
}


Matrix Camera::getProjection(float nearPlaneDistance, float farPlaneDistance) const
{
	if (!nearPlaneDistance)
		nearPlaneDistance = this->nearPlaneDistance;

	if (!farPlaneDistance)
		farPlaneDistance = this->farPlaneDistance;

	return Matrix::createPerspectiveFieldOfView(D3DXToRadian(fieldOfView), aspectRatio, nearPlaneDistance, farPlaneDistance);
}


const Vector3& Camera::getPosition() const 
{ 
	return (locked) ? lockedPosition : position; 
}


const Quaternion& Camera::getRotation() const 
{ 
	return (locked) ? lockedRotation : rotation; 
}


void Camera::toggleLock()
{
	lockedPosition = position;
	lockedRotation = rotation;
	lockedViewFrustum = getViewFrustum();

	locked = !locked;
}


BoundingFrustum Camera::getViewFrustum() const
{
	return (locked) ? lockedViewFrustum : BoundingFrustum(getView() * getProjection());
}


void Camera::pitch(float angle)
{
	rotationY += angle;

	if (std::abs(rotationY) > D3DX_PI / 2.0f)
		rotationY = (D3DX_PI / 2.0f) * ((rotationY > 0) ? 1 : -1);
}


void Camera::yaw(float angle)
{
	rotationX += angle;

	if (std::abs(rotationX) > D3DX_PI * 2)
		rotationX -= (D3DX_PI * 2.0f) * ((rotationX > 0) ? 1 : -1);
}


void Camera::lookAt(const Vector3& target)
{
	look(target - position);
}


void Camera::look(const Vector3& direction)
{
	Vector3 d = Vector3::normalize(direction);
	Vector3 vx = Vector3::normalize(Vector3(d.x, 0, d.z));

	rotationX = std::acos(MathHelper::clamp(Vector3::dot(Vector3::forward, vx), -1.0f, 1.0f));
	rotationY = std::acos(MathHelper::clamp(Vector3::dot(vx, d), -1.0f, 1.0f));

	rotationX *= (direction.x < 0) ? -1 : 1;
	rotationY *= (direction.y < 0) ? 1 : -1;

	updateRotation();
}


Vector3 Camera::getForward() const
{
	return Vector3::transform(Vector3::forward, Matrix::createFromQuaternion(rotation));
}


Vector3 Camera::getUp() const
{
	return Vector3::transform(Vector3::up, Matrix::createFromQuaternion(rotation));
}


void Camera::outputCameraView(std::string filename)
{
	std::ofstream outputFile;

	outputFile.open(filename, std::ios::app);
	outputFile << position.x << std::endl;
	outputFile << position.y << std::endl;
	outputFile << position.z << std::endl;
	outputFile << rotationX << std::endl;
	outputFile << rotationY << std::endl;
	outputFile.close();
}


void Camera::visitCameraViews(std::string filename, float visitTime)
{
	if(visitationMode)
		stopVisitCameraViews();
	cameraViewInput.open(filename, std::ios::in);
	this->visitTime = visitTime;
	elapsedVisitTime = visitTime;
	visitationMode = true;
	prevPosition = position;
	prevRotationX = rotationX;
	prevRotationY = rotationY;
	viewCounter = -1;
}


void Camera::stopVisitCameraViews()
{
	cameraViewInput.close();
	visitTime = 0;
	elapsedVisitTime = 0;
	visitationMode = false;
	position = prevPosition;
	rotationX = prevRotationX;
	rotationY = prevRotationY;
	viewCounter = -1;
}


float Camera::getCameraViewValue()
{
	std::string line;
	std::getline(cameraViewInput, line);
	return Converter::convert<float>(line);
}


void Camera::goToNextCameraView()
{
	if (visitationMode)
	{
		elapsedVisitTime = 0;
		readNextCameraView();
		updateRotation();
	}
}


void Camera::readNextCameraView()
{
	if(cameraViewInput.eof())
	{
		stopVisitCameraViews();
		return;
	}

	position.x = getCameraViewValue();
	position.y = getCameraViewValue();
	position.z = getCameraViewValue();
	rotationX = getCameraViewValue();
	rotationY = getCameraViewValue();
	++viewCounter;
}