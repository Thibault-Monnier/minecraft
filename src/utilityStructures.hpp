#pragma once

struct Vector3Int {
    int x;
    int y;
    int z;

    bool operator==(const Vector3Int& other) const noexcept {
        return x == other.x && y == other.y && z == other.z;
    }
};

namespace std {

template <>
struct hash<Vector3Int> {
    std::size_t operator()(const Vector3Int& v) const noexcept {
        return std::hash<int>()(v.x) ^ (std::hash<int>()(v.y) << 1) ^ (std::hash<int>()(v.z) << 2);
    }
};

}  // namespace std
