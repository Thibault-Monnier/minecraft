#pragma once

#include <algorithm>

struct Vector3Int {
    int x;
    int y;
    int z;

    bool operator==(const Vector3Int& other) const noexcept {
        return x == other.x && y == other.y && z == other.z;
    }

    Vector3Int operator+(const Vector3Int& other) const noexcept {
        return {x + other.x, y + other.y, z + other.z};
    }
};

struct Vector2Int {
    int x;
    int y;

    bool operator==(const Vector2Int& other) const noexcept {
        return x == other.x && y == other.y;
    }

    Vector2Int operator+(const Vector2Int& other) const noexcept {
        return {x + other.x, y + other.y};
    }
};

// template <>
// struct std::hash<Vector3Int> {
//     std::size_t operator()(const Vector3Int& v) const noexcept {
//         return std::hash<int>()(v.x) ^ (std::hash<int>()(v.y) << 1) ^ (std::hash<int>()(v.z) <<
//         2);
//     }
// };

template <typename H>
H AbslHashValue(H hash, Vector3Int const& v) noexcept {
    return H::combine(std::move(hash), v.x, v.y, v.z);
}