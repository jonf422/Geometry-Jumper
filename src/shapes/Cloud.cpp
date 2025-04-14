//
// Created by Lisa Dion on 5/29/24.
//

#include "Cloud.h"

Cloud::Cloud() {};

Cloud::Cloud(Shader& shader, vec2 pos) {
    shapes.push_back(make_unique<Circle>(shader, vec2(pos.x + 3, pos.y + 15), vec2(5, 5), vec2(-1, 0), color(1, 1, 1, 1)));
    shapes.push_back(make_unique<Circle>(shader, vec2(pos.x - 10, pos.y + 5), vec2(5, 5), vec2(-1, 0), color(1, 1, 1, 1)));
    shapes.push_back(make_unique<Circle>(shader, vec2(pos.x + 10, pos.y + 5), vec2(5, 5), vec2(-1, 0), color(1, 1, 1, 1)));
    shapes.push_back(make_unique<Rect>(shader, pos, vec2(15, 15), color(1, 1, 1, 1)));
}

void Cloud::setUniformsAndDraw() const {
    for (const unique_ptr<Shape>& s : shapes) {
        s->setUniforms();
        s->draw();
    }
}

void Cloud::moveXWithinBounds(int delta, const unsigned int width) {
    for (const unique_ptr<Shape> &s: shapes) {
        s->moveX(delta);
        if (s->getPosX() < -20) {
            s->setPosX(width + 20);
        }
    }
}

bool Cloud::isOverlapping(const Rect& r) const {
    return r.isOverlapping(*shapes[3]);
}