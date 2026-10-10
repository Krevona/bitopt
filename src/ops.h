#pragma once
#include <cstdint>

namespace bitopt {

enum class Op : uint8_t { Input, Const, Not, Neg, Add, Sub, And, Or, Xor, Shl, Shr, Sar };

inline bool isLeaf(Op o) { return o == Op::Input || o == Op::Const; }

inline int arity(Op o) {
    switch (o) {
        case Op::Input: case Op::Const: return 0;
        case Op::Not: case Op::Neg: return 1;
        default: return 2;
    }
}

inline bool commutative(Op o) {
    return o == Op::Add || o == Op::And || o == Op::Or || o == Op::Xor;
}

inline const char* mnemonic(Op o) {
    switch (o) {
        case Op::Not: return "not";  case Op::Neg: return "neg";
        case Op::Add: return "add";  case Op::Sub: return "sub";
        case Op::And: return "and";  case Op::Or:  return "or";
        case Op::Xor: return "xor";  case Op::Shl: return "shl";
        case Op::Shr: return "shr";  case Op::Sar: return "sar";
        default: return "?";
    }
}

// 8-bit semantics. Shift amounts are masked with 7.
inline uint8_t apply(Op o, uint8_t a, uint8_t b) {
    switch (o) {
        case Op::Not: return static_cast<uint8_t>(~a);
        case Op::Neg: return static_cast<uint8_t>(-a);
        case Op::Add: return static_cast<uint8_t>(a + b);
        case Op::Sub: return static_cast<uint8_t>(a - b);
        case Op::And: return static_cast<uint8_t>(a & b);
        case Op::Or:  return static_cast<uint8_t>(a | b);
        case Op::Xor: return static_cast<uint8_t>(a ^ b);
        case Op::Shl: return static_cast<uint8_t>(a << (b & 7));
        case Op::Shr: return static_cast<uint8_t>(a >> (b & 7));
        case Op::Sar: return static_cast<uint8_t>(static_cast<int8_t>(a) >> (b & 7));
        default: return 0;
    }
}

} // namespace bitopt