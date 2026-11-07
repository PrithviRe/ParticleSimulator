#pragma once
#include <cmath>

struct Particle {
    float x = 0.0f;
    float y = 0.0f;

    float velocityX = 0.0f;
    float velocityY = 0.0f;

    float red = 1.0f;
    float green = 1.0f;
    float blue = 1.0f;

    float lifetime = 1.0f;
    float maxLifetime = 1.0f;

    void update(float deltaTime) {
        x += velocityX * deltaTime;
        y += velocityY * deltaTime;

        if (x > 1.0f || x < -1.0f) {
            velocityX *= -1.0f;
            x = (x > 1.0f) ? 1.0f : -1.0f;
        }

        if (y > 1.0f || y < -1.0f) {
            velocityY *= -1.0f;
            y = (y > 1.0f) ? 1.0f : -1.0f;
        }

        lifetime -= deltaTime;
    }

    float getAlpha() const {
        return lifetime > 0.0f ? lifetime / maxLifetime : 0.0f;
    }

    void limitSpeed(float maxSpeed) {
        float speedSquared = velocityX * velocityX + velocityY * velocityY;

        if (speedSquared > maxSpeed * maxSpeed) {
            float scale = maxSpeed / std::sqrt(speedSquared);

            velocityX *= scale;
            velocityY *= scale;
        }
    }
};
