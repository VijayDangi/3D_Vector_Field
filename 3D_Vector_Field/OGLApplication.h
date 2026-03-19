#ifndef __OGL_APPLICATION_H__
#define __OGL_APPLICATION_H__

#include "Common.h"

namespace OglApplication {
    bool Init();
    void Resize(unsigned int width, unsigned int height);
    void KeyDownEventHandler(unsigned int key);
    void KeyUpEventHandler(unsigned int key);
    void MouseDownEventHandler(EMouseButton mouse);
    void MouseUpEventHandler(EMouseButton button);
    void MouseMoveEventHandler(EMouseButton button, unsigned int x, unsigned int y);
    void MouseWheelEventHandler(EMouseButton button, unsigned int x, unsigned int y, float delta);
    void RenderImGui(double deltaTime);
    void Render(double deltaTime);
    void Destroy();
};

#endif  //__OGL_APPLICATION_H__
