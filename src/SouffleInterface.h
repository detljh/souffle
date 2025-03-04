/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2015, Oracle and/or its affiliates. All rights reserved
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

/************************************************************************
 *
 * @file CompiledSouffle.h
 *
 * Main include file for generated C++ classes of Souffle
 *
 ***********************************************************************/

#pragma once

#include "RamTypes.h"
#include "SymbolTable.h"

#include <initializer_list>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include <cassert>
#include <cstddef>

namespace souffle {

class tuple;

/**
 * Object-oriented wrapper class for Souffle's templatized relations.
 *
 * Tuples can be inserted into a relation. To access the stored tuples, iterator_base and iteraor are
 * used. A relation is manipulated by the souffle program (create new relation, load input etc).
 *
 * A relation stores the master-copy collection of tuples and their
 * indices in tables. (check CompiledRelation.h for more details.)
 * Which table is a single link-list structure. (check Table.h for more details.)
 */
class Relation {
protected:
    /**
     * Abstract iterator class.
     *
     * When tuples are inserted into a relation, they will be stored contiguously.
     * Intially, the iterator_base of a relation will point to the first tuple inserted.
     * iterator_base can be moved to point to the next tuple until the end.
     * The tuple iterator_base is pointing to can be accessed.
     * However, users can not use this to access tuples since iterator class is protected.
     * Instead, they should use the public class - iterator which interacts with iterator_base.
     */
    class iterator_base {
    protected:
        /**
         * Required for identifying type of iterator
         * (NB: LLVM has no typeinfo).
         *
         * TODO (Honghyw) : Provide a clear documentation of what id is used for.
         */
        uint32_t id;

    public:
        /**
         * Get the ID of the iterator_base object.
         *
         * @return ID of the iterator_base object (unit32_t)
         */
        virtual uint32_t getId() const {
            return id;
        }

        /**
         * Constructor.
         *
         * Create an instance of iterator_base and set its ID to be arg_id.
         *
         * @param arg_id ID of an iterator object (unit32_t)
         */
        iterator_base(uint32_t arg_id) : id(arg_id) {}

        /**
         * Destructor.
         */
        virtual ~iterator_base() = default;

        /**
         * Overload the "++" operator.
         *
         * Increment the iterator_base so that the iterator_base will now point to the next tuple.
         * The definition of this overloading has to be defined by the child class of iterator_base.
         */
        virtual void operator++() = 0;

        /**
         * Overload the "*" operator.
         *
         * Return the tuple that is pointed to by the iterator_base.
         * The definition of this overloading has to be defined by the child class of iterator_base.
         *
         * @return tuple Reference to a tuple object
         */
        virtual tuple& operator*() = 0;

        /**
         * Overload the "==" operator.
         *
         * @param o Reference to an object of the iterator_base class
         * @return A boolean value, if the ID of o is the same as the ID of the current object and equal(o)
         * returns true. Otherwise return false
         */
        bool operator==(const iterator_base& o) const {
            return this->getId() == o.getId() && equal(o);
        }

        /**
         * Clone the iterator_base.
         * The definition of clone has to be defined by the child class of iterator_base.
         *
         * @return An iterator_base pointer
         */
        virtual iterator_base* clone() const = 0;

    protected:
        /**
         * Check if the passed-in object of o is the the same as the current iterator_base.
         *
         * TODO (Honghyw) : Provide a clear documentation of what equal function does.
         *
         * @param o Reference to an object of the iterator_base class
         * @return A boolean value. If two iterator_base are the same return true. Otherwise return false
         */
        virtual bool equal(const iterator_base& o) const = 0;
    };

public:
    /**
     * Destructor.
     */
    virtual ~Relation() = default;

    /**
     * Wrapper class for abstract iterator.
     *
     * Users must use iterator class to access the tuples stored in a relation.
     */
    class iterator {
    protected:
        /*
         * Iterator_base class pointer.
         *
         * TODO (Honghyw) : Provide a documentation that explains why we need to set iter to null prt.
         */
        iterator_base* iter = nullptr;

    public:
        /**
         * Constructor.
         */
        iterator() = default;

        /**
         * Constructor.
         *
         * Initialize the iter to be the same as arg.
         *
         * TODO (Honghyw) : Provide a documentation explaining why this is useful.
         *
         * @param arg An iterator_base class pointer
         */
        iterator(iterator_base* arg) : iter(arg) {}

        /**
         * Destructor.
         *
         * The iterator_base instance iter is pointing is destructed.
         */
        ~iterator() {
            delete iter;
        }

        /**
         * Constructor.
         *
         * Initialize the iter to be the clone of arg.
         *
         * TODO (Honghyw) : Provide a documentation explaining why this is useful
         *
         * @param o Reference to an iterator object
         */
        iterator(const iterator& o) : iter(o.iter->clone()) {}

        /**
         * Overload the "=" operator.
         *
         * The original iterator_base instance is destructed.
         */
        iterator& operator=(const iterator& o) {
            delete iter;
            iter = o.iter->clone();
            return *this;
        }

        /**
         * Overload the "++" operator.
         *
         * Increment the iterator_base object that iter is pointing to so that iterator_base object points to
         * next tuple.
         *
         * @return Reference to the iterator object which points to the next tuple in a relation
         */
        iterator& operator++() {
            ++(*iter);
            return *this;
        }

        /**
         * Overload the "*" operator.
         *
         * This will return the tuple that the iterator is pointing to.
         *
         * @return Reference to a tuple object
         */

        tuple& operator*() const {
            return *(*iter);
        }

        /**
         * Overload the "==" operator.
         *
         * Check if either the iter of o and the iter of current object are the same or the corresponding
         * iterator_base objects are the same.
         *
         * @param o Reference to a iterator object
         * @return Boolean. True, if either of them is true. False, otherwise
         */
        bool operator==(const iterator& o) const {
            return (iter == o.iter) || (*iter == *o.iter);
        }

        /**
         * Overload the "!=" operator.
         *
         * Check if the iterator object o is not the same as the current object.
         *
         * @param o Reference to a iterator object
         * @return Boolean. True, if they are not the same. False, otherwise
         */
        bool operator!=(const iterator& o) const {
            return !(*this == o);
        }
    };

    /**
     * Insert a new tuple into the relation.
     * The definition of insert function has to be defined by the child class of relation class.
     *
     * @param t Reference to a tuple class object
     */
    virtual void insert(const tuple& t) = 0;

    /**
     * Check whether a tuple exists in a relation.
     * The definition of contains has to be defined by the child class of relation class.
     *
     * @param t Reference to a tuple object
     * @return Boolean. True, if the tuple exists. False, otherwise
     */
    virtual bool contains(const tuple& t) const = 0;

    /**
     * Return an iterator pointing to the first tuple of the relation.
     * This iterator is used to access the tuples of the relation.
     *
     * @return Iterator
     */
    virtual iterator begin() const = 0;

    /**
     * Return an iterator pointing to next to the last tuple of the relation.
     *
     * @return Iterator
     */
    virtual iterator end() const = 0;

    /**
     * Get the number of tuples in a relation.
     *
     * @return The number of tuples in a relation (std::size_t)
     */
    virtual std::size_t size() const = 0;

    /**
     * Get the name of a relation.
     *
     * @return The name of a relation (std::string)
     */
    virtual std::string getName() const = 0;

    /**
     * Get the attribute type of a relation at the column specified by the parameter.
     * The attribute type is in the form "<primitive type>:<type name>".
     * <primitive type> can be s or n standing for symbol or number which are two primitive types in Souffle.
     * <type name> is the name given by the user in the Souffle Program after ".type", "symbol_type" or
     * "number_type". For example, for ".type Node", the type name is "Node".
     *
     * @param The index of the column starting starting from 0 (size_t)
     * @return The constant string of the attribute type
     */
    virtual const char* getAttrType(size_t) const = 0;

    /**
     * Get the attribute name of a relation at the column specified by the parameter.
     * The attribute name is the name given to the type by the user in the .decl statement. For example, for
     * ".decl edge (node1:Node, node2:Node)", the attribute names are node1 and node2.
     *
     * @param The index of the column starting starting from 0 (size_t)
     * @return The constant string of the attribute name
     */
    virtual const char* getAttrName(size_t) const = 0;

    /**
     * Get the arity of a relation.
     * For example for a tuple (1 2) the arity is 2 and for a tuple (1 2 3) the arity is 3.
     *
     * @return Arity of a relation (size_t)
     */
    virtual size_t getArity() const = 0;

    /**
     * Get the symbol table of a relation.
     * The symbols in a tuple to be stored into a relation are stored and assigned with a number in a table
     * called symbol table. For example, to insert ("John","Student") to a relation, "John" and "Student" are
     * stored in symbol table and they are assigned with number say 0 and 1. After this, instead of inserting
     * ("John","Student"), (0, 1) is inserted. When accessing this tuple, 0 and 1 will be looked up in the
     * table and replaced by "John" and "Student". This is done so to save memory space if same symbols are
     * inserted many times. Symbol table has many rows where each row contains a symbol and its corresponding
     * assigned number.
     *
     * @return Reference to a symbolTable object
     */
    virtual SymbolTable& getSymbolTable() const = 0;

    /**
     * Get the signature of a relation.
     * The signature is in the form <<primitive type 1>:<type name 1>,<primitive type 2>:<type name 2>...> for
     * all the attributes in a relation. For example, <s:Node,s:Node>. The primitive type and type name are
     * explained in getAttrType.
     *
     * @return String of the signature of a relation
     */
    std::string getSignature() {
        if (getArity() == 0) {
            return "<>";
        }

        std::string signature = "<" + std::string(getAttrType(0));
        for (size_t i = 1; i < getArity(); i++) {
            signature += "," + std::string(getAttrType(i));
        }
        signature += ">";
        return signature;
    }

    /**
     * Delete all the tuples in relation.
     *
     * When purge() is called, it sets the head and tail of the table (table is a
     * singly-linked list structure) to nullptr, and for every elements
     * in the table, set the next element pointer points to the current element itself.
     */
    virtual void purge() = 0;
};

/**
 * Defines a tuple for the OO interface such that
 * relations with varying columns can be accessed.
 *
 * Tuples are stored in relations.
 * In Souffle, one row of data to be stored into a relation is represented as a tuple.
 * For example if we have a relation called dog with attributes name, colour and age which are string, string
 * and interger type respectively. One row of data a relation to be stored can be (mydog, black, 3). However,
 * this is not directly stored as a tuple. There will be a symbol table storing the actual content and
 * associate them with numbers (For example, |1|mydog| |2|black| |3|3|). And when this row of data is stored
 * as a tuple, (1, 2, 3) will be stored.
 */
class tuple {
    /**
     * The relation to which the tuple belongs.
     */
    const Relation& relation;

    /**
     * Dynamic array used to store the elements in a tuple.
     */
    std::vector<RamDomain> array;

    /**
     * pos shows what the current position of a tuple is.
     * Initially, pos is 0 meaning we are at the head of the tuple.
     * If we have an empty tuple and try to insert things, pos lets us know where to insert the element. After
     * the element is inserted, pos will be incremented by 1. If we have a tuple with content, pos lets us
     * know where to read the element. After we have read one element, pos will be incremented by 1. pos also
     * helps to make sure we access an insert a tuple within the bound by making sure pos never exceeds the
     * arity of the relation.
     */
    size_t pos;

public:
    /**
     * Constructor.
     *
     * Tuples are constructed here by passing a relation, then may be subsequently inserted into that same
     * passed relation. The passed relation pointer will be stored within the tuple instance, while the arity
     * of the relation will be used to initialize the vector holding the elements of the tuple. Where such an
     * element is of integer type, it will be stored directly within the vector. Otherwise, if the element is
     * of a string type, the index of that string within the associated symbol table will be stored instead.
     * The tuple also stores the index of some "current" element, referred to as its position. The constructor
     * initially sets this position to the first (zeroth) element of the tuple, while subsequent methods of
     * this class use that position for element access and modification.
     *
     * @param r Relation pointer pointing to a relation
     */
    tuple(const Relation* r) : relation(*r), array(r->getArity()), pos(0), data(array.data()) {}

    /**
     * Constructor.
     *
     * Tuples are constructed here by passing a tuple.
     * The relation to which the passed tuple belongs to will be stored.
     * The array of the passed tuple, which stores the elements will be stroed.
     * The pos will be set to be the same as the pos of passed tuple.
     * belongs to.
     *
     * @param Reference to a tuple object.
     */
    tuple(const tuple& t) : relation(t.relation), array(t.array), pos(t.pos), data(array.data()) {}

    /**
     * Allows printing using WriteStream.
     */
    const RamDomain* data = nullptr;

    /**
     * Get the reference to the relation to which the tuple belongs.
     *
     * @return Reference to a relation.
     */
    const Relation& getRelation() const {
        return relation;
    }

    /**
     * Return the number of elements in the tuple.
     *
     * @return the number of elements in the tuple (size_t).
     */
    size_t size() const {
        return array.size();
    }

    /**
     * Overload the operator [].
     *
     * Direct access to tuple elements via index and
     * return the element in idx position of a tuple.
     *
     * TODO (Honghyw) : This interface should be hidden and
     * only be used by friendly classes such as
     * iterators; users should not use this interface.
     *
     * @param idx This is the idx of element in a tuple (size_t).
     */
    RamDomain& operator[](size_t idx) {
        return array[idx];
    }

    /**
     * Overload the operator [].
     *
     * Direct access to tuple elements via index and
     * Return the element in idx position of a tuple. The returned element can not be changed.
     *
     * TODO (Honghyw) : This interface should be hidden and
     * only be used by friendly classes such as
     * iterators; users should not use this interface.
     *
     * @param idx This is the idx of element in a tuple (size_t).
     */
    const RamDomain& operator[](size_t idx) const {
        return array[idx];
    }

    /**
     * Reset the index giving the "current element" of the tuple to zero.
     */
    void rewind() {
        pos = 0;
    }

    /**
     * Set the "current element" of the tuple to the given string, then increment the index giving the current
     * element.
     *
     * @param str Symbol to be added (std::string)
     * @return Reference to the tuple
     */
    tuple& operator<<(const std::string& str) {
        assert(pos < size() && "exceeded tuple's size");
        assert(*relation.getAttrType(pos) == 's' && "wrong element type");
        array[pos++] = relation.getSymbolTable().lookup(str);
        return *this;
    }

    /**
     * Set the "current element" of the tuple to the given number, then increment the index giving the current
     * element.
     *
     * @param number Number to be added (RamDomain)
     * @return Reference to the tuple
     */
    tuple& operator<<(RamDomain number) {
        assert(pos < size() && "exceeded tuple's size");
        assert((*relation.getAttrType(pos) == 'i' || *relation.getAttrType(pos) == 'r') &&
                "wrong element type");
        array[pos++] = number;
        return *this;
    }

    /**
     * Get the "current element" of the tuple as a string, then increment the index giving the current
     * element.
     *
     * @param str Symbol to be loaded from the tuple(std::string)
     * @return Reference to the tuple
     */
    tuple& operator>>(std::string& str) {
        assert(pos < size() && "exceeded tuple's size");
        assert(*relation.getAttrType(pos) == 's' && "wrong element type");
        str = relation.getSymbolTable().resolve(array[pos++]);
        return *this;
    }

    /**
     * Get the "current element" of the tuple as a number, then increment the index giving the current
     * element.
     *
     * @param number Number to be loaded from the tuple (RamDomain)
     * @return Reference to the tuple
     */
    tuple& operator>>(RamDomain& number) {
        assert(pos < size() && "exceeded tuple's size");
        assert((*relation.getAttrType(pos) == 'i' || *relation.getAttrType(pos) == 'r') &&
                "wrong element type");
        number = array[pos++];
        return *this;
    }

    /**
     * Iterator for direct access to tuple's data.
     *
     * @see Relation::iteraor::begin()
     */
    decltype(array)::iterator begin() {
        return array.begin();
    }

    /**
     * Direct constructor using initialization list.
     *
     * TODO (Honghyw) : Provide a ducumentation explainning what this is doing.
     */
    tuple(Relation* r, std::initializer_list<RamDomain> il) : relation(*r), array(il), pos(il.size()) {
        assert(il.size() == r->getArity() && "wrong tuple arity");
    }
};

/**
 * Abstract base class for generated Datalog programs.
 */
class SouffleProgram {
private:
    /**
     * Define a relation map for external access, when getRelation(name) is called,
     * the relation with the given name will be returned from this map,
     * relationMap stores all the relations in a map with its name
     * as the key and relation as the value.
     */
    std::map<std::string, Relation*> relationMap;

    /**
     * inputRelations stores all the input relation in a vector.
     */
    std::vector<Relation*> inputRelations;

    /**
     * outputRelations stores all the output relation in a vector.
     */
    std::vector<Relation*> outputRelations;

    /**
     * internalRelation stores all the relation in a vector that are neither an input or an output.
     */
    std::vector<Relation*> internalRelations;

    /**
     * allRelations store all the relation in a vector.
     */
    std::vector<Relation*> allRelations;
    std::size_t numThreads = 1;

protected:
    /**
     * Add the relation to relationMap (with its name) and allRelations,
     * depends on the properties of the relation, if the relation is an input relation, it will be added to
     * inputRelations, else if the relation is an output relation, it will be added to outputRelations,
     * otherwise will add to internalRelations. (a relation could be both input and output at the same time.)
     *
     * @param name the name of the relation (std::string)
     * @param rel a pointer to the relation (std::string)
     * @param isInput a bool argument, true if the relation is a input relation, else false (bool)
     * @param isOnput a bool argument, true if the relation is a ouput relation, else false (bool)
     */
    void addRelation(const std::string& name, Relation* rel, bool isInput, bool isOutput) {
        relationMap[name] = rel;
        allRelations.push_back(rel);
        if (isInput) {
            inputRelations.push_back(rel);
        }
        if (isOutput) {
            outputRelations.push_back(rel);
        }
        if (!isInput && !isOutput) {
            internalRelations.push_back(rel);
        }
    }

public:
    /**
     * Destructor.
     *
     * Destructor of SouffleProgram.
     */
    virtual ~SouffleProgram() = default;

    /**
     * Execute the souffle program, without any loads or stores.
     */
    virtual void run(size_t stratumIndex = -1) {}

    /**
     * Execute program, loading inputs and storing outputs as required.
     * Read all input relations and store all output relations from the given directory.
     * First argument is the input directory, second argument is the output directory,
     * if no directory is given, the default is to use the current working directory.
     * The implementation of this function occurs in the C++ code generated by Souffle.
     * To view the generated C++ code, run Souffle with the `-g` option.
     */
    virtual void runAll(std::string inputDirectory = ".", std::string outputDirectory = ".",
            size_t stratumIndex = -1) = 0;

    /**
     * Read all input relations from the given directory. If no directory is given, the
     * default is to use the current working directory. The implementation of this function
     * occurs in the C++ code generated by Souffle. To view the generated C++ code, run
     * Souffle with the `-g` option.
     */
    virtual void loadAll(std::string inputDirectory = ".") = 0;

    /**
     * Store all output relations individually in .CSV file in the given directory, with
     * the relation name as the file name of the .CSV file. If no directory is given, the
     * default is to use the current working directory. The implementation of this function
     * occurs in the C++ code generated by Souffle. To view the generated C++ code, run
     * Souffle with the `-g` option.
     */
    virtual void printAll(std::string outputDirectory = ".") = 0;

    /**
     * Output all the input relations in stdout, without generating any files. (for debug purposes).
     */
    virtual void dumpInputs(std::ostream& out = std::cout) = 0;

    /**
     * Output all the output relations in stdout, without generating any files. (for debug purposes).
     */
    virtual void dumpOutputs(std::ostream& out = std::cout) = 0;

    /**
     * Set the number of threads to be used
     */
    void setNumThreads(std::size_t numThreads) {
        this->numThreads = numThreads;
    }

    /**
     * Get the number of threads to be used
     */
    std::size_t getNumThreads() {
        return numThreads;
    }

    /**
     * Get Relation by its name from relationMap, if relation not found, return a nullptr.
     *
     * @param name The name of the target relation (const std::string)
     * @return The pointer of the target relation, or null pointer if the relation not found (Relation*)
     */
    Relation* getRelation(const std::string& name) const {
        auto it = relationMap.find(name);
        if (it != relationMap.end()) {
            return (*it).second;
        } else {
            return nullptr;
        }
    };

    /**
     * Return the size of the target relation from relationMap.
     *
     * @param name The name of the target relation (const std::string)
     * @return The size of the target relation (std::size_t)
     */
    std::size_t getRelationSize(const std::string& name) const {
        return getRelation(name)->size();
    }

    /**
     * Return the name of the target relation from relationMap.
     *
     * @param name The name of the target relation (const std::string)
     * @return The name of the target relation (std::string)
     */
    std::string getRelationName(const std::string& name) const {
        return getRelation(name)->getName();
    }

    /**
     * Getter of outputRelations, which this vector structure contains all output relations.
     *
     * @return outputRelations (std::vector)
     * @see outputRelations
     */
    std::vector<Relation*> getOutputRelations() const {
        return outputRelations;
    }

    /**
     * Getter of inputRelations, which this vector structure contains all input relations.
     *
     * @return intputRelations (std::vector)
     * @see inputRelations
     */
    std::vector<Relation*> getInputRelations() const {
        return inputRelations;
    }

    /**
     * Getter of internalRelations, which this vector structure contains all relations
     * that are neither an input relation or an output relation.
     *
     * @return internalRelations (std::vector)
     * @see internalRelations
     */
    std::vector<Relation*> getInternalRelations() const {
        return internalRelations;
    }

    /**
     * Getter of allRelations, which this vector structure contains all relations.
     *
     * @return allRelations (std::vector)
     * @see allRelations
     */
    std::vector<Relation*> getAllRelations() const {
        return allRelations;
    }

    /**
     * TODO (NubKel) : Should provide documentation for this method.
     */
    virtual void executeSubroutine(std::string name, const std::vector<RamDomain>& args,
            std::vector<RamDomain>& ret, std::vector<bool>& retErr) {}

    /**
     * Get the symbol table of the program.
     */
    virtual SymbolTable& getSymbolTable() = 0;

    /**
     * Remove all the tuples from the outputRelations, calling the purge method of each.
     *
     * @see Relation::purge()
     */
    void purgeOutputRelations() {
        for (Relation* relation : outputRelations) {
            relation->purge();
        }
    }

    /**
     * Remove all the tuples from the inputRelations, calling the purge method of each.
     *
     * @see Relation::purge()
     */
    void purgeInputRelations() {
        for (Relation* relation : inputRelations) {
            relation->purge();
        }
    }

    /**
     * Remove all the tuples from the internalRelations, calling the purge method of each.
     *
     * @see Relation::purge()
     */
    void purgeInternalRelations() {
        for (Relation* relation : internalRelations) {
            relation->purge();
        }
    }

    /**
     * Helper function for the wrapper function Relation::insert() and Relation::contains().
     */
    template <typename Tuple, size_t N>
    struct tuple_insert {
        static void add(const Tuple& t, souffle::tuple& t1) {
            tuple_insert<Tuple, N - 1>::add(t, t1);
            t1 << std::get<N - 1>(t);
        }
    };

    /**
     * Helper function for the wrapper function Relation::insert() and Relation::contains() for the
     * first element of the tuple.
     */
    template <typename Tuple>
    struct tuple_insert<Tuple, 1> {
        static void add(const Tuple& t, souffle::tuple& t1) {
            t1 << std::get<0>(t);
        }
    };

    /**
     * Insert function with std::tuple as input (wrapper)
     *
     * @param t The insert tuple (std::tuple)
     * @param relation The relation that perform insert operation (Relation*)
     * @see Relation::insert()
     */
    template <typename... Args>
    void insert(const std::tuple<Args...>& t, Relation* relation) {
        tuple t1(relation);
        tuple_insert<decltype(t), sizeof...(Args)>::add(t, t1);
        relation->insert(t1);
    }

    /**
     * Contains function with std::tuple as input (wrapper)
     *
     * @param t The existence searching tuple (std::tuple)
     * @param relation The relation that perform contains operation (Relation*)
     * @return A boolean value, return true if the tuple found, otherwise return false
     * @see Relation::contains()
     */
    template <typename... Args>
    bool contains(const std::tuple<Args...>& t, Relation* relation) {
        tuple t1(relation);
        tuple_insert<decltype(t), sizeof...(Args)>::add(t, t1);
        return relation->contains(t1);
    }
};

/**
 * Abstract program factory class.
 */
class ProgramFactory {
protected:
    /**
     * Singly linked-list to store all program factories
     * Note that STL data-structures are not possible due
     * to "static initialization order fiasco (problem)".
     * (The problem of the order static objects get initialized, causing effect
     * such as program access static variables before they initialized.)
     * The static container needs to be a primitive type such as pointer
     * set to NULL.
     * Link to next factory.
     */
    ProgramFactory* link = nullptr;

    /**
     * The name of factory.
     */
    std::string name;

protected:
    /**
     * Constructor.
     *
     * Constructor adds factory to static singly-linked list
     * for registration.
     */
    ProgramFactory(std::string name) : name(std::move(name)) {
        registerFactory(this);
    }

private:
    /**
     * Helper method for creating a factory map, which map key is the name of the program factory, map value
     * is the pointer of the ProgramFactory.
     *
     * TODO (NubKel) : Improve documentation of use and interaction between inline and static, here and for
     * the whole class.
     *
     * @return The factory registration map (std::map)
     */
    static inline std::map<std::string, ProgramFactory*>& getFactoryRegistry() {
        static std::map<std::string, ProgramFactory*> factoryReg;
        return factoryReg;
    }

protected:
    /**
     * Create and insert a factory into the factoryReg map.
     *
     * @param factory Pointer of the program factory (ProgramFactory*)
     */
    static inline void registerFactory(ProgramFactory* factory) {
        auto& entry = getFactoryRegistry()[factory->name];
        assert(!entry && "double-linked/defined souffle analyis");
        entry = factory;
    }

    /**
     * Find a factory by its name, return the fatory if found, return nullptr if the
     * factory not found.
     *
     * @param factoryName The factory name (const std::string)
     * @return The pointer of the target program factory, or null pointer if the program factory not found
     * (ProgramFactory*)
     */
    static inline ProgramFactory* find(const std::string& factoryName) {
        const auto& reg = getFactoryRegistry();
        auto pos = reg.find(factoryName);
        return (pos == reg.end()) ? nullptr : pos->second;
    }

    /**
     * Create new instance (abstract).
     */
    virtual SouffleProgram* newInstance() = 0;

public:
    /**
     * Destructor.
     *
     * Destructor of ProgramFactory.
     */
    virtual ~ProgramFactory() = default;

    /**
     * Create an instance by finding the name of the program factory, return nullptr if the instance not
     * found.
     *
     * @param name Instance name (const std::string)
     * @return The new instance(SouffleProgram*), or null pointer if the instance not found
     */
    static SouffleProgram* newInstance(const std::string& name) {
        ProgramFactory* factory = find(name);
        if (factory != nullptr) {
            return factory->newInstance();
        } else {
            return nullptr;
        }
    }
};
}  // namespace souffle
