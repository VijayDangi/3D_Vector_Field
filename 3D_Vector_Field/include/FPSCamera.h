#ifndef __SS_GL_CAMERA__H_
#define __SS_GL_CAMERA__H_

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class FPSCamera
{
    public:
        //camera attributes
        glm::vec3 vPosition;
        glm::vec3 vFront;
        glm::vec3 vUp;
        glm::vec3 vRight;
        glm::vec3 vWorldUp;
        
        //eular angles
        float fYaw;
        float fPitch;
        
        //camera options
        float movementSpeed;
        float mouseSensitivity;
        
        FPSCamera( float mouseSensitivity, float movementSpeed, glm::vec3 position, glm::vec3 up, float yaw = -90.0f, float pitch = 0.0f)
        {
            //code
			this->mouseSensitivity = mouseSensitivity;
			this->movementSpeed = movementSpeed;
            this->vPosition = position;
            this->vWorldUp = up;
            this->fYaw = yaw;
            this->fPitch = pitch;
            
            updateCameraVectors();
        }
        
        void moveForward( float deltaTime)
        {
            float velocity = movementSpeed * deltaTime;
            this->vPosition += vFront * velocity;
        }
        
        void moveBackward( float deltaTime)
        {
            float velocity = movementSpeed * deltaTime;
            this->vPosition -= vFront * velocity;
        }
        
        void moveRight( float deltaTime)
        {
            float velocity = movementSpeed * deltaTime;
            this->vPosition += vRight * velocity;
        }
        
        void moveLeft( float deltaTime)
        {
            float velocity = movementSpeed * deltaTime;
            this->vPosition -= vRight * velocity;
        }
  
        void moveUp( float deltaTime)
        {
            float velocity = movementSpeed * deltaTime;
            this->vPosition += vUp * velocity;
        }
        
        void moveDown( float deltaTime)
        {
            float velocity = movementSpeed * deltaTime;
            this->vPosition -= vUp * velocity;
        }
  
        glm::mat4 getViewMatrix( void)
        {
            return( glm::lookAt( vPosition, vPosition + vFront, vUp));
        }
  
        void rotate( float xOffset, float yOffset, bool constraintPitch = true)
        {
            xOffset *= mouseSensitivity;
            yOffset *= mouseSensitivity;
            
			fPitch += xOffset;
			fYaw += yOffset;
            
            //make sure that when pitch is out of bounds, screen doesn't get flipped
            if( constraintPitch)
            {
                if( fPitch > 89.0f)
                    fPitch = 89.0f;
                if(fPitch < -89.0f)
                    fPitch = -89.0f;
            }
            
            updateCameraVectors();
        }
    
        void updateCameraVectors( void)
        {
            //code
            glm::vec3 front;
            
            front[0] = cos( glm::radians(fYaw)) * cos( glm::radians(fPitch));
            front[1] = sin( glm::radians(fPitch));
            front[2] = sin( glm::radians(fYaw)) * cos( glm::radians(fPitch));
            
            this->vFront = glm::normalize( front);
            this->vRight = glm::normalize( glm::cross( vFront, vWorldUp));
            this->vUp = glm::normalize( glm::cross( vRight, vFront));
        }
};

#endif
