/**
 * @file core_optional.hh
 * @author Bartosz ksel (275472@student.pwr.edu.pl)
 * @brief Extension of std::optional capable of storing optional references.
 * @version 0.1
 * @date 2026-05-16
 *
 * @copyright Copyright (c) 2026
 *
 */

#pragma once

#include <memory>
#include <optional>
#include <type_traits>

namespace core {

// Base template for standard value types
template <typename T>
class optional {
    // --- TYPE TRAITS CHECKS ---
    static_assert(!std::is_reference_v<T>,
                  "core::optional primary template cannot hold references. Check for T&&.");
    static_assert(!std::is_same_v<std::remove_cv_t<T>, void>, "core::optional cannot hold void.");

private:
    std::optional<T> storage_;

public:
    constexpr optional() noexcept = default;
    constexpr optional(std::nullopt_t) noexcept {}
    constexpr optional(const T& v)
      : storage_(v)
    {
    }
    constexpr optional(T&& v)
      : storage_(std::move(v))
    {
    }

    constexpr bool has_value() const noexcept
    {
        return storage_.has_value();
    }
    constexpr explicit operator bool() const noexcept
    {
        return has_value();
    }

    constexpr T& operator*()
    {
        return *storage_;
    }
    constexpr const T& operator*() const
    {
        return *storage_;
    }
    constexpr T* operator->()
    {
        return &(*storage_);
    }
    constexpr const T* operator->() const
    {
        return &(*storage_);
    }

    constexpr T& value()
    {
        return storage_.value();
    }
    constexpr const T& value() const
    {
        return storage_.value();
    }
};

// --- THE SPECIALIZATION: Zero-overhead reference optional ---
template <typename T>
class optional<T&> {
    // --- TYPE TRAITS CHECKS ---
    static_assert(!std::is_reference_v<T>,
                  "Forming a reference-to-reference is invalid in core::optional.");

private:
    T* ptr_ = nullptr;

public:
    constexpr optional() noexcept = default;
    constexpr optional(std::nullopt_t) noexcept
      : ptr_(nullptr)
    {
    }

    constexpr optional(T& ref) noexcept
      : ptr_(std::addressof(ref))
    {
    }

    // Prevent dangling references from temporary construction
    optional(T&&) = delete;

    constexpr optional& operator=(std::nullopt_t) noexcept
    {
        ptr_ = nullptr;
        return *this;
    }

    constexpr optional& operator=(T& ref) noexcept
    {
        ptr_ = std::addressof(ref);
        return *this;
    }

    // --- CRITICAL FIX: Prevent dangling references from temporary assignment ---
    optional& operator=(T&&) = delete;

    constexpr bool has_value() const noexcept
    {
        return ptr_ != nullptr;
    }
    constexpr explicit operator bool() const noexcept
    {
        return has_value();
    }

    constexpr T& operator*() const
    {
        return *ptr_;
    }
    constexpr T* operator->() const
    {
        return ptr_;
    }

    constexpr T& value() const
    {
        if (!ptr_)
            throw std::bad_optional_access{};
        return *ptr_;
    }

    constexpr void reset() noexcept
    {
        ptr_ = nullptr;
    }
};

static constexpr std::nullopt_t nullopt = std::nullopt;

} // namespace core