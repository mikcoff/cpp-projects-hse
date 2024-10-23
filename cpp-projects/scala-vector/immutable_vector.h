#pragma once

#include <cmath>
#include <cstddef>
#include <memory>
#include <vector>

const int kCHILD = 32;
const int kBITS = 5;

template <class T>
class ImmutableVector {
private:
    struct Node {
        Node(const T& v) : value(v), children(kCHILD, nullptr) {
        }
        Node(const T& v, const std::vector<std::shared_ptr<Node>>& child)
            : value(v), children(child) {
        }
        Node() : children(kCHILD, nullptr) {
        }
        T value;
        std::vector<std::shared_ptr<Node>> children;
    };

public:
    ImmutableVector() : tree_root_(std::make_shared<Node>(T())), size_(0) {
    }

    ImmutableVector(std::shared_ptr<Node> root, size_t size) : tree_root_(root), size_(size) {
    }

    explicit ImmutableVector(size_t count, const T& value = T()) : ImmutableVector() {
        for (size_t i = 0; i < count; ++i) {
            *this = PushBack(value);
        }
    }

    template <typename Iterator>
    ImmutableVector(Iterator first, Iterator last) : ImmutableVector() {
        for (Iterator& it = first; it != last; ++it) {
            *this = PushBack(*it);
        }
    }

    ImmutableVector(std::initializer_list<T> l) : ImmutableVector() {
        for (auto& el : l) {
            *this = PushBack(el);
        }
    }

    ImmutableVector Set(size_t index, const T& value) {
        return ImmutableVector(Set(index, value, tree_root_, 0), size_);
    }

    const T& Get(size_t index) const {
        return Get(index, tree_root_, 0);
    }

    ImmutableVector PushBack(const T& value) {
        return ImmutableVector(Set(size_, value, tree_root_, 0), size_ + 1);
    }

    ImmutableVector PopBack() {
        return ImmutableVector(Set(size_ - 1, T(), tree_root_, 0), size_ - 1);
    }

    size_t Size() const {
        return size_;
    }

private:
    std::shared_ptr<Node> tree_root_;
    size_t size_;

    std::shared_ptr<Node> Set(size_t index, const T& value, std::shared_ptr<Node> node,
                              size_t depth) {
        if (depth == CalculateDepth(index)) {
            if (node) {
                return std::make_shared<Node>(value, node->children);
            }
            return std::make_shared<Node>(value);
        }
        size_t current = (index >> (kBITS * depth)) & (kCHILD - 1);
        std::shared_ptr<Node> updated_node = std::make_shared<Node>(*node);
        if (!node->children[current]) {
            node->children[current] = std::make_shared<Node>(T());
        }
        updated_node->children[current] = Set(index, value, node->children[current], depth + 1);
        return updated_node;
    }

    size_t CalculateDepth(size_t index) const {
        if (index == 0) {
            return 1;
        }
        size_t num_bits = static_cast<size_t>(log2(index)) + 1;
        if (num_bits % kBITS == 0) {
            return num_bits / kBITS;
        }
        return num_bits / kBITS + 1;
    }

    const T& Get(size_t index, std::shared_ptr<Node> node, size_t depth) const {
        if (depth == CalculateDepth(index)) {
            return node->value;
        }
        size_t current = (index >> (kBITS * depth)) & (kCHILD - 1);
        return Get(index, node->children[current], depth + 1);
    }
};
