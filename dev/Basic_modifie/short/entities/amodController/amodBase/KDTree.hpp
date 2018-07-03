/*
 * KDTree.hpp
 *
 * Simple Templated KD-Tree implementation. T should have the [d] operator to give coordinates corresponding to dimension d.
 * T must also implement a size() to return the number of dimensions.
 *
 *  Created on: Mar 24, 2015
 *      Author: haroldsoh
 */

#pragma once

#include <iostream>
#include <vector>
#include <memory>
#include <stdexcept>
#include <random>
#include <ostream>
#include <queue>
#include <cmath>
#include <limits>

namespace sim_mob{

namespace kdt {

template<typename T>
struct KDTreeNode {
public:
    T data_;
    std::shared_ptr<KDTreeNode<T>> left_;
    std::shared_ptr<KDTreeNode<T>> right_;

    KDTreeNode() {}
    KDTreeNode(T& data) : data_(data) {}
    friend std::ostream &operator<<( std::ostream &out, KDTreeNode<T> node) {
	out << "(";
	for (int i=0; i<node.data_.dims()-1; i++) {
	    out << node.data_[i] << ", ";
	}
	out << node.data_[node.data_.dims()-1] << ")";
	return out;
    }
};

template<typename T>
class KDTree {
public:
    KDTree() : size_(0) {}
    virtual ~KDTree() {}

    void build(std::vector<T> &points);
    void print(std::ostream &out);
    T findNN(const T &p, double eps=-1.0) const;
    T findNN(const std::vector<double> &p, double eps=-1.0) const;
    int size() const { return size_; }

private:
    std::shared_ptr<KDTreeNode<T>> root_;

    std::default_random_engine generator;

    std::shared_ptr<KDTreeNode<T>> buildTreeHelper(std::vector<T>&points, int low, int high, int depth, int dim);

    // helper functions
    int partition(std::vector<T> &points, int low, int high, int pivot_idx, int axis);
    int quickSelect(std::vector<T> &points, int low, int high, int k, int axis);
    void swap(std::vector<T> &points, int i, int j);

    void findNNHelper(std::shared_ptr<KDTreeNode<T>> root, const T &p, T *best_elem, double *best_dist, int depth, int dim, double eps) const;

    double sqDist(const T& a, const T&b, int axis = -1) const;

    int size_; //size of the kd-tree
};





template<typename T>
void KDTree<T>::build(std::vector<T> &points) {
    // error checks
    if (points.size() == 0) return;
    int dim = 0;
    try {
	dim = points[0].dims();
    } catch (std::exception &e) {
	std::cout << e.what() << std::endl;
	throw e;
    }

    if (dim == 0) {
	throw std::runtime_error("KDTree: T's dims() function not defined or returned 0");
    }

    // standard case
    root_ = buildTreeHelper(points, 0, points.size()-1, 0, dim);
}

template<typename T>
std::shared_ptr<KDTreeNode<T>> KDTree<T>::buildTreeHelper(std::vector<T>&points, int low, int high, int depth, int dim) {
    // base case
    if (low == high) {
	std::shared_ptr<KDTreeNode<T>> pnode = std::make_shared<KDTreeNode<T>>(points[low]);
	size_++;
	return pnode;
    }
    if (low > high) return nullptr;


    // standard case
    int axis = depth % dim;
    int k = low + (high-low)/2;
    int med = quickSelect(points, low, high, k , axis);

    std::shared_ptr<KDTreeNode<T>> pnode = std::make_shared<KDTreeNode<T>>(points[med]);
    size_++;
    pnode->left_ = buildTreeHelper(points, low, med-1, depth+1, dim);
    pnode->right_ = buildTreeHelper(points, med+1, high, depth+1, dim);

    return pnode;
}

template<typename T>
T KDTree<T>::findNN(const T &p, double eps) const {
    //set eps to negative or 0 for exact search
    if (!root_) {
	throw std::runtime_error("KDTree: Build a tree before you can perform a NN search");
    }
    // initialize to root
    double best_dist = sqDist(root_->data_, p);
    T best_elem = root_->data_;
    findNNHelper(root_, p, &best_elem, &best_dist, 0, p.dims(), eps);
    return best_elem;
}

template<typename T>
T KDTree<T>::findNN(const std::vector<double> &x, double eps) const {
    //set eps to negative or 0 for exact search
    if (!root_) {
	throw std::runtime_error("KDTree: Build a tree before you can perform a NN search");
    }
    // create a data element
    T p;
    for (unsigned int i=0; i<x.size(); i++) {
	p[i] = x[i];
    }

    // return
    return findNN(p, eps);
}

template<typename T>
void KDTree<T>::findNNHelper(std::shared_ptr<KDTreeNode<T>> root, const T &p, T *best_elem,
		double *best_dist, int depth, int dim, double eps) const {
    if (!root) {
	return;
    }

    int axis = depth % dim;

    double dist = sqDist(root->data_, p);
    double dist_dim = sqDist(root->data_, p, axis);

    if (dist < *best_dist) {
	*best_dist = dist;
	*best_elem = root->data_;
    }

    // return if we found a completely matching element or are close enough (Approximate search)
    if (*best_dist == 0 || *best_dist < eps) {
	return;
    }

    // recurse through the tree
    findNNHelper((p[axis] < root->data_[axis]) ? root->left_ : root->right_, p, best_elem, best_dist, depth+1, dim, eps);

    if (*best_dist < dist_dim ) {
	return;
    }
    findNNHelper((p[axis] < root->data_[axis]) ? root->right_ : root->left_, p, best_elem, best_dist, depth+1, dim, eps);
}


template<typename T>
void KDTree<T>::print(std::ostream &out) {
    std::queue<std::shared_ptr<KDTreeNode<T>>> q;
    q.push(root_);
    int num_curr_level = 1;

    while (!q.empty()) {
	std::shared_ptr<KDTreeNode<T>> pnode = q.front();
	q.pop();
	num_curr_level--;

	// add children
	if (pnode) q.push(pnode->left_);
	if (pnode) q.push(pnode->right_);

	// print out the node
	if (pnode) {
	    std::cout << *pnode << " ";
	} else {
	    std::cout << "(NULL) ";
	}
	if (num_curr_level == 0) {
	    std::cout << std::endl;
	    num_curr_level = q.size();
	}
    }
}




template<typename T>
void KDTree<T>::swap(std::vector<T> &points, int i, int j) {
    T temp;
    temp = points[i];
    points[i] = points[j];
    points[j] = temp;
}

template<typename T>
int KDTree<T>::partition(std::vector<T> &points, int low, int high, int pivot_idx, int axis) {
    int part = low;
    swap(points, pivot_idx, high);

    for (int i = low; i <high; i++) {
	if (points[i][axis] < points[high][axis] ) {
	    swap(points, i, part);
	    part++;
	}
    }

    swap(points, part, high);
    return part;
}

template<typename T>
int KDTree<T>::quickSelect(std::vector<T> &points, int low, int high, int k, int axis) {
    if (low == high) {
	return low;
    }
    while (true) {
	std::uniform_int_distribution<int> urand(low, high);
	int pivot_idx = urand(generator);

	int part = partition(points, low, high, pivot_idx, axis);
	if (part == k) return part;
	if (k < part) {
	    high = part-1;
	} else {
	    low = part+1;
	}
    }
}

template<typename T>
double KDTree<T>::sqDist(const T &a, const T&b, int axis) const {
    if (a.dims() != b.dims()) throw std::runtime_error("KDTree: sqDist: elements don't have the same number of dims");

    int n = a.dims();
    double sum = 0;
    if (axis == -1) {
	for (int i=0; i<n; i++) {
	    sum += pow(a[i] - b[i], 2.0);
	}
    } else {
	if (axis >= n) {
	    throw std::runtime_error("KDTree: sqDist: axis is larger than dimension");
	}
	sum = pow(a[axis] - b[axis], 2.0);
    }
    return sum;
}
}

}
