#ifndef _LIGHT_H_
#define _LIGHT_H_

#include <glm/glm.hpp>

class Light
{
private:
    glm::vec3 ambient,diffuse,specular;
    glm::vec4 position,spot_direction;
    float spot_cutoff;
public:
	Light(void);
	~Light(void);

	void setAmbient(float r,float g,float b);
	void setDiffuse(float r,float g,float b);
	void setSpecular(float r,float g,float b);
    void setAmbient(glm::vec3 color);
    void setDiffuse(glm::vec3 color);
    void setSpecular(glm::vec3 color);
    void setPosition(glm::vec4 pos);

	void setID(unsigned int id);
	void init();
	void setLocalLightStatus(bool flag);
    void setSpotDirection(glm::vec4 dir);
	void setSpotAngle(float angle);
    glm::vec3 getAmbient();
    glm::vec3 getDiffuse();
    glm::vec3 getSpecular();
    glm::vec4 getPosition();
    bool getLocalLightStatus();
    glm::vec4 getSpotDirection();
	float getSpotAngle();
};

#endif
