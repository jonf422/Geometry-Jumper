#ifndef RUNNER_CLOUD_H
#define RUNNER_CLOUD_H

#include "circle.h"
#include "rect.h"
#include "shape.h"
#include "../shader/shader.h"
#include <memory>
using std::make_unique, std::unique_ptr;

class Cloud {
private:
    // Each cloud contains 3 circles and 1 rectangle
    // Store the shapes in a vector and use polymorphism
    // to draw
    vector<unique_ptr<Shape>> shapes;

public:
    // Constructors
    Cloud();
    Cloud(Shader& shader, vec2 pos);

    // Draw the cloud
    void setUniformsAndDraw() const;

    // This will allow us to move the clouds left and right
    void moveXWithinBounds(int delta, const unsigned int width);

    // Return true if r overlaps with the rectangle in the cloud
    // and false otherwise
    bool isOverlapping(const Rect& r) const;
};

#endif //RUNNER_CLOUD_H
