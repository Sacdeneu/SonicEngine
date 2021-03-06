#pragma once
#include "rigidbody.h"

class Rigidbody; //permet de lier mututellement les classes sans erreur d'import

enum class PrimitiveType { plane, box };

class Primitive
{
public:
	//CONSTRUCTEURS
	Primitive();
	~Primitive() = default;

	Matrix4 GetTransform();
	void UpdateOffsetTRS(Vector3D position, Quaternion orientation, Vector3D scale);

public:
	Rigidbody* rigidbody;
	PrimitiveType type;

protected:
	Matrix4 transformOffset;
};
