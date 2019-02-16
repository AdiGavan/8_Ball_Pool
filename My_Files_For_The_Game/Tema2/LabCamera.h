#pragma once
// "Copyright [2018] Gavan Adrian-George, 334CA"
#include <include/glm.h>
#include <include/math.h>

namespace Tema2Camera
{
	class Camera
	{
		public:
			Camera()
			{
				position = glm::vec3(0, 2, 5);
				forward = glm::vec3(0, 0, -1);
				up		= glm::vec3(0, 1, 0);
				right	= glm::vec3(1, 0, 0);
				distanceToTarget = 1;
			}

			Camera(const glm::vec3 &position, const glm::vec3 &center, const glm::vec3 &up)
			{
				Set(position, center, up);
			}

			~Camera()
			{ }

			// Update camera
			void Set(const glm::vec3 &position, const glm::vec3 &center, const glm::vec3 &up)
			{
				this->position = position;
				forward = glm::normalize(center-position);
				right	= glm::cross(forward, up);
				this->up = glm::cross(right,forward);
			}

			void MoveForward(float distance)
			{
				glm::vec3 dir = glm::normalize(glm::vec3(forward.x, 0, forward.z));
				position = position + glm::normalize(dir) * distance;
			}

			void TranslateForward(float distance)
			{
				glm::vec3 dir = glm::normalize(glm::vec3(forward.x, 0, forward.z));
				position = position + glm::normalize(dir) * distance;
			
			}

			void TranslateUpword(float distance)
			{
				position = position + glm::normalize(up) * distance;
			}

			void TranslateRight(float distance)
			{
				// Translate the camera using the "right" vector
				position = position + glm::normalize(right) * distance;
			}

			void RotateFirstPerson_OX(float angle)
			{
				forward = glm::normalize(glm::rotate(glm::mat4(1.0f), angle, right) * glm::vec4(forward, 0));
				up = glm::normalize(glm::cross(right, forward));
			}

			void RotateFirstPerson_OY(float angle)
			{
				forward = glm::normalize(glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 1, 0)) * glm::vec4(forward, 0));
				right = glm::normalize(glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 1, 0)) * glm::vec4(right, 0));
				up = glm::normalize(glm::cross(right, forward));
			}

			void RotateFirstPerson_OZ(float angle)
			{
				right = glm::normalize(glm::rotate(glm::mat4(1.0f), angle, forward) * glm::vec4(right, 0));
				up = glm::normalize(glm::cross(right, forward));
			}

			void RotateThirdPerson_OX(float angle)
			{
				TranslateForward(distanceToTarget);
				RotateFirstPerson_OX(angle);
				TranslateForward(-distanceToTarget);
			}

			void RotateThirdPerson_OY(float angle)
			{
				TranslateForward(distanceToTarget);
				RotateFirstPerson_OY(angle);
				TranslateForward(-distanceToTarget);
			}

			void RotateThirdPerson_OZ(float angle)
			{
				TranslateForward(distanceToTarget);
				RotateFirstPerson_OZ(angle);
				TranslateForward(-distanceToTarget);
			}

			glm::mat4 GetViewMatrix()
			{
				// Returns the View Matrix
				return glm::lookAt(position, position + forward, up);
			}

			glm::vec3 GetTargetPosition()
			{
				return position + forward * distanceToTarget;
			}

			glm::vec3 GetPosition()
			{
				return position;
			}

			glm::vec3 GetForDist()
			{
				return forward * distanceToTarget;
			}

			glm::vec3 GetDir()
			{
				glm::vec3 dir = glm::normalize(glm::vec3(forward.x, 0, forward.z));
				return dir; 
			}


		public:
			float distanceToTarget;
			glm::vec3 position;
			glm::vec3 forward;
			glm::vec3 right;
			glm::vec3 up;
		};
}