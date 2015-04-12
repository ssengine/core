#include "stage2d.h"

#include <assert.h>

ss_node2d::ss_node2d()
    : scale(1), rotation(0),
    prev(NULL), next(NULL), parent(NULL), first_child(NULL), last_child(NULL),
    need_calc(true), need_update(true)
{

}

ss_node2d::~ss_node2d()
{
    assert(parent == NULL && "Node was deleted before removed.");
    ss_node2d* next;
    for (auto itor = first_child; itor; itor = next){
        next = itor ? itor->next : nullptr;
#ifndef NDEBUG
        itor->parent = NULL;
#endif
        delete itor;
    }
}

void ss_node2d::_require_recalc(){
    need_calc = true;
    _require_update();
}

void ss_node2d::_require_update(){
    if (!need_update){
        need_update = true;
        if (parent){
            parent->_require_update();
        }
    }
}

//(V*A+B)*C+D = V*A*C+B*C+D
void ss_node2d::calc(){
    if (need_calc){
        need_calc = false;
        if (parent){
            transform = parent->get_transform();
            transform.add += transform.mul * displace;
            transform.mul *= scale;
            //transform.mul *= crotation;
            transform.mul *= std::complex<float>(cosf(rotation), sinf(rotation));
        }
        else {
            transform.add = displace;
            transform.mul = std::complex<float>(cosf(rotation), sinf(rotation))*scale;
        }
    }
    if (need_update){
        need_update = false;
        //recalc/update children.
        for (auto itor = first_child; itor; itor = itor->next){
            itor->calc();
        }
    }
}

void ss_node2d::insert_first(ss_node2d* other){
    if (other->parent){
        other->remove();
    }

    other->next = first_child;
    if (first_child){
        first_child->prev = other;
    }
    other->parent = this;
    first_child = other;
    if (!last_child){
        last_child = other;
    }
}

void ss_node2d::insert_last(ss_node2d* other){
    if (other->parent){
        other->remove();
    }
    other->prev = last_child;
    if (last_child){
        last_child->next = other;
    }
    other->parent = this;
    last_child = other;
    if (!first_child){
        first_child = other;
    }
}

void ss_node2d::insert_after(ss_node2d* other){
    assert(parent && "Cannot insert after a root node.");
    if (other->parent){
        other->remove();
    }
    other->prev = this;
    other->next = next;
    if (next){
        next->prev = other;
    }
    else {
        parent->last_child = other;
    }
    next = other;
}

void ss_node2d::insert_before(ss_node2d* other){
    assert(parent && "Cannot insert before a root node.");
    if (other->parent){
        other->remove();
    }
    other->prev = prev;
    other->next = this;
    if (prev){
        prev->next = other;
    }
    else {
        parent->first_child = other;
    }
    prev = other;
}

void ss_node2d::remove(){
    assert(parent && "Cannot remove a root node");
    if (next){
        next->prev = prev;
    }
    else {
        parent->last_child = prev;
    }
    if (prev){
        prev->next = next;
    }
    else {
        parent->first_child = next;
    }
    prev = NULL;
    next = NULL;
    parent = NULL;

    // Now I'm a root, but still keep my children.
}
