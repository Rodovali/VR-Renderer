// VR Renderer - Macros
// Rodolphe VALICON
// 2025


#pragma once


/// @brief Generate an integer literal with only the nth bit set.
#define BIT(n) 1 << n

/// @brief Generate an integer literal with only the nth bit set.
#define BIT_B(b, n) 1##b << n