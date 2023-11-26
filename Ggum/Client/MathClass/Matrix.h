#pragma once
#include "Vector.h"
#include "../../System/SystemPch.h"

namespace GG {
	class Mat2x2
	{
	public:
		union {
			struct {
				Vector2 r1;
				Vector2 r2;
			};
			Vector2 r[2];
			float elem[2][2];
		};

		Mat2x2(Vector2 _r1 = { 1.0f, 0.0f }, Vector2 _r2 = { 0.0f, 1.0f }) {
			r1 = _r1;
			r2 = _r2;
		};

		Mat2x2 operator+(const Mat2x2& other) const {
			Mat2x2 mat;
			mat.r1 = r1 + other.r1;
			mat.r2 = r2 + other.r2;
			return mat;
		};

		Mat2x2 operator+=(const Mat2x2& other) {
			Mat2x2 mat;
			mat.r1 += other.r1;
			mat.r2 += other.r2;
			return mat;
		};

		Mat2x2 operator-(const Mat2x2& other) const {
			Mat2x2 mat;
			mat.r1 = r1 - other.r1;
			mat.r2 = r2 - other.r2;
			return mat;
		};

		Mat2x2 operator-=(const Mat2x2& other) {
			Mat2x2 mat;
			mat.r1 -= other.r1;
			mat.r2 -= other.r2;
			return mat;
		};

		Mat2x2 operator*(const float scalar) const {
			Mat2x2 mat;
			mat.r1 = r1 * scalar;
			mat.r2 = r2 * scalar;
			return mat;
		};

		Mat2x2 operator*=(const float scalar) {
			Mat2x2 mat;
			mat.r1 *= scalar;
			mat.r2 *= scalar;
			return mat;
		};

		Mat2x2 operator*(Mat2x2& other) {
			Mat2x2 mat;
			other.Transpose();
			for (int i = 0; i < 2; i++) {
				for (int j = 0; j < 2; j++) {
					mat.elem[i][j] = r[i].dot(other.r[j]);
				}
			}
			other.Transpose();
			return mat;
		}

		Vector2 operator*(Vector2& other) {
			Vector2 vec2 = { r1.dot(other), r2.dot(other) };
			return vec2;
		}

		Mat2x2 operator/(const float scalar) const {
			if (scalar == 0.0f) {
				std::cout << "Can't be divied by zero.." << std::endl;
				return *this;
			}
			Mat2x2 mat;
			mat.r1 = r1 / scalar;
			mat.r2 = r2 / scalar;
			return mat;
		};

		Mat2x2 operator/=(const float scalar) {
			if (scalar == 0.0f) {
				std::cout << "Can't be divied by zero.." << std::endl;
				return *this;
			}
			Mat2x2 mat;
			mat.r1 /= scalar;
			mat.r2 /= scalar;
			return mat;
		};

		Mat2x2 Transpose() {
			for (int i = 0; i < 2; i++) {
				for (int j = i + 1; j < 2; j++) {
					float tmp = elem[i][j];
					elem[i][j] = elem[j][i];
					elem[j][i] = tmp;
				}
			}
			return *this;
		}

		friend std::ostream& operator<<(std::ostream& os, const Mat2x2& mat)
		{
			os << mat.r1 << mat.r2;
			return os;
		}
	};

	class Mat3x3
	{
	public:
		union {
			struct {
				Vector3 r1;
				Vector3 r2;
				Vector3 r3;
			};
			Vector3 r[3];
			float elem[3][3];
		};


		Mat3x3(Vector3 _r1 = { 1.0f, 0.0f, 0.0f }, Vector3 _r2 = { 0.0f, 1.0f, 0.0f }, Vector3 _r3 = { 0.0f, 0.0f, 1.0f }) {
			r1 = _r1;
			r2 = _r2;
			r3 = _r3;
		};

		Mat3x3 operator+(const Mat3x3& other) const {
			Mat3x3 mat;
			mat.r1 = r1 + other.r1;
			mat.r2 = r2 + other.r2;
			mat.r3 = r3 + other.r3;
			return mat;
		};

		Mat3x3 operator+=(const Mat3x3& other) {
			Mat3x3 mat;
			mat.r1 += other.r1;
			mat.r2 += other.r2;
			mat.r3 += other.r3;
			return mat;
		};

		Mat3x3 operator-(const Mat3x3& other) const {
			Mat3x3 mat;
			mat.r1 = r1 - other.r1;
			mat.r2 = r2 - other.r2;
			mat.r3 = r3 - other.r3;
			return mat;
		};

		Mat3x3 operator-=(const Mat3x3& other) {
			Mat3x3 mat;
			mat.r1 -= other.r1;
			mat.r2 -= other.r2;
			mat.r3 -= other.r3;
			return mat;
		};

		Mat3x3 operator*(const float scalar) const {
			Mat3x3 mat;
			mat.r1 = r1 * scalar;
			mat.r2 = r2 * scalar;
			mat.r3 = r3 * scalar;
			return mat;
		};

		Mat3x3 operator*=(const float scalar) {
			Mat3x3 mat;
			mat.r1 *= scalar;
			mat.r2 *= scalar;
			mat.r3 *= scalar;
			return mat;
		};

		Mat3x3 operator*(Mat3x3& other) {
			Mat3x3 mat;
			other.Transpose();
			for (int i = 0; i < 3; i++) {
				for (int j = 0; j < 3; j++) {
					mat.elem[i][j] = r[i].dot(other.r[j]);
				}
			}
			other.Transpose();
			return mat;
		}

		Vector3 operator*(Vector3& other) {
			Vector3 vec3 = { r1.dot(other), r2.dot(other), r3.dot(other) };
			return vec3;
		}

		Mat3x3 operator/(const float scalar) const {
			if (scalar == 0.0f) {
				std::cout << "Can't be divied by zero.." << std::endl;
				return *this;
			}
			Mat3x3 mat;
			mat.r1 = r1 / scalar;
			mat.r2 = r2 / scalar;
			mat.r3 = r3 / scalar;
			return mat;
		};

		Mat3x3 operator/=(const float scalar) {
			if (scalar == 0.0f) {
				std::cout << "Can't be divied by zero.." << std::endl;
				return *this;
			}
			Mat3x3 mat;
			mat.r1 /= scalar;
			mat.r2 /= scalar;
			mat.r3 /= scalar;
			return mat;
		};

		Mat3x3 Transpose() {
			for (int i = 0; i < 3; i++) {
				for (int j = i + 1; j < 3; j++) {
					float tmp = elem[i][j];
					elem[i][j] = elem[j][i];
					elem[j][i] = tmp;
				}
			}
			return *this;
		}

		friend std::ostream& operator<<(std::ostream& os, const Mat3x3& mat)
		{
			os << mat.r1 << mat.r2 << mat.r3;
			return os;
		}
	};

	class Mat4x4
	{
	public:
		union {
			struct {
				Vector4 r1;
				Vector4 r2;
				Vector4 r3;
				Vector4 r4;
			};
			Vector4 r[4];
			float elem[4][4];
		};


		Mat4x4(Vector4 _r1 = { 1.0f, 0.0f, 0.0f, 0.0f }, Vector4 _r2 = { 0.0f, 1.0f, 0.0f, 0.0f }, Vector4 _r3 = { 0.0f, 0.0f, 1.0f, 0.0f }, Vector4 _r4 = { 0.0f, 0.0f, 0.0f, 1.0f }) {
			r1 =_r1;
			r2 = _r2;
			r3 = _r3;
			r4 = _r4;
		};

		Mat4x4 operator+(const Mat4x4& other) const {
			Mat4x4 mat;
			mat.r1 = r1 + other.r1;
			mat.r2 = r2 + other.r2;
			mat.r3 = r3 + other.r3;
			mat.r4 = r4 + other.r4;
			return mat;
		};

		Mat4x4 operator+=(const Mat4x4& other) {
			Mat4x4 mat;
			mat.r1 += other.r1;
			mat.r2 += other.r2;
			mat.r3 += other.r3;
			mat.r4 += other.r4;
			return mat;
		};

		Mat4x4 operator-(const Mat4x4& other) const {
			Mat4x4 mat;
			mat.r1 = r1 - other.r1;
			mat.r2 = r2 - other.r2;
			mat.r3 = r3 - other.r3;
			mat.r4 = r4 - other.r4;
			return mat;
		};

		Mat4x4 operator-=(const Mat4x4& other) {
			Mat4x4 mat;
			mat.r1 -= other.r1;
			mat.r2 -= other.r2;
			mat.r3 -= other.r3;
			mat.r4 -= other.r4;
			return mat;
		};

		Mat4x4 operator*(const float scalar) const {
			Mat4x4 mat;
			mat.r1 = r1 * scalar;
			mat.r2 = r2 * scalar;
			mat.r3 = r3 * scalar;
			mat.r4 = r4 * scalar;
			return mat;
		};

		Mat4x4 operator*=(const float scalar) {
			Mat4x4 mat;
			mat.r1 *= scalar;
			mat.r2 *= scalar;
			mat.r3 *= scalar;
			mat.r4 *= scalar;
			return mat;
		};

		Mat4x4 operator*(Mat4x4& other) {
			Mat4x4 mat;
			other.Transpose();
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 4; j++) {
					mat.elem[i][j] = r[i].dot(other.r[j]);
				}
			}
			other.Transpose();
			return mat;
		}

		Vector4 operator*(Vector4& other) {
			Vector4 vec4 = { r1.dot(other), r2.dot(other), r3.dot(other), r4.dot(other) };
			return vec4;
		}

		Mat4x4 operator/(const float scalar) const {
			if (scalar == 0.0f) {
				std::cout << "Can't be divied by zero.." << std::endl;
				return *this;
			}
			Mat4x4 mat;
			mat.r1 = r1 / scalar;
			mat.r2 = r2 / scalar;
			mat.r3 = r3 / scalar;
			mat.r4 = r4 / scalar;
			return mat;
		};

		Mat4x4 operator/=(const float scalar) {
			if (scalar == 0.0f) {
				std::cout << "Can't be divied by zero.." << std::endl;
				return *this;
			}
			Mat4x4 mat;
			mat.r1 /= scalar;
			mat.r2 /= scalar;
			mat.r3 /= scalar;
			mat.r4 /= scalar;
			return mat;
		};

		Mat4x4 Transpose() {
			for (int i = 0; i < 4; i++) {
				for (int j = i + 1; j < 4; j++) {
					float tmp = elem[i][j];
					elem[i][j] = elem[j][i];
					elem[j][i] = tmp;
				}
			}
			return *this;
		}

		friend std::ostream& operator<<(std::ostream& os, const Mat4x4& mat)
		{
			os << mat.r1 << mat.r2 << mat.r3 << mat.r4;
			return os;
		}
	};


};
