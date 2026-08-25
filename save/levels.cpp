#include "levels.h"
#include "physics.h"

Level_system::Level_system() {
    Signature s = ecs.update_signature<Collider>();
    collectors.push_back(Collector{s, false});

    s = ecs.update_signature<Camera2D>();
    collectors.push_back(Collector{s, false});

    Level level;
    level.range = vec4(-16.0f, 0.0f, 16.0f, 32.0f);

    levels.push_back(level);
}

void Level_system::load_level(uint32_t i) {
    Level l = levels[i];

    uint32_t camera_entity = *collectors[1].entities.begin();

    Camera2D& camera = ecs.get_component<Camera2D>(camera_entity);
    Transform2D& camera_transform = ecs.get_component<Transform2D>(camera_entity);

    camera_transform.position = ivec2(0, (l.range.x + l.range.y) * 0.5f);

    camera.scale = 1.0f / 8.0f;//((l.range.y - l.range.x) * 0.4f);

    range = l.range;
}