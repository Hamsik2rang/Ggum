#pragma once
#include "../../System/SystemPch.h"

namespace GG {
	class Vector2
	{
	public:
		float x, y;
		Vector2(float x = 0.0f, float y = 0.0f) : x(x), y(y) {};

		Vector2 operator+(const Vector2& other) const {
			return Vector2(x + other.x, y + other.y);
		}

		Vector2 operator+=(const Vector2& other) {
			x += other.x;
			y += other.y;
			return *this;
		}

		Vector2 operator-(const Vector2& other) const {
			return Vector2(x - other.x, y - other.y);
		}

		Vector2 operator-=(const Vector2& other) {
			x -= other.x;
			y -= other.y;
			return *this;
		}

		Vector2 operator*(const float scalar) const {
			return Vector2(x * scalar, y * scalar);
		}

		Vector2 operator*=(const float scalar) {
			x *= scalar;
			y *= scalar;
			return *this;
		}

		Vector2 operator/(const float scalar) const {
			if (scalar == 0.0f) {
				std::cout << "Can't be divied by zero.." << std::endl;
				return *this;
			}
			return Vector2(x / scalar, y / scalar);
		}

		Vector2 operator/=(const float scalar) {
			if (scalar == 0.0f) {
				std::cout << "Can't be divied by zero.." << std::endl;
				return *this;
			}
			x /= scalar;
			y /= scalar;
			return *this;
		}

		float dot(const Vector2& other) const {
			return x * other.x + y * other.y;
		}

		float length() const {
			return std::sqrt(x * x + y * y);
		}

		Vector2 normalize() const {
			float len = length();
			if (len == 0.0f) {
				std::cout << "This vector length is zero.." << std::endl;
				return *this;
			}
			else
				return *this * (1.0f / len);
		}

		friend std::ostream& operator<<(std::ostream& os, const Vector2 vec2)
		{
			os << "(" << vec2.x << ", " << vec2.y << ")\n";
			return os;
		}
	};

	class Vector3 {
	public:
		float x, y, z;
		Vector3(float x = 0.0f, float y = 0.0f, float z = 0.0f) : x(x), y(y), z(z) {};

		Vector3 operator+(const Vector3& other) const {
			return Vector3(x + other.x, y + other.y, z + other.z);
		}

		Vector3 operator+=(const Vector3& other) {
			x += other.x;
			y += other.y;
			z += other.z;
			return *this;
		}

		Vector3 operator-(const Vector3& other) const {
			return Vector3(x - other.x, y - other.y, z - other.z);
		}

		Vector3 operator-=(const Vector3& other) {
			x -= other.x;
			y -= other.y;
			z -= other.z;
			return *this;
		}

		Vector3 operator*(const float scalar) const {
			return Vector3(scalar * x, scalar * y, scalar * z);
		}

		Vector3 operator*=(const float scalar) {
			x *= scalar;
			y *= scalar;
			z *= scalar;
			return *this;
		}

		Vector3 operator/(const float scalar) const {
			if (scalar == 0.0f) {
				std::cout << "Can't be divied by zero.." << std::endl;
				return *this;
			}
			return Vector3(x / scalar, y / scalar, z / scalar);
		}

		Vector3 operator/=(const float scalar) {
			if (scalar == 0.0f) {
				std::cout << "Can't be divied by zero.." << std::endl;
				return *this;
			}
			x /= scalar;
			y /= scalar;
			z /= scalar;
			return *this;
		}


		float dot(const Vector3& other) const {
			return x * other.x + y * other.y + z * other.z;
		}

		Vector3 cross(const Vector3& other) const {
			return Vector3(
				y * other.z - z * other.y,
				z * other.x - x * other.z,
				x * other.y - y * other.x
			);
		}

		float length() const {
			return std::sqrt(x * x + y * y + z * z);
		}

		Vector3 normalize() const {
			float len = length();
			if (len == 0.0f) {
				std::cout << "This vector length is zero.." << std::endl;
				return *this;
			}
			else
				return Vector3(*this * ((1.0f) / len));
		}

		friend std::ostream& operator<<(std::ostream& os, const Vector3 vec3)
		{
			os << "(" << vec3.x << ", " << vec3.y << ", " << vec3.z << ")\n";
			return os;
		}
	};

	class Vector4 {
	public:
		float x, y, z, w;

		Vector4(float x = 0.0f, float y = 0.0f, float z = 0.0f, float w = 1.0f) : x(x), y(y), z(z), w(w) {};

		Vector4 operator+(const Vector4& other) const {
			return Vector4(x + other.x, y + other.y, z + other.z, w + other.w);
		}

		Vector4 operator+=(const Vector4& other) {
			x += other.x;
			y += other.y;
			z += other.z;
			w += other.w;
			return *this;
		}

		Vector4 operator-(const Vector4& other) const {
			return Vector4(x - other.x, y - other.y, z - other.z, w - other.w);
		}

		Vector4 operator-=(const Vector4& other) {
			x -= other.x;
			y -= other.y;
			z -= other.z;
			w -= other.w;
			return *this;
		}

		Vector4 operator*(const float scalar) const {
			return Vector4(scalar * x, scalar * y, scalar * z, scalar * w);
		}

		Vector4 operator*=(const float scalar) {
			x *= scalar;
			y *= scalar;
			z *= scalar;
			w *= scalar;
			return *this;
		}

		Vector4 operator/(const float scalar) const {
			if (scalar == 0.0f) {
				std::cout << "Can't be divied by zero.." << std::endl;
				return *this;
			}
			return Vector4(x / scalar, y / scalar, z / scalar, w / scalar);
		}

		Vector4 operator/=(const float scalar) {
			if (scalar == 0.0f) {
				std::cout << "Can't be divied by zero.." << std::endl;
				return *this;
			}
			x /= scalar;
			y /= scalar;
			z /= scalar;
			w /= scalar;
			return *this;
		}


		float dot(const Vector4& other) const {
			return x * other.x + y * other.y + z * other.z + w * other.w;
		}

		float length() const {
			return std::sqrt(x * x + y * y + z * z + w * w);
		}

		Vector4 normalize() const {
			float len = length();
			if (len == 0.0f) {
				std::cout << "This vector length is zero.." << std::endl;
				return *this;
			}
			else
				return Vector4(*this * ((1.0f) / len));
		}

		friend std::ostream& operator<<(std::ostream& os, const Vector4 vec4)
		{
			os << "(" << vec4.x << ", " << vec4.y << ", " << vec4.z << ", " << vec4.w << ")\n";
			return os;
		}
	};
};

