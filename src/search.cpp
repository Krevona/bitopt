#include "search.h"
#include "ops.h"

#include <chrono>
#include <cstring>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace bitopt {
namespace {

struct Node { Op op; int a; int b; int cost; };

uint64_t fnv1a(const uint8_t* p, size_t n) {
    uint64_t h = 1469598103934665603ull;
    for (size_t i = 0; i < n; ++i) { h ^= p[i]; h *= 1099511628211ull; }
    return h;
}

class Searcher {
public:
    explicit Searcher(const Spec& s) : spec_(s) { buildPoints(); }
    Result run(int maxLen);

private:
    const Spec& spec_;
    size_t n_ = 256;                       // number of test points
    int maxLen_ = 0;
    std::vector<uint8_t> xs_, ys_, target_, tmp_, arena_;
    std::vector<Node> nodes_;
    std::vector<std::vector<int>> byCost_;
    std::unordered_set<uint64_t> seen_;    // 64-bit hash of output vectors (collisions negligible)
    size_t evaluated_ = 0, falseFits_ = 0;
    Result res_;

    void buildPoints() {
        xs_.assign(n_, 0);
        ys_.assign(n_, 0);
        if (spec_.arity == 1) {
            for (size_t k = 0; k < n_; ++k) xs_[k] = static_cast<uint8_t>(k);   // exhaustive
        } else {
            static const uint8_t corners[][2] = {
                {0,0},{0,255},{255,0},{255,255},{1,1},{1,0},{0,1},
                {127,128},{128,127},{128,128},{127,127},{85,170},{170,85}};
            size_t k = 0;
            for (auto& c : corners) { xs_[k] = c[0]; ys_[k] = c[1]; ++k; }
            uint32_t s = 2463534242u;                                           // xorshift32
            for (; k < n_; ++k) {
                s ^= s << 13; s ^= s >> 17; s ^= s << 5;
                xs_[k] = static_cast<uint8_t>(s & 0xFF);
                ys_[k] = static_cast<uint8_t>((s >> 8) & 0xFF);
            }
        }
        target_.resize(n_);
        for (size_t k = 0; k < n_; ++k) target_[k] = spec_.fn(xs_[k], ys_[k]);
        tmp_.resize(n_);
    }

    uint8_t eval(int idx, uint8_t x, uint8_t y) const {
        const Node& nd = nodes_[idx];
        switch (nd.op) {
            case Op::Input: return nd.a == 0 ? x : y;
            case Op::Const: return static_cast<uint8_t>(nd.a);
            default: {
                uint8_t A = eval(nd.a, x, y);
                uint8_t B = arity(nd.op) == 2 ? eval(nd.b, x, y) : 0;
                return apply(nd.op, A, B);
            }
        }
    }

    bool verify(int root) const {            // full-domain check
        if (spec_.arity == 1) {
            for (int x = 0; x < 256; ++x)
                if (eval(root, x, 0) != spec_.fn(x, 0)) return false;
        } else {
            for (int x = 0; x < 256; ++x)
                for (int y = 0; y < 256; ++y)
                    if (eval(root, x, y) != spec_.fn(x, y)) return false;
        }
        return true;
    }

    void collect(int idx, std::vector<int>& order, std::unordered_set<int>& vis) const {
        if (!vis.insert(idx).second) return;
        const Node& nd = nodes_[idx];
        if (isLeaf(nd.op)) return;
        collect(nd.a, order, vis);
        if (arity(nd.op) == 2) collect(nd.b, order, vis);
        order.push_back(idx);
    }

    void finish(int root) {
        std::vector<int> order;
        std::unordered_set<int> vis;
        collect(root, order, vis);
        std::unordered_map<int, int> temp;
        for (size_t i = 0; i < order.size(); ++i) temp[order[i]] = static_cast<int>(i);

        auto name = [&](int idx) -> std::string {
            const Node& nd = nodes_[idx];
            if (nd.op == Op::Input) return nd.a == 0 ? "x" : "y";
            if (nd.op == Op::Const) return std::to_string(static_cast<int>(static_cast<int8_t>(nd.a)));
            return "t" + std::to_string(temp[idx]);
        };

        std::string out;
        for (int idx : order) {
            const Node& nd = nodes_[idx];
            out += "  t" + std::to_string(temp[idx]) + " = " + mnemonic(nd.op) + " " + name(nd.a);
            if (arity(nd.op) == 2) out += ", " + name(nd.b);
            out += "\n";
        }
        out += "  return " + name(root) + "\n";

        res_.found = true;
        res_.verified = true;
        res_.cost = nodes_[root].cost;
        res_.dagSize = static_cast<int>(order.size());
        res_.program = out;
    }

    bool addLeaf(Op op, int a, const uint8_t* vals) {
        if (!seen_.insert(fnv1a(vals, n_)).second) return false;
        nodes_.push_back({op, a, 0, 0});
        arena_.insert(arena_.end(), vals, vals + n_);
        byCost_[0].push_back(static_cast<int>(nodes_.size()) - 1);
        if (std::memcmp(vals, target_.data(), n_) == 0 && verify(static_cast<int>(nodes_.size()) - 1))
            finish(static_cast<int>(nodes_.size()) - 1);
        return true;
    }

    // Returns true when a verified solution has been found.
    bool consider(Op op, int a, int b, int c, bool last) {
        ++evaluated_;
        const uint8_t* A = &arena_[static_cast<size_t>(a) * n_];
        const uint8_t* B = arity(op) == 2 ? &arena_[static_cast<size_t>(b) * n_] : A;

        if (last) {                                   // scan only: early exit, nothing stored
            for (size_t k = 0; k < n_; ++k)
                if (apply(op, A[k], B[k]) != target_[k]) return false;
            nodes_.push_back({op, a, b, c});
            int id = static_cast<int>(nodes_.size()) - 1;
            if (verify(id)) { finish(id); return true; }
            nodes_.pop_back();
            ++falseFits_;
            return false;
        }

        for (size_t k = 0; k < n_; ++k) tmp_[k] = apply(op, A[k], B[k]);
        if (!seen_.insert(fnv1a(tmp_.data(), n_)).second) return false;   // equivalent value already known

        nodes_.push_back({op, a, b, c});
        int id = static_cast<int>(nodes_.size()) - 1;
        arena_.insert(arena_.end(), tmp_.begin(), tmp_.end());
        byCost_[c].push_back(id);

        if (std::memcmp(tmp_.data(), target_.data(), n_) == 0) {
            if (verify(id)) { finish(id); return true; }
            ++falseFits_;
        }
        return false;
    }

    bool runLevel(int c) {
        const bool last = (c == maxLen_);
        static const Op unary[]  = {Op::Not, Op::Neg};
        static const Op binary[] = {Op::Add, Op::Sub, Op::And, Op::Or, Op::Xor,
                                    Op::Shl, Op::Shr, Op::Sar};

        for (Op op : unary) {
            const auto& L = byCost_[c - 1];
            for (size_t i = 0; i < L.size(); ++i)
                if (consider(op, L[i], L[i], c, last)) return true;
        }
        for (Op op : binary) {
            const bool comm = commutative(op);
            for (int ca = 0; ca <= c - 1; ++ca) {
                int cb = c - 1 - ca;
                if (comm && ca > cb) continue;        // commutative: visit each pair once
                const auto& LA = byCost_[ca];
                const auto& LB = byCost_[cb];
                for (size_t i = 0; i < LA.size(); ++i) {
                    size_t j0 = (comm && ca == cb) ? i : 0;
                    for (size_t j = j0; j < LB.size(); ++j)
                        if (consider(op, LA[i], LB[j], c, last)) return true;
                }
            }
        }
        return false;
    }
};

Result Searcher::run(int maxLen) {
    auto t0 = std::chrono::steady_clock::now();
    maxLen_ = maxLen;
    byCost_.assign(static_cast<size_t>(maxLen) + 1, {});

    addLeaf(Op::Input, 0, xs_.data());
    if (spec_.arity == 2) addLeaf(Op::Input, 1, ys_.data());
    for (uint8_t cst : spec_.consts) {
        std::vector<uint8_t> v(n_, cst);
        addLeaf(Op::Const, cst, v.data());
    }

    for (int c = 1; c <= maxLen && !res_.found; ++c) runLevel(c);

    res_.nodesStored = arena_.size() / n_;
    res_.candidatesEvaluated = evaluated_;
    res_.falseFits = falseFits_;
    res_.seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
    return res_;
}

} // namespace

Result synthesize(const Spec& spec, int maxLen) {
    Searcher s(spec);
    return s.run(maxLen);
}

} // namespace bitopt