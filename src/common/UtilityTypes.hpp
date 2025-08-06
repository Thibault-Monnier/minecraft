#pragma once

#include <optional>

template <typename T>
using OptionalRef = std::optional<std::reference_wrapper<const T>>;