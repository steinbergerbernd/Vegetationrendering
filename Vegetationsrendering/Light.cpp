#include "Light.h"


Light::Light()
{
	ZeroMemory(this, sizeof(Light));
}


Light::Light(const Vector3& direction, const Vector4& ambient, const Vector4& diffuse, const Vector4& specular)
{
	ZeroMemory(this, sizeof(Light));

	Direction = direction;
	Ambient = ambient;
	Diffuse = diffuse;
	Specular = specular;
}